#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

char ssid[] = "Netlab-OIL";        // your network SSID (name)
char pass[] = "DesignChallenge"; 
int status = WL_IDLE_STATUS;
char host[] = "192.168.155.34";


const int sensorPin = 3; 
int magnetCounter = 0; 
int inactiveCounter = 0;
unsigned long kcalCounter = 0;
int previousState = HIGH;
int prevMilles = 0;
int pointsCounter = 0;

WiFiClient client;




void sendDataToServer(int amount, int kcal_burned) {
    if (!client.connect(host, 3000)) {
        Serial.println("Connection to server failed");
        return;
    }
    else{
        Serial.println ("It works! :D");
    }
    JsonDocument doc;

    doc["amount"] = amount;
    doc["userId"] = 1;
    doc["kcal_burned"] = kcal_burned;

    String url = "/user";
    String data;
    serializeJson(doc, data);
    client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                "Content-Type: application/json\r\n" + // Specify JSON content type
                "Content-Length: " + data.length() + "\r\n" +
                "Host: " + host + "\r\n" +
                "Connection: close\r\n\r\n");
    client.print(data);
 
    delay(10);
    client.stop();
}

void setup() {
  pinMode(sensorPin, INPUT);

  Serial.begin(9600);

  while (!Serial) {
    // wait for serial port to connect. Needed for native USB port only
  }
  
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }
  
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }
  
  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
     
    // wait 10 seconds for connection:
    delay(10000);
  }
  
  Serial.println("Wifi Connected :)");
  // printWiFiStatus();
 
}



void loop() {
  if (!client.connected()) {
    while (!client.connected()) {
      client.connect(host, 3000);
      delay(500);
    }
  }


  int currentState = digitalRead(sensorPin);
  if (currentState != previousState) {
    previousState = currentState;
      Serial.print("Magnet detected! Counter: ");
      Serial.println(magnetCounter);
      inactiveCounter = 0;

      magnetCounter++;
      if (magnetCounter >= 5) {
        kcalCounter++;
        pointsCounter += 40;
        magnetCounter = 0;
        Serial.print("Calories burned: ");
        Serial.println(kcalCounter);
      }
  } 

  int currentMilles = millis();
  
  if (currentState == previousState) {
    if ((currentMilles - prevMilles) >= 1000) {
      prevMilles = currentMilles;
      inactiveCounter++;
      Serial.println("Counter: "  + String(inactiveCounter));
      if (inactiveCounter >= 10 && pointsCounter > 0) {
        Serial.println("send to database, points: " + String(pointsCounter));
        sendDataToServer(pointsCounter, kcalCounter);
        pointsCounter = 0;
        inactiveCounter = 0;
        kcalCounter = 0;
      }
    }
  } 
} 