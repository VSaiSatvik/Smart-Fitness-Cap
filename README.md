# Smart-Fitness-Cap
This repository contains code for a smart wearable device that tracks steps, detects falls, measures light intensity, UV Index, and temperature. It provides hydration reminders and uses IR sensors to detect obstacles and ensure the cap is worn. It continuously processes data and gives real-time feedback.

## Smart Wearable Health and Safety Device

This repository contains the code for a smart wearable device that monitors various health and environmental parameters to ensure user safety and well-being. The device is equipped with multiple sensors and provides real-time feedback on step counting, fall detection, ambient light intensity, temperature, and hydration reminders. Below is a detailed description of the functionalities implemented in the code:

### Features

1. **MPU6050 Sensor for Motion Detection**:
   - **Step Counting**: Detects steps based on acceleration data, displaying the step count on the serial monitor.
   - **Fall Detection**: Monitors sudden changes in acceleration to detect falls, providing immediate alerts.

2. **Light Intensity Monitoring**:
   - Uses an LDR (Light Dependent Resistor) to measure ambient light intensity.
   - Estimates UV Index (UVI) based on light intensity and recommends appropriate SPF for sun protection.

3. **Temperature Monitoring**:
   - Utilizes an NTC thermistor to measure the ambient temperature.
   - Converts and displays temperature in both Celsius and Fahrenheit.
   - Alerts the user if the temperature exceeds a predefined threshold.

4. **Hydration Reminder**:
   - Tracks the time since the last drink and steps taken to calculate the hourly water intake requirement.
   - Provides reminders to drink water based on the calculated requirement, adjusted for physical activity and temperature.

5. **IR Sensor and Buzzer**:
   - Detects obstacles using an IR sensor and activates a buzzer as a warning.
   - Monitors if the wearable cap is being worn using a second IR sensor.

### Hardware Requirements

- MPU6050 Accelerometer and Gyroscope
- Light Dependent Resistor (LDR)
- NTC Thermistor
- IR Sensors (x2)
- Buzzer
- Pushbutton
- Resistors and connecting wires
- Arduino-compatible board

### Software Requirements

- Arduino IDE
- MPU6050 and Wire libraries

### Code Overview

The code initializes and configures the sensors, then enters a loop to continuously monitor and process sensor data. Key functionalities include:

- **Setup**: Initializes serial communication, sensors, and pins.
- **Loop**: Continuously reads sensor data, processes it, and provides feedback through the serial monitor and buzzer.
- **MPU6050 Update**: Updates accelerometer data to detect steps and falls.
- **Light Intensity and UVI**: Reads LDR values, estimates UVI, and recommends SPF.
- **Temperature Monitoring**: Reads and processes thermistor data to display temperature.
- **Hydration Reminder**: Calculates water intake requirement and reminds the user to drink water.
- **Obstacle Detection**: Monitors IR sensor data to detect obstacles and activates the buzzer.

### Usage

1. **Compile and Upload**: Compile the code in the Arduino IDE and upload it to your Arduino-compatible board.
2. **Connect Sensors**: Connect the sensors and components as per the defined pins.
3. **Monitor Serial Output**: Open the serial monitor to view real-time data and alerts from the device.

### Future Enhancements

- Integration with a mobile app for remote monitoring and notifications.
- Addition of more sensors for enhanced health monitoring.
- Wireless connectivity for data synchronization and cloud storage.
