set(srcs
       "main.c"
       "dac.c"
       "adc.c"
       "pwm.c"
       )

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_driver_dac esp_driver_gpio esp_driver_gptimer  esp_adc esp_phy esp_driver_ledc 
                       )
