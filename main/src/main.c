#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "main.h"
#include "defines.h"

#include "peripherals/adc/adc.h"
#include "peripherals/dac/dac.h"
#include "peripherals/pwm/pwm.h"
#include "peripherals/pwm/pwm.h"
#include "peripherals/display/display.h"
#include "peripherals/encoder/encoder.h"
#include "peripherals/digital/digital.h"
#include "peripherals/lvgl/lvgl.h"
#include "update.h"
#include "graphic.h"

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
        runGraphic();
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}
