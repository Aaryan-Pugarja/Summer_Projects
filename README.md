# Arduino Uno R3 Project Sketches

This package contains one Arduino sketch for each project in the supplied PDF:

1. Automatic Night Lamp
2. Joystick Controlled Servo
3. Accident Detection and Emergency Alert
4. Industrial Gas and Temperature Monitor
5. Intelligent Liquid Volume Estimator
6. Smart Battery Monitoring and Intelligent Charging System
7. MPU6050 Controlled 2WD Robot Car
8. RFID-Based Smart Attendance System
9. Water Quality Monitor
10. Smart Irrigation Controller

## External libraries

Install these through Arduino IDE Library Manager where applicable:

- Servo (normally included with Arduino IDE)
- DHT sensor library by Adafruit
- MFRC522
- Adafruit INA219
- Adafruit MAX1704X

The MPU6050 sketches communicate directly over I2C and therefore need only the
built-in Wire library.

## Important calibration

The following values are examples and must be calibrated using the actual
sensor modules:

- LDR light threshold
- Pulse-sensor thresholds
- MQ-2 warning levels
- MPU6050 impact and tilt thresholds
- Container radius and height
- TDS calibration factor
- Water-level threshold
- Soil dry/wet thresholds
- Relay active-high/active-low setting

## Electrical safety

- Do not power motors, pumps, or servos directly from an Arduino I/O pin.
- Use a separate suitable power supply and a common ground where required.
- The MFRC522 is a 3.3 V device.
- The TP4056 is intended for one Li-ion cell. Do not use it to charge two cells
  in series.
- Li-ion charging and battery protection require correctly rated hardware.
- The accident detector is an educational prototype, not a certified medical
  or emergency system.
