#include "Wire.h"
#include <MPU6050_light.h>
#include "DHT.h"

// Define pins
#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
#define MPU_UPDATE_INTERVAL 10 // Time interval for updating MPU data

// MPU6050 setup
MPU6050 mpu(Wire);
unsigned long mpuTimer = 0;

// Variables for step counting
int stepCount = 0;
bool stepDetected = false;
float stepThreshold = 1.0;  // Lower the threshold for more sensitivity
float lowerThreshold = 0.5; // Lower threshold to detect step end

// DHT11 setup
DHT dht(DHTPIN, DHTTYPE);

// Variables to store previous values for comparison
float prevHumidity = -1.0;
float prevTemperature = -1.0;
float prevHeatIndexC = -1.0;
float prevHeatIndexF = -1.0;
int prevStepCount = -1;  // Track previous step count

// Timing for DHT sensor
unsigned long dhtTimer = 0;
#define DHT_UPDATE_INTERVAL 2000 // Time interval for updating DHT data

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

  // Initialize DHT11
  dht.begin();
}

void loop() {
  unsigned long currentMillis = millis();
  
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

    mpuTimer = currentMillis;
  }

  // Update DHT11 sensor data
  if (currentMillis - dhtTimer > DHT_UPDATE_INTERVAL) {
    float h = dht.readHumidity();
    float t = dht.readTemperature(); // Read temperature as Celsius
    float f = dht.readTemperature(true); // Read temperature as Fahrenheit

    // Check if any reads failed and exit early (to try again)
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println(F("Failed to read from DHT sensor!"));
    } else {
      // Compute heat index
      float hif = dht.computeHeatIndex(f, h);
      float hic = dht.computeHeatIndex(t, h, false);

      // Only display values if they have changed
      if (h != prevHumidity || t != prevTemperature || hic != prevHeatIndexC || hif != prevHeatIndexF) {
        Serial.print(F("Humidity: "));
        Serial.print(h);
        Serial.print(F("%  Temperature: "));
        Serial.print(t);
        Serial.print(F("°C "));
        Serial.print(f);
        Serial.print(F("°F  Heat index: "));
        Serial.print(hic);
        Serial.print(F("°C "));
        Serial.print(hif);
        Serial.println(F("°F"));

        // Update previous values
        prevHumidity = h;
        prevTemperature = t;
        prevHeatIndexC = hic;
        prevHeatIndexF = hif;
      }
    }
    dhtTimer = currentMillis;
  }
}
