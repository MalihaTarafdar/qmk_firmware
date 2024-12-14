#ifdef RGB_MATRIX_FRAMEBUFFER_EFFECTS
#   ifdef ENABLE_RGB_MATRIX_ELECTRONS
RGB_MATRIX_EFFECT(ELECTRONS)
#       ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

// TODO: change back to 500
#define E_EFFECT_INTERVAL 3000

#define SET_RGB(R, G, B)  {.r = (R), .g = (G), .b = (B)}

static const RGB E_RGB_OFF = SET_RGB(0, 0, 0);
static const RGB E_RGB_KEYS = SET_RGB(80, 190, 255);
static const RGB E_RGB_STRIP = SET_RGB(130, 255, 230);

static const uint8_t E_STRIP_START = 67;

// TODO: modify g_rgb_frame_buffer to include LED strip
// or store start_pos and length in NO_LED positions in g_rgb_frame_buffer
static const uint8_t MATRIX_COLS_LEDS_ONLY[6] = {15, 15, 14, 14, 9, 38};

static bool e_init = false;

/*
 * LED MAP (105 LEDs total)
 * 00  01  02  03  04  05  06  07  08  09  10  11  12  13  14
 * 15  16  17  18  19  20  21  22  23  24  25  26  27  28  29
 * 30  31  32  33  34  35  36  37  38  39  40  41      42  43
 * 44  45  46  47  48  49  50  51  52  53  54  55      56  57
 * 58  59  60              61              62  63  64  65  66
 * 67-104 in LED strip
 */

static uint32_t interval(void) {
    return E_EFFECT_INTERVAL / scale16by8(qadd8(rgb_matrix_config.speed, 16), 16);
}

static bool ELECTRONS(effect_params_t* params) {
    static uint32_t wait_timer = 0;
    if (wait_timer > g_rgb_timer) {
        return false;
    }

    if (!e_init && params->init) {
        // clear LEDs & reset frame buffer
        rgb_matrix_set_color_all(0, 0, 0);
        memset(g_rgb_frame_buffer, 0, sizeof(g_rgb_frame_buffer));

        // generate random traveling lines in frame buffer
        // frame buffer stores the remaining length of the traveling line
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            uint8_t length = random8_min_max(MATRIX_COLS_LEDS_ONLY[r] / 2, MATRIX_COLS_LEDS_ONLY[r] / 4 * 3);
            uint8_t start_pos = random8_max(MATRIX_COLS_LEDS_ONLY[r]);

            // get accurate start_pos (skipping over NO_LEDs)
            uint8_t acc_start_pos = 0;
            for (uint8_t c = 0; c < MATRIX_COLS; c++) {
                uint8_t led[LED_HITS_TO_REMEMBER];
                uint8_t led_count = rgb_matrix_map_row_column_to_led(r, c, led);

                if (led_count == 0) continue;

                start_pos--;
                if (start_pos == 0) {
                    acc_start_pos = c;
                    break;
                }
            }

            // fill g_rgb_frame_buffer
            for (uint8_t p = acc_start_pos; p < acc_start_pos + length; p++) {
#ifndef ENABLE_RGB_MATRIX_ELECTRONS_WRAP
                if (p >= MATRIX_COLS) {
                    break;
                }
#endif // ENABLE_RGB_MATRIX_ELECTRONS_WRAP

                uint8_t c = (p < MATRIX_COLS) ? p : p - MATRIX_COLS;

                // skip over NO_LEDs
                uint8_t led[LED_HITS_TO_REMEMBER];
                uint8_t led_count = rgb_matrix_map_row_column_to_led(r, c, led);
                if (led_count == 0) {
                    length++;
                    continue;
                }

                g_rgb_frame_buffer[r][c] = length - (p - acc_start_pos);
            }

            // find all NO_LEDs and set g_rgb_frame_buffer to 255
            for (uint8_t c = 0; c < MATRIX_COLS; c++) {
                uint8_t led[LED_HITS_TO_REMEMBER];
                uint8_t led_count = rgb_matrix_map_row_column_to_led(r, c, led);
                if (led_count == 0) g_rgb_frame_buffer[r][c] = 255;
            }
        }

        // DEBUG
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            for (uint8_t c = 0; c < MATRIX_COLS; c++) {
                dprintf("%u ", g_rgb_frame_buffer[r][c]);
            }
            dprint("\n");
        }

        e_init = true;
    }

    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    // light lines
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        for (uint8_t c = 0; c < MATRIX_COLS; c++) {
            uint8_t led[LED_HITS_TO_REMEMBER];
            uint8_t led_count = rgb_matrix_map_row_column_to_led(r, c, led);

            if (led_count > 0) {
                if (g_rgb_frame_buffer[r][c] > 0) {
                    rgb_matrix_set_color(led[0], E_RGB_KEYS.r, E_RGB_KEYS.g, E_RGB_KEYS.b);
                } else {
                    rgb_matrix_set_color(led[0], E_RGB_OFF.r, E_RGB_OFF.g, E_RGB_OFF.b);
                }
            }
        }
    }

    if (!rgb_matrix_check_finished_leds(led_max)) {
        // TODO: moves lines
        // TODO: reverse direction when speed == 0
        // TODO: skip NO_LEDs

        // set pulse timer
        wait_timer = g_rgb_timer + interval();
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#      endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#   endif // ENABLE_RGB_MATRIX_ELECTRONS
#endif // #ifdef RGB_MATRIX_FRAMEBUFFER_EFFECTS
