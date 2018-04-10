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
    // TODO: Implement me
}

// TODO: Change to enum use for user_profile
void SetCougarOptions(USBDevice &dev, bool user_profile, bool emu_on)
{
    dev.WriteBulkEP({4,2}, cCougarEndpointBulkOut);
    if (! emu_on)
        dev.WriteBulkEP({7}, cCougarEndpointBulkOut);

    unsigned char options_bm = ((emu_on ? 1 : 0) << 1) | (user_profile ? 1 : 0);
    dev.WriteBulkEP({3, options_bm}, cCougarEndpointBulkOut);    
}

//////////////////////////////////////////////////////////////////////
// Main/Usage
//////////////////////////////////////////////////////////////////////

static void PrintUsage(const char* appName)
{
    std::cout << "Usage: " << appName << " [-u|-e|-m] [-p FILE]\n";
    std::cout << "Options:\n";
    std::cout << "  -u \tActivate user axis profile\n";
    std::cout << "  -e \tEnable Button/Axis emulation mode\n";        
    std::cout << "  -m \tUse manual calibration data\n"
    std::cout << "  -p FILE\tUpload a tmc user profile\n";
    std::cout << "Defaults: default axis profile, no emulation, auto calibration.\n";
}

int main( int argc, char *argv[])
{
    bool manual_calibration = false;
    bool button_emulation = false;
    bool user_profile = false;
    std::string profile_filename;

    try
    {                
        // Disable default error message
        int opt;

        while ((opt = getopt(argc, argv, ":eup:hm")) != -1)
        {
            switch (opt)
            {
                case 'h':
                    PrintUsage(argv[0]);
                    return EXIT_SUCCESS;
                case 'm':
                    manual_calibration = true;
                    break;
                case 'e':
                    button_emulation = true;
                    break;
                case 'p':
                    profile_filename = optarg;
                    break;
                case 'u':
                    user_profile = true;
                    break;
                case '?':
                    throw std::invalid_argument(std::string("Invalid option -") + static_cast<char>(optopt));
                case ':':
                    throw std::invalid_argument(std::string("Option -") + static_cast<char>(optopt) + " missing argument");  
            }
        }
    }
    catch( const std::invalid_argument &e )
    {
        std::cout << "Error: " << e.what() << "\n\n";
        PrintUsage(argv[0]);
        
        return EXIT_FAILURE;
    }

    try
    {
        USBDevice usb_device(cCougarVID, cCougarPID);
        usb_device.Open();
        usb_device.ClaimInterface(cCougarInterfaceBulkOut);

        if (! profile_filename.empty())
             UploadProfile(usb_device, profile_filename);
        // TODO: Manual calibration enable support
        // TODO: Switch to options enum mask
        SetCougarOptions(usb_device, user_profile, button_emulation);
    } 
    catch( const std::exception &e )
    {
        std::cout << "Error: " << e.what() << "\n";
        
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
