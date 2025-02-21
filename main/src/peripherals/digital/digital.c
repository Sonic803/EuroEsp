#include "driver/gpio.h"
#include "esp_log.h"

#include "defines.h"

static const char *TAG = "espvco digital";

void configDigital(void)
{
    gpio_config_t io_conf1 = {
        .pin_bit_mask = (1ULL << DIGI1_GPIO),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf1);
    gpio_config_t io_conf2 = {
        .pin_bit_mask = (1ULL << DIGI2_GPIO),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf2);
}