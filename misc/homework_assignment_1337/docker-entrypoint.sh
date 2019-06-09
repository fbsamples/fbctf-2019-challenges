#!/bin/bash

set -m

KEY="fb{congr@ts_you_w1n_the_g@me}"

echo "Using KEY=${KEY}"

# Start the serivce
./thrift-challenge -server -flag \"${KEY}\" --addr 0.0.0.0:9090
