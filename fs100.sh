#!/bin/bash

i=0
end=100
zo=0
while [ $i -ne $end ]; do
    let "i=i+1"
    ./start.sh > o.o
    ./test-lab-1-a.pl ./yfs1 > o.o
    if [ $? -ne $zo ]; then
        echo "lab1a wrong!"
        exit 1
    fi
    ./test-lab-1-b.pl ./yfs1 ./yfs2 > o.o
    if [ $? -ne $zo ]; then
        echo "lab1b wrong!"
        exit 1
    fi
    ./test-lab-1-c.pl ./yfs1 > o.o
    if [ $? -ne $zo ]; then
        echo "lab1c wrong!"
        exit 1
    fi
    ./test-lab-2-a ./yfs1 ./yfs2 > o.o
    if [ $? -ne $zo ]; then
        echo "lab2a wrong!"
        exit 1
    fi
    ./test-lab-2-b ./yfs1 ./yfs2 > o.o
    if [ $? -ne $zo ]; then
        echo "lab2b wrong!"
        exit 1
    fi
    ./stop.sh > o.o
    echo "------------------------------------------------------------PASS------------ $i"
done
echo "PASS!"
exit 0
