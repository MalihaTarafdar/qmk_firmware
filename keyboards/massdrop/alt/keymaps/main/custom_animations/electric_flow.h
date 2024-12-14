#ifdef ENABLE_RGB_MATRIX_ELECTRIC_FLOW
RGB_MATRIX_EFFECT(ELECTRIC_FLOW)
#   ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#define EF_EFFECT_INTERVAL 500
#define EF_STRIP_START 67

#define SET_RGB(R, G, B)  {.r = (R), .g = (G), .b = (B)}

static const RGB EF_RGB_OFF = SET_RGB(0, 0, 0);
static const RGB EF_RGB_KEYS = SET_RGB(80, 190, 255);
static const RGB EF_RGB_STRIP = SET_RGB(130, 255, 230);

static bool ELECTRIC_FLOW(effect_params_t* params) {
    // TODO: use g_rgb_frame_buffer instead
    // LED state array
    static bool led[RGB_MATRIX_LED_COUNT];

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
            led[i] = (random8() & 1);
        }
    }

    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    // light LEDs based on state array
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();

        RGB rgb = (led[i]) ? ((i < EF_STRIP_START) ? EF_RGB_KEYS : EF_RGB_STRIP) : EF_RGB_OFF;

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
        bool tmp = led[0];

        // shift LED states forward
        for (uint8_t i = 0; i < led_max - 1; i++) {
            led[i] = led[i + 1];
        }

        // fill last LED
        led[led_max - 1] = tmp;

        // set pulse timer
        wait_timer = g_rgb_timer + interval();
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#   endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif // ENABLE_RGB_MATRIX_ELECTRIC_FLOW
