#include <type_traits>

#include "cougardevice.h"
#include "usbdevice.h"

namespace CougarDevice {

//////////////////////////////////////////////////////////////////////
// Cougar Helpers
//////////////////////////////////////////////////////////////////////

void UploadProfile(USBDevice &dev, const std::string& filename)
{
    // TODO: Implement me
}

void SetCougarOptions(USBDevice &dev, CougarOptions options)
{
    dev.WriteBulkEP({4,2}, cCougarEndpointBulkOut);

    // Whilst emulation is part of the 03 xx command, HOTAS CCP appears
    // to always send an extra 07 anytime emulation is not active. Not sure
    // why, but replicated anyway.
    if ( (options & CougarOptions::ButtonAxisEmulation) != CougarOptions::ButtonAxisEmulation )
        dev.WriteBulkEP({7}, cCougarEndpointBulkOut);

    static_assert(std::is_same<std::underlying_type<CougarOptions>::type, unsigned char>::value && "CougarOptions type mismatch");

    unsigned char options_bm = static_cast<unsigned char>(options);
    dev.WriteBulkEP({3, options_bm}, cCougarEndpointBulkOut);    
}

//////////////////////////////////////////////////////////////////////

} // namespace CougarDevice|
