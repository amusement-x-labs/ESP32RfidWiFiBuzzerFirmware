# ESP32RfidWiFiBuzzerFirmware
Firmware for ESP32 implementing a break management system using RFID. Tracks user presence (In/Out) via RFID-RC522 module, plays distinctive melodies on a buzzer for check-in/out events, and sends status updates (State) to a configured web server (HTTP POST) to enforce break reminders based on session duration.

External links:
* pithes.h is taken from [here](https://esp32io.com/tutorials/esp32-piezo-buzzer)
* RFID library for ESP32 MFRC522v2  can be installed via Library manager


Pins connection:
| ESP32 PIN | SCHEME |
| ------ | ------ |
| D4 | Resistor |
| GND | LEDs' cathodes (small one) |
| ------ | ------ |
| D16 | Buzzer plus |
| GND | Buzzer GND |
| ------ | ------ |
| 3.3V | 3.3V on RFID-RC522 |
| GND | GND on RFID-RC522 |
| D21 | RST on RFID-RC522 |
| D19 | MISO on RFID-RC522 |
| D23 | MOSI on RFID-RC522 |
| D18 | SCK on RFID-RC522 |
| D5 | SDA on RFID-RC522 |

## Related repos:
* Web Server: https://github.com/amusement-x-labs/WebServerESP32RfidStatus

## Fullfit demonstration
https://youtu.be/nuOQyAf8-Kw