/*
Cougar-Util

Copyright © 2018 Gary Preston (gary@mups.co.uk)

This file is part of cougar-util.

cougar-util is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef COUGARDEVICE_H
#define COUGARDEVICE_H

#include <cstdint>
#include <type_traits>

#include "usbdevice.h"

namespace CougarDevice {

//////////////////////////////////////////////////////////////////////

const uint16_t cCougarVID = 0x044f;
const uint16_t cCougarPID = 0x0400;

const int cCougarInterfaceBulkOut = 3;
const int cCougarInterfaceBulkIn  = 4;

const int cCougarEndpointBulkOut = 4;
const int cCougarEndpointBulkIn  = 5 | 0x80;

//////////////////////////////////////////////////////////////////////
// Cougar Options bitflags
//////////////////////////////////////////////////////////////////////

enum class CougarOptions : unsigned char
{
    Defaults            = 0,
    UserProfile         = 1,
    ButtonAxisEmulation = 2,
    ManualCalibration   = 4
};

//////////////////////////////////////////////////////////////////////
// Bitmask Operators
//////////////////////////////////////////////////////////////////////

constexpr CougarOptions operator|(CougarOptions a, CougarOptions b)
{
    return static_cast<CougarOptions>( static_cast<std::underlying_type<CougarOptions>::type>(a) |
                                       static_cast<std::underlying_type<CougarOptions>::type>(b));
}

constexpr CougarOptions operator&(CougarOptions a, CougarOptions b)
{
    return static_cast<CougarOptions>( static_cast<std::underlying_type<CougarOptions>::type>(a) &
                                       static_cast<std::underlying_type<CougarOptions>::type>(b));
}

constexpr CougarOptions& operator|=(CougarOptions &a, CougarOptions b)
{
    a = a | b;
    return a;
}

constexpr CougarOptions& operator&=(CougarOptions &a, CougarOptions b)
{
    a = a & b;
    return a;
}

//////////////////////////////////////////////////////////////////////
// Cougar Helpers
//////////////////////////////////////////////////////////////////////

void UploadFirmware(USBDevice &usb_device, const std::string& firmware_filename);
void UploadProfile(USBDevice &dev, const std::string& filename);
void UploadTMJBinary(USBDevice &dev, const std::string& filename);
void SetCougarOptions(USBDevice &dev, CougarOptions options);

//////////////////////////////////////////////////////////////////////

} // namespace CougarDevice|

#endif // COUGARDEVICE_H
