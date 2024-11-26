#include QMK_KEYBOARD_H

// NOTE: must toggle function instead of momentary activation to start recording dynamic macros

#define LCG(kc) (QK_LCTL | QK_LGUI | (kc))

uint8_t rgb_debug_led = 0;

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
    DM_SPAM,                  // spam key custom dynamic macro
    MS_CLK,                   // mouse keys momentary layer + click on release
    DBG_TST                   // programmable debug test key
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
const uint8_t CYCLE_MODE_START = 0;
const uint8_t CYCLE_MODE_END = 2;
uint8_t mode_index;

const uint8_t CYCLE_LAYOUT_START = 0;
const uint8_t CYCLE_LAYOUT_END = 2;
uint8_t layout_index;

bool spam_config_active;
bool spam_active;
uint16_t spam_keycode;
uint32_t spam_timer;

/* Custom Shortcuts
 * Linux/Windows Mode
 * For the following shortcuts, only SFT may also be active.
 *     [1] ALT + LEFT -> CTL + LEFT : move word left
 *     [2] ALT + RGHT -> CTL + RGHT : move word right
 *     [3] CTL + LEFT -> HOME : move to beginning of line
 *     [4] CTL + RGHT -> END : move to end of line
 *     [5] ALT + UP -> CTL + UP : move to beginning of paragraph
 *     [6] ALT + DOWN -> CTL + DOWN : move to end of paragraph
 *     [7] CTL + UP -> CTL + HOME : move to beginning of document
 *     [8] CTL + DOWN -> CTL + END : move to end of document
 *     [9] ALT + BSPC -> CTL + BSPC : delete word left
 *     [10] ALT + DEL -> CTL + DEL : delete word right
 *     [11] CTL + BSPC -> SFT + HOME, BSPC : delete to beginning of line
 *     [12] CTL + DEL -> SFT + END, BSPC : delete to end of line
 *
 * Windows Mode
 *     [14] GUI + LEFT -> GUI + CTL + LEFT : move desktop left
 *     [15] GUI + RGHT -> GUI + CTL + RGHT : move desktop right
 *     [16] GUI + UP -> GUI + TAB : launch task view
 *
 * MacOS Mode
 *     [13] GUI + DEL -> SFT + END, BSPC : delete to end of line
*/

bool mode_is_linux_or_windows = false;
bool mode_is_macos = false;
bool mode_is_windows = false;

bool shct11_action(bool activated, void *context) {
    if (activated) {
        register_code(KC_LSFT);
        register_code(KC_HOME);
        unregister_code(KC_HOME);
        unregister_code(KC_LSFT);
        tap_code(KC_BSPC);
    }
    return false;
}

bool shct12_13_action(bool activated, void *context) {
    if (activated) {
        register_code(KC_LSFT);
        register_code(KC_END);
        unregister_code(KC_END);
        unregister_code(KC_LSFT);
        tap_code(KC_BSPC);
    }
    return false;
}

const key_override_t shct1_override = {
        .trigger_mods                           = MOD_MASK_ALT,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_ALT,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_CG,
        .custom_action                          = NULL,
        .context                                = NULL,
        .trigger                                = KC_LEFT,
        .replacement                            = LCTL(KC_LEFT),
        .enabled                                = &mode_is_linux_or_windows
};
const key_override_t shct2_override = {
        .trigger_mods                           = MOD_MASK_ALT,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_ALT,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_CG,
        .custom_action                          = NULL,
        .context                                = NULL,
        .trigger                                = KC_RGHT,
        .replacement                            = LCTL(KC_RGHT),
        .enabled                                = &mode_is_linux_or_windows
};
const key_override_t shct3_override = {
        .trigger_mods                           = MOD_MASK_CTRL,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_CTRL,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_AG,
        .custom_action                          = NULL,
        .context                                = NULL,
        .trigger                                = KC_LEFT,
        .replacement                            = KC_HOME,
        .enabled                                = &mode_is_linux_or_windows
};
const key_override_t shct4_override = {
        .trigger_mods                           = MOD_MASK_CTRL,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_CTRL,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_AG,
        .custom_action                          = NULL,
        .context                                = NULL,
        .trigger                                = KC_RGHT,
        .replacement                            = KC_END,
        .enabled                                = &mode_is_linux_or_windows
};
const key_override_t shct5_override = {
        .trigger_mods                           = MOD_MASK_ALT,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_ALT,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_CG,
        .custom_action                          = NULL,
        .context                                = NULL,
        .trigger                                = KC_UP,
        .replacement                            = LCTL(KC_UP),
        .enabled                                = &mode_is_linux_or_windows
};
const key_override_t shct6_override = {
        .trigger_mods                           = MOD_MASK_ALT,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_ALT,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_CG,
        .custom_action                          = NULL,
        .context                                = NULL,
        .trigger                                = KC_DOWN,
        .replacement                            = LCTL(KC_DOWN),
        .enabled                                = &mode_is_linux_or_windows
};
const key_override_t shct7_override = {
        .trigger_mods                           = MOD_MASK_CTRL,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_CTRL,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_AG,
        .custom_action                          = NULL,
        .context                                = NULL,
        .trigger                                = KC_UP,
        .replacement                            = LCTL(KC_HOME),
        .enabled                                = &mode_is_linux_or_windows
};
const key_override_t shct8_override = {
        .trigger_mods                           = MOD_MASK_CTRL,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_CTRL,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_AG,
        .custom_action                          = NULL,
        .context                                = NULL,
        .trigger                                = KC_DOWN,
        .replacement                            = LCTL(KC_END),
        .enabled                                = &mode_is_linux_or_windows
};
const key_override_t shct9_override = {
        .trigger_mods                           = MOD_MASK_ALT,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_ALT,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_CSG,
        .custom_action                          = NULL,
        .context                                = NULL,
        .trigger                                = KC_BSPC,
        .replacement                            = LCTL(KC_BSPC),
        .enabled                                = &mode_is_linux_or_windows
};
const key_override_t shct10_override = {
        .trigger_mods                           = MOD_MASK_ALT,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_ALT,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_CSG,
        .custom_action                          = NULL,
        .context                                = NULL,
        .trigger                                = KC_DEL,
        .replacement                            = LCTL(KC_DEL),
        .enabled                                = &mode_is_linux_or_windows
};
const key_override_t shct11_override = {
        .trigger_mods                           = MOD_MASK_CTRL,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_CTRL,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_SAG,
        .custom_action                          = shct11_action,
        .context                                = NULL,
        .trigger                                = KC_BSPC,
        .replacement                            = KC_NO,
        .enabled                                = &mode_is_linux_or_windows
};
const key_override_t shct12_override = {
        .trigger_mods                           = MOD_MASK_CTRL,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_CTRL,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_SAG,
        .custom_action                          = shct12_13_action,
        .context                                = NULL,
        .trigger                                = KC_DEL,
        .replacement                            = KC_NO,
        .enabled                                = &mode_is_linux_or_windows
};
const key_override_t shct13_override = {
        .trigger_mods                           = MOD_MASK_GUI,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_GUI,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_CSA,
        .custom_action                          = shct12_13_action,
        .context                                = NULL,
        .trigger                                = KC_DEL,
        .replacement                            = KC_NO,
        .enabled                                = &mode_is_macos
};
const key_override_t shct14_override = {
        .trigger_mods                           = MOD_MASK_GUI,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_GUI,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_CSA,
        .custom_action                          = NULL,
        .context                                = NULL,
        .trigger                                = KC_LEFT,
        .replacement                            = LCG(KC_LEFT),
        .enabled                                = &mode_is_windows
};
const key_override_t shct15_override = {
        .trigger_mods                           = MOD_MASK_GUI,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_GUI,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_CSA,
        .custom_action                          = NULL,
        .context                                = NULL,
        .trigger                                = KC_RGHT,
        .replacement                            = LCG(KC_RGHT),
        .enabled                                = &mode_is_windows
};
const key_override_t shct16_override = {
        .trigger_mods                           = MOD_MASK_GUI,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_GUI,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_CSA,
        .custom_action                          = NULL,
        .context                                = NULL,
        .trigger                                = KC_UP,
        .replacement                            = LGUI(KC_TAB),
        .enabled                                = &mode_is_windows
};

bool ms_override_action(bool activated, void *context) {
    if (activated) {
        layer_on(_MS);
    } else {
        layer_off(_MS);
        if (get_mods() & MOD_BIT_LSHIFT) {
            unregister_mods(MOD_BIT_LSHIFT);
            return false;
        }
        if (!(get_mods() & MOD_BIT_RSHIFT)) {
            tap_code16(MS_BTN1);
        } else {
            unregister_mods(MOD_BIT_RSHIFT);
            tap_code16(MS_BTN2);
        }
    }
    return false;
}

const key_override_t ms_override = {
        .trigger_mods                           = 0,
        .layers                                 = ~(1 << _MS),
        .suppressed_mods                        = 0,
        .options                                = ko_option_no_unregister_on_other_key_down,
        .negative_mod_mask                      = 0,
        .custom_action                          = ms_override_action,
        .context                                = NULL,
        .trigger                                = MS_CLK,
        .replacement                            = KC_NO,
        .enabled                                = NULL
};

const key_override_t *key_overrides[] = {
    &shct1_override,
    &shct2_override,
    &shct3_override,
    &shct4_override,
    &shct5_override,
    &shct6_override,
    &shct7_override,
    &shct8_override,
    &shct9_override,
    &shct10_override,
    &shct11_override,
    &shct12_override,
    &shct13_override,
    &shct14_override,
    &shct15_override,
    &shct16_override,
    &ms_override
};

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
        _______, _______, DM_SPAM, DB_TOGG, DM_REC1, DM_REC2, DM_PLY1, DM_PLY2, KO_TOGG, CYC_LT,  KC_SCRL, KC_PAUS,          _______, CYC_MD,
        _______, EE_CLR, U_T_AUTO,U_T_AGCR, _______, MD_BOOT, NK_TOGG, TG(_MS), KC_MUTE, KC_VOLD, KC_VOLU, _______,          TT(_RGB),MS_CLK,
        _______, _______, _______,                            DBG_TST,                            _______, _______, KC_MPRV, KC_MPLY, KC_MNXT
    ),
    [_MS] = LAYOUT_65_ansi_blocker(
        TG(_MS), MS_ACL0, MS_ACL1, MS_ACL2, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, MS_ACL0, MS_ACL1, MS_ACL2, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, MS_UP,   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, MS_BTN4, MS_WHLU, MS_BTN5, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, MS_LEFT, MS_DOWN, MS_RGHT, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, MS_WHLL, MS_WHLD, MS_WHLR,          XXXXXXX, XXXXXXX,
        _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, TG(_MS), MS_BTN3, MS_BTN1, MS_BTN2, _______,          KC_UP,   _______,
        _______, _______, _______,                            KC_SPC,                             _______, _______, KC_LEFT, KC_DOWN, KC_RGHT
    ),
    [_RGB] = LAYOUT_65_ansi_blocker(
        TG(_RGB),XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, RGB_SPD, RGB_VAI, RGB_SPI, RGB_HUI, RGB_SAI, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, RGB_RMOD,RGB_VAD, RGB_MOD, RGB_HUD, RGB_SAD, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,          XXXXXXX, XXXXXXX,
        XXXXXXX, RGB_TOG, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,          _______, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX,                            XXXXXXX,                            XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
    )
};

void spam_init(void) {
    spam_active = false;
    spam_config_active = false;
    spam_timer = 0;
    spam_keycode = 0;
}

void init_user(void) {
    debug_enable = true; // REMOVE
    // Linux/Windows mode enabled by default
    mode_index = M_LINUX;
    mode_is_linux_or_windows = true;

    // QWERTY keyboard layout enabled by default
    layout_index = CYCLE_LAYOUT_START;

    spam_init();
}

void keyboard_post_init_user() {
    init_user();

    // init from EEPROM
    user_config.raw = eeconfig_read_user();
    if (user_config.raw != 0) {
        mode_index = user_config.mode_index;
        mode_is_linux_or_windows = (mode_index == M_LINUX || mode_index == M_WINDOWS);
        mode_is_macos = (mode_index == M_MACOS);
        mode_is_windows = (mode_index == M_WINDOWS);

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
    dprintf("current state:\t%s (%u), highest layer: %u\n", state_binary_str, state, get_highest_layer(state));
    free(state_binary_str);
    return state;
}

bool keycode_is_spamable(uint16_t keycode) {
    return keycode != KC_ESC && keycode != KO_TOGG && keycode != NK_TOGG && keycode != DB_TOGG &&
            keycode != TT(_FN) && keycode != TT(_RGB) && keycode != TG(_MS) &&
            keycode != CYC_MD && keycode != CYC_LT && keycode != DM_SPAM &&
            keycode != DM_REC1 && keycode != DM_REC2 && keycode != DM_PLY1 && keycode != DM_PLY2 &&
            keycode != MD_BOOT && keycode != EE_CLR && keycode != U_T_AUTO && keycode != U_T_AGCR;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    static uint32_t key_timer;

    // activate spam
    if (spam_config_active && keycode_is_spamable(keycode)) {
        if (record->event.pressed) {
            spam_config_active = false;
            spam_keycode = keycode;
            spam_timer = timer_read32();
            spam_active = true;
            dprintf("spam_active: 1, keycode: %u\n", keycode);
        }
        return false;
    }

    mod_state = get_mods();

    switch (keycode) {
        case DBG_TST:
            if (record->event.pressed) {
                rgb_debug_led++;
            }
            return false;
        case CYC_MD:
            if (record->event.pressed) {
                mode_index++;
                if (mode_index > CYCLE_MODE_END) {
                    mode_index = CYCLE_MODE_START;
                }
                mode_is_linux_or_windows = (mode_index == M_LINUX || mode_index == M_WINDOWS);
                mode_is_macos = (mode_index == M_MACOS);
                mode_is_windows = (mode_index == M_WINDOWS);

                // write mode_index to EEPROM
                user_config.mode_index = mode_index;
                eeconfig_update_user(user_config.raw);
                dprintf("mode_index [EEPROM]: %u\n", user_config.mode_index);
            }
            return false;
        case CYC_LT:
            if (record->event.pressed) {
                default_layer_set(1 << layout_index);
                dprintf("layer turned off: %u\n", layout_index);

                layout_index++;
                if (layout_index > CYCLE_LAYOUT_END) {
                    layout_index = CYCLE_LAYOUT_START;
                }

                // default_layer_set(1 << layout_index);
                set_single_persistent_default_layer(layout_index);
                dprintf("layer turned on: %u\n", layout_index);

                // write layout_index to EEPROM
                user_config.layout_index = layout_index;
                eeconfig_update_user(user_config.raw);
                dprintf("layout_index [EEPROM]: %u\n", user_config.layout_index);
            }
            return false;
        case KC_ESC:
            // deactivate spam config & spam
            if (record->event.pressed) {
                if (spam_config_active || spam_active) {
                    spam_init();
                    dprintf("spam_config_active: %u\n", spam_config_active);
                    return false;
                }
            }
            return true;
        case DM_SPAM:
            // activate spam config
            if (record->event.pressed) {
                spam_config_active = !spam_config_active;
                dprintf("spam_config_active: %u\n", spam_config_active);
            }
            return false;
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
                if (timer_elapsed32(key_timer) >= BOOT_DELAY) {
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

bool rgb_matrix_indicators_user() {
    // rgb_matrix_set_color(rgb_debug_led, 255, 255, 255);

    return true;
}

void matrix_scan_user() {
    if (spam_active) {
        if (timer_elapsed32(spam_timer) > SPAM_DELAY) {
            tap_code(spam_keycode);
            spam_timer = timer_read32();
        }
    }
}
