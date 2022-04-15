#!/usr/bin/env python3
import serial
import threading
import os

def createSerialDevice():
    os.system("sudo rfcomm watch hci0 ")

if __name__ == '__main__':
    serArduino = serial.Serial('/dev/ttyUSB0', 9600)
    serArduino.flush()
    serArduino.reset_input_buffer()

    thread = threading.Thread(target=createSerialDevice, daemon=1)
    thread.start()

    serBt = serial.Serial('/dev/rfcomm0', 9600)
    serBt.flush()
    serBt.reset_input_buffer()

    while True:
        cmd = serBt.readline()
        cmdAsInt = int(cmd)
        if cmdAsInt == 1:   #forward
            serArduino.write(1)
        elif cmdAsInt == 2: #backward
            serArduino.write(2)
        elif cmdAsInt == 3: #left
            serArduino.write(3)
        elif cmdAsInt == 4: #right
            serArduino.write(4)
        elif cmdAsInt == 5: #stopmoving
            serArduino.write(5)


            