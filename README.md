# 🤖 17 DOF Humanoid Robot Walking (RKI-1204 Servo Controller)

This repository contains code files for controlling a **17 Degree of Freedom (DOF) humanoid robot** using the **RKI-1204 Servo Controller Board**, which supports up to 18 servo motors. The robot performs different walking behaviors like **straight walking**, **circular walking**, and includes **manual servo control via serial commands**.

---

## 📁 Project Files

| File Name                 | Description                                         |
|--------------------------|-----------------------------------------------------|
| `completeed_walking.ino` | Final integrated walking logic for the robot        |
| `servo_manual_serial.ino`| Manual control of individual servos via serial input|
| `walking_in_circle.ino`  | Circular walking pattern                            |
| `walking_stable.ino`     | Stable linear walking pattern                       |

---

## ⚙️ Hardware Used

- **17 Servo Motors** (used in 17 DOF humanoid body)
- **RKI-1204 Servo Controller Board** (18-channel)
- **Power Supply** (sufficient for running 17 servo motors)
- Frame & mechanical parts for humanoid structure
- PC/laptop for uploading code and serial communication

---

## 🔌 Connections

- Each servo is connected to one channel (1 to 17) of the RKI-1204 board
- RKI-1204 handles all servo PWM generation internally
- Power to RKI-1204 board must be stable (recommended 5V–6V @ 5A or more)
- Serial commands can be sent directly to the controller for testing

---

## 💡 Features

- Manual control of each servo via serial input
- Predefined walking gaits: stable walk & circular walk
- Smooth walking cycles implemented through precise angle control

---

## ⚠️ Power Supply Tip

- Use **dedicated high-current power** (5V–6V) for the RKI-1204
- Do **not power servos through USB**
- Make sure **GND** of power supply and serial source are common
