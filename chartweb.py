#!/usr/bin/python3

import http.server
import argparse
from urllib.parse import urlparse, parse_qs
from pathlib import Path
from socketserver import ThreadingMixIn
import json
import time
import errno
import random

class camhandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        pr = urlparse(self.path)
        pf = pr.path.split('/')
        if pf[-1] == '' or pf[-1] == 'index.html':
            print("send index")
            sfp=Path('index.html')
            with sfp.open('r') as sfile:
                xs=sfile.read()
                self.simpleSend(xs)
        elif pf[-1] == 'highcharts.js':
            print("send highcharts.js")
            sfp=Path('highcharts.js')
            with sfp.open('rb') as sfile:
                xs=sfile.read()
            self.send_response(200)
            self.send_header('Content-Type', 'text/javascript')
            self.end_headers()
            self.wfile.write(xs)
        elif pf[-1] == 'temperature':
            print("send temperature data")
            xs = str.encode("123")
            self.send_response(200)
            self.send_header('Content-Type', 'text/plain')
            self.end_headers()
            self.wfile.write(xs)
        elif pf[-1] == 'humidity':
            print("send humidity data")
            xs = str.encode("456")
            self.send_response(200)
            self.send_header('Content-Type', 'text/plain')
            self.end_headers()
            self.wfile.write(xs)
        elif pf[-1] == 'pressure':
            print("send pressure data")
            xs = str.encode("789")
            self.send_response(200)
            self.send_header('Content-Type', 'text/plain')
            self.end_headers()
            self.wfile.write(xs)
        else:
            print("what is", pf[-1])
            print(pf)

    def simpleSend(self, thcontent):
        self.send_response(200)
        self.send_header('Content-Type', 'text/html; charset=utf-8')
        self.end_headers()
        self.wfile.write(thcontent.encode('utf-8'))


class ThreadedHTTPServer(ThreadingMixIn, http.server.HTTPServer):
    """Handle requests in a separate thread."""

def findMyIp():
    """
    A noddy function to find local machines' IP address for simple cases....
    based on info from https://stackoverflow.com/questions/166506/finding-local-ip-addresses-using-pythons-stdlib
    
    returns an array of IP addresses (in simple cases there will only be one entry)
    """
    import socket
    return([ip for ip in socket.gethostbyname_ex(socket.gethostname())[2] if not ip.startswith("127.")] or 
            [[(s.connect(("8.8.8.8", 53)), s.getsockname()[0], s.close()) for s in [socket.socket(socket.AF_INET, socket.SOCK_DGRAM)]][0][1]]
          )

DEFWEBPORT      = 8088

if __name__ == '__main__':
    import sys, os, pathlib
    clparse = argparse.ArgumentParser(description='runs a simple webserver to chart the cpu temperature. ')
    clparse.add_argument( "-w", "--webport", type=int, default=DEFWEBPORT,
        help="port used for the webserver, default %d" % DEFWEBPORT)
    args=clparse.parse_args()
    webport = args.webport
    server = ThreadedHTTPServer(('',webport),camhandler)
    ips=findMyIp()
    if len(ips)==0:
        print('starting webserver on internal IP only (no external IP addresses found)')
    elif len(ips)==1:
        print('Starting webserver on %s:%d' % (ips[0],webport))
    else:
        print('Starting webserver on multiple ip addresses (%s), port:%d' % (str(ips),webport))
    try:
        server.serve_forever()
        print('webserver shut down')
    except KeyboardInterrupt:
        server.socket.close()
