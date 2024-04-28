#include <LiquidCrystal.h>
#include <SD.h>

const int trigPin = 9;
const int echoPin = 10;
const int speedSensorPin = 2;

LiquidCrystal lcd(6, 7, 5, 4, 3, 2);

float distance = 0;
float speed = 0;

unsigned long previousMillisA = 0;
unsigned long previousMillisB = 0;
const long interval = 1000;

const int SAMPLE_SIZE = 10;
unsigned long crossingTimes[SAMPLE_SIZE];
int crossingIndex = 0;
unsigned long averageCrossingTime = 0;

File dataFile;
String vehicleNumberPlate;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(speedSensorPin, INPUT_PULLUP);
  
  if (!SD.begin(10)) {
    Serial.println("SD Card initialization failed!");
    return;
  }
  
  dataFile = SD.open("data.csv", FILE_WRITE);
  if (!dataFile) {
    Serial.println("Error opening data file!");
    return;
  }

  lcd.clear();
  lcd.print("Traffic Scanner");
  lcd.setCursor(0, 1);
  lcd.print("System Ready");
  delay(2000);
}

void loop() {
  scanNumberPlate();
  distance = measureDistance();
  speed = measureSpeed();
  writeCrossingTimes(distance);
  calculateTrafficStatus();
  displayData(distance, speed, vehicleNumberPlate);
}

void scanNumberPlate() {
  vehicleNumberPlate = "ABCD1234";
}

float measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  float duration = pulseIn(echoPin, HIGH);
  float distance_cm = (duration * 0.034) / 2;
  return distance_cm;
}

float measureSpeed() {
  unsigned long currentMillis = millis();
  unsigned long elapsedTime = currentMillis - previousMillisA;

  if (digitalRead(speedSensorPin) == LOW && elapsedTime >= interval) {
    speed = distance / ((float)elapsedTime / 1000) * 3.6;
    previousMillisA = currentMillis;
  }

  return speed;
}

void writeCrossingTimes(float distance) {
  unsigned long currentMillis = millis();
  
  if (distance < 50) {
    dataFile.print("Point A,");
    dataFile.println(currentMillis);
    crossingTimes[crossingIndex] = currentMillis;
    crossingIndex = (crossingIndex + 1) % SAMPLE_SIZE;
    previousMillisA = currentMillis;
  }
  
  if (distance < 50 && previousMillisA != 0) {
    dataFile.print("Point B,");
    dataFile.println(currentMillis);
    previousMillisB = currentMillis;
  }
}

void calculateTrafficStatus() {
  unsigned long sum = 0;
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    sum += crossingTimes[i];
  }
  averageCrossingTime = sum / SAMPLE_SIZE;

  if (previousMillisA == 0 || previousMillisB == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Waiting for data");
  } else if (previousMillisA < averageCrossingTime && previousMillisB < averageCrossingTime) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("High Traffic");
  } else if (previousMillisA > averageCrossingTime && previousMillisB > averageCrossingTime) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Low Traffic");
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Medium Traffic");
  }
}

void displayData(float distance, float speed, String vehicleNumberPlate) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Distance: ");
  lcd.print(distance);
  lcd.print(" cm");

  lcd.setCursor(0, 1);
  lcd.print("Speed: ");
  lcd.print(speed);
  lcd.print(" km/h");

  lcd.setCursor(0, 2);
  lcd.print("Plate: ");
  lcd.print(vehicleNumberPlate);
}
