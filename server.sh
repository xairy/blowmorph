#!/bin/bash

set -eux

export LD_LIBRARY_PATH="`pwd`/bin"
./bin/server
