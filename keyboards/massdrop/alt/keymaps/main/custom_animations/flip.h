#ifdef ENABLE_RGB_MATRIX_FLIP
RGB_MATRIX_EFFECT(FLIP)
#   ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#define F_EFFECT_INTERVAL 1000

static const RGB F_RGB_OFF = SET_RGB(0, 0, 0);
static const RGB F_RGB_KEYS = SET_RGB(RGB_KEYS_R, RGB_KEYS_G, RGB_KEYS_B);
static const RGB F_RGB_STRIP = SET_RGB(RGB_STRIP_R, RGB_STRIP_G, RGB_STRIP_B);

static bool flag = false;

static bool FLIP(effect_params_t* params) {
    static uint32_t wait_timer = 0;
    if (wait_timer > g_rgb_timer) {
        return false;
    }

    inline uint32_t interval(void) {
        return F_EFFECT_INTERVAL / scale16by8(qadd8(rgb_matrix_config.speed, 16), 16);
    }

    if (params->init) {
        // clear LEDs
        rgb_matrix_set_color_all(0, 0, 0);
    }

    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    // light LEDs based on state array
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();

        bool cond = (flag) ? (i % 2 == 0) : (i % 2 == 1);
        RGB rgb = (cond) ? ((i < STRIP_START) ? F_RGB_KEYS : F_RGB_STRIP) : F_RGB_OFF;

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
        flag = !flag;

        // set pulse timer
        wait_timer = g_rgb_timer + interval();
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#   endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif // ENABLE_RGB_MATRIX_FLIP
