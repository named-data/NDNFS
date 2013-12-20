#!/bin/bash

for i in `seq 1 100`;
do
    ../build/cat-file-pipe -n /ndn/ucla.edu/cs/irl/imac/ndnfs/boost.zip >> temp1.txt
done

(cat temp1.txt | grep Throughput) > ndnfs-pipe.txt

rm temp1.txt
