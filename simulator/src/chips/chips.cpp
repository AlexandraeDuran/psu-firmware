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
#include "chips.h"
#include "arduino_internal.h"
#include "datetime.h"

namespace eez {
namespace psu {
namespace simulator {
namespace chips {

/// Instance of EEPROM chip (selected with EEPROM_SELECT LOW)
EepromChip eeprom_chip;

// Instance of RTC chip (selected with RTC_SELECT HIGH)
RtcChip rtc_chip;

// Instance of BP chip (selected with BP_SELECT LOW)
BPChip bp_chip;

// Instance of IOEXP chip for the CH1 (selected with IO_EXPANDER1 LOW)
IOExpanderChip ioexp_chip1;

// Instance of IOEXP chip for the CH2 (selected with IO_EXPANDER2 LOW)
IOExpanderChip ioexp_chip2;

// Instance of ADC chip for the CH1 (selected with ADC1_SELECT LOW)
AnalogDigitalConverterChip adc_chip1(ioexp_chip1, CONVEND1);

// Instance of ADC chip for the CH2 (selected with ADC2_SELECT LOW)
AnalogDigitalConverterChip adc_chip2(ioexp_chip2, CONVEND2);

// Instance of DAC chip for the CH1 (selected with DAC1_SELECT LOW)
DigitalAnalogConverterChip dac_chip1(adc_chip1);

// Instance of DAC chip for the CH2 (selected with DAC2_SELECT LOW)
DigitalAnalogConverterChip dac_chip2(adc_chip2);

/// Currently selected chip on SPI bus
Chip *selected_chip = 0;

void select(int pin, int state) {
    if (pin == EEPROM_SELECT) {
        if (!state) {
            selected_chip = &eeprom_chip;
            selected_chip->select();
        }
        else {
            if (selected_chip == &eeprom_chip) {
                selected_chip = 0;
            }
        }
    }
    else if (pin == RTC_SELECT) {
        if (state) {
            selected_chip = &rtc_chip;
            selected_chip->select();
        }
        else {
            if (selected_chip == &rtc_chip) {
                selected_chip = 0;
            }
        }
    }
    else if (pin == BP_SELECT) {
        if (!state) {
            selected_chip = &bp_chip;
            selected_chip->select();
        }
        else {
            if (selected_chip == &bp_chip) {
                selected_chip = 0;
            }
        }
    }
    else if (pin == IO_EXPANDER1) {
        if (!state) {
            selected_chip = &ioexp_chip1;
            selected_chip->select();
        }
        else {
            if (selected_chip == &ioexp_chip1) {
                selected_chip = 0;
            }
        }
    }
    else if (pin == IO_EXPANDER2) {
        if (!state) {
            selected_chip = &ioexp_chip2;
            selected_chip->select();
        }
        else {
            if (selected_chip == &ioexp_chip2) {
                selected_chip = 0;
            }
        }
    }
    else if (pin == ADC1_SELECT) {
        if (!state) {
            selected_chip = &adc_chip1;
            selected_chip->select();
        }
        else {
            if (selected_chip == &adc_chip1) {
                selected_chip = 0;
            }
        }
    }
    else if (pin == ADC2_SELECT) {
        if (!state) {
            selected_chip = &adc_chip2;
            selected_chip->select();
        }
        else {
            if (selected_chip == &adc_chip2) {
                selected_chip = 0;
            }
        }
    }
    else if (pin == DAC1_SELECT) {
        if (!state) {
            selected_chip = &dac_chip1;
            selected_chip->select();
        }
        else {
            if (selected_chip == &dac_chip1) {
                selected_chip = 0;
            }
        }
    }
    else if (pin == DAC2_SELECT) {
        if (!state) {
            selected_chip = &dac_chip2;
            selected_chip->select();
        }
        else {
            if (selected_chip == &dac_chip2) {
                selected_chip = 0;
            }
        }
    }
}

uint8_t transfer(uint8_t data) {
    return selected_chip ? selected_chip->transfer(data) : 0;
}

void tick() {
    adc_chip1.tick();
    adc_chip2.tick();
}

////////////////////////////////////////////////////////////////////////////////

EepromChip::EepromChip()
    : state(IDLE)
{
    char *file_path = getConfFilePath("EEPROM.state");
    fp = fopen(file_path, "r+b");
    if (fp == NULL) {
        fp = fopen(file_path, "w+b");
    }
}

EepromChip::~EepromChip() {
    if (fp != NULL) fclose(fp);
}

void EepromChip::select() {
    state = IDLE;
}

uint8_t EepromChip::transfer(uint8_t data) {
    uint8_t result = 0;

    if (state == IDLE) {
        if (data == eeprom::READ) {
            state = READ_ADDR_MSB;
        }
        else if (data == eeprom::WRITE) {
            state = WRITE_ADDR_MSB;
        }
        else if (data == eeprom::RDSR) {
            state = RDSR;
        }
    }
    else if (state == READ_ADDR_MSB) {
        address = ((uint16_t)data) << 8;
        state = READ_ADDR_LSB;
    }
    else if (state == READ_ADDR_LSB) {
        address |= data;
        address_index = 0;
        state = READ;
    }
    else if (state == READ) {
        result = read_byte();
        if (++address_index == 64) address_index = 0;
    }
    else if (state == WRITE_ADDR_MSB) {
        address = ((uint16_t)data) << 8;
        state = WRITE_ADDR_LSB;
    }
    else if (state == WRITE_ADDR_LSB) {
        address |= data;
        address_index = 0;
        state = WRITE;
    }
    else if (state == WRITE) {
        write_byte(data);
        if (++address_index == 64) address_index = 0;
    }
    else if (state == RDSR) {
    }

    return result;
}

uint8_t EepromChip::read_byte() {
    if (fp == NULL) return 0;
    fseek(fp, address + address_index, SEEK_SET);
    uint8_t data = 0;
    fread(&data, 1, 1, fp);
    return data;
}

void EepromChip::write_byte(uint8_t data) {
    if (fp == NULL) return;
    fseek(fp, address + address_index, SEEK_SET);
    fwrite(&data, 1, 1, fp);
    fflush(fp);
}

////////////////////////////////////////////////////////////////////////////////

RtcChip::RtcChip()
    : state(IDLE)
{
    char *file_path = getConfFilePath("RTC.state");
    fp = fopen(file_path, "r+b");
    if (fp == NULL) {
        offset = 0;

        fp = fopen(file_path, "wb");
        if (fp != NULL) {
            fseek(fp, 0, SEEK_SET);
            fwrite(&offset, sizeof(offset), 1, fp);
        }
    }
    else {
        fread(&offset, sizeof(offset), 1, fp);
    }
}

RtcChip::~RtcChip() {
    if (fp != NULL) fclose(fp);
}

void RtcChip::select() {
    state = IDLE;
}

uint8_t RtcChip::transfer(uint8_t data) {
    uint8_t result = 0;

    if (state == IDLE) {
        if (data == rtc::RD_CONTROL_1) {
            state = RD_CONTROL_1;
        }
        else if (data == rtc::WR_CONTROL_1) {
            state = WR_CONTROL_1;
        }
        else if (data == rtc::RD_SECONDS) {
            state = RD_SECONDS;
        }
        else if (data == rtc::WR_SECONDS) {
            state = WR_SECONDS;
        }
        else if (data == rtc::RD_DAYS) {
            state = RD_DAYS;
        }
        else if (data == rtc::WR_DAYS) {
            state = WR_DAYS;
        }
    }
    else if (state == RD_CONTROL_1) {
        result = ctrl1;
        state = RD_CONTROL_2;
    }
    else if (state == RD_CONTROL_2) {
        result = ctrl2;
    }
    else if (state == WR_CONTROL_1) {
        ctrl1 = data;
        state = WR_CONTROL_2;
    }
    else if (state == WR_CONTROL_2) {
        ctrl2 = data;
    }
    else if (state == RD_SECONDS) {
        result = util::toBCD(getSecond());
        state = RD_MINUTES;
    }
    else if (state == RD_MINUTES) {
        result = util::toBCD(getMinute());
        state = RD_HOURS;
    }
    else if (state == RD_HOURS) {
        result = util::toBCD(getHour());
        state = RD_DAYS;
    }
    else if (state == RD_DAYS) {
        result = util::toBCD(getDay());
        state = RD_WEEKDAYS;
    }
    else if (state == RD_WEEKDAYS) {
        result = util::toBCD(getWeekday());
        state = RD_MONTHS;
    }
    else if (state == RD_MONTHS) {
        result = util::toBCD(getMonth());
        state = RD_YEARS;
    }
    else if (state == RD_YEARS) {
        result = util::toBCD(getYear());
    }
    else if (state == WR_SECONDS) {
        setSecond(util::fromBCD(data));
        state = WR_MINUTES;
    }
    else if (state == WR_MINUTES) {
        setMinute(util::fromBCD(data));
        state = WR_HOURS;
    }
    else if (state == WR_HOURS) {
        setHour(util::fromBCD(data));
        state = WR_DAYS;
    }
    else if (state == WR_DAYS) {
        setDay(util::fromBCD(data));
        state = WR_WEEKDAYS;
    }
    else if (state == WR_WEEKDAYS) {
        setWeekday(util::fromBCD(data));
        state = WR_MONTHS;
    }
    else if (state == WR_MONTHS) {
        setMonth(util::fromBCD(data));
        state = WR_YEARS;
    }
    else if (state == WR_YEARS) {
        setYear(util::fromBCD(data));
    }

    return result;
}

uint32_t RtcChip::nowUtc() {
    time_t now_time_t = time(0);
    struct tm *now_tm = gmtime(&now_time_t);
    return datetime::makeTime(1900 + now_tm->tm_year, now_tm->tm_mon + 1, now_tm->tm_mday, now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec);
}

void RtcChip::getTime(uint8_t &result_year, uint8_t &result_month, uint8_t &result_day, uint8_t &result_weekday, uint8_t &result_hour, uint8_t &result_minute, uint8_t &result_second) {
    int year, month, day, hour, minute, second;
    datetime::breakTime(offset + nowUtc(), year, month, day, hour, minute, second);

    result_year = uint8_t(year - 2000);
    result_month = uint8_t(month);
    result_day = uint8_t(day);

    result_weekday = datetime::dayOfWeek(year, month, day) - 1;

    result_hour = uint8_t(hour);
    result_minute = uint8_t(minute);
    result_second = uint8_t(second);
}

void RtcChip::setOffset(uint32_t offset_) {
    offset = offset_;
    if (fp != NULL) {
        fseek(fp, 0, SEEK_SET);
        fwrite(&offset, sizeof(offset), 1, fp);
        fflush(fp);
    }
}

uint8_t RtcChip::getSecond() {
    uint8_t year, month, day, weekday, hour, minute, second;
    getTime(year, month, day, weekday, hour, minute, second);
    return second;
}

void RtcChip::setSecond(uint8_t second_) {
    uint8_t year, month, day, weekday, hour, minute, second;
    getTime(year, month, day, weekday, hour, minute, second);
    setOffset(datetime::makeTime(year + 2000, month, day, hour, minute, second_) - nowUtc());
}

uint8_t RtcChip::getMinute() {
    uint8_t year, month, day, weekday, hour, minute, second;
    getTime(year, month, day, weekday, hour, minute, second);
    return minute;
}

void RtcChip::setMinute(uint8_t minute_) {
    uint8_t year, month, day, weekday, hour, minute, second;
    getTime(year, month, day, weekday, hour, minute, second);
    setOffset(datetime::makeTime(year + 2000, month, day, hour, minute_, second) - nowUtc());
}

uint8_t RtcChip::getHour() {
    uint8_t year, month, day, weekday, hour, minute, second;
    getTime(year, month, day, weekday, hour, minute, second);
    return hour;
}

void RtcChip::setHour(uint8_t hour_) {
    uint8_t year, month, day, weekday, hour, minute, second;
    getTime(year, month, day, weekday, hour, minute, second);
    setOffset(datetime::makeTime(year + 2000, month, day, hour_, minute, second) - nowUtc());
}

uint8_t RtcChip::getDay() {
    uint8_t year, month, day, weekday, hour, minute, second;
    getTime(year, month, day, weekday, hour, minute, second);
    return day;
}

void RtcChip::setDay(uint8_t day_) {
    uint8_t year, month, day, weekday, hour, minute, second;
    getTime(year, month, day, weekday, hour, minute, second);
    setOffset(datetime::makeTime(year + 2000, month, day_, hour, minute, second) - nowUtc());
}

uint8_t RtcChip::getWeekday() {
    uint8_t year, month, day, weekday, hour, minute, second;
    getTime(year, month, day, weekday, hour, minute, second);
    return weekday;
}

void RtcChip::setWeekday(uint8_t weekdays) {
}

uint8_t RtcChip::getMonth() {
    uint8_t year, month, day, weekday, hour, minute, second;
    getTime(year, month, day, weekday, hour, minute, second);
    return month;
}

void RtcChip::setMonth(uint8_t month_) {
    uint8_t year, month, day, weekday, hour, minute, second;
    getTime(year, month, day, weekday, hour, minute, second);
    setOffset(datetime::makeTime(year + 2000, month_, day, hour, minute, second) - nowUtc());
}

uint8_t RtcChip::getYear() {
    uint8_t year, month, day, weekday, hour, minute, second;
    getTime(year, month, day, weekday, hour, minute, second);
    return year;
}

void RtcChip::setYear(uint8_t year_) {
    uint8_t year, month, day, weekday, hour, minute, second;
    getTime(year, month, day, weekday, hour, minute, second);
    setOffset(datetime::makeTime(year_ + 2000, month, day, hour, minute, second) - nowUtc());
}

////////////////////////////////////////////////////////////////////////////////

BPChip::BPChip()
    : state(IDLE)
{
}

void BPChip::select() {
    state = IDLE;
}

uint8_t BPChip::transfer(uint8_t data) {
    uint8_t result = 0;

    if (state == IDLE) {
        value = ((uint16_t)data) << 8;
        state = READ_MSB;
    }
    else {
        value |= data;
        state = IDLE;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////

IOExpanderChip::IOExpanderChip()
    : state(IDLE)
    , pwrgood(true)
    , rpol(false)
{
}

bool IOExpanderChip::getPwrgood(int pin) {
    return pin == IO_EXPANDER1 ? ioexp_chip1.pwrgood : ioexp_chip2.pwrgood;
}

void IOExpanderChip::setPwrgood(int pin, bool on) {
    if (pin == IO_EXPANDER1) ioexp_chip1.pwrgood = on;
    else ioexp_chip2.pwrgood = on;
}

bool IOExpanderChip::getRPol(int pin) {
    return pin == IO_EXPANDER1 ? ioexp_chip1.rpol : ioexp_chip2.rpol;
}

void IOExpanderChip::setRPol(int pin, bool on) {
    if (pin == IO_EXPANDER1) ioexp_chip1.rpol = on;
    else ioexp_chip2.rpol = on;
}

void IOExpanderChip::select() {
    state = IDLE;
}

uint8_t IOExpanderChip::transfer(uint8_t data) {
    uint8_t result = 0;

    if (state == IDLE) {
        if (data == IOExpander::IOEXP_READ) {
            state = READ_REGISTER_INDEX;
        }
        else if (data == IOExpander::IOEXP_WRITE) {
            state = WRITE_REGISTER_INDEX;
        }
    }
    else if (state == READ_REGISTER_INDEX) {
        register_index = data;
        state = READ_REGISTER_VALUE;
    }
    else if (state == READ_REGISTER_VALUE) {
        Channel &channel = Channel::get(this == &ioexp_chip1 ? 0 : 1);

        if (channel.boardRevision == CH_BOARD_REVISION_R5B12 && register_index == IOExpander::REG_GPIOA ||
            channel.boardRevision != CH_BOARD_REVISION_R5B12 && register_index == IOExpander::REG_GPIO) 
        {
            result = register_values[register_index];

            if (pwrgood) {
                result |= 1 << IOExpander::IO_BIT_IN_PWRGOOD;
            } else {
                result &= ~(1 << IOExpander::IO_BIT_IN_PWRGOOD);
            }

            if (channel.getFeatures() & CH_FEATURE_RPOL) {
                if (!rpol) {
                    result |= 1 << IOExpander::IO_BIT_IN_RPOL;
                } else {
                    result &= ~(1 << IOExpander::IO_BIT_IN_RPOL);
                }
            }

            if (cv) {
                result |= 1 << IOExpander::IO_BIT_IN_CV_ACTIVE;
            } else {
                result &= ~(1 << IOExpander::IO_BIT_IN_CV_ACTIVE);
            }

            if (cc) {
                result |= 1 << IOExpander::IO_BIT_IN_CC_ACTIVE;
            } else {
                result &= ~(1 << IOExpander::IO_BIT_IN_CC_ACTIVE);
            }
        }
        else {
            result = register_values[register_index];
        }
    }
    else if (state == WRITE_REGISTER_INDEX) {
        register_index = data;
        state = WRITE_REGISTER_VALUE;
    }
    else if (state == WRITE_REGISTER_VALUE) {
        register_values[register_index] = data;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////

AnalogDigitalConverterChip::AnalogDigitalConverterChip(IOExpanderChip &ioexp_chip_, int convend_pin_)
    : ioexp_chip(ioexp_chip_)
    , convend_pin(convend_pin_)
    , state(IDLE)
    , tick_counter(0)
    , start(false)
{
}

void AnalogDigitalConverterChip::select() {
    state = IDLE;
}

uint8_t AnalogDigitalConverterChip::transfer(uint8_t data) {
    uint8_t result = 0;

    if (state == IDLE) {
        if (data == AnalogDigitalConverter::ADC_RESET) {
        }
        else if (data == AnalogDigitalConverter::ADC_RD3S1) {
            register_index = 1;
            state = READ_REG;
        }
        else if (data == AnalogDigitalConverter::ADC_WR3S1) {
            register_index = 1;
            state = WRITE_REG;
        }
        else if (data == AnalogDigitalConverter::ADC_WR1S0) {
            state = WR1S0;
        }
        else if (data == AnalogDigitalConverter::ADC_RDATA) {
            state = RDATA_MSB;
        }
        else if (data == AnalogDigitalConverter::ADC_START) {
            start = true;
            tick();
        }
    }
    else if (state == READ_REG) {
        result = register_values[register_index++];
    }
    else if (state == WRITE_REG) {
        register_values[register_index++] = data;
    }
    else if (state == WR1S0) {
        register_values[0] = data;
        state = IDLE;
    }
    else if (state == RDATA_MSB) {
        result = getValue() >> 8;
        state = RDATA_LSB;
    }
    else if (state == RDATA_LSB) {
        result = getValue() & 0xFF;
    }

    return result;
}

void AnalogDigitalConverterChip::tick() {
    if (tick_counter < 4) {
        ++tick_counter;

        if (start) {
            start = false;

            //static const int CODE_TO_SPS [] = { 20, 45, 90, 175, 330, 600, 1000 };
            //psu::delayMicroseconds(1000000 / CODE_TO_SPS[ADC_SPS]);

            InterruptCallback callback = interrupt_callbacks[convend_pin];
            if (callback) {
                callback();
            }
        }

        --tick_counter;
    }
}

uint16_t AnalogDigitalConverterChip::getValue() {
    updateValues();

    if (register_values[0] == AnalogDigitalConverter::ADC_REG0_READ_U_MON) {
        return u_mon;
    }
    if (register_values[0] == AnalogDigitalConverter::ADC_REG0_READ_I_MON) {
        return i_mon;
    }
    if (register_values[0] == AnalogDigitalConverter::ADC_REG0_READ_U_SET) {
        return u_set;
    }
    else {
        return i_set;
    }
}

void AnalogDigitalConverterChip::setDacValue(uint8_t data_buffer, uint16_t dac_value) {
    if (data_buffer == DigitalAnalogConverter::DATA_BUFFER_A) {
        float value = util::remap(dac_value,
            DigitalAnalogConverter::DAC_MIN, AnalogDigitalConverter::ADC_MIN,
            DigitalAnalogConverter::DAC_MAX, AnalogDigitalConverter::ADC_MAX);
        u_set = (uint16_t)util::clamp(value,
            AnalogDigitalConverter::ADC_MIN, AnalogDigitalConverter::ADC_MAX);
    }
    else {
        float value = util::remap(dac_value,
            DigitalAnalogConverter::DAC_MIN, AnalogDigitalConverter::ADC_MIN,
            DigitalAnalogConverter::DAC_MAX, AnalogDigitalConverter::ADC_MAX);
        i_set = (uint16_t)util::clamp(value,
            AnalogDigitalConverter::ADC_MIN, AnalogDigitalConverter::ADC_MAX);
    }
    updateValues();
    tick();
}

void AnalogDigitalConverterChip::updateValues() {
    for (int i = 0; i < CH_NUM; ++i) {
        Channel &channel = Channel::get(i);
        if (channel.convend_pin == convend_pin) {
            if (channel.simulator.getLoadEnabled()) {
                float u_set_v = channel.isRemoteProgrammingEnabled() ? util::remap(channel.simulator.voltProgExt, 0, 0, 2.5, channel.u.max) : channel.remapAdcDataToVoltage(u_set);
                float i_set_a = channel.remapAdcDataToCurrent(i_set);

                float u_mon_v = i_set_a * channel.simulator.load;
                float i_mon_a = i_set_a;
                if (u_mon_v > u_set_v) {
                    u_mon_v = u_set_v;
                    i_mon_a = u_set_v / channel.simulator.load;

                    ioexp_chip.cv = true;
                    ioexp_chip.cc = false;
                }
                else {
                    ioexp_chip.cv = false;
                    ioexp_chip.cc = true;
                }

                u_mon = channel.remapVoltageToAdcData(u_mon_v);
                i_mon = channel.remapCurrentToAdcData(i_mon_a);

                return;
            }
            else {
                if (channel.isOutputEnabled()) {
                    u_mon = u_set;
                    i_mon = 0;
                    if (u_set > 0 && i_set > 0) {
                        ioexp_chip.cv = true;
                        ioexp_chip.cc = false;
                    }
                    else {
                        ioexp_chip.cv = false;
                        ioexp_chip.cc = false;
                    }
                    return;
                }
            }
        }
    }

    u_mon = 0;
    i_mon = 0;
    ioexp_chip.cc = false;
    ioexp_chip.cv = false;
}

////////////////////////////////////////////////////////////////////////////////

DigitalAnalogConverterChip::DigitalAnalogConverterChip(AnalogDigitalConverterChip &adc_chip_)
    : adc_chip(adc_chip_)
    , state(IDLE)
{
}

void DigitalAnalogConverterChip::select() {
    state = IDLE;
}

uint8_t DigitalAnalogConverterChip::transfer(uint8_t data) {
    uint8_t result = 0;

    if (state == IDLE) {
        if (data == DigitalAnalogConverter::DATA_BUFFER_A || data == DigitalAnalogConverter::DATA_BUFFER_B) {
            data_buffer = data;
            state = DATA_BUFFER_MSB;
        }
    }
    else if (state == DATA_BUFFER_MSB) {
        value = ((uint16_t)data) << 8;
        state = DATA_BUFFER_LSB;
    }
    else if (state == DATA_BUFFER_LSB) {
        value |= data;
        adc_chip.setDacValue(data_buffer, value);
    }

    return result;
}

}
}
}
} // namespace eez::psu::simulator::chips