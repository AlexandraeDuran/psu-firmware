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
 
#pragma once

/*
PSU 0-50V/3A I/O expander pinout:

CH_BOARD_REVISION_R4B43A

Pin 0  In, ADC interrupt/DRDY
Pin 1  Out, DP enable (active low)
Pin 2  In, CC_ACTIVE
Pin 3  In, Temp sensor (V/F)
Pin 4  In, Battery NTC sensor temperature presented as frequency
Pin 5  In, CV_ACTIVE
Pin 6  In, PWRGOOD
Pin 7  Out, OUTPUT_ENABLE

CH_BOARD_REVISION_R5B6B

Pin 0  In, "ADC interrupt/DRDY" or "remote sense reverse polarity detection" on CH_BOARD_REVISION_R5B9 and CH_BOARD_REVISION_R5B10 
Pin 1  Out, DP enable (active low)
Pin 2  In, CC_ACTIVE
Pin 3  Out, SET_100% (active low)
Pin 4  Out, EXT_PROG
Pin 5  In, CV_ACTIVE
Pin 6  In, PWRGOOD
Pin 7  Out, OUTPUT_ENABLE

*/

namespace eez {
namespace psu {

class Channel;

/// IO Expander HW used by the channel.
class IOExpander {
public:
    static const uint8_t IO_BIT_IN_ADC_DRDY = 0;
	static const uint8_t IO_BIT_IN_RPOL = 0; // remote sense reverse polarity detection
    static const uint8_t IO_BIT_IN_CC_ACTIVE = 2;
    static const uint8_t IO_BIT_IN_TEMP_SENSOR = 3;
    static const uint8_t IO_BIT_IN_CV_ACTIVE = 5;
    static const uint8_t IO_BIT_IN_PWRGOOD = 6;

    static const uint8_t IO_BIT_OUT_DP_ENABLE = 1;
    const uint8_t IO_BIT_OUT_SET_100_PERCENT;
    const uint8_t IO_BIT_OUT_EXT_PROG;
    static const uint8_t IO_BIT_OUT_OUTPUT_ENABLE = 7;

    static const uint8_t IO_BIT_5A = 8;
    static const uint8_t IO_BIT_500mA = 9;

    static const uint8_t IOEXP_READ = 0B01000001;
    static const uint8_t IOEXP_WRITE = 0B01000000;

    static const uint8_t REG_IODIR = 0x00;
    static const uint8_t REG_IPOL = 0x01;
    static const uint8_t REG_GPINTEN = 0x02;
    static const uint8_t REG_DEFVAL = 0x03;
    static const uint8_t REG_INTCON = 0x04;
    static const uint8_t REG_IOCON = 0x05;
    static const uint8_t REG_GPPU = 0x06;
    static const uint8_t REG_INTF = 0x07;
    static const uint8_t REG_INTCAP = 0x08;
    static const uint8_t REG_GPIO = 0x09;
    static const uint8_t REG_OLAT = 0x0A;

    static const uint8_t REG_IODIRA = 0x00;
    static const uint8_t REG_IPOLA = 0x02;
    static const uint8_t REG_GPINTENA = 0x04;
    static const uint8_t REG_DEFVALA = 0x06;
    static const uint8_t REG_INTCONA = 0x08;
    static const uint8_t REG_IOCONA = 0x0A;
    static const uint8_t REG_GPPUA = 0x0C;
    static const uint8_t REG_INTFA = 0x0E;
    static const uint8_t REG_INTCAPA = 0x10;
    static const uint8_t REG_GPIOA = 0x12;
    static const uint8_t REG_OLATA = 0x14;

    static const uint8_t REG_IODIRB = 0x01;
    static const uint8_t REG_IPOLB = 0x03;
    static const uint8_t REG_GPINTENB = 0x05;
    static const uint8_t REG_DEFVALB = 0x07;
    static const uint8_t REG_INTCONB = 0x09;
    static const uint8_t REG_IOCONB = 0x0B;
    static const uint8_t REG_GPPUB = 0x16;
    static const uint8_t REG_INTFB = 0x0F;
    static const uint8_t REG_INTCAPB = 0x11;
    static const uint8_t REG_GPIOB = 0x13;
    static const uint8_t REG_OLATB = 0x15;

    static const size_t NUM_REGISTERS = REG_OLATB + 1;

    psu::TestResult g_testResult;

    IOExpander(Channel &channel, 
        uint8_t IO_BIT_OUT_SET_100_PERCENT_,
        uint8_t IO_BIT_OUT_EXT_PROG_
    );

    void init();
    bool test();

    void tick(uint32_t tick_usec);

	uint8_t readGpio();

    bool testBit(int io_bit);
    void changeBit(int io_bit, bool set);

private:
    Channel &channel;
	uint8_t gpioa;
    uint8_t gpiob;

	uint8_t getRegInitValue(int i);
    uint8_t reg_read(uint8_t reg);
    void reg_write(uint8_t reg, uint8_t val);
};

}
} // namespace eez::psu
