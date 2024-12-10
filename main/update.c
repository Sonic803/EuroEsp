#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"
#include "driver/dac_oneshot.h"
#include "main.h"
#include "esp_log.h"

extern dac_oneshot_handle_t chan0_handle;
extern dac_oneshot_handle_t chan1_handle;

static bool IRAM_ATTR on_timer_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan0_handle, vcoVal));
    ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan1_handle, lfoVal));
    // printf("hello");
    // ESP_DRAM_LOGI(TAG, "point: %d", point);

    return false;
}

void initUpdate(void)
{
    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0,
        .alarm_count = EXAMPLE_TIMER_ALARM_COUNT,
        .flags.auto_reload_on_alarm = true,
    };
    gptimer_event_callbacks_t cbs = {
        .on_alarm = on_timer_alarm_cb,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    ESP_ERROR_CHECK(gptimer_start(gptimer));
    ESP_LOGI(TAG, "finished initUpdate");
}
