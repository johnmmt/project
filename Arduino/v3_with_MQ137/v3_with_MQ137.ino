#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <ESP8266WebServer.h> 
#include <ArduinoOTA.h>


//-----------------------------------------------------------------------------------

#define WIFI_SSID "AR"
#define WIFI_PASSWORD "Nyarmotis3000"
//-----------------------------------------------------------------------------------

float lat;                                                         //Storing the Latitude
float lon;
String latitude;
String longitude;
                                                        //Storing the Longitude
const char* serverName = "http://192.168.100.37/post-esp-data--.php/";  // REPLACE with your Domain name and URL path or IP address with path
String apiKeyValue = "mmt2";                                            // If you change the apiKeyValue value, the PHP file /post-esp-data.php also needs to have the same key

//float lon= 5.53885;
//float lat= 6.34563;


//-----------------------------------------------------------------------------------
//GPS Module RX pin to NodeMCU D1
//GPS Module TX pin to NodeMCU D2
const int RXPin = 4, TXPin = 5;
SoftwareSerial neo6m(RXPin, TXPin);
TinyGPSPlus gps;
//-----------------------------------------------------------------------------------

// MQ-137 CONNECTION
float RL = 47 ; //The value of resistor RL is 47K
float m = -0.263; //Enter calculated Slope 
float b = 0.42; //Enter calculated intercept
float Ro = 20; //Enter found Ro value



// MQ-137 Read
  float VRL; //Voltage drop across the MQ sensor
  float Rs; //Sensor resistance at gas concentration 
  float ratio; //Define variable for ratio

// connections for drive Motor FR & BR
int enA = 14;
int in1 = 15;
int in2 = 13;
// connections for drive Motor FL & BL
int in3 = 2;
int in4 = 0;
int enB = 12;

String command;          // String to store app command state.
int SPEED = 330;       // 330 - 1023.
int speed_Coeff = 10;

ESP8266WebServer server(80);      // Create a webserver object that listens for HTTP request on port 80

unsigned long previousMillis = 0;

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
void setup() {

  Serial.begin(115200);

  neo6m.begin(9600);
  pinMode(D0, INPUT);
 // Serial.println("Intitialising");
 // Serial.println("------------------------------------------------");
  // Set all the motor control pins to outputs
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(enB, OUTPUT);

  // Set the speed to start, from 0 (off) to 1023 (max speed)
  analogWrite(enA, SPEED);
  analogWrite(enB, SPEED);
  
  // Turn off motors - Initial state
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);

  wifiConnect();

  server.on ( "/", HTTP_handleRoot );       // call the 'handleRoot' function when a client requests URI "/"
  server.onNotFound ( HTTP_handleRoot );    // when a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
  server.begin();                           // actually start the server
  
  ArduinoOTA.begin();                       // enable to receive update/uploade firmware via Wifi OTA

}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM


//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
void loop() {

  smartdelay_gps(1);

   
  VRL = analogRead(A0)*(3.3/1023.0); //Measure the voltage drop and convert to 0-5V
  Rs = ((5.0*RL)/VRL)-RL; //Use formula to get Rs value
  ratio = Rs/Ro;  // find ratio Rs/Ro
  float ppm = pow(10, ((log10(ratio)-b)/m)); //use formula to calculate ppm


  //Measure voltage of metal detector
  int sensorValue = analogRead(D0);
  float voltage = sensorValue * (3.3 / 1023.0);


//Display results for both sensors in serial monitor
  Serial.print("voltage: ");
  Serial.println(voltage);
  Serial.print("ppm: ");
  Serial.println(ppm);
  Serial.print("VRL: "); 
  Serial.println(VRL );


  if (voltage >= 3 || VRL>=2) {
    

    if (gps.location.isValid()) {
      float lat = gps.location.lat();
      float lon = gps.location.lng();

      String latitude = String(lat,6);
      String longitude = String(lon,6);

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
      delay(1);

    } else {
      Serial.println("No valid GPS data found.");
    }

    delay(5);
  }

  ArduinoOTA.handle();          // listen for update OTA request from clients
    server.handleClient();        // listen for HTTP requests from clients
    
      command = server.arg("State");          // check HTPP request, if has arguments "State" then saved the value
      if (command == "F") Forward();          // check string then call a function or set a value
      else if (command == "B") Backward();
      else if (command == "R") TurnRight();
      else if (command == "L") TurnLeft();
      else if (command == "G") ForwardLeft();
      else if (command == "H") BackwardLeft();
      else if (command == "I") ForwardRight();
      else if (command == "J") BackwardRight();
      else if (command == "S") Stop();
      else if (command == "0") SPEED = 330;
      else if (command == "1") SPEED = 400;
      else if (command == "2") SPEED = 470;
      else if (command == "3") SPEED = 540;
      else if (command == "4") SPEED = 610;
      else if (command == "5") SPEED = 680;
      else if (command == "6") SPEED = 750;
      else if (command == "7") SPEED = 820;
      else if (command == "8") SPEED = 890;
      else if (command == "9") SPEED = 960;
      else if (command == "q") SPEED = 1023;

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
    delay(3);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM


// function prototypes for HTTP handlers
void HTTP_handleRoot(void){
  server.send ( 200, "text/html", "" );       // Send HTTP status 200 (Ok) and send some text to the browser/client
  
  if( server.hasArg("State") ){
     Serial.println(server.arg("State"));
  }
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

// function to move forward
void Forward(){ 
  analogWrite(enA, SPEED);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(enB, SPEED);
}

// function to move backward
void Backward(){
  analogWrite(enA, SPEED);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enB, SPEED);
}

// function to turn right
void TurnRight(){
  analogWrite(enA, SPEED);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(enB, SPEED);
}

// function to turn left
void TurnLeft(){
  analogWrite(enA, SPEED);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enB, SPEED);
}

// function to move forward left
void ForwardLeft(){
  analogWrite(enA, SPEED);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(enB, SPEED/speed_Coeff);
}

// function to move backward left
void BackwardLeft(){
  analogWrite(enA, SPEED);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enB, SPEED/speed_Coeff);
}

// function to move forward right
void ForwardRight(){
  analogWrite(enA, SPEED/speed_Coeff);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(enB, SPEED);
}

// function to move backward right
void BackwardRight(){ 
  analogWrite(enA, SPEED/speed_Coeff);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enB, SPEED);
}

// function to stop motors
void Stop(){  
  analogWrite(enA, 0);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  analogWrite(enB, 0);
}