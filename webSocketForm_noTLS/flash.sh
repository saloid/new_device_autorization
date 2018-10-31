#!/bin/sh

/home/qitment/.arduino15/packages/esp8266/tools/esptool/0.4.12/esptool -vv -cd nodemcu -cb 921600 -cp /dev/ttyUSB0 -ca 0x0 -cz 0x100000 -ca 0x00000 -cf webSocketForm_noTLS.ino.generic.bin