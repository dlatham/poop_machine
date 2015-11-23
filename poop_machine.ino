/*************************************************** 
  The Poop Machine
  Written by Dave Latham
  Version 1.0 (Nov 2015)
  
  Program uses the the Adafruit CC3000 Wifi Breakout & Shield
  ----> https://www.adafruit.com/products/1469

 ****************************************************/
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"

const int BOOB_RIGHT = 14;
const int PEE = 15;
const int POOP = 16;
const int BOOB_LEFT = 17;
const int AWAKE = 18;
const int SLEEP = 19;
const int ledPin = 3;
int boobRightState = 0;
int peeState = 0;
int poopState = 0;
int boobLeftState = 0;
int awakeState = 0;
int sleepState = 0;
int x = 255;
String result;

// Interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   2  // MUST be an interrupt pin!
#define ADAFRUIT_CC3000_VBAT  9
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID       "INSERT_WIFI_NETWORK"           // cannot be longer than 32 characters!
#define WLAN_PASS       "INSERT WIFI PASSWORD"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If server is quick to
                                   // respond, you can reduce this value.

// IFTTT Maker channel info
#define WEBSITE      "maker.ifttt.com"
#define WEBPAGE      "/trigger/INSERT_MAKER_CHANNEL/with/key/INSERT_KEY"


/**************************************************************************/
/*!
    @brief  Sets up the HW and the CC3000 module (called automatically
            on startup)
*/
/**************************************************************************/

uint32_t ip;

void setup(void)
{
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  analogWrite(ledPin, 255);

  Serial.begin(115200);
  Serial.println(F("Hello, Poop Machine!\n")); 

  uint8_t major, minor;
  
  if(!cc3000.getFirmwareVersion(&major, &minor))
  {
    Serial.println(F("Unable to retrieve the firmware version!\r\n"));
  }
  else
  {
    Serial.print(F("Firmware V. : "));
    Serial.print(major); Serial.print(F(".")); Serial.println(minor);
  }

  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);

  delay(2000);
  fadeLed();
  
  /* Initialise the module */
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
  
  Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    connectionError();
    while(1);
  }
   
  Serial.println(F("Connected!"));
  
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }  

  /* Display the IP address DNS, Gateway, etc. */  
  while (! displayConnectionDetails()) {
    delay(1000);
  }
  Serial.println(F("Waiting for command..."));
  successLed();
  
}

void loop(void)
{
  boobRightState = digitalRead(BOOB_RIGHT);
  peeState = digitalRead(PEE);
  poopState = digitalRead(POOP);
  boobLeftState = digitalRead(BOOB_LEFT);
  awakeState = digitalRead(AWAKE);
  sleepState = digitalRead(SLEEP);

  if (boobRightState == LOW) {
    digitalWrite(ledPin, HIGH);
    Serial.println("Boob Right");
    fadeLed();
    sendCommand("Feeding R");
  } else if (peeState == LOW) {
    digitalWrite(ledPin, HIGH);
    Serial.println("Pee");
    fadeLed();
    sendCommand("Pee Pee");
  } else if (poopState == LOW) {
    digitalWrite(ledPin, HIGH);
    Serial.println("Poop");
    fadeLed();
    sendCommand("Pooper");
  } else if (boobLeftState == LOW) {
    digitalWrite(ledPin, HIGH);
    Serial.println("Boob Left");
    fadeLed();
    sendCommand("Feeding L");
  } else if (awakeState == LOW) {
    digitalWrite(ledPin, HIGH);
    Serial.println("Awake");
    fadeLed();
    sendCommand("Awake");
  } else if (sleepState == LOW) {
    digitalWrite(ledPin, HIGH);
    Serial.println("sleep");
    fadeLed();
    sendCommand("Sleep");
  } else {
    // turn LED off:
    digitalWrite(ledPin, LOW);
  }
}


/**************************************************************************/
/*!
    @brief  Tries to read the IP address and other connection details
*/
/**************************************************************************/
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}

bool sendCommand(char command[10]){
  Serial.println(cc3000.getStatus());
  if(cc3000.getStatus()==0){
    resetWifi();
  }
  ip = 0;
  // Try looking up the website's IP address-----------------MOVE BELOW
  Serial.print(WEBSITE); Serial.print(F(" -> "));
  while (ip == 0) {
    if (! cc3000.getHostByName(WEBSITE, &ip)) {
      Serial.println(F("Couldn't resolve!"));
    }
    delay(500);
  }

  cc3000.printIPdotsRev(ip);

  /* Try connecting to the website.
     Note: HTTP/1.1 protocol is used to keep the server from closing the connection before all data is read.
  */
  Adafruit_CC3000_Client www = cc3000.connectTCP(ip, 80);
  if (www.connected()) {
    www.fastrprint(F("POST "));
    www.fastrprint(WEBPAGE);
    www.fastrprint(F(" HTTP/1.1\r\n"));
    www.fastrprint(F("Host: ")); www.fastrprint(WEBSITE); www.fastrprint(F("\r\n"));
    www.fastrprint(F("Content-Length: 22\r\n"));
    www.fastrprint(F("Content-Type: application/json\r\n"));
    www.fastrprint(F("\r\n"));
    www.fastrprint(F("{\"value1\":\"")); www.fastrprint(command); www.fastrprint(F("\"}\r\n"));
    www.println();
  } else {
    Serial.println(F("Connection failed"));
    connectionError();    
    return false;
  }

  Serial.println(F("-------------------------------------"));
  
  /* Read data until either the connection is closed, or the idle timeout is reached. */ 
  unsigned long lastRead = millis();
  while (www.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
    while (www.available()) {
      char c = www.read();
      result += c;
      Serial.print(c);
      lastRead = millis();
    }
  }
  www.close();
  Serial.println(F("-------------------------------------"));
  if (result.indexOf("200 OK")) {
    Serial.println("Success");
    successLed();
  } else {
    Serial.println("Error");
    connectionError();
  }
  result = String("");
}

void fadeLed (void){
  while (x > 96){
    analogWrite(ledPin, x);
    x = x-1;
    delay (5);
  }
  x = 255;
}

void successLed (void){
  analogWrite(ledPin, 255);
  while (x > 50){
    analogWrite(ledPin, x);
    x = x-1;
    delay (5);
  }
  x = 255;
  analogWrite(ledPin, 255);
  while (x > -1){
    analogWrite(ledPin, x);
    x = x-1;
    delay (5);
  }
  x = 255;
}

void connectionError (void){
  while(1){
    while (x > -1){
      analogWrite(ledPin, x);
      x = x-1;
      delay (5);
    }
    x = 255;
  }
}

bool resetWifi (void){
  Serial.println("Restarting wifi...");
  cc3000.reboot();
    Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    connectionError();
    while(1);
  }
   
  Serial.println(F("Connected!"));
  
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }  

  /* Display the IP address DNS, Gateway, etc. */  
  while (! displayConnectionDetails()) {
    delay(1000);
  }
}

