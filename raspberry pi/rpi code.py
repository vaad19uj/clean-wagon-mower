#!/usr/bin/env python3
import serial
import time
import requests
from picamera import PiCamera
from time import sleep

if __name__ == '__main__':
    ser = serial.Serial('/dev/ttyxxx', 9600, timeout=1)
    ser.reset_input_buffer()
    while True:
        cmd = ser.read()
        if cmd == 0:
            takePicture()

def takePicture():
    camera = PiCamera()
    camera.start_preview()
    sleep(1)
    camera.capture('/home/pi/Desktop/image.jpg')
    camera.stop_preview()
    #camera.close()
    
    # Take image from desktop and send to backend, using HTTP

    #url = 'https://www.w3schools.com/python/demopage.php'
    #obstacleImg = {'somekey': 'somevalue'}

    #response = requests.post(url, data = obstacleImg)
    #print(response.text)
        
    