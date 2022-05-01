# clean-wagon-mower - Code for project in intelligent mobile systems

Connect the Raspberry pi Zero W and Arduino Mega 2560 via a USB cable. 

Arduino:

  1. Include the following repository as a .zip-library in Arduino IDE: https://github.com/Makeblock-official/Makeblock-Libraries
  2. Change board to "Arduino Mega or Mega 2560" in the Arduino IDE. 
  3. Upload and run the "mBot.ino" file

Raspberry pi:

  1. Place files "Backend_communication.py" and "Bluetooth_server.py" on the Raspberry pi Desktop.
  2. Open two terminal windows.
  3. "cd /home/pi/Desktop" in both terminals.
  3a. "sudo python Backend_communication.py" in one of the terminals. 
  3b. "sudo python Bluetooth_server.py" in the other terminal.
