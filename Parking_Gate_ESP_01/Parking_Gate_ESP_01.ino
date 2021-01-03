/*********
 * File:        Parking_Gate_ESP_01.ino
 * Author:      Francisco Javier Ortín Cervera
 * Created:     15.12.2020
 * Last edit:   03.01.2021
 * 
 * Part of this code has been taken from:
 *  https://RandomNerdTutorials.com/
 * Written by Rui Santos
 * Also taken from:
 * https://stackoverflow.com/questions/63237625/esp8266-espasyncwebserver-does-not-toggle-gpio-in-callback
*********/

// Import required libraries
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

#include Params.h

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 2);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 255, 0);

const char* PARAM_INPUT_1 = "state";

const int output = 2;

// Create AsyncWebServer object on port 8888
AsyncWebServer server(8888);

boolean toggle_gpio = false;
unsigned long starting = millis();

int counter = 0;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Parking gate</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 2.6rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 10px;}
    .button {
          background-color: #195B6A;
          border: none;
          color: white;
          text-align: center;
          text-decoration: none;
          display: inline-block;
          margin: 4px 2px;
        }
.bottomleft {
  position: absolute;
  bottom: 8px;
  left: 16px;
  font-size: 18px;
border-radius: 10px; padding: 10px; font-size: 10px;
}
    .button1 {border-radius: 12px; padding: 20px; font-size: 16px;}
    .button2 {border-radius: 12px; padding: 10px; font-size: 12px;}
  </style>
</head>
<body>
  <h2>Parking gate</h2>
  <p>Parking gate virtual key: </p>
  <button onclick="switchButton()" class="button button1">Switch</button>
  <p><br>Parking gate virtual key used: <span id="times">%TIMES%</span> times<br><br></p>
  <button onclick="logoutButton()" class="button button2">Logout</button>
<script>
var counter = 0;

function switchButton() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/switch", true); 
  xhr.send();
}
function logoutButton() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/logout", true);
  xhr.send();
  setTimeout(function(){ window.open("/logged-out","_self"); }, 1000);
}
</script>
</body>
</html>
)rawliteral";

const char logout_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <p>Logged out or <a href="/">return to homepage</a>.</p>
  <p><strong>Note:</strong> close all web browser tabs to complete the logout process.</p>
</body>
</html>
)rawliteral";

// Replaces placeholder with button section in your web page
String processor(const String& var){
  Serial.print("Var: ");
  Serial.println(var);
  
  if (var == "TIMES"){
    Serial.print("Counter: ");
    Serial.println(counter);
    return String(counter);
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println("Setup init");

  pinMode(output, OUTPUT);
  digitalWrite(output, LOW);

  // Set your Static IP address

  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    request->send_P(200, "text/html", index_html, processor);
  });
    
  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(401);
  });

  server.on("/logged-out", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", logout_html, processor);
  });
  

  server.on("/switch", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    toggle_gpio = true;
    request->send_P(200, "text/html", index_html, processor);
  });
  
  // Start server
  Serial.println("Starting server...");
  server.begin();
}
  
void loop() {
  if(toggle_gpio) {
    counter++;
    Serial.println("toggling GPIO");
    delay(10);
    digitalWrite(output, HIGH);
    delay(3000);
    digitalWrite(output, LOW);
    toggle_gpio = false;
  }
  if ((millis()-starting) >= 2000){
    Serial.println(WiFi.localIP());
    starting = millis();
  }
}
