#!/bin/sh
python src/master-server/notify.py
export LD_LIBRARY_PATH="`pwd`/bin"
./bin/server
