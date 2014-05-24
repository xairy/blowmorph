#-*- coding: utf-8 -*-
# Copyright (c) 2014 Blowmorph Team

# Game server script: it sends IP and pc name to the master server.

import requests
import socket

f = open("../../data/mclient.cfg")
lines = f.readlines()
f.close()

for line in lines:
    if len(line) >= 9 and line[0:9] == "  host = ":
        host = line[10:-3]
    if len(line) >= 9 and line[0:9] == "  port = ":
        port = line[9:-2]

address = "http://" + host + ":" + port
r = requests.post(address, params = {"name" : socket.gethostname()})
print "Message to master-server was sent."