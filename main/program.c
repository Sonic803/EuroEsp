#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"
#include "driver/dac_oneshot.h"
#include "main.h"
#include "esp_log.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/ledc.h"
#include "esp_err.h"

#include <assert.h>
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_filter.h"

#include "defines.h"



extern int vcoVal;
extern int lfoVal;
extern int pwm1Val;
extern int pwm2Val;
extern bool updated;
extern adc_oneshot_unit_handle_t adc_handle;
extern int pots_val[2];
extern int jack_val[3];
extern adc_channel_t pwm1_chan, pwm2_chan;


float lfo=0;


enum {
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE
} state;

void IRAM_ATTR adsr(){
    static float current=0;
    int gate=jack_val[0];
    bool trig = gate>100;
    if (trig){
        if (state == RELEASE){
            state = ATTACK;
        }
    }else{
        state=RELEASE;
    }
    switch (state){
        case ATTACK:
            current+=0.1;
            if (current>220){
                state=DECAY;
            }
            break;
        case DECAY:
            current-=0.1;
            if (current<200){
                state=SUSTAIN;
            }
            break;
        case SUSTAIN:
            current=200;
            break;
        case RELEASE:
            current-=0.02;
            if (current<0){
                current=0;
            }
            break;
    }
    vcoVal = (int)current;

}


void IRAM_ATTR calc(){
    lfo = (lfo + 0.2);
    //do mod 255
    lfo = fmod(lfo, 255);
    // vcoVal = (vcoVal + 1) % 255;
    adsr();
    lfoVal = ((int)lfo);
    pwm1Val = ((int)lfo);
    return;
}