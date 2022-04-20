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
  notDecided,
  autonomous,
  bluetooth,
  stopRunning
};

enum commands {
  obstacleDetected,
  sendCoordinate,
  startJourney,
  stopJourney
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
operationMode mode = notDecided;
unsigned long runningForwardTime;
unsigned long backingTime;
unsigned long turningTime;
unsigned long stopTime;

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

void sendCoordinateToRpi(int x, int y) {
  Serial.println(sendCoordinate);
  Serial.println(x);
  Serial.println(y);
}

void positionData(directions direction, int speed) {

  //distance = speed × time
  int distance = 0;
  unsigned long timePassed = 0;

  switch (direction) {
    case directionForward:
      timePassed = stopTime - runningForwardTime;
      distance = timePassed * speed;
      mowerPosition.y += distance;
      break;
    case directionBackward:
      timePassed = turningTime - backingTime;
      distance = timePassed * speed;
      mowerPosition.y -= distance;
      break;
    case directionLeft:
      timePassed = runningForwardTime - turningTime;
      distance = timePassed * speed;
      mowerPosition.x -= distance;
      break;
    case directionRight:
      timePassed = runningForwardTime - turningTime;
      distance = timePassed * speed;
      mowerPosition.x += distance;
      break;
    case directionStopMoving:
      // do nothing, position is not changing
      break;
    default:
      // do nothing
      break;
  }
  
  sendCoordinateToRpi(mowerPosition.x, mowerPosition.y);
}

//Function to move robot
void move(directions direction, int speed)
{
  int leftSpeed = 0;
  int rightSpeed = 0;

  //We only send position data to backend while being in autonomous mode
  if (mode == autonomous) {
    positionData(direction, speed);
  }

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
    turningTime = millis();
    move(right, 15 / 100.0 * 255);
    _delay(1);
    move(right, 0);
  }
  else {
    //Turn left at 15% speed for 1 second
    turningTime = millis();
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
  //Stop moving
  stopTime = millis();
  move(stopMoving, 0);
  _delay(0.5);

  //Move backward at 25% speed for 1 second
  backingTime = millis();
  move(backward, 25 / 100.0 * 255);
  _delay(1);
  move(backward, 0);

  turn();
}

void obstacleDetectedEvent() {

  commands cmd = obstacleDetected;

  //Stop moving
  stopTime = millis();
  move(stopMoving, 0);

  //Flag to Rpi that obstacle avoidance event occured
  Serial.println(cmd);
  Serial.println(mowerPosition.x);
  Serial.println(mowerPosition.y);

  _delay(0.5);
  _delay(2);

  //Move backward at 25% speed for 1 second
  backingTime = millis();
  move(backward, 25 / 100.0 * 255);
  _delay(1);
  move(backward, 0);

  turn();
}

void runAutonomous() {
  int distanceToObstacle = 25;

  runningForwardTime = millis();
  //Move forward at 25% speed
  move(forward, 25 / 100.0 * 255);
  if (ultrasonic_7.distanceCm() < distanceToObstacle) {
    obstacleDetectedEvent();
  } else {
    //If line follower sensor detects black
    if (linefollower_6.readSensors() != 3) {
      lineDetected();
    }
  }
}

void runBluetooth() {
  if (Serial.available() > 0)
  {
    int cmd = Serial.read();

    switch (cmd) {
      case '1':
        move(forward, 25 / 100.0 * 255);
        break;
      case '2':
        move(backward, 25 / 100.0 * 255);
        break;
      case '3':
        //Turn left at 25% speed for 1 second
        move(left, 25 / 100.0 * 255);
        _delay(1);
        move(left, 0);
        move(forward, 25 / 100.0 * 255);
        break;
      case '4':
        //Turn left at 25% speed for 1 second
        move(right, 25 / 100.0 * 255);
        _delay(1);
        move(left, 0);
        move(forward, 25 / 100.0 * 255);
        break;
      case '5':
        move(stopMoving, 0);
        break;
    }
  }
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

  commands startJourney = startJourney;
  commands stopJourney = stopJourney;

  mowerPosition.x = 0;
  mowerPosition.y = 0;

  mode = autonomous;

  Serial.println(startJourney);
  sendCoordinateToRpi(mowerPosition.x, mowerPosition.y);

  //used to calculate how long the mower has been running forward since the start
  stopTime = millis();

  //for testing
  int counter = 0;

  while (1) {

    /*   if (Serial.available() > 0) {
         char modeInput = Serial.read();
         switch (modeInput) {
           case 'a':
             mode = autonomous;
             Serial.println(startJourney);
             sendCoordinateToRpi(mowerPosition.x, mowerPosition.y);
             break;
           case 'b':
             mode = bluetooth;
             break;
           case 's':
             mode = stopRunning;
             break;
         }
       }*/

    switch (mode) {
      case autonomous:
        runAutonomous();
        break;
      case bluetooth:
        runBluetooth();
        break;
      case stopRunning:
        Serial.println(stopJourney);
        Serial.println(mowerPosition.x);
        Serial.println(mowerPosition.y);
        move(stopMoving, 0);
        stopTime = millis();
      default:
        break;
    }

    // for testing
    counter++;
    if (counter == 1000) {
      mode = stopRunning;
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
