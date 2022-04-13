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
  directionForward,
  directionBackward,
  directionLeft,
  directionRight,
  directionStopMoving
};

enum operationMode {
  autonomous,
  bluetooth,
  remote
};

enum commands {
  takePicture
};

struct coordinate {
  int x = 0;
  int y = 0;
};

coordinate mowerPosition;
directions backward = directionBackward;
directions left = directionLeft;
directions right = directionRight;
directions forward = directionForward;
directions stopMoving = directionStopMoving;

void isr_process_encoder1(void)
{
  if (digitalRead(Encoder_1.getPortB()) == 0) {
    Encoder_1.pulsePosMinus();
  } else {
    Encoder_1.pulsePosPlus();
  }
}
void isr_process_encoder2(void)
{
  if (digitalRead(Encoder_2.getPortB()) == 0) {
    Encoder_2.pulsePosMinus();
  } else {
    Encoder_2.pulsePosPlus();
  }
}

void positionData(directions direction, int speed) {
  //TODO: Calculate position based on direction and speed? Using dead reckoning

  switch (direction) {
    case directionForward:
      //  mowerPosition.y += ??
      break;
    case directionBackward:
      //  mowerPosition.y -= ??
      break;
    case directionLeft:
      //  mowerPosition.x -= ??
      break;
    case directionRight:
      //  mowerPosition.x += ??
      break;
    case directionStopMoving:
      // do nothing, position is not changing
      break;
    default:
      // do nothing
      break;
  }

  //TODO: Send new position to backend
}

//Function to move robot
void move(directions direction, int speed)
{
  int leftSpeed = 0;
  int rightSpeed = 0;

  //positionData(direction, speed);
  // do we need to add duration here? So we know how long we are travelling in one direction?

  //Set the direction of 2 motors' movement
  switch (direction) {
    case directionForward:
      leftSpeed = -speed;
      rightSpeed = speed;
      break;
    case directionBackward:
      leftSpeed = speed;
      rightSpeed = -speed;
      break;
    case directionLeft:
      leftSpeed = -speed;
      rightSpeed = -speed;
      break;
    case directionRight:
      leftSpeed = speed;
      rightSpeed = speed;
      break;
    case directionStopMoving:
      // 0
      leftSpeed = speed;
      rightSpeed = speed;
      break;
    default:
      // do nothing
      break;
  }

  Encoder_1.setTarPWM(leftSpeed);
  Encoder_2.setTarPWM(rightSpeed);
}

void _delay(float seconds) {
  if (seconds < 0.0) {
    seconds = 0.0;
  }
  long endTime = millis() + seconds * 1000;
  while (millis() < endTime) _loop();
}

void turn() {
  static int turnCounter = 0;

  // turn to the right 5 times in a row, then switch to turning left 5 times in a row
  if (turnCounter < 5) {
    //Turn right at 15% speed for 1 second
    move(right, 15 / 100.0 * 255);
    _delay(1);
    move(right, 0);
  }
  else {
    //Turn left at 15% speed for 1 second
    move(left, 15 / 100.0 * 255);
    _delay(1);
    move(left, 0);
  }

  turnCounter += 1;

  if (turnCounter == 10) {
    turnCounter = 0;
  }
}

void lineDetected() {
  //Move backward at 25% speed for 1 second
  move(backward, 25 / 100.0 * 255);
  _delay(1);
  move(backward, 0);

  turn();
}

void obstacleDetected() {

  commands cmd = takePicture;

  //Stop moving
  move(stopMoving, 0);

  //Flag that Rpi should take picture
  Serial.println(cmd);

  _delay(0.5);
  _delay(2);

  //Move backward at 25% speed for 1 second
  move(backward, 25 / 100.0 * 255);
  _delay(1);
  move(backward, 0);

  turn();
}

void runAutonomous() {
  int distanceToObstacle = 25;

  //Move forward at 25% speed
  move(forward, 25 / 100.0 * 255);
  if (ultrasonic_7.distanceCm() < distanceToObstacle) {
    obstacleDetected();
  } else {
    //If line follower sensor detects black
    if(linefollower_6.readSensors() != 3){
      lineDetected();
    }
  }
}

void runBluetooth() {
  if (Serial.available() > 0)
  {
    int cmd = Serial.read();
    Serial.write(cmd);

    switch (cmd) {
      case '1':
        move(forward, 25 / 100.0 * 255);
        break;
      case '2':
        move(backward, 25 / 100.0 * 255);
        break;
      case '3':
        move(left, 25 / 100.0 * 255);
        break;
      case '4':
        move(right, 25 / 100.0 * 255);
        break;
      case '5':
        move(stopMoving, 0);
        break;
    }
  }
}

void runRemote() {
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

  Serial.begin(9600);

  //TODO: Decide mode by input from app?
  operationMode mode = autonomous;
  //operationMode mode = bluetooth;

  while (1) {

    switch (mode) {
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
