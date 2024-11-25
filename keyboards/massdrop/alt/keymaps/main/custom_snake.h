#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
#    ifdef ENABLE_RGB_MATRIX_CUSTOM_SNAKE
RGB_MATRIX_EFFECT(CUSTOM_SNAKE)
#        ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static HSV CUSTOM_SNAKE_math(HSV hsv, uint16_t offset) {
#            ifdef RGB_MATRIX_CUSTOM_SNAKE_GRADIENT_MODE
    hsv.h = scale16by8(g_rgb_timer, qadd8(rgb_matrix_config.speed, 8) >> 4);
#            endif
    hsv.h += scale8(255 - offset, 64);
    return hsv;
}

bool CUSTOM_SNAKE(effect_params_t* params) {
    return effect_runner_reactive(params, &CUSTOM_SNAKE_math);
}

#        endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#    endif     // ENABLE_RGB_MATRIX_CUSTOM_SNAKE
#endif         // RGB_MATRIX_KEYREACTIVE_ENABLED
