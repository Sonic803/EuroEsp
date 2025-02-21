#pragma once

#include "utils/libs.h"
#include "defines.h"

using namespace std;

#define PITCH_TABLE_SIZE 256


class vcoScreen : public screen
{
public:
    enableAdc enableadc = {true, true, true, false, false};
    enableOut enableout = {true, false, {false, false}, {false, false}};
    int shape = 0;
    int freq = 0;
    int time;
    int val=0;
    float frequency;
    int sampling = 0;
    long phase = 0;
    float * pitchTable;
    lv_obj_t *freq_label;
    char freq_label_text[32];
    led *led_obj;

    vcoScreen();
    void IRAM_ATTR update() override;
    void IRAM_ATTR refresh() override;
};
