git clone https://github.com/mabels/Timer.git lib/Timer
git clone https://github.com/adafruit/Adafruit_MCP9808_Library.git lib/Adafruit_MCP9808_Library
pip install -U platformio



exit 0
AVR_BASE=/Applications/Arduino.app/Contents/Java/hardware/tools/avr

LIB_BASE=/Applications/Arduino.app/Contents/Java/hardware/arduino/avr/libraries
CORE_BASE=/Applications/Arduino.app//Contents/Java/hardware/arduino/avr/cores/arduino
FLAVOUR_BASE=/Applications/Arduino.app//Contents/Java/hardware/arduino/avr/variants/leonardo
UPLOAD_PORT=/dev/cu.usbmodem1411



cp karre.ino karre.cpp

mkdir -p target

files = ""
for i in $CORE_BASE/*.c $LIB_BASE/Wire/utility/*.c
do
  $AVR_BASE/bin/avr-gcc -c -g -Os -w -ffunction-sections -fdata-sections -MMD \
  -mmcu=atmega32u4 -DF_CPU=16000000L -DARDUINO=10707 -DARDUINO_AVR_LEONARDO \
  -DARDUINO_ARCH_AVR -DUSB_VID=0x2a03 -DUSB_PID=0x8036 \
  -DUSB_MANUFACTURER="Unknown" -DUSB_PRODUCT="Arduino Leonardo" \
  -I/Applications/Arduino.app/Contents/Java/hardware/arduino/avr/cores/arduino \
  -I/Applications/Arduino.app/Contents/Java/hardware/arduino/avr/variants/leonardo \
  $i  -o target/$(basename $i).o
  files=$files" "target/$(basename $i).o

done

for i in karre.cpp Timer/*.cpp Adafruit_MCP9808_Library/*.cpp \
$LIB_BASE/Wire/*.cpp $CORE_BASE/*.cpp
do
  $AVR_BASE/bin/avr-g++ -c -g -Os -Wall -fno-exceptions -ffunction-sections \
  -fdata-sections -mmcu=atmega32u4 -DF_CPU=16000000L -MMD -DUSB_VID=0x2341 \
  -DUSB_PID=0x8036 -DARDUINO=105 -D__PROG_TYPES_COMPAT__  \
  -I$CORE_BASE \
  -I$FLAVOUR_BASE \
  -ITimer \
  -I$LIB_BASE/Wire \
  -I$LIB_BASE/Wire/utility \
  -IAdafruit_MCP9808_Library \
  $i -o target/$(basename $i).o
  files=$files" "target/$(basename $i).o
done

$AVR_BASE/bin/avr-gcc -Os -Wl,--gc-sections -mmcu=atmega32u4 -o karre.cpp.elf \
  $files


#/Applications/Arduino.app/Contents/Java/hardware/tools/avr/bin/avr-gcc -w -Os -Wl,--gc-sections -mmcu=atmega32u4 -o /var/folders/c9/2bhsxr6j7dd0ykz7cxnk0t2m0000gq/T/build4329950085712680889.tmp/sketch_oct30a.cpp.elf /var/folders/c9/2bhsxr6j7dd0ykz7cxnk0t2m0000gq/T/build4329950085712680889.tmp/sketch_oct30a.cpp.o /var/folders/c9/2bhsxr6j7dd0ykz7cxnk0t2m0000gq/T/build4329950085712680889.tmp/Timer/Event.cpp.o /var/folders/c9/2bhsxr6j7dd0ykz7cxnk0t2m0000gq/T/build4329950085712680889.tmp/Timer/Timer.cpp.o /var/folders/c9/2bhsxr6j7dd0ykz7cxnk0t2m0000gq/T/build4329950085712680889.tmp/Wire/Wire.cpp.o /var/folders/c9/2bhsxr6j7dd0ykz7cxnk0t2m0000gq/T/build4329950085712680889.tmp/Wire/utility/twi.c.o /var/folders/c9/2bhsxr6j7dd0ykz7cxnk0t2m0000gq/T/build4329950085712680889.tmp/Adafruit MCP9808 Library/Adafruit_MCP9808.cpp.o /var/folders/c9/2bhsxr6j7dd0ykz7cxnk0t2m0000gq/T/build4329950085712680889.tmp/core.a -L/var/folders/c9/2bhsxr6j7dd0ykz7cxnk0t2m0000gq/T/build4329950085712680889.tmp -lm 
$AVR_BASE/bin/avr-objcopy -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load \
  --no-change-warnings --change-section-lma .eeprom=0 \
  karre.cpp.elf karre.cpp.eep
$AVR_BASE/bin/avr-objcopy -O ihex -R .eeprom  karre.cpp.elf karre.cpp.hex


$AVR_BASE/bin/avrdude -C$AVR_BASE/etc/avrdude.conf -v -patmega32u4 -cavr109 \
  -P$UPLOAD_PORT -b57600 -D -Uflash:w:karre.cpp.hex:i 

