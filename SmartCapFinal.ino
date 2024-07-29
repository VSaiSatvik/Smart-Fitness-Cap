#include "Wire.h"
#include <MPU6050_light.h>

// Define pins
#define MPU_UPDATE_INTERVAL 10 // Time interval for updating MPU data
const int ldrPin = A0;  // Define the pin where the LDR is connected
const int buttonPin = 2;  // the number of the pushbutton pin
int ldrValue = 0;       // Variable to store the LDR value
int buttonState = 0;    // Variable for reading the pushbutton status
int prevButtonState = 0; // Variable to save previous button state
unsigned long lastDrinkTime = 0; // Time of the last button press

int ThermistorPin = A1;

// Known resistor value
float R1 = 2200;

// Steinhart-Hart coefficients for your thermistor
float c1 = 0.0009627972766;
float c2 = 0.0001951519226;
float c3 = 0.00000009765144211;

MPU6050 mpu(Wire);
unsigned long mpuTimer = 0; // Declare mpuTimer variable
unsigned long hourTimer = 0; // Timer to track hourly interval
unsigned long tempDisplayTimer = 0; // Timer to display temperature every 2 minutes

// Variables for step counting
int stepCount = 0;
bool stepDetected = false;
float stepThreshold = 1.0;  // Lower threshold for more sensitivity
float lowerThreshold = 0.5; // Lower threshold to detect step end

// Variables for fall detection
float coordChangeThreshold = 1.5;  // Threshold for detecting a fall based on coordinate changes
bool fallDetected = false;
bool capWorn = false; // To check if cap is being worn

// Variables to store previous acceleration values for fall detection
float prevAx = 0;
float prevAy = 0;
float prevAz = 0;

// Variables for break reminder
bool breakReminderDisplayed = false;

// IR sensor and buzzer setup
int irSensorPin = 8;
int buzzerPin = 9;
int irSensorPin2 = 3; // IR sensor for cap detection
bool obstacleDetected = false;  // Track the obstacle state

// Variable to store the previous SPF value
int previousSPF = -1;

// Variables to store previous temperature values
float prevTempC = -1000.0;
float prevTempF = -1000.0;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Initialize MPU6050
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while(status != 0) { // stop everything if could not connect to MPU6050
    Serial.println(F("Could not connect to MPU6050, retrying..."));
    delay(1000);
    status = mpu.begin();
  }

  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu.calcOffsets(); // gyro and accelero
  Serial.println("Done!\n");

  // Initialize IR sensor and buzzer
  pinMode(irSensorPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(irSensorPin2, INPUT); // Initialize the IR sensor for cap detection
  pinMode(ldrPin, INPUT); 

  // Initialize pushbutton pin as input
  pinMode(buttonPin, INPUT);

  // Initialize hour timer
  hourTimer = millis();
  lastDrinkTime = millis(); // Initialize last drink time
  tempDisplayTimer = millis(); // Initialize temperature display timer
}

void loop() {
  unsigned long currentMillis = millis();

  // Check if cap is being worn
  int sensorValue2 = digitalRead(irSensorPin2); // Read the value from the IR sensor for cap detection
  if (sensorValue2 == LOW) { // IR sensor outputs LOW when the cap is being worn
    if (!capWorn) {
      Serial.println("Cap is being worn");
      capWorn = true; // Set the flag to true
    }
  } else {
    capWorn = false; // Set the flag to false if the cap is not being worn
  }

  // Update MPU6050 data
  if (currentMillis - mpuTimer > MPU_UPDATE_INTERVAL) {
    mpu.update();

    float ax = mpu.getAccX();
    float ay = mpu.getAccY();
    float az = mpu.getAccZ();

    // Calculate magnitude of the acceleration vector
    float magnitude = sqrt(ax * ax + ay * ay + az * az);

    // Step detection based on peak detection algorithm
    if (magnitude > stepThreshold && !stepDetected) {
      stepDetected = true;
      stepCount++;
      Serial.print("Steps: ");
      Serial.println(stepCount);
    } else if (magnitude < lowerThreshold && stepDetected) {
      stepDetected = false;
    }

    // Fall detection based on coordinate changes
    if (capWorn && (abs(ax - prevAx) > coordChangeThreshold || abs(ay - prevAy) > coordChangeThreshold || abs(az - prevAz) > coordChangeThreshold)) {
      if (!fallDetected) {
        fallDetected = true;
        Serial.println("Fall detected!");
      }
    } else {
      fallDetected = false;
    }

    // Update previous acceleration values
    prevAx = ax;
    prevAy = ay;
    prevAz = az;

    mpuTimer = currentMillis;
  }

  // Update IR sensor and buzzer
  int sensorValue = digitalRead(irSensorPin);  // Read the value from the IR sensor
  if (sensorValue == LOW) {  // IR sensor outputs LOW when an obstacle is detected
    digitalWrite(buzzerPin, HIGH);  // Turn the buzzer on
    if (!obstacleDetected) {
      Serial.println("Obstacle detected!");  // Print message to serial monitor only once
      obstacleDetected = true;  // Set obstacle detected flag
    }
  } else {
    digitalWrite(buzzerPin, LOW);  // Turn the buzzer off
    obstacleDetected = false;  // Reset obstacle detected flag
  }
  
  ldrValue = analogRead(ldrPin); // Read the value from the LDR
  float lightIntensity = (ldrValue * 100.0) / 1023.0; // Convert to percentage

  int uvi = estimateUVI(lightIntensity);
  int spf = recommendSPF(uvi);

  // Only display light intensity and SPF recommendation if the SPF value changes
  if (spf != previousSPF) {
    Serial.print("Light Intensity: ");
    Serial.print(lightIntensity);
    Serial.print("% - Estimated UVI: ");
    Serial.print(uvi);
    Serial.print(" - Recommended SPF: ");
    Serial.println(spf);
    previousSPF = spf; // Update the previous SPF value
  }

  // NTC Thermistor Reading
  int Vo = analogRead(ThermistorPin);
  float R2 = R1 * (1023.0 / Vo - 1.0);
  float logR2 = log(R2);
  float T = 1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2);
  float Tc = T - 273.15;               // Convert Kelvin to Celsius
  float Tf = (Tc * 9.0) / 5.0 + 32.0;  // Convert Celsius to Fahrenheit

  // Print temperature only if it changes or every 2 minutes
  if (Tc != prevTempC || Tf != prevTempF || (currentMillis - tempDisplayTimer >= 120000)) { // 120000 ms = 2 minutes
    Serial.print("Temperature: ");
    Serial.print(Tf);
    Serial.print(" F; ");
    Serial.print(Tc);
    Serial.println(" C");
    prevTempC = Tc;
    prevTempF = Tf;
    tempDisplayTimer = currentMillis; // Reset temperature display timer
  }

  if (Tc >= 30) {
    Serial.println("You are getting a little hot, here is your current temperature: ");
    Serial.println(Tc);
  }

  // Read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  // Check if the pushbutton is pressed
  if (prevButtonState != buttonState) {
    if (buttonState == HIGH) {
      lastDrinkTime = currentMillis; // Update the last drink time
      Serial.println("Button is being pressed");
    }
    prevButtonState = buttonState; // Save the current state as the previous state for the next loop iteration
  }

  // Calculate water intake requirement every hour based on last drink time
  if (currentMillis - lastDrinkTime >= 3600000) { // 3600000 ms = 1 hour since last drink
    float baseWaterRequirement = 3000.0 / 24; // Divide base requirement by 24 for hourly requirement
    float waterPerStep = 0.2; // ml per step
    float additionalWaterFromSteps = stepCount * waterPerStep;
    float temperatureThreshold = 37.0; // degrees Celsius
    float temperatureAdjustmentFactor = 0.005; // 0.5% of base requirement per degree above threshold
    float temperatureAdjustment = 0.0;

    if (Tc >= temperatureThreshold) {
      temperatureAdjustment = baseWaterRequirement * temperatureAdjustmentFactor * (Tc - temperatureThreshold);
    }

    float totalHourlyWaterRequirement = baseWaterRequirement + additionalWaterFromSteps + temperatureAdjustment;

    // Remind to drink water if the current intake is less than the required amount
    Serial.print("Please drink water. Recommended intake since last hour: ");
    Serial.print(totalHourlyWaterRequirement);
    Serial.println(" ml");

    // Reset step count
    stepCount = 0;
    lastDrinkTime = currentMillis; // Reset the last drink time
  }

  delay(1000);
}

int estimateUVI(float lightIntensity) {
  if (lightIntensity < 20.0) return 2;
  else if (lightIntensity < 40.0) return 4;
  else if (lightIntensity < 60.0) return 6;
  else if (lightIntensity < 80.0) return 8;
  else return 11;
}

int recommendSPF(int uvi) {
  if (uvi <= 2) return 15;
  else if (uvi <= 4) return 30;
  else if (uvi <= 6) return 50;
  else return 50;
}
