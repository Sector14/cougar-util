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

void UploadProfile(USBDevice &dev, const std::string& filename);
void SetCougarOptions(USBDevice &dev, CougarOptions options);

//////////////////////////////////////////////////////////////////////

} // namespace CougarDevice|

#endif // COUGARDEVICE_H
