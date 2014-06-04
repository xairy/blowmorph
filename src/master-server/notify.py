#-*- coding: utf-8 -*-
# Copyright (c) 2014 Blowmorph Team

# This script sends notifies the master server of a new game server run.

from __future__ import unicode_literals
import requests, socket, sys
from pylibconfig import Config

def load_config(config, name):
    value = config.value(name.encode("utf8"))
    assert value[1] == True
    value = value[0]
    if type(value) == str:
        value = value.decode("utf8")
    return value

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print "Usage: %s <active>" % (sys.argv[0],)
        exit(0)
    active = (sys.argv[1] == "true")

    config = Config()
    config.readFile("data/server.cfg".encode("utf8"))

    ms_host = load_config(config, "master-server.host")
    ms_port = load_config(config, "master-server.port")
    gs_name = load_config(config, "server.name")
    gs_port = load_config(config, "server.port")

    address = "http://%s:%d" % (ms_host, ms_port)
    try:
        r = requests.post(address, params = {"name" : gs_name, "port": gs_port, "active": active})
    except requests.exceptions.ConnectionError:
        print "Could not connect to master server %s:%d!" % (ms_host, ms_port)
    else:
        print "Master server %s:%d notified." % (ms_host, ms_port)
