# CANDisplay
A display device for CAN data.

## Capabilities

* Connect to the CAN bus and get data/power from there
* Display shift light via an array of neopixels
* Display additional data via an OLED display
* Control display data via a rotary encoder
* (backlog) On-device configuration


## Connections

* SSD1306
  * SCL -> D1
  * SDA -> D2

* KY040
  * CLK -> D5
  * DT -> D6
  * SW -> D7

* WS2812
  * DI -> D8

## UX tree

_This UX tree is from MODULAMP, for reference only_

The UX is initially set to accept volume changes.

* VOLUME
  * 0...20
* INPUT
  * STREAM
  * PHONO
  * AUX

_New UX tree for CANDISPLAY - exploration_

* RPM MAX - _in RPM, indicates the upper limit for the LED display_
* SHIFT - _in RPM, indicates the lower bound of the optimal shift range_