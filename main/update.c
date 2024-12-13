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
extern adc_channel_t pwm1_chan,pwm2_chan;

static const char *TAG = "update";

static bool IRAM_ATTR on_timer_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan0_handle, vcoVal));
    ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan1_handle, -lfoVal));

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_LS_CH0_CHANNEL, pwm1Val*0.5);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_LS_CH0_CHANNEL);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_LS_CH1_CHANNEL, pwm2Val);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_LS_CH1_CHANNEL);
    
    // adc_oneshot_read(adc_handle, pwm1_chan, &adc1_raw);  
    // ESP_ERROR_CHECK();
    // ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, pwm2_chan, &adc2_raw));

    updated=true;

    return false;
}

void configUpdate(void)
{

    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = EXAMPLE_TIMER_RESOLUTION, // 1MHz, 1 tick = 1us
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
    ESP_LOGI(TAG, "finished initUpdate");
}
