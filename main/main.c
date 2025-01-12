/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "driver/dac_oneshot.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_filter.h"
#include "main.h"

#include <assert.h>
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_filter.h"

#include "driver/gpio.h"
#include "defines.h"

// #define EXAMPLE_DAC_CHAN0_ADC_CHAN          ADC_CHANNEL_6   // GPIO17, same as DAC channel 0
// #define EXAMPLE_DAC_CHAN1_ADC_CHAN          ADC_CHANNEL_7

#define EXAMPLE_ADC_UNIT ADC_UNIT_2
#define _EXAMPLE_ADC_UNIT_STR(unit) #unit
#define EXAMPLE_ADC_UNIT_STR(unit) _EXAMPLE_ADC_UNIT_STR(unit)
#define EXAMPLE_ADC_CONV_MODE ADC_CONV_SINGLE_UNIT_2
#define EXAMPLE_DAC_CHAN0_ADC_CHAN ADC_CHANNEL_1 // GPIO15
#define EXAMPLE_DAC_CHAN1_ADC_CHAN ADC_CHANNEL_5 // GPIO16
#define EXAMPLE_ADC_WIDTH ADC_BITWIDTH_DEFAULT
#define EXAMPLE_ADC_ATTEN ADC_ATTEN_DB_0
#define EXAMPLE_ADC_GET_CHANNEL(p_data) ((p_data)->type1.channel)
#define EXAMPLE_ADC_GET_DATA(p_data) ((p_data)->type1.data)
#define NO_OF_SAMPLES (8) //< ADC multisampling number of samples

static const char *TAG = "espvco";
float f;

uint8_t sin_wav[EXAMPLE_ARRAY_LEN]; // Used to store sine wave values
uint8_t tri_wav[EXAMPLE_ARRAY_LEN]; // Used to store triangle wave values
uint8_t saw_wav[EXAMPLE_ARRAY_LEN]; // Used to store sawtooth wave values
uint8_t squ_wav[EXAMPLE_ARRAY_LEN]; // Used to store square wave values

adc_oneshot_unit_handle_t adc2_handle;
adc_continuous_handle_t adc2cont_handle;
static TaskHandle_t s_task_handle;

#define NO_OF_CHANNELS (2)

static adc_channel_t channel[] = {ADC_CHANNEL_6}; //,ADC_CHANNEL_7};

static void example_generate_wave(void)
{
    uint32_t pnt_num = EXAMPLE_ARRAY_LEN;

    for (int i = 0; i < pnt_num; i++)
    {
        sin_wav[i] = (uint8_t)((sin(i * CONST_PERIOD_2_PI / pnt_num) + 1) * (double)(EXAMPLE_DAC_AMPLITUDE) / 2 + 0.5);
        tri_wav[i] = (i > (pnt_num / 2)) ? (2 * EXAMPLE_DAC_AMPLITUDE * (pnt_num - i) / pnt_num) : (2 * EXAMPLE_DAC_AMPLITUDE * i / pnt_num);
        saw_wav[i] = (i == pnt_num) ? 0 : (i * EXAMPLE_DAC_AMPLITUDE / pnt_num);
        squ_wav[i] = (i < (pnt_num / 2)) ? EXAMPLE_DAC_AMPLITUDE : 0;
    }
}

int vcoVal;
int lfoVal;
int pwm1Val;
int pwm2Val;
bool updated = false;
bool digi1 = true, digi2 = true;
extern int pots_val[2];
extern int jack_val[3];

extern adc_oneshot_unit_handle_t adc_handle;
int adc1_raw, adc2_raw, adc3_raw, adc4_raw, adc5_raw;
extern adc_channel_t pwm1_chan, pwm2_chan, pwm3_chan, pwm4_chan, pwm5_chan;

void configure_gpio_input(int gpio_num)
{
    // Create a GPIO configuration structure
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio_num),    // Select the GPIO pin(s) to configure
        .mode = GPIO_MODE_INPUT,               // Set the pin as input
        .pull_up_en = GPIO_PULLUP_ENABLE,      // Disable pull-up resistor
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // Disable pull-down resistor
        .intr_type = GPIO_INTR_DISABLE         // Disable interrupts
    };

    // Apply the configuration
    gpio_config(&io_conf);
}
// void configure_gpio_input(int gpio_num)
// {
//     // Create a GPIO configuration structure
//     gpio_config_t io_conf = {
//         .pin_bit_mask = (1ULL << gpio_num),    // Select the GPIO pin(s) to configure
//         .mode = GPIO_MODE_DISABLE,             // Set the pin as input
//         .pull_up_en = GPIO_PULLUP_DISABLE,     // Disable pull-up resistor
//         .pull_down_en = GPIO_PULLDOWN_ENABLE, // Disable pull-down resistor
//         .intr_type = GPIO_INTR_DISABLE         // Disable interrupts
//     };

//     // Apply the configuration
//     gpio_config(&io_conf);
// }




void app_main(void)
{
    // vTaskDelay(pdMS_TO_TICKS(3000));

    // gpio_reset_pin(18);
    // gpio_reset_pin(10);
    // configure_gpio_input(18);
    // configure_gpio_input(2);
    example_generate_wave();

    gpio_dump_io_configuration(stdout, (1ULL << 17) | (1ULL << 18));

    vcoVal = 0;
    lfoVal = 100;
    pwm1Val = 0;
    pwm2Val = 0;

    configDac();
    // configAdc();
    configAdcContinous();
    configDisplay();
    configPwm();
    configEncoder();
    configDigital();
    configUpdate();

    // dump lfo configuriation TODO
    //  gpio_dump_io_configuration(stdout, (1ULL << 1) | (1ULL << 2) | (1ULL << 7) | (1ULL << 8)| (1ULL << 9)| (1ULL << 10));
    //  gpio_dump_io_configuration(stdout, (1ULL << 17) | (1ULL << 18));

    int count = 0;

    startGraphic();
    while (1)
    {
        runGraphic();

        // if (pressed)
        // {
        //     ESP_LOGI(TAG, "Pressed");

        // }

        // // read_adc();
        // ESP_LOGI(TAG, "Pots: %d %d Jacks: %d %d %d", pots_val[0], pots_val[1], jack_val[0], jack_val[1], jack_val[2]);

        // char str[10];
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    // while (1)
    // {
    //     vTaskDelay(pdMS_TO_TICKS(1000));
    // }

    // configDisplay();
}
