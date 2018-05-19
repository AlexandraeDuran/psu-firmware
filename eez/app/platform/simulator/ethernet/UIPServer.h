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

#include "eez/app/platform/simulator/ethernet/UIPClient.h"

namespace eez {
namespace app {
namespace simulator {
namespace arduino {

/// Bare minimum implementation of the Arduino EthernetServer class
class EthernetServer {
public:
    EthernetServer(int port);

    void begin();
    EthernetClient available();

private:
    bool bind_result;
    int port;
    EthernetClient client;
};

}
}
}
} // namespace eez::app::simulator::arduino;

using namespace eez::app::simulator::arduino;
