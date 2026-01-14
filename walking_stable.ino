#include <Servo.h>
#include <math.h>

// Servo objects
Servo S1, S2, S3, S4, S5, S6, S7;
Servo S8, S9, S10, S11, S12, S13, S14, S15, S16, S17;

// Time and walking parameters
const int stepDelay = 50;
const float pi = 3.14159;
float t = 0;

// Neutral positions (in degrees)
int neutral[] = {
  80,   // S1: Head Yaw
  80,   // S2: R Shoulder Pitch
  30,   // S3: R Shoulder Roll
  40,   // S4: R Elbow
  60,   // S5: L Shoulder Pitch
  58,   // S6: L Shoulder Roll
  144,  // S7: L Elbow
  30,   // S8: R Hip Roll
  70,   // S9: R Hip Pitch
  80,   // S10: R Knee Pitch
  2,    // S11: R Ankle Pitch
  76,   // S12: R Ankle Roll
  40,   // S13: L Hip Roll
  168,  // S14: L Hip Pitch
  30,   // S15: L Knee Pitch
  168,  // S16: L Ankle Pitch
  108   // S17: L Ankle Roll
};

void setup() {
  // Attach servos to their respective pins
  S1.attach(2);   S2.attach(3);   S3.attach(4);   S4.attach(5);   S5.attach(6);
  S6.attach(7);   S7.attach(8);   S8.attach(9);   S9.attach(10);  S10.attach(11);
  S11.attach(12); S12.attach(A0); S13.attach(A1); S14.attach(A2); S15.attach(A3);
  S16.attach(A4); S17.attach(A5);
}

void loop() {
  // Calculate gait offsets (walking motion)
  float headYawOffset = 5 * sin(t);
  float RShPitchOffset = -10 * sin(t);
  float LShPitchOffset = -10 * sin(t + pi);
  float RElbowOffset = 10 * sin(t + pi);
  float LElbowOffset = 10 * sin(t);

  float RHipRollOffset = 5 * sin(t);
  float LHipRollOffset = -5 * sin(t);

  float RHipPitchOffset = 10 * sin(t);
  float LHipPitchOffset = 10 * sin(t + pi);

  float RKneeOffset = 20 * sin(t - pi / 2);
  float LKneeOffset = 20 * sin(t + pi / 2);

  float RAnklePitchOffset = 10 * sin(t + pi / 2);
  float LAnklePitchOffset = 10 * sin(t - pi / 2);

  float RAnkleRollOffset = 5 * sin(t);
  float LAnkleRollOffset = -5 * sin(t);

  // Apply neutral + gait offsets
  S1.write(neutral[0] + headYawOffset);          // Head Yaw
  S2.write(neutral[1] + RShPitchOffset);         // R Shoulder Pitch
  S3.write(neutral[2]);                          // R Shoulder Roll (static)
  S4.write(neutral[3] + RElbowOffset);           // R Elbow
  S5.write(neutral[4] + LShPitchOffset);         // L Shoulder Pitch
  S6.write(neutral[5]);                          // L Shoulder Roll (static)
  S7.write(neutral[6] + LElbowOffset);           // L Elbow

  S8.write(neutral[7] + RHipRollOffset);         // R Hip Roll
  S9.write(neutral[8] + RHipPitchOffset);        // R Hip Pitch
  S10.write(neutral[9] - RKneeOffset);           // R Knee Pitch
  S11.write(neutral[10] + RAnklePitchOffset);    // R Ankle Pitch
  S12.write(neutral[11] + RAnkleRollOffset);     // R Ankle Roll

  S13.write(neutral[12] + LHipRollOffset);       // L Hip Roll
  S14.write(neutral[13] + LHipPitchOffset);      // L Hip Pitch
  S15.write(neutral[14] - LKneeOffset);          // L Knee Pitch
  S16.write(neutral[15] + LAnklePitchOffset);    // L Ankle Pitch
  S17.write(neutral[16] + LAnkleRollOffset);     // L Ankle Roll

  // Advance time
  t += 0.1;
  if (t > 2 * pi) t = 0;

  delay(stepDelay);
}

