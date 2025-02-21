#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"

#include "defines.h"

#include "screens/screen.h"
#include "screens/adsr/adsr.h"
#include "screens/vco/vco.h"
#include "screens/scope/scope.h"

#include "peripherals/encoder/encoder.h"


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

    screens.push_back(std::make_unique<vcoScreen>());
    screens.push_back(std::make_unique<scopeScreen>());
    screens.push_back(std::make_unique<adsrScreen>());

    screens[0]->select();

    lvgl_port_lock(0);
    lv_group_set_editing(screens[0]->group, false);
    lvgl_port_unlock();

    // a = new adsrScreen();
    updateFunction = update;
}

extern "C" void runGraphic()
{
    updateEncoder();
    screens[current_screen]->refresh();
}
