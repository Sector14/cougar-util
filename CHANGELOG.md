# Cougar Util - Change Log

Breaking changes to the public functionality of this utility (e.g command line args)
may be made without warning until version 1.0. From version 1.0 any breaking changes
will include a major version bump.

This is a summary of the main changes. A complete history of all changes can be
found in the [project git repository](https://bitbucket.org/BWGaryP/cougar-util).

This project uses [Semantic Versioning](https://semver.org/).

## [Unreleased]
### Added
   - Add "-f" option to extract firmware from HOTASUpdate.exe and flash Cougar.   
   - Add "-t" option to upload pre-compiled TMJ binaries.
   - libcrypto++6 build dependency
   
### Fixed
   - dunc_dx_replacement.bin incorrectly used dunc_dx.tmm file.

### Changed
   - UDev rules updated to prevent auto configuration if cougar-util running.
   - README changed to include new usage notes.
   - Build now requires openssl

## [0.1.0] - 2018-04-12
### Added
   - Initial release of cougar-util
   - GPL3 license source code
   - Support uploading user profiles (tcm) to device
   - Support enabling custom user profile, button/axis emulation and manual
     calibration modes.
