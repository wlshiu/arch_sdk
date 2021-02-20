#!/bin/bash
# Copyright (c) 2019, All Rights Reserved.
# @file    z_get_lib_version.sh
# @author  Wei-Lun Hsu
# @version 0.1

Red='\e[0;31m'
Yellow='\e[1;33m'
Green='\e[0;32m'
Cyan='\e[0;36m'
NC='\e[0m' # No Color

set -e

help()
{
    echo -e "usage: $0 [exec_objdump] [library-name]"
    exit 1;
}

if [ $# != 2 ]; then
    help
fi

OBJDUMP=$1
lib_name=$2

sha1=$(${OBJDUMP} -d ${lib_name} | grep -A 6 '_git_sha_id_' | grep '[.]word[[:space:]+]0x[a-fA-F0-9]*$' | awk '{print $(NF)}' | sed 's/0x//g')

date=$(${OBJDUMP} -d ${lib_name} | grep -A 6 '_git_commit_date_' | grep '[.]word[[:space:]+]0x[a-fA-F0-9]*$' | awk '{print $(NF)}' | sed 's/0x//g')

echo -e "${Yellow}${lib_name}${NC} Version: ${Green}sha1=${sha1}, date=${date}${NC}"