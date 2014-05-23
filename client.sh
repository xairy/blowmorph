#!/bin/sh

BASEDIR=$(dirname $0)

# '../../.' is relative path to the launcher location.
if [ "$BASEDIR" = '../../.' ]; then
	cd ../../bin
else
  cd bin
fi

export LD_LIBRARY_PATH=`pwd`
cd ..
./bin/client
