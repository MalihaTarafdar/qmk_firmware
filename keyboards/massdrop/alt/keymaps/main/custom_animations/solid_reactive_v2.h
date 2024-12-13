#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
#    ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_V2

RGB_MATRIX_EFFECT(SOLID_REACTIVE_V2)

#        ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#define SET_RGB(R, G, B)  {.r = (R), .g = (G), .b = (B)}

typedef RGB (*custom_reactive_f)(RGB rgb, uint16_t offset);

const RGB srv2_rgb_keys = SET_RGB(100, 235, 255);
const RGB srv2_rgb_strip = SET_RGB(130, 255, 230);
const RGB srv2_rgb6 = SET_RGB(70, 235, 255);
// const RGB srv2_rgb_press = SET_RGB(0, 50, 255);

const uint8_t srv2_strip_start = 67;

const uint8_t srv2_dr = 100;
const uint8_t srv2_dg = 185;

static bool custom_effect_runner_reactive(effect_params_t* params, custom_reactive_f effect_func, RGB base_colors[]) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    uint16_t max_tick = 65535 / qadd8(rgb_matrix_config.speed, 1);
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        uint16_t tick = max_tick;
        // reverse search to find most recent key hit
        for (int8_t j = g_last_hit_tracker.count - 1; j >= 0; j--) {
            if (g_last_hit_tracker.index[j] == i && g_last_hit_tracker.tick[j] < tick) {
                tick = g_last_hit_tracker.tick[j];
                break;
            }
        }

        uint16_t offset = scale16by8(tick, qadd8(rgb_matrix_config.speed, 1));
        RGB rgb = effect_func(base_colors[i], offset);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

static RGB SOLID_REACTIVE_V2_math(RGB rgb, uint16_t offset) {
    // key press = srv2_rgb_press
    // inverse offset, scale to srv2_dr & srv2_dg
    uint8_t r = qsub8(rgb.r, scale8(255 - offset, srv2_dr));
    uint8_t g = qsub8(rgb.g, scale8(255 - offset, srv2_dg));
    uint8_t b = rgb.b;

    RGB rgb_f = SET_RGB(r, g, b);

    // match with config hsv.v
    if (rgb_matrix_config.hsv.v < 255) {
        HSV hsv_f = rgb_to_hsv(rgb_f);
        hsv_f.v = rgb_matrix_config.hsv.v;
        rgb_f = hsv_to_rgb(hsv_f);
    }

    return rgb_f;
}

/*
 * LED MAP
 * 00  01  02  03  04  05  06  07  08  09  10  11  12  13  14
 * 15  16  17  18  19  20  21  22  23  24  25  26  27  28  29
 * 30  31  32  33  34  35  36  37  38  39  40  41      42  43
 * 44  45  46  47  48  49  50  51  52  53  54  55      56  57
 * 58  59  60          61          62  63          64  65  66
 * 67-104 in LED strip
 */

bool SOLID_REACTIVE_V2(effect_params_t* params) {
    // temporary fix since different LEDs have different tints even when assigned the same value
    RGB base_colors[RGB_MATRIX_LED_COUNT];

    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    for (uint8_t i = led_min; i < led_max; i++) {
        base_colors[i] = (i < srv2_strip_start) ? srv2_rgb_keys : srv2_rgb_strip;
    }

    // fix for key 6
    base_colors[6] = srv2_rgb6;

    // TODO: fix other keys

    return custom_effect_runner_reactive(params, &SOLID_REACTIVE_V2_math, base_colors);
}

#        endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#    endif     // ENABLE_RGB_MATRIX_SOLID_REACTIVE_V2
#endif         // RGB_MATRIX_KEYREACTIVE_ENABLED
