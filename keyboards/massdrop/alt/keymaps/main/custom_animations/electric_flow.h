#ifdef ENABLE_RGB_MATRIX_ELECTRIC_FLOW
RGB_MATRIX_EFFECT(ELECTRIC_FLOW)
#   ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#define EF_EFFECT_INTERVAL 500

#define SET_RGB(R, G, B)  {.r = (R), .g = (G), .b = (B)}

const RGB ef_rgb_off = SET_RGB(0, 0, 0);
const RGB ef_rgb_keys = SET_RGB(100, 235, 255);
const RGB ef_rgb_strip = SET_RGB(130, 255, 230);

const uint8_t ef_strip_start = 67;

static bool ELECTRIC_FLOW(effect_params_t* params) {
    // LED state array
    static RGB led_states[RGB_MATRIX_LED_COUNT];

    static uint32_t wait_timer = 0;
    if (wait_timer > g_rgb_timer) {
        return false;
    }

    inline uint32_t interval(void) {
        return EF_EFFECT_INTERVAL / scale16by8(qadd8(rgb_matrix_config.speed, 16), 16);
    }

    if (params->init) {
        // clear LEDs
        rgb_matrix_set_color_all(0, 0, 0);

        // fill LED state array
        for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
            led_states[i] = (random8() & 2) ? ef_rgb_off : ((i < ef_strip_start) ? ef_rgb_keys : ef_rgb_strip);
        }
    }

    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    // light LEDs based on state array
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();

        RGB rgb = led_states[i];

        // match with config hsv.v
        bool led_on = rgb.r != 0 && rgb.g != 0 && rgb.b != 0;
        if (led_on && rgb_matrix_config.hsv.v < 255) {
            HSV hsv = rgb_to_hsv(rgb);
            hsv.v = rgb_matrix_config.hsv.v;
            rgb = hsv_to_rgb(hsv);
        }

        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }

    if (!rgb_matrix_check_finished_leds(led_max)) {
        // save first LED to wrap
        RGB tmp = SET_RGB(led_states[0].r, led_states[0].g, led_states[0].b);

        // shift LED states forward
        for (uint8_t i = 0; i < led_max - 1; i++) {
            led_states[i] = led_states[i + 1];
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
