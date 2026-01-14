#include <Wire.h>

// Robokits servo controller I2C addresses
#define servo1  (16>>1)
#define servo2  (18>>1)
#define LED 13

// System configuration
const int NUM_SERVOS = 17;
int lockedAngles[NUM_SERVOS];

// step 2
// int defaultAngles[NUM_SERVOS] = {75, 50, 25, 70, 30, 50, 144, 30, 70, 70, 2, 76, 40, 168, 30, 168, 108}; 
//  step 
// int defaultAngles[NUM_SERVOS] ={
//   75, 75, 25, 35, 35, 55, 144, 40, 95, 80, 2, 76, 40, 168, 30, 168, 108
// };
// int defaultAngles[NUM_SERVOS] = {80, 80, 50, 55, 30, 58, 144, 15, 60, 80, 2, 76, 40, 168, 30, 168, 108}; 
int defaultAngles[NUM_SERVOS] = {80, 80, 30, 40, 30, 58, 144, 30, 70, 80, 2, 76, 40, 168, 30, 168, 108}; 
int currentServo = 0;
int currentAngle = 0;
bool movingUp = true;
unsigned long lastUpdate = 0;
bool isRotating = false;
char LEDState = 0;

// Function declarations
void I2C_SERVOSET(unsigned char servo_num, unsigned int servo_pos);
void I2C_SERVOSPEED(unsigned char value);
void I2C_SERVOMIN(unsigned char servo_num, unsigned int servo_pos);
void I2C_SERVOMAX(unsigned char servo_num, unsigned int servo_pos);
void I2C_SERVOOFFSET(unsigned char servo_num, int value);
void I2C_SERVOREVERSE(unsigned char servo_num, unsigned char servo_dir);
char I2C_SERVOEND(void);
void LEDToggle(void);
void initializeServos(void);
void startServoRotation(int servoNum);
void rotateCurrentServo(void);
void showAllAngles(void);
unsigned int angleToPosition(int angle);

void setup() {
  Serial.begin(9600);
  pinMode(LED, OUTPUT);
  
  Wire.begin();
  TWSR = 3;
  TWBR = 10;
  delay(500);
  
  initializeServos();
  
  // Initialize locked angles with default values
  for (int i = 0; i < NUM_SERVOS; i++) {
    lockedAngles[i] = defaultAngles[i];
  }
  
  Serial.println(F("\n=== SERVO CONTROL READY ==="));
  Serial.println(F("Enter servo number (1-17) to rotate"));
  Serial.println(F("Enter 0 to show all angles"));
  Serial.print(F("Which servo to rotate? "));
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readString();
    input.trim();
    
    if (!isRotating) {
      // Asking for servo number
      int servoNum = input.toInt();
      
      if (servoNum == 0) {
        showAllAngles();
        Serial.print(F("Which servo to rotate? "));
      }
      else if (servoNum >= 1 && servoNum <= NUM_SERVOS) {
        currentServo = servoNum;
        startServoRotation(servoNum);
      }
      else {
        Serial.println(F("Invalid! Enter 1-17 or 0 for angles"));
        Serial.print(F("Which servo to rotate? "));
      }
    }
    else {
      // User pressed ENTER to lock current angle
      lockedAngles[currentServo - 1] = currentAngle;
      Serial.print(F("Locked Servo "));
      Serial.print(currentServo);
      Serial.print(F(" at "));
      Serial.print(currentAngle);
      Serial.println(F(" degrees"));
      
      isRotating = false;
      delay(1000);
      
      Serial.println(F("Enter servo number (1-17) to rotate"));
      Serial.println(F("Enter 0 to show all angles"));
      Serial.print(F("Which servo to rotate? "));
    }
  }
  
  // Rotate current servo if active
  if (isRotating) {
    rotateCurrentServo();
  }
  
  LEDToggle();
}

void startServoRotation(int servoNum) {
  currentAngle = 0;
  movingUp = true;
  lastUpdate = millis();
  isRotating = true;
  
  Serial.print(F("Rotating Servo "));
  Serial.print(servoNum);
  Serial.println(F(" - Press ENTER to lock angle"));
  
  I2C_SERVOSET(servoNum, angleToPosition(currentAngle));
}

void rotateCurrentServo() {
  if (millis() - lastUpdate >= 200) {  // Slower rotation - 200ms delay
    lastUpdate = millis();
    
    if (movingUp) {
      currentAngle += 5;  // Smaller steps for smoother control
      if (currentAngle >= 180) {
        currentAngle = 180;
        movingUp = false;
      }
    } else {
      currentAngle -= 5;
      if (currentAngle <= 0) {
        currentAngle = 0;
        movingUp = true;
      }
    }
    
    I2C_SERVOSET(currentServo, angleToPosition(currentAngle));
    
    Serial.print(F("Servo "));
    Serial.print(currentServo);
    Serial.print(F(" -> "));
    Serial.print(currentAngle);
    Serial.println(F(" degrees"));
  }
}

void showAllAngles() {
  Serial.println(F("\n=== ALL SERVO ANGLES ==="));
  for (int i = 0; i < NUM_SERVOS; i++) {
    Serial.print(F("Servo "));
    Serial.print(i + 1);
    Serial.print(F(": "));
    Serial.print(lockedAngles[i]);
    Serial.println(F(" degrees"));
  }
  Serial.println(F("========================\n"));
}

void initializeServos() {
  Serial.print(F("Initializing servos"));
  
  I2C_SERVOSPEED(30);  // Slower speed for better control
  Serial.print(F("."));
  
  for (int i = 1; i <= NUM_SERVOS; i++) {
    I2C_SERVOMAX(i, 2500);
    I2C_SERVOMIN(i, 500);
    I2C_SERVOOFFSET(i, 1500);
    I2C_SERVOREVERSE(i, 0);
    delay(10);
    if (i % 5 == 0) Serial.print(F("."));
  }
  
  // Set all servos to default positions
  for (int i = 1; i <= NUM_SERVOS; i++) {
    I2C_SERVOSET(i, angleToPosition(defaultAngles[i - 1]));
    delay(20);
  }
  
  while (!I2C_SERVOEND()) {
    delay(10);
  }
  
  Serial.println(F(" Done!"));
}

unsigned int angleToPosition(int angle) {
  return map(angle, 0, 180, 500, 2500);
}

// ============== I2C SERVO FUNCTIONS ==============

void I2C_SERVOSET(unsigned char servo_num, unsigned int servo_pos) {
  if (servo_pos < 500) servo_pos = 500;
  else if (servo_pos > 2500) servo_pos = 2500;

  if (servo_pos > 501)
    servo_pos = (((servo_pos - 2) * 2) - 1000);
  else
    servo_pos = 0;

  if (servo_num < 19)
    Wire.beginTransmission(servo1);
  else
    Wire.beginTransmission(servo2);
  Wire.write(servo_num - 1);
  Wire.write(servo_pos >> 8);
  Wire.write(servo_pos & 0XFF);
  Wire.endTransmission();
}

void I2C_SERVOSPEED(unsigned char value) {
  Wire.beginTransmission(servo1);
  Wire.write(18 * 2);
  Wire.write(value);
  Wire.write(0);
  Wire.endTransmission();
  Wire.beginTransmission(servo2);
  Wire.write(18 * 2);
  Wire.write(value);
  Wire.write(0);
  Wire.endTransmission();
  delay(20);
}

void I2C_SERVOMIN(unsigned char servo_num, unsigned int servo_pos) {
  if (servo_pos < 500) servo_pos = 500;
  else if (servo_pos > 2500) servo_pos = 2500;
  servo_pos = ((servo_pos * 2) - 1000);

  if (servo_num < 19)
    Wire.beginTransmission(servo1);
  else
    Wire.beginTransmission(servo2);
  Wire.write((servo_num - 1) + (18 * 4));
  Wire.write(servo_pos >> 8);
  Wire.write(servo_pos & 0XFF);
  Wire.endTransmission();
  delay(20);
}

void I2C_SERVOMAX(unsigned char servo_num, unsigned int servo_pos) {
  if (servo_pos < 500) servo_pos = 500;
  else if (servo_pos > 2500) servo_pos = 2500;
  servo_pos = ((servo_pos * 2) - 1000);

  if (servo_num < 19)
    Wire.beginTransmission(servo1);
  else
    Wire.beginTransmission(servo2);
  Wire.write((servo_num - 1) + (18 * 3));
  Wire.write(servo_pos >> 8);
  Wire.write(servo_pos & 0XFF);
  Wire.endTransmission();
  delay(20);
}

void I2C_SERVOOFFSET(unsigned char servo_num, int value) {
  value = 3000 - value;
  value = value - 1500;

  if (value < -500) value = -500;
  else if (value > 500) value = 500;

  if (value > 0)
    value = 2000 + (value * 2);
  else if (value <= 0)
    value = -value * 2;

  if (servo_num < 19)
    Wire.beginTransmission(servo1);
  else
    Wire.beginTransmission(servo2);
  Wire.write((servo_num - 1) + (18 * 6));
  Wire.write(value >> 8);
  Wire.write(value & 0XFF);
  Wire.endTransmission();
  delay(20);
}

void I2C_SERVOREVERSE(unsigned char servo_num, unsigned char servo_dir) {
  if (servo_dir > 0) servo_dir = 1;
  if (servo_num < 19)
    Wire.beginTransmission(servo1);
  else
    Wire.beginTransmission(servo2);
  Wire.write((servo_num - 1) + (18 * 7));
  Wire.write(servo_dir);
  Wire.write(0);
  Wire.endTransmission();
  delay(20);
}

char I2C_SERVOEND(void) {
  int n;
  char buffer;
  Wire.beginTransmission(servo1);
  n = Wire.write(181);
  if (n != 1) return (-10);

  n = Wire.endTransmission(false);
  if (n != 0) return (n);

  delayMicroseconds(350);
  Wire.requestFrom(servo1, 1, true);
  while (Wire.available())
    buffer = Wire.read();

  return (buffer);
}

void LEDToggle(void) {
  LEDState = !LEDState;
  digitalWrite(LED, LEDState);
}