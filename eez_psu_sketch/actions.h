/*
* EEZ PSU Firmware
* Copyright (C) 2016-present, Envox d.o.o.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program.  If not, see http://www.gnu.org/licenses.
*/

#pragma once

namespace eez {
namespace psu {

enum ActionsEnum {
    ACTION_ID_NONE,
    ACTION_ID_TOGGLE_CHANNEL,
    ACTION_ID_SHOW_CHANNEL_SETTINGS,
    ACTION_ID_SHOW_MAIN_PAGE,
    ACTION_ID_EDIT,
    ACTION_ID_EDIT_MODE_SLIDER,
    ACTION_ID_EDIT_MODE_STEP,
    ACTION_ID_EDIT_MODE_KEYPAD,
    ACTION_ID_EXIT_EDIT_MODE,
    ACTION_ID_TOGGLE_INTERACTIVE_MODE,
    ACTION_ID_NON_INTERACTIVE_ENTER,
    ACTION_ID_NON_INTERACTIVE_DISCARD,
    ACTION_ID_KEYPAD_KEY,
    ACTION_ID_KEYPAD_SPACE,
    ACTION_ID_KEYPAD_BACK,
    ACTION_ID_KEYPAD_CLEAR,
    ACTION_ID_KEYPAD_CAPS,
    ACTION_ID_KEYPAD_OK,
    ACTION_ID_KEYPAD_CANCEL,
    ACTION_ID_KEYPAD_SIGN,
    ACTION_ID_KEYPAD_UNIT,
    ACTION_ID_KEYPAD_MAX,
    ACTION_ID_KEYPAD_DEF,
    ACTION_ID_TOUCH_SCREEN_CALIBRATION,
    ACTION_ID_YES,
    ACTION_ID_NO,
    ACTION_ID_OK,
    ACTION_ID_CANCEL,
    ACTION_ID_SHOW_PREVIOUS_PAGE,
    ACTION_ID_TURN_OFF,
    ACTION_ID_SHOW_SYS_SETTINGS,
    ACTION_ID_SHOW_MAIN_HELP_PAGE,
    ACTION_ID_SHOW_CH_SETTINGS_PROT,
    ACTION_ID_SHOW_CH_SETTINGS_PROT_CLEAR,
    ACTION_ID_SHOW_CH_SETTINGS_PROT_OCP,
    ACTION_ID_SHOW_CH_SETTINGS_PROT_OVP,
    ACTION_ID_SHOW_CH_SETTINGS_PROT_OPP,
    ACTION_ID_SHOW_CH_SETTINGS_PROT_OTP,
    ACTION_ID_SHOW_CH_SETTINGS_ADV,
    ACTION_ID_SHOW_CH_SETTINGS_ADV_LRIPPLE,
    ACTION_ID_SHOW_CH_SETTINGS_ADV_LIMITS,
    ACTION_ID_SHOW_CH_SETTINGS_ADV_RSENSE,
    ACTION_ID_SHOW_CH_SETTINGS_ADV_RPROG,
    ACTION_ID_SHOW_CH_SETTINGS_DISP,
    ACTION_ID_SHOW_CH_SETTINGS_INFO,
    ACTION_ID_SHOW_SYS_SETTINGS_CAL,
    ACTION_ID_SHOW_SYS_SETTINGS_CAL_CH,
    ACTION_ID_SYS_SETTINGS_CAL_EDIT_PASSWORD,
    ACTION_ID_SYS_SETTINGS_CAL_CH_PARAMS_ENABLED,
    ACTION_ID_SYS_SETTINGS_CAL_CH_WIZ_START,
    ACTION_ID_SYS_SETTINGS_CAL_CH_WIZ_STEP_PREVIOUS,
    ACTION_ID_SYS_SETTINGS_CAL_CH_WIZ_STEP_NEXT,
    ACTION_ID_SYS_SETTINGS_CAL_CH_WIZ_STOP_AND_SHOW_PREVIOUS_PAGE,
    ACTION_ID_SYS_SETTINGS_CAL_CH_WIZ_STOP_AND_SHOW_MAIN_PAGE,
    ACTION_ID_SYS_SETTINGS_CAL_CH_WIZ_STEP_SET,
    ACTION_ID_SYS_SETTINGS_CAL_CH_WIZ_SAVE,
    ACTION_ID_SYS_SETTINGS_CAL_TOGGLE_ENABLE,
    ACTION_ID_CH_SETTINGS_PROT_CLEAR,
    ACTION_ID_CH_SETTINGS_PROT_CLEAR_AND_DISABLE,
    ACTION_ID_CH_SETTINGS_PROT_TOGGLE_STATE,
    ACTION_ID_CH_SETTINGS_PROT_EDIT_LEVEL,
    ACTION_ID_CH_SETTINGS_PROT_EDIT_DELAY,
    ACTION_ID_CH_SETTINGS_PROT_SET,
    ACTION_ID_CH_SETTINGS_PROT_DISCARD
};

typedef void (*ACTION)();

extern ACTION actions[];

}
} // namespace eez::psu::gui
