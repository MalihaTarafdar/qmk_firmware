#ifdef RGB_MATRIX_FRAMEBUFFER_EFFECTS
#   ifdef ENABLE_RGB_MATRIX_DIGITAL_RAIN_V2
RGB_MATRIX_EFFECT(DIGITAL_RAIN_V2)
#       ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

// lower the number for denser effect/wider keyboard
#define RGB_DIGITAL_RAIN_DROPS 20

static const uint8_t DRV2_STRIP_START = 67;

// TODO: caps lock change color
bool DIGITAL_RAIN_V2(effect_params_t* params) {
    // algorithm ported from https://github.com/tremby/Kaleidoscope-LEDEffect-DigitalRain
    const uint8_t drop_ticks           = scale8(255 - rgb_matrix_config.speed, 128);
    const uint8_t pure_blue_intensity = (((uint16_t)rgb_matrix_config.hsv.v) * 3) >> 2;
    const uint8_t max_brightness_boost = (((uint16_t)rgb_matrix_config.hsv.v) * 3) >> 2;
    const uint8_t max_intensity        = rgb_matrix_config.hsv.v;
    const uint8_t decay_ticks          = 0xff / scale8(max_intensity, 64);

    static uint8_t drop  = 0;
    static uint8_t decay = 0;

    if (params->init) {
        rgb_matrix_set_color_all(0, 0, 0);
        memset(g_rgb_frame_buffer, 0, sizeof(g_rgb_frame_buffer));
        drop = 0;
    }

    decay++;
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            if (row == 0 && drop == 0 && rand() < RAND_MAX / RGB_DIGITAL_RAIN_DROPS) {
                // top row, pixels have just fallen and we're
                // making a new rain drop in this column
                g_rgb_frame_buffer[row][col] = max_intensity;
            } else if (g_rgb_frame_buffer[row][col] > 0 && g_rgb_frame_buffer[row][col] < max_intensity) {
                // neither fully bright nor dark, decay it
                if (decay == decay_ticks) {
                    g_rgb_frame_buffer[row][col]--;
                }
            }
            // set the pixel colour
            uint8_t led[LED_HITS_TO_REMEMBER];
            uint8_t led_count = rgb_matrix_map_row_column_to_led(row, col, led);

            // TODO: multiple leds are supported mapped to the same row/column
            if (led_count > 0) {
                if (!HAS_ANY_FLAGS(g_led_config.flags[led[0]], params->flags)) continue;
                if (g_rgb_frame_buffer[row][col] > pure_blue_intensity) {
                    const uint8_t boost = (uint8_t)((uint16_t)max_brightness_boost * (g_rgb_frame_buffer[row][col] - pure_blue_intensity) / (max_intensity - pure_blue_intensity));
                    rgb_matrix_set_color(led[0], scale8(boost, 128), boost, max_intensity);
                } else {
                    const uint8_t blue = (uint8_t)((uint16_t)max_intensity * g_rgb_frame_buffer[row][col] / pure_blue_intensity);
                    rgb_matrix_set_color(led[0], 0, scale8(blue, 32), blue);
                }
            }
        }
    }

    // light strip
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        const uint8_t blue = (uint8_t)((uint16_t)max_intensity);
        if (i > DRV2_STRIP_START) rgb_matrix_set_color(i, scale8(blue, 8), scale8(blue, 64), blue);
    }

    if (decay == decay_ticks) {
        decay = 0;
    }

    if (++drop > drop_ticks) {
        // reset drop timer
        drop = 0;
        for (uint8_t row = MATRIX_ROWS - 1; row > 0; row--) {
            for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                // if ths is on the bottom row and bright allow decay
                if (row == MATRIX_ROWS - 1 && g_rgb_frame_buffer[row][col] == max_intensity) {
                    g_rgb_frame_buffer[row][col]--;
                }
                // check if the pixel above is bright
                if (g_rgb_frame_buffer[row - 1][col] >= max_intensity) { // Note: can be larger than max_intensity if val was recently decreased
                    // allow old bright pixel to decay
                    g_rgb_frame_buffer[row - 1][col] = max_intensity - 1;
                    // make this pixel bright
                    g_rgb_frame_buffer[row][col] = max_intensity;
                }
            }
        }
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#       endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#   endif // ENABLE_RGB_MATRIX_DIGITAL_RAIN_V2
#endif // RGB_MATRIX_FRAMEBUFFER_EFFECTS
