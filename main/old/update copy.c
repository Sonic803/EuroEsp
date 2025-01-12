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

#define EXAMPLE_TIMER_RESOLUTION 1000000 // 1MHz, 1 tick = 1us
#define EXAMPLE_TIMER_ALARM_COUNT 50     // The count value that trigger the timer alarm callback
#define EXAMPLE_TIMER_ALARM_COUNT2 50    // The count value that trigger the timer alarm callback

extern dac_oneshot_handle_t chan0_handle;
extern dac_oneshot_handle_t chan1_handle;

extern int vcoVal;
extern int lfoVal;
extern int pwm1Val;
extern int pwm2Val;
extern bool updated;
extern adc_oneshot_unit_handle_t adc_handle;
extern int adc1_raw;
extern int adc2_raw;
extern adc_channel_t pwm1_chan, pwm2_chan;

extern bool digi1, digi2;

static const char *TAG = "update";

static bool IRAM_ATTR on_timer_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    // ESP_DRAM_LOGI(TAG, "Timer1 callback triggered");
    vcoVal = (vcoVal + 1) % 255;
    ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan0_handle, vcoVal));
    ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan1_handle, -lfoVal));

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

static bool IRAM_ATTR on_timer_alarm_cb2(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    // ESP_DRAM_LOGI(TAG, "Timer2 callback triggered");

    read_adc();
    pwm1Val = (pwm1Val + 1) % 255;
    int i = 0;
    while (i < 1000)
    {
        i++;
    }
    // while(1){

    // }

    uint64_t current_time = 0;
    gptimer_get_raw_count(timer, &current_time);

    gptimer_alarm_config_t alarm_config = {
        .alarm_count = current_time + 500,
    };
    gptimer_set_alarm_action(timer, &alarm_config);

    return false;
}

void configUpdate(void)
{

    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = EXAMPLE_TIMER_RESOLUTION, // 1MHz, 1 tick = 1us
        .intr_priority = 1,

    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0,
        .alarm_count = EXAMPLE_TIMER_ALARM_COUNT,
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
        .resolution_hz = EXAMPLE_TIMER_RESOLUTION, // 1MHz, 1 tick = 1us
        .intr_priority = 0,
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config2, &gptimer2));

    gptimer_alarm_config_t alarm_config2 = {
        .reload_count = 0,
        .alarm_count = EXAMPLE_TIMER_ALARM_COUNT2,
        .flags.auto_reload_on_alarm = false,
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
