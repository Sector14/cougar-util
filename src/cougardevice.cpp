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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <type_traits>

#include "cougardevice.h"
#include "usbdevice.h"

namespace CougarDevice {

static const size_t cTMCFileSizeBytes = 171;

// Profile data returned by 04 01
static const int cProfileDataWindowsAxisIDX = 168;

//////////////////////////////////////////////////////////////////////
// File I/O
//////////////////////////////////////////////////////////////////////

// Pass requiredSize > 0 to throw if file is not of expected size
std::vector<unsigned char> LoadBinaryFile(const std::string& filename, size_t requiredSize = 0)
{
    std::ifstream file(filename, std::ios::binary);
    if (! file.is_open())
        throw std::runtime_error("Unable to open file " + filename);

    // Sanity check user selected suitable file based on expected size
    auto begin_pos = file.tellg();
    file.seekg(0, std::ios::end);
    auto end_pos = file.tellg();
    
    size_t file_size = end_pos - begin_pos;

    if (requiredSize != 0 && file_size != requiredSize)
        throw std::runtime_error("Loaded file is " + std::to_string(file_size) + " bytes." +
                                 "Required " + std::to_string(requiredSize) + " bytes.");

    // Read in the entire file
    std::vector<unsigned char> newData;
    newData.resize(file_size);

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char *>(newData.data()), newData.size());

    return newData;
}

//////////////////////////////////////////////////////////////////////
// Cougar Helpers
//////////////////////////////////////////////////////////////////////

void UploadFirmware(USBDevice &dev, const std::string& filename)
{
    std::cout << "Not implemented yet\n";
}

// Pre-compiled TMJ/TMM file
void UploadProfile(USBDevice &dev, const std::string& filename)
{
    // Read current profile data to determine current options setting
    dev.WriteBulkEP({4,1}, cCougarEndpointBulkOut);
    auto oldData = dev.ReadBulkEP(256, cCougarEndpointBulkIn);
    
    auto newData = LoadBinaryFile(filename, cTMCFileSizeBytes);

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

void UploadTMJBinary(USBDevice &dev, const std::string& filename)
{
    std::cout << "Not implemented yet\n";
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
