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

#define num_ch 5

#define TOTAL_BYTES (num_ch * 2 * 2)

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

adc_unit_t unit;
adc_channel_t pots_chan[2];
adc_channel_t jack_chan[3];
adc_channel_t all_chan[num_ch];

int pots_val[2];
int jack_val[3];
int vals[20];

void read_adc()
{

    // time this function
    uint8_t result[TOTAL_BYTES] = {0};
    uint32_t ret_num = 0;
    esp_err_t ret = adc_continuous_read(handle, result, TOTAL_BYTES, &ret_num, 0);

    if (ret == ESP_OK)
    {
        // ESP_LOGI("TASK", "ret is %x, ret_num is %" PRIu32 " bytes", ret, ret_num);
        for (int i = 0; i < ret_num; i += SOC_ADC_DIGI_RESULT_BYTES)
        {
            adc_digi_output_data_t *p = (adc_digi_output_data_t *)&result[i];
            uint32_t chan_num = EXAMPLE_ADC_GET_CHANNEL(p);
            uint32_t data = EXAMPLE_ADC_GET_DATA(p);
            /* Check the channel number validation, the data is invalid if the channel num exceed the maximum channel */
            if (chan_num < SOC_ADC_CHANNEL_NUM(unit))
            {
                // ESP_LOGI(TAG, "Channel: %"PRIu32", Value: %"PRIx32, chan_num, data);
                vals[chan_num] = data;
            }
            else
            {
                // ESP_LOGW(TAG, "Invalid data [%" PRIu32 "_%" PRIx32 "]", chan_num, data);
            }
        }
        /**
         * Because printing is slow, so every time you call `ulTaskNotifyTake`, it will immediately return.
         * To avoid a task watchdog timeout, add a delay here. When you replace the way you process the data,
         * usually you don't need this delay (as this task will block for a while).
         */
        pots_val[0] = vals[pots_chan[0] & 0x7];
        pots_val[1] = vals[pots_chan[1] & 0x7];
        jack_val[0] = vals[jack_chan[0] & 0x7];
        jack_val[1] = vals[jack_chan[1] & 0x7];
        jack_val[2] = vals[jack_chan[2] & 0x7];
        // for (int i = 0; i < num_ch; i++)
        // {
        //     int chan_num = all_chan[i] & 0x7;
        //     // ESP_LOGI(TAG, "Channel: %d, Value: %d", chan_num, vals[chan_num]);
        // }
    }
}

void configAdcContinous(void)
{

    adc_continuous_io_to_channel(POT0_GPIO, &unit, &pots_chan[0]);
    adc_continuous_io_to_channel(POT1_GPIO, &unit, &pots_chan[1]);
    adc_continuous_io_to_channel(JACK0_GPIO, &unit, &jack_chan[0]);
    adc_continuous_io_to_channel(JACK1_GPIO, &unit, &jack_chan[1]);
    adc_continuous_io_to_channel(JACK2_GPIO, &unit, &jack_chan[2]);

    s_task_handle = xTaskGetCurrentTaskHandle();

    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = TOTAL_BYTES,
        .conv_frame_size = TOTAL_BYTES,
        .flags.flush_pool = 1,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 20 * 1000,
        .conv_mode = EXAMPLE_ADC_CONV_MODE,
        .format = EXAMPLE_ADC_OUTPUT_TYPE,
    };

    memcpy(all_chan, pots_chan, sizeof(pots_chan));
    memcpy(all_chan + 2, jack_chan, sizeof(jack_chan));

    // print EXAMPLE_ADC_BIT_WIDTH
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

    // adc_continuous_iir_filter_config_t iir_cfg = {
    //     .unit = unit,
    //     .channel = POT0_GPIO,
    //     .coeff = ADC_DIGI_IIR_FILTER_COEFF_64};
    // adc_iir_filter_handle_t adc_filter_handle;
    // ESP_ERROR_CHECK(adc_new_continuous_iir_filter(handle, &iir_cfg, &adc_filter_handle));
    // ESP_ERROR_CHECK(adc_continuous_iir_filter_enable(adc_filter_handle));


    adc_continuous_evt_cbs_t cbs = {
        .on_conv_done = s_conv_done_cb,
    };
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(handle, &cbs, NULL));
    ESP_ERROR_CHECK(adc_continuous_start(handle));

    // while(1){
    //     read_adc();
    //     vTaskDelay(pdMS_TO_TICKS(50));
    // }

    // ESP_ERROR_CHECK(adc_continuous_stop(handle));
    // ESP_ERROR_CHECK(adc_continuous_deinit(handle));
}
