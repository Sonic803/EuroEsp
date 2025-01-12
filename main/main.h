/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#define CONST_PERIOD_2_PI           6.2832                  // 2 * PI

#define EXAMPLE_ARRAY_LEN           400                     // Length of wave array
#define EXAMPLE_DAC_AMPLITUDE       255                     // Amplitude of DAC voltage. If it's more than 256 will causes dac_output_voltage() output 0.

typedef enum {
    DAC_SINE_WAVE,
    DAC_TRIANGLE_WAVE,
    DAC_SAWTOOTH_WAVE,
    DAC_SQUARE_WAVE,
    DAC_WAVE_MAX,
} dac_example_wave_type_t;

/**
 * @brief Use DMA to convert continuously
 *
 */
void example_dac_continuous_by_dma(void);

/**
 * @brief Use timer to convert continuously
 *
 */
void example_dac_continuous_by_timer(void);
void configDac(void);
void configAdc(void);
void configAdcContinous(void);
void configPwm(void);
void configUpdate(void);
void configDisplay(void);
void configEncoder(void);
void configDigital(void);


void IRAM_ATTR read_adc(void);
void print_string(const char *str);
void getEncoderValue(int *ret_val, bool* pressed);

void startGraphic();
void runGraphic();


void IRAM_ATTR calc(void);
/**
 * @brief Print the example log information
 *
 * @param conv_freq     DAC conversion frequency
 * @param wave_freq     The frequency of the wave
 */
void example_log_info(uint32_t conv_freq, uint32_t wave_freq);