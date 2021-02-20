#!/bin/bash -

set -e

jgdb_serv_pid=`ps -a | grep JLinkGDBServer | awk '{print $1}'`

help()
{
    echo "usage: $0 [CPU type]"
    echo "    CPU type      'm4' or 'm0'"
    exit 1;
}

if [ ! -z ${jgdb_serv_pid} ]; then
    # echo ${jgdb_serv_pid}
    sudo kill -2 ${jgdb_serv_pid}
fi

if [ $# != 1 ]; then
    help
fi

if [ "$1" = "m4" ]; then
    TARGET_DEV=Cortex-M4
elif [ "$1" = "m0" ]; then
    TARGET_DEV=Cortex-M0
elif [ "$1" = "fa606te" ]; then
    TARGET_DEV=fa606te
fi

JLinkGDBServer -Device ${TARGET_DEV} -if JTAG -speed 5000 &
