
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Kylie";
const char* password = "776565763d";
const char* server_name ="https://api-http.telemetry.aphetor.org/temperatures";
unsigned int timer_delay = 5000;
// last_time is an unsigned long as it will quickly become a bigger number than can be stored in an int.
unsigned long last_time = 0;

void setup() {
  // Setup wifi.
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Attempting to connect to ");
  Serial.println(ssid);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Microcontroller will attempt to Post data every 5 seconds.");
}

void loop() {
  
  //Send an HTTP POST request every 5 seconds.
  if ((millis() - last_time) > timer_delay) {
    
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      http.begin(server_name);
      
      // Specify content-type header.
      http.addHeader("Content-Type", "application/json");
      
      // Data to send with HTTP POST.
      String httpRequestData = "{\"hub_id\":\"1\",\"hardware_id\":\"1\",\"value\":\"30\"}";    
            
      // Send HTTP POST request.
      int httpResponseCode = http.POST(httpRequestData);

      Serial.print("Sending json: ");
      Serial.println(httpRequestData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      http.end();
    }
    
    else {
      Serial.println("WiFi Disconnected");
      
    }
    last_time = millis();
  }
}
