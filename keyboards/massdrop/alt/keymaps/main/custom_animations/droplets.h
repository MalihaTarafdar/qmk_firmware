#if defined RGB_MATRIX_FRAMEBUFFER_EFFECTS && defined RGB_MATRIX_KEYREACTIVE_ENABLED
#   ifdef ENABLE_RGB_MATRIX_DROPLETS

RGB_MATRIX_EFFECT(DROPLETS)

#       ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#include "host.h" // to check caps lock state

#define CAPS_LOCK_KEY 30 // hardcoded because there is no way to get the keycode

typedef RGB (*d_reactive_f)(RGB rgb, uint16_t offset);

static const RGB D_RGB_KEYS = SET_RGB(RGB_KEYS_R, RGB_KEYS_G, RGB_KEYS_B);
static const RGB D_RGB_STRIP = SET_RGB(RGB_STRIP_R, RGB_STRIP_G, RGB_STRIP_B);
static const RGB D_RGB_PRESS = SET_RGB(RGB_PRESS_R, RGB_PRESS_G, RGB_PRESS_B);

#define DROPLET_CHANCE_CONSTANT 8 // (DROPLET_CHANCE_CONSTANT / 256) chance of a droplet starting on a key
#define ANIMATION_STEPS 128 // cannot be larger than 128

static void start_new_droplets(void) {
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            if (g_rgb_frame_buffer[row][col] == 0 && random8() < 256 / DROPLET_CHANCE_CONSTANT) {
                g_rgb_frame_buffer[row][col] = 1;
            }
        }
    }
}

static void update_droplets(void) {
	for (uint8_t col = 0; col < MATRIX_COLS; col++) {
		for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
			if (g_rgb_frame_buffer[row][col] > 0) {
				g_rgb_frame_buffer[row][col]++;
			}
			if (g_rgb_frame_buffer[row][col] >= 2 * ANIMATION_STEPS) {
				g_rgb_frame_buffer[row][col] = 0;
			}
		}
	}
}

bool DROPLETS(effect_params_t* params) {
    static uint8_t tick = 0;
    // a single tick occurs every (255 - rgb_matrix_config.speed) / (16 / 256) frames
    const uint8_t tick_speed = scale8(255 - rgb_matrix_config.speed, 16);

    if (params->init) {
        rgb_matrix_set_color_all(0, 0, 0);
        // randomly set g_rgb_frame_buffer to increase variation
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                // upperbound is exclusive, but need to use 16-bit function to avoid overflow on argument
                g_rgb_frame_buffer[row][col] = (uint8_t) random16_max(ANIMATION_STEPS * 2);
            }
        }
    }

    // process animation ticks
    if (++tick > tick_speed) {
        tick = 0;

        update_droplets();
        start_new_droplets();
    }

    // render keys
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t i = g_led_config.matrix_co[row][col];

            if (i != NO_LED) {
                if (!HAS_ANY_FLAGS(g_led_config.flags[i], params->flags)) continue;

                uint8_t step = g_rgb_frame_buffer[row][col];
                uint8_t value;

                if (step == 0) { // not animating
                    value = 255;
                } else {
                    if (step <= ANIMATION_STEPS) { // decreasing phase
                        value = 255 - (step * (256 / ANIMATION_STEPS)) + 1;
                    } else { // increasing phase
                        value = 255 - ((2 * ANIMATION_STEPS - step) * (256 / ANIMATION_STEPS)) + 1;
                    }
                }

                // keypress effects
                uint16_t keypress_max_tick = 65535 / qadd8(rgb_matrix_config.speed, 1);
                uint16_t keypress_tick = keypress_max_tick;

                // reverse search to find most recent key hit
                for (int8_t j = g_last_hit_tracker.count - 1; j >= 0; j--) {
                    if (g_last_hit_tracker.index[j] == i && g_last_hit_tracker.tick[j] < keypress_tick) {
                        keypress_tick = g_last_hit_tracker.tick[j];
                        break;
                    }
                }

                // scale keypress offset
                uint16_t offset = scale16by8(keypress_tick, qadd8(rgb_matrix_config.speed, 1));
                uint16_t scaled_offset = offset;
#if CUSTOM_RGB_MATRIX_KEYPRESS_SCALING == 2 // quadratic scaling
                scaled_offset = (offset * offset) / 255;
#elif CUSTOM_RGB_MATRIX_KEYPRESS_SCALING == 3 // cubic scaling
                scaled_offset = (offset * offset * offset) / (255 * 255);
#elif CUSTOM_RGB_MATRIX_KEYPRESS_SCALING == 4 // quartic scaling
                scaled_offset = (offset * offset * offset * offset) / (255 * 255 * 255);
#endif

                // invert offset & scale
                uint8_t r = qsub8(D_RGB_KEYS.r, scale8(255 - scaled_offset, D_RGB_KEYS.r - D_RGB_PRESS.r));
                uint8_t g = qsub8(D_RGB_KEYS.g, scale8(255 - scaled_offset, D_RGB_KEYS.g - D_RGB_PRESS.g));
                uint8_t b = D_RGB_KEYS.b;

                RGB rgb = SET_RGB(r, g, b);

                // make color complementary if caps lock is on
                if (i == CAPS_LOCK_KEY && host_keyboard_led_state().caps_lock) {
                    rgb.r = 255 - rgb.r;
                    rgb.g = 255 - rgb.g;
                    rgb.b = 255 - rgb.b;
                }

                // show animation
                HSV hsv = rgb_to_hsv(rgb);
                hsv.v = value;
                if (rgb_matrix_config.hsv.v < 255) hsv.v = scale8(hsv.v, rgb_matrix_config.hsv.v);

                RGB rgb_f = hsv_to_rgb(hsv);
                rgb_matrix_set_color(i, rgb_f.r, rgb_f.g, rgb_f.b);
            }
        }
    }

    // render strip
    for (uint8_t i = STRIP_START; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        rgb_matrix_set_color(i, D_RGB_STRIP.r, D_RGB_STRIP.g, D_RGB_STRIP.b);
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#       endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#   endif // ENABLE_RGB_MATRIX_DROPLETS
#endif // RGB_MATRIX_FRAMEBUFFER_EFFECTS && defined RGB_MATRIX_KEYREACTIVE_ENABLED
