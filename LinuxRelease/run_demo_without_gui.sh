#!/bin/sh
cd "$( dirname "$0" )"
# ./Robot -f -l ERR ../build/main -m maps/1.txt
# ./Robot -f -l ERR ../build/main -m maps/2.txt
# ./Robot -f -l ERR ../build/main -m maps/3.txt
# ./Robot -f -l ERR ../build/main -m maps/4.txt
./Robot -f -l ERR ../build/main -m maps/map0.txt
./Robot -f -l ERR Demo/SimpleDemo -m maps/map0.txt