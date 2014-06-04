#-*- coding: utf-8 -*-
# Copyright (c) 2014 Blowmorph Team

from __future__ import unicode_literals
import time, BaseHTTPServer
from pylibconfig import Config

game_servers = {}

def load_config(config, name):
    value = config.value(name.encode("utf8"))
    assert value[1] == True
    value = value[0]
    if type(value) == str:
        value = value.decode("utf8")
    return value

class RequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    def do_POST(self):
        try:
            args = self.path.decode().split("/?")[1].split("&")
            args = dict(map(lambda x: tuple(x.split("=")), args))
            gs_active = args["active"]
            gs_name = args["name"]
            gs_port = int(args["port"])
            gs_host = unicode(self.client_address[0])
        except:
            print "[%s] Error: wrong POST data from %s!" % \
              (str(time.asctime()), self.client_address[0])
            return

        key = "%s:%d" % (gs_host, gs_port)
        if gs_active == "True":
            game_servers[key] = {
              "name": gs_name,
              "host": gs_host,
              "port": gs_port
            }
        elif key in game_servers.keys():
            game_servers.pop(key, None)
        print game_servers

        self.send_response(200)
        
    def do_GET(self):
        self.wfile.write(str(game_servers))
        
if __name__ == '__main__':
    config = Config()
    config.readFile("data/master-server.cfg".encode("utf8"))
    port = load_config(config, "master-server.port")

    http_server = BaseHTTPServer.HTTPServer(("0.0.0.0", port), RequestHandler)
    print "[%s] Server started on port %d." % (str(time.asctime()), port)
    try:
        http_server.serve_forever()
    except KeyboardInterrupt:
        pass
    http_server.server_close()
    print "[%s] Server stopped." % (str(time.asctime()),)
