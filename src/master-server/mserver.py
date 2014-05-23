#-*- coding: utf-8 -*-
# Copyright (c) 2014 Blowmorph Team

import time
import BaseHTTPServer

# "0.0.0.0" will be available from LAN.
host = "0.0.0.0"
port = 4242
servers_dict = {}

class RequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    def do_POST(self):
        # In POST request game server should send its name, making request at
        # http://address:port/?name=game_server_name.  
        # Correct path = /?name=your_name_here.
        args = self.path.decode().split("/?")
        if len(args) > 2:
            print "Error: too many parameters in POST request"
            return 
        
        # Correct name = ['name', posted_name]. 
        name = args[1].split("=")
        if name[0] != "name":
            print "Error: first parameter in POST isn't name"
            return
        else:
            server_name = name[1]
            
            # client_address = (client_address, port).
            servers_dict[server_name] = self.client_address[0]
            
        self.send_response(200)
        
    def do_GET(self):
        self.wfile.write(str(servers_dict))
        
if __name__ == '__main__':
    server_class = BaseHTTPServer.HTTPServer
    httpd = server_class((host, port), RequestHandler)
    print time.asctime(), "Server Starts - %s:%s" % (host, port)
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    
    httpd.server_close()
    
    print time.asctime(), "\nServer Stops - %s:%s" % (host, port)

