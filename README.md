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

Home  <ShowDefaultScreen()>
* Settings
  * Info <ShowSplashScreen()>
  * Max RPM (0...20000 step 500)
  * Brightness
    * Autodim
      * Enabled
      * Disabled
    * Day (0...255 step 10)
    * Night (0...255 step 10)
    * Settings -> Settings
  * Test
    * RPM (0...20000 step 500)
    * Strip <StripLaunch()>
    * Settings -> Settings
  * Display // this sets the info shown on the SSD1306 display
    * Engine speed
    * Vehicle speed
    * ? Turbo RPM
    * ? Turbo PSI
    * ? (other turbo params)
    * Settings -> Settings
  * Pattern // this sets the patterns for the WS2812 LED strip
    * ITA // (GREENx1/3, WHITEx1/3, REDx1/3, BLUE BLINKxALL)
    * F1 // (GREENx1/3, YELLOWx1/3, BLUEx1/3, BLUE BLINKxALL)
  * Exit -> Home

The FIAT 500 Abarth is KWP FAST CAN 29bit, its using the ISO ISO15765-4 protocol.

https://www.fiat500owners.com/threads/fiat-obd2-protocol.100978/

http://www.obdtester.com/obd2_protocols

https://www.fiat500owners.com/threads/500e-can-bus-decoding.150166/#post-1299589

