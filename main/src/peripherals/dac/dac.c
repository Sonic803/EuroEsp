#include "driver/dac_oneshot.h"
#include "esp_log.h"

DRAM_ATTR dac_oneshot_handle_t chan0_handle;
DRAM_ATTR dac_oneshot_handle_t chan1_handle;

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