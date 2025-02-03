import struct
import serial 
import base64


if __name__ == "__main__":
  ser = serial.Serial(
      port = "/dev/ttyAMA0",
      baudrate = 9600,
      timeout = 10) # timeout in seconds
  f = open("uart.txt","w")

  while (1):
    buf = ser.read()
    f.write("".join([f"{byte:02x} " if (byte != 0x23) else f"{byte:02x}\n"for byte in buf]))
    f.flush()
  data_buffer = bytearray()
  while (1):
    while (ser.read()[0] != 0x24):
      continue
    while (True):
      char = ser.read()[0]
      if (char == 0x23):
        break
      else:
        data_buffer.append(char)
    print("".join([f"{byte:02x} " for byte in data_buffer]), flush = True)
    
    # buf = ser.read()

    # f.write("".join([f"{byte:02x} " if (byte != 0x23) else f"{byte:02x}\n"for byte in buf]))
    # f.flush()
    # print("str buffer\n","".join([f"{byte:02x} " for byte in str_buffer]), flush = True)
    #print("id\n",hex(struct.unpack_from("B",str_buffer,0)[0]))
    # print("str buffer slice:\n","".join([f"{byte:02x} " for byte in str_buffer[1:-1]]), flush = True)
    # print("sliced length\n",len(str_buffer[1:-1]))
    # print()
    # buffer = base64.b64decode(str_buffer[1:-1])
    # print(buffer)