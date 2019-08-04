#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#include <ThingSpeak.h>
#include "ESP8266WiFi.h"
#include <DHT.h>
#include <math.h>

#define DHTPIN D2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;

float chk;
float hum;
float temp;

const char* ssid = "";
const char* pass = "";
const int cID = 342038;
const char* writeAPIKey = "BEXG20AHHNVR8C1Q";
const char* server = "api.thingspeak.com";
const int postingInterval = 5 * 1000;

int wifiTimeout = 1000 * 30;
int bungEvents = 0;

// This code runs once upon startup. 
void setup() {
  pinMode(D1, OUTPUT);
  digitalWrite(D1, LOW);
  Serial.begin(9600);
  dht.begin();
  WiFi.begin(ssid, pass);

  // pause the code execution till we have a wifi connection
  // However if we exceed the timeout, then send the reset signal
  // anyway
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting..");
    if (millis() > wifiTimeout){
      digitalWrite(D1, HIGH);
      }
    }
  ThingSpeak.begin(client);
  // put your setup code here, to run once:

}

//This code loops continually. However because we are sending
//a signal that powers down this controller, it'll only run once.
void loop() {
  // Read in our data
  hum = dht.readHumidity();
  temp = dht.readTemperature();

  // If we don't get any information then wait for a bit
  // However if we have more then 5 failed attempts at
  // information gathering, then reset and try later
  if (isnan(hum) || isnan(temp)) {
    bungEvents++;
    if (bungEvents > 4) {
      digitalWrite(D1, HIGH);
    } else {
      delay(500);
      bungEvents = 0;
    }
  }

  // We have data, good to go
  else {
    //Serial output
    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.print(" %, Temp: ");
    Serial.print(temp);
    Serial.println(" Celsius");

    // Thingspeak output
    // Check we can connect connect first
    if (client.connect(server, 80)) {
      ThingSpeak.setField(1, temp);
      ThingSpeak.setField(2, hum);
      ThingSpeak.writeFields(cID, writeAPIKey);
    }
    else {
      Serial.println("Could not connect to ThingSpeak!");
    }

    // Finished. Send the reset signal
    digitalWrite(D1, HIGH);

    // This will delay the posting to thingspeak if we are connected 
    // via usb. However if not, then before this is finished the
    // controller will have powered down
    delay(5000);
    
  }
}
