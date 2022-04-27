import sys
import os
import time
from bluetooth import *
import serial

# Make device visible
os.system("hciconfig hci0 piscan")

serArduino = serial.Serial('/dev/ttyUSB0', 9600)
serArduino.flush()
serArduino.reset_input_buffer() 

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
        cmd = client_sock.recv(1024)

        if cmd == "MANUALMODE":
            serArduino.write('b')
        elif cmd == "MANUALDISCONNECT":
            serArduino.write('d')
            break
        elif cmd == "AUTOMODE":
            serArduino.write('a')
        elif cmd == "STOPAUTOMODE":
            serArduino.write('s')
            break
        elif cmd == "FORWARD":
            serArduino.write(1)
        elif cmd == "BACKWARD":
            serArduino.write(2)
        elif cmd == "LEFT":
            serArduino.write(3)
        elif cmd == "RIGHT":
            serArduino.write(4)
        elif cmd == "STOPMOVING":
            serArduino.write(5)

    except IOError:
        pass

    except KeyboardInterrupt:

        if client_sock is not None:
            client_sock.close()

        server_sock.close()

        print("Server going down")
        break


if client_sock is not None:
    client_sock.close()

server_sock.close()
