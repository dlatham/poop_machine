#The Poop Machine#

- - - -

The poop machine is an Arduino script written to fire a simple webservice for recording baby-related events to a database of your choosing. I currently run the service off of an IFTTT Maker trigger that records the events to a Google Drive spreadsheet.

##Setting up IFTTT##

Goto maker.iftt.com and register a new event. Insert you event name into the code along with your Maker key.


##Wifi Connectivity##

The current program uses a hard-coded SSID and password in order to connect to your network. Future iterations will include the ability to update the network configuration via web browser or mobile app.


##Hardware##

Tested with the Arduino Uno but should function on any Arduino device with hardware SPI. Using the Adafruit CC3000 Wifi breakout for connectivity including Adafruit's CC3000 Arduino library available here:

https://learn.adafruit.com/adafruit-cc3000-wifi
