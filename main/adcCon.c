
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

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated)
    {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK)
        {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
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
#endif

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


adc_unit_t unit1,unit2,unit3,unit4,unit5;
adc_channel_t pwm1_chan,pwm2_chan,pwm3_chan,pwm4_chan,pwm5_chan;
void configAdc(void)
{

    adc_oneshot_io_to_channel(ADC1_GPIO, &unit1,&pwm1_chan);
    adc_oneshot_io_to_channel(ADC2_GPIO, &unit2,&pwm2_chan);
    adc_oneshot_io_to_channel(ADC3_GPIO, &unit3,&pwm3_chan);
    adc_oneshot_io_to_channel(ADC4_GPIO, &unit4,&pwm4_chan);
    adc_oneshot_io_to_channel(ADC5_GPIO, &unit5,&pwm5_chan);

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = unit1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, pwm1_chan, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, pwm2_chan, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, pwm3_chan, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, pwm4_chan, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, pwm5_chan, &config));
    
    ESP_LOGI(TAG, "%d\t%d\t%d\t%d\t%d",unit1,unit2,unit3,unit4,unit5);
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