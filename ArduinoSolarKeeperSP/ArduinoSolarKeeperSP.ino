#include <SPI.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include <Wire.h>
#include "Adafruit_VEML6070.h"
#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

// the IP address for the shield:
IPAddress ip(192, 168, 4, 1);

int status = WL_IDLE_STATUS;
WiFiServer server(8080);

float  h = 0;
float hic = 0;

float tempW = 0;

float pHValue = 0;
float chlValue = 0;

int uv_raw = 0;
int index = 0;

#define DHTPIN 2
#define DHTTYPE DHT11
#define ONE_WIRE_BUS 3

#define SensorPin A0            //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00            //deviation compensate
#define LED 13
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  40    //times of collection
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex=0;

String message;

Adafruit_VEML6070 uv = Adafruit_VEML6070();
DHT dht(DHTPIN, DHTTYPE);


// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//convertToIndex receives the UV raw value and convert it to index
//from int 0 to 11 and returns the value,
//it uses the VEML6070_4_T integration time
int convertToIndex(int uvraw) {
  int index = 0;
  switch (uvraw) {
    case 0 ... 746:
      index = 0;
      break;
    case 747 ... 1493:
      index = 1;
      break;
    case 1494 ... 2240:
      index = 2;
      break;
    case 2241 ... 2988:
      index = 3;
      break;
    case 2989 ... 3735:
      index = 4;
      break;
    case 3736 ... 4482:
      index = 5;
      break;
    case 4483 ... 5229:
      index = 6;
      break;
    case 5230 ... 5976:
      index = 7;
      break;
    case 5977 ... 6722:
      index = 8;
      break;
    case 6723 ... 7469:
      index = 9;
      break;
    case 7470 ... 8216:
      index = 10;
      break;
    default:
      index = 11;
      break;
  }
  return index;
}

void setup() {
  Serial.begin(9600);
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  while (status != WL_CONNECTED) {
    Serial.println("Connecting to wifi...");
    status = WiFi.begin(ssid, pass);
    WiFi.config(ip);
    delay(1000);// don't continue
  }

  server.begin();
  printWifiStatus();

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }

  { Serial.println("Sensors Begin");
    uv.begin(VEML6070_4_T);  // pass in the integration time constant
    dht.begin();
    sensors.begin();
  }
}

void loop() {
  { //DHT11
    delay(2000);
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) ) {
      Serial.println(F("Failed to read from DHT sensor!"));
    }

    // Compute heat index in Celsius (isFahreheit = false)
    hic = dht.computeHeatIndex(t, h, false);

    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C  Heat index: "));
    Serial.print(hic);
    Serial.print(F("°C "));
  }
  { //DS18B20
    delay(1000);
    Serial.print("Requesting temperatures...");
    sensors.requestTemperatures(); // Send the command to get temperatures
    Serial.println("DONE");
    Serial.print("Water temperature is: ");
    tempW = sensors.getTempCByIndex(0);
    Serial.println(sensors.getTempCByIndex(0));
  }
  { //UV_VEML6070
    uv_raw = uv.readUV();
    index = convertToIndex(uv_raw);

    Serial.print("UV light level: "); Serial.print(uv_raw);
    Serial.print(" UV Index: "); Serial.println(index);
  }

  { // pH_meter
    static unsigned long samplingTime = millis();
    static unsigned long printTime = millis();
    static float pHValue, voltage;
    if (millis() - samplingTime > samplingInterval)
    {
      pHArray[pHArrayIndex++] = analogRead(SensorPin);
      if (pHArrayIndex == ArrayLenth)pHArrayIndex = 0;
      voltage = avergearray(pHArray, ArrayLenth) * 5.0 / 1024;
      pHValue = 3.5 * voltage + Offset;
      samplingTime = millis();
    }
    if (millis() - printTime > printInterval)  //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
    {
      Serial.print("Voltage:");
      Serial.print(voltage, 2);
      Serial.print("    pH value: ");
      Serial.println(pHValue, 2);
      digitalWrite(LED, digitalRead(LED) ^ 1);
      printTime = millis();
    }
  }
  { //WebServer message
    //values = [0:uvValue 1:uVindex 2:tempAr 3:HumAr 4:tempAgua 5:pH 6:ConcentrCloro]
    message = String(uv_raw) + " " + String(index) + " " + String(hic) + " " + String(h) + " " + String(tempW) + " " + String(pHValue) + " " + String(chlValue);
    Serial.println(message);
  }
  { //WebServerDesign

   while (status != WL_CONNECTED) {
    Serial.println("Reconnecting...");
    status = WiFi.begin(ssid, pass);
    WiFi.config(ip);
    delay(1000);
   }
    WiFiClient client = server.available();   // listen for incoming clients
    if (client) {                             // if you get a client,
      // an http request ends with a blank line
      boolean currentLineIsBlank = true;
      Serial.println("new client");           // print a message out the serial port
      while (client.connected()) {            // loop while the client's connected
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          client.println(prepareHtmlPage());
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
      // give the web browser time to receive the data
      delay(1);
      //close the connection:
      Serial.println("disconnecting.");
      client.stop();
      Serial.println("client disconnected");
    }
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

String prepareHtmlPage() {
  String htmlPage =
    String("HTTP/1.1 200 OK\r\n") +
    "Content-Type: text/html\r\n" +
    "Connection: close\r\n" +
    "\r\n" +
    //"<!DOCTYPE HTML>\r\n" +
    //"<html>\r\n" +
    message;
  //"</html>\r\n";
  return htmlPage;
}

double avergearray(int* arr, int number) {
  int i;
  int max, min;
  double avg;
  long amount = 0;
  if (number <= 0) {
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if (number < 5) { //less than 5, calculated directly statistics
    for (i = 0; i < number; i++) {
      amount += arr[i];
    }
    avg = amount / number;
    return avg;
  } else {
    if (arr[0] < arr[1]) {
      min = arr[0]; max = arr[1];
    }
    else {
      min = arr[1]; max = arr[0];
    }
    for (i = 2; i < number; i++) {
      if (arr[i] < min) {
        amount += min;      //arr<min
        min = arr[i];
      } else {
        if (arr[i] > max) {
          amount += max;  //arr>max
          max = arr[i];
        } else {
          amount += arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount / (number - 2);
  }//if
  return avg;
}
