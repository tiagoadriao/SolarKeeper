#include <SPI.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include <Wire.h>
//#include "Adafruit_VEML6070.h"
#include "Adafruit_VEML6075.h"
#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>

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

float uvRawA = 0.0;
float uvRawB = 0.0;
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
    uv.setHighDynamic(false);
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
  
    // Set the calibration coefficients
    uv.setCoefficients(2.55, 1.00,  // UVA_A and UVA_B coefficients
                       3.80, 1.10,  // UVB_C and UVB_D coefficients
                       0.006, 0.0031); // UVA and UVB responses

    //----------------------//
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
    Serial.println(F("°C "));
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

  { //UV_VEML6075
    uvRawA = uv.readUVA();
    uvRawB = uv.readUVB();
    uvIndex = 11*sqrt(uv.readUVI());
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
  { //ORP meter
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
  { //WebServer message
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
