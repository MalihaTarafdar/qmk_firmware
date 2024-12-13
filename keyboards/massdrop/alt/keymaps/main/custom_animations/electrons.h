#ifdef ENABLE_RGB_MATRIX_ELECTRONS
RGB_MATRIX_EFFECT(ELECTRONS)
#   ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#define EFFECT_INTERVAL 3000

#define SET_RGB(R, G, B)  {.r = (R), .g = (G), .b = (B)}

const RGB e_rgb_off = SET_RGB(0, 0, 0);
const RGB e_rgb_keys = SET_RGB(100, 235, 255);
const RGB e_rgb_strip = SET_RGB(130, 255, 230);

/*
 * LED MAP
 * 00  01  02  03  04  05  06  07  08  09  10  11  12  13  14
 * 15  16  17  18  19  20  21  22  23  24  25  26  27  28  29
 * 30  31  32  33  34  35  36  37  38  39  40  41      42  43
 * 44  45  46  47  48  49  50  51  52  53  54  55      56  57
 * 58  59  60          61          62  63          64  65  66
 * 67-104 in LED strip
 */

static uint32_t interval(void) {
    return EFFECT_INTERVAL / scale16by8(qadd8(rgb_matrix_config.speed, 16), 16);
}

static bool ELECTRONS(effect_params_t* params) {
    // jagged LED state array
    RGB *led_states[6];
    RGB row1[15];
    RGB row2[15];
    RGB row3[14];
    RGB row4[14];
    RGB row5[9];
    RGB row6[38];
    led_states[0] = row1;
    led_states[1] = row2;
    led_states[2] = row3;
    led_states[3] = row4;
    led_states[4] = row5;
    led_states[5] = row6;

    uint8_t row_len[6] = {15, 15, 14, 14, 9, 38};
    uint8_t rows = 6;

    static uint32_t wait_timer = 0;
    if (wait_timer > g_rgb_timer) {
        return false;
    }

    if (params->init) {
        rgb_matrix_set_color_all(0, 0, 0);

        // generate random traveling lines
        for (uint8_t i = 0; i < rows; i++) {
            // random start pos & length
            uint8_t start_pos = random8_max(row_len[i]);
            uint8_t len = random8_min_max(row_len[i] / 2, row_len[i]);

            for (uint8_t j = 0; j < row_len[i]; j++) {
                bool in_line = false;
                if (start_pos + len <= row_len[i]) { // no wrap
                    in_line = (j >= start_pos && j < (start_pos + len));
                } else { // wrap
#ifdef ENABLE_RGB_MATRIX_ELECTRONS_WRAP
                    in_line = (j >= start_pos || j < ((start_pos + len) % row_len[i]));
#endif // ENABLE_RGB_MATRIX_ELECTRONS_WRAP
                }
                led_states[i][j] = (in_line) ? e_rgb_keys : e_rgb_off;
            }
        }
    }

    // REMOVE debug
    for (uint8_t i = 0; i < rows; i++) {
        for (uint8_t j = 0; j < row_len[i]; j++) {
            bool led_on = (led_states[i][j].r != 0 && led_states[i][j].g != 0 && led_states[i][j].b != 0);
            dprintf("%u ", led_on);
        }
        dprint("\n");
    }

    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    // light LEDs based on state array
    // for (uint8_t led = led_min; led < led_max; led++) {
    //     if (!HAS_ANY_FLAGS(g_led_config.flags[led], params->flags)) continue;

    //     uint8_t i = 0;
    //     uint8_t j = 0;
    //     uint8_t x = led;

    //     if (x < row_len[0]) {
    //         j = x;
    //     } else {
    //         // iterate to find pos in LED state array
    //         for (uint8_t r = 1; r < rows; r++) {
    //             if (x - row_len[r] < 0) {
    //                 i = r;
    //                 j = x + row_len[r];
    //                 break;
    //             }
    //             x -= row_len[r];
    //         }
    //     }

    //     rgb_matrix_set_color(led, led_states[i][j].r, led_states[i][j].g, led_states[i][j].b);
    // }

    if (!rgb_matrix_check_finished_leds(led_max)) {
        // TODO: shift LED states forward

        // set pulse timer
        wait_timer = g_rgb_timer + interval();
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#   endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif // ENABLE_RGB_MATRIX_ELECTRONS
