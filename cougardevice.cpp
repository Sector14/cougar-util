#include <fstream>
#include <iostream>
#include <type_traits>

#include <chrono>
#include <thread>

#include "cougardevice.h"
#include "usbdevice.h"

namespace CougarDevice {

static const size_t cTMCFileSizeBytes = 171;

static const int cProfileDataWindowsAxisIDX = 168;

//////////////////////////////////////////////////////////////////////
// Cougar Helpers
//////////////////////////////////////////////////////////////////////

void UploadProfile(USBDevice &dev, const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (! file.is_open())
        throw std::runtime_error("Unable to open profile file " + filename);

    // TODO: Once TCM format reversed, validate file before uploading.
    // TCM profiles are 171 bytes, sanity check specified user file
    auto beginPos = file.tellg();
    file.seekg(0, std::ios::end);
    auto endPos = file.tellg();

    if (endPos - beginPos != cTMCFileSizeBytes)
        throw std::runtime_error("TCM profiles expected to be 171 bytes. User file is " + std::to_string(endPos - beginPos) + " bytes.");

    // Read in the entire file
    std::vector<unsigned char> newData;
    newData.resize(cTMCFileSizeBytes);

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char *>(newData.data()), newData.size());

    // Read current profile data to determine if the "Windows Axis" state has changed
    bool window_axis_changed = false;
    dev.WriteBulkEP({4,1}, cCougarEndpointBulkOut);
    auto oldData = dev.ReadBulkEP(256, cCougarEndpointBulkIn);
    
    // Upload to Cougar
    dev.WriteBulkEP(newData, cCougarEndpointBulkOut);

    // Has "Window axis" flag changed in new profile? Device reconnect required to take apply.
    if (oldData.at(cProfileDataWindowsAxisIDX) != newData.at(cProfileDataWindowsAxisIDX))   
    {     
        // Blocking call, will take several seconds
        std::cout << "Windows axis state change requires device reconnection. Reconnecting device...";
        dev.Reconnect();
    }
}

void SetCougarOptions(USBDevice &dev, CougarOptions options)
{
    static_assert(std::is_same<std::underlying_type<CougarOptions>::type, unsigned char>::value && "CougarOptions type mismatch");

    // Whilst emulation is part of the 03 xx command bitmask, HOTAS CCP appears
    // to always send an extra 07 anytime emulation is not active. Not sure
    // why, but replicated anyway.
    if ( (options & CougarOptions::ButtonAxisEmulation) != CougarOptions::ButtonAxisEmulation )
        dev.WriteBulkEP({7}, cCougarEndpointBulkOut);

    unsigned char options_bm = static_cast<unsigned char>(options);
    dev.WriteBulkEP({3, options_bm}, cCougarEndpointBulkOut);    
}

//////////////////////////////////////////////////////////////////////

} // namespace CougarDevice|
