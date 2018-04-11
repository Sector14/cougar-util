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

#ifndef USBDEVICE_H
#define USBDEVICE_H

#include <cstdint>
#include <unordered_set>
#include <vector>

//////////////////////////////////////////////////////////////////////
// Forwards
//////////////////////////////////////////////////////////////////////

struct libusb_device_handle;

//////////////////////////////////////////////////////////////////////
// USBDevice
//////////////////////////////////////////////////////////////////////

class USBDevice
{
public:
    USBDevice(uint16_t vendorID, uint16_t productID);
    ~USBDevice();

    // TODO: Delete copy/assign etc
    
    void Open();
    void Close();
    
    // Blocking call, may take several seconds to return
    void Reconnect();

    void ClaimInterface(int interfaceNum);
    void ReleaseInterface(int interfaceNum);

    // Ensure interface claimed prior to any endpoint I/O
    void WriteBulkEP(const std::vector<unsigned char>& data, int endpoint);
    std::vector<unsigned char> ReadBulkEP(size_t readSize, int endpoint);
    
private:
    uint16_t vendorID;
    uint16_t productID;
    
    std::unordered_set<int> claimedInterfaces;

    libusb_device_handle *deviceHandle = nullptr;
};

#endif // USBDEVICE_H
