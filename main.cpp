// Supports uploading tmc user profile to connected Cougar HOTAS
// as well as activating a pre-existing profile and setting
// button emulation on/off.

#include <iostream>
#include <unistd.h>
#include <exception>
#include <string>
#include <cstring>
#include <cstdlib>

#include <libusb.h>

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

void UploadProfile(libusb_device_handle *dev, const std::string& filename)
{

}

void SetCougarOptions(libusb_device_handle *dev, bool user_profile, bool emu_on)
{
    int err;
    unsigned char data[2];

    data[0] = 4;
    data[1] = 2;
    err = libusb_bulk_transfer(dev, cCougarEndpointBulkOut, data, 2, nullptr, 1000);
    if (err)
        throw std::runtime_error(libusb_strerror(static_cast<libusb_error>(err)));

    // Disabling emulation mode appears to work without 0x7, windows CCP sends this, so we will too.
    if ( ! emu_on )
    {
        data[0] = 7;
        err = libusb_bulk_transfer(dev, cCougarEndpointBulkOut, data, 1, nullptr, 1000);
        if (err)
            throw std::runtime_error(libusb_strerror(static_cast<libusb_error>(err)));
    }

    data[0] = 3;
    data[1] = ((emu_on ? 1 : 0) << 1) | (user_profile ? 1 : 0);
    std::cout << "Sending emu/profile: " << static_cast<int>(data[1]) << "\n";
    err = libusb_bulk_transfer(dev, cCougarEndpointBulkOut, data, 2, nullptr, 1000);        
    if (err)
        throw std::runtime_error(libusb_strerror(static_cast<libusb_error>(err)));        
}

//////////////////////////////////////////////////////////////////////
// LibUSB
//////////////////////////////////////////////////////////////////////

// returns NULL if no valid device found.
libusb_device_handle* open_device( uint16_t vendorID, uint16_t productID )
{
    libusb_device **list;
    libusb_device *found = nullptr;
    libusb_device_handle *handle = nullptr;

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

        if (found) 
        {
            int err = libusb_open(found, &handle);
            if (err)
                throw std::runtime_error(libusb_strerror(static_cast<libusb_error>(err)));
        }
    }
    catch(const std::exception &e)
    {
        // free and unreferences devices
        libusb_free_device_list(list, 1);
        throw;
    }
    
    return handle;
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

    int err = libusb_init(nullptr);
    if (err)
    {
        std::cout << "Failed to initialise libusb" << libusb_strerror(static_cast<libusb_error>(err));
        return EXIT_FAILURE;
    }

    libusb_device_handle *dev = nullptr;

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

        dev = open_device(cCougarVID, cCougarPID);
        if (dev == nullptr)
            throw std::runtime_error("Cougar device not found.");

        err = libusb_claim_interface(dev, cCougarInterfaceBulkOut);
        if (err)
            throw std::runtime_error(libusb_strerror(static_cast<libusb_error>(err)));

        // Execute the cmdline options
        if (! profile_filename.empty())
             UploadProfile(dev, profile_filename);
        SetCougarOptions(dev, activate_user, emulation_on);

        libusb_release_interface(dev, cCougarInterfaceBulkOut);
    } 
    catch( const std::exception &e )
    {
        std::cout << "Error: " << e.what() << "\n";
        
        // TODO: Duplicated cleanup! Wrap libusb resources to avoid manual lifetime handling
        if (dev != nullptr)
            libusb_close(dev);
    
        libusb_exit(nullptr);

        return EXIT_FAILURE;
    }

    // Cleanup
    if (dev != nullptr)
        libusb_close(dev);

    libusb_exit(nullptr);

    return EXIT_SUCCESS;
}