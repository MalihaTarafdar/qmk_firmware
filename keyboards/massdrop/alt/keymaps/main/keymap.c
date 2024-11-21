#include QMK_KEYBOARD_H

#define MODS_SHIFT  (get_mods() & MOD_BIT(KC_LSFT) || get_mods() & MOD_BIT(KC_RSFT))
#define MODS_CTRL  (get_mods() & MOD_BIT(KC_LCTL) || get_mods() & MOD_BIT(KC_RCTL))
#define MODS_ALT  (get_mods() & MOD_BIT(KC_LALT) || get_mods() & MOD_BIT(KC_RALT))

uint16_t KEYBOARD_LAYOUTS[3];
uint8_t NUM_LAYOUTS = 3;
uint8_t cycle_layout_index;

bool spam_config_active = false;
bool spam_active = false;
uint16_t spam_keycode;
uint32_t spam_timer = 0;

// for persistent storage
typedef union {
  uint32_t raw;
  struct {
    uint16_t saved_layer_state :16;
    uint8_t cycle_layout_index :2;
  };
} user_config_t;
user_config_t user_config;

enum alt_keycodes {
    U_T_AUTO = SAFE_RANGE,    // USB Extra Port Toggle Auto Detect / Always Active
    U_T_AGCR,                 // USB Toggle Automatic GCR control
    DBG_TOG,                  // DEBUG Toggle On / Off
    DBG_MTRX,                 // DEBUG Toggle Matrix Prints
    DBG_KBD,                  // DEBUG Toggle Keyboard Prints
    DBG_MOU,                  // DEBUG Toggle Mouse Prints
    MD_BOOT,                  // restart into bootloader after hold timeout
    TOG_MD,                   // toggle keyboard mode (Linux(+Windows)/Mac)
    CYC_LT,                   // cycle keyboard layout (QWERTY, Dvorak, Colemak)
    SPAM,                     // spam key macro
    DMACRO                    // disable function layer & enable macro layer
};

enum layer_names {
    _MAIN,    // main layer (QWERTY layout)
    _DVK,     // Dvorak layout
    _CMK,     // Colemak layout layer
    _LIN,     // Linux layer
    _MAC,     // MacOS layer
    _FN,      // function layer
    _DM,      // dynamic macro layer
    _MS,      // mouse keys layer
    _RGB      // RGB layer
};

// reperesent layer state (uint16_t) as a binary string for debug
char *layer_state_to_binary_string(layer_state_t state) {
    int num_bits = sizeof(layer_state_t) * 8;
    char *ret = malloc(num_bits);

    for (int i = num_bits - 1; i >= 0; i--) {
        ret[i] = (state & 1) + '0';
        state >>= 1;
    }

    ret[num_bits] = '\0';
    return ret;
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_MAIN] = LAYOUT_65_ansi_blocker(
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, KC_DEL,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_HOME,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,          KC_ENT,  KC_PGUP,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,          KC_UP,   KC_PGDN,
        _______, KC_LALT, _______,                            KC_SPC,                             _______, TT(_FN), KC_LEFT, KC_DOWN, KC_RGHT
    ),
    [_DVK] = LAYOUT_65_ansi_blocker(
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_LBRC, KC_RBRC, KC_BSPC, KC_DEL,
        KC_TAB,  KC_QUOT, KC_COMM, KC_DOT,  KC_P,    KC_Y,    KC_F,    KC_G,    KC_C,    KC_R,    KC_L,    KC_SLSH, KC_EQL,  KC_BSLS, KC_HOME,
        KC_CAPS, KC_A,    KC_O,    KC_E,    KC_U,    KC_I,    KC_D,    KC_H,    KC_T,    KC_N,    KC_S,    KC_MINS,          KC_ENT,  KC_PGUP,
        KC_LSFT, KC_SCLN, KC_Q,    KC_J,    KC_K,    KC_X,    KC_B,    KC_M,    KC_W,    KC_V,    KC_Z,    KC_RSFT,          KC_UP,   KC_PGDN,
        _______, KC_LALT, _______,                            KC_SPC,                             _______, TT(_FN), KC_LEFT, KC_DOWN, KC_RGHT
    ),
    [_CMK] = LAYOUT_65_ansi_blocker(
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, KC_DEL,
        KC_TAB,  KC_Q,    KC_W,    KC_F,    KC_P,    KC_G,    KC_J,    KC_L,    KC_U,    KC_Y,    KC_SCLN, KC_LBRC, KC_RBRC, KC_BSLS, KC_HOME,
        KC_CAPS, KC_A,    KC_R,    KC_S,    KC_T,    KC_D,    KC_H,    KC_N,    KC_E,    KC_I,    KC_O,    KC_QUOT,          KC_ENT,  KC_PGUP,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_K,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,          KC_UP,   KC_PGDN,
        _______, KC_LALT, _______,                            KC_SPC,                             _______, TT(_FN), KC_LEFT, KC_DOWN, KC_RGHT
    ),
    [_LIN] = LAYOUT_65_ansi_blocker(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______,
        KC_LGUI, _______, KC_LCTL,                            _______,                            KC_RCTL, _______, _______, _______, _______
    ),
    [_MAC] = LAYOUT_65_ansi_blocker(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______,
        KC_LCTL, _______, KC_LGUI,                            _______,                            KC_RGUI, _______, _______, _______, _______
    ),
    [_FN] = LAYOUT_65_ansi_blocker(
        KC_GRV,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_DEL,  KC_INS,
        _______, KC_F13,  KC_F14,  KC_F15,  KC_F16,  KC_F17,  KC_F18,  KC_F19,  _______, _______, _______, KC_BRID, KC_BRIU, KC_PSCR, KC_END,
        _______, _______, SPAM,    DMACRO,  _______, KC_HOME, KC_END,  KC_PGUP, KC_PGDN, _______, KC_SCRL, KC_PAUS,          _______, CYC_LT,
        _______, EE_CLR, U_T_AUTO,U_T_AGCR, _______, MD_BOOT, NK_TOGG, TG(_MS), KC_MUTE, KC_VOLD, KC_VOLU, _______,          MO(_RGB),TOG_MD,
        _______, _______, _______,                            DB_TOGG,                            _______, _______, KC_MPRV, KC_MPLY, KC_MNXT
    ),
    [_DM] = LAYOUT_65_ansi_blocker(
        TG(_DM), _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, DM_REC1,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, DM_REC2,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, DM_PLY1,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, DM_PLY2,
        _______, _______, _______,                            _______,                            _______, _______, _______, _______, _______
    ),
    [_MS] = LAYOUT_65_ansi_blocker(
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, MS_ACL0, MS_ACL1, MS_ACL2, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, MS_UP,   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, MS_BTN4, MS_WHLU, MS_BTN5, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, MS_LEFT, MS_DOWN, MS_RGHT, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, MS_WHLL, MS_WHLD, MS_WHLR,          XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, TG(_MS), MS_BTN3, MS_BTN1, MS_BTN2, XXXXXXX,          XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX,                            XXXXXXX,                            XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
    ),
    [_RGB] = LAYOUT_65_ansi_blocker(
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, RGB_SPD, RGB_VAI, RGB_SPI, RGB_HUI, RGB_SAI, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, RGB_RMOD,RGB_VAD, RGB_MOD, RGB_HUD, RGB_SAD, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,          XXXXXXX, XXXXXXX,
        XXXXXXX, RGB_TOG, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, _______,          XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX,                            XXXXXXX,                            XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
    )
};

void init_user(void) {
    debug_enable = true; // REMOVE
    // init keyboard layout cycle
    KEYBOARD_LAYOUTS[0] = _MAIN;
    KEYBOARD_LAYOUTS[1] = _DVK;
    KEYBOARD_LAYOUTS[2] = _CMK;
    cycle_layout_index = 0;

    // Linux/Windows mode enabled by default
    layer_on(_LIN);
}

void keyboard_post_init_user() {
    init_user();

    // init from EEPROM
    user_config.raw = eeconfig_read_user();
    if (user_config.raw != 0) {
        layer_state_set(user_config.saved_layer_state);
        cycle_layout_index = user_config.cycle_layout_index;
    }
}

void eeconfig_init_user() {
    init_user();
    user_config.raw = 0;
    eeconfig_update_user(user_config.raw);
}

layer_state_t layer_state_set_user(layer_state_t state) {
    char *read_layer_state = layer_state_to_binary_string(user_config.saved_layer_state);
    dprintf("Read Layer State:\t%s (%u)\n", read_layer_state, user_config.saved_layer_state);
    free(read_layer_state);

    char *cur_state = layer_state_to_binary_string(state);
    dprintf("Current State:\t%s (%u), Highest Layer: %u\n", cur_state, state, get_highest_layer(state));
    free(cur_state);

    // turn off function layer if dynamic macro layer is on
    if (IS_LAYER_ON_STATE(state, _DM) && IS_LAYER_ON_STATE(state, _FN)) {
        state -= (1 << _FN);
        layer_off(_FN);
    }

    return state;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    static uint32_t key_timer;

    // activate spam
    if (spam_config_active && record->event.pressed && keycode != KC_ESC) {
        spam_keycode = keycode;
        spam_config_active = false;
        spam_active = true;
        spam_timer = timer_read32();
        dprint("SPAM ACTIVE\n");
    }

    switch (keycode) {
        case KC_ESC:
            // deactivate spam config & spam
            if (record->event.pressed) {
                spam_config_active = false;
                spam_active = false;
            }
            return true;
        case SPAM:
            // activate spam config
            if (record->event.pressed) {
                spam_config_active = true;
                dprint("SPAM CONFIG ACTIVE\n");
            }
            return false;
        case DMACRO:
            // layer_off(_FN); // _FN is re-enabled on FN key release
            layer_on(_DM);
            dprint("DYNAMIC MACROS ON\n");
            return false;
        case TOG_MD:
            // toggle keyboard mode (Linux(+Windows)/Mac)
            if (record->event.pressed) {
                if (IS_LAYER_ON(_MAC)) {
                    layer_off(_MAC);
                    layer_on(_LIN);
                } else if (IS_LAYER_ON(_LIN)) {
                    layer_off(_LIN);
                    layer_on(_MAC);
                } else {
                    layer_on(_LIN);
                }
                char *wb_layer_state = layer_state_to_binary_string(layer_state);
                dprintf("WB Layer State:\t%s (%u)\n", wb_layer_state, layer_state);
                free(wb_layer_state);

                // write layer state to EEPROM
                // user_config.saved_layer_state = layer_state & ~(1 << (_FN + 1)); // remove function layer from layer mask
                user_config.saved_layer_state = layer_state - (1 << _FN); // remove function layer from layer mask
                eeconfig_update_user(user_config.raw);

                char *wa_layer_state = layer_state_to_binary_string(user_config.saved_layer_state);
                dprintf("WA Layer State:\t%s (%u)\n", wa_layer_state, user_config.saved_layer_state);
                free(wa_layer_state);
            }
            return false;
        case CYC_LT:
            // cycle through keyboard layouts (QWERTY, Dvorak, Colemak)
            if (record->event.pressed) {
                default_layer_set(1 << KEYBOARD_LAYOUTS[cycle_layout_index]);
                dprintf("layer turned OFF: %u\n", KEYBOARD_LAYOUTS[cycle_layout_index]);

                cycle_layout_index++;
                if (cycle_layout_index >= NUM_LAYOUTS) {
                    cycle_layout_index = 0;
                }

                // default_layer_set(1 << KEYBOARD_LAYOUTS[cycle_layout_index]);
                set_single_persistent_default_layer(KEYBOARD_LAYOUTS[cycle_layout_index]);

                // write cycle_layout_index to EEPROM
                // NOTE: did not get rid of local variable to allow for data persistence removal in the future
                user_config.cycle_layout_index = cycle_layout_index;
                eeconfig_update_user(user_config.raw);

                dprintf("layer turned ON: %u\n", KEYBOARD_LAYOUTS[cycle_layout_index]);
            }
            return false;
        case U_T_AUTO:
            if (record->event.pressed && MODS_SHIFT && MODS_CTRL) {
                TOGGLE_FLAG_AND_PRINT(usb_extra_manual, "USB extra port manual mode");
            }
            return false;
        case U_T_AGCR:
            if (record->event.pressed && MODS_SHIFT && MODS_CTRL) {
                TOGGLE_FLAG_AND_PRINT(usb_gcr_auto, "USB GCR auto mode");
            }
            return false;
        case DBG_TOG:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_enable, "Debug mode");
            }
            return false;
        case DBG_MTRX:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_matrix, "Debug matrix");
            }
            return false;
        case DBG_KBD:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_keyboard, "Debug keyboard");
            }
            return false;
        case DBG_MOU:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_mouse, "Debug mouse");
            }
            return false;
        case MD_BOOT:
            if (record->event.pressed) {
                key_timer = timer_read32();
            } else {
                if (timer_elapsed32(key_timer) >= 500) {
                    reset_keyboard();
                }
            }
            return false;
        case RGB_TOG:
            if (record->event.pressed) {
              switch (rgb_matrix_get_flags()) {
                case LED_FLAG_ALL: {
                    rgb_matrix_set_flags(LED_FLAG_KEYLIGHT | LED_FLAG_MODIFIER | LED_FLAG_INDICATOR);
                    rgb_matrix_set_color_all(0, 0, 0);
                  }
                  break;
                case (LED_FLAG_KEYLIGHT | LED_FLAG_MODIFIER | LED_FLAG_INDICATOR): {
                    rgb_matrix_set_flags(LED_FLAG_UNDERGLOW);
                    rgb_matrix_set_color_all(0, 0, 0);
                  }
                  break;
                case LED_FLAG_UNDERGLOW: {
                    rgb_matrix_set_flags(LED_FLAG_NONE);
                    rgb_matrix_disable_noeeprom();
                  }
                  break;
                default: {
                    rgb_matrix_set_flags(LED_FLAG_ALL);
                    rgb_matrix_enable_noeeprom();
                  }
                  break;
              }
            }
            return false;
        default:
            return true; // process all other keycodes normally
    }
}

void matrix_scan_user() {
    if (spam_active) {
        if (timer_elapsed32(spam_timer) > SPAM_DELAY) {
            tap_code(spam_keycode);
            dprintf("keycode pressed: %u\n", spam_keycode);
            spam_timer = timer_read32();
        }
    }
}
