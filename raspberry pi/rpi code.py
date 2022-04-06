#!/usr/bin/env python3
import serial
import time
import requests
from picamera import PiCamera
from time import sleep

def takePicture(camera):
    camera.start_preview()
    sleep(1)
    camera.capture('/home/pi/Desktop/image.png')
    camera.stop_preview()
    
    # Take image from desktop and send to backend, using HTTP

    #url = 'https://www.w3schools.com/python/demopage.php'
    #obstacleImg = {'somekey': 'somevalue'}

    #response = requests.post(url, data = obstacleImg)
    #print(response.text)

if __name__ == '__main__':
    ser = serial.Serial('/dev/ttyUSB0', 9600)
    ser.flush()
    ser.reset_input_buffer()
    camera = PiCamera()
    while True:
        cmd = ser.readline()
        cmdAsInt = int(cmd)
        if cmdAsInt == 0:
            takePicture(camera)


        
    