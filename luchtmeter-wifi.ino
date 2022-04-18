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

#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#ifdef ESP32
  #include <SPIFFS.h>
#endif

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

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

char apiKey[40];

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

WiFiClient client;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

    //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config1.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config1.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);

#ifdef ARDUINOJSON_VERSION_MAJOR >= 6
        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buf.get());
        serializeJson(json, Serial);
        if ( ! deserializeError ) {
#else
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
#endif
          Serial.println("\nparsed json");
          strcpy(apiKey, json["apiKey"]);
        } else {
          Serial.println("failed to load json config");
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter deviceId("apiKey", "Device Id", apiKey, 40);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  wifiManager.addParameter(&deviceId);

  //reset settings - for testing
  //wifiManager.resetSettings();

  if (!wifiManager.autoConnect("Air-Meter-AP", "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  //read updated parameters
  strcpy(apiKey, deviceId.getValue());

  Serial.println("The values in the file are: ");
  Serial.println("\tkey : " + String(apiKey));

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
#ifdef ARDUINOJSON_VERSION_MAJOR >= 6
    DynamicJsonDocument json(1024);
#else
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
#endif
    json["apiKey"] = apiKey;

    File configFile = SPIFFS.open("/config1.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

#ifdef ARDUINOJSON_VERSION_MAJOR >= 6
    serializeJson(json, Serial);
    serializeJson(json, configFile);
#else
    json.printTo(Serial);
    json.printTo(configFile);
#endif
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());

  while (!Serial) { delay(10); }
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

  // EEPROM.begin(sizeof(baselines));
  // EEPROM.get(0, baselines);
  // Serial.print("< Restauring baselines values: eCO2: 0x"); Serial.print(baselines.co2, HEX);
  // Serial.print(" & TVOC: 0x"); Serial.println(baselines.tvoc, HEX);
  // sgp.setIAQBaseline(baselines.co2, baselines.tvoc); 

}

int counter = 0;
void loop() {
  if ((WiFi.status() == WL_CONNECTED)) {
    
    delay(5000);  

    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();

    // Check if all values are read correctly, if not try again and exit loop()
    if (isnan(h) || isnan(t)) {
      Serial.println("Reading DHT sensor failed!");
      return;
    }
    // for calibration optional
    //uint16_t co2idx = 1;


    // keep this lines just in case

    if (! sgp.IAQmeasure()) {
      Serial.println("Measurement failed");
      return;
    }

    TVOC = sgp.TVOC;
    eCO2 = sgp.eCO2;

    Serial.print("TVOC "); Serial.print(TVOC); Serial.print(" ppb\t");
    Serial.print("eCO2 "); Serial.print(eCO2); Serial.println(" ppm");

    if (! sgp.IAQmeasureRaw()) {
      Serial.println("Raw Measurement failed");
      return;
    }

    // calibratie

    counter++;
    if (counter == 30) {
      counter = 0;

      uint16_t TVOC_base, eCO2_base;
      if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
        Serial.println("Failed to get baseline readings");
        return;
      }
      Serial.print("****Baseline values: eCO2: 0x"); Serial.print(eCO2_base, HEX);
      Serial.print(" & TVOC: 0x"); Serial.println(TVOC_base, HEX);
    }

    // Optioneel voor calibratie

    // if (!sgp.IAQmeasure()) {
    //   Serial.println("Measurement failed");
    //   return;
    // }

    // TVOC = sgp.TVOC;
    // eCO2 = sgp.eCO2;

    // if (co2idx++ % 360 == 0) {
    //   if (!sgp.getIAQBaseline(&baselines.co2, &baselines.tvoc)) {
    //     Serial.println("Failed to get baseline readings");
    //   } else {
    //     Serial.print("> Saving baselines values: eCO2: 0x"); Serial.print(baselines.co2, HEX);
    //     Serial.print(" & TVOC: 0x"); Serial.println(baselines.tvoc, HEX);
    //     EEPROM.put(0, baselines);
    //     EEPROM.commit();
    //   }
    // }

    Serial.print("connecting to ");
    Serial.println(serverName);
 
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(serverName, httpPort)) {
        Serial.println("connection failed");
        return;
    }

    Serial.print("TVOC "); Serial.print(TVOC); Serial.print(" ppb\t");
    Serial.print("eCO2 "); Serial.print(eCO2); Serial.println(" ppm");
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
                 "Host: " + serverName + "\r\n" +
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