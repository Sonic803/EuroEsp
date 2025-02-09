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

// #include "esp_system.h"

static const char *TAG = "espvco";

void app_main(void)
{
    vTaskDelay(pdMS_TO_TICKS(2000));

    configDac();
    configAdcContinous();
    configDisplay();
    configPwm();
    configEncoder();
    configDigital();
    configLvgl();

    startGraphic();

    configUpdate();

    ESP_LOGI("MEM", "Free heap size: %" PRIu32 " KB", esp_get_free_heap_size() / 1024);
    ESP_LOGI("MEM", "Free DRAM: %zu KB", heap_caps_get_free_size(MALLOC_CAP_8BIT) / 1024);
    ESP_LOGI("MEM", "Free IRAM: %zu KB", heap_caps_get_free_size(MALLOC_CAP_EXEC) / 1024);
    ESP_LOGI("MEM", "Free PSRAM: %zu KB", heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024);

    while (1)
    {
        updateEncoder();
        runGraphic();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
