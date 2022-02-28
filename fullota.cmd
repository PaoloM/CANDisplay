:: Full OTA script for Jeeves Smart Home system
:: v1.0 20200627 Paolo Marcucci
::
:: Remember to update IP in this file and ID on the Jeeves server

:: -- CANDisplay --
pio run -e esp32 -t upload --upload-port 192.168.1.153