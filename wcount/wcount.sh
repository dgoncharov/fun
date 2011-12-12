#!/bin/bash

# This program reads the specified file and for every word in the file prints
# to stdout the number of occurences of that word in every line where the word
# is present.
#
# Distributed under the terms of the bsd license.
# Copyright (c) 2011 Dmtiry Goncharov (dgoncharov@users.sf.net).

if [[ $# != 1 ]]
then
    echo "usage: $0 <filename>"
    exit 1
fi

readonly src=$1

declare -A words

linenum=0
while read line 
do
    for w in $line
    do
        l=${w}lines
        c=${w}copies
        declare -a ${l}
        declare -a ${c}
        let ${l}[$linenum]=$linenum
        let ${c}[$linenum]+=1
        words[$w]=a
    done
    let linenum+=1
done < ${src}

for w in ${!words[@]}
do
    echo $w
    lns=${w}lines[@]
    declare -a lines=("${!lns}")

    cps=${w}copies[@]
    declare -a copies=("${!cps}")

    cnt=0
    for i in ${lines[@]}
    do
        echo "    ${copies[$cnt]} occurences in line $i"
        let cnt+=1
    done
done

