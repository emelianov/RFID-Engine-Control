# RFID-Engine-Control

### Thirdparty libraries used
* [Adafruit_SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
* [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
* [MFRC522](https://github.com/miguelbalboa/rfid)
* [EEPROM2.h](http://freeduino.ru/arduino/sample_EEPROM.html) [Direct link to source code](http://www.freeduino.ru/arduino/files/EEPROM2.zip)
### Hardware
* [Relays shield](http://m.intl.taobao.com/detail/detail.html?spm=0.0.0.0&id=521493640182)
Relay pins: 10, 11, 12, 13. That pinout is incompatible with SPI bus. So You have to turn back or cutout these pins at Relay Shield and reconnect it to other Arduino pin by wire.
* RC522 reader
* I2C 128x64 display
* Arduino UNO board
### Pinout used
PIN	| Description
-------|---------------
A0	| Relays L0
A1     | Relays L1
A2     | Pin1 RFID card add
A3     | Pin2 Shutdown force
A4     | OLED SDA
A5     | OLED SDL
10	| RFID SS
11	| RFID MSIO
12	| RFID MISO
13	| RFID CLK

Pin1 and Pin2 inputs configured so that conniction they to ground initiates requsted action.

![Board](https://github.com/emelianov/RFID-Engine-Control/blob/master/docs/wiring.png)

### Adding cards
At start of controller You can add new card. Controller enters adding new card adding mode if first slot is empty or Pin1 connected to ground.
After presenting and storing one new card controller continue normal startup procedure. To erase stored cards use 'diag' firmware.

### Startup procedure
At startup controller performs hardware initialization and engine shutdown (for safety reasons).
If no stored cards or Pin1 connected to ground controller enters adding new card mode.

### Watchdog timer
For case if something goes wrong with firmware 8 seconds watchdog timer is set. If firmware code hangs controller will be restarted and perform normal startup procedure.	