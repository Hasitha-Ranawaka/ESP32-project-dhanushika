#define BLYNK_TEMPLATE_ID "TMPLkjSJoS1Z"
#define BLYNK_DEVICE_NAME "Water Quality Monitoring"
#define BLYNK_AUTH_TOKEN "JSU76u_ynXfFPHlkGUuQv0lTY2NozwvP"
#define BLYNK_PRINT Serial

#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "HDR Fiber";
char pass[] = "Hdr987654321";

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DS18B20PIN 26
#define turbidity 36
#define echoPin 14
#define trigPin 27
#define SENSOR  13
#define potPin 34
#define relay1 19
#define relay2 18

OneWire oneWire(DS18B20PIN);
DallasTemperature sensor(&oneWire);

long duration;
int distance, tank, valur_t, button, sta1;
float ph;
float Value = 0;

float volt;
float ntu;

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned long flowMilliLitres;
unsigned int totalMilliLitres;
float flowLitres;
float totalLitres;
int sensorValue = 0;

void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

void setup() {

  Serial.begin(9600);
  Serial2.begin(9600);
  pinMode(relay1, OUTPUT);
  digitalWrite(relay1,LOW);
  Blynk.begin(auth, ssid, pass);
  Wire.begin();
  sensor.begin();
  pinMode(potPin, INPUT);
  pinMode(turbidity, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(SENSOR, INPUT_PULLUP);
  
  pinMode(relay2, OUTPUT);
  //  digitalWrite(relay1,HIGH);
  //  

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(30, 30);
  display.write("Welcome TO");
  display.setCursor(40, 40);
  display.write("System");
  display.display();
  delay(2000);

  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;
  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
}

void loop() {
  Blynk.run();
  read_18b20();
  turbidityData();
  sro4();
  displayFun();
  flowRead();
  phRead();
  smsPro();
}
void read_18b20() {
  sensor.requestTemperatures();
  Serial.print("Temperature is: ");
  Serial.println(sensor.getTempCByIndex(0));
  Blynk.virtualWrite(V0, sensor.getTempCByIndex(0));
  delay(1000);
}
void turbidityData() {
  valur_t = analogRead(turbidity);
  Serial.print("Turbidity= ");
  valur_t = map(valur_t, 0, 4095, 0, 100);
  Serial.print(valur_t);
  Serial.println(" %");
  Blynk.virtualWrite(V1, valur_t);
}

void sro4() {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  tank = map(distance, 0, 30, 100, 0);
  Serial.print("Level: ");
  Serial.println(tank);
  Blynk.virtualWrite(V6, tank);

  if (distance >15 && sta1 == 0) {
    sta1 = 1;
    Serial.println("Tank Level LOW");
    digitalWrite(relay1, HIGH);
  } else if (distance < 5 && sta1 == 1) {
    sta1 = 0;
    Serial.println("Tank Level HIGH");
    digitalWrite(relay1, LOW);
  }
}
void displayFun() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(30, 7);
  display.write("Welcome TO");
  display.setCursor(40, 20);
  display.write("System");
  display.setCursor(0, 30);
  display.print("Temperature : ");
  display.println(sensor.getTempCByIndex(0));
  display.setCursor(0, 40);
  display.print("Turbidity : ");
  display.print(valur_t);
  display.println(" %");
  display.setCursor(0, 50);
  display.print("Level : ");
  display.print(tank);
  display.println(" %");
  display.display();
}
void flowRead() {

  currentMillis = millis();
  if (currentMillis - previousMillis > interval) {

    pulse1Sec = pulseCount;
    pulseCount = 0;
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();

    flowMilliLitres = (flowRate / 60) * 1000;
    flowLitres = (flowRate / 60);

    totalMilliLitres += flowMilliLitres;
    totalLitres += flowLitres;

    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalLitres);
    Serial.println("L");
    Blynk.virtualWrite(V3, totalLitres);
    Blynk.virtualWrite(V4, totalLitres * 30);
  }
}
void updateSerial() {
  delay(500);
  while (Serial.available()) {
    Serial2.write(Serial.read());
  }
  while (Serial2.available()) {
    Serial.write(Serial2.read());
  }
}
void smsPro() {
  if (valur_t < 40) {
    Blynk.logEvent("notification", "Alert : Water Quality issue");
    Serial.println("SMS OK++++++++++++++++++++++++++++++++++++++++++++++++++");
    Serial2.println("AT+CMGF=1");
    updateSerial();
    Serial2.println("AT+CMGS=\"+94702165377\"");
    updateSerial();
    Serial2.print("Water Quality issue Please Check");
    updateSerial();
    Serial2.write(26);
  }
}
void phRead() {
  Value = analogRead(potPin);
  Serial.print(Value);
  Serial.print(" | ");
  float voltage = Value * (3.3 / 4095.0);
  ph = (3.3 * voltage);
  Serial.println(ph);
  Blynk.virtualWrite(V2, ph);
  if (ph > 8) {
    Blynk.logEvent("notification", "Alert : Water Quality issue");
    Serial.println("SMS OK++++++++++++++++++++++++++++++++++++++++++++++++++");
    Serial2.println("AT+CMGF=1");
    updateSerial();
    Serial2.println("AT+CMGS=\"+94702165377\"");
    updateSerial();
    Serial2.print("Water Quality issue Please Check");
    updateSerial();
    Serial2.write(26);
  }

}
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V5);
}
BLYNK_WRITE(V5) {
  button = param.asInt();
  Serial.println(button);
  if (button == 1) {
    digitalWrite(relay2, HIGH);
  } else {
    digitalWrite(relay2, LOW);
  }

}
