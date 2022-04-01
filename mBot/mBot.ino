#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <MeAuriga.h>

MeEncoderOnBoard Encoder_1(SLOT1);
MeEncoderOnBoard Encoder_2(SLOT2);
MeUltrasonicSensor ultrasonic_7(7);
MeLineFollower linefollower_6(6);
MeLightSensor lightsensor_12(12);

enum directions {
  forward,
  backward,
  left,
  right,
  stopMoving 
};

enum operationMode {
  autonomous,
  bluetooth,
  remote
};

struct coordinate{
  int x = 0;
  int y = 0;
};

coordinate mowerPosition;

void isr_process_encoder1(void)
{
  if(digitalRead(Encoder_1.getPortB()) == 0){
    Encoder_1.pulsePosMinus();
  }else{
    Encoder_1.pulsePosPlus();
  }
}
void isr_process_encoder2(void)
{
  if(digitalRead(Encoder_2.getPortB()) == 0){
    Encoder_2.pulsePosMinus();
  }else{
    Encoder_2.pulsePosPlus();
  }
}

void positionData(directions direction, int speed){
  //TODO: Calculate position based on direction and speed? Using dead reckoning 

  switch(direction){
    case forward:
    //  mowerPosition.y += ??
    break;
    case backward:
    //  mowerPosition.y -= ??
    break;
    case left:
    //  mowerPosition.x -= ??
    break;
    case right:
    //  mowerPosition.x += ??
    break;
    case stopMoving:
      // do nothing, position is not changing 
    break;
    default:
    // do nothing
    break; 

    //TODO: Send new position to backend 
  }
}

void obstacleDetected(){
      directions backward = backward;
      directions right = right;
      directions stopMoving = stopMoving;
      
      //Stop moving
      move(stopMoving, 0);
      _delay(0.5);
      _delay(2);

      //TODO: Flag to RPi to take picture here and send to backend?

      //Move backward at 25% speed for 1 second
      move(backward, 25 / 100.0 * 255);
      _delay(1);
      move(backward, 0);

      //Turn right at 20% speed for 1 second
      move(right, 20 / 100.0 * 255);
      _delay(1);
      move(right, 0);
}

void lineDetected(){
      directions backward = backward;
      directions left = left;
      directions right = right;
      
      //Move backward at 25% speed for 1 second
      move(backward, 25 / 100.0 * 255);
      _delay(1);
      move(backward, 0);

      //TODO: randomize if mower should turn left or right
      
      //Turn right at 10% speed for 1 second
      move(right, 10 / 100.0 * 255);
      _delay(1);
      move(right, 0);
}

//Function to move robot
void move(directions direction, int speed)
{
  int leftSpeed = 0;
  int rightSpeed = 0;

  positionData(direction, speed);

  //Set the direction of 2 motors' movement
  if(direction == forward){
    leftSpeed = -speed;
    rightSpeed = speed;
  }else if(direction == backward){
    leftSpeed = speed;
    rightSpeed = -speed;
  }else if(direction == left){
    leftSpeed = -speed;
    rightSpeed = -speed;
  }else if(direction == right){
    leftSpeed = speed;
    rightSpeed = speed;
  }
  else if(direction == stopMoving){
    leftSpeed = speed;
    rightSpeed = speed;
  }
  Encoder_1.setTarPWM(leftSpeed);
  Encoder_2.setTarPWM(rightSpeed);
}

void _delay(float seconds) {
  if(seconds < 0.0){
    seconds = 0.0;
  }
  long endTime = millis() + seconds * 1000;
  while(millis() < endTime) _loop();
}

void runAutonomous (){
    directions forward = forward;
    int distanceToObstacle = 25;
    
    //Move forward at 25% speed
    move(forward, 25 / 100.0 * 255);
    if(ultrasonic_7.distanceCm() < distanceToObstacle){
        obstacleDetected();
    }else{
        //If line follower sensor detects left black
        if((0?(2==0?linefollower_6.readSensors()==0:(linefollower_6.readSensors() & 2)==2):(2==0?linefollower_6.readSensors()==3:(linefollower_6.readSensors() & 2)==0))){
            lineDetected();
  
        }
        //If line follower sensor detects right black
        if((0?(1==0?linefollower_6.readSensors()==0:(linefollower_6.readSensors() & 1)==1):(1==0?linefollower_6.readSensors()==3:(linefollower_6.readSensors() & 1)==0))){
            lineDetected();
  
        }
        //If line follower sensor detects BOTH black (necessary?)
        if((0?(3==0?linefollower_6.readSensors()==0:(linefollower_6.readSensors() & 3)==3):(3==0?linefollower_6.readSensors()==3:(linefollower_6.readSensors() & 3)==0))){
            lineDetected();
        }
    }
}

void runBluetooth(){
  //TODO: Run mower by Bluetooth commands sent from app?
}

void runRemote(){
  //TODO: Run mower by command from app, sent from backend?
}

void setup() {
  TCCR1A = _BV(WGM10);
  TCCR1B = _BV(CS11) | _BV(WGM12);
  TCCR2A = _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(CS21);
  attachInterrupt(Encoder_1.getIntNum(), isr_process_encoder1, RISING);
  attachInterrupt(Encoder_2.getIntNum(), isr_process_encoder2, RISING);
  randomSeed((unsigned long)(lightsensor_12.read() * 123456));

  //TODO: Decide mode by input from app?
  operationMode mode = autonomous;
  
  while(1) {
      switch(mode){
        case autonomous:
          runAutonomous();
        break;
        case bluetooth:
          runBluetooth();
        break;
        case remote:
          runRemote();
        break;
        default:
        // do nothing
        break;
      }
    _loop();
   }  
}


void _loop() {
  Encoder_1.loop();
  Encoder_2.loop();
}

void loop() {
  _loop();
}
