const int ledPin = 13;      // LED connected to digital pin 13
const int knockSensor = A0; // the piezo is connected to analog pin 0
const int threshold = 100;  // threshold value to decide when the detected sound is a knock or not

// these variables will change:
int sensorReading = 0;      // variable to store the value read from the sensor pin
int ledState = LOW;         // variable used to store the last LED status, to toggle the light

constexpr size_t NUM_READINGS = 128;
static int sensor_readings[NUM_READINGS];
static size_t sensor_reading_index = 0;

void setup() {
  for (int &s : sensor_readings) s = 0;

  pinMode(ledPin, OUTPUT); // declare the ledPin as as OUTPUT
  Serial.begin(9600);      // use the serial port
}

void loop() {
  sensor_readings[sensor_reading_index++] = analogRead(knockSensor);

  if (sensor_reading_index == NUM_READINGS) {
    int max_reading = 0;
    int sum = 0;
    
    for (int s : sensor_readings) {
      if (s > max_reading) {
        max_reading = s;
      }
      sum += s;
    }

    Serial.println(max_reading);

//    if (max_reading > 0) {
//      Serial.print(max_reading);
//      //Serial.print("\t\t\t");
//      //Serial.println(sum);
//      delay(10);
//    }
  }
}

