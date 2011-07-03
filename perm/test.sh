#!/bin/bash

cc -o perm perm.c
[[ 0 == $? ]] || exit 1
n=$(./perm 1 | sort -u |wc -l)
if [[ 1 != $n ]]; then
    echo failure $n != 1
    exit 1
fi
n=$(./perm 12 | sort -u |wc -l)
if [[ 2 != $n ]]; then
    echo failure $n != 2
    exit 1
fi
n=$(./perm 6214375 | sort -u |wc -l)
if [[ 5040 != $n ]]; then
    echo failure $n != 5040
    exit 1
fi
exit 0

