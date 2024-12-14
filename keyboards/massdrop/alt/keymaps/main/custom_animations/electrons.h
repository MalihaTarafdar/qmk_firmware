#ifdef ENABLE_RGB_MATRIX_ELECTRONS
RGB_MATRIX_EFFECT(ELECTRONS)
#   ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

// TODO: change back to 500
#define E_EFFECT_INTERVAL 3000

#define SET_RGB(R, G, B)  {.r = (R), .g = (G), .b = (B)}

static const RGB e_rgb_off = SET_RGB(0, 0, 0);
static const RGB e_rgb_keys = SET_RGB(80, 190, 255);
static const RGB e_rgb_strip = SET_RGB(130, 255, 230);

static const uint8_t e_strip_start = 67;

static bool e_init = false;

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
    return E_EFFECT_INTERVAL / scale16by8(qadd8(rgb_matrix_config.speed, 16), 16);
}

static bool ELECTRONS(effect_params_t* params) {
    // MATRIX_ROWS + 1 to include LED strip
    const uint8_t rows = 6;
    const uint8_t row_len[6] = {15, 15, 14, 14, 9, 38};

    struct line {
        uint8_t start_pos;
        uint8_t length;
    };

    struct line lines[6];

    static uint32_t wait_timer = 0;
    if (wait_timer > g_rgb_timer) {
        return false;
    }

    if (!e_init && params->init) {
        // clear LEDs
        rgb_matrix_set_color_all(0, 0, 0);

        dprint("INIT\n"); // REMOVE

        // generate random traveling lines
        for (uint8_t i = 0; i < rows; i++) {
            // random start pos & length
            lines[i].start_pos = random8_max(row_len[i]);
            lines[i].length = random8_min_max(row_len[i] / 2, row_len[i]);
            dprintf("row_len[i]: %u\n", lines[i].length); // REMOVE debug
        }

        e_init = true;
    } else {
        // REMOVE debug
        for (uint8_t i = 0; i < rows; i++) {
            dprintf("line %u, start_pos: %u, length: %u\n", i, lines[i].start_pos, lines[i].length);
        }
    }

    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    // light lines
//     for (uint8_t led = led_min; led < led_max; led++) {
//         if (!HAS_ANY_FLAGS(g_led_config.flags[led], params->flags)) continue;

//         bool in_line = false;
//         uint8_t x = led;

//         // check if LED is in a line
//         for (uint8_t i = 0; i < rows; i++) {
//             if (x - row_len[i] < 0) {
//                 if (x >= lines[i].start_pos && x < (lines[i].start_pos + lines[i].length)) { // no wrap
//                     in_line = true;
//                 } else {
// #       ifdef ENABLE_RGB_MATRIX_ELECTRONS_WRAP

// #       endif // ENABLE_RGB_MATRIX_ELECTRONS_WRAP
//                 }
//                 break;
//             }
//             x -= row_len[i];
//         }

//         if (in_line) {
//             if (led < e_strip_start) {
//                 rgb_matrix_set_color(led, e_rgb_keys.r, e_rgb_keys.g, e_rgb_keys.b);
//             } else {
//                 rgb_matrix_set_color(led, e_rgb_strip.r, e_rgb_strip.g, e_rgb_strip.b);
//             }
//         } else {
//             rgb_matrix_set_color(led, e_rgb_off.r, e_rgb_off.g, e_rgb_off.b);
//         }
//     }

    if (!rgb_matrix_check_finished_leds(led_max)) {
        // moves lines
        // for (uint8_t i = 0; i < rows; i++) {
        //     // lines move backward if speed == 0
        //     if (rgb_matrix_config.speed != 0) {
        //         lines[i].start_pos += 1;
        //         // if (lines[i].start_pos == )
        //     } else {
        //         lines[i].start_pos -= 1;
        //     }
        // }

        // set pulse timer
        wait_timer = g_rgb_timer + interval();
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#   endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif // ENABLE_RGB_MATRIX_ELECTRONS
