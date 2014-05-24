#!/bin/sh
cd bin
export LD_LIBRARY_PATH=`pwd`
cd ..
cd src/master-server/
python mclient.py
cd ../..
./bin/server
