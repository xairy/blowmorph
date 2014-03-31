#!/bin/sh
cd bin
export LD_LIBRARY_PATH=`pwd`
cd ..
valgrind --leak-check=full ./bin/server
