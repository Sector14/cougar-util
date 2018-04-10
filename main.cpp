// Supports uploading tmc user profile to connected Cougar HOTAS
// as well as activating a pre-existing profile and setting
// button emulation on/off.

#include <iostream>
#include <unistd.h>
#include <stdexcept>
#include <string>
#include <cstring>
#include <cstdlib>
#include <type_traits>

#include "usbdevice.h"
#include "cougardevice.h"

using CougarOptions = CougarDevice::CougarOptions;

//////////////////////////////////////////////////////////////////////
// Main/Usage
//////////////////////////////////////////////////////////////////////

static void PrintUsage(const char* appName)
{
    std::cout << "Usage: " << appName << " [-u|-e|-m] [-p FILE]\n";
    std::cout << "Options:\n";
    std::cout << "  -u \tActivate user axis profile\n";
    std::cout << "  -e \tEnable Button/Axis emulation mode\n";        
    std::cout << "  -m \tUse manual calibration data (implies -u)\n";
    std::cout << "  -p FILE\tUpload a tmc user profile (implies -u)\n";
    std::cout << "Defaults: default axis profile, no emulation, auto calibration.\n";
}

int main( int argc, char *argv[])
{
    std::string profile_filename;
    CougarOptions cougar_options = CougarOptions::Defaults;

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
                    cougar_options = cougar_options | CougarOptions::ManualCalibration | CougarOptions::UserProfile;
                    break;
                case 'e':
                    cougar_options = cougar_options | CougarOptions::ButtonAxisEmulation;
                    break;
                case 'p':
                    profile_filename = optarg;
                    cougar_options = cougar_options | CougarOptions::UserProfile;                    
                    break;
                case 'u':
                    cougar_options = cougar_options | CougarOptions::UserProfile;
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
        USBDevice usb_device(CougarDevice::cCougarVID, CougarDevice::cCougarPID);
        usb_device.Open();
        usb_device.ClaimInterface(CougarDevice::cCougarInterfaceBulkOut);

        if (! profile_filename.empty())
             CougarDevice::UploadProfile(usb_device, profile_filename);
        
        CougarDevice::SetCougarOptions(usb_device, cougar_options);
    } 
    catch( const std::exception &e )
    {
        std::cout << "Error: " << e.what() << "\n";
        
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
