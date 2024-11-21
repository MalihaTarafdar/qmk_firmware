#include QMK_KEYBOARD_H

// NOTE: must toggle function instead of momentary activation to start recording dynamic macros

// #define MODS_ONLY_SFT ((get_mods() & ~MOD_MASK_SHIFT) == 0 && (get_mods() & MOD_MASK_SHIFT))
// #define MODS_ONLY_CTL ((get_mods() & ~MOD_MASK_CTRL) == 0 && (get_mods() & MOD_MASK_CTRL))
// #define MODS_ONLY_ALT ((get_mods() & ~MOD_MASK_ALT) == 0 && (get_mods() & MOD_MASK_ALT))
// #define MODS_ONLY_GUI ((get_mods() & ~MOD_MASK_GUI) == 0 && (get_mods() & MOD_MASK_GUI))

enum alt_keycodes {
    U_T_AUTO = SAFE_RANGE,    // USB Extra Port Toggle Auto Detect / Always Active
    U_T_AGCR,                 // USB Toggle Automatic GCR control
    DBG_TOG,                  // DEBUG Toggle On / Off
    DBG_MTRX,                 // DEBUG Toggle Matrix Prints
    DBG_KBD,                  // DEBUG Toggle Keyboard Prints
    DBG_MOU,                  // DEBUG Toggle Mouse Prints
    MD_BOOT,                  // restart into bootloader after hold timeout
    CYC_MD,                   // cycle keyboard mode
    CYC_LT,                   // cycle keyboard layout
    SPAM,                     // spam key macro
    TG_SHCT                   // toggle custom shortcuts
};

enum layer_names {
    _MAIN,    // main layer (QWERTY layout)
    _DVK,     // Dvorak layout
    _CMK,     // Colemak layout layer
    _FN,      // function layer
    _MS,      // mouse keys layer
    _RGB      // RGB layer
};

uint8_t mod_state;

/*
 * Keyboard Mode Rules
 * Linux
 *     - KC_LCTL: send KC_LGUI instead
 *     - KC_LGUI: send KC_LALT instead
 *     - KC_LALT: send KC_LCTL instead
 *     - KC_RALT: send KC_RCTL instead
 * MacOS
 *     - KC_LGUI: send KC_LALT instead
 *     - KC_LALT: send KC_LGUI instead
 *     - KC_RALT: send KC_RGUI instead
 * Windows
 *     - KC_LCTL: send KC_LGUI instead
 *     - KC_LGUI: send KC_LALT instead
 *     - KC_LALT: send KC_LCTL instead
 *     - KC_RALT: send KC_RCTL instead
*/
enum modes {
    M_LINUX,
    M_MACOS,
    M_WINDOWS
};
uint8_t CYCLE_MODE_START = 0;
uint8_t CYCLE_MODE_END = 2;
uint8_t mode_index;

uint8_t CYCLE_LAYOUT_START = 0;
uint8_t CYCLE_LAYOUT_END = 2;
uint8_t layout_index;

bool spam_config_active = false;
bool spam_active = false;
uint16_t spam_keycode;
uint32_t spam_timer;

/* Custom Shortcuts
 * [1] CTL + LEFT -> ALT + LEFT
 * [2] CTL + RGHT -> ALT + RGHT
 * [3] HOME -> CTL + LEFT
 * [4] END -> CTL + RGHT
*/
bool custom_shortcuts_enabled;

typedef union {
  uint32_t raw;
  struct {
    uint8_t mode_index :2;
    uint8_t layout_index :2;
  };
} user_config_t;
user_config_t user_config;

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
        KC_LCTL, KC_LGUI, KC_LALT,                            KC_SPC,                             KC_RALT, TT(_FN), KC_LEFT, KC_DOWN, KC_RGHT
    ),
    [_DVK] = LAYOUT_65_ansi_blocker(
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_LBRC, KC_RBRC, KC_BSPC, KC_DEL,
        KC_TAB,  KC_QUOT, KC_COMM, KC_DOT,  KC_P,    KC_Y,    KC_F,    KC_G,    KC_C,    KC_R,    KC_L,    KC_SLSH, KC_EQL,  KC_BSLS, KC_HOME,
        KC_CAPS, KC_A,    KC_O,    KC_E,    KC_U,    KC_I,    KC_D,    KC_H,    KC_T,    KC_N,    KC_S,    KC_MINS,          KC_ENT,  KC_PGUP,
        KC_LSFT, KC_SCLN, KC_Q,    KC_J,    KC_K,    KC_X,    KC_B,    KC_M,    KC_W,    KC_V,    KC_Z,    KC_RSFT,          KC_UP,   KC_PGDN,
        KC_LCTL, KC_LGUI, KC_LALT,                            KC_SPC,                             KC_RALT, TT(_FN), KC_LEFT, KC_DOWN, KC_RGHT
    ),
    [_CMK] = LAYOUT_65_ansi_blocker(
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, KC_DEL,
        KC_TAB,  KC_Q,    KC_W,    KC_F,    KC_P,    KC_G,    KC_J,    KC_L,    KC_U,    KC_Y,    KC_SCLN, KC_LBRC, KC_RBRC, KC_BSLS, KC_HOME,
        KC_CAPS, KC_A,    KC_R,    KC_S,    KC_T,    KC_D,    KC_H,    KC_N,    KC_E,    KC_I,    KC_O,    KC_QUOT,          KC_ENT,  KC_PGUP,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_K,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,          KC_UP,   KC_PGDN,
        KC_LCTL, KC_LGUI, KC_LALT,                            KC_SPC,                             KC_RALT, TT(_FN), KC_LEFT, KC_DOWN, KC_RGHT
    ),
    [_FN] = LAYOUT_65_ansi_blocker(
        KC_GRV,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_DEL,  KC_INS,
        _______, KC_F13,  KC_F14,  KC_F15,  KC_F16,  KC_F17,  KC_F18,  KC_F19,  _______, _______, _______, KC_BRID, KC_BRIU, KC_PSCR, KC_END,
        _______, TG_SHCT, SPAM,    _______, DM_REC1, DM_REC2, DM_PLY1, DM_PLY2, _______, _______, KC_SCRL, KC_PAUS,          _______, CYC_LT,
        _______, EE_CLR, U_T_AUTO,U_T_AGCR, _______, MD_BOOT, NK_TOGG, TG(_MS), KC_MUTE, KC_VOLD, KC_VOLU, _______,          TT(_RGB),CYC_MD,
        _______, _______, _______,                            DB_TOGG,                            _______, _______, KC_MPRV, KC_MPLY, KC_MNXT
    ),
    [_MS] = LAYOUT_65_ansi_blocker(
        TG(_MS), XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, MS_ACL0, MS_ACL1, MS_ACL2, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, MS_UP,   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, MS_BTN4, MS_WHLU, MS_BTN5, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, MS_LEFT, MS_DOWN, MS_RGHT, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, MS_WHLL, MS_WHLD, MS_WHLR,          XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, TG(_MS), MS_BTN3, MS_BTN1, MS_BTN2, XXXXXXX,          XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX,                            XXXXXXX,                            XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
    ),
    [_RGB] = LAYOUT_65_ansi_blocker(
        TG(_RGB),XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, RGB_SPD, RGB_VAI, RGB_SPI, RGB_HUI, RGB_SAI, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, RGB_RMOD,RGB_VAD, RGB_MOD, RGB_HUD, RGB_SAD, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,          XXXXXXX, XXXXXXX,
        XXXXXXX, RGB_TOG, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,          _______, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX,                            XXXXXXX,                            XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
    )
};

void init_user(void) {
    debug_enable = true; // REMOVE
    // Linux/Windows mode enabled by default
    mode_index = M_LINUX;

    // QWERTY keyboard layout enabled by default
    layout_index = CYCLE_LAYOUT_START;

    // custom shortcuts enabled by default
    custom_shortcuts_enabled = true;
}

void keyboard_post_init_user() {
    init_user();

    // init from EEPROM
    user_config.raw = eeconfig_read_user();
    if (user_config.raw != 0) {
        mode_index = user_config.mode_index;
        layout_index = user_config.layout_index;
    }
}

void eeconfig_init_user() {
    init_user();
    user_config.raw = 0;
    eeconfig_update_user(user_config.raw);
}

layer_state_t layer_state_set_user(layer_state_t state) {
    char *state_binary_str = layer_state_to_binary_string(state);
    dprintf("Current State:\t%s (%u), Highest Layer: %u\n", state_binary_str, state, get_highest_layer(state));
    free(state_binary_str);
    return state;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    static uint32_t key_timer;

    // activate spam
    if (spam_config_active && record->event.pressed && keycode != KC_ESC) {
        spam_config_active = false;
        spam_keycode = keycode;
        spam_timer = timer_read32();
        spam_active = true;
        dprint("SPAM ACTIVE\n");
    }

    mod_state = get_mods();

    switch (keycode) {
        case KC_ESC:
            // deactivate spam config & spam
            if (record->event.pressed) {
                spam_config_active = false;
                spam_active = false;
            }
            return true;
        case KC_LCTL:
            if (mode_index == M_LINUX || mode_index == M_WINDOWS) {
                // send KC_LGUI instead of KC_LCTL
                if (record->event.pressed) {
                    register_code(KC_LGUI);
                } else {
                    unregister_code(KC_LGUI);
                }
                return false;
            }
            return true;
        case KC_LGUI:
            if (mode_index == M_LINUX || mode_index == M_MACOS || mode_index == M_WINDOWS) {
                // send KC_LALT instead of KC_LGUI
                if (record->event.pressed) {
                    register_code(KC_LALT);
                } else {
                    unregister_code(KC_LALT);
                }
                return false;
            }
            return true;
        case KC_LALT:
            if (mode_index == M_LINUX || mode_index == M_WINDOWS) {
                // send KC_LCTL instead of KC_LALT
                if (record->event.pressed) {
                    register_code(KC_LCTL);
                } else {
                    unregister_code(KC_LCTL);
                }
                return false;
            } else if (mode_index == M_MACOS) {
                // send KC_LGUI instead of KC_LALT
                if (record->event.pressed) {
                    register_code(KC_LGUI);
                } else {
                    unregister_code(KC_LGUI);
                }
                return false;
            }
            return true;
        case KC_RALT:
            if (mode_index == M_LINUX || mode_index == M_WINDOWS) {
                // send KC_RCTL instead of KC_RALT
                if (record->event.pressed) {
                    register_code(KC_RCTL);
                } else {
                    unregister_code(KC_RCTL);
                }
                return false;
            } else if (mode_index == M_MACOS) {
                // send KC_RGUI instead of KC_RALT
                if (record->event.pressed) {
                    register_code(KC_RGUI);
                } else {
                    unregister_code(KC_RGUI);
                }
                return false;
            }
            return true;
        case KC_LEFT:
            if (custom_shortcuts_enabled) {
                static bool left_registered = false;
                static bool home_registered = false;

                if (record->event.pressed) {
                    if (mode_index == M_WINDOWS || mode_index == M_MACOS) {
                        bool mods_cond_shct1 = ((get_mods() & MOD_MASK_ALT) && !(get_mods() & MOD_MASK_CTRL) && !(get_mods() & MOD_MASK_GUI));
                        bool mods_cond_shct3 = ((get_mods() & MOD_MASK_CTRL) && !(get_mods() & MOD_MASK_ALT) && !(get_mods() & MOD_MASK_GUI));

                        if (mods_cond_shct1) {
                            dprint("shortcut 1 registered\n");

                            // remove ALT and register KC_LEFT with KC_LCTRL
                            set_mods((mod_state & ~MOD_MASK_ALT) | MOD_BIT_LCTRL);
                            register_code(KC_LEFT);
                            left_registered = true;

                            return false;
                        } else if (mods_cond_shct3) {
                            dprint("shortcut 3 registered\n");

                            // remove mods and register KC_HOME
                            clear_mods();
                            register_code(KC_HOME);
                            home_registered = true;

                            return false;
                        }
                    }
                } else {
                    if (left_registered) {
                        // unregister KC_LEFT and reapply mod state
                        unregister_code(KC_LEFT);
                        left_registered = false;
                        set_mods(mod_state);
                        return false;
                    }
                    if (home_registered) {
                        // unregister KC_HOME and reapply mod state
                        unregister_code(KC_HOME);
                        home_registered = false;
                        set_mods(mod_state);
                        return false;
                    }
                }
            }
            return true;
        case KC_RGHT:
            if (custom_shortcuts_enabled) {
                static bool rght_registered = false;
                static bool end_registered = false;

                if (record->event.pressed) {
                    if (mode_index == M_WINDOWS || mode_index == M_MACOS) {
                        bool mods_cond_shct2 = ((get_mods() & MOD_MASK_ALT) && !(get_mods() & MOD_MASK_CTRL) && !(get_mods() & MOD_MASK_GUI));
                        bool mods_cond_shct4 = ((get_mods() & MOD_MASK_CTRL) && !(get_mods() & MOD_MASK_ALT) && !(get_mods() & MOD_MASK_GUI));

                        if (mods_cond_shct2) {
                            dprint("shortcut 2 registered\n");

                            // remove ALT and register KC_RGHT with KC_LCTRL
                            set_mods((mod_state & ~MOD_MASK_ALT) | MOD_BIT_LCTRL);
                            register_code(KC_RGHT);
                            rght_registered = true;

                            return false;
                        } else if (mods_cond_shct4) {
                            dprint("shortcut 4 registered\n");

                            // remove mods and register KC_END
                            clear_mods();
                            register_code(KC_END);
                            end_registered = true;

                            return false;
                        }
                    }
                } else {
                    if (rght_registered) {
                        // unregister KC_RGHT and reapply mod state
                        unregister_code(KC_RGHT);
                        rght_registered = false;
                        set_mods(mod_state);
                        return false;
                    }
                    if (end_registered) {
                        // unregister KC_END and reapply mod state
                        unregister_code(KC_END);
                        end_registered = false;
                        set_mods(mod_state);
                        return false;
                    }
                }
            }
            return true;
        case TG_SHCT:
            custom_shortcuts_enabled = !custom_shortcuts_enabled;
            return false;
        case SPAM:
            // activate spam config
            if (record->event.pressed) {
                spam_config_active = true;
                dprint("SPAM CONFIG ACTIVE\n");
            }
            return false;
        case CYC_MD:
            if (record->event.pressed) {
                mode_index++;
                if (mode_index > CYCLE_MODE_END) {
                    mode_index = CYCLE_MODE_START;
                }

                // write mode_index to EEPROM
                user_config.mode_index = mode_index;
                eeconfig_update_user(user_config.raw);
            }
            return false;
        case CYC_LT:
            if (record->event.pressed) {
                default_layer_set(1 << layout_index);
                dprintf("layer turned OFF: %u\n", layout_index);

                layout_index++;
                if (layout_index > CYCLE_LAYOUT_END) {
                    layout_index = CYCLE_LAYOUT_START;
                }

                // default_layer_set(1 << layout_index);
                set_single_persistent_default_layer(layout_index);
                dprintf("layer turned ON: %u\n", layout_index);

                // write layout_index to EEPROM
                user_config.layout_index = layout_index;
                eeconfig_update_user(user_config.raw);
            }
            return false;
        case U_T_AUTO:
            if (record->event.pressed && (get_mods() & MOD_MASK_SHIFT) && (get_mods() & MOD_MASK_CTRL)) {
                TOGGLE_FLAG_AND_PRINT(usb_extra_manual, "USB extra port manual mode");
            }
            return false;
        case U_T_AGCR:
            if (record->event.pressed && (get_mods() & MOD_MASK_SHIFT) && (get_mods() & MOD_MASK_CTRL)) {
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
            spam_timer = timer_read32();
        }
    }
}
