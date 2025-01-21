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

static const char *TAG = "espvco";

void app_main(void)
{
    vTaskDelay(pdMS_TO_TICKS(1000));

    configDac();
    configAdcContinous();
    configDisplay();
    configPwm();
    configEncoder();
    configDigital();

    startGraphic();
    configUpdate();

    while (1)
    {
        runGraphic();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
