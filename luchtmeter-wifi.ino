#include <Wire.h>
#include "Adafruit_SGP30.h"

Adafruit_SGP30 sgp;

struct BASELINES {
  uint16_t co2;
  uint16_t tvoc;
};

BASELINES baselines;

#include <ESP8266WiFi.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

#include <EEPROM.h>

#include "DHT.h"

#define DHTPIN 14  // d5 / GPIO14
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors

DHT dht(DHTPIN, DHTTYPE);

float humidityVal;           // humidity
float tempValC;              // temperature in degrees Celcius
float tempValF;              // temperature in degrees Fahrenheit
float heatIndexC;            // windchill in degrees Celcius
float heatIndexF;            // windchill in degrees Fahrenheit

float TVOC;
float eCO2;

//Your Domain name with URL path or IP address with path
//const char* serverName = "https://api.thingspeak.com/update";
const char* serverName = "api.thingspeak.com";

const String apiKey = "";

WiFiClient client;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  dht.begin(); // start with reading the DHT sensor

  if (! sgp.begin()){
    Serial.println("Sensor not found :(");
    while (1);
  }
  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);

  // Calibration baselines wordt opgeslagen in EEPROM, zodat er geen calibratie van 12 uur lang hoef plaats te vinden

  EEPROM.begin(sizeof(baselines));
  EEPROM.get(0, baselines);
  Serial.print("< Restauring baselines values: eCO2: 0x"); Serial.print(baselines.co2, HEX);
  Serial.print(" & TVOC: 0x"); Serial.println(baselines.tvoc, HEX);
  sgp.setIAQBaseline(baselines.co2, baselines.tvoc); 

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  while (!Serial) { delay(10); }

  //reset saved settings
  //wifiManager.resetSettings();

  // make AP with name
  wifiManager.autoConnect("AutoConnectAP");
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();  
}

int counter = 0;
void loop() {
  if ((WiFi.status() == WL_CONNECTED)) {

    delay(1000);

    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();

    // Check if all values are read correctly, if not try again and exit loop()
    if (isnan(h) || isnan(t)) {
      Serial.println("Reading DHT sensor failed!");
      return;
    }

    uint16_t co2idx = 1;


    // keep this lines just in case

    // if (! sgp.IAQmeasure()) {
    //   Serial.println("Measurement failed");
    //   return;
    // }

    // TVOC = sgp.TVOC;
    // eCO2 = sgp.eCO2;

    // Serial.print("TVOC "); Serial.print(TVOC); Serial.print(" ppb\t");
    // Serial.print("eCO2 "); Serial.print(eCO2); Serial.println(" ppm");

    // if (! sgp.IAQmeasureRaw()) {
    //   Serial.println("Raw Measurement failed");
    //   return;
    // }

    if (!sgp.IAQmeasure()) {
      Serial.println("Measurement failed");
      return;
    }

    TVOC = sgp.TVOC;
    eCO2 = sgp.eCO2;

    if (co2idx++ % 360 == 0) {
      if (!sgp.getIAQBaseline(&baselines.co2, &baselines.tvoc)) {
        Serial.println("Failed to get baseline readings");
      } else {
        Serial.print("> Saving baselines values: eCO2: 0x"); Serial.print(baselines.co2, HEX);
        Serial.print(" & TVOC: 0x"); Serial.println(baselines.tvoc, HEX);
        EEPROM.put(0, baselines);
        EEPROM.commit();
      }
    }

    Serial.print("connecting to ");
    Serial.println(serverName);
 
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(serverName, httpPort)) {
        Serial.println("connection failed");
        return;
    }
    // We now create a URI for the request
    String url = "/update";
    url += "?api_key=";
    url += apiKey;
    url += "&field1=";
    url += t;
    url += "&field2=";
    url += h;
    url += "&field3=";
    url += TVOC;
    url += "&field4=";
    url += eCO2;
 
    Serial.print("Requesting URL: ");
    Serial.println(url);
 
    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + "api.thingspeak.com" + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }
 
    // Read all the lines of the reply from server and print them to Serial
    while(client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }
 
    Serial.println();
    Serial.println("closing connection");
    
  } else {
    //USE_SERIAL.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    Serial.println("Geen wifi");
  }
  
}