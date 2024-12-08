#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <NewPing.h>
//Analog 4 -> SDA   komola
//Analog 5 -> SCL   holud
//CH1--3
//CH3--2
//CH4--4
//CH5--5
#define SERVOMIN 150  // Min pulse length out of 4096
#define SERVOMAX 600  // Max pulse length out of 4096

#define trig_pin A1 //analog input 1
#define echo_pin A2 //analog input 2

#define maximum_distance 200
boolean goesForward = false;
int distance = 100;

NewPing sonar(trig_pin, echo_pin, maximum_distance); //sensor function

const int ch1Pin = 2; // Channel 1 signal connected to Digital Pin 2
const int ch2Pin = 3; // Channel 2 signal connected to Digital Pin 3
const int ch3Pin = 4; // Channel 2 signal connected to Digital Pin 3
const int ch4Pin = 5;

Adafruit_PWMServoDriver pwm1 = Adafruit_PWMServoDriver();
Adafruit_PWMServoDriver pwm2 = Adafruit_PWMServoDriver(0x41);  // Assuming the second PCA9685 has address 0x41

// Define channels for the first PCA9685
const int coxaChannels[] = { 0, 3, 6 };
const int femurChannels[] = { 1, 4, 7 };
const int tibiaChannels[] = { 2, 5, 8 };

// Define channels for the second PCA9685
const int coxaChannelsT[] = { 0, 3, 6 };
const int femurChannelsT[] = { 1, 4, 7 };
const int tibiaChannelsT[] = { 2, 5, 8 };

#define PAN_SERVO_CHANNEL 9   // Channel for the pan servo
#define TILT_SERVO_CHANNEL 10  // Channel for the tilt servo

int mapValue(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void moveLeg(Adafruit_PWMServoDriver &pwm, int legIndex, int coxaPos, int femurPos, int tibiaPos) {
  pwm.setPWM(coxaChannels[legIndex], 0, coxaPos);
  pwm.setPWM(femurChannels[legIndex], 0, femurPos);
  pwm.setPWM(tibiaChannels[legIndex], 0, tibiaPos);
}

void moveLegT(Adafruit_PWMServoDriver &pwm, int legIndex, int coxaPos, int femurPos, int tibiaPos) {
  pwm.setPWM(coxaChannelsT[legIndex], 0, coxaPos);
  pwm.setPWM(femurChannelsT[legIndex], 0, femurPos);
  pwm.setPWM(tibiaChannelsT[legIndex], 0, tibiaPos);
}

void movePanTilt(int panPos, int tiltPos) {
  pwm1.setPWM(PAN_SERVO_CHANNEL, 0, panPos);
  pwm1.setPWM(TILT_SERVO_CHANNEL, 0, tiltPos);
}

void moveLegInit() {
  for (int i = 0; i < 3; i++) {
    moveLeg(pwm1, i, 375, 375, 375);    //150, 600    345, 370
  }
}

void moveLegTInit() {
  for (int i = 0; i < 3; i++) {
    moveLegT(pwm2, i, 375, 375, 375);   //600, 150    407, 420
  }
}

// Declare functions
void rotateLTR();
void rotateRTL();
void MoveForward();
void MoveBackward();
void sideLeft();
void sideRight();


void setup() {
  Serial.begin(9600);

  pwm1.begin();
  pwm2.begin();

  pwm1.setPWMFreq(60);
  pwm2.setPWMFreq(60);

  pinMode(ch1Pin, INPUT);
  pinMode(ch2Pin, INPUT);
  pinMode(ch3Pin, INPUT);

  moveLegInit();
  moveLegTInit();

  movePanTilt(375, 375);
  delay(3000);
  distance = readPing();
  delay(1000);
}

const int UP_THRESHOLD = 1900;
const int DOWN_THRESHOLD = 1000;
const int SIDE_RIGHT_THRESHOLD = 1600;
const int SIDE_LEFT_THRESHOLD = 1400;
const int ROTATE_RIGHT_THRESHOLD = 1600;
const int ROTATE_LEFT_THRESHOLD = 1400;
const int PAN_TILT_THRESHOLD = 1300;
const int AVOIDANCE_THRESHOLD = 1900;

void loop() {
  unsigned long duration1 = pulseIn(ch1Pin, HIGH); // up-down
  unsigned long duration2 = pulseIn(ch2Pin, HIGH); // side
  unsigned long duration3 = pulseIn(ch3Pin, HIGH); // rotate
  unsigned long duration4 = pulseIn(ch4Pin, HIGH); // pan/tilt toggle

  Serial.println(duration4);

  // Check if pan/tilt mode is active
  if (duration4 < PAN_TILT_THRESHOLD) {
    if (duration1 > UP_THRESHOLD) {
      Serial.println("UP");
      MoveForward(2);
    } else if (duration1 > 0 && duration1 < DOWN_THRESHOLD) {
      Serial.println("DOWN");
      MoveBackward(2);
    } else if (duration2 > SIDE_RIGHT_THRESHOLD) {
      Serial.println("Side RIGHT");
      sideRight();
    } else if (duration2 > 0 && duration2 < SIDE_LEFT_THRESHOLD) {
      Serial.println("Side LEFT");
      sideLeft();
    } else if (duration3 > ROTATE_RIGHT_THRESHOLD) {
      Serial.println("RIGHT");
      rotateLTR();
    } else if (duration3 > 0 && duration3 < ROTATE_LEFT_THRESHOLD) {
      Serial.println("LEFT");
      rotateRTL();
    }  else {
      Serial.println("No signal");
      moveLegInit();
      moveLegTInit();
    }

    // Short delay for stable reading
    delay(50);
  } 
  if (duration4 > 1300 && duration4 < 1400) {
      // Pan/tilt control mode
    int panPos = map(duration1, 2000, 1000, SERVOMIN, SERVOMAX);
    int tiltPos = map(duration2, 2000, 1000, SERVOMIN, SERVOMAX);

    movePanTilt(panPos, tiltPos);
    Serial.println("Pan/Tilt Control Active");
    Serial.print("panPos: ");
    Serial.println(panPos);
    Serial.print("tiltPos: ");
    Serial.println(tiltPos);

    delay(100);
  } else if (duration4 > 1400) {
    Serial.println("Commanding obstratcle avoidance...");
    Avoid_Obstracle();
  }
}

int lookRight(){
  Serial.println("Looking Right");
  //servo_motor.write(50);
  for(int i = 375; i >= 250; i -= 5) {
    movePanTilt(375, i);
    delay(10);
  }
  movePanTilt(375, 250);
  delay(500);
  int distance = readPing();
  delay(100);
  //servo_motor.write(115);
  for(int i = 250; i <= 375; i += 5) {
    movePanTilt(375, i);
    delay(10);
  }
  movePanTilt(375, 375);
  return distance;
}

int lookLeft(){
  //servo_motor.write(170);
  Serial.println("Looking Left");
  for(int i = 375; i <= 550; i += 5) {
    movePanTilt(375, i);
    delay(10);
  }
  movePanTilt(375, 550);
  delay(500);
  int distance = readPing();
  delay(100);
  //servo_motor.write(115);
  for(int i = 550; i >= 375; i -= 5) {
    movePanTilt(375, i);
    delay(10);
  }
  movePanTilt(375, 375);
  return distance;
  delay(100);
}

int readPing(){
  delay(70);
  int cm = sonar.ping_cm();
  if (cm==0){
    cm=250;
  }
  return cm;
}

void Avoid_Obstracle() {
  movePanTilt(375, 375);
  int distanceRight = 0;
  int distanceLeft = 0;
  delay(50);
  if (distance <= 20){
    moveLegInit();
    moveLegTInit();
    delay(300);
    Serial.println("Moving Backward");
    MoveBackward(2);
    MoveBackward(2);
    MoveBackward(2);
    delay(100);
    moveLegInit();
    moveLegTInit();
    delay(300);
    distanceRight = lookRight();
    delay(300);
    distanceLeft = lookLeft();
    delay(300);

    if (distance >= distanceLeft){
      rotateRTL();
      rotateRTL();
      rotateRTL();
      rotateRTL();
      moveLegInit();
      moveLegTInit();
      Serial.println("Rotating Right");
      delay(300);
    }
    else{
      rotateLTR();
      rotateLTR();
      rotateLTR();
      rotateLTR();
      moveLegInit();
      moveLegTInit();
      Serial.println("Rotating Left");
      delay(300);
    }
  } else{
    MoveForward(2); 
  }

  distance = readPing();
  Serial.println("Distance: ");
  Serial.println(distance);
}