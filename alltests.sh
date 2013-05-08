#!/bin/bash

i=0
end=500
zo=0
#./stop.sh
#while [ $i -ne $end ]; do
#    let "i=i+1"
#    export RPC_LOSSY=0
#    ./rpc/rpctest > o.o
#    if [ $? -ne $zo ]; then
#        echo "rpc wrong!"
#        exit 1
#    fi
#    export RPC_LOSSY=5
#    ./lock_server 3722 > o2.o &
#    ./lock_tester 3722 > o.o
#    if [ $? -ne $zo ]; then
#        echo "tester wrong!"
#        exit 1
#    fi
#    killall lock_server
#    echo "------------------lock---------------------------PASS------------ $i"
#done
#echo "PASS lock!!"

#j=0
#while [ $j -ne $end ]; do
#    let "j=j+1"
#    ./start.sh > o.o
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
#    ./stop.sh > o.o
#    ./stop.sh > o.o
#    ./stop.sh > o.o
#    echo "--------------fs1-------------------------------PASS------------ $j"
#done

#z=0
#while [ $z -ne $end ]; do
#    let "z=z+1"
#    export RPC_LOSSY=5
#    ./start.sh > o.o
#    ./test-lab-2-a ./yfs1 ./yfs2 > o.o
#    if [ $? -ne $zo ]; then
#        echo "lab2a wrong!"
#        exit 1
#    fi
#    ./test-lab-2-b ./yfs1 ./yfs2 > o.o
#    if [ $? -ne $zo ]; then
#        echo "lab2b wrong!"
#        exit 1
#    fi
#    ./stop.sh > o.o
#    ./stop.sh > o.o
#    ./stop.sh > o.o
#    echo "--------------fs2-------------------------------PASS------------ $z"
#done
#echo "PASS fs!"
k=0
while [ $k -ne $end ]; do
    let "k=k+1"
    export RPC_LOSSY=5
    killall lock_server
    ./lock_server 3772 > server.log &
    ./lock_tester 3772 > tester.log
    if [ $? -ne $zo ]; then
        echo "wrong!"
        exit 1
    fi
    killall lock_server
    echo "--------------teser-------------------------------PASS------------ $k"
done
echo "PASS all!"
exit 0
