/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_filter.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_timer.h"

#include "defines.h"

#define _EXAMPLE_ADC_UNIT_STR(unit) #unit
#define EXAMPLE_ADC_UNIT_STR(unit) _EXAMPLE_ADC_UNIT_STR(unit)
#define EXAMPLE_ADC_CONV_MODE ADC_CONV_SINGLE_UNIT_1
#define EXAMPLE_ADC_ATTEN ADC_ATTEN_DB_12
#define EXAMPLE_ADC_BIT_WIDTH SOC_ADC_DIGI_MAX_BITWIDTH

#define EXAMPLE_ADC_OUTPUT_TYPE ADC_DIGI_OUTPUT_FORMAT_TYPE1
#define EXAMPLE_ADC_GET_CHANNEL(p_data) ((p_data)->type1.channel)
#define EXAMPLE_ADC_GET_DATA(p_data) ((p_data)->type1.data)

int num_ch;

int total_bytes;

static TaskHandle_t s_task_handle;
static const char *TAG = "EXAMPLE";

static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
{
    BaseType_t mustYield = pdFALSE;
    // Notify that ADC continuous driver has done enough number of conversions
    vTaskNotifyGiveFromISR(s_task_handle, &mustYield);

    return (mustYield == pdTRUE);
}

adc_continuous_handle_t handle = NULL;
adc_cali_handle_t handle_cali = NULL;

adc_unit_t unit;
adc_channel_t pots_chan[2];
adc_channel_t jack_chan[3];
adc_channel_t all_chan[5];
uint32_t max_chann = 0;

int pots_val[2];
int jack_val[3];
int vals[32];

DRAM_ATTR SemaphoreHandle_t enabledMutex;

void IRAM_ATTR read_adc()
{

    if (handle == NULL || handle_cali == NULL)
        return;

    if (xSemaphoreTakeFromISR(enabledMutex, NULL) == pdFALSE)
        return;

    uint8_t result[12] = {0};
    uint32_t ret_num = 0;
    esp_err_t ret = adc_continuous_read(handle, result, total_bytes, &ret_num, 0);
    int mv = 0;

    if (ret == ESP_OK)
    {
        for (int i = 0; i < ret_num; i += SOC_ADC_DIGI_RESULT_BYTES)
        {
            adc_digi_output_data_t *p = (adc_digi_output_data_t *)&result[i];
            uint32_t chan_num = EXAMPLE_ADC_GET_CHANNEL(p);
            if (chan_num < max_chann)
            {
                uint32_t data = EXAMPLE_ADC_GET_DATA(p);
                adc_cali_raw_to_voltage(handle_cali, data, &mv);
                // vals[chan_num] = mv * 2450 / 1100;
                vals[chan_num] = mv * 2200 / 1100;
            }
        }
        pots_val[0] = vals[pots_chan[0] & 0x7];
        pots_val[1] = vals[pots_chan[1] & 0x7];
        jack_val[0] = vals[jack_chan[0] & 0x7] * 2.0851;
        jack_val[1] = vals[jack_chan[1] & 0x7] * 2.0851;
        jack_val[2] = vals[jack_chan[2] & 0x7] * 2.0851;
    }
    // TODO possibly error
    xSemaphoreGiveFromISR(enabledMutex,NULL);
    return;
}

struct enableAdc
{
    bool pots[2];
    bool jacks[3];
};

void configAdcEnabled(struct enableAdc enable)
{

    if (xSemaphoreTake(enabledMutex, portMAX_DELAY) == pdFALSE)
    {
        return;
    }

    if (handle != NULL)
    {
        esp_err_t ret = adc_continuous_stop(handle);
        if (ret != ESP_OK)
        {
            xSemaphoreGive(enabledMutex);
            return;
        }
        adc_continuous_deinit(handle);
        handle = NULL;
    }

    num_ch = enable.pots[0] + enable.pots[1] + enable.jacks[0] + enable.jacks[1] + enable.jacks[2];

        int freq = 20000 * num_ch;

    if (freq > 80000)
    {
        freq = 80000;
    }

    total_bytes = 2 * (num_ch);

    total_bytes = total_bytes + total_bytes % 4; // Make it multiple of 4

    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = total_bytes,
        .conv_frame_size = total_bytes,
        .flags.flush_pool = 1,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = freq,
        .conv_mode = EXAMPLE_ADC_CONV_MODE,
        .format = EXAMPLE_ADC_OUTPUT_TYPE,
    };
    int j = 0;
    for (int i = 0; i < 2; i++)
    {
        if (enable.pots[i])
        {
            all_chan[j] = pots_chan[i];
            j++;
        }
    }
    for (int i = 0; i < 3; i++)
    {
        if (enable.jacks[i])
        {
            all_chan[j] = jack_chan[i];
            j++;
        }
    }
    ESP_LOGI(TAG, "EXAMPLE_ADC_BIT_WIDTH is :%" PRIu8, EXAMPLE_ADC_BIT_WIDTH);

    adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
    dig_cfg.pattern_num = num_ch;
    for (int i = 0; i < num_ch; i++)
    {
        adc_pattern[i].atten = EXAMPLE_ADC_ATTEN;
        adc_pattern[i].channel = all_chan[i] & 0x7;
        adc_pattern[i].unit = unit;
        adc_pattern[i].bit_width = EXAMPLE_ADC_BIT_WIDTH;

        ESP_LOGI(TAG, "adc_pattern[%d].atten is :%" PRIx8, i, adc_pattern[i].atten);
        ESP_LOGI(TAG, "adc_pattern[%d].channel is :%" PRIx8, i, adc_pattern[i].channel);
        ESP_LOGI(TAG, "adc_pattern[%d].unit is :%" PRIx8, i, adc_pattern[i].unit);
    }
    dig_cfg.adc_pattern = adc_pattern;
    ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));

    ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");

    adc_continuous_evt_cbs_t cbs = {
        .on_conv_done = s_conv_done_cb,
    };
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(handle, &cbs, NULL));
    ESP_ERROR_CHECK(adc_continuous_start(handle));

    ESP_LOGI(TAG, "finished configAdcEnabled");
    xSemaphoreGive(enabledMutex);
    return;
}

void configAdcContinous(void)
{

    adc_continuous_io_to_channel(POT0_GPIO, &unit, &pots_chan[0]);
    adc_continuous_io_to_channel(POT1_GPIO, &unit, &pots_chan[1]);
    adc_continuous_io_to_channel(JACK0_GPIO, &unit, &jack_chan[0]);
    adc_continuous_io_to_channel(JACK1_GPIO, &unit, &jack_chan[1]);
    adc_continuous_io_to_channel(JACK2_GPIO, &unit, &jack_chan[2]);

    pots_chan[0] = pots_chan[0] & 0x7;
    pots_chan[1] = pots_chan[1] & 0x7;
    jack_chan[0] = jack_chan[0] & 0x7;
    jack_chan[1] = jack_chan[1] & 0x7;
    jack_chan[2] = jack_chan[2] & 0x7;

    max_chann = SOC_ADC_CHANNEL_NUM(unit);

    s_task_handle = xTaskGetCurrentTaskHandle();
    enabledMutex = xSemaphoreCreateMutex();

    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = unit,
        .atten = EXAMPLE_ADC_ATTEN,
        .bitwidth = SOC_ADC_DIGI_MAX_BITWIDTH,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config, &handle_cali));

    configAdcEnabled((struct enableAdc){.pots = {true, true}, .jacks = {true, true, true}});

    ESP_LOGI(TAG, "finished configAdcContinous");
}
