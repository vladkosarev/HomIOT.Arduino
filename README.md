HomIOT.Arduino
==============

Arduino sketch for HomIOT.  
Requires ESP8266 flashed with default AT firmware and connected to Arduino's hardware UART.

This sketch will connect Arduino to WiFi and start sending PIN status information to HomIOT server.

Works with [HomIOT.Server](https://github.com/vladkosarev/HomIOT.Server) and [HomIOT.Mobile](https://github.com/vladkosarev/HomIOT.Mobile).


By default pins 2 to 12 are being watched (you can change that in code).  
On first bootup server will receive statuses of every pin. After that server will receive a new status any time the ping changes.
