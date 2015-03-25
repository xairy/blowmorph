#-*- coding: utf-8 -*-
# Copyright (c) 2015 Blowmorph Team

from __future__ import unicode_literals
import json, time, BaseHTTPServer

game_servers = {}

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
    config_file = open('data/master-server.json')
    config = json.loads(config_file.read())
    config_file.close()
    port = config["master-server"]["port"]

    http_server = BaseHTTPServer.HTTPServer(("0.0.0.0", port), RequestHandler)
    print "[%s] Server started on port %d." % (str(time.asctime()), port)
    try:
        http_server.serve_forever()
    except KeyboardInterrupt:
        pass
    http_server.server_close()
    print "[%s] Server stopped." % (str(time.asctime()),)
