#include "usbdevice.h"

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <string>

#include <libusb.h>

//////////////////////////////////////////////////////////////////////

USBDevice::USBDevice(uint16_t vendorID, uint16_t productID) : vendorID(vendorID), productID(productID)
{
    int err = libusb_init(nullptr);
    if (err)
        throw std::runtime_error(std::string("Failed to initialise libusb. ") + libusb_strerror(static_cast<libusb_error>(err)));
}

USBDevice::~USBDevice()
{
    for (const auto& interfaceNum : claimedInterfaces)
        libusb_release_interface(deviceHandle, interfaceNum);

    if (deviceHandle != nullptr)
        libusb_close(deviceHandle);

    libusb_exit(nullptr);
}

//////////////////////////////////////////////////////////////////////

void USBDevice::Open()
{
    assert(deviceHandle == nullptr && "Open called on already opened device");

    if (deviceHandle != nullptr)
        return;

    libusb_device **list;
    libusb_device *found = nullptr;

    try
    {
        ssize_t cnt = libusb_get_device_list(nullptr, &list);
        if (cnt < 0)
            throw std::runtime_error(libusb_strerror(static_cast<libusb_error>(cnt)));

        ssize_t i = 0;
        for (i = 0; i < cnt; i++) 
        {
            libusb_device *device = list[i];
            libusb_device_descriptor desc;
            libusb_get_device_descriptor(device, &desc);
            if (desc.idVendor == vendorID && desc.idProduct == productID)
            {
                found = device;
                break;
            }
        }

        if (! found)
            throw std::runtime_error("Unable to find usb device");
        
        int err = libusb_open(found, &deviceHandle);
        if (err)
            throw std::runtime_error(libusb_strerror(static_cast<libusb_error>(err)));
    }
    catch(const std::exception &e)
    {
        // free and unreference devices
        libusb_free_device_list(list, 1);
        throw;
    }
}

void USBDevice::Close()
{
    assert(deviceHandle != nullptr && "Close called on already closed device");

    if (deviceHandle != nullptr)
        libusb_close(deviceHandle);
    
    deviceHandle = nullptr;
}

void USBDevice::Reconnect()
{
    auto err = libusb_reset_device(deviceHandle);

    if (err == 0)
        return;

    // Device reset may fail causing deviceHandle to be invalidated and requiring re-open
    if (err = LIBUSB_ERROR_NOT_FOUND)
    {
        Close();
        Open();
    }
    else
        // Unhandled error
        throw std::runtime_error(libusb_strerror(static_cast<libusb_error>(err)));
}

//////////////////////////////////////////////////////////////////////

void USBDevice::ClaimInterface(int interfaceNum)
{
    if( claimedInterfaces.find(interfaceNum) != claimedInterfaces.end() )
        return;
    
    int err = libusb_claim_interface(deviceHandle, interfaceNum);
    if (err)
        throw std::runtime_error(libusb_strerror(static_cast<libusb_error>(err)));

    claimedInterfaces.insert(interfaceNum);
}

void USBDevice::ReleaseInterface(int interfaceNum)
{
    if( claimedInterfaces.find(interfaceNum) == claimedInterfaces.end() )
        return;
    
    libusb_release_interface(deviceHandle, interfaceNum);

    claimedInterfaces.erase(interfaceNum);
}

//////////////////////////////////////////////////////////////////////

void USBDevice::WriteBulkEP(const std::vector<unsigned char>& data, int endpoint)
{
    // NOTE: No attempt is made to check that correct interface has been claimed for the endpoint to write to.
    assert( ! claimedInterfaces.empty() && "Cannot write to endpoint without claiming interface first");
    assert( (endpoint & LIBUSB_ENDPOINT_IN) != LIBUSB_ENDPOINT_IN && "WriteBulkEP requires OUT endpoint for writing");

    int written = 0;
    int err = libusb_bulk_transfer(deviceHandle, endpoint, const_cast<unsigned char*>(data.data()), data.size(), &written, 1000);
    if (err)
        throw std::runtime_error(libusb_strerror(static_cast<libusb_error>(err)));
    if (written != data.size())
        throw std::runtime_error("WriteBulkEP only transferred " + std::to_string(written) + " bytes out of " + std::to_string(data.size()) );
}

std::vector<unsigned char> USBDevice::ReadBulkEP(size_t readSize, int endpoint)
{
    // NOTE: No attempt is made to check that correct interface has been claimed for the endpoint to write to.
    assert( ! claimedInterfaces.empty() && "Cannot write to endpoint without claiming interface first");
    assert( (endpoint & LIBUSB_ENDPOINT_IN) == LIBUSB_ENDPOINT_IN && "ReadBulkEP requires IN endpoint for reading");

    std::vector<unsigned char> data;
    data.resize(readSize);

    int read_count = 0;
    int err = libusb_bulk_transfer(deviceHandle, endpoint, const_cast<unsigned char*>(data.data()), data.size(), &read_count, 1000);
    if (err)
        throw std::runtime_error(libusb_strerror(static_cast<libusb_error>(err)));

    data.resize(read_count);

    return data;
}