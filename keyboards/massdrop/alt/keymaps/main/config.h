#pragma once

#define TAPPING_TOGGLE 2
#define DUMMY_MOD_NEUTRALIZER_KEYCODE KC_RIGHT_CTRL

#define MK_3_SPEED
#define MK_C_OFFSET_0 4
#define MK_C_INTERVAL_0 7
#define MK_C_OFFSET_1 16
#define MK_C_INTERVAL_1 7
#define MK_C_OFFSET_2 32
#define MK_C_INTERVAL_2 7
#define MK_W_OFFSET_0 1
#define MK_W_INTERVAL_0 150
#define MK_W_OFFSET_1 1
#define MK_W_INTERVAL_1 50
#define MK_W_OFFSET_2 1
#define MK_W_INTERVAL_2 10

#define BOOT_DELAY 250
#define SPAM_DELAY 100
#define BLINK_DELAY 300

#define SNAKE_FRAME_DELAY 300
#define MAX_SNAKE_LENGTH 255
#define MOVE_QUEUE_SIZE 2 // max 2 due to debounce issues

#undef RGB_MATRIX_LED_FLUSH_LIMIT
#define RGB_MATRIX_LED_FLUSH_LIMIT 16
#define CUSTOM_RGB_MATRIX_TIMEOUT 600000 // 600 seconds (10 minutes)

#undef USE_CIE1931_CURVE

#define RGB_MATRIX_KEYPRESSES
#define RGB_MATRIX_FRAMEBUFFER_EFFECTS

/* RGB Matrix Keypress Scaling
 * 1 = no scaling (linear)
 * 2 = quadratic scaling
 * 3 = cubic scaling
 * 4 = quartic scaling
 */
#define CUSTOM_RGB_MATRIX_KEYPRESS_SCALING 3

// disable all effects enabled by Massdrop maintainers
#undef ENABLE_RGB_MATRIX_ALPHAS_MODS
#undef ENABLE_RGB_MATRIX_GRADIENT_UP_DOWN
#undef ENABLE_RGB_MATRIX_GRADIENT_LEFT_RIGHT
#undef ENABLE_RGB_MATRIX_BREATHING
#undef ENABLE_RGB_MATRIX_BAND_SAT
#undef ENABLE_RGB_MATRIX_BAND_VAL
#undef ENABLE_RGB_MATRIX_BAND_PINWHEEL_SAT
#undef ENABLE_RGB_MATRIX_BAND_PINWHEEL_VAL
#undef ENABLE_RGB_MATRIX_BAND_SPIRAL_SAT
#undef ENABLE_RGB_MATRIX_BAND_SPIRAL_VAL
#undef ENABLE_RGB_MATRIX_CYCLE_ALL
#undef ENABLE_RGB_MATRIX_CYCLE_LEFT_RIGHT
#undef ENABLE_RGB_MATRIX_CYCLE_UP_DOWN
#undef ENABLE_RGB_MATRIX_RAINBOW_MOVING_CHEVRON
#undef ENABLE_RGB_MATRIX_CYCLE_OUT_IN
#undef ENABLE_RGB_MATRIX_CYCLE_OUT_IN_DUAL
#undef ENABLE_RGB_MATRIX_CYCLE_PINWHEEL
#undef ENABLE_RGB_MATRIX_CYCLE_SPIRAL
#undef ENABLE_RGB_MATRIX_DUAL_BEACON
#undef ENABLE_RGB_MATRIX_RAINBOW_BEACON
#undef ENABLE_RGB_MATRIX_RAINBOW_PINWHEELS
#undef ENABLE_RGB_MATRIX_RAINDROPS
#undef ENABLE_RGB_MATRIX_JELLYBEAN_RAINDROPS
#undef ENABLE_RGB_MATRIX_HUE_BREATHING
#undef ENABLE_RGB_MATRIX_HUE_PENDULUM
#undef ENABLE_RGB_MATRIX_HUE_WAVE
#undef ENABLE_RGB_MATRIX_PIXEL_RAIN
#undef ENABLE_RGB_MATRIX_PIXEL_FLOW
#undef ENABLE_RGB_MATRIX_PIXEL_FRACTAL
#undef ENABLE_RGB_MATRIX_TYPING_HEATMAP
#undef ENABLE_RGB_MATRIX_DIGITAL_RAIN
#undef ENABLE_RGB_MATRIX_SOLID_REACTIVE_SIMPLE
#undef ENABLE_RGB_MATRIX_SOLID_REACTIVE
#undef ENABLE_RGB_MATRIX_SOLID_REACTIVE_WIDE
#undef ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTIWIDE
#undef ENABLE_RGB_MATRIX_SOLID_REACTIVE_CROSS
#undef ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTICROSS
#undef ENABLE_RGB_MATRIX_SOLID_REACTIVE_NEXUS
#undef ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTINEXUS
#undef ENABLE_RGB_MATRIX_SPLASH
#undef ENABLE_RGB_MATRIX_MULTISPLASH
#undef ENABLE_RGB_MATRIX_SOLID_SPLASH
#undef ENABLE_RGB_MATRIX_SOLID_MULTISPLASH

// selected RGB matrix effects
#define ENABLE_RGB_MATRIX_RAINBOW_MOVING_CHEVRON
// #define ENABLE_RGB_MATRIX_CYCLE_OUT_IN
// #define ENABLE_RGB_MATRIX_CYCLE_OUT_IN_DUAL
// #define ENABLE_RGB_MATRIX_CYCLE_SPIRAL
// #define ENABLE_RGB_MATRIX_PIXEL_RAIN
// #define ENABLE_RGB_MATRIX_PIXEL_FLOW
// #define ENABLE_RGB_MATRIX_PIXEL_FRACTAL
// #define ENABLE_RGB_MATRIX_DIGITAL_RAIN
// #define ENABLE_RGB_MATRIX_FLOWER_BLOOMING
// #define ENABLE_RGB_MATRIX_STARLIGHT
// #define ENABLE_RGB_MATRIX_STARLIGHT_DUAL_HUE
// #define ENABLE_RGB_MATRIX_STARLIGHT_DUAL_SAT
// #define ENABLE_RGB_MATRIX_RIVERFLOW
// #define ENABLE_RGB_MATRIX_SOLID_REACTIVE
// #define RGB_MATRIX_SOLID_REACTIVE_GRADIENT_MODE
// #define ENABLE_RGB_MATRIX_RAINDROPS
// #define ENABLE_RGB_MATRIX_JELLYBEAN_RAINDROPS
#define ENABLE_RGB_MATRIX_TYPING_HEATMAP
#define ENABLE_RGB_MATRIX_MULTISPLASH


// custom RGB matrix effects
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_V2
#define ENABLE_RGB_MATRIX_RIVERFLOW_V2
#define ENABLE_RGB_MATRIX_DROPLETS
#define ENABLE_RGB_MATRIX_MULTICROSS_V2
#define ENABLE_RGB_MATRIX_ELECTRONS
#define ENABLE_RGB_MATRIX_ELECTRONS_NO_WRAP
#define ENABLE_RGB_MATRIX_ELECTRIC_FLOW
#define ENABLE_RGB_MATRIX_FLIP
#define ENABLE_RGB_MATRIX_DIGITAL_RAIN_V2

#define RGB_MATRIX_STARTUP_MODE RGB_MATRIX_CUSTOM_SOLID_REACTIVE_V2

#define STRIP_START 67
#define SET_RGB(R, G, B)  {.r = (R), .g = (G), .b = (B)}

#define RGB_KEYS_R 80
#define RGB_KEYS_G 195
#define RGB_KEYS_B 255

#define RGB_STRIP_R 130
#define RGB_STRIP_G 255
#define RGB_STRIP_B 230

#define RGB_PRESS_R 0
#define RGB_PRESS_G 40
#define RGB_PRESS_B 255

#define RGB_KEY6_R 50
#define RGB_KEY6_G 190
#define RGB_KEY6_B 255

#define RGB_CUSTOM_RED_R 255
#define RGB_CUSTOM_RED_G 5
#define RGB_CUSTOM_RED_B 5

#define RGB_CUSTOM_ORANGE_R 255
#define RGB_CUSTOM_ORANGE_G 100
#define RGB_CUSTOM_ORANGE_B 0

#define RGB_CUSTOM_YELLOW_R 235
#define RGB_CUSTOM_YELLOW_G 255
#define RGB_CUSTOM_YELLOW_B 0

#define RGB_CUSTOM_GREEN_R 0
#define RGB_CUSTOM_GREEN_G 255
#define RGB_CUSTOM_GREEN_B 5

#define RGB_CUSTOM_BLUE_R 0
#define RGB_CUSTOM_BLUE_G 40
#define RGB_CUSTOM_BLUE_B 255

#define RGB_CUSTOM_PURPLE_R 30
#define RGB_CUSTOM_PURPLE_G 0
#define RGB_CUSTOM_PURPLE_B 255

#define RGB_CUSTOM_CYAN_R 0
#define RGB_CUSTOM_CYAN_G 200
#define RGB_CUSTOM_CYAN_B 255

#define RGB_CUSTOM_MAGENTA_R 255
#define RGB_CUSTOM_MAGENTA_G 0
#define RGB_CUSTOM_MAGENTA_B 100

#define RGB_SNAKE_GREEN_R 0
#define RGB_SNAKE_GREEN_G 255
#define RGB_SNAKE_GREEN_B 0

#define RGB_SNAKE_RED_R 255
#define RGB_SNAKE_RED_G 0
#define RGB_SNAKE_RED_B 0
