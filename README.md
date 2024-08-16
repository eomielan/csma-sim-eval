# Medium Access and Random Backoff Simulator

## Overview

This program consists of a simulator that evaluates the performance of simplified Carrier Sense Multiple Access (CSMA) protocols in a wired network. When given an input file with configuration parameters, we run the simulation, then write the link utilization rate to an output file.

## Running the Simulator

Run the simulator program by entering the following into the command line, where inputFileName is the name of the input file, and outputFileName (optional) is the name of the output file:

```
./csma <inputFileName> [outputFileName]
```

Example:

```
./csma input.txt output.txt
```

Note: The input file must have the parameters listed below, each delimited by a new line. Note that the value(s) of the parameter must be separated by a space.

Parameters:

- number of nodes N
- the packet length L
- the minimum value of R (and how it increments)
- the value of M
- the total simulation time T

For an example input file, please refer to [input.txt](/input.txt).

## Testing

We used [Pytest](https://docs.pytest.org/en/8.0.x/), a Python testing framework, to test our code. These test files can be found in the `src/test` directory. To run the test suite, first ensure that you have Pytest installed, then do the following:

1. In the command line, navigate to the test directory using `cd src/test`

2. Run `pytest` to execute the test suite.

3. The results will be displayed on the console.

## Simulation Design

### Clock Ticks

The simulation uses a for-loop to simulate the passage of time. Each iteration of the loop represents one clock tick, and the simulation ends when the number of elapsed clock ticks equals the maximum simulation time. The maximum simulation time is passed in as T in the input file.

### Node Behaviour

On each clock tick, one of the following cases can occur:

**Case 1: Occupied channel, transmission in progress**

This occurs when a node is in the middle of transmitting a packet. During this phase, the channel is occupied and other nodes cannot transmit their packets. The backoff of other nodes are stopped and decremented only after the packet transmission is finished.

**Case 2: Unoccupied channel, nodes not ready**

No packets are being transmitted through the channel, but the backoff of all nodes are greater than zero, i.e. they must wait until they are ready to transmit a packet. During this phase, the backoff of all nodes are decremented.

**Case 3: Unoccupied channel, one node ready**

No packets are being transmitted through the channel, but the backoff of all nodes, except for one, are greater than zero. The node with backoff equal to zero is ready to transmit a packet. Upon seeing that the channel is idle, occupies the channel and starts the transmission.

**Case 4: Unoccupied channel, collision**

No packets are being transmitted through the channel, but two or more nodes are ready to transmit their packet. This means that a collision occurs as only one node is allowed to occupy the channel at a time. For all the nodes involved in the collision:

- collision count is incremented by one
- R is changed to the next value specified in the input file
- a new backoff is generated and assigned for the next transmission attempt

Note that when the number of collisions exceeds the maximum retries, denoted by M in the input file, the node drops the packet and attempts to transmit a new packet.

### Backoff

To ensure reproducibility, the following pseudorandom number generator to set the node's backoff time is used.

```
backoff = mod(nodeID + ticks, R)
```

where:

- `nodeID` and `ticks` both start from 0
- `R` is the backoff window

Note that the backoff is set in two main cases:

1. When a node has successfully completed its packet transmission, and needs a new backoff for its next packet.

2. When a node attempts to transmit a packet during the same clock tick as another node. In this case, both nodes involved in the collision must generate a new backoff.
