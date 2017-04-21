/*
 * EEZ PSU Firmware
 * Copyright (C) 2015 Envox d.o.o.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
/** @file conf_user.h
@brief Configuration overrided by the user.
Use this header file to override anything from conf.h or conf_advanced.h.
*/

#pragma once

#define ETHERNET_MAC_ADDRESS { 0x74, 0x69, 0x69, 0x2D, 0x30, 0x00 }

/* Redefine channels
#undef CHANNELS
#define CHANNELS \
    CHANNEL(1, CH_BOARD_REVISION_R5B9_PARAMS, CH_PINS_1, CH_PARAMS_40V_5A_R5B9), \
    CHANNEL(2, CH_BOARD_REVISION_R5B9_PARAMS, CH_PINS_2, CH_PARAMS_40V_5A_R5B9) \
*/

// Select Arduino shield r3B4
//#undef EEZ_PSU_SELECTED_REVISION
//#define EEZ_PSU_SELECTED_REVISION EEZ_PSU_REVISION_R3B4

// Enable TFT display with SDD1289 controller
//#undef DISPLAY_TYPE
//#define DISPLAY_TYPE TFT_320QVT_1289

// Disable encoder
//#undef OPTION_ENCODER
//#define OPTION_ENCODER 0

// Disable SD card
//#undef OPTION_SD_CARD
//#define OPTION_SD_CARD 0
