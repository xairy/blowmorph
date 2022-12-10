#!/bin/bash

set -eux

export LD_LIBRARY_PATH="`pwd`/bin"
valgrind --leak-check=full ./bin/client
