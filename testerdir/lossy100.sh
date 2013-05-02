#!/bin/bash

i=0
end=100
zo=0
while [ $i -ne $end ]; do
    let "i=i+1"
    export RPC_LOSSY=0
    ../rpc/rpctest > o.o
    if [ $? -ne $zo ]; then
        echo "rpc wrong!"
        exit 1
    fi
    killall lock_server
    export RPC_LOSSY=5
    ./lock_server 3722 &
    ./lock_tester 3722 > o.o
    if [ $? -ne $zo ]; then
        echo "tester wrong!"
        exit 1
    fi
    echo "------------------------------------------------------------PASS------------ $i"
done
killall lock_server
echo "PASS!"
exit 0
