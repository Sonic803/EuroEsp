#include "utils/libs.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_filter.h"

#include "esp_lcd_panel_vendor.h"
// #include "main.h"

#include "esp_timer.h"

#include "defines.h"
#include "math.h"

using namespace std;

class vcoScreen : public screen
{
public:
    int shape = 0;
    int freq = 0;
    float frequency;
    int sampling = 0;
    long phase = 0;
    lv_obj_t *freq_label;
    char freq_label_text[32];

    vcoScreen();
    void IRAM_ATTR update() override;
    void IRAM_ATTR refresh() override;
};
