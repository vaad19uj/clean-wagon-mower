#!/usr/bin/env python3
import serial
import time
import requests
from picamera import PiCamera
from time import sleep
import json

ser = serial.Serial('/dev/ttyUSB0', 9600)
ser.flush()
ser.reset_input_buffer()
camera = PiCamera()
journeyId = -2
mowerId = 2
coordinateId = -4
baseURL = 'http://ec2-16-170-167-138.eu-north-1.compute.amazonaws.com/api/v1'

def takePicture(camera):
    camera.resolution = (1080, 768)
    camera.start_preview()
    sleep(1)
    camera.capture('/home/pi/Desktop/image.png')
    camera.stop_preview()

    fileName = 'image.png'
    filePath = '/home/pi/Desktop/image.png'
    obstacleImg =[('image',(fileName,open(filePath,'rb'),'image/png'))]   

    return obstacleImg

def obstacleDetected(camera, x, y):
    sendCoordinate(x, y)
    url = baseURL + '/events'
    payload = {'coordinate_id': coordinateId, 'event_type': 'Obstacle', 'mower_id': mowerId}
    files = takePicture(camera)

    response = requests.request("POST", url, data=payload, files=files)
    print("obstacle detected response: ", response)

def sendCoordinate(x, y):
    global coordinateId
    print("send coord journey id: ", journeyId)
    url = baseURL + '/coordinates'
    payload = json.dumps({
      "x": x,
      "y": y,
      "journey_id": journeyId
    })
    headers = {
      'Content-Type': 'application/json'
    }

    response = requests.request("POST", url, headers=headers, data=payload)
    print("send coordinate response: ", response)
    data = response.json()
    coordinateId = data["coordinate_id"]
    print("coord id: ", coordinateId)

def startJourney():
    global journeyId
    url = baseURL + '/journeys/start-journey'

    payload = json.dumps({
      "mower_id": mowerId
    })

    headers = {
      'Content-Type': 'application/json'
    }

    response = requests.request("POST", url, headers=headers, data=payload)
    print("startJourney response: ", response)
    data = response.json()
    journeyId = data["journey_id"]
    print("journey id: ", journeyId)

def stopJourney(x, y):
    url = baseURL + '/journeys/stop-journey'

    payload = json.dumps({
      "journey_id": journeyId,
      "x": x,
      "y": y
    })
    headers = {
      'Content-Type': 'application/json'
    }

    response = requests.request("PUT", url, headers=headers, data=payload)
    print("stopJourney response")

while True:
    cmd = ser.readline()
    print(cmd)
    try:
        cmdAsInt = int(cmd)
        if cmdAsInt == 0:
            x = ser.readline()
            print(x)
            y = ser.readline()
            print(y)
            obstacleDetected(camera, x, y)
        elif cmdAsInt == 1:
            x = ser.readline()
            print(x)
            y = ser.readline()
            print(y)
            sendCoordinate(x, y)
        elif cmdAsInt == 2:
            startJourney()
        elif cmdAsInt == 3:
            x = ser.readline()
            print(x)
            y = ser.readline()
            print(y)
            stopJourney(x, y)
    except KeyboardInterrupt:
	    print("Backend communication going down")
        break
    except:
        pass