#ifdef RGB_MATRIX_FRAMEBUFFER_EFFECTS
#   ifdef ENABLE_RGB_MATRIX_TEST
RGB_MATRIX_EFFECT(TEST)
#       ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#define RGB_TEST 20
#define MAX_BRIGHTNESS 255
#define ANIMATION_STEPS 32

bool TEST(effect_params_t* params) {
    static uint8_t tick = 0;
    const uint8_t tick_speed = scale8(255 - rgb_matrix_config.speed, 64);

    if (params->init) {
        rgb_matrix_set_color_all(MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
        memset(g_rgb_frame_buffer, 0, sizeof(g_rgb_frame_buffer));
    }

    // Process animation ticks
    if (++tick > tick_speed) {
        tick = 0;

        // Start new breathing cycles
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                // Only start new cycle if key isn't already animating
                if (g_rgb_frame_buffer[row][col] == 0 && rand() < RAND_MAX / RGB_TEST) {
                    g_rgb_frame_buffer[row][col] = 1; // Start animation at step 1
                }
            }
        }

        // Update existing animations
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                if (g_rgb_frame_buffer[row][col] > 0) {
                    g_rgb_frame_buffer[row][col] = (g_rgb_frame_buffer[row][col] % (2 * ANIMATION_STEPS)) + 1;
                }
            }
        }
    }

    // Render LEDs
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t led[LED_HITS_TO_REMEMBER];
            uint8_t led_count = rgb_matrix_map_row_column_to_led(row, col, led);

            if (led_count > 0) {
                if (!HAS_ANY_FLAGS(g_led_config.flags[led[0]], params->flags)) continue;

                uint8_t step = g_rgb_frame_buffer[row][col];
                uint8_t brightness;

                if (step == 0) {
                    brightness = MAX_BRIGHTNESS; // Not animating - full brightness
                } else {
                    if (step <= ANIMATION_STEPS) {
                        // Decreasing phase
                        brightness = MAX_BRIGHTNESS - ((step * MAX_BRIGHTNESS) / ANIMATION_STEPS);
                    } else {
                        // Increasing phase
                        step = 2 * ANIMATION_STEPS - step;
                        brightness = MAX_BRIGHTNESS - ((step * MAX_BRIGHTNESS) / ANIMATION_STEPS);
                    }
                }

                rgb_matrix_set_color(led[0], brightness, brightness, brightness);
            }
        }
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#       endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#   endif // ENABLE_RGB_MATRIX_TEST
#endif // RGB_MATRIX_FRAMEBUFFER_EFFECTS
