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
        args = map(lambda x: x.split("="), args)
        if args[0][0] != "name" or args[1][0] != "port":
            print "Error: wrong POST data"
            return
        else:
            server_name = args[0][1]
            server_port = args[1][1]
            server_host = self.client_address[0]
            game_servers[server_host + ":" + server_port] = (server_name, server_host, server_port)
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
