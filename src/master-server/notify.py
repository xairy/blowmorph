#-*- coding: utf-8 -*-
# Copyright (c) 2014 Blowmorph Team

# This script sends notifies the master server of a new game server run.

import requests
import socket
import sys
from pylibconfig import Config

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print "Usage: %s <active>" % (sys.argv[0],)
        exit(0)
    active = (sys.argv[1] == "true")

    config = Config()
    config.readFile("data/server.cfg")

    ms_host = config.value("master-server.host")
    ms_port = config.value("master-server.port")
    assert (ms_host[1] == True) and (ms_port[1] == True)
    ms_host, ms_port = ms_host[0], ms_port[0]

    gs_name = config.value("server.name")
    gs_port = config.value("server.port")
    assert (gs_name[1] == True) and (gs_port[1] == True)
    gs_name, gs_port = gs_name[0], gs_port[0]

    address = "http://" + ms_host + ":" + str(ms_port)
    try:
        r = requests.post(address, params = {"name" : gs_name, "port": gs_port, "active": active})
    except requests.exceptions.ConnectionError:
        print "Could not connect to master server %s:%d!" % (ms_host, ms_port)
    else:
        print "Master server %s:%d notified." % (ms_host, ms_port)
