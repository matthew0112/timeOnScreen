#include <WiFi.h>
#include "Arduino_GigaDisplay_GFX.h"
#include "mbed.h"
#include <mbed_mktime.h>
#include "secrets.h"


const int timeZone = -7;


GigaDisplay_GFX display;
#define BLACK 0x0000

int status = WL_IDLE_STATUS;
IPAddress timeServer(162, 159, 200, 123); //pool.ntp.org
byte packetBuffer[48];
WiFiUDP udp;

int counter = 0;

void setup() {
  Serial.begin(9600); //TEMP
  while (!Serial);

  display.begin();
  display.setRotation(1);
  display.fillScreen(BLACK);
  display.setCursor(200, 220); 
  display.setTextSize(5);

  if (WiFi.status() == WL_NO_MODULE) {
    display.print("No MODULE");
    while (true);
  }

  while (status != WL_CONNECTED) {
    display.fillScreen(BLACK);
    display.setCursor(200, 220); 
    display.print("Connecting...");
    status = WiFi.begin(SSID, PASS);

    delay(3000);
  }

  udp.begin(2390); //2390 is the local listening port
  updateTime();
}

void loop() {
  tm t;

  _rtc_localtime(time(NULL), &t, RTC_4_YEAR_LEAP_YEAR_SUPPORT);

  display.fillScreen(BLACK);
  display.setCursor(130, 170);
  display.setTextSize(10); 

  char buffer[32];
  strftime(buffer, 32, "%k:%M:%S", &t);
  display.print(buffer);

  counter += 1;
  if (counter % 60) {
    //getWeather
  }
  if (counter == 3600) {
    counter = 0;
    updateTime();
  }
  delay(1000);
}

void updateTime() {
  Serial.println("UPDATING TIME");
  memset(packetBuffer, 0, 48);

  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  
  udp.beginPacket(timeServer, 123);
  udp.write(packetBuffer, 48);
  udp.endPacket();

  delay(1000);

  if (udp.parsePacket()) {
    udp.read(packetBuffer, 48);

    unsigned long high = word(packetBuffer[40], packetBuffer[41]);
    unsigned long low = word(packetBuffer[42], packetBuffer[43]);

    unsigned long seconds1900 = high << 16 | low;
    const unsigned long seventyYears = 2208988800UL;
    unsigned long epoch = seconds1900 - seventyYears;

    set_time(epoch + (timeZone * 3600) + 3);
  }
}