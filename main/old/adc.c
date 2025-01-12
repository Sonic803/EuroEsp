
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
#include "defines.h"

static const char *TAG = "espvco adc";

/*---------------------------------------------------------------
        ADC Calibration
---------------------------------------------------------------*/
static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

    if (!calibrated)
    {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK)
        {
            calibrated = true;
        }
    }

    *out_handle = handle;
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "Calibration Success");
    }
    else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated)
    {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    }
    else
    {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

adc_oneshot_unit_handle_t adc_handle;

adc_unit_t unit;
adc_channel_t pots_chan[2];
adc_channel_t jack_chan[3];

adc_cali_handle_t pots_cal_chan[2];
adc_cali_handle_t jack_cal_chan[3];

void configAdc(void)
{

    adc_oneshot_io_to_channel(POT0_GPIO, &unit, &pots_chan[0]);
    adc_oneshot_io_to_channel(POT1_GPIO, &unit, &pots_chan[1]);
    adc_oneshot_io_to_channel(JACK0_GPIO, &unit, &jack_chan[0]);
    adc_oneshot_io_to_channel(JACK1_GPIO, &unit, &jack_chan[1]);
    adc_oneshot_io_to_channel(JACK2_GPIO, &unit, &jack_chan[2]);

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = unit,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, pots_chan[0], &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, pots_chan[1], &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, jack_chan[0], &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, jack_chan[1], &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, jack_chan[2], &config));

    example_adc_calibration_init(unit, pots_chan[0], ADC_ATTEN_DB_12, &pots_cal_chan[0]);
    example_adc_calibration_init(unit, pots_chan[1], ADC_ATTEN_DB_12, &pots_cal_chan[1]);
    example_adc_calibration_init(unit, jack_chan[0], ADC_ATTEN_DB_12, &jack_cal_chan[0]);
    example_adc_calibration_init(unit, jack_chan[1], ADC_ATTEN_DB_12, &jack_cal_chan[1]);
    example_adc_calibration_init(unit, jack_chan[2], ADC_ATTEN_DB_12, &jack_cal_chan[2]);

    ESP_LOGI(TAG, "finished configAdc");

    // // //-------------ADC1 Calibration Init---------------//
    // adc_cali_handle_t adc1_cali_chan0_handle = NULL;
    // adc_cali_handle_t adc1_cali_chan1_handle = NULL;
    // bool do_calibration1_chan0 = example_adc_calibration_init(unit3, pwm3_chan, ADC_ATTEN_DB_12, &adc1_cali_chan0_handle);

    // int adc_raw, voltage;
    // while (1)
    // {
    //     ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, pwm3_chan, &adc_raw));
    //     if (do_calibration1_chan0)
    //     {
    //         ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan0_handle, adc_raw, &voltage));
    //         ESP_LOGI(TAG, "ADC Raw: %d\tVoltage: %d mV",adc_raw , voltage);
    //     }
    //     // ESP_LOGI(TAG, "ADC2 Channel[%d] Raw Data: %d", pwm1_chan, adc1_raw);

    //     vTaskDelay(pdMS_TO_TICKS(100));
    // }
}