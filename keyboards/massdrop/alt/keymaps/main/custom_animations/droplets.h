// #ifdef RGB_MATRIX_FRAMEBUFFER_EFFECTS
#   ifdef ENABLE_RGB_MATRIX_DROPLETS

RGB_MATRIX_EFFECT(DROPLETS)

#       ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

typedef RGB (*d_reactive_f)(RGB rgb, uint16_t offset);

static const RGB D_RGB_KEYS = SET_RGB(RGB_KEYS_R, RGB_KEYS_G, RGB_KEYS_B);
static const RGB D_RGB_STRIP = SET_RGB(RGB_STRIP_R, RGB_STRIP_G, RGB_STRIP_B);
static const RGB D_RGB_PRESS = SET_RGB(RGB_PRESS_R, RGB_PRESS_G, RGB_PRESS_B);

static uint8_t custom_rgb_frame_buffer[MATRIX_ROWS][MATRIX_COLS] = {{0}};

#define DROPLET_CHANCE_CONSTANT 8 // (DROPLET_CHANCE_CONSTANT / 256) chance of a droplet starting on a key
#define ANIMATION_STEPS 128 // cannot be larger than 128

static void start_new_droplets(void) {
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            if (custom_rgb_frame_buffer[row][col] == 0 && random8() < 256 / DROPLET_CHANCE_CONSTANT) {
                custom_rgb_frame_buffer[row][col] = 1;
            }
        }
    }
}

static void update_droplets(void) {
	for (uint8_t col = 0; col < MATRIX_COLS; col++) {
		for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
			if (custom_rgb_frame_buffer[row][col] > 0) {
				custom_rgb_frame_buffer[row][col]++;
			}
			if (custom_rgb_frame_buffer[row][col] >= 2 * ANIMATION_STEPS) {
				custom_rgb_frame_buffer[row][col] = 0;
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
        // randomly set custom_rgb_frame_buffer to increase variation
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                // upperbound is exclusive, but need to use 16-bit function to avoid overflow on argument
                custom_rgb_frame_buffer[row][col] = (uint8_t) random16_max(ANIMATION_STEPS * 2);
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
            uint8_t led[LED_HITS_TO_REMEMBER];
            uint8_t led_count = rgb_matrix_map_row_column_to_led(row, col, led);

            if (led_count > 0) {
                if (!HAS_ANY_FLAGS(g_led_config.flags[led[0]], params->flags)) continue;

                uint8_t step = custom_rgb_frame_buffer[row][col];
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

                HSV hsv = rgb_to_hsv(D_RGB_KEYS);
                hsv.v = value;
                RGB rgb = hsv_to_rgb(hsv);
                rgb_matrix_set_color(led[0], rgb.r, rgb.g, rgb.b);
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
// #endif // RGB_MATRIX_FRAMEBUFFER_EFFECTS
