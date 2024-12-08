#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
//Analog 4 -> SDA   komola
//Analog 5 -> SCL   holud
//CH1--3
//CH3--2
//CH4--4
//CH5--5
#define SERVOMIN 150  // Min pulse length out of 4096
#define SERVOMAX 600  // Max pulse length out of 4096

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
    moveLeg(pwm1, i, 375, 375, 375);    //150, 600
  }
}

void moveLegTInit() {
  for (int i = 0; i < 3; i++) {
    moveLegT(pwm2, i, 375, 375, 375);   //600, 150
  }
}

// Declare functions
void rotateLTR();
void rotateRTL();
void MoveForward();
void MoveBackward();

unsigned long startTime;

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
  delay(5000);
  //startTime = millis();  // Initialize start time
}

const int UP_THRESHOLD = 1900;
const int DOWN_THRESHOLD = 1000;
const int SIDE_RIGHT_THRESHOLD = 1600;
const int SIDE_LEFT_THRESHOLD = 1400;
const int ROTATE_RIGHT_THRESHOLD = 1600;
const int ROTATE_LEFT_THRESHOLD = 1400;
const int PAN_TILT_THRESHOLD = 1400;

void loop() {
  unsigned long duration1 = pulseIn(ch1Pin, HIGH); // up-down
  unsigned long duration2 = pulseIn(ch2Pin, HIGH); // side
  unsigned long duration3 = pulseIn(ch3Pin, HIGH); // rotate
  unsigned long duration4 = pulseIn(ch4Pin, HIGH); // pan/tilt toggle

  // Check if pan/tilt mode is active
  if (duration4 < PAN_TILT_THRESHOLD) {
    if (duration1 > UP_THRESHOLD) {
      Serial.println("UP");
      MoveForward();
    } else if (duration1 > 0 && duration1 < DOWN_THRESHOLD) {
      Serial.println("DOWN");
      MoveBackward();
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
    } else {
      Serial.println("No signal");
      moveLegInit();
      moveLegTInit();
    }

    // Short delay for stable reading
    delay(50);
  } else {
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
  }
}
