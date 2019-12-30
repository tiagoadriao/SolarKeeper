#include <SPI.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include <Wire.h>
//#include "Adafruit_VEML6070.h"
#include "Adafruit_VEML6075.h"
#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>

/** INFORMATION

This code is used to read UV index from a VEML6075, air temperature from a DHT22, and the water temperature from a DS18B20.

A arduino Wifi rev.2 was used to send the information via a self Wi-Fi Access Point via a WebAPI (HTTP GET) which sends the following information:
  [0:uvValueA 1:uVindex 2:tempAr 3:HumAr 4:tempAgua 5:pH 6:ConcentrCloro]

uVValueA = raw UV value;
UVIndex = index calculated by VEML6075 algorithm;
tempAr = air temperature;
HumAr = Air Pressure (Humidity);
TempAgua = Water temperute measured from Hotel pools;
pH = pH value from pool's water;
ConcentrCloro = chlorine concentration (to be tested);

*/

char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;
WiFiServer server(8080);
 
float  h = 0;
float hic = 0;

float tempW = 0;

float chlValue = 0;

//int uv_raw = 0;
//int index = 0;

int uvRawA = 0;
int uvRawB = 0;
int uvIndex = 0;

#define DHTPIN 2
#define DHTTYPE DHT11
#define ONE_WIRE_BUS 3

#define SensorPin A0            //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00            //deviation compensate
#define samplingInterval 20
#define printInterval 800   
#define ArrayLenth  40    //times of collection

static unsigned long orpTimer = millis(); //analog sampling interval
static unsigned long printTime = millis();
static float pHValue, voltage;
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex = 0;

#define VOLTAGE 5.00    //system voltage
#define OFFSET 0        //zero drift voltage
#define orpPin A1          //orp meter output,connect to Arduino controller ADC pin
double orpValue;
int orpArray[ArrayLenth];
int orpArrayIndex = 0;

String message;

//Adafruit_VEML6070 uv = Adafruit_VEML6070();
Adafruit_VEML6075 uv = Adafruit_VEML6075();
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
    case -1 ... 746:
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

  status = WiFi.beginAP(ssid);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    // don't continue
  }

  server.begin();
  printWifiStatus();

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }

  { Serial.println("Sensors Begin");
    //uv.begin(VEML6070_4_T);  // pass in the integration time constant
      // Set the integration constant

    uv.begin();
    uv.setIntegrationTime(VEML6075_100MS);
      // Get the integration constant and print it!
    Serial.print("Integration time set to ");
    switch (uv.getIntegrationTime()) {
      case VEML6075_50MS: Serial.print("50"); break;
      case VEML6075_100MS: Serial.print("100"); break;
      case VEML6075_200MS: Serial.print("200"); break;
      case VEML6075_400MS: Serial.print("400"); break;
      case VEML6075_800MS: Serial.print("800"); break;
    }
    Serial.println("ms");
      // Set the high dynamic mode
    uv.setHighDynamic(true);
    // Get the mode
    if (uv.getHighDynamic()) {
      Serial.println("High dynamic reading mode");
    } else {
      Serial.println("Normal dynamic reading mode");
    }
  
    // Set the mode
    uv.setForcedMode(false);
    // Get the mode
    if (uv.getForcedMode()) {
      Serial.println("Forced reading mode");
    } else {
      Serial.println("Continuous reading mode");
    }
  
    // Set the calibration coefficients for UV
    uv.setCoefficients(2.22, 1.33,  // UVA_A and UVA_B coefficients
                       2.95, 1.74,  // UVB_C and UVB_D coefficients
                       0.001461, 0.002591); // UVA and UVB responses

    //----------------------//
    dht.begin();
    sensors.begin();
  }
}

void loop() {
  { //DHT11 air temperature sensor
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
    Serial.println(F("°C "));
  }
  { //DS18B20 water temperature sensor
    delay(1000);
    Serial.print("Requesting temperatures...");
    sensors.requestTemperatures(); // Send the command to get temperatures
    Serial.println("DONE");
    Serial.print("Water temperature is: ");
    tempW = sensors.getTempCByIndex(0);
    Serial.println(sensors.getTempCByIndex(0));
  }
//  { //UV_VEML6070
//    uv_raw = uv.readUV();
//    index = convertToIndex(uv_raw);
//
//    Serial.print("UV light level: "); Serial.print(uv_raw);
//    Serial.print(" UV Index: "); Serial.println(index);
//  }

  { //UV_VEML6075 UV index sensor
    uvRawA = uv.readUVA();
    uvRawB = uv.readUVB();
    uvIndex = uv.readUVI();
    Serial.print("Raw UVA reading:  "); Serial.println(uvRawA);
    Serial.print("Raw UVB reading:  "); Serial.println(uvRawB);
    Serial.print("UV Index reading: "); Serial.println(uvIndex);
  }

  { // pH_meter
    static unsigned long samplingTime = millis();
    static unsigned long printTime = millis();
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
      printTime = millis();
    }
  }
  { //ORP meter (Chlorine concentration sensor)
    if (millis() >= orpTimer)
    {
      orpTimer = millis() + 20;
      orpArray[orpArrayIndex++] = analogRead(orpPin);  //read an analog value every 20ms
      if (orpArrayIndex == ArrayLenth) {
        orpArrayIndex = 0;
      }
      orpValue = ((30 * (double)VOLTAGE * 1000) - (75 * avergearray(orpArray, ArrayLenth) * VOLTAGE * 1000 / 1024)) / 75 - OFFSET; //convert the analog value to orp according the circuit
    }
    if (millis() >= printTime)  //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
    {
      printTime = millis() + 800;
      Serial.print("ORP: ");
      Serial.print((int)orpValue);
      Serial.println("mV");
    }
  }

  { //WebAPI
    //values = [0:uvValue 1:uVindex 2:tempAr 3:HumAr 4:tempAgua 5:pH 6:ConcentrCloro]
    message = String(uvRawA) + " " + String(uvIndex) + " " + String(hic) + " " + String(h) + " " + String(tempW) + " " + String(pHValue) + " " + String(orpValue);
    Serial.println(message);
  }
  { //WebServerDesign
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
    "Content-Type: application/json\r\n" +
    "Connection: close\r\n" +
    // "Refresh: 10\r\n" +
    "\r\n" +
    //"<!DOCTYPE HTML>\r\n" +
    //"<html>\r\n" +
    message;
  //"</html>\r\n";
  return htmlPage;
}

//this method is used on ORP and pH calculations to get an average meter.
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
