#!/bin/bash

sigint_handler()
{
  python src/master-server/notify.py false
  exit $?
}

trap sigint_handler SIGINT

python src/master-server/notify.py true

export LD_LIBRARY_PATH="`pwd`/bin"
valgrind --leak-check=full ./bin/server
