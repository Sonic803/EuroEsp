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

class adsrScreen : public screen
{
public:
    enum
    {
        ATTACK,
        DECAY,
        SUSTAIN,
        RELEASE
    } state;
    int voltages[3] = {200, 170, 0};
    int times[3] = {100, 100, 100};
    float current = 0;

    adsrScreen();
    void IRAM_ATTR update();
};
