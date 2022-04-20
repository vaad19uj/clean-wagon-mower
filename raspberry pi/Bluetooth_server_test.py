import socket

hostMACAddress = '00:1f:e1:dd:08:3d' # The MAC address of a Bluetooth adapter on the server. 
port = 3 # 3 is an arbitrary choice. However, it must match the port used by the client.
backlog = 1
size = 1024
s = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
s.bind((hostMACAddress,port))
s.listen(backlog)
try:
    client, address = s.accept()
    while 1:
        data = client.recv(size)
        if data:
            print(data)
            client.send(data)
except:	
    print("Closing socket")	
    client.close()
    s.close()

##########################################################

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

