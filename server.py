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

import json

class inverterData:
    inverter_temp : float = 0
    motor_temp : float = 0
    input_voltage : int = 0
    FOC_Id : float = 0
    FOC_Iq : float = 0
    erpm : int = 0
    duty_cycle : float = 0
    ac_current : float = 0
    dc_current : float = 0
    throttle_in : int = 0
    brake_in : int = 0
    Drive_EN : bool = 0
    CAN_MapVers : bytes = 0
    DigitalIO : bytes = 0
    ActiveLimitsByte4 : bytes = 0
    ActiveLimitsByte5 : bytes = 0
    FAULT_CODE : bytes = 0

class bmsData:
    HighOpenCellVoltage : float = 0
    LowOpenCellVoltage : float = 0
    HighOpenCellID : int = 0
    LowOpenCellID : int = 0
    PackDCL : int = 0
    PackAbsCurrent : int = 0
    PackOpenVoltage : int = 0
    PackSOC : int = 0
    PackCurrent : int = 0
    AverageCurrent : int = 0
    HighTemperature : int = 0
    InternalTemperature : int = 0
    DTCFlags_1 : int = 0
    DTCFlags_2 : int = 0
    BalancingEnabled : bool = 0
    DischargeEnableInverted : bool = 0

class cellData:
    InstantVoltage : float = 0
    InternalResistance : float = 0
    OpenVoltage : float = 0

class plexData:
    RadiatorIN : float = 0
    RadiatorOUT : float = 0
    BatteryVoltage : float = 0
    GPS_Fix : int = 0
    CAN1_Load : int = 0
    CAN1_Errors : int = 0
    Throttle_1 : float = 0
    Throttle_2 : float = 0
    Brake : float = 0
    accLong : float = 0
    accLat : float = 0
    accVert : float = 0
    yawRate : float = 0
    Pitch : float = 0
    Roll : float = 0 

dti = inverterData()
bms = bmsData()
plex = plexData() 
TotalCells = 120
cell_data = [cellData() for i in range(TotalCells)]

thermistor_data = []

class camhandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        pr = urlparse(self.path)
        pf = pr.path.split('/')
        if pf[-1] == '' or pf[-1] == 'main.html':
            sfp=Path('canvasJS/main.html')
            with sfp.open('r') as sfile:
                xs=sfile.read()
                self.simpleSend(xs)
        elif pf[-1] == 'bms.html':
            sfp=Path('bms.html')
            with sfp.open('r') as sfile:
                xs=sfile.read()
                self.simpleSend(xs)
        elif pf[-1] == 'bms.js':
            sfp=Path('bms.js')
            with sfp.open('rb') as sfile:
                xs=sfile.read()
            self.send_response(200)
            self.send_header('Content-Type', 'text/javascript')
            self.end_headers()
            self.wfile.write(xs)
        elif pf[-1] == 'main.js':
            sfp=Path('canvasJS/main.js')
            with sfp.open('rb') as sfile:
                xs=sfile.read()
            self.send_response(200)
            self.send_header('Content-Type', 'text/javascript')
            self.end_headers()
            self.wfile.write(xs)
        elif pf[-1] == 'canvas.js':
            sfp=Path('canvasJS/canvasjs.min.js')
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
        elif pf[-1] == 'favicon.ico':
            pass
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
CONNECTIONS = set()



"""
websocket server portion
"""
import websockets
from websockets.asyncio.server import serve,broadcast
import asyncio

async def handler(websocket):
    CONNECTIONS.add(websocket)
    try:
        await websocket.wait_closed()
    finally:
        CONNECTIONS.remove(websocket)

async def main():
    async with serve(handler, "", DEFSOCKETPORT):
        await asyncio.get_running_loop().create_future()  # run forever



def SerialParser(serial_port):
    ser = serial.Serial(
        port = serial_port,
        baudrate = 9600,
        timeout = 0.1) # timeout in seconds
    ser.reset_input_buffer()
    while (True):
        buffer = ser.read_until(expected = '#')
        if(len(buffer) != 0):   
            message_id = struct.unpack_from("I",buffer,1)[0]
            print(message_id, flush = True)
            print(buffer, flush = True)
            print(len(buffer), flush = True)
            if ((message_id >> 8) == 0xF): # Cell Voltage Message
                segment_id = message_id & 0x0F
                data_tuple = struct.unpack_from("HHHHHHHHHHHH",buffer,2)
                for cell in range(12):
                    cell_data[segment_id*12 + cell].OpenVoltage = data_tuple[cell] / 10
            elif ((message_id >> 8) == 0xE): # thermistor message
                thermistor_id_tens = message_id & 0x0F
                data_tuple = struct.unpack_from("bbbbbbbb",buffer,2)
                for thermistor in range(8):
                    thermistor_data[thermistor_id_tens * 10 + thermistor] = data_tuple[thermistor]
                continue
            else:
                match message_id:
                    case 0xCC: # fast critical 
                        data_tuple = struct.unpack_from("IhhhhhBBBBbBBB",buffer,2)
                        dti.erpm = data_tuple[0]
                        dti.duty_cycle = data_tuple[1] / 10
                        dti.ac_current = data_tuple[2] / 10
                        dti.dc_current = data_tuple[3] / 10
                        plex.Throttle_1 = data_tuple[4] / 10
                        plex.Throttle_2 = data_tuple[5] / 10
                        plex.Brake = data_tuple[6] / 10
                        bms.HighOpenCellVoltage = data_tuple[7] / 50
                        bms.LowOpenCellVoltage = data_tuple[8] / 50
                        bms.HighOpenCellID = data_tuple[9]
                        bms.LowOpenCellID = data_tuple[10]
                        dti.throttle_in = data_tuple[11]
                        dti.ActiveLimitsByte4 = data_tuple[12]
                        dti.ActiveLimitsByte5 = data_tuple[13]
                        dti.FAULT_CODE = data_tuple[14]

                        data = {
                            "inv_erpm" : str(dti.erpm),
                            "inv_duty_cycle" : str(dti.duty_cycle),
                            "inv_ac_current" : str(dti.ac_current),
                            "inv_dc_current" : str(dti.dc_current),
                            "plex_throttle_1" : str(plex.Throttle_1),
                            "plex_throttle_2" : str(plex.Throttle_2),
                            "plex_brake" : str(plex.Brake),
                            "bms_high_open_voltage" : str(bms.HighOpenCellVoltage),
                            "bms_low_open_voltage" : str(bms.LowOpenCellVoltage),
                            "bms_high_open_id" : str(bms.HighOpenCellID),
                            "bms_low_open_id" : str(bms.LowOpenCellID),
                            "inv_throttle_in" : str(dti.throttle_in),
                            "inv_limits_4" : str(dti.ActiveLimitsByte4),
                            "inv_limits_5" : str(dti.ActiveLimitsByte5),
                            "inv_fault_code" : str(dti.FAULT_CODE)                           
                        }
                        broadcast(CONNECTIONS,json.dumps(data))
                    case 0xAA: # fast info 
                        data_tuple = struct.unpack_from("hhhhhhh",buffer,2)
                        bms.PackCurrent = data_tuple[0]
                        plex.accLong = data_tuple[1] / 1000
                        plex.accLat = data_tuple[2] / 1000
                        plex.accVert = data_tuple[3] / 1000
                        plex.yawRate = data_tuple[4] / 10
                        plex.Pitch = data_tuple[5] / 10
                        plex.Roll = data_tuple[6] / 10

                        data = {
                            "bms_pack_current" : str(bms.PackCurrent),
                            "plex_acc_long" : str(plex.accLong),
                            "plex_acc_lat" : str(plex.accLat),
                            "plex_acc_vert" : str(plex.accVert),
                            "plex_yaw_rate" : str(plex.yawRate),
                            "plex_pitch" : str(plex.Pitch),
                            "plex_roll" : str(plex.Roll)
                        }
                        broadcast(CONNECTIONS,json.dumps(data))
                    case 0x12: # slow 1
                        data_tuple = struct.unpack_from("hhhhHHHHHhBbb",buffer,2)

                        dti.inverter_temp = data_tuple[0] / 10
                        dti.motor_temp = data_tuple[1] / 10
                        dti.input_voltage = data_tuple[2] 
                        bms.AverageCurrent = data_tuple[3] 
                        bms.PackOpenVoltage = data_tuple[4] 
                        bms.PackDCL = data_tuple[5] 
                        bms.PackAbsCurrent = data_tuple[6] 
                        plex.RadiatorIN = data_tuple[7] / 10 
                        plex.RadiatorOUT = data_tuple[8]  / 10
                        plex.BatteryVoltage = data_tuple[9] / 1000
                        bms.PackSOC = data_tuple[10] 
                        bms.HighTemperature = data_tuple[11] 
                        bms.InternalTemperature = data_tuple[12] 

                        data = {
                            "inv_temp" : str(dti.inverter_temp),
                            "motor_temp" : str(dti.motor_temp),
                            "inv_voltage" : str(dti.input_voltage),
                            "bms_average_current" : str(bms.AverageCurrent),
                            "bms_open_voltage" : str(bms.PackOpenVoltage),
                            "bms_pack_dcl" : str(bms.PackDCL),
                            "bms_pack_abs_current" : str(bms.PackAbsCurrent),
                            "plex_radiator_in" : str(plex.RadiatorIN),
                            "plex_radiator_out" : str(plex.RadiatorOUT),
                            "plex_battery_voltage" : str(plex.BatteryVoltage),
                            "bms_soc": str(bms.PackSOC),
                            "bms_high_temperature" : str(bms.HighTemperature),
                            "bms_internal_temperature" : str(bms.InternalTemperature)
                        }
                        broadcast(CONNECTIONS,json.dumps(data))
                    case 0x34: # slow 2
                        data_tuple = struct.unpack_from("IIHHHBBBBB??",2)
                        dti.FOC_Id = data_tuple[0] / 100
                        dti.FOC_Iq = data_tuple[1] / 100
                        plex.GPS_Fix = data_tuple[2]
                        plex.CAN1_Load = data_tuple[3]
                        plex.CAN1_Errors = data_tuple[4]
                        dti.DigitalIO = data_tuple[5]
                        dti.Drive_EN = data_tuple[6]
                        dti.CAN_MapVers = data_tuple[7]
                        bms.DTCFlags_1 = data_tuple[8]
                        bms.DTCFlags_2 = data_tuple[9]
                        bms.BalancingEnabled = data_tuple[10]
                        bms.DischargeEnableInverted = data_tuple[11]

                        data = {
                            "inv_foc_id" : str(dti.FOC_Id),
                            "inv_foc_iq" : str(dti.FOC_Iq),
                            "plex_gps_fix" : str(plex.GPS_Fix),
                            "plex_can1_load" : str(plex.CAN1_Load),
                            "plex_can1_errors" : str(plex.CAN1_Errors),
                            "inv_digital_io" : str(dti.DigitalIO),
                            "inv_can_map_vers" : str(dti.CAN_MapVers),
                            "bms_dtc_flags_1" : str(bms.DTCFlags_1),
                            "bms_dtc_flags_2" : str(bms.DTCFlags_2),
                            "bms_balancing_enabled" : str(bms.BalancingEnabled),
                            "bms_discharge_enable_inverted" : str(bms.DischargeEnableInverted)
                        }
                        broadcast(CONNECTIONS,json.dumps(data))
                    case _:
                        print("bruh what")
        else:
            dti.inverter_temp = random.randint(35,39)
            dti.motor_temp = random.randint(45,50)
            bms.InternalTemperature = random.randint(26,30)

            bms.PackOpenVoltage = 476 + random.randint(-2,2)
            dti.input_voltage = 476 + random.randint(-2,2)
            data = {
                "inv_voltage" : str(dti.input_voltage),
                "bms_open_voltage" : str(bms.PackOpenVoltage),
                "inv_temp" : str(dti.inverter_temp),
                "motor_temp" : str(dti.motor_temp),
                "bms_internal_temperature" : str(bms.InternalTemperature)
            }
            broadcast(CONNECTIONS,json.dumps(data))
            time.sleep(0.1)

if __name__ == "__main__":
    httpServerThread = Thread(target=HTTPserverThread,args=[DEFWEBPORT])
    httpServerThread.start()


    serialParserThread= Thread(target=SerialParser,args=["/dev/ttyAMA0"])
    serialParserThread.start()
    asyncio.run(main())
