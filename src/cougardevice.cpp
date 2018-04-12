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

// TODO: HOTASUpdate.exe sha256sum for safety check

static const size_t cHOTASUpdateFirmwareSize = 25030;
static const std::vector<unsigned char> cDefaultTCMProfile = {
    0x02,0x00,0x01,0x08,0x09,0x02,0x03,0x04,0x05,0x06,0x07,0x00,0x00,0x9e,0x38,0x90,0x46,0x90,0x80,0x77,0x80,0x81,0x10,0x88,0x00,0x88,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x17,0x9d,0x10,0x19,0x14,0x78,0x15,0x8e,0x25,0x76,0x31,0x74,0x29,0x26,0x24,0xbe,0x16,0xca,0x16,0xcb,0x19,0xca,0x10,0x3e,0x19,0xf2,0x10,0x2e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x08,0x00,0x08,0x00,0x10,0xcc,0x10,0xcc,0x00,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xef,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xff,0xff,0xff,0xff,0xff,0xff,0x00
};

//////////////////////////////////////////////////////////////////////
// File I/O
//////////////////////////////////////////////////////////////////////

// Pass requiredSize > 0 to throw if file is not of expected size
std::vector<unsigned char> LoadBinaryFile(const std::string& filename, size_t requiredSize = 0)
{
    // TODO: use ate instead
    std::ifstream file(filename, std::ios::binary);
    if (! file.is_open())
        throw std::runtime_error("Unable to open file " + filename);

    // Sanity check user selected suitable file based on expected size
    auto begin_pos = file.tellg();
    file.seekg(0, file.end);
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

    // TODO: Check if file "bad" or throw    
    return newData;
}

//////////////////////////////////////////////////////////////////////
// Cougar Helpers
//////////////////////////////////////////////////////////////////////

void WaitResetDevice(USBDevice &dev)
{
    // Reset device
    dev.WriteBulkEP({9,5}, cCougarEndpointBulkOut);

    // Blocking call, may take several seconds
    dev.Reconnect(); 
}

void UploadProfileData(USBDevice &dev, const std::vector<unsigned char> &data)
{
    // Read current profile data to determine current options setting
    dev.WriteBulkEP({4,1}, cCougarEndpointBulkOut);
    auto old_data = dev.ReadBulkEP(256, cCougarEndpointBulkIn);

    // Upload to Cougar
    dev.WriteBulkEP(data, cCougarEndpointBulkOut);

    // Has "Window axis" flag changed in new profile? Device reconnect required to apply.
    if (old_data.at(cProfileDataWindowsAxisIDX) != data.at(cProfileDataWindowsAxisIDX))
        WaitResetDevice(dev);
}

// Pre-compiled TMJ/TMM file
void UploadProfile(USBDevice &dev, const std::string& filename)
{   
    auto new_data = LoadBinaryFile(filename, cTMCFileSizeBytes);

    UploadProfileData(dev, new_data);
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

void UploadFirmware(USBDevice &dev, const std::string& filename)
{
    // TODO: Verify sha256sum of HOTASUpdate.exe

    // TODO: Extract firmware using hardcoded offset
    std::ifstream file(filename, std::ios::binary);
    if (! file.is_open())
        throw std::runtime_error("Unable to open file " + filename);
    file.seekg(-cHOTASUpdateFirmwareSize, file.end);
    
    // "0x05" firmware update command followed by firmware itself and "0xff"
    std::vector<unsigned char> firmware{5};
    firmware.reserve(cHOTASUpdateFirmwareSize+2);
    firmware.resize(cHOTASUpdateFirmwareSize+1);
    file.read(reinterpret_cast<char *>(firmware.data()+1), cHOTASUpdateFirmwareSize);
    firmware.emplace_back(0xff);

    if (! file.good())
        throw std::runtime_error("Unable to extract firmware data from " + filename);

    dev.WriteBulkEP(firmware, cCougarEndpointBulkOut);

    // Firmware upload causes a device reset
    dev.Reconnect();

    // New device has no profile loaded, flash a default one
    UploadProfileData(dev, cDefaultTCMProfile);

    std::cout << "Resetting device\n";
    WaitResetDevice(dev);
}

//////////////////////////////////////////////////////////////////////

} // namespace CougarDevice|
