import struct
import serial 
import base64


if __name__ == "__main__":
  ser = serial.Serial(
      port = "/dev/ttyAMA0",
      baudrate = 115200,
      timeout = 10) # timeout in seconds


  while (1):
    data_buffer = bytearray()
    while (True):
      buf = ser.read()
      try:
        if (ser.read()[0] == 0x24):
          break
      except Exception as e:
        print("serial timed out")
    while (True):
      try:
        char = ser.read()[0]
        if (char == 0x23):
          break
        else:
          data_buffer.append(char)
      except Exception as e:
        print("serial timed out")
    try:
      buffer = base64.b64decode(data_buffer[1:])
      print(buffer)
    except Exception as e:
      print(f"sliced length : {len(data_buffer[1:])}")
    print("".join([f"{byte:02x} " for byte in data_buffer]), flush = True)
    print()
    
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