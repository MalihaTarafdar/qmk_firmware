#ifdef RGB_MATRIX_FRAMEBUFFER_EFFECTS
#   ifdef ENABLE_RGB_MATRIX_DROPLETS

RGB_MATRIX_EFFECT(DROPLETS)

#       ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#define D_EFFECT_INTERVAL 4000

typedef RGB (*d_reactive_f)(RGB rgb, uint16_t offset);

static const RGB D_RGB_KEYS = SET_RGB(RGB_KEYS_R, RGB_KEYS_G, RGB_KEYS_B);
static const RGB D_RGB_STRIP = SET_RGB(RGB_STRIP_R, RGB_STRIP_G, RGB_STRIP_B);
static const RGB D_RGB_PRESS = SET_RGB(RGB_PRESS_R, RGB_PRESS_G, RGB_PRESS_B);

static uint32_t rgb_to_uint32(RGB rgb) {
    return (rgb.r << 16) | (rgb.g << 8) | rgb.b;
}

static RGB uint32_to_rgb(uint32_t x) {
    RGB rgb = {
        .r = (x >> 16) & 0xFF,
        .g = (x >> 8) & 0xFF,
        .b = x & 0xFF
    };
    return rgb;
}

bool DROPLETS(effect_params_t* params) {

    static uint32_t wait_timer = 0;
    if (wait_timer > g_rgb_timer) {
        return false;
    }

    inline uint32_t interval(void) {
        return D_EFFECT_INTERVAL / scale16by8(qadd8(rgb_matrix_config.speed, 16), 16);
    }

    if (params->init) {
        rgb_matrix_set_color_all(0, 0, 0);
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            for (uint8_t c = 0; c < MATRIX_COLS; c++) {
                g_rgb_frame_buffer[r][c] = rgb_to_uint32(D_RGB_KEYS);

                // REMOVE debug
                uint8_t led[LED_HITS_TO_REMEMBER];
                uint8_t led_count = rgb_matrix_map_row_column_to_led(r, c, led);

                if (led_count > 0) {
                    if (!HAS_ANY_FLAGS(g_led_config.flags[led[0]], params->flags)) continue;

                    RGB rgb = uint32_to_rgb(g_rgb_frame_buffer[r][c]);

                    if (rgb.r != 0 || rgb.g != 0 || D_RGB_KEYS.b != rgb.b) {
                        rgb_matrix_set_color(led[0], RGB_CUSTOM_RED_R, RGB_CUSTOM_RED_G, RGB_CUSTOM_RED_B);
                    } else {
                        rgb_matrix_set_color(led[0], rgb.r, rgb.g, rgb.b);
                    }
                }
            }
        }
    }

    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    if (!rgb_matrix_check_finished_leds(led_max)) {
        // select LED to start effect
        // uint8_t r;
        // uint8_t c;
        // bool valid_led = false;

        // while (!valid_led) {
        //     r = random8_max(MATRIX_ROWS);
        //     c = random8_max(MATRIX_COLS);
        //     uint8_t led_count = rgb_matrix_map_row_column_to_led(row, col, led);

        //     if (led_count > 0) {
        //         if (!HAS_ANY_FLAGS(g_led_config.flags[led[0]], params->flags)) continue;

        //         HSV current_hsv = rgb_to_hsv(uint32_to_rgb(g_rgb_frame_buffer[r][c]));
        //         if (current_hsv.v == rgb_to_hsv(D_RGB_KEYS).v) {

        //         }
        //     }
        // }

        // light LEDs based on framebuffer
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            for (uint8_t c = 0; c < MATRIX_COLS; c++) {
                uint8_t led[LED_HITS_TO_REMEMBER];
                uint8_t led_count = rgb_matrix_map_row_column_to_led(r, c, led);

                if (led_count > 0) {
                    if (!HAS_ANY_FLAGS(g_led_config.flags[led[0]], params->flags)) continue;
                    // RGB rgb = uint32_to_rgb(g_rgb_frame_buffer[r][c]);
                    // rgb_matrix_set_color(led[0], rgb.r, rgb.g, rgb.b);
                }
            }
        }

        // light strip
        for (uint8_t i = STRIP_START; i < led_max; i++) {
            RGB_MATRIX_TEST_LED_FLAGS();
            rgb_matrix_set_color(i, D_RGB_STRIP.r, D_RGB_STRIP.g, D_RGB_STRIP.b);
        }

        // set timer
        wait_timer = g_rgb_timer + interval();
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#       endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#   endif // ENABLE_RGB_MATRIX_DROPLETS
#endif // RGB_MATRIX_FRAMEBUFFER_EFFECTS
