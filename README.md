# RFID-Engine-Control

### Thirdparty libraries used
* [Adafruit_SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
* [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
* [MFRC522](https://github.com/miguelbalboa/rfid)
* [EEPROM2.h](http://freeduino.ru/arduino/sample_EEPROM.html)
### Hardware
* [Relays shield](http://m.intl.taobao.com/detail/detail.html?spm=0.0.0.0&id=521493640182)
Relay pins: 10, 11, 12, 13. That pinout is incompatible with SPI bus. So You have to turn back or cutout these pins at Relay Shield and reconnect it to other Arduino pin by wire.
* RC522 reader
* I2C 128x64 display
* Arduino UNO board
![Board](https://github.com/emelianov/RFID-Engine-Control/blob/master/docs/wiring.png)
