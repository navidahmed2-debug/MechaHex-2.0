#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
//Analog 4 -> SDA   komola
//Analog 5 -> SCL   holud

#define SERVOMIN 150  // Min pulse length out of 4096
#define SERVOMAX 600  // Max pulse length out of 4096


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

void movePanTilt(int panPos, int tiltPos) {
  pwm1.setPWM(PAN_SERVO_CHANNEL, 0, panPos);
  pwm1.setPWM(TILT_SERVO_CHANNEL, 0, tiltPos);
}

void setup() {
  Serial.begin(9600);

  pwm1.begin();
  pwm2.begin();

  pwm1.setPWMFreq(60);
  pwm2.setPWMFreq(60);

  moveLegInit();
  moveLegTInit();
  movePanTilt(375, 375);
  delay(5000);
  //startTime = millis();  // Initialize start time
}

void loop() {
  // Leg Forward
  moveLeg(pwm1, 0, 375, 345, 370);    //1     //p=4, z=6.8       // Expand motion
  moveLeg(pwm1, 1, 375, 340, 327);    //2     //p=2, z=6
  moveLeg(pwm1, 2, 350, 345, 370);    //3     //p=3, z=6.5

  /*moveLegT(pwm2, 0, 375, 407, 420);     //2   //p=2, z=6 
  moveLegT(pwm2, 1, 375, 402, 377);     //1   //p=4, z=6.8                // Expand motion  407, 420);     //2   //p=2, z=6  
  moveLegT(pwm2, 2, 350, 402, 397);     //3   //p=3, z=6.5*/
  for (int i = 0; i <= 100; i += 5) {
    moveLegT(pwm2, 0, 375, mapValue(i, 0, 100, 407, 480), mapValue(i, 0, 100, 420, 467));   //375, 407, 420
    moveLegT(pwm2, 1, 375, mapValue(i, 0, 100, 402, 462), mapValue(i, 0, 100, 377, 420));   //375, 402, 377           // Condense motion
    moveLegT(pwm2, 2, 350, mapValue(i, 0, 100, 402, 470), mapValue(i, 0, 100, 397, 442));   //395, 402, 397
    delay(5);
  }


  moveLegT(pwm2, 0, 375, 480, 467);     //1   //p=2, z=4          //Condence
  moveLegT(pwm2, 1, 375, 462, 420);     //1   //p=4, z=4.8
  moveLegT(pwm2, 2, 350, 470, 442);     //3   //p=3, z=4.5
  //delay(100);

  /*moveLeg(pwm1, 0, 375, 345, 370);    //1     //p=4, z=6.8       // Expand motion
  moveLeg(pwm1, 1, 375, 340, 327);    //2     //p=2, z=6
  moveLeg(pwm1, 2, 400, 345, 350);    //3     //p=3, z=6.5*/
  // Condense motion
  for (int i = 0; i <= 100; i += 5) {
    moveLeg(pwm1, 0, 375, mapValue((100 - i), 0, 100, 340, 345), mapValue((100 - i), 0, 100, 327, 370));     //375, 295, 367     // Condense motion
    moveLeg(pwm1, 1, 375, mapValue(i, 0, 100, 340, 345), mapValue(i, 0, 100, 327, 370));    //375, 315, 427                     // Expand motion
    moveLeg(pwm1, 2, mapValue(i, 0, 100, 350, 400), 345, 370);    //395, 302, 397


    moveLegT(pwm2, 0, 375, mapValue((100 - i), 0, 100, 462, 480), mapValue((100 - i), 0, 100, 420, 467));   //375, 477, 345  //p=8.5, z=3
    moveLegT(pwm2, 1, 375, mapValue(i, 0, 100, 462, 480), mapValue(i, 0, 100, 420, 467));   //375, 510, 410        //p=6, z=3   // Condense motion
    moveLegT(pwm2, 2, mapValue(i, 0, 100, 350, 400), 470, 442);   //355, 495, 377   ////p=7.25, z=3.7
    delay(10);
  }
  delay(100);




//LegT Forward-------------------------------------------
  /*moveLegT(pwm2, 0, 375, 462, 420);     //1   //p=4, z=4.8           //Condence
  moveLegT(pwm2, 1, 375, 480, 467);     //1   //p=2, z=4
  moveLegT(pwm2, 2, 350, 470, 442);     //3   //p=3, z=4.5*/
  for (int i = 0; i <= 100; i += 5) {
    moveLegT(pwm2, 0, 375, mapValue((100 - i), 0, 100, 402, 462), mapValue((100 - i), 0, 100, 377, 420));   //375, 440, 407  //p=8.5, z=3
    moveLegT(pwm2, 1, 375, mapValue((100 - i), 0, 100, 407, 480), mapValue((100 - i), 0, 100, 420, 467));   //375, 450, 450        //p=6, z=3   // Condense motion
    moveLegT(pwm2, 2, 400, mapValue((100 - i), 0, 100, 402, 470), mapValue((100 - i), 0, 100, 397, 422));   //355, 442, 427   ////p=7.25, z=3.7
    delay(5);
  }

  moveLegT(pwm2, 0, 375, 402, 377);     //1   //p=2, z=6
  moveLegT(pwm2, 1, 375, 407, 420);     //2   //p=4, z=6.8p=2, z=6                  // Expand motion
  moveLegT(pwm2, 2, 400, 402, 397);     //3   //p=3, z=6.5
  
  /*moveLeg(pwm1, 0, 375, 340, 327);    //2     //p=2, z=6       // Expand motion  
  moveLeg(pwm1, 1, 375, 345, 370);    //1     //p=4, z=6.8
  moveLeg(pwm1, 2, 400, 345, 350);    //3     //p=3, z=6.5*/
  for (int i = 0; i <= 100; i += 5) {
    moveLeg(pwm1, 0, 375, mapValue((100 - i), 0, 100, 267, 340), mapValue((100 - i), 0, 100, 280, 327));     //375, 340, 327     // Condense motion
    moveLeg(pwm1, 1, 375, mapValue((100 - i), 0, 100, 285, 345), mapValue((100 - i), 0, 100, 327, 370));    //375, 345, 370                     // Expand motion
    moveLeg(pwm1, 2, 400, mapValue((100 - i), 0, 100, 277, 345), mapValue((100 - i), 0, 100, 305, 370));    //395, 355, 350
    delay(5);
  }
  moveLeg(pwm1, 0, 375, 267, 280);    //1     //p=2, z=4
  moveLeg(pwm1, 1, 375, 285, 327);    //1     //p=4, z=4.8                                // Expand motion
  moveLeg(pwm1, 2, 400, 277, 305);    //3     //p=3, z=4.5

  // condance motion
  for (int i = 0; i <= 100; i += 5) {
    moveLegT(pwm2, 0, 375, mapValue(i, 0, 100, 402, 407), mapValue(i, 0, 100, 377, 420));   //375, 452, 380
    moveLegT(pwm2, 1, 375, mapValue((100 - i), 0, 100, 402, 407), mapValue((100 - i), 0, 100, 377, 420));   //375, 432, 320           // Condense motion
    moveLegT(pwm2, 2, mapValue((100 - i), 0, 100, 350, 400), 402, 397);   //395, 445, 350


    moveLeg(pwm1, 0, 375, mapValue(i, 0, 100, 267, 285), mapValue(i, 0, 100, 280, 327));     //375, 270, 402
    moveLeg(pwm1, 1, 375, mapValue((100 - i), 0, 100, 267, 285), mapValue((100 - i), 0, 100, 280, 327));    //375, 237, 337           // Condense motion
    moveLeg(pwm1, 2, mapValue((100 - i), 0, 100, 350, 400), 277, 305);    //355, 270, 377
    delay(10);
  }
  //H
  moveLeg(pwm1, 0, 375, 285, 327);    //1     //p=4, z=4.8
  moveLeg(pwm1, 1, 375, 267, 280);    //1     //p=2, z=4                                // Expand motion      
  moveLeg(pwm1, 2, 350, 277, 305);    //3     //p=3, z=4.5
  for (int i = 0; i <= 100; i += 5) {
    moveLeg(pwm1, 0, 375, mapValue(i, 0, 100, 285, 345), mapValue(i, 0, 100, 327, 370));     //375, 307, 340     // Condense motion
    moveLeg(pwm1, 1, 375, mapValue(i, 0, 100, 267, 340), mapValue(i, 0, 100, 280, 327));    //375, 297, 297                     // Expand motion
    moveLeg(pwm1, 2, 350, mapValue(i, 0, 100, 277, 345), mapValue(i, 0, 100, 305, 370));    //395, 305, 320
    delay(5);
  }
  delay(100);
  /*moveLeg(pwm1, 0, 375, 345, 370);    //1     //p=4, z=6.8       // Expand motion
  moveLeg(pwm1, 1, 375, 340, 327);    //2     //p=2, z=6
  moveLeg(pwm1, 2, 400, 345, 350);    //3     //p=3, z=6.5*/

}