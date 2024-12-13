#ifdef ENABLE_RGB_MATRIX_ELECTRIC_FLOW
RGB_MATRIX_EFFECT(ELECTRIC_FLOW)
#   ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#define EFFECT_INTERVAL 500

#define SET_RGB(R, G, B)  {.r = (R), .g = (G), .b = (B)}

const RGB ef_rgb_off = SET_RGB(0, 0, 0);
const RGB ef_rgb_keys = SET_RGB(100, 235, 255);
// const RGB ef_rgb_strip = SET_RGB(130, 255, 230);

static bool ELECTRIC_FLOW(effect_params_t* params) {
    // LED state array
    static RGB led_states[RGB_MATRIX_LED_COUNT];

    static uint32_t wait_timer = 0;
    if (wait_timer > g_rgb_timer) {
        return false;
    }

    inline uint32_t interval(void) {
        return EFFECT_INTERVAL / scale16by8(qadd8(rgb_matrix_config.speed, 16), 16);
    }

    if (params->init) {
        // clear LEDs and fill the state array
        rgb_matrix_set_color_all(0, 0, 0);
        for (uint8_t j = 0; j < RGB_MATRIX_LED_COUNT; ++j) {
            led_states[j] = (random8() & 2) ? ef_rgb_off : ef_rgb_keys;
        }
    }

    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    // light LEDs based on state array
    for (uint8_t i = led_min; i < led_max; ++i) {
        RGB_MATRIX_TEST_LED_FLAGS();
        rgb_matrix_set_color(i, led_states[i].r, led_states[i].g, led_states[i].b);
    }

    if (!rgb_matrix_check_finished_leds(led_max)) {
        // save first LED to wrap
        RGB tmp = SET_RGB(led_states[0].r, led_states[0].g, led_states[0].b);

        // shift LED states forward
        for (uint8_t j = 0; j < led_max - 1; ++j) {
            led_states[j] = led_states[j + 1];
        }

        // fill last LED
        led_states[led_max - 1] = tmp;

        // set pulse timer
        wait_timer = g_rgb_timer + interval();
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#   endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif // ENABLE_RGB_MATRIX_ELECTRIC_FLOW
