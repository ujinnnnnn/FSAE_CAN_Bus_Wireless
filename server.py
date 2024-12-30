import http.server
import argparse
from urllib.parse import urlparse, parse_qs
from pathlib import Path
from socketserver import ThreadingMixIn
import json
import time
import errno
import random
import serial 
import struct
import threading
from threading import Thread

import websockets
from websockets.asyncio.server import serve
import asyncio
import json

class inverterData:
    erpm : int = 0
    duty_cycle : float = 0
    input_voltage : int = 0
    ac_current : float = 0
    dc_current : float = 0
    inverter_temp : float = 0
    motor_temp : float = 0
    FAULT_CODE : bytes = 0x12 
    FOC_Id : float = 0
    FOC_Iq : float = 0
    throttle_in : int = 0
    brake_in : int = 0
    DigitalIO : bytes = 0
    Drive_EN : bool = 0
    ActiveLimitsByte4 : bytes = 0
    ActiveLimitsByte5 : bytes = 0
    CAN_MapVers : bytes = 0

class bmsData:
    PackAbsCurrent : int = 0
    AverageCurrent : int = 0
    MaxCellVoltage : float = 0
    PackCCL : float = 0
    PackDCL : int = 0
    MaxPackVoltage : int = 0
    PackCurrent : int = 0
    PackSOC : int = 0
    bitfield1 : bytes = 0
    bitfield2 : bytes = 0
    bitfield3 : bytes = 0
    bitfield4 : bytes = 0
    HighTemperature : int = 0
    InternalTemperature : int = 0
    DischargeEnableInverted : bool = 0
    ChargerSafetyRelayFault : bool = 0

class cellData:
    InstantVoltage : float = 0
    InternalResistance : float = 0
    OpenVoltage : float = 0

dti = inverterData()
bmt = bmsData()
TotalCells = 120
cell_data = [cellData() for i in range(TotalCells)]

thermistor_data = []

class camhandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        pr = urlparse(self.path)
        pf = pr.path.split('/')
        if pf[-1] == '' or pf[-1] == 'client.html':
            sfp=Path('client.html')
            with sfp.open('r') as sfile:
                xs=sfile.read()
                self.simpleSend(xs)
        elif pf[-1] == 'chart.js':
            sfp=Path('chart.js')
            with sfp.open('rb') as sfile:
                xs=sfile.read()
            self.send_response(200)
            self.send_header('Content-Type', 'text/javascript')
            self.end_headers()
            self.wfile.write(xs)
        elif pf[-1] == 'update.js':
            sfp=Path('update.js')
            with sfp.open('rb') as sfile:
                xs=sfile.read()
            self.send_response(200)
            self.send_header('Content-Type', 'text/javascript')
            self.end_headers()
            self.wfile.write(xs)
        elif pf[-1] == 'update_chart.js':
            sfp=Path('update_chart.js')
            with sfp.open('rb') as sfile:
                xs=sfile.read()
            self.send_response(200)
            self.send_header('Content-Type', 'text/javascript')
            self.end_headers()
            self.wfile.write(xs)
        elif pf[-1] == 'style.css':
            sfp=Path('style.css')
            with sfp.open('rb') as sfile:
                xs=sfile.read()
            self.send_response(200)
            self.send_header('Content-Type', 'text/css')
            self.end_headers()
            self.wfile.write(xs)
        else :
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

def HTTPserverThread(webport):
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
    

DEFWEBPORT = 8088
DEFSOCKETPORT = 8000
wsServer = None

dataDirty = True

def SerialParser(serial_port):
    global dataDirty
    ser = serial.Serial(
        port = serial_port,
        baudrate = 9600,
        timeout = 0.1) # timeout in seconds
    ser.reset_input_buffer()
    while (True):
        buffer = ser.read_until(expected = '#')
        if(len(buffer) != 0):   
            message_id = struct.unpack_from("I",buffer,1)[0]
            print(message_id)
            print(buffer, flush = True)
            print(len(buffer))
            match message_id:
                case 1234:
                    test = struct.unpack_from("fffhHHHBBbb",buffer,5)
                case _:
                    print("bruh what")
            dataDirty = True
        #else:
            #print("nothing",flush=True)

async def handler(websocket):
    global dataDirty
    while True:
        data = {
            "inv-temp" : str(dti.inverter_temp),
            "motor-temp" : str(dti.motor_temp),
            "max-cell-voltage" : str(bmsData.MaxCellVoltage),
            "input-voltage" : str(dti.input_voltage),
            "pack-abs-current" : str(bmsData.PackAbsCurrent),
            "average-current" : str(bmsData.AverageCurrent),
            "max-pack-voltage" : str(bmsData.MaxPackVoltage),
            "inv-fault-code" : str(dti.FAULT_CODE),
            "pack-soc" : str(bmsData.PackSOC),
            "bms-high-temp" : str(bmsData.HighTemperature),
            "bms-internal-temp" : str(bmsData.InternalTemperature)
        }
        await websocket.send(json.dumps(data))
        await asyncio.sleep(5)
        #async for message in websocket:
            #await websocket.send(message)
            #print(message)

async def main():
    async with serve(handler, "", DEFSOCKETPORT):
        await asyncio.get_running_loop().create_future()  # run forever


if __name__ == "__main__":
    httpServerThread = Thread(target=HTTPserverThread,args=[DEFWEBPORT])
    httpServerThread.start()


    serialParserThread= Thread(target=SerialParser,args=["/dev/ttyAMA0"])
    serialParserThread.start()
    asyncio.run(main())
