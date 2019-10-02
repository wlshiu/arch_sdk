#!/bin/bash -

set -e

gdb_serv_pid=`ps -a | grep JLinkGDBServer | awk '{print $1}'`

if [ ! -z ${gdb_serv_pid} ]; then
    # echo ${gdb_serv_pid}
    sudo kill -2 ${gdb_serv_pid}
fi

TARGET_DEV=Cortex-M4
# TARGET_DEV=Cortex-M0

JLinkGDBServer -Device ${TARGET_DEV} -if JTAG -speed 5000 &
