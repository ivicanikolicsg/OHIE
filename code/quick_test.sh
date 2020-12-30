#!/bin/bash
make
# kill all nodes
# fuser -k nodeouput*

# remove created folders (if any)
rm -rf _Blockchains
rm -rf _Blocks
rm -rf _Hashes
rm -rf _Pings
rm -rf _Sessions

# remove previous outputs
rm outputnode*

# start 3 nodes
nohup  ./Node 8091 _peers 127.0.0.1 > outputnode1.txt &
nohup  ./Node 8092 _peers 127.0.0.1 > outputnode2.txt &
nohup  ./Node 8093 _peers 127.0.0.1 > outputnode3.txt &
