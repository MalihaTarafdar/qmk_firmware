#include QMK_KEYBOARD_H
#include "lib/lib8tion/lib8tion.h"
#include "lib/lib8tion/math8.h"
#include "lib/lib8tion/random8.h"
#include "lib/lib8tion/scale8.h"
#include "lib/lib8tion/trig8.h"

// NOTE: must toggle function layer instead of momentary activation to start recording dynamic macros
// NOTE: key overrides must be enabled for MS_CLK action

#define LCG(kc) (QK_LCTL | QK_LGUI | (kc))

uint8_t rgb_debug_led = 0;
uint32_t rgb_timer;
bool rgb_disabled;

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
    DBG_TST,                  // programmable debug test key
    LC_CYC,                   // cycle layer color indicator mods
    PLY_SNK,                  // start snake game
    SNK_UP,                   // move snake up
    SNK_DOWN,                 // move snake down
    SNK_LEFT,                 // move snake left
    SNK_RGHT                  // move snake right
};

enum layer_names {
    _MAIN,    // main layer (QWERTY layout)
    _DVK,     // Dvorak layout
    _CMK,     // Colemak layout
    _FN,      // function layer
    _MS,      // mouse keys
    _RGB,     // RGB
    _SNK      // snake
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
 *     - KC_LGUI: send KC_RALT instead (note KC_RALT)
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

enum layer_color_modes {
    LC_NONE,
    LC_RGB,
    LC_MONOCHROMATIC,
    LC_DICHROMATIC
};

const uint8_t CYCLE_LC_MODE_START = 0;
const uint8_t CYCLE_LC_MODE_END = 3;
uint8_t lc_mode_index;
const RGB RGB_MONOCHROMATIC = SET_RGB(RGB_KEYS_R, RGB_KEYS_G, RGB_KEYS_B);
const RGB RGB_DICHROMATIC = SET_RGB(RGB_PRESS_R, RGB_PRESS_G, RGB_PRESS_B);

bool blink;
uint32_t blink_timer;

/* Custom Shortcuts
 * [NO] trigger -> replacement : description
 *
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
 *     [17] GUI + ALT + LEFT -> GUI + LEFT : snap left
 *     [18] GUI + ALT + RGHT -> GUI + RGHT : snap right
 *     [19] GUI + ALT + UP -> GUI + UP : maximize window
 *     [20] GUI + ALT + DOWN -> GUI + DOWN : minimize window
 *     [21] GUI + SFT + ALT + UP -> GUI + SFT + UP : stretch window vertical
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
const key_override_t shct17_override = {
        .trigger_mods                           = MOD_MASK_AG,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_AG,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_CS,
        .custom_action                          = NULL,
        .context                                = NULL,
        .trigger                                = KC_LEFT,
        .replacement                            = LGUI(KC_LEFT),
        .enabled                                = &mode_is_windows
};
const key_override_t shct18_override = {
        .trigger_mods                           = MOD_MASK_AG,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_AG,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_CS,
        .custom_action                          = NULL,
        .context                                = NULL,
        .trigger                                = KC_RGHT,
        .replacement                            = LGUI(KC_RGHT),
        .enabled                                = &mode_is_windows
};
const key_override_t shct19_override = {
        .trigger_mods                           = MOD_MASK_AG,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_AG,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_CS,
        .custom_action                          = NULL,
        .context                                = NULL,
        .trigger                                = KC_UP,
        .replacement                            = LGUI(KC_UP),
        .enabled                                = &mode_is_windows
};
const key_override_t shct20_override = {
        .trigger_mods                           = MOD_MASK_AG,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_AG,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_CS,
        .custom_action                          = NULL,
        .context                                = NULL,
        .trigger                                = KC_DOWN,
        .replacement                            = LGUI(KC_DOWN),
        .enabled                                = &mode_is_windows
};
const key_override_t shct21_override = {
        .trigger_mods                           = MOD_MASK_SAG,
        .layers                                 = ~0, // all layers
        .suppressed_mods                        = MOD_MASK_SAG,
        .options                                = ko_options_default,
        .negative_mod_mask                      = MOD_MASK_CTRL,
        .custom_action                          = NULL,
        .context                                = NULL,
        .trigger                                = KC_UP,
        .replacement                            = LGUI(KC_UP),
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
    &shct17_override,
    &shct18_override,
    &shct19_override,
    &shct20_override,
    &shct21_override,
    &ms_override
};

typedef union {
  uint32_t raw;
  struct {
    uint8_t mode_index :2;
    uint8_t layout_index :2;
    uint8_t lc_mode_index :2;
  };
} user_config_t;
user_config_t user_config;

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
        _______, KC_F13,  KC_F14,  KC_F15,  KC_F16,  KC_F17,  KC_F18,  KC_F19,  _______, _______, PLY_SNK, KC_BRID, KC_BRIU, KC_PSCR, KC_END,
        _______, LC_CYC,  DM_SPAM, DB_TOGG, DM_REC1, DM_REC2, DM_PLY1, DM_PLY2, KO_TOGG, CYC_LT,  KC_SCRL, KC_PAUS,          _______, CYC_MD,
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
        _______, RGB_TOG, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, _______,          _______, XXXXXXX,
        _______, _______, _______,                            XXXXXXX,                            _______, _______, XXXXXXX, XXXXXXX, XXXXXXX
    ),
    [_SNK] = LAYOUT_65_ansi_blocker(
        KC_ESC, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,          XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,          SNK_UP,  XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX,                            XXXXXXX,                            XXXXXXX, XXXXXXX, SNK_LEFT,SNK_DOWN,SNK_RGHT
    )
};

// ======================================================= SNAKE =======================================================

typedef struct Position {
    uint8_t r;
    uint8_t c;
} pos_t;

enum direction {
    UP,
    RIGHT,
    DOWN,
    LEFT,
};

const pos_t SNAKE_START_POS = {.r = 2, .c = 4};
const int SNAKE_START_DIR = RIGHT;
const uint8_t SNAKE_START_LEN = 2;

pos_t snake[MAX_SNAKE_LENGTH];
int snake_dir;
uint8_t snake_len;
pos_t apple;
bool snake_initialized;

uint32_t snake_timer;

bool is_in_bounds(uint8_t r, uint8_t c) {
    return r >= 0 && c >= 0 && r < MATRIX_ROWS && c < MATRIX_COLS;
}

pos_t get_random_pos(void) {
    uint8_t r = random8_max(MATRIX_ROWS);
    uint8_t c = random8_max(MATRIX_COLS);
    uint8_t led[LED_HITS_TO_REMEMBER];
    uint8_t led_count = rgb_matrix_map_row_column_to_led(r, c, led);

    while (led_count == 0) {
        r = random8_max(MATRIX_ROWS);
        c = random8_max(MATRIX_COLS);
        led_count = rgb_matrix_map_row_column_to_led(r, c, led);
    }

    pos_t pos = {.r = r, .c = c};
    return pos;
}

pos_t get_valid_pos(uint8_t r, uint8_t c, int dir) {
    if (is_in_bounds(r, c)) {
        const uint8_t mid = MATRIX_COLS / 2;
        uint8_t led[LED_HITS_TO_REMEMBER];
        uint8_t led_count = rgb_matrix_map_row_column_to_led(r, c, led);

        while (led_count == 0) {
            switch (dir) {
                case UP:
                case DOWN:
                    if (c <= mid) c++;
                    if (c > mid) c--;
                    break;
                case RIGHT:
                    c++;
                    break;
                case LEFT:
                    c--;
                    break;
                default:
                    break;
            }
            led_count = rgb_matrix_map_row_column_to_led(r, c, led);
        }
    }

    pos_t pos = {.r = r, .c = c};
    return pos;
}

void change_snake_dir(int dir) {
    snake_dir = dir;
}

// returns -1 if collision, 1 if apple, 0 otherwise
int8_t move_snake(void) {
    pos_t next_pos = {0};

    switch (snake_dir) {
        case UP:
            next_pos = get_valid_pos(snake[0].r - 1, snake[0].c, UP);
            break;
        case RIGHT:
            next_pos = get_valid_pos(snake[0].r, snake[0].c + 1, RIGHT);
            break;
        case DOWN:
            next_pos = get_valid_pos(snake[0].r + 1, snake[0].c, DOWN);
            break;
        case LEFT:
            next_pos = get_valid_pos(snake[0].r, snake[0].c - 1, LEFT);
            break;
        default:
            break;
    }

    // check snake collision
    bool snake_collision = false;

    // check out of bounds
    // TODO: implement wrap mode
    if (!is_in_bounds(next_pos.r, next_pos.c) || snake_collision) {
        return -1;
    }

    // check apple
    if (next_pos.r == apple.r && next_pos.c == apple.c) {
        return 1;
    }

    // move snake
    for (uint8_t s = 1; s < snake_len; s++) {
        snake[s].r = snake[s - 1].r;
        snake[s].c = snake[s - 1].c;
    }
    snake[0].r = next_pos.r;
    snake[0].c = next_pos.c;

    return 0;
}

void snake_init(void) {
    pos_t s0 = {.r = 2, .c = 5};
    snake[0] = s0;

    pos_t s1 = {.r = 2, .c = 4};
    snake[1] = s1;

    snake_dir = SNAKE_START_DIR;
    snake_len = SNAKE_START_LEN;
    apple = get_random_pos();

    snake_timer = timer_read32();

    snake_initialized = true;

    dprintf("snake[0].r: %u, snake[0].c: %u\n", snake[0].r, snake[0].c);
    dprintf("apple.r: %u, apple.c: %u\n", apple.r, apple.c);
    dprintf("snake_dir: %u, snake_len: %u\n", snake_dir, snake_len);
}

// =====================================================================================================================

void keyboard_blink(void) {
    blink = true;
    blink_timer = timer_read32();
}

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

void blink_init(void) {
    blink_timer = 0;
    blink = false;
}

void rgb_timeout_init(void) {
    rgb_disabled = false;
    rgb_timer = timer_read32();
}

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

    lc_mode_index = LC_MONOCHROMATIC;

    spam_init();
    rgb_timeout_init();
    blink_init();

    snake_initialized = false;
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

        lc_mode_index = user_config.lc_mode_index;
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

bool dynamic_macro_record_start_user(int8_t direction) {
    keyboard_blink();
    return true;
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

    if (record->event.pressed) {
        if (rgb_disabled) rgb_matrix_enable_noeeprom();
        rgb_timeout_init();
    }

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
        case KC_ESC:
            // deactivate spam config & spam
            if (record->event.pressed) {
                if (spam_config_active || spam_active) {
                    spam_init();
                    dprintf("spam_config_active: %u\n", spam_config_active);
                    return false;
                }
                if (snake_initialized) {
                    snake_initialized = false;
                    dprint("snake de-initialized\n");
                    layer_off(_SNK);
                    return false;
                }
            }
            return true;
        case PLY_SNK:
            if (record->event.pressed) {
                if (!snake_initialized) {
                    snake_init();
                }
                layer_on(_SNK);
            }
            return false;
        case SNK_UP:
            change_snake_dir(UP);
            return false;
        case SNK_RGHT:
            change_snake_dir(RIGHT);
            return false;
        case SNK_DOWN:
            change_snake_dir(DOWN);
            return false;
        case SNK_LEFT:
            change_snake_dir(LEFT);
            return false;
        case LC_CYC:
            if (record->event.pressed) {
                lc_mode_index++;
                if (lc_mode_index > CYCLE_LC_MODE_END) {
                    lc_mode_index = CYCLE_LC_MODE_START;
                }
                // write lc_mode_index to EEPROM
                user_config.lc_mode_index = lc_mode_index;
                eeconfig_update_user(user_config.raw);
                dprintf("lc_mode_index [EEPROM]: %u\n", user_config.lc_mode_index);
            }
            return false;
        case DBG_TST:
            if (record->event.pressed) {
                rgb_debug_led++;
                dprintf("DEBUG LED: %u\n", rgb_debug_led);
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

                set_single_persistent_default_layer(layout_index);
                dprintf("layer turned on: %u\n", layout_index);

                // write layout_index to EEPROM
                user_config.layout_index = layout_index;
                eeconfig_update_user(user_config.raw);
                dprintf("layout_index [EEPROM]: %u\n", user_config.layout_index);
            }
            return false;
        case DM_SPAM:
            // activate spam config
            if (record->event.pressed) {
                spam_config_active = !spam_config_active;
                keyboard_blink();
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
            if (mode_index == M_LINUX || mode_index == M_MACOS) {
                // send KC_LALT instead of KC_LGUI
                if (record->event.pressed) {
                    register_code(KC_LALT);
                } else {
                    unregister_code(KC_LALT);
                }
                return false;
            }
            if (mode_index == M_WINDOWS) {
                // send KC_RALT instead of KC_LGUI
                if (record->event.pressed) {
                    register_code(KC_RALT);
                } else {
                    unregister_code(KC_RALT);
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

RGB get_mono_di_chromatic_rgb(uint8_t level) {
    const uint8_t MAX_LEVELS = 4;
    const uint8_t END = 255;
    const uint8_t RANGE = END;
    HSV hsv = rgb_to_hsv(RGB_MONOCHROMATIC);

    hsv.v = (level > 0) ? END - (RANGE / MAX_LEVELS) * (level - 1) : 0;

    // invert & scale (cubic)
    hsv.v = (hsv.v * hsv.v * hsv.v) / (255 * 255);

    if (lc_mode_index == LC_DICHROMATIC) {
        if (level == 1) {
            hsv = rgb_to_hsv(RGB_DICHROMATIC);
        } else if (hsv.v > 0) {
            hsv.v = 255;
        }
    }

    return hsv_to_rgb(hsv);
}

RGB get_layer_indicator_rgb(uint8_t layer, uint8_t index, uint16_t kc) {
    switch (layer) {
        case _FN:
            if ((kc >= KC_F1 && kc <= KC_F12) || (kc >= KC_F13 && kc <= KC_F19) || kc == KC_DEL ||
                    kc == KC_INS || kc == KC_PSCR || kc == KC_END || kc == KC_SCRL || kc == KC_PAUS) {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_KEYS_R, RGB_KEYS_G, RGB_KEYS_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(1);
                }
            } else if (kc == KC_BRID || kc == KC_BRIU || kc == KC_MUTE || kc == KC_VOLD ||
                    kc == KC_VOLU || kc == KC_MPRV || kc == KC_MPLY || kc == KC_MNXT) {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_PRESS_R, RGB_PRESS_G, RGB_PRESS_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(1);
                }
            } else if (kc == DM_SPAM || kc == DM_REC1 || kc == DM_REC2 || kc == DM_PLY1 ||
                    kc == DM_PLY2) {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_PRESS_R, RGB_PRESS_G, RGB_PRESS_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(3);
                }
            } else if (kc == EE_CLR || kc == U_T_AUTO || kc == U_T_AGCR || kc == MD_BOOT) {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_CUSTOM_ORANGE_R, RGB_CUSTOM_ORANGE_G, RGB_CUSTOM_ORANGE_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(3);
                }
            } else if (kc == TT(_RGB) || kc == TG(_MS) || kc == MS_CLK || index == 63) {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_CUSTOM_RED_R, RGB_CUSTOM_RED_G, RGB_CUSTOM_RED_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(1);
                }
            } else if (kc == CYC_LT || kc == CYC_MD || kc == DBG_TST || kc == LC_CYC || kc == PLY_SNK) {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_CUSTOM_PURPLE_R, RGB_CUSTOM_PURPLE_G, RGB_CUSTOM_PURPLE_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(2);
                }
            } else if (kc == DB_TOGG || kc == NK_TOGG || kc == KO_TOGG) {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_CUSTOM_GREEN_R, RGB_CUSTOM_GREEN_G, RGB_CUSTOM_GREEN_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(4);
                }
            } else {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_KEYS_R, RGB_KEYS_G, RGB_KEYS_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(0);
                }
            }
            break;
        case _MS:
            if (kc == MS_UP || kc == MS_DOWN || kc == MS_LEFT || kc == MS_RGHT || kc == MS_WHLU ||
                    kc == MS_WHLD || kc == MS_WHLL || kc == MS_WHLR) {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_PRESS_R, RGB_PRESS_G, RGB_PRESS_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(1);
                }
            } else if (kc == MS_BTN1 || kc == MS_BTN2 || kc == MS_BTN3 || kc == MS_BTN4 || kc == MS_BTN5) {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_CUSTOM_ORANGE_R, RGB_CUSTOM_ORANGE_G, RGB_CUSTOM_ORANGE_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(2);
                }
            } else if (kc == MS_ACL0 || kc == MS_ACL1 || kc == MS_ACL2) {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_CUSTOM_GREEN_R, RGB_CUSTOM_GREEN_G, RGB_CUSTOM_GREEN_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(3);
                }
            } else if (kc == KC_SPC || kc == KC_LEFT || kc == KC_RGHT || kc == KC_UP || kc == KC_DOWN) {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_CUSTOM_PURPLE_R, RGB_CUSTOM_PURPLE_G, RGB_CUSTOM_PURPLE_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(3);
                }
            } else if (kc == TG(_MS) || index == 51) {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_CUSTOM_RED_R, RGB_CUSTOM_RED_G, RGB_CUSTOM_RED_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(1);
                }
            } else {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_KEYS_R, RGB_KEYS_G, RGB_KEYS_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(0);
                }
            }
            break;
        case _RGB:
            if (kc == TG(_RGB) || index == 63 || index == 56 || kc == RGB_TOG) {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_CUSTOM_RED_R, RGB_CUSTOM_RED_G, RGB_CUSTOM_RED_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(1);
                }
            } else if (kc == RGB_SPD || kc == RGB_SPI) {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_CUSTOM_ORANGE_R, RGB_CUSTOM_ORANGE_G, RGB_CUSTOM_ORANGE_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(3);
                }
            } else if (kc == RGB_RMOD || kc == RGB_MOD) {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_CUSTOM_YELLOW_R, RGB_CUSTOM_YELLOW_G, RGB_CUSTOM_YELLOW_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(3);
                }
            } else if (kc == RGB_VAI || kc == RGB_VAD) {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_CUSTOM_GREEN_R, RGB_CUSTOM_GREEN_G, RGB_CUSTOM_GREEN_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(2);
                }
            } else if (kc == RGB_HUI || kc == RGB_HUD) {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_PRESS_R, RGB_PRESS_G, RGB_PRESS_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(2);
                }
            } else if (kc == RGB_SAI || kc == RGB_SAD) {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_CUSTOM_PURPLE_R, RGB_CUSTOM_PURPLE_G, RGB_CUSTOM_PURPLE_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(3);
                }
            } else {
                if (lc_mode_index == LC_RGB) {
                    RGB rgb = SET_RGB(RGB_KEYS_R, RGB_KEYS_G, RGB_KEYS_B);
                    return rgb;
                } else if (lc_mode_index == LC_MONOCHROMATIC || LC_DICHROMATIC) {
                    return get_mono_di_chromatic_rgb(0);
                }
            }
            break;
    }

    RGB rgb = SET_RGB(0, 0, 0);
    return rgb;
}

bool rgb_matrix_indicators_user() {
    if (blink) {
        // blink LED strip only
        for (int i = STRIP_START; i < RGB_MATRIX_LED_COUNT; i++) {
            rgb_matrix_set_color(i, RGB_CUSTOM_ORANGE_R, RGB_CUSTOM_ORANGE_G, RGB_CUSTOM_ORANGE_B);
        }
    }
    return true;
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    uint8_t layer = get_highest_layer(layer_state);
    switch (layer) {
        case _FN:
            if (lc_mode_index == LC_NONE) return true;
            for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
                for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
                    uint8_t index = g_led_config.matrix_co[row][col];
                    uint16_t kc = keymap_key_to_keycode(layer, (keypos_t){col,row});

                    if (index >= led_min && index < led_max && index != NO_LED) {
                        RGB rgb = get_layer_indicator_rgb(layer, index, kc);
                        HSV hsv = rgb_to_hsv(rgb);
                        hsv.v = (lc_mode_index == LC_RGB) ? rgb_matrix_config.hsv.v : scale8(hsv.v, rgb_matrix_config.hsv.v);
                        RGB rgb_f = hsv_to_rgb(hsv);
                        rgb_matrix_set_color(index, rgb_f.r, rgb_f.g, rgb_f.b);
                    }
                }
            }
            break;
        case _MS:
            if (lc_mode_index == LC_NONE) return true;
            for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
                for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
                    uint8_t index = g_led_config.matrix_co[row][col];
                    uint16_t kc = keymap_key_to_keycode(layer, (keypos_t){col,row});

                    if (index >= led_min && index < led_max && index != NO_LED) {
                        RGB rgb = get_layer_indicator_rgb(layer, index, kc);
                        HSV hsv = rgb_to_hsv(rgb);
                        hsv.v = (lc_mode_index == LC_RGB) ? rgb_matrix_config.hsv.v : scale8(hsv.v, rgb_matrix_config.hsv.v);
                        rgb = hsv_to_rgb(hsv);
                        rgb_matrix_set_color(index, rgb.r, rgb.g, rgb.b);
                    }
                }
            }
            break;
        case _RGB:
            if (lc_mode_index == LC_NONE) return true;
            for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
                for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
                    uint8_t index = g_led_config.matrix_co[row][col];
                    uint16_t kc = keymap_key_to_keycode(layer, (keypos_t){col,row});

                    if (index >= led_min && index < led_max && index != NO_LED) {
                        RGB rgb = get_layer_indicator_rgb(layer, index, kc);
                        HSV hsv = rgb_to_hsv(rgb);
                        hsv.v = (lc_mode_index == LC_RGB) ? rgb_matrix_config.hsv.v : scale8(hsv.v, rgb_matrix_config.hsv.v);
                        rgb = hsv_to_rgb(hsv);
                        rgb_matrix_set_color(index, rgb.r, rgb.g, rgb.b);
                    }
                }
            }
            break;
        case _SNK:
            for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
                for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
                    uint8_t index = g_led_config.matrix_co[row][col];

                    if (index >= led_min && index < led_max && index != NO_LED) {
                        rgb_matrix_set_color(index, 0, 0, 0);

                        // render snake
                        for (uint8_t s = 0; s < snake_len; s++) {
                            if (snake[s].r == row && snake[s].c == col) {
                                rgb_matrix_set_color(index, RGB_CUSTOM_GREEN_R, RGB_CUSTOM_GREEN_G, RGB_CUSTOM_GREEN_B);
                            }
                        }

                        // render apple
                        if (apple.r == row && apple.c == col) {
                            rgb_matrix_set_color(index, RGB_CUSTOM_RED_R, RGB_CUSTOM_RED_G, RGB_CUSTOM_RED_B);
                        }
                    }
                }
            }

            // LED strip
            for (int i = STRIP_START; i < RGB_MATRIX_LED_COUNT; i++) {
                rgb_matrix_set_color(i, RGB_CUSTOM_GREEN_R, RGB_CUSTOM_GREEN_G, RGB_CUSTOM_GREEN_B);
            }
            break;
        default:
            break;
    }
    return true;
}

void matrix_scan_user() {
    if (snake_initialized) {
        if (timer_elapsed32(snake_timer) > SNAKE_FRAME_DELAY) {
            // move snake
            move_snake();
            // TODO: eat apple
            // TODO: check lose cond

            snake_timer = timer_read32();
        }
    }
    if (spam_active) {
        if (timer_elapsed32(spam_timer) > SPAM_DELAY) {
            tap_code(spam_keycode);
            spam_timer = timer_read32();
        }
    }
    if (!rgb_disabled) {
        if (timer_elapsed32(rgb_timer) > CUSTOM_RGB_MATRIX_TIMEOUT) {
            rgb_matrix_disable_noeeprom();
            rgb_disabled = true;
        }
    }
    if (timer_elapsed32(blink_timer) > BLINK_DELAY) {
        blink_timer = 0;
        blink = false;
    }
}
