#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
#    ifdef ENABLE_RGB_MATRIX_MULTICROSS_V2

RGB_MATRIX_EFFECT(MULTICROSS_V2)

#        ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#define MV2_STRIP_START 67

#define SET_RGB(R, G, B)  {.r = (R), .g = (G), .b = (B)}

typedef RGB (*mv2_reactive_splash_f)(RGB hsv, int16_t dx, int16_t dy, uint8_t dist, uint16_t tick);

static const RGB MV2_RGB_KEYS = SET_RGB(80, 190, 255);
static const RGB MV2_RGB_STRIP = SET_RGB(130, 255, 230);
static const RGB MV2_RGB6 = SET_RGB(50, 190, 255);
static const RGB MV2_RGB_PRESS = SET_RGB(0, 40, 255);

bool mv2_effect_runner_reactive_splash(uint8_t start, effect_params_t* params, mv2_reactive_splash_f effect_func, RGB base_colors[]) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    uint8_t count = g_last_hit_tracker.count;
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        RGB rgb = base_colors[i];
        for (uint8_t j = start; j < count; j++) {
            int16_t  dx   = g_led_config.point[i].x - g_last_hit_tracker.x[j];
            int16_t  dy   = g_led_config.point[i].y - g_last_hit_tracker.y[j];
            uint8_t  dist = sqrt16(dx * dx + dy * dy);
            uint16_t tick = scale16by8(g_last_hit_tracker.tick[j], qadd8(rgb_matrix_config.speed, 1));
            rgb           = effect_func(rgb, dx, dy, dist, tick);
        }
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

static RGB MULTICROSS_V2_math(RGB rgb, int16_t dx, int16_t dy, uint8_t dist, uint16_t tick) {
    if (abs(dx) > 32 || abs(dy) > 32) return rgb;

    uint16_t effect = tick + dist;
    dx              = dx < 0 ? dx * -1 : dx;
    dy              = dy < 0 ? dy * -1 : dy;
    dx              = dx * 16 > 255 ? 255 : dx * 16;
    dy              = dy * 16 > 255 ? 255 : dy * 16;
    effect += dx > dy ? dy : dx;

    if (effect > 255) effect = 255;

    uint8_t r = qsub8(rgb.r, scale8(255 - effect, MV2_RGB_KEYS.r - MV2_RGB_PRESS.r));
    uint8_t g = qsub8(rgb.g, scale8(255 - effect, MV2_RGB_KEYS.g - MV2_RGB_PRESS.g));
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
 * LED MAP (105 LEDs total)
 * 00  01  02  03  04  05  06  07  08  09  10  11  12  13  14
 * 15  16  17  18  19  20  21  22  23  24  25  26  27  28  29
 * 30  31  32  33  34  35  36  37  38  39  40  41      42  43
 * 44  45  46  47  48  49  50  51  52  53  54  55      56  57
 * 58  59  60              61              62  63  64  65  66
 * 67-104 in LED strip
 */

bool MULTICROSS_V2(effect_params_t* params) {
    // temporary fix since different LEDs have different tints even when assigned the same value
    RGB base_colors[RGB_MATRIX_LED_COUNT];

    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    for (uint8_t i = led_min; i < led_max; i++) {
        base_colors[i] = (i < MV2_STRIP_START) ? MV2_RGB_KEYS : MV2_RGB_STRIP;
    }

    // fix for key 6
    base_colors[6] = MV2_RGB6;

    // TODO: fix other keys

    return mv2_effect_runner_reactive_splash(qsub8(g_last_hit_tracker.count, 1), params, &MULTICROSS_V2_math, base_colors);
}

#        endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#    endif     // ENABLE_RGB_MATRIX_MULTICROSS_V2
#endif         // RGB_MATRIX_KEYREACTIVE_ENABLED
