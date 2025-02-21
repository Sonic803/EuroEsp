/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include "esp_adc/adc_oneshot.h"

#define CONST_PERIOD_2_PI           6.2832                  // 2 * PI

#define EXAMPLE_ARRAY_LEN           400                     // Length of wave array
#define EXAMPLE_DAC_AMPLITUDE       255                     // Amplitude of DAC voltage. If it's more than 256 will causes dac_output_voltage() output 0.


#ifdef __cplusplus
extern "C" {
#endif



struct enableAdc
{
    bool pots[2];
    bool jacks[3];
};

struct enableOut
{
    bool vco;
    bool lfo;
    bool pwm[2];
    bool digi[2];
};

void configAdcEnabled(struct enableAdc enable);


void IRAM_ATTR read_adc(void);
void print_string(const char *str);
void getEncoderValue(int *ret_val, bool* pressed);




// void IRAM_ATTR calc(void);
/**
 * @brief Print the example log information
 *
 * @param conv_freq     DAC conversion frequency
 * @param wave_freq     The frequency of the wave
 */
void example_log_info(uint32_t conv_freq, uint32_t wave_freq);


extern int vcoVal;
extern int lfoVal;
extern int pwm1Val;
extern int pwm2Val;
extern bool updated;
extern adc_oneshot_unit_handle_t adc_handle;
extern int pots_val[2];
extern int jack_val[3];
extern adc_channel_t pwm1_chan, pwm2_chan;


typedef void (*FunctionPointer)();

#ifdef __cplusplus
}
#endif