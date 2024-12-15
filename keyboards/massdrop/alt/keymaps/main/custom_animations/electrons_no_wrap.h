// NOTE: no wrap = after a line has finished, randomize its length and start again from the starting edge
// TODO: reverse direction when speed == 0
#ifdef RGB_MATRIX_FRAMEBUFFER_EFFECTS
#   ifdef ENABLE_RGB_MATRIX_ELECTRONS_NO_WRAP
RGB_MATRIX_EFFECT(ELECTRONS_NO_WRAP)
#       ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#define E_EFFECT_INTERVAL 500

static const RGB ENW_RGB_OFF = SET_RGB(0, 0, 0);
static const RGB ENW_RGB_KEYS = SET_RGB(RGB_KEYS_R, RGB_KEYS_G, RGB_KEYS_B);
static const RGB ENW_RGB_STRIP = SET_RGB(RGB_STRIP_R, RGB_STRIP_G, RGB_STRIP_B);

static uint8_t enw_strip_start_pos = 0;
static uint8_t enw_strip_length = 0;

static const uint8_t ENW_MATRIX_COLS_LEDS_ONLY[6] = {15, 15, 14, 14, 9, 38};
static const uint8_t ENW_SR = 6;

static bool enw_init = false;

/*
 * LED MAP (105 LEDs total)
 * 00  01  02  03  04  05  06  07  08  09  10  11  12  13  14
 * 15  16  17  18  19  20  21  22  23  24  25  26  27  28  29
 * 30  31  32  33  34  35  36  37  38  39  40  41      42  43
 * 44  45  46  47  48  49  50  51  52  53  54  55      56  57
 * 58  59  60              61              62  63  64  65  66
 * 67-104 in LED strip
 */

static uint32_t enw_interval(void) {
    return E_EFFECT_INTERVAL / scale16by8(qadd8(rgb_matrix_config.speed, 16), 64);
}

static int enw_rand_start_pos(uint8_t r) {
    return random8_max(ENW_MATRIX_COLS_LEDS_ONLY[r]);
}

static int enw_rand_length(uint8_t r) {
    return random8_min_max(scale8(ENW_MATRIX_COLS_LEDS_ONLY[r], 128), ENW_MATRIX_COLS_LEDS_ONLY[r] - 1);
}

static void ELECTRONS_NO_WRAP_init(void) {
    // generate random traveling lines in frame buffer
    // frame buffer stores the remaining length of the traveling line
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        uint8_t start_pos = enw_rand_start_pos(r);
        uint8_t length = enw_rand_length(r);

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
            if (p >= MATRIX_COLS) break;

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

    // generate random traveling line for LED strip
    enw_strip_start_pos = enw_rand_start_pos(ENW_SR - 1);
    enw_strip_length = enw_rand_length(ENW_SR - 1);

    // DEBUG
    dprintf("enw_strip_start_pos: %u\n", enw_strip_start_pos);
    dprintf("enw_strip_length: %u\n", enw_strip_length);
}

static bool ELECTRONS_NO_WRAP(effect_params_t* params) {
    static uint32_t wait_timer = 0;
    if (wait_timer > g_rgb_timer) {
        return false;
    }

    if (!enw_init && params->init) {
        // clear LEDs & frame buffer
        rgb_matrix_set_color_all(0, 0, 0);
        memset(g_rgb_frame_buffer, 0, sizeof(g_rgb_frame_buffer));

        ELECTRONS_NO_WRAP_init();

        // DEBUG
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            for (uint8_t c = 0; c < MATRIX_COLS; c++) {
                dprintf("%u ", g_rgb_frame_buffer[r][c]);
            }
            dprint("\n");
        }

        enw_init = true;
    }

    // light lines
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        for (uint8_t c = 0; c < MATRIX_COLS; c++) {
            uint8_t led[LED_HITS_TO_REMEMBER];
            uint8_t led_count = rgb_matrix_map_row_column_to_led(r, c, led);

            if (led_count > 0) {
                if (!HAS_ANY_FLAGS(g_led_config.flags[led[0]], params->flags)) continue;
                if (g_rgb_frame_buffer[r][c] > 0) {
                    RGB rgb = ENW_RGB_KEYS;
                    // match with config hsv.v
                    if (rgb_matrix_config.hsv.v < 255) {
                        HSV hsv = rgb_to_hsv(rgb);
                        hsv.v = rgb_matrix_config.hsv.v;
                        rgb = hsv_to_rgb(hsv);
                    }
                    rgb_matrix_set_color(led[0], rgb.r, rgb.g, rgb.b);
                } else {
                    rgb_matrix_set_color(led[0], ENW_RGB_OFF.r, ENW_RGB_OFF.g, ENW_RGB_OFF.b);
                }
            }
        }
    }

    // light LED strip line
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    for (uint8_t i = STRIP_START; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();

        uint8_t x = i - STRIP_START;
        bool in_line = (x >= enw_strip_start_pos && x < (enw_strip_start_pos + enw_strip_length)) ||
                (((enw_strip_start_pos + enw_strip_length) >= ENW_MATRIX_COLS_LEDS_ONLY[ENW_SR - 1]) &&
                        (x < (enw_strip_start_pos + enw_strip_length - ENW_MATRIX_COLS_LEDS_ONLY[ENW_SR - 1])));

        if (in_line) {
            RGB rgb = ENW_RGB_STRIP;
            // match with config hsv.v
            if (rgb_matrix_config.hsv.v < 255) {
                HSV hsv = rgb_to_hsv(rgb);
                hsv.v = rgb_matrix_config.hsv.v;
                rgb = hsv_to_rgb(hsv);
            }
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        } else {
            rgb_matrix_set_color(i, ENW_RGB_OFF.r, ENW_RGB_OFF.g, ENW_RGB_OFF.b);
        }
    }

    if (!rgb_matrix_check_finished_leds(led_max)) {
        // moves lines forward
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            bool has_line = false;

            // move lines no wrap
            for (uint8_t c = 0; c < MATRIX_COLS; c++) {
                if (c != (MATRIX_COLS - 1) && g_rgb_frame_buffer[r][c] == 0) {
                    uint8_t prev = c + 1;
                    while (prev < MATRIX_COLS && g_rgb_frame_buffer[r][prev] == 255) prev++;
                    if (g_rgb_frame_buffer[r][prev] > 0 && g_rgb_frame_buffer[r][prev] != 255) {
                        g_rgb_frame_buffer[r][c] = g_rgb_frame_buffer[r][prev];
                    }
                } else if (g_rgb_frame_buffer[r][c] != 255 && g_rgb_frame_buffer[r][c] > 0) {
                    g_rgb_frame_buffer[r][c]--;
                    has_line = true;
                }
            }

            // find last LED
            uint8_t last = 0;
            for (uint8_t c = 0; c < MATRIX_COLS; c++) {
                if (g_rgb_frame_buffer[r][c] != 255) last = c;
            }

            // start new line
            if (!has_line) {
                g_rgb_frame_buffer[r][last] = enw_rand_length(r);
            }
        }

        // move LED strip line forward
        enw_strip_start_pos++;
        if (enw_strip_start_pos == (led_max - STRIP_START)) {
            enw_strip_start_pos = 0;
            enw_strip_length = enw_rand_length(ENW_SR - 1);
        }

        // set pulse timer
        wait_timer = g_rgb_timer + enw_interval();
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#      endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#   endif // ENABLE_RGB_MATRIX_ELECTRONS_NO_WRAP
#endif // #ifdef RGB_MATRIX_FRAMEBUFFER_EFFECTS
