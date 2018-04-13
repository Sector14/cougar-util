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

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unistd.h>

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
    std::cout << "  -m \tUse manual calibration data (implies -u)\n\n";
    std::cout << "  -p FILE\tUpload a tmc user profile (implies -u)\n";
    std::cout << "  -t FILE\tUpload a compiled tjm binary\n";
    std::cout << "  -f FILE\tUpload new firmware to Cougar\n";
    std::cout << "Defaults: default axis profile, no emulation, auto calibration.\n";
}

int main( int argc, char *argv[])
{
    std::string profile_filename;
    std::string tjmbin_filename;
    std::string firmware_filename;
    CougarOptions cougar_options = CougarOptions::Defaults;

    try
    {                
        int opt;

        // Args with default error message disabled
        while ((opt = getopt(argc, argv, ":euf:p:t:hm")) != -1)
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
                case 'f':
                    firmware_filename = optarg;
                    break;
                case 'p':
                    profile_filename = optarg;
                    cougar_options = cougar_options | CougarOptions::UserProfile;
                    break;
                case 't':
                    tjmbin_filename = optarg;
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
        if (! firmware_filename.empty())
        {
            std::cout << "********************************************************************************\n"
                         "*    WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING   *\n"
                         "********************************************************************************\n\n"
                         "Thrustmaster recommend you DISCONNECT your THROTTLE from the Cougar before "
                         "uploading new firmware. Users have reported permanent damage to the throttle "
                         "if left connected. Firmware update is at your own risk.\n\n";
            std::cout << "Before proceeding, unplug your Cougar (and disconnect your throttle) then whilst holding "
                         "down the trigger, plug your Cougar back in. Keep the trigger held down for "
                         "at least four seconds after connection to wipe any existing firmware. Then release the trigger "
                         "and wait a few more seconds for Linux to re-detect the device.\n\n";
            std::cout << "Proceed with firmware upload? (y/n): ";

            std::string temp;
            std::getline(std::cin, temp);
            if (temp != "y")
            {
                std::cout << "Aborting\n";
                return EXIT_FAILURE;
            }
        }

        USBDevice usb_device(CougarDevice::cCougarVID, CougarDevice::cCougarPID);
        usb_device.Open();
        usb_device.ClaimInterface(CougarDevice::cCougarInterfaceBulkOut);
        usb_device.ClaimInterface(CougarDevice::cCougarInterfaceBulkIn);

        // Options can be chained but always complete in priority of firmware, profile, tjm, options
        if (! firmware_filename.empty())
        {
            std::cout << "Uploading firmware. This may take several seconds to complete...\n" << std::flush;
            CougarDevice::UploadFirmware(usb_device, firmware_filename);

            std::cout << "\nFirmware upload complete. "
                         "Please disconnect your Cougar, re-attach the throttle and reconnect. Wait a few seconds "
                         "for device detection then move each axis through its full range of motion, "
                         "holding at each limit for 3 seconds to allow auto calibration to work.\n\n";
            std::cout << "Once you have completed auto calibration, press ENTER exit.\n";
            
            // Wait for user to complete auto calibration. This prevents the udev rules kicking in
            // when the device is reconnected after attaching the throttle. This is important as the
            // udev rule will (unless user modified) switch to manual calibration.            
            std::string temp;
            std::getline(std::cin, temp);
            
            return EXIT_SUCCESS;            
        }
        
        if (! profile_filename.empty())
            CougarDevice::UploadProfile(usb_device, profile_filename);

        if (! tjmbin_filename.empty())
            CougarDevice::UploadTMJBinary(usb_device, tjmbin_filename);

        CougarDevice::SetCougarOptions(usb_device, cougar_options);
    } 
    catch( const std::exception &e )
    {
        std::cout << "Error: " << e.what() << "\n";
        
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
