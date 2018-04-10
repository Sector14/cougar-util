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

    bool emulation_on = false;
    bool activate_user = false;
    std::string profile_filename;

    try
    {                
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
        SetCougarOptions(usb_device, activate_user, emulation_on);
    } 
    catch( const std::exception &e )
    {
        std::cout << "Error: " << e.what() << "\n";
        
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
