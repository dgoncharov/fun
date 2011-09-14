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

echo one >> /tmp/1
echo two >> /tmp/1
echo two >> /tmp/1
echo three >> /tmp/1
echo three >> /tmp/1
echo three >> /tmp/1


declare -A words
#declare -A lines

linenum=1
while read line 
do
    for w in $line
    do
        cnt=${words[$w]}
        let cnt+=1
        words[$w]=$cnt
    done
    let linenum+=1
done < /tmp/1
for k in ${!words[@]}
do
    echo '${words['$k']}='${words[$k]}
done

