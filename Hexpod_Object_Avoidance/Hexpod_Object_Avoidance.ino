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
    moveLeg(pwm1, i, 375, 345, 370);    //150, 600
  }
}

void moveLegTInit() {
  for (int i = 0; i < 3; i++) {
    moveLegT(pwm2, i, 375, 407, 420);   //600, 150
  }
}

// Declare functions
void rotateLTR();
void rotateRTL();
void MoveForward();
void MoveBackward();


void setup() {
  Serial.begin(9600);

  pwm1.begin();
  pwm2.begin();

  pwm1.setPWMFreq(60);
  pwm2.setPWMFreq(60);

  moveLegInit();
  moveLegTInit();

  movePanTilt(375, 375);
  delay(3000);
  distance = readPing();
  delay(1000);
}


void loop() {
  
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
