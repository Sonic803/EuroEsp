file(GLOB screens_srcs "src/screens/*/*.c"   "src/screens/*/*.cpp")
file(GLOB widgets_srcs "src/widgets/*/*.c" "src/widgets/*/*.cpp")

set(srcs
       "src/main.c"
       "src/peripherals/dac/dac.c"
       "src/peripherals/pwm/pwm.c"
       "src/peripherals/display/display.c"
       "src/peripherals/adc/adc.c"
       "src/peripherals/digital/digital.c"
       "src/peripherals/encoder/encoder.c"
       "src/peripherals/lvgl/lvgl.c"
       "src/update.c"
       "src/graphic.cpp"
       "src/screens/screen.cpp"
       "src/screens/widgets.cpp"
       ${screens_srcs}
       ${widgets_srcs}
       )

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "src"
                       PRIV_REQUIRES esp_driver_dac esp_driver_gpio esp_driver_gptimer  esp_adc esp_phy esp_driver_ledc  esp_lcd encoder esp_idf_lib_helpers
                       )


# Add custom compile options
target_compile_options(${COMPONENT_LIB} PRIVATE
-mtext-section-literals
)

# # Apply -mtext-section-literals only for graphic.cpp
# set_source_files_properties("graphic.cpp" PROPERTIES COMPILE_OPTIONS "-mtext-section-literals")