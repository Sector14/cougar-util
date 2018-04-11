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

    // Ensure interface claimed prior to writing to any endpoints
    void WriteBulkEP(const std::vector<unsigned char>& data, int endpoint);

private:
    uint16_t vendorID;
    uint16_t productID;
    
    std::unordered_set<int> claimedInterfaces;

    libusb_device_handle *deviceHandle = nullptr;
};

#endif // USBDEVICE_H
