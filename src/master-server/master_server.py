#-*- coding: utf-8 -*-
# Copyright (c) 2014 Blowmorph Team

import time
import BaseHTTPServer
from pylibconfig import Config

game_servers = {}

class RequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    def do_POST(self):
        args = self.path.decode().split("/?")[1].split("&")
        args = dict(map(lambda x: tuple(x.split("=")), args))
        if not("name" in args.keys()) or \
           not("port" in args.keys()) or \
           not("active" in args.keys()):
            print "[%s] Error: wrong POST data from %s!" % \
              (str(time.asctime()), self.client_address[0])
            return
        else:
            gs_active = args["active"]
            gs_name = args["name"]
            gs_port = args["port"]
            gs_host = self.client_address[0]
            key = gs_host + ":" + gs_port
            if gs_active == "True":
                game_servers[key] = (gs_name, gs_host, gs_port)
            elif key in game_servers.keys():
                game_servers.pop(key, None)
            print game_servers
        self.send_response(200)
        
    def do_GET(self):
        self.wfile.write(str(game_servers))
        
if __name__ == '__main__':
    config = Config()
    config.readFile("data/master-server.cfg")
    port = config.value("master-server.port")
    assert port[1] == True
    port = port[0]

    http_server = BaseHTTPServer.HTTPServer(("0.0.0.0", port), RequestHandler)
    print "[%s] Server started on port %d." % (str(time.asctime()), port)
    try:
        http_server.serve_forever()
    except KeyboardInterrupt:
        pass
    http_server.server_close()
    print "[%s] Server stopped." % (str(time.asctime()),)
