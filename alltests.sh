#!/bin/bash

end=500
zo=0
k=0
while [ $k -ne $end ]; do
    let "k=k+1"
#    export RPC_LOSSY=0
#    ./stop.sh
#    killall lock_server
#    ./lock_server 3772 > server.log &
#    ./lock_tester 3772 > tester.log
#    if [ $? -ne $zo ]; then
#        echo "wrong!"
#        exit 1
#    fi
#    killall lock_server
#    echo "--------------lock_teser with lossy 0----------- (1/6) ----------PASS------------ $k"
#    export RPC_LOSSY=5
#    killall lock_server
#    ./lock_server 3772 > server.log &
#    ./lock_tester 3772 > tester.log
#    if [ $? -ne $zo ]; then
#        echo "wrong!"
#        exit 1
#    fi
#    killall lock_server
#    echo "--------------lock_teser with lossy 5----------- (2/6) ----------PASS------------ $k"
#    ./stop.sh > o.o || ./stop.sh > o.o || ./stop.sh > o.o || ./stop.sh o.o
#    ./start.sh 0 > o.o
#    ./test-lab-1-a.pl ./yfs1 > o.o
#    if [ $? -ne $zo ]; then
#        echo "lab1a wrong!"
#        exit 1
#    fi
#    ./test-lab-1-b.pl ./yfs1 ./yfs2 > o.o
#    if [ $? -ne $zo ]; then
#        echo "lab1b wrong!"
#        exit 1
#    fi
#    ./test-lab-1-c.pl ./yfs1 > o.o
#    if [ $? -ne $zo ]; then
#        echo "lab1c wrong!"
#        exit 1
#    fi
#    echo "--------------lab1 with lossy 0----------------- (3/6) ----------PASS------------ $k"
#    ./stop.sh > o.o || ./stop.sh > o.o || ./stop.sh > o.o || ./stop.sh o.o
#    ./start.sh 5 > o.o
#    ./test-lab-1-a.pl ./yfs1 > o.o
#    if [ $? -ne $zo ]; then
#        echo "lab1a wrong!"
#        exit 1
#    fi
#    ./test-lab-1-b.pl ./yfs1 ./yfs2 > o.o
#    if [ $? -ne $zo ]; then
#        echo "lab1b wrong!"
#        exit 1
#    fi
#    ./test-lab-1-c.pl ./yfs1 > o.o
#    if [ $? -ne $zo ]; then
#        echo "lab1c wrong!"
#        exit 1
#    fi
#    echo "--------------lab1 with lossy 5----------------- (4/6) ----------PASS------------ $k"
    ./stop.sh
    ./stop.sh
    ./stop.sh
    ./stop.sh
    ./start.sh 0
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
    ./stop.sh
    ./stop.sh
    ./stop.sh
    ./stop.sh
    echo "--------------fs2 with lossy 0------------------- (5/6) ------------PASS------------ $k"
    ./stop.sh
    ./stop.sh
    ./stop.sh
    ./stop.sh
    ./start.sh 5
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
    ./stop.sh
    ./stop.sh
    ./stop.sh
    ./stop.sh
    echo "--------------fs2 with lossy 5------------------- (6/6) ------------PASS------------ $k"

done
echo "PASS all!"
exit 0
