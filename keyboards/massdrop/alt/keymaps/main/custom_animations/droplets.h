// FIX

#ifdef RGB_MATRIX_KEYPRESSES
#   ifdef ENABLE_RGB_MATRIX_DROPLETS

RGB_MATRIX_EFFECT(DROPLETS)

#       ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#define D_EFFECT_INTERVAL 2000

typedef RGB (*d_reactive_f)(RGB rgb, uint16_t offset);

static const RGB D_RGB_KEYS = SET_RGB(RGB_KEYS_R, RGB_KEYS_G, RGB_KEYS_B);
static const RGB D_RGB_STRIP = SET_RGB(RGB_STRIP_R, RGB_STRIP_G, RGB_STRIP_B);
static const RGB D_RGB6 = SET_RGB(RGB_KEY6_R, RGB_KEY6_G, RGB_KEY6_B);
static const RGB D_RGB_PRESS = SET_RGB(RGB_PRESS_R, RGB_PRESS_G, RGB_PRESS_B);

static last_hit_t last_drop_tracker = {0};
static uint32_t timer_buffer;

static void d_effect_runner_reactive(effect_params_t* params, d_reactive_f effect_func) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    uint16_t max_tick = 65535 / qadd8(rgb_matrix_config.speed, 1);
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        uint16_t tick = max_tick;
        // reverse search to find most recent key hit
        for (int8_t j = last_drop_tracker.count - 1; j >= 0; j--) {
            if (last_drop_tracker.index[j] == i && last_drop_tracker.tick[j] < tick) {
                tick = last_drop_tracker.tick[j];
                break;
            }
        }

        uint16_t offset = scale16by8(tick, qadd8(rgb_matrix_config.speed, 1));
        RGB rgb = effect_func(D_RGB_KEYS, offset);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
}

static RGB DROPLETS_math(RGB rgb, uint16_t offset) {
    uint16_t scaled_offset = offset;
#if CUSTOM_RGB_MATRIX_KEYPRESS_SCALING == 2 // quadratic scaling
    scaled_offset = (offset * offset) / 255;
#elif CUSTOM_RGB_MATRIX_KEYPRESS_SCALING == 3 // cubic scaling
    scaled_offset = (offset * offset * offset) / (255 * 255);
#elif CUSTOM_RGB_MATRIX_KEYPRESS_SCALING == 4 // quartic scaling
    scaled_offset = (offset * offset * offset * offset) / (255 * 255 * 255);
#endif
    // inverse offset & scale
    HSV hsv = rgb_to_hsv(rgb);
    hsv.v = qsub8(hsv.v, 255 - scaled_offset);

    if (rgb_matrix_config.hsv.v < 255) hsv.v = scale8(hsv.v, rgb_matrix_config.hsv.v);
    rgb = hsv_to_rgb(hsv);

    return rgb;
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

bool DROPLETS(effect_params_t* params) {
    static uint32_t wait_timer = 0;
    if (wait_timer > g_rgb_timer) {
        return false;
    }

    inline uint32_t interval(void) {
        return D_EFFECT_INTERVAL / scale16by8(qadd8(rgb_matrix_config.speed, 16), 16);
    }

    if (params->init) {
        // clear LEDs
        rgb_matrix_set_color_all(0, 0, 0);

        // init timer
        timer_buffer = g_rgb_timer;

        // init last_drop_tracker
        last_drop_tracker.count = 0;
        for (uint8_t i = 0; i < LED_HITS_TO_REMEMBER; ++i) {
            last_drop_tracker.tick[i] = UINT16_MAX;
        }
    }

    d_effect_runner_reactive(params, &DROPLETS_math);

    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    if (!rgb_matrix_check_finished_leds(led_max)) {
        // add to last_drop_tracker
        uint8_t led[LED_HITS_TO_REMEMBER];
        uint8_t led_count = 0;
        led_count = rgb_matrix_map_row_column_to_led(random8_max(MATRIX_ROWS), random8_max(MATRIX_COLS), led);

        if (last_drop_tracker.count + led_count > LED_HITS_TO_REMEMBER) {
            memcpy(&last_drop_tracker.x[0], &last_drop_tracker.x[led_count], LED_HITS_TO_REMEMBER - led_count);
            memcpy(&last_drop_tracker.y[0], &last_drop_tracker.y[led_count], LED_HITS_TO_REMEMBER - led_count);
            memcpy(&last_drop_tracker.tick[0], &last_drop_tracker.tick[led_count], (LED_HITS_TO_REMEMBER - led_count) * 2); // 16 bit
            memcpy(&last_drop_tracker.index[0], &last_drop_tracker.index[led_count], LED_HITS_TO_REMEMBER - led_count);
            last_drop_tracker.count = LED_HITS_TO_REMEMBER - led_count;
        }

        for (uint8_t i = 0; i < led_count; i++) {
            uint8_t index                = last_drop_tracker.count;
            last_drop_tracker.x[index]     = g_led_config.point[led[i]].x;
            last_drop_tracker.y[index]     = g_led_config.point[led[i]].y;
            last_drop_tracker.index[index] = led[i];
            last_drop_tracker.tick[index]  = 0;
            last_drop_tracker.count++;
        }

        // update last_drop_tracker ticks
        uint32_t deltaTime = sync_timer_elapsed32(timer_buffer);
        uint8_t count = last_drop_tracker.count;
        for (uint8_t i = 0; i < count; ++i) {
            if (UINT16_MAX - deltaTime < last_drop_tracker.tick[i]) {
                last_drop_tracker.count--;
                continue;
            }
            last_drop_tracker.tick[i] += deltaTime;
        }

        // update timer
        timer_buffer = sync_timer_read32();

        // set pulse timer
        wait_timer = g_rgb_timer + interval();
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#       endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#   endif // ENABLE_RGB_MATRIX_DROPLETS
#endif // RGB_MATRIX_KEYPRESSES
