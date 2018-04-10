# Cougar Util

Provides support for using a pre-configured Cougar HOTAS in Linux.

Pre-configured in this instance means, User axis mapping, manual calibration
and uploading of a tmj/tmm file (if required) should have been performed
in Windows using the HOTAS CCP and Foxy software.

Upon plugging in the (pre-configured) Cougar, it will be in a default
axis, no button emulation, automatic calibration mode, however user
profile, tmj/tmm and manual calibration data from prior configuration
(in Windows) is retained in the Cougar's flash. 

This utility allows setting three configuration options within the Cougar
to make use of the pre-configured data. Selection of default or user axis
profile, using manual or automatic calibration data and enabling/disabling
button/axis emulation.

In a way this utility provides similar functionality to that of the
HOTAS CCP application that runs during windows startup to configure the
device. It does not recreate the gui configuration abilities of HOTAS CCP
or indeed Foxy provides.

Handles only a single connected Cougar HOTAS. Connections will be made
to the first detected device in the case of several matching vid:pid devices.

# Usage

Run the utility with -h for help.

Running ./cougar-util with no options will configure the Cougar to the same
state it would be in if you reconnect the device. Default axis profile, no
emulation, automatic calibration.

Assuming your Cougar has been one-time configured in Windows, most users
will simply need to run this utility with the following switches:

  ./cougar-util -u -e -m

Which will activate the user profile mapping, enable button and axis emulation
and enable your manual calibration data.

cougar-util will need to be run as root in order to access the usb device.
Refer to the "Automatic Configuation" section to resolve to avoid this.

## Options

  -u    Selects "user" axis profile, otherwise the "default" profile is used.

Using a user profile is required if you've made changes to the axes curves or
swapped the RDR Cursor and toe break axis. The image below shows such a setup
in HOTAS CCP.

![RDR Cursor axis configuration](images/axis-config.png)

  -e    Turns button emulation on, otherwise button emulation is off.

Button emulation is only required if you're making use of a tmj/tmm file
which outputs key presses or axis values. When off, only regular DX button
presses will be generated. For example, you should enable button emulation
if you've loaded the dunc_replacement tmj/tmm file via Foxy which turns the
paddle switch into a dual function, autopilot override + wheel brake application.

  -m    Enables manual calibration mode, otherwise automatic calibration is used.

In automatic calibration mode, the Cougar will determine axis limits through
general usage of the device. Moving every axis through its full range of
motion and holding each axis at that limit for 3 seconds is sufficient to
calibrate.

Users of force mods such as the FSSB/FCC however will want to use manual
calibration mode. Otherwise the more pressure you place on the joystick
the further the pressure range will increase making it difficult to reliably
hit the same max limit.

You will need to perform a one-time manual calibration via HOTAS CCP in Windows.
After which using "-m on" will ensure the manual data is used. Note, it's
recommended to leave all axis as linear, do not apply curves, Falcon BMS
will handle this for you.

  -p    Allows a specified tmc axis profile to be uploaded to the Cougar's flash.

After configuring axes in HOTAS CCP and making curve/deadzone adjustments
plus performing a manually calibration, any TMC file you then save can be uploaded
from Linux using the above option.

NOTE: Uploaded TMC files include manual calibration data. You should only
use/upload TMC files you have generated yourself via HOTAS CCP if you intend
to use manual calibration mode.

# Other Notes

Access to Windows to perform manual calibration or to compile and upload/switch
tmj/tmm files is required.

Users with a working Cougar joystick that has no user axis profile uploaded
and have no access to Windows can upload the bundled config\falcon-rdr-cursor-on.tmc
(or off) profile via the -p switch.

This enables support for the RDR Cursor axis at the expense of the rudder toe brake
axes (see Falcon BMS Cougar setup docs for more information). Upload only needs to
be performed once. After that use the "-p user" option above.

NOTE: As tmc files include manual calibration data, unless you have stolen my
joystick you should leave the device in auto calibration mode if using the above tmc
files. In order to use manual calibration mode you should generate your
own tmc file in Windows.

# Usage Examples

Activate user axis profile (for RDR Cursor), button emulation mode for
use by currently uploaded tmj/tmm profile and manual calibration data.

  ./cougar-util -u -e -m

Activate manual calibration data. Button emulation is disabled and 
default axis mode selected.

  ./cougar-util -m on

Use a user axis mapping, button emulation and automatic calibration

  ./cougar-util -u -m

Upload and activate a custom user axis profile. Button emulation will be off
and calibration mode will be automatic.

  ./cougar-util -p config/rdr-cursor-on.tmc

Switch back to all default, automatic calibration, default axis, button 
emulation off.

  ./cougar-util

# Building

Requires libusb-1.0 and libusb-1.0-dev.

Run make in the root directory and copy the resulting cougar-util binary
to somewhere on your path. 

# Automatic Configuration

In most cases, upon connection of the Cougar you'll want to automatically
apply the user profile, manual calibration and emulation mode (or variation of)
without running the utility manually.

This can be done via... 

TODO: Udev rules to invoke on connection with -c user -e on as root.

# Thoughts on the Future

It would be nice to not need to use Windows at all to do the pre-configuration
but that cannot happen without further Linux utilities and additional feature
support in this utility to cover:-

  1. Firmware flashing
  2. Perform manual calibration
  3. Configure axis mappings and generate tmc file
  4. Upload pre-made tmc                                   (supported) 
  5. Activate user/default tmc profile                     (supported)
  6. Compile tmj/tmm files
  7. Upload compiled tmj/tmm
  8. Enable/disable Emulation mode                         (supported)
  9. Select manual or automatic calibration                (planned) 

Of the unsupported options:-

"1" - Flashing firmware is feasible to support but without tmj/tmm and manual
calibration support, would be of limited use.

"2" - I've not reversed the TMC file format yet which will be required
in order to make any use manual calibration data. Generating this data isn't
difficult for simple linear based limits. A possible future feature.

"3" - As with "2", TMC file format needs reversing. I'm not aware of any existing
Linux application to configure axis mappings and really a gui app would be of
limited use as generally the only need to make a custom tmc is to switch
rudder/toe brake axis mappings out with the RDR Cursor and such a tmc for that is bundled.

TMC reversing will however enable embedded manual calibration data to be updated
once "2" is supported.

"6" - Compilation of tmj/tmm is not going to be supported and I'm not aware
of any other application at this time for Linux or Windows that will compile
the Foxy tmj/tmm files and output a data file suitable for upload. This
likely involves a non trivial amount of work.

"7" - Uploading is pointless unless there is a way to easily produce suitable 
compiled data files at the very least via Windows. Foxy does not provide this.

A capture of the packet data for a specific tmj/tmm profile could be saved out
and bundled with this utility or you could save your own mappings out for various
games. But saving the files is quite user unfriendly and with no ability
to modify mappings for any bundled files, unlikely to be of great use. That said
upload support can be added should anyone really want this.

If anyone else creates an application that replicates Foxy's compilation process
and outputs to a data file. I will add support for uploading the result and at this
time a separate util to allow quick switching between different game tmj/tmm profiles
would also make sense.


# WARNING / DISCLAIMER

I have not properly reverse engineered the Cougar USB protocol and whilst I use
this application myself without issue, I take no responsibility nor provide any
guarantee that this utility will not cause any damage. Use at your own risk.

# Contact

Please let me know if you encounter any bugs and as always monetary support is
appreciated. Donations can be made via https://www.paypal.me/GPreston42

Gary Preston <gary@mups.co.uk>


