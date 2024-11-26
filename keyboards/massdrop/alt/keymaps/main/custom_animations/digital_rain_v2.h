#ifdef ENABLE_RGB_MATRIX_DIGITAL_RAIN_V2
RGB_MATRIX_EFFECT(DIGITAL_RAIN_V2)
#   ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

bool DIGITAL_RAIN_V2(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    RGB rgb = {
        .r = 0,
        .g = 255,
        .b = 0
    };
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

#   endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif // ENABLE_RGB_MATRIX_DIGITAL_RAIN_V2
