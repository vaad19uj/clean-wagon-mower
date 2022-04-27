#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <MeAuriga.h>

MeEncoderOnBoard Encoder_1(SLOT1);
MeEncoderOnBoard Encoder_2(SLOT2);
MeUltrasonicSensor ultrasonic_7(7);
MeLineFollower linefollower_6(6);
MeLightSensor lightsensor_12(12);
MeGyro gyro_0(0, 0x69);

enum directions {
  directionForward,
  directionBackward,
  directionLeft,
  directionRight,
  directionStopMoving,
  noDirection
};

enum operationMode {
  notDecided,
  autonomous,
  bluetooth,
  stopAutonomous,
  stopBluetooth
};

enum lastTurn {
  none,
  leftTurn,
  rightTurn
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
directions lastDirection = noDirection;
operationMode mode = notDecided;
lastTurn lastTurn = none;
unsigned long runningForwardTime;
unsigned long backingTime;
unsigned long turningTime;
unsigned long stopTime;
bool updateStartTime = true;

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
  commands cmd = sendCoordinate;
  Serial.println(cmd);
  Serial.println(x);
  Serial.println(y);
}

void positionData(directions direction, int speed) {
  int distance = 0;
  unsigned long timePassed = 0;

  double angle = fmod(gyro_0.getAngle(3), 360.0);
  double angleInRad = radians(angle);

  switch (direction) {
    case directionForward:

      if (turningTime != -2) {
        //When running forward, we calculate how long the mower was turning left or right
        timePassed = runningForwardTime - turningTime;
        timePassed = timePassed / 1000;
        distance = timePassed * speed;

        mowerPosition.x += distance * cos(angleInRad);
        mowerPosition.y += distance * sin(angleInRad);
      }
      break;
    case directionBackward:
      //When backing, do nothing since the mower was been standing still before
      break;
    case directionLeft:
      //When turning, calculate how long the mower was backing
      timePassed = turningTime - backingTime;
      timePassed = timePassed / 1000;
      distance = timePassed * speed;

      mowerPosition.x += distance * cos(angleInRad);
      mowerPosition.y += distance * sin(angleInRad);
      break;
    case directionRight:
      //When turning, calculate how long the mower was backing
      timePassed = turningTime - backingTime;
      timePassed = timePassed / 1000;
      distance = timePassed * speed;

      mowerPosition.x += distance * cos(angleInRad);
      mowerPosition.y += distance * sin(angleInRad);
      break;
    case directionStopMoving:
      // When stopping, we calculate how long the mower has been running forward
      timePassed = stopTime - runningForwardTime;
      timePassed = timePassed / 1000;
      distance = timePassed * speed;

      mowerPosition.x += distance * cos(angleInRad);
      mowerPosition.y += distance * sin(angleInRad);
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
  if (mode == autonomous && lastDirection != direction) {
    positionData(direction, speed);
  }

  lastDirection = direction;

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
    lastTurn = rightTurn;
    //Turn right at 20% speed for 1 second
    turningTime = millis();
    move(right, 20 / 100.0 * 255);
    _delay(1);
    move(right, 0);
  }
  else {
    //Turn left at 20% speed for 1 second
    lastTurn = leftTurn;
    turningTime = millis();
    move(left, 20 / 100.0 * 255);
    _delay(1);
    move(left, 0);
  }

  turnCounter += 1;

  if (turnCounter == 10) {
    turnCounter = 0;
  }
}

void lineDetectedEvent() {
  //Stop moving
  stopTime = millis();
  move(stopMoving, 0);
  _delay(0.5);

  //Move backward at 20% speed for 1 second
  backingTime = millis();
  move(backward, 20 / 100.0 * 255);
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

  //Move backward at 20% speed for 1 second
  backingTime = millis();
  move(backward, 20 / 100.0 * 255);
  _delay(1);
  move(backward, 0);

  turn();
}

void runAutonomous() {
  int distanceToObstacle = 25;

  if (updateStartTime) {
    runningForwardTime = millis();
    updateStartTime = false;
  }

  //Move forward at 20% speed
  move(forward, 20 / 100.0 * 255);

  if (ultrasonic_7.distanceCm() < distanceToObstacle) {
    obstacleDetectedEvent();
    updateStartTime = true;

  } else if (linefollower_6.readSensors() != 3) {
    //If line follower sensor detects black
    lineDetectedEvent();
    updateStartTime = true;
  }
}

void runBluetooth() {
  if (Serial.available() > 0)
  {
    int cmd = Serial.read();
    
    switch (cmd) {
      case '1':
        move(forward, 20 / 100.0 * 255);
        break;
      case '2':
        move(backward, 20 / 100.0 * 255);
        break;
      case '3':
        //Turn left at 20% speed for 1 second
        move(left, 20 / 100.0 * 255);
        _delay(1);
        move(left, 0);
        move(forward, 20 / 100.0 * 255);
        break;
      case '4':
        //Turn left at 20% speed for 1 second
        move(right, 20 / 100.0 * 255);
        _delay(1);
        move(left, 0);
        move(forward, 20 / 100.0 * 255);
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
  gyro_0.begin();

  commands cmdStartJourney = startJourney;
  commands cmdStopJourney = stopJourney;

  mowerPosition.x = 0;
  mowerPosition.y = 0;

  //Wait for rpi to tell us to the operation mode
  while (Serial.available() == 0) {}

  char modeInput = Serial.read();
  switch (modeInput) {
    case 'a':
      //Start running autonomous and then send startJourney command
      mode = autonomous;
      Serial.println(startJourney);
      sendCoordinateToRpi(mowerPosition.x, mowerPosition.y);
      break;
    case 'b':
      mode = bluetooth;
      break;
    default:
      break;
  }

  //Used to calculate coordinates, set to "bad value" when no turns has been made yet
  turningTime = -2;

  while (1) {

    switch (mode) {
      case autonomous:
        runAutonomous();
        break;
      case bluetooth:
        runBluetooth();
        break;
      case stopAutonomous:
        Serial.println(cmdStopJourney);
        Serial.println(mowerPosition.x);
        Serial.println(mowerPosition.y);
        stopTime = millis();
        move(stopMoving, 0);
        break;
      case stopBluetooth:
        move(stopMoving, 0);
        break;
      default:
        break;
    }

  /*  if (Serial.available() > 0) {
      char modeInput = Serial.read();
      switch (modeInput) {
        case 's':
          mode = stopAutonomous;
          break;
        case 'd':
          mode = stopBluetooth;
          break;
        default:
          break;
      }
    }

    */

    if (mode == stopAutonomous || mode == stopBluetooth) {
      break;
    }

    _loop();
  }
}

void _loop() {
  gyro_0.update();
  Encoder_1.loop();
  Encoder_2.loop();
}

void loop() {
  _loop();
}
