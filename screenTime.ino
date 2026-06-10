#include <WiFi.h>
#include "Arduino_GigaDisplay_GFX.h"
#include "secrets.h"


const int timeZone = -7;



GigaDisplay_GFX display;
#define BLACK 0x0000

int status = WL_IDLE_STATUS;
IPAddress timeServer(162, 159, 200, 123); //pool.ntp.org
byte packetBuffer[48];
WiFiUDP udp;

void setup() {
  Serial.begin(9600); //TEMP
  while (!Serial);

  display.begin();
  display.setRotation(1);
  display.fillScreen(BLACK);
  display.setCursor(10, 10); 
  display.setTextSize(5);

  if (WiFi.status() == WL_NO_MODULE) {
    display.print("No MODULE");
    while (true);
  }

  while (status != WL_CONNECTED) {
    display.fillScreen(BLACK);
    display.setCursor(10, 10); 
    display.print("Connecting...");
    status = WiFi.begin(SSID, PASS);

    delay(3000);
  }

  udp.begin(2390); //2390 is the local listening port
}

void loop() {
  send();
  delay(1000);

  if (udp.parsePacket()) {
    udp.read(packetBuffer, 48);

    unsigned long high = word(packetBuffer[40], packetBuffer[41]);
    unsigned long low = word(packetBuffer[42], packetBuffer[43]);

    unsigned long seconds1900 = high << 16 | low;
    convertTimeToUTC(seconds1900);
  }

  delay(1000);
}


unsigned long send() {
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
}

void convertTimeToUTC(unsigned long seconds1900) {
  const unsigned long seventyYears = 2208988800UL;
  unsigned long epoch = seconds1900 - seventyYears;

  const int hour = ((epoch  % 86400L) / 3600) + timeZone;
  const int minute = (epoch  % 3600) / 60;
  const int seconds = epoch % 60;
  String hourString = "";
  String ampm = "";
  String minStrting = "";
  String secStrting = "";

  display.fillScreen(BLACK);
  display.setCursor(10, 10);
  display.setTextSize(10); 
  
  if (hour < 0) {
    hourString = String(hour + 12);
    ampm = "PM";
  } else {
    hourString = String(hour);
    ampm = "AM";
  }

  if (minute < 10) {
    minStrting = "0" + String(minute);
  } else {
    minStrting = String(minute);
  }

  if (seconds < 10) {
    secStrting = "0" + String(seconds);
  } else {
    secStrting = String(seconds);
  }

  display.print(hourString + ":" + minStrting + ":" + secStrting + " " + ampm);
  Serial.println(hourString + ":" + minStrting + ":" + secStrting + " " + ampm);
}