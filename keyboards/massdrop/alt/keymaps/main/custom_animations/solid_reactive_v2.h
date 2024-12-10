#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
#    ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_V2
RGB_MATRIX_EFFECT(SOLID_REACTIVE_V2)
#        ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static HSV SOLID_REACTIVE_V2_math(HSV hsv, uint16_t offset) {
    hsv.h = 0;
    hsv.s = 255;
    hsv.v = 255;

    hsv.h += scale8(255 - offset, 128);
    if (hsv.h > 255) {
        hsv.h = 0;
        hsv.s = 0;
    }

    return hsv;
}

bool SOLID_REACTIVE_V2(effect_params_t* params) {
    return effect_runner_reactive(params, &SOLID_REACTIVE_V2_math);
}

#        endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#    endif     // ENABLE_RGB_MATRIX_SOLID_REACTIVE_V2
#endif         // RGB_MATRIX_KEYREACTIVE_ENABLED
