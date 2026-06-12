# Digital Clock on Arduino GIGA R1 Display

## Features

- Digital Clock
- Time from the [Netwok Time Protocol](https://en.wikipedia.org/wiki/Network_Time_Protocol)
- 12 or 24 Hour Time
- Current Date
- Home Assistant Sensor (Optional)
- Light and Dark Mode

## Time Setup
1. Enter WiFi Secrets into [secrets.h](secrets.h)
2. Change Time Zone and Millitary time settings in [screenTime.ino](screenTime.ino)
3. Choose light or dark mode also in [screenTime.ino](screenTime.ino)
4. Optionally, Setup Home Assistant Sensor
5. Upload!

## Home Assistant Setup
1. Log into your Home Assistant Dashboard
2. Open the menu on the left side
3. Click on your profile at the very bottom
4. Go to the security tab
5. Scroll all the way to the bottom until you see "Long-lived access tokens"
6. Hit the blue "Create Token" Button
7. Enter a name, which can be what ever you want
8. Click the blue "Create Token" Button
9. Copy the access token into [secrets.h](secrets.h)
10. In [screenTime.ino](screenTime.ino) enter your Home Assistant server IP address
11. Enter the sensor ID you wish to display (I like the temperature)