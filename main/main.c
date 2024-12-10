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

void app_main(void)
{

    example_generate_wave();
    //configDac();

    // while (1)
    // {
    //     vTaskDelay(pdMS_TO_TICKS(1000));
    // }

    //configPwm();
    configAdc();
}
