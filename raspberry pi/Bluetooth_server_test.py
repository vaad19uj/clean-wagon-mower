import sys
import os
import time
from bluetooth import *

# Make device visible
os.system("hciconfig hci0 piscan")

# Create a new server socket using RFCOMM protocol
server_sock = BluetoothSocket(RFCOMM)
# Bind to any port
server_sock.bind(("", PORT_ANY))
# Start listening
server_sock.listen(1)

# Get the port the server socket is listening
port = server_sock.getsockname()[1]

# The service UUID to advertise
uuid = "7be1fcb3-5776-42fb-91fd-2ee7b5bbb86d"

# Start advertising the service
advertise_service(server_sock, "RpiBtSrv",
                    service_id=uuid,
                    service_classes=[uuid, SERIAL_PORT_CLASS],
                    profiles=[SERIAL_PORT_PROFILE])

print("Waiting for connection on RFCOMM channel %d" % port)

client_sock = None

# This will block until we get a new connection
client_sock, client_info = server_sock.accept()
print("Accepted connection from ", client_info)

while True:
    try:
        # Read the data sent by the client
        data = client_sock.recv(1024)
        if len(data) == 0:
            break

        print("Received [%s]" % data)

        # Handle the request
        #if data == "getop":
        #    response = "op:%s" % ",".join(operations)
        #elif data == "ping":
        #    response = "msg:Pong"
        #elif data == "example":
        #    response = "msg:This is an example"
        ## Insert more here
        #else:
        #    response = "msg:Not supported"

        #client_sock.send(response)
        #print("Sent back [%s]" % response)

    except IOError:
        pass

    except KeyboardInterrupt:

        if client_sock is not None:
            client_sock.close()

        server_sock.close()

        print("Server going down")
        break



########################################################


#import socket

#hostMACAddress = 'B8:27:EB:3A:D7:A4' # The MAC address of a Bluetooth adapter on the server. 
#port = 3 # 3 is an arbitrary choice. However, it must match the port used by the client.
#backlog = 1
#size = 1024
#s = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
#s.bind((hostMACAddress,port))
#print("socket bind")
#s.listen(backlog)
#print("socket listening")
#try:
#    print("waiting for connection")
#    client, address = s.accept()
#    print("Accepted connection from ", address)
#    while 1:
#        data = client.recv(size)
#        if data:
#            print(data)
#            client.send(data)
#except:	
#    print("Closing socket")	
#    client.close()
#    s.close()


######################################################################################################33


# import socket
# import serial

# serArduino = serial.Serial('/dev/ttyUSB0', 9600)
# serArduino.flush()
# serArduino.reset_input_buffer()

# server = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM)

# port = 3
# host = ""

# server.bind(host, port)
# server.listen(1)
# client, addr = server.accept()

# while True:
#     try:
#         cmd = server.recv(1024)
#         cmdAsInt = int(cmd)
#         if cmdAsInt == 1:   #forward
#             serArduino.write(1)
#             client.send("Ack: forward")
#         elif cmdAsInt == 2: #backward
#             serArduino.write(2)
#             client.send("Ack: backward")
#         elif cmdAsInt == 3: #left
#             serArduino.write(3)
#             client.send("Ack: left")
#         elif cmdAsInt == 4: #right
#             serArduino.write(4)
#             client.send("Ack: right")
#         elif cmdAsInt == 5: #stopmoving
#             serArduino.write(5)
#             client.send("Ack: stop moving")
        
#         if(cmd == 'q'): #quit
#             break;
#     except:
#         print("Error")

# client.close()
# server.close()

