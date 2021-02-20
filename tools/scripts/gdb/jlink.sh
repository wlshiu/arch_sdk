#!/bin/bash -

set -e

TARGET_DEV=Cortex-M4
# TARGET_DEV=Cortex-M0
# TARGET_DEV=fa606te

JLinkExe -device ${TARGET_DEV} -if JTAG -speed 5000 -jtagconf -1,-1 -autoconnect 1

