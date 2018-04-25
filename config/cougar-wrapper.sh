#!/bin/sh

# Check cougar-util is not already running (for example a
# reconnection during firmware update) and then set user
# defaults.

cougar_pid=`pgrep cougar-util`

if [ -z ${cougar_pid} ]; then 
  /usr/local/bin/cougar-util -u -m -e
fi
