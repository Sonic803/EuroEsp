#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"
#include "driver/dac_oneshot.h"
#include "main.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "defines.h"

#define LEDC_LS_MODE LEDC_LOW_SPEED_MODE
#define LEDC_LS_TIMER LEDC_TIMER_1

static const char *TAG = "pwm";

void configPwm(void)
{
    ESP_LOGI(TAG, "started configPwm");
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_8_BIT, // resolution of PWM duty
        .freq_hz = 100000,                   // frequency of PWM signal
        .speed_mode = LEDC_LS_MODE,          // timer mode
        .timer_num = LEDC_LS_TIMER,          // timer index
        .clk_cfg = LEDC_AUTO_CLK,            // Auto select the source clock
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel0 = {
        .channel = PWM0_CHANNEL,
        .duty = 0,
        .gpio_num = PWM0_GPIO,
        .speed_mode = LEDC_LS_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_LS_TIMER,
        .flags.output_invert = 0,
        .duty = 100};
    ledc_channel_config(&ledc_channel0);

    ledc_channel_config_t ledc_channel1 = {
        .channel = PWM1_CHANNEL,
        .duty = 0,
        .gpio_num = PWM1_GPIO,
        .speed_mode = LEDC_LS_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_LS_TIMER,
        .flags.output_invert = 0,
        .duty = 100};
    ledc_channel_config(&ledc_channel1);

    ESP_LOGI(TAG, "finished configPwm");
}