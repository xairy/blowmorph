#-*- coding: utf-8 -*-
# Copyright (c) 2014 Blowmorph Team

from pylibconfig import Config
import time
import BaseHTTPServer

config = Config()
config.readFile("data/master-server.cfg")
port = config.value("master-server.port")
assert port[1] == True
port = port[0]

game_servers = {}

class RequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    def do_POST(self):
        args = self.path.decode().split("/?")
        args = args[1].split("&")
        args = map(lambda x: tuple(x.split("=")), args)
        args = dict(args)
        if not("name" in args.keys()) or not("port" in args.keys()) or not("active" in args.keys()):
            print "Error: wrong POST data"
            return
        else:
            active = args["active"]
            server_name = args["name"]
            server_port = args["port"]
            server_host = self.client_address[0]
            key = server_host + ":" + server_port
            if active == "True":
                game_servers[key] = (server_name, server_host, server_port)
            elif key in game_servers.keys():
                game_servers.pop(key, None)
            print game_servers
            
        self.send_response(200)
        
    def do_GET(self):
        self.wfile.write(str(game_servers))
        
if __name__ == '__main__':
    http_server = BaseHTTPServer.HTTPServer(("0.0.0.0", port), RequestHandler)
    print "[%s] Server started on port %d." % (str(time.asctime()), port)
    try:
        http_server.serve_forever()
    except KeyboardInterrupt:
        pass
    
    http_server.server_close()
    
    print "[%s] Server stopped." % (str(time.asctime()),)
