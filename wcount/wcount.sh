#!/bin/bash

#count[2]=1
#count[3]=4
#r=count
#words[gore]=r
#
#ref=$(eval echo \$${words[gore]})
#echo ${ref}
#eval echo \${$ref[@]}
#
#
#arr=(one two three)
#ref=arr
#tab[gore]=ref
#a=$(eval echo \$${tab[gore]})
#eval echo \${$a[*]}
#
#eval set \${$a[3]=four}
#eval set \${$a[4]=five}
#eval echo \${$a[@]}

# {word1 => {linenum1 => noccurences1, linenum2 => noccurences2}, word2 =>
# {linenum1 => noccurences3}}

echo one one one one >> /tmp/1
echo two >> /tmp/1
echo two >> /tmp/1
echo three >> /tmp/1
echo three >> /tmp/1
echo three >> /tmp/1


declare -A words
declare -a lines

linenum=1
while read line 
do
    for w in $line
    do
        cnt=${lines[$linenum]}
        let cnt+=1
        lines[$linenum]=$cnt
        words[$w]=lines
#        words[gore]=lines
    done
    let linenum+=1
done < /tmp/1

#testlines=(6 7 8)
#words[gore]=testlines
#r=${words[gore]}
#for n in $(eval echo \${!$r[@]})
#do
#    echo -n '$words[gore]['$n']='
#    eval echo \${$r[$n]}
#done

for k in ${!words[@]}
do
#    echo '${words['$k']}='${words[$k]}
    r=${words[$k]}
    for n in $(eval echo \${!$r[@]})
    do
        echo -n '$words['$k']['$n']='
        eval echo \${$r[$n]}
    done
done

