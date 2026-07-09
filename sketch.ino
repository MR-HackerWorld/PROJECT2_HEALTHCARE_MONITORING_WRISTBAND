#include <Wire.h>
#include <MPU6050.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SDA_PIN 21
#define SCL_PIN 22

#define ONE_WIRE_BUS 4
#define BUZZER 15
#define RED 12
#define GREEN 13
#define BLUE 14
#define BUTTON 27
#define BATTERY_PIN 34

MPU6050 mpu;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
Adafruit_SSD1306 display(128, 64, &Wire, -1);

unsigned long previousMillis = 0;
unsigned long fallTimer = 0;

bool impactDetected = false;
bool monitoringInactivity = false;

float simulatedBattery = 4.2;   // Start full

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  mpu.initialize();
  sensors.begin();

  pinMode(BUZZER, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(BUTTON, INPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  setGreen();
}

void loop() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 500) {
    previousMillis = currentMillis;

    // ---- TEMPERATURE ----
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);

    // ---- MPU READ ----
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);

    float Ax = ax / 16384.0;
    float Ay = ay / 16384.0;
    float Az = az / 16384.0;

    float A = sqrt(Ax*Ax + Ay*Ay + Az*Az);
    float angle = atan2(Ay, Az) * 180 / PI;

    // ---- FALL DETECTION ----
    if (A < 0.5 && !impactDetected) {
      fallTimer = currentMillis;
      impactDetected = true;
    }

    if (impactDetected && (currentMillis - fallTimer < 500)) {
      if (A > 2.5) {
        monitoringInactivity = true;
        fallTimer = currentMillis;
      }
    }

    if (monitoringInactivity) {
      if (abs(angle) > 60) {
        if (currentMillis - fallTimer > 8000) {
          triggerFall();
          monitoringInactivity = false;
          impactDetected = false;
        }
      }
    }

    // ---- SIMULATED MAX30102 ----
    int heartRate = 72 + 5 * sin(currentMillis / 800.0);
    int spo2 = 97 + 2 * sin(currentMillis / 1200.0);

    // ---- BATTERY DECAY SIMULATION ----
    simulatedBattery -= 0.00005;   // slow discharge

    if (simulatedBattery < 3.0)
      simulatedBattery = 4.2;   // auto recharge loop for demo

    if (simulatedBattery < 3.3) {
      digitalWrite(BUZZER, HIGH);
      setRed();
    }

    // ---- OLED DASHBOARD ----
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("HR: ");
    display.print(heartRate);

    display.setCursor(0,15);
    display.print("SpO2: ");
    display.print(spo2);

    display.setCursor(0,30);
    display.print("Temp: ");
    display.print(tempC);

    display.setCursor(0,45);
    display.print("Bat: ");
    display.print(simulatedBattery);

    display.display();

    Serial.print("HR:");
    Serial.print(heartRate);
    Serial.print(" SpO2:");
    Serial.print(spo2);
    Serial.print(" Bat:");
    Serial.println(simulatedBattery);
  }

  if (digitalRead(BUTTON) == HIGH) {
    triggerFall();
  }
}

void triggerFall() {
  Serial.println("CONFIRMED FALL DETECTED");
  digitalWrite(BUZZER, HIGH);
  setRed();
  delay(2000);
  digitalWrite(BUZZER, LOW);
  setGreen();
}

void setGreen() {
  digitalWrite(RED, LOW);
  digitalWrite(GREEN, HIGH);
  digitalWrite(BLUE, LOW);
}

void setRed() {
  digitalWrite(RED, HIGH);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);
}