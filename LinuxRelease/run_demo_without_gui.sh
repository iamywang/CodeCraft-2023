#!/bin/sh
cd "$( dirname "$0" )"
v1=$(./Robot -f -l ERR /run/media/iamywang/Data/workspace/CodeCraft-2023/build/main -m maps/1.txt | jq -r '.score')
v2=$(./Robot -f -l ERR /run/media/iamywang/Data/workspace/CodeCraft-2023/build/main -m maps/2.txt | jq -r '.score')
v3=$(./Robot -f -l ERR /run/media/iamywang/Data/workspace/CodeCraft-2023/build/main -m maps/3.txt | jq -r '.score')
v4=$(./Robot -f -l ERR /run/media/iamywang/Data/workspace/CodeCraft-2023/build/main -m maps/4.txt | jq -r '.score')
echo $v1+$v2+$v3+$v4 | bc