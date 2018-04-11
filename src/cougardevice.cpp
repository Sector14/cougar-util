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

#include <fstream>
#include <iostream>
#include <type_traits>

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
        // Reset device  
        dev.WriteBulkEP({9,5}, cCougarEndpointBulkOut);

        // Blocking call, may take several seconds
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
