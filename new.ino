#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include "I2Cdev.h"
#include "MPU6050.h"

MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;

static const int RXPin = 7, TXPin = 8;
static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps;

SoftwareSerial ss(RXPin, TXPin);
SoftwareSerial SIM900(2, 3);

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  pinMode(13, OUTPUT); // For getting a sound from the buzzer
  pinMode(12, INPUT_PULLUP);
  
  Serial.begin(19200);
  ss.begin(GPSBaud);
  SIM900.begin(9600);  // Initialize SIM900 GSM module
  
  Serial.print("SIM900 ready...");
  SIM900.print("AT+CMGF=1\r");
  delay(100);
  SIM900.print("AT+CNMI=2,2,0,0,0\r");
  delay(100);

  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin();
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
  #endif

  Serial.begin(19200);

  Serial.println("Initializing I2C devices...");
  accelgyro.initialize();

  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  lcd.begin();
  lcd.backlight();
}

void loop() {
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  Serial.println(ax);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(ax);
  delay(100);
  
  if (ax > 5000 || ax < -5000) {
    sendsms();
    makeCall();  // Call the function to make a call
  }
}

void sendsms() {
  if (ss.available() > 0) {
    gps.encode(ss.read());
    if (gps.location.isUpdated()) {
      Serial.print("GPS FIXED");
      Serial.print("Latitude= ");
      Serial.print(gps.location.lat(), 6);
      Serial.print(" Longitude= ");
      Serial.println(gps.location.lng(), 6);
    }
  }
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Crash Detected!");
  lcd.setCursor(0, 1);
  lcd.print("Sending SMS...");

  // Replace with the desired phone number
  SIM900.println("AT+CMGS=\"+919350599238\"");
  delay(100);
  Serial.println("Sending SMS");
  SIM900.println("A CRASH of the vehicle has been detected. Please try to contact the driver.");
  delay(100);

  SIM900.print("https://www.google.com/maps/?q=");
  SIM900.print(gps.location.lat(), 6);
  SIM900.print(",");
  SIM900.print(gps.location.lng(), 6);
  delay(100);

  SIM900.write((char)26);
  delay(100);
  SIM900.println();
  delay(5000);
}

void makeCall() {
  // Replace with the desired phone number
  SIM900.print("ATD+918529286929;\r\n");
  delay(1000); // Adjust delay as needed

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Making a call...");
  
  while (SIM900.available()) {
    char c = SIM900.read();
    lcd.write(c);  // Print response from the GSM module on LCD
    Serial.write(c);  // Print response from the GSM module to Serial Monitor
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Call completed");
  delay(2000);
}
