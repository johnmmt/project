#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>


//-----------------------------------------------------------------------------------
#define FIREBASE_AUTH "MLQXmysuLkPqbRCoGBUItLC5okq1VDwtVqqYuDOw"
#define WIFI_SSID "AR"
#define WIFI_PASSWORD "Nyarmotis3000"
//-----------------------------------------------------------------------------------

float latitude;                                                         //Storing the Latitude
float longitude;                                                        //Storing the Longitude
const char* serverName = "http://192.168.100.37/post-esp-data--.php/";  // REPLACE with your Domain name and URL path or IP address with path
String apiKeyValue = "mmt2";                                            // If you change the apiKeyValue value, the PHP file /post-esp-data.php also needs to have the same key

//float longitude= 5.53885;
//float latitude= 6.34563;


//-----------------------------------------------------------------------------------
//GPS Module RX pin to NodeMCU D1
//GPS Module TX pin to NodeMCU D2
const int RXPin = 4, TXPin = 5;
SoftwareSerial neo6m(RXPin, TXPin);
TinyGPSPlus gps;
//-----------------------------------------------------------------------------------


//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
void setup() {

  Serial.begin(115200);

  neo6m.begin(9600);

  wifiConnect();
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM


//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
void loop() {

  smartdelay_gps(1000);

  //Measure voltage
  int sensorValue = analogRead(A0);
  float voltage = sensorValue * (3.3 / 1023.0);
  Serial.println(voltage);
  if (voltage >= 3) {

    if (gps.location.isValid()) {
      float latitude = gps.location.lat();
      float longitude = gps.location.lng();

      //-------------------------------------------------------------
      //Send to Serial Monitor for Debugging
      //Serial.print("LAT:  ");
      //Serial.println(latitude);  // float to x decimal places
      //Serial.print("LONG: ");
      //Serial.println(longitude);
      //-------------------------------------------------------------

      //-------------------------------------------------------------
      if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;

        // Your Domain name with URL path or IP address with path
        http.begin(client, serverName);

        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        String httpRequestData = "api_key=" + apiKeyValue + "&longitude=" + longitude
                                 + "&latitude=" + latitude + "";



        int httpResponseCode = http.POST(httpRequestData);

        //if (httpResponseCode > 0) {
        //Serial.print("HTTP Response code: ");
        //Serial.println(httpResponseCode);

        Serial.print(httpRequestData);

        //} else {
        //Serial.print("Error code: ");
        //Serial.println(http.errorToString(httpResponseCode).c_str());
        //}
        //Free resources
        http.end();
      } else {
        Serial.println("WiFi Disconnected");
      }
      //Send an HTTP POST request every 30 seconds
      delay(150);

    } else {
      Serial.println("No valid GPS data found.");
    }

    delay(5000);
  }
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM


//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
static void smartdelay_gps(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (neo6m.available())
      gps.encode(neo6m.read());
  } while (millis() - start < ms);
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM


//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
void wifiConnect() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
/*void readVoltage() {
  int sensorValue = analogRead(A0);
  float voltage = sensorValue * (3.3 / 1023.0);
  Serial.println(voltage);
}*/