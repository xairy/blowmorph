#-*- coding: utf-8 -*-
# Copyright (c) 2015 Blowmorph Team

# This script sends notifies the master server of a new game server being run.

from __future__ import unicode_literals
import json, requests, socket, sys

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print "Usage: %s <active>" % (sys.argv[0],)
        exit(0)
    active = (sys.argv[1] == "true")

    config_file = open('data/server.json')
    config = json.loads(config_file.read())
    config_file.close()

    ms_host = config["master-server"]["host"]
    ms_port = config["master-server"]["port"]
    gs_name = config["server"]["name"]
    gs_port = config["server"]["port"]

    address = "http://%s:%d" % (ms_host, ms_port)
    try:
        r = requests.post(address, params = {"name" : gs_name, "port": gs_port, "active": active})
    except requests.exceptions.ConnectionError:
        print "Couldn't connect to master server %s:%d!" % (ms_host, ms_port)
    else:
        print "Master server %s:%d notified." % (ms_host, ms_port)
