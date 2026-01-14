#include <Wire.h>

// Robokits servo controller I2C addresses
#define servo1  (16>>1)
#define servo2  (18>>1)
#define LED 13

// System configuration
const int NUM_SERVOS = 17;

// Default stable stance angles
int defaultAngles[NUM_SERVOS] = {80, 85, 25, 35, 30, 58, 139, 25, 75, 80, 2, 76, 40, 168, 30, 168, 108};

// YOUR CUSTOM WALKING STEPS (4 steps only)
int customLeftStep1[NUM_SERVOS] = {
  80, 80, 30, 40, 30, 50, 144, 30, 70, 70, 2, 76, 40, 168, 30, 168, 108
};
int customLeftStep2[NUM_SERVOS] = {
  75, 50, 25, 70, 30, 50, 144, 30, 70, 70, 2, 76, 40, 168, 30, 168, 108
};
int customLeftStep3[NUM_SERVOS] = {
  75, 50, 25, 65, 35, 55, 144, 15, 70, 80, 2, 76, 40, 168, 30, 168, 108
};
int customLeftStep4[NUM_SERVOS] = {
  75, 75, 25, 35, 35, 55, 144, 40, 95, 80, 2, 76, 40, 168, 30, 168, 108
};

// Current angles storage
int currentAngles[NUM_SERVOS];

// Walking control variables
bool isWalking = false;
int currentStep = 0;
unsigned long lastStepTime = 0;
int stepDelay = 550;
int moveSpeed = 30;

// Enhanced stability variables
bool useSmoothing = true;
int smoothingSteps = 4;
bool autoBalance = true;

// Function declarations
void I2C_SERVOSET(unsigned char servo_num, unsigned int servo_pos);
void I2C_SERVOSPEED(unsigned char value);
void I2C_SERVOMIN(unsigned char servo_num, unsigned int servo_pos);
void I2C_SERVOMAX(unsigned char servo_num, unsigned int servo_pos);
void I2C_SERVOOFFSET(unsigned char servo_num, int value);
void I2C_SERVOREVERSE(unsigned char servo_num, unsigned char servo_dir);
char I2C_SERVOEND(void);
void initializeRobot(void);
void setDefaultPosition(void);
void startWalking(void);
void stopWalking(void);
void executeWalkingStep(void);
void moveToPosition(int targetAngles[]);
void smoothMoveToPosition(int targetAngles[]);
void emergencyStop(void);
unsigned int angleToPosition(int angle);
void printMenu(void);
void printStatus(void);
bool validateAngles(int angles[]);

void setup() {
  Serial.begin(9600);
  pinMode(LED, OUTPUT);
  
  Wire.begin();
  TWSR = 3;
  TWBR = 10;
  delay(500);
  
  Serial.println(F("\n===== 17 DOF HUMANOID ROBOT - CUSTOM WALK ====="));
  initializeRobot();
  setDefaultPosition();
  
  // Initialize current angles with default values
  for (int i = 0; i < NUM_SERVOS; i++) {
    currentAngles[i] = defaultAngles[i];
  }
  
  Serial.println(F("✓ Custom 4-step walking cycle loaded"));
  Serial.println(F("✓ Robot ready!"));
  printMenu();
}

void loop() {
  // Check for serial commands
  if (Serial.available() > 0) {
    String command = Serial.readString();
    command.trim();
    command.toLowerCase();
    
    if (command == "walk" || command == "w") {
      if (!isWalking) {
        startWalking();
      } else {
        Serial.println(F("⚠️ Already walking!"));
      }
    }
    else if (command == "stop" || command == "s") {
      if (isWalking) {
        stopWalking();
      } else {
        Serial.println(F("ℹ️ Not walking!"));
      }
    }
    else if (command == "emergency" || command == "e") {
      emergencyStop();
    }
    else if (command == "reset" || command == "r") {
      stopWalking();
      setDefaultPosition();
      Serial.println(F("✓ Robot reset to default position"));
    }
    else if (command == "speed+" || command == "sp+") {
      if (stepDelay > 200) {
        stepDelay -= 50;
        Serial.print(F("⬆️ Speed increased - Delay: "));
        Serial.print(stepDelay);
        Serial.println(F("ms"));
      }
    }
    else if (command == "speed-" || command == "sp-") {
      if (stepDelay < 1000) {
        stepDelay += 50;
        Serial.print(F("⬇️ Speed decreased - Delay: "));
        Serial.print(stepDelay);
        Serial.println(F("ms"));
      }
    }
    else if (command == "smooth" || command == "sm") {
      useSmoothing = !useSmoothing;
      Serial.print(F("🔄 Smooth movement: "));
      Serial.println(useSmoothing ? "ON" : "OFF");
    }
    else if (command == "status" || command == "st") {
      printStatus();
    }
    else if (command == "menu" || command == "m") {
      printMenu();
    }
    else {
      Serial.println(F("❌ Unknown command! Type 'menu' for help"));
    }
  }
  
  // Execute walking if active
  if (isWalking) {
    executeWalkingStep();
  }
  
  // LED indicator
  if (isWalking) {
    static unsigned long ledTime = 0;
    if (millis() - ledTime > 200) {
      digitalWrite(LED, !digitalRead(LED));
      ledTime = millis();
    }
  } else {
    digitalWrite(LED, LOW);
  }
}

void printMenu() {
  Serial.println(F("\n===== 🤖 ROBOT CONTROL MENU ====="));
  Serial.println(F("🚶 walk / w      - Start custom walking"));
  Serial.println(F("🛑 stop / s      - Stop walking"));
  Serial.println(F("🚨 emergency / e - Emergency stop"));
  Serial.println(F("🔄 reset / r     - Reset position"));
  Serial.println(F("⬆️ speed+ / sp+  - Increase speed"));
  Serial.println(F("⬇️ speed- / sp-  - Decrease speed"));
  Serial.println(F("✨ smooth / sm   - Toggle smooth movement"));
  Serial.println(F("📊 status / st   - Show status"));
  Serial.println(F("📋 menu / m      - Show menu"));
  Serial.println(F("=================================\n"));
}

void printStatus() {
  Serial.println(F("\n===== 📊 STATUS ====="));
  Serial.print(F("Walking: ")); Serial.println(isWalking ? "✅ YES" : "❌ NO");
  Serial.print(F("Step Delay: ")); Serial.print(stepDelay); Serial.println(F("ms"));
  Serial.print(F("Smooth Movement: ")); Serial.println(useSmoothing ? "✅ ON" : "❌ OFF");
  Serial.print(F("Current Step: ")); Serial.println(currentStep + 1);
  Serial.println(F("====================\n"));
}

void startWalking() {
  Serial.println(F("🚶 Starting custom 4-step walking..."));
  
  isWalking = true;
  currentStep = 0;
  lastStepTime = millis();
  
  // Start with first step
  if (useSmoothing) {
    smoothMoveToPosition(customLeftStep1);
  } else {
    moveToPosition(customLeftStep1);
  }
}

void stopWalking() {
  Serial.println(F("🛑 Stopping walking..."));
  isWalking = false;
  
  delay(150);
  if (useSmoothing) {
    smoothMoveToPosition(defaultAngles);
  } else {
    setDefaultPosition();
  }
  Serial.println(F("✅ Walking stopped"));
}

void emergencyStop() {
  Serial.println(F("🚨 EMERGENCY STOP!"));
  isWalking = false;
  setDefaultPosition();
  Serial.println(F("✅ Emergency stop complete"));
}

void executeWalkingStep() {
  if (millis() - lastStepTime >= stepDelay) {
    lastStepTime = millis();
    
    switch (currentStep) {
      case 0:
        Serial.println(F("Step 1"));
        if (useSmoothing) smoothMoveToPosition(customLeftStep1);
        else moveToPosition(customLeftStep1);
        break;
      case 1:
        Serial.println(F("Step 2"));
        if (useSmoothing) smoothMoveToPosition(customLeftStep2);
        else moveToPosition(customLeftStep2);
        break;
      case 2:
        Serial.println(F("Step 3"));
        if (useSmoothing) smoothMoveToPosition(customLeftStep3);
        else moveToPosition(customLeftStep3);
        break;
      case 3:
        Serial.println(F("Step 4"));
        if (useSmoothing) smoothMoveToPosition(customLeftStep4);
        else moveToPosition(customLeftStep4);
        break;
    }
    
    currentStep++;
    if (currentStep >= 4) {
      currentStep = 0;
      Serial.println(F("🔄 Cycle complete - Repeating"));
    }
  }
}

bool validateAngles(int angles[]) {
  for (int i = 0; i < NUM_SERVOS; i++) {
    if (angles[i] < 0 || angles[i] > 180) {
      Serial.print(F("⚠️ Invalid angle for servo "));
      Serial.print(i + 1);
      Serial.print(F(": "));
      Serial.println(angles[i]);
      return false;
    }
  }
  return true;
}

void smoothMoveToPosition(int targetAngles[]) {
  if (!validateAngles(targetAngles)) {
    Serial.println(F("❌ Invalid angles - movement aborted"));
    return;
  }
  
  // Calculate intermediate steps for smooth movement
  for (int step = 1; step <= smoothingSteps; step++) {
    for (int i = 0; i < NUM_SERVOS; i++) {
      int startAngle = currentAngles[i];
      int targetAngle = constrain(targetAngles[i], 0, 180);
      
      float progress = (float)step / smoothingSteps;
      progress = progress * progress * (3.0 - 2.0 * progress); // Smooth curve
      
      int intermediateAngle = startAngle + ((targetAngle - startAngle) * progress);
      
      I2C_SERVOSET(i + 1, angleToPosition(intermediateAngle));
      delay(2);
    }
    delay(25);
  }
  
  // Update current angles
  for (int i = 0; i < NUM_SERVOS; i++) {
    currentAngles[i] = constrain(targetAngles[i], 0, 180);
  }
}

void moveToPosition(int targetAngles[]) {
  if (!validateAngles(targetAngles)) {
    Serial.println(F("❌ Invalid angles - movement aborted"));
    return;
  }
  
  for (int i = 0; i < NUM_SERVOS; i++) {
    int safeAngle = constrain(targetAngles[i], 0, 180);
    I2C_SERVOSET(i + 1, angleToPosition(safeAngle));
    currentAngles[i] = safeAngle;
    delay(3);
  }
}

void setDefaultPosition() {
  Serial.print(F("🏠 Moving to default position... "));
  
  I2C_SERVOSPEED(moveSpeed);
  
  for (int i = 0; i < NUM_SERVOS; i++) {
    I2C_SERVOSET(i + 1, angleToPosition(defaultAngles[i]));
    currentAngles[i] = defaultAngles[i];
    delay(8);
  }
  
  while (!I2C_SERVOEND()) {
    delay(15);
  }
  
  Serial.println(F("✅ Ready!"));
}

void initializeRobot() {
  Serial.print(F("🔧 Initializing robot"));
  
  I2C_SERVOSPEED(moveSpeed);
  
  for (int i = 1; i <= NUM_SERVOS; i++) {
    I2C_SERVOMAX(i, 2500);
    I2C_SERVOMIN(i, 500);
    I2C_SERVOOFFSET(i, 1500);
    I2C_SERVOREVERSE(i, 0);
    delay(12);
    if (i % 4 == 0) Serial.print(F("."));
  }
  
  Serial.println(F(" ✅ Complete!"));
}

unsigned int angleToPosition(int angle) {
  angle = constrain(angle, 0, 180);
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