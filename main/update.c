#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"
#include "driver/dac_oneshot.h"
#include "main.h"
#include "esp_log.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/ledc.h"
#include "esp_err.h"

#include <assert.h>
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_filter.h"

#include "defines.h"

extern dac_oneshot_handle_t chan0_handle;
extern dac_oneshot_handle_t chan1_handle;

int vcoVal;
int lfoVal;
int pwm1Val;
int pwm2Val;
bool digi1, digi2;
bool updated;
extern adc_oneshot_unit_handle_t adc_handle;
extern int pots_val[2];
extern int jack_val[3];
extern adc_channel_t pwm1_chan, pwm2_chan;

static const char *TAG = "update";

static bool IRAM_ATTR on_timer_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    // ESP_DRAM_LOGI(TAG, "Timer1 callback triggered");
    ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan0_handle, vcoVal));
    ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan1_handle, 255-lfoVal));

    ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM0_CHANNEL, pwm1Val);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, PWM0_CHANNEL);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM1_CHANNEL, pwm2Val);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, PWM1_CHANNEL);

    gpio_set_level(DIGI1_GPIO, digi1);
    gpio_set_level(DIGI2_GPIO, digi2);
    // digi1 = !digi1;

    updated = true;

    return false;
}

typedef void (*FunctionPointer)();

extern FunctionPointer updateFunction;

static bool IRAM_ATTR on_timer_alarm_cb2(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    static int count = 0;
    read_adc();
    if (updateFunction != NULL)
    {
        updateFunction();
    }

    count = (count + 1) % 3000;

    return false;
}

void configUpdate(void)
{

    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_RESOLUTION,
        .intr_priority = 2,

    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0,
        .alarm_count = TIMER_COUNT,
        .flags.auto_reload_on_alarm = true,
    };
    gptimer_event_callbacks_t cbs = {
        .on_alarm = on_timer_alarm_cb,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    ESP_ERROR_CHECK(gptimer_start(gptimer));

    gptimer_handle_t gptimer2 = NULL;
    gptimer_config_t timer_config2 = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_RESOLUTION,
        .intr_priority = 1,
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config2, &gptimer2));

    gptimer_alarm_config_t alarm_config2 = {
        .reload_count = 0,
        .alarm_count = TIMER_COUNT,
        .flags.auto_reload_on_alarm = true,
    };
    gptimer_event_callbacks_t cbs2 = {
        .on_alarm = on_timer_alarm_cb2,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer2, &cbs2, NULL));
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer2, &alarm_config2));

    ESP_ERROR_CHECK(gptimer_enable(gptimer2));
    ESP_ERROR_CHECK(gptimer_start(gptimer2));

    ESP_LOGI(TAG, "finished initUpdate");
}
