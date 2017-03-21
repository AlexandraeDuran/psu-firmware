/*
 * EEZ PSU Firmware
 * Copyright (C) 2015-present, Envox d.o.o.
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
 
#include "psu.h"
#include "scpi_psu.h"
#include "temp_sensor.h"
#include "channel_dispatcher.h"

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

static scpi_choice_def_t channel_choice[] = {
    { "CH1", 1 },
    { "CH2", 2 },
    SCPI_CHOICE_LIST_END /* termination of option list */
};

#define TEMP_SENSOR(NAME, INSTALLED, PIN, CAL_POINTS, CH_NUM, QUES_REG_BIT, SCPI_ERROR) { #NAME, temp_sensor::NAME }

scpi_choice_def_t temp_sensor_choice[] = {
	TEMP_SENSORS,
    SCPI_CHOICE_LIST_END /* termination of option list */
};

#undef TEMP_SENSOR

scpi_choice_def_t internal_external_choice[] = {
    { "INTernal", 0 },
    { "EXTernal", 1 },
    SCPI_CHOICE_LIST_END /* termination of option list */
};

////////////////////////////////////////////////////////////////////////////////

bool check_channel(scpi_t *context, int32_t ch) {
    if (ch < 1 || ch > CH_NUM) {
        SCPI_ErrorPush(context, SCPI_ERROR_CHANNEL_NOT_FOUND);
        return false;
    }

    if (!psu::isPowerUp()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return false;
    }

    if (Channel::get(ch - 1).isTestFailed()) {
        SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_ERROR);
        return false;
    }

    if (!Channel::get(ch - 1).isPowerOk()) {
        SCPI_ErrorPush(context, SCPI_ERROR_CH1_FAULT_DETECTED - (ch - 1));
        return false;
    }

    if (!Channel::get(ch - 1).isOk()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return false;
    }

    return true;
}

Channel *param_channel(scpi_t *context, scpi_bool_t mandatory, scpi_bool_t skip_channel_check) {
    int32_t ch;

    if (!SCPI_ParamChoice(context, channel_choice, &ch, mandatory)) {
        if (mandatory || SCPI_ParamErrorOccurred(context)) {
            return 0;
        }
        scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;
        ch = psu_context->selected_channel_index;
    }

    if (!skip_channel_check && !check_channel(context, ch)) return 0;

    return &Channel::get(ch - 1);
}

Channel *set_channel_from_command_number(scpi_t *context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;

    int32_t ch;
    SCPI_CommandNumbers(context, &ch, 1, psu_context->selected_channel_index);
    if (ch < 1 || ch > CH_NUM) {
        SCPI_ErrorPush(context, SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE);
        return 0;
    }

    if (!check_channel(context, ch)) return 0;

    return &Channel::get(ch - 1);
}

bool param_temp_sensor(scpi_t *context, int32_t &sensor) {
    if (!SCPI_ParamChoice(context, temp_sensor_choice, &sensor, FALSE)) {
#if OPTION_AUX_TEMP_SENSOR
        if (SCPI_ParamErrorOccurred(context)) {
            return false;
        }
        sensor = temp_sensor::AUX;
#else
        SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
		return false;
#endif
    }

	if (!temp_sensor::sensors[sensor].installed) {
        SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
		return false;
	}

	return true;
}

bool get_voltage_param(scpi_t *context, float &value, const Channel *channel, const Channel::Value *cv) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_voltage_from_param(context, param, value, channel, cv);
}

bool get_voltage_protection_level_param(scpi_t *context, float &value, float min, float max, float def) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_voltage_protection_level_from_param(context, param, value, min, max, def);
}

bool get_current_param(scpi_t *context, float &value, const Channel *channel, const Channel::Value *cv) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_current_from_param(context, param, value, channel, cv);
}

bool get_power_param(scpi_t *context, float &value, float min, float max, float def) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_power_from_param(context, param, value, min, max, def);
}

bool get_temperature_param(scpi_t *context, float &value, float min, float max, float def) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_temperature_from_param(context, param, value, min, max, def);
}

bool get_duration_param(scpi_t *context, float &value, float min, float max, float def) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_duration_from_param(context, param, value, min, max, def);
}

bool get_voltage_from_param(scpi_t *context, const scpi_number_t &param, float &value, const Channel *channel, const Channel::Value *cv) {
    if (param.special) {
		if (channel) {
			if (param.tag == SCPI_NUM_MAX) {
				value = channel_dispatcher::getUMax(*channel);
			}
			else if (param.tag == SCPI_NUM_MIN) {
				value = channel_dispatcher::getUMin(*channel);
			}
			else if (param.tag == SCPI_NUM_DEF) {
				value = channel_dispatcher::getUDef(*channel);
			}
			else if (param.tag == SCPI_NUM_UP && cv) {
				value = cv->set + cv->step;
				if (value > channel_dispatcher::getUMax(*channel)) value = channel_dispatcher::getUMax(*channel);
			}
			else if (param.tag == SCPI_NUM_DOWN && cv) {
				value = cv->set - cv->step;
				if (value < channel_dispatcher::getUMin(*channel)) value = channel_dispatcher::getUMin(*channel);
			}
			else {
				SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
				return false;
			}
		} else {
			SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
			return false;
		}
    }
    else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_VOLT) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;
		
		if (channel) {
			if (value < channel_dispatcher::getUMin(*channel) || value > channel_dispatcher::getUMax(*channel)) {
				SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
				return false;
			}
		}
    }

    return true;
}

bool get_voltage_protection_level_from_param(scpi_t *context, const scpi_number_t &param, float &value, float min, float max, float def) {
    if (param.special) {
        if (param.tag == SCPI_NUM_MAX) {
            value = max;
        }
        else if (param.tag == SCPI_NUM_MIN) {
            value = min;
        }
        else if (param.tag == SCPI_NUM_DEF) {
            value = def;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return false;
        }
    }
    else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_VOLT) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;
        if (value < min || value > max) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return false;
        }
    }
    return true;
}

bool get_current_from_param(scpi_t *context, const scpi_number_t &param, float &value, const Channel *channel, const Channel::Value *cv) {
    if (param.special) {
        if (param.tag == SCPI_NUM_MAX) {
            value = channel_dispatcher::getIMax(*channel);
        }
        else if (param.tag == SCPI_NUM_MIN) {
            value = channel_dispatcher::getIMin(*channel);
        }
        else if (param.tag == SCPI_NUM_DEF) {
            value = channel_dispatcher::getIDef(*channel);
        }
        else if (param.tag == SCPI_NUM_UP && cv) {
            value = cv->set + cv->step;
            if (value > channel_dispatcher::getIMax(*channel)) value = channel_dispatcher::getIMax(*channel);
        }
        else if (param.tag == SCPI_NUM_DOWN && cv) {
            value = cv->set - cv->step;
            if (value < channel_dispatcher::getIMin(*channel)) value = channel_dispatcher::getIMin(*channel);
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return false;
        }
    }
    else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_AMPER) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;
        if (value < channel_dispatcher::getIMin(*channel) || value > channel_dispatcher::getIMax(*channel)) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return false;
        }
    }

    return true;
}

bool get_power_from_param(scpi_t *context, const scpi_number_t &param, float &value, float min, float max, float def) {
    if (param.special) {
        if (param.tag == SCPI_NUM_MAX) {
            value = max;
        }
        else if (param.tag == SCPI_NUM_MIN) {
            value = min;
        }
        else if (param.tag == SCPI_NUM_DEF) {
            value = def;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return false;
        }
    }
    else {
        if (param.unit != SCPI_UNIT_NONE) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;
        if (value < min || value > max) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return false;
        }
    }
    return true;
}

bool get_temperature_from_param(scpi_t *context, const scpi_number_t &param, float &value, float min, float max, float def) {
    if (param.special) {
        if (param.tag == SCPI_NUM_MAX) {
            value = max;
        }
        else if (param.tag == SCPI_NUM_MIN) {
            value = min;
        }
        else if (param.tag == SCPI_NUM_DEF) {
            value = def;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return false;
        }
    }
    else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_CELSIUS) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;
        if (value < min || value > max) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return false;
        }
    }

    return true;
}

bool get_duration_from_param(scpi_t *context, const scpi_number_t &param, float &value, float min, float max, float def) {
    if (param.special) {
        if (param.tag == SCPI_NUM_MAX) {
            value = max;
        }
        else if (param.tag == SCPI_NUM_MIN) {
            value = min;
        }
        else if (param.tag == SCPI_NUM_DEF) {
            value = def;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return false;
        }
    }
    else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_SECOND) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;
        if (value < min || value > max) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return false;
        }
    }

    return true;
}

bool get_voltage_limit_param(scpi_t *context, float &value, const Channel *channel, const Channel::Value *cv) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_voltage_limit_from_param(context, param, value, channel, cv);
}

bool get_current_limit_param(scpi_t *context, float &value, const Channel *channel, const Channel::Value *cv) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_current_limit_from_param(context, param, value, channel, cv);
}

bool get_power_limit_param(scpi_t *context, float &value, const Channel *channel, const Channel::Value *cv) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_power_limit_from_param(context, param, value, channel, cv);
}

bool get_voltage_limit_from_param(scpi_t *context, const scpi_number_t &param, float &value, const Channel *channel, const Channel::Value *cv) {
    if (param.special) {
		if (channel) {
			if (param.tag == SCPI_NUM_MAX) {
				value = channel_dispatcher::getUMaxLimit(*channel);
			}
			else if (param.tag == SCPI_NUM_MIN) {
				value = channel_dispatcher::getUMin(*channel);
			}
			else if (param.tag == SCPI_NUM_DEF) {
				value = channel_dispatcher::getUMaxLimit(*channel);
			}
			else {
				SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
				return false;
			}
		} else {
			SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
			return false;
		}
    }
    else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_VOLT) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;
		
		if (channel) {
			if (value < channel_dispatcher::getUMin(*channel) || channel_dispatcher::getUMaxLimit(*channel)) {
				SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
				return false;
			}
		}
    }

    return true;
}

bool get_current_limit_from_param(scpi_t *context, const scpi_number_t &param, float &value, const Channel *channel, const Channel::Value *cv) {
    if (param.special) {
        if (param.tag == SCPI_NUM_MAX) {
            value = channel_dispatcher::getIMaxLimit(*channel);
        }
        else if (param.tag == SCPI_NUM_MIN) {
            value = channel_dispatcher::getIMax(*channel);
        }
        else if (param.tag == SCPI_NUM_DEF) {
            value = channel_dispatcher::getIMaxLimit(*channel);
        }
    }
    else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_AMPER) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;
        if (value < channel_dispatcher::getIMin(*channel) || value > channel_dispatcher::getIMaxLimit(*channel)) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return false;
        }
    }

    return true;
}

bool get_power_limit_from_param(scpi_t *context, const scpi_number_t &param, float &value, const Channel *channel, const Channel::Value *cv) {
    if (param.special) {
        if (param.tag == SCPI_NUM_MAX) {
            value = channel_dispatcher::getPowerMaxLimit(*channel);
        }
        else if (param.tag == SCPI_NUM_MIN) {
            value = channel_dispatcher::getPowerMinLimit(*channel);
        }
        else if (param.tag == SCPI_NUM_DEF) {
            value = channel_dispatcher::getPowerMaxLimit(*channel);
        }
    }
    else {
        if (param.unit != SCPI_UNIT_NONE) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;
        if (value < channel_dispatcher::getPowerMinLimit(*channel) || value > channel_dispatcher::getPowerMaxLimit(*channel)) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return false;
        }
    }

    return true;
}

scpi_result_t result_float(scpi_t *context, Channel *channel, float value, ValueType valueType) {
    char buffer[32] = { 0 };

    int numSignificantDecimalDigits;
    if (channel && channel->boardRevision == CH_BOARD_REVISION_R5B12 && valueType == VALUE_TYPE_FLOAT_AMPER && util::lessOrEqual(value, 0.5, getPrecision(VALUE_TYPE_FLOAT_AMPER))) {
        numSignificantDecimalDigits = CURRENT_NUM_SIGNIFICANT_DECIMAL_DIGITS_R5B12;
    } else {
        numSignificantDecimalDigits = getNumSignificantDecimalDigits(valueType);
    }

    util::strcatFloat(buffer, value, numSignificantDecimalDigits);
    SCPI_ResultCharacters(context, buffer, strlen(buffer));
    return SCPI_RES_OK;
}

bool get_profile_location_param(scpi_t * context, int &location, bool all_locations) {
    int32_t param;
    if (!SCPI_ParamInt(context, &param, true)) {
        return false;
    }

    if (param < (all_locations ? 0 : 1) || param > NUM_PROFILE_LOCATIONS - 1) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return false;
    }

    location = (int)param;

    return true;
}

void outputOnTime(scpi_t* context, uint32_t time) {
    char str[128];
	ontime::counterToString(str, sizeof(str), time);
	SCPI_ResultText(context, str);
}

}
}
} // namespace eez::psu::scpi