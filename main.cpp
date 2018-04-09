// Supports uploading tmc user profile to connected Cougar HOTAS
// as well as activating a pre-existing profile and setting
// button emulation on/off.

#include <iostream>
#include <unistd.h>
#include <stdexcept>
#include <string>
#include <cstring>
#include <cstdlib>

#include <libusb.h>

#include "usbdevice.h"

//////////////////////////////////////////////////////////////////////

const uint16_t cCougarVID = 0x044f;
const uint16_t cCougarPID = 0x0400;

const int cCougarInterfaceBulkOut = 3;
const int cCougarInterfaceBulkIn  = 4;

const int cCougarEndpointBulkOut = 4 | LIBUSB_ENDPOINT_OUT;
const int cCougarEndpointBulkIn  = 5 | LIBUSB_ENDPOINT_IN;

//////////////////////////////////////////////////////////////////////
// Cougar Helpers
//////////////////////////////////////////////////////////////////////

void UploadProfile(USBDevice &dev, const std::string& filename)
{

}

void SetCougarOptions(USBDevice &dev, bool user_profile, bool emu_on)
{
    int err;
    unsigned char data[2];

    data[0] = 4;
    data[1] = 2;
    err = libusb_bulk_transfer(dev.DeviceHandle(), cCougarEndpointBulkOut, data, 2, nullptr, 1000);
    if (err)
        throw std::runtime_error(libusb_strerror(static_cast<libusb_error>(err)));

    // Disabling emulation mode appears to work without 0x7, windows CCP sends this, so we will too.
    if ( ! emu_on )
    {
        data[0] = 7;
        err = libusb_bulk_transfer(dev.DeviceHandle(), cCougarEndpointBulkOut, data, 1, nullptr, 1000);
        if (err)
            throw std::runtime_error(libusb_strerror(static_cast<libusb_error>(err)));
    }

    data[0] = 3;
    data[1] = ((emu_on ? 1 : 0) << 1) | (user_profile ? 1 : 0);
    std::cout << "Sending emu/profile: " << static_cast<int>(data[1]) << "\n";
    err = libusb_bulk_transfer(dev.DeviceHandle(), cCougarEndpointBulkOut, data, 2, nullptr, 1000);        
    if (err)
        throw std::runtime_error(libusb_strerror(static_cast<libusb_error>(err)));        
}


//////////////////////////////////////////////////////////////////////
// Main/Usage
//////////////////////////////////////////////////////////////////////

static void PrintUsage(const char* appName)
{
    std::cout << "Usage: " << appName << " -e -c [-u]\n";
    std::cout << "Required:\n";
    std::cout << "  -e on|off\tSet Button/Axis emulation mode\n";        
    std::cout << "  -c user|default\tActivate user or default axis profile\n";
    std::cout << "Optional:\n";
    std::cout << "  -u FILE\tUpload a tmc user profile\n";
}

int main( int argc, char *argv[])
{
    if (argc <= 1)
    {
        PrintUsage(argv[0]);
        return EXIT_FAILURE;
    }

    try
    {                
        bool emulation_on = false;
        bool activate_user = false;
        std::string profile_filename;

        // Disable default error message
        int opt;

        while ((opt = getopt(argc, argv, ":e:u:c:")) != -1)
        {
            switch (opt)
            {
                case 'e':
                    if (strcmp(optarg, "on") != 0 && strcmp(optarg,"off") != 0) 
                        throw std::invalid_argument("-e option accepts 'on' or 'off'");
                    emulation_on = strcmp(optarg, "on") == 0;
                    break;
                case 'u':
                    profile_filename = optarg;
                    break;
                case 'c':
                    if (strcmp(optarg, "user") != 0 && strcmp(optarg,"default") != 0) 
                        throw std::invalid_argument("-c option accepts 'user' or 'default'");
                    activate_user = strcmp(optarg, "user") == 0;
                    break;
                case '?':
                    PrintUsage(argv[0]);
                    throw std::invalid_argument(std::string("Invalid option -") + static_cast<char>(optopt));
                case ':':
                    PrintUsage(argv[0]);
                    throw std::invalid_argument(std::string("Option -") + static_cast<char>(optopt) + " missing argument");  
            }
        }

        USBDevice usb_device(cCougarVID, cCougarPID);
        usb_device.Open();
        usb_device.ClaimInterface(cCougarInterfaceBulkOut);

        // Execute the cmdline options
        if (! profile_filename.empty())
             UploadProfile(usb_device, profile_filename);
        SetCougarOptions(usb_device, activate_user, emulation_on);
    } 
    catch( const std::exception &e )
    {
        std::cout << "Error: " << e.what() << "\n";
        
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
