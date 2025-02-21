#pragma once

#include "utils/libs.h"
#include "defines.h"

using namespace std;

class adsrScreen : public screen
{
public:
    enableAdc enableadc = {true, false, true, false, false};
    enableOut enableout = {true, false, {false, false}, {false, false}};
    enum
    {
        ATTACK,
        DECAY,
        SUSTAIN,
        RELEASE
    } state;
    int voltages[3] = {200, 170, 0};
    int times[3] = {100, 100, 100};
    float cur_val = 0;
    led *led_obj;

    adsrScreen();
    void IRAM_ATTR update();
    void IRAM_ATTR refresh();
};
