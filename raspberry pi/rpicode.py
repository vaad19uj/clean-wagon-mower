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
    
    # Take image from desktop and send to backend, using API
    url = 'http://ec2-16-170-167-138.eu-north-1.compute.amazonaws.com/api/v1/images'

    fileName = 'image.png'
    filePath = '/home/pi/Desktop/image.png'
    obstacleImg =[('image',(fileName,open(filePath,'rb'),'image/png'))]   

    response = requests.request("POST", url, headers=headers, data=payload, files=obstacleImg)
    print(response.status_code)

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


        
    