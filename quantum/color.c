/* Copyright 2017 Jason Williams
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "color.h"
#include "led_tables.h"
#include "progmem.h"
#include "util.h"

RGB hsv_to_rgb_impl(HSV hsv, bool use_cie) {
    RGB      rgb;
    uint8_t  region, remainder, p, q, t;
    uint16_t h, s, v;

    if (hsv.s == 0) {
#ifdef USE_CIE1931_CURVE
        if (use_cie) {
            rgb.r = rgb.g = rgb.b = pgm_read_byte(&CIE1931_CURVE[hsv.v]);
        } else {
            rgb.r = hsv.v;
            rgb.g = hsv.v;
            rgb.b = hsv.v;
        }
#else
        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;
#endif
        return rgb;
    }

    h = hsv.h;
    s = hsv.s;
#ifdef USE_CIE1931_CURVE
    if (use_cie) {
        v = pgm_read_byte(&CIE1931_CURVE[hsv.v]);
    } else {
        v = hsv.v;
    }
#else
    v = hsv.v;
#endif

    region    = h * 6 / 255;
    remainder = (h * 2 - region * 85) * 3;

    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
        case 6:
        case 0:
            rgb.r = v;
            rgb.g = t;
            rgb.b = p;
            break;
        case 1:
            rgb.r = q;
            rgb.g = v;
            rgb.b = p;
            break;
        case 2:
            rgb.r = p;
            rgb.g = v;
            rgb.b = t;
            break;
        case 3:
            rgb.r = p;
            rgb.g = q;
            rgb.b = v;
            break;
        case 4:
            rgb.r = t;
            rgb.g = p;
            rgb.b = v;
            break;
        default:
            rgb.r = v;
            rgb.g = p;
            rgb.b = q;
            break;
    }

    return rgb;
}

HSV rgb_to_hsv_impl(RGB rgb, bool use_cie) {
    HSV hsv;
    uint8_t r, g, b;
    uint8_t c_min, c_max, delta;

    r = rgb.r;
    g = rgb.g;
    b = rgb.b;

#ifdef USE_CIE1931_CURVE
    if (use_cie) {
        r = pgm_read_byte(&CIE1931_CURVE[rgb.r]);
        g = pgm_read_byte(&CIE1931_CURVE[rgb.g]);
        b = pgm_read_byte(&CIE1931_CURVE[rgb.b]);
    }
#endif

    c_max = r;
    if (g > c_max) c_max = g;
    if (b > c_max) c_max = b;

    c_min = r;
    if (g < c_min) c_min = g;
    if (b < c_min) c_min = b;

    delta = c_max - c_min;

    // hsv.h
    if (delta == 0) {
        hsv.h = 0;
    } else if (c_max == r) {
        hsv.h = 43 * (g - b) / delta;
    } else if (c_max == g) {
        hsv.h = 85 + 43 * (b - r) / delta;
    } else if (c_max == b) {
        hsv.h = 171 + 43 * (r - g) / delta;
    }

    // wrap hue + ensure >= 0
    hsv.h = (hsv.h + 255) % 255;

    // hsv.s
    hsv.s = (c_max != 0) ? delta * 255 / c_max : 0;

    // hsv.v
    hsv.v = c_max;

    return hsv;
}

RGB hsv_to_rgb(HSV hsv) {
#ifdef USE_CIE1931_CURVE
    return hsv_to_rgb_impl(hsv, true);
#else
    return hsv_to_rgb_impl(hsv, false);
#endif
}

RGB hsv_to_rgb_nocie(HSV hsv) {
    return hsv_to_rgb_impl(hsv, false);
}

HSV rgb_to_hsv(RGB rgb) {
#ifdef USE_CIE1931_CURVE
    return rgb_to_hsv_impl(rgb, true);
#else
    return rgb_to_hsv_impl(rgb, false);
#endif
}

HSV rgb_to_hsv_nocie(RGB rgb) {
    return rgb_to_hsv_impl(rgb, false);
}

#ifdef WS2812_RGBW
void convert_rgb_to_rgbw(rgb_led_t *led) {
    // Determine lowest value in all three colors, put that into
    // the white channel and then shift all colors by that amount
    led->w = MIN(led->r, MIN(led->g, led->b));
    led->r -= led->w;
    led->g -= led->w;
    led->b -= led->w;
}
#endif
