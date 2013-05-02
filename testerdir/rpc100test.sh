#!/bin/bash

i=0
end=100
zo=0
while [ $i -ne $end ]; do
    let "i=i+1"
    ../rpc/rpctest > o.o
    if [ $? -ne $zo ]; then
        exit 1
    fi
    echo "------------------------------------------------------------PASS------------ $i"
done
echo "PASS!"
exit 0
