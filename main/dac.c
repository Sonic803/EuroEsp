#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"
#include "driver/dac_oneshot.h"
#include "main.h"
#include "esp_log.h"


dac_oneshot_handle_t chan0_handle;
dac_oneshot_handle_t chan1_handle;

extern uint8_t sin_wav[EXAMPLE_ARRAY_LEN]; // Used to store sine wave values
extern uint8_t tri_wav[EXAMPLE_ARRAY_LEN]; // Used to store triangle wave values
extern uint8_t saw_wav[EXAMPLE_ARRAY_LEN]; // Used to store sawtooth wave values
extern uint8_t squ_wav[EXAMPLE_ARRAY_LEN]; // Used to store square wave values

extern float f;

static const char *TAG = "dac";

void configDac(void)
{
    dac_oneshot_config_t dac0_cfg = {
        .chan_id = DAC_CHAN_0,
    };
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&dac0_cfg, &chan0_handle));
    
    dac_oneshot_config_t dac1_cfg = {
        .chan_id = DAC_CHAN_1,
    };
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&dac1_cfg, &chan1_handle));

    ESP_LOGI(TAG, "finished configDac");
}