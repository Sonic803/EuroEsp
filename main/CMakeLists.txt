file(GLOB screens_srcs "screens/*.c" "screens/*.cpp")

set(srcs
       "main.c"
       "init/dac.c"
       "init/pwm.c"
       "init/display.c"
       "init/adcCon.c"
       "init/digital.c"
       "init/encoder.c"
       "init/lvgl.c"
       "update.c"
       "graphic.cpp"
       "screens/adsr.cpp"
       ${screens_srcs}
       )

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_driver_dac esp_driver_gpio esp_driver_gptimer  esp_adc esp_phy esp_driver_ledc  esp_lcd encoder esp_idf_lib_helpers
                       )


# Add custom compile options
target_compile_options(${COMPONENT_LIB} PRIVATE
-mtext-section-literals
)

# # Apply -mtext-section-literals only for graphic.cpp
# set_source_files_properties("graphic.cpp" PROPERTIES COMPILE_OPTIONS "-mtext-section-literals")