#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"
#include "driver/dac_oneshot.h"
#include "main.h"
#include "esp_log.h"

#include <inttypes.h>
#include <freertos/task.h>
#include <string.h>
#include <encoder.h>
#include <esp_idf_lib_helpers.h>

#define RE_A_GPIO 4
#define RE_B_GPIO 5
#define RE_BTN_GPIO 6

#define EV_QUEUE_LEN 30

static QueueHandle_t event_queue;

rotary_encoder_event_t e;

static rotary_encoder_t re;
int val = 0;

static const char *TAG = "encoder";

void configEncoder(void)
{

    // Create event queue for rotary encoders
    event_queue = xQueueCreate(EV_QUEUE_LEN, sizeof(rotary_encoder_event_t));

    // Setup rotary encoder library
    ESP_ERROR_CHECK(rotary_encoder_init(event_queue));

    // Add one encoder
    memset(&re, 0, sizeof(rotary_encoder_t));
    re.pin_a = RE_A_GPIO;
    re.pin_b = RE_B_GPIO;
    re.pin_btn = RE_BTN_GPIO;
    ESP_ERROR_CHECK(rotary_encoder_add(&re));

    ESP_LOGI(TAG, "Initial value: %d", val);
    // while (1)
    // {
    //     xQueueReceive(event_queue, &e, portMAX_DELAY);

    //     switch (e.type)
    //     {
    //     case RE_ET_BTN_PRESSED:
    //         ESP_LOGI(TAG, "Button pressed");
    //         break;
    //     case RE_ET_BTN_RELEASED:
    //         ESP_LOGI(TAG, "Button released");
    //         break;
    //     case RE_ET_BTN_CLICKED:
    //         ESP_LOGI(TAG, "Button clicked");
    //         rotary_encoder_enable_acceleration(&re, 100);
    //         ESP_LOGI(TAG, "Acceleration enabled");
    //         break;
    //     case RE_ET_BTN_LONG_PRESSED:
    //         ESP_LOGI(TAG, "Looooong pressed button");
    //         rotary_encoder_disable_acceleration(&re);
    //         ESP_LOGI(TAG, "Acceleration disabled");
    //         break;
    //     case RE_ET_CHANGED:
    //         val += e.diff;
    //         ESP_LOGI(TAG, "Value = %" PRIi32, val);
    //         break;
    //     default:
    //         break;
    //     }
    // }
    rotary_encoder_enable_acceleration(&re, 20);
    // rotary_encoder_disable_acceleration(&re);
    ESP_LOGI(TAG, "finished encoder");
}

void getEncoderValue(int *ret_val, bool *pressed)

{
    // *ret_val = 0;
    *pressed = false;
    int start_val = val;
    // TODO
    while (xQueueReceive(event_queue, &e, 0))
    {
        switch (e.type)
        {
        case RE_ET_BTN_PRESSED:
            ESP_LOGI(TAG, "Button pressed");
            break;
        case RE_ET_BTN_RELEASED:
            ESP_LOGI(TAG, "Button released");
            break;
        case RE_ET_BTN_CLICKED:
            ESP_LOGI(TAG, "Button clicked");
            *pressed = true;
            // rotary_encoder_enable_acceleration(&re, 100);
            // ESP_LOGI(TAG, "Acceleration enabled");
            break;
        case RE_ET_BTN_LONG_PRESSED:
            ESP_LOGI(TAG, "Looooong pressed button");
            // rotary_encoder_disable_acceleration(&re);
            // ESP_LOGI(TAG, "Acceleration disabled");
            break;
        case RE_ET_CHANGED:
            val += e.diff;
            ESP_LOGI(TAG, "Value = %d", val);
            break;
        default:
            break;
        }
    }
    *ret_val = -((int)val/2-(int)start_val/2);
}
