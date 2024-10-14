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
#define NO_OF_SAMPLES (8)  //< ADC multisampling number of samples

static const char *TAG = "espvco";
float f;

uint8_t sin_wav[EXAMPLE_ARRAY_LEN]; // Used to store sine wave values
uint8_t tri_wav[EXAMPLE_ARRAY_LEN]; // Used to store triangle wave values
uint8_t saw_wav[EXAMPLE_ARRAY_LEN]; // Used to store sawtooth wave values
uint8_t squ_wav[EXAMPLE_ARRAY_LEN]; // Used to store square wave values

adc_oneshot_unit_handle_t adc2_handle;
adc_continuous_handle_t adc2cont_handle;
static TaskHandle_t s_task_handle;

static adc_channel_t channel[2] = {ADC_CHANNEL_2};

static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
{
    BaseType_t mustYield = pdFALSE;
    // Notify that ADC continuous driver has done enough number of conversions
    vTaskNotifyGiveFromISR(s_task_handle, &mustYield);

    return (mustYield == pdTRUE);
}

static void adc_monitor_task(void *args)
{

    int chan0_val = 0;
    int chan1_val = 0;
    while (1)
    {
        /* Read the DAC output voltage */
        // ESP_ERROR_CHECK(adc_oneshot_read(adc2_handle, EXAMPLE_DAC_CHAN0_ADC_CHAN, &chan0_val));
        // printf("DAC channel 0 value: %4d\t", chan0_val);
        // f=chan0_val;

        // Read from adc2cont_handle
        uint8_t result[NO_OF_SAMPLES * SOC_ADC_DIGI_DATA_BYTES_PER_CONV] = {0};
        uint32_t ret_num = 0;

        char unit[] = EXAMPLE_ADC_UNIT_STR(EXAMPLE_ADC_UNIT);

        esp_err_t ret = adc_continuous_read(adc2cont_handle, result, sizeof(result), &ret_num, 0);

        if (ret == ESP_OK)
        {
            printf("Unit %s: ", unit);
            int count=0;
            int sum=0;
            for (int i = 0; i < ret_num; i += SOC_ADC_DIGI_RESULT_BYTES)
            {

                adc_digi_output_data_t *p = (adc_digi_output_data_t *)&result[i];
                uint32_t chan_num = EXAMPLE_ADC_GET_CHANNEL(p);
                uint32_t data = EXAMPLE_ADC_GET_DATA(p);
                // printf("Channel: %" PRIu32 ", Value: %ld\t", chan_num, data);
                sum+=data;
                count++;
            }
            f=(float)sum/count;
            printf("f: %f,\t ret_num:%d \n ",f,(int)ret_num/SOC_ADC_DIGI_RESULT_BYTES);
        }
        else
        {
            // printf("\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
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
    configDac();
    configAdc();

    adc_oneshot_unit_init_cfg_t adc_cfg = {
        .unit_id = EXAMPLE_ADC_UNIT,
        .ulp_mode = false,
    };
    // ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_cfg, &adc2_handle));
    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = EXAMPLE_ADC_ATTEN,
        .bitwidth = EXAMPLE_ADC_WIDTH,

    };
    // ESP_ERROR_CHECK(adc_oneshot_config_channel(adc2_handle, EXAMPLE_DAC_CHAN0_ADC_CHAN, &chan_cfg));
    // ESP_ERROR_CHECK(adc_oneshot_config_channel(adc2_handle, EXAMPLE_DAC_CHAN1_ADC_CHAN, &chan_cfg));
    /* Create ADC monitor task to detect the voltage on DAC pin */

    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = NO_OF_SAMPLES * SOC_ADC_DIGI_DATA_BYTES_PER_CONV,
        .conv_frame_size = NO_OF_SAMPLES * SOC_ADC_DIGI_DATA_BYTES_PER_CONV,
        .flags.flush_pool = 0,
    };

    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &adc2cont_handle));

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 20 * 1000,
        .conv_mode = EXAMPLE_ADC_CONV_MODE,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE1,
    };
    adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
    dig_cfg.pattern_num = 1;
    for (int i = 0; i < 2; i++)
    {
        adc_pattern[i].atten = EXAMPLE_ADC_ATTEN;
        adc_pattern[i].channel = channel[i] & 0x7;
        adc_pattern[i].unit = EXAMPLE_ADC_UNIT;
        adc_pattern[i].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;

        ESP_LOGI(TAG, "adc_pattern[%d].atten is :%" PRIx8, i, adc_pattern[i].atten);
        ESP_LOGI(TAG, "adc_pattern[%d].channel is :%" PRIx8, i, adc_pattern[i].channel);
        ESP_LOGI(TAG, "adc_pattern[%d].unit is :%" PRIx8, i, adc_pattern[i].unit);
    }
    dig_cfg.adc_pattern = adc_pattern;

    ESP_ERROR_CHECK(adc_continuous_config(adc2cont_handle, &dig_cfg));

    adc_continuous_iir_filter_config_t iir_cfg = {
        .unit = EXAMPLE_ADC_UNIT,
        .channel =ADC_CHANNEL_2,
        .coeff=ADC_DIGI_IIR_FILTER_COEFF_64
    };
    adc_iir_filter_handle_t adc_filter_handle;
    ESP_ERROR_CHECK(adc_new_continuous_iir_filter(adc2cont_handle,&iir_cfg,&adc_filter_handle));
    ESP_ERROR_CHECK(adc_continuous_iir_filter_enable(adc_filter_handle));


    s_task_handle = xTaskGetCurrentTaskHandle();

    adc_continuous_evt_cbs_t cbs = {
        .on_conv_done = s_conv_done_cb,
    };
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(adc2cont_handle, &cbs, NULL));
    ESP_ERROR_CHECK(adc_continuous_start(adc2cont_handle));

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    xTaskCreate(adc_monitor_task, "adc_monitor_task", 4096, adc2_handle, 5, NULL);
}
