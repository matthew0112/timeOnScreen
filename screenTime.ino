#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "Arduino_GigaDisplay_GFX.h"
#include "mbed.h"
#include <mbed_mktime.h>
#include "secrets.h"

// Config
const int timeZone = 0; // Your time zone (including daylight savings). If your not sure what yours is: here's a helpful map https://www.timeanddate.com/time/map/
const bool militaryTime = false; // If you want the time to be displayed in 24 hour time. e.g. 13:05:00

const bool darkMode = true; // White text on a black background

const char* server = ""; // Home Assistant Server IP (HTTP only, not required)
const int port = 8123; // Home Assistant port (usually 8123)
const String sensorId = ""; // Home Assistant Sensor ID. e.g. "sensor.temperature"


GigaDisplay_GFX display;
#define BLACK 0x0000
#define WHITE 0xFFFF

WiFiClient client;
int status = WL_IDLE_STATUS;
IPAddress timeServer(162, 159, 200, 123); //pool.ntp.org
byte packetBuffer[48];
WiFiUDP udp;

String endpoint = ("/api/states/" + sensorId); 

int counter = 0;
String weatherString = "";

void setup() {
  display.begin();
  display.setRotation(1);

  themeFillScreen();

  if (WiFi.status() == WL_NO_MODULE) {
    printCentered("No Module", 220, 5);
    while (true);
  }

  while (status != WL_CONNECTED) {
    themeFillScreen();
    printCentered("Connecting...", 220, 5);
    status = WiFi.begin(SSID, PASS);

    delay(3000);
  }

  udp.begin(2390); //2390 is the local listening port
  updateTime();
  weatherString = getWeather();
  weatherString.replace("°", "");
}

void loop() {
  tm t;
  _rtc_localtime(time(NULL), &t, RTC_4_YEAR_LEAP_YEAR_SUPPORT);
  themeFillScreen();

  char buffer[32];
  if (militaryTime) {
    strftime(buffer, 32, "%k:%M:%S", &t);
  } else {
    strftime(buffer, 32, "%I:%M:%S %p", &t);
  }

  printCentered(buffer, 170, 10);
  strftime(buffer, 32, "%A, %B %e", &t);
  printCentered(buffer, 270, 5);
  
  counter += 1;

  if ((counter % 60 == 0)) {
    weatherString = getWeather();
    weatherString.replace("°", ""); // It can't display ° :(
  }
    
  printCentered(weatherString, 330, 5);

  if (counter == 3600) {
    counter = 0;
    updateTime();
  }
  delay(1000);
}

void updateTime() {
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

    set_time(epoch + (timeZone * 3600));
  }
}

String getWeather() {
  if (strlen(server) > 0) {
    String returnString = "";

    if (client.connect(server, port)) {
      // Send HTTP GET request line
      client.println("GET " + endpoint + " HTTP/1.1");
      client.print("Host: ");
      client.println(server);
      

      client.print("Authorization: Bearer ");
      client.println(HOMEASSISTANT);
      
      client.println("Connection: close");
      client.println(); // Blank line tells server we are done sending headers

      while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") { 
          // An empty line means headers are done! Next comes the JSON.
          break; 
        }
      }

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, client);
      if (!error) {
        String state = doc["state"];
        String unit = "";
        if (doc["attributes"]["state_class"] == "measurement") {
          unit = doc["attributes"]["unit_of_measurement"].as<String>();
        }
        
        returnString = state + " " + unit;
        
      } else {
        returnString = "N/A";
      }

      client.stop(); 
    } else {
      return "N/A";
    }
    return returnString;
  }
  return "";
}

void themeFillScreen() {
  if (darkMode) {
    display.fillScreen(BLACK);
  } else {
    display.fillScreen(WHITE);
  }
}

void printCentered(String text, int y, int size) {
  if (darkMode) {
    display.setTextColor(WHITE);
  } else {
    display.setTextColor(BLACK);
  }

  int16_t x1, y1;
  uint16_t w, h;

  display.setTextSize(size);
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  int x = (display.width() - w) / 2;

  display.setCursor(x, y);
  display.print(text);
}