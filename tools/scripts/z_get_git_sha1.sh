#!/bin/bash -
# Copyright (c) 2019, All Rights Reserved.
# @file    release_sdk.sh
# @author  Wei-Lun Hsu
# @version 0.1

set -e

help()
{
    echo -e "usage: $0 [path]"
    echo -e "   e.g. $0 ./foo"
    exit 1;
}

if [ $# != 1 ]; then
    help
fi

target_dir=$1
cur_dir=`pwd`


# if ! git ls-remote -q &> /dev/null ; then
if ! git status &> /dev/null ; then
    echo "00000000"
    exit 0;
fi


cd ${target_dir}
git describe --long --all --always --abbrev=8 | sed 's/.*-g\(.*\)/\1/'

cd ${cur_dir}
