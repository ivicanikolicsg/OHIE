# OHIE - Blockchain scaling

The repository contains C++ implementation of OHIE. 
The technical aspects of the approach are described in [our paper](https://arxiv.org/pdf/1811.12628.pdf). 

## Dependencies

The code has been tested on Ubuntu 16.04 with Boost ASIO library installed:

	sudo apt-get install libboost-all-dev

## Quick Test

1. Compile the code `make` 
2. Run the script `quick_test.sh`

This will run OHIE network of 3 nodes -- their outputs are written in `outputnodeX.txt`. So, while running, for example, check the output with `tail -f outputnode1.txt`. 
At the end, make sure to kill the network, i.e. `fuser -k *`.

## Parameters

There are many parameters that can be configured, starting form the IP address of the nodes, to number of chains, block sizes, mining times, etc: 
- For most widely used parameters, check the file `_configuration`. 
- For a full list of parameters, check `configuration.cpp`. 
- The list of network nodes (ip:port) is defined in a separate file, check `_peers`
- To start a single node use 
```
./Node <portNumber> <file_peers> 
```
