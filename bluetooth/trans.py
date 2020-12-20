import sys
import time
import bluetooth

from atexit import register

pre_timestamp = 0
cur_timestamp = pre_timestamp

@register
def Exit():
  bluetooth_socket.send(DISCONNECT)
  bluetooth_socket.close()

def AddToFile(data_frame):
  global pre_timestamp
  global cur_timestamp
  cur_timestamp = int(round(time.time() * 1000))
  sample_rate = 1000 / (cur_timestamp - pre_timestamp) * FRAME_SIZE/6
  print("sample_rate:" + str(int(sample_rate)) + "Hz\n")
  pre_timestamp = cur_timestamp

CONNECT = bytes.fromhex("01")
DISCONNECT = bytes.fromhex("00")
ST_DATAFRAME = bytes.fromhex("11111111")
EN_DATAFRAME = bytes.fromhex("00000000")
FRAME_SIZE = 6000

avail_devices = bluetooth.discover_devices(lookup_names=True)

print(avail_devices)

if ('11:11:11:11:11:11', 'ACCEL-SSP') in avail_devices:
  print("found ACCEL-SSP\n")
else: 
  print("didn't find ACCEL-SSP\n")
  exit(0)

bluetooth_socket = bluetooth.BluetoothSocket(bluetooth.RFCOMM);

bluetooth_socket.connect(("11:11:11:11:11:11", 1))

bluetooth_socket.send(CONNECT)

rev_st_dataframe = False
rev_en_dataframe = False

data_frame = bytes.fromhex("")
pre_timestamp = int(round(time.time() * 1000))
while True:
  recv_temp = bluetooth_socket.recv(4)
  if recv_temp == ST_DATAFRAME:
    rev_st_dataframe = True
    rev_en_dataframe = False
    data_frame = b''
  elif recv_temp == EN_DATAFRAME:
    rev_en_dataframe = True
    if rev_st_dataframe == True and rev_en_dataframe == True:
      if len(data_frame) == FRAME_SIZE:
        AddToFile(data_frame)
      else:
        print("loss packages\n")
      rev_st_dataframe = False
      rev_en_dataframe = False
  else:
    if rev_st_dataframe == True and rev_en_dataframe == False:
      if len(data_frame) >= FRAME_SIZE:  #可能溢出或者结束帧丢失
        rev_st_dataframe = False
        print("overflow\n")
      else:
        data_frame += recv_temp