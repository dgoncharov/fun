#!/bin/bash

cc -o perm perm.c
[[ 0 == $? ]] || exit 1
n=$(./perm 12345 | sort -u |wc -l)
if [[ 120 != $n ]]; then
    echo failure $n != 120
    exit 1
fi
n=$(./perm 6214375 | sort -u |wc -l)
if [[ 5040 != $n ]]; then
    echo failure $n != 5040
    exit 1
fi
exit 0

