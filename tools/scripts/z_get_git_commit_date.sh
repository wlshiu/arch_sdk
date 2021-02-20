#!/bin/bash -
# Copyright (c) 2019, All Rights Reserved.
# @file   z_get_git_commit_date.sh
# @author  Wei-Lun Hsu
# @version 0.1

set -e

if ! git status &> /dev/null ; then
    echo "00000000"
    exit 0;
fi

git log -1 --pretty=oneline --pretty=format:"%ci" | cut -d+ -f1 | gawk -F " " '{print $1}' | gawk -F "-" '{print $1$2$3}'
