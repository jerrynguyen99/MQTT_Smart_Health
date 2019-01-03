#include <ESP8266WiFi.h>
#include "MAX30100_PulseOximeter.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const char* ssid     = "International University";
const char* password = "";

// Declaration for NTP Server
#define NTP_OFFSET  25200 // Timezone, in second 
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "1.asia.pool.ntp.org"
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

// Declaration for SSD1306 display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_MOSI   D7
#define OLED_CLK    D4
#define OLED_RESET D6
#define OLED_CS D5
#define OLED_DC D3
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK,
                         OLED_DC, OLED_RESET, OLED_CS);

// Declaration for BUTTON
#define BUTTON_PIN  D0
int buttonState = 0;

// Declaration for MAX30100
#define REPORTING_PERIOD_MS     1000
#define SCREEN_UPDATED_PERIOD_MS 500
PulseOximeter sensor;

//Callback method
void onBeatDetected() {
  Serial.println("Beat!");
}

//Setup for the first run
void setup() {
  //Serial setup
  Serial.begin(9600);

  //Display setup
  if (!display.begin(SSD1306_SWITCHCAPVCC))
  {
    Serial.println("Initialize failed");
    for (;;); // Don't proceed, loop forever
  }
  else
  {
    Serial.println("DISPLAY SUCCESS");
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.startscrollright(0x00, 0x0F);
  display.setTextSize(3);
  display.setCursor(0, 0);
  display.println("SMART");
  display.println("HEALTH");
  display.display();
  delay(2000);
  //Sensor setup
  if (!sensor.begin())
  {
    Serial.println("Initialize failed");
    for (;;); // Don't proceed, loop forever
  }
  else
  {
    Serial.println("SENSOR SUCCESS");
  }
  sensor.setOnBeatDetectedCallback(onBeatDetected);
  //WiFi setup
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("WiFi: ");
  display.println(ssid);
  display.print("STATUS: ");
  display.display();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    display.print(".");
    display.display();
  }
  display.println();
  display.println("WiFi connected.");
  display.println("IP address: ");
  display.println(WiFi.localIP());
  display.display();
  delay(2000);

  //RealTime setup
  timeClient.begin();

  //pinMode setup
  pinMode(BUTTON_PIN, INPUT);
}

int state = 0;
int lastButtonState = 0;
uint32_t tsLastReport = 0;
uint32_t tsLastScreen = 0;

void loop() {
  buttonState = digitalRead(BUTTON_PIN);
  if (buttonState != lastButtonState && buttonState == 1)
  {
    state = !state;
  }
  lastButtonState = buttonState;  
  if (state == 0) {
    if (millis() - tsLastScreen > SCREEN_UPDATED_PERIOD_MS) {
      tsLastScreen = millis();
      timeClient.update();
      String formattedTime = timeClient.getFormattedTime();
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(15, 10);
      display.print(formattedTime);
      display.display();
      Serial.println(formattedTime);
    }
     sensor.shutdown();
  }
  else
  {
    if (millis() - tsLastScreen > SCREEN_UPDATED_PERIOD_MS) {
      display.clearDisplay();
      int heartRate = sensor.getHeartRate();
      int spo2 = sensor.getSpO2();
      if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        sensor.resume();
        sensor.update();
        display.setCursor(0, 30);
        display.print(heartRate);
        display.print("bpm");
        display.setCursor(0, 50);
        display.print("O2: ");
        display.print(spo2);
        display.print(" %");
        display.display();
        tsLastReport = millis();
        Serial.print("Heart rate:");
        Serial.print(heartRate);
        Serial.print("bpm / SpO2:");
        Serial.print(spo2);
        Serial.println("%");
      }
      tsLastScreen = millis();
    }
  }

}
