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

#include <string>
#include <algorithm>
#include <vector>

#include "esp_timer.h"

#include "defines.h"
#include "math.h"

#include "screens/screen.h"
#include "screens/adsr.h"
#include "screens/vco.h"

using namespace std;

static const char *TAG = "graphic";

// TODO
//  vedere https://docs.lvgl.io/master/details/widgets/scale.html

void (*updateFunction)() = NULL;

extern "C" void IRAM_ATTR update()
{
    screens[current_screen]->update();
}

extern "C" void startGraphic()
{
    ESP_LOGI(TAG, "Starting graphic");

    screens.push_back(std::make_unique<adsrScreen>());
    screens.push_back(std::make_unique<vcoScreen>());

    screens[0]->select();

    // a = new adsrScreen();
    updateFunction = update;
}

extern "C" void runGraphic()
{
}
