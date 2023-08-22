#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <map>
#include "wifi_autoconnect.h"

// Sensor constants and variables.
// Setup a oneWire instance on GPIO 13 to communicate with any OneWire device
// and pass its reference onto Dallas Temperature.
OneWire one_wire(13);
DallasTemperature sensors(&one_wire);
int number_of_devices;
DeviceAddress temperature_device_address; 
char* sensor_id;
float temp;
char temp_string[8];
const char* hub_id = "h01";

// Wifi and server constants and variables.
const char* server_name ="https://api-http.telemetry.aphetor.org/temperatures";
unsigned int timer_delay = 5000;
// last_time is an unsigned long as it will quickly become a bigger number than can be stored in an int.
unsigned long start = 0;

// Dictionary mapping address onto id.
std::map<String, char*> lookup_table = {
  {"28DE2D0A4320018C", "h1-s01"},
  {"2870A700432001AE", "h1-s02"},
  {"28B6A10243200118", "h1-s03"},
  {"284ADFFD4120017C", "h1-s04"},
  {"289741F54220011F", "h1-s05"}
};

// Function to convert a device address to a string.
String returnAddress(DeviceAddress deviceAddress) {
    String address = "";
  for (uint8_t i = 0; i < 8; i ++){
    if (deviceAddress[i] < 16) address += "0";
    address += String(deviceAddress[i], HEX);
  }
  address.toUpperCase();
  return address;
}

void setup(){
  // start the serial port and the sensors.
  Serial.begin(115200);
  sensors.begin();
  
  // Get a count of the number of devices on the wire.
  number_of_devices = sensors.getDeviceCount();
  Serial.println();
  Serial.println();
  Serial.print("Locating devices...");
  Serial.print("found ");
  Serial.print(number_of_devices, DEC);
  Serial.println(" devices.");

  // Loop through each device and get its address and id.
  for(int i = 0; i < number_of_devices; i ++){
    // Search the wire for the device's address.
    if(sensors.getAddress(temperature_device_address, i)){
      Serial.print("Found device with address ");
      Serial.print(returnAddress(temperature_device_address));
      Serial.print(" and id "); 
      Serial.print(lookup_table[returnAddress(temperature_device_address)]);
      Serial.println(".");
      
    } else {
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.println(" but could not detect address, check power and cabling.");
    }
  }

  // Setup wifi.
  Serial.println("Setting up WiFi with AutoConnect.");
  setup_wifi("h01");
  while(WiFi.status() != WL_CONNECTED) {
    handle_wifi();
  }
  Serial.println("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void loop(){ 
  if (millis() - start > timer_delay) {
    start = millis();
    
    // Reset message
    char message[1024] = "{\"hub_id\":\"";
    strcat(message, hub_id);
    strcat(message,"\",\"data\":{");
    
    // Tell the sensors to take a temperature reading.
    sensors.requestTemperatures();
    
    // Loop through each device and get its temperature data.
    for(int i = 0; i < number_of_devices; i ++){
      // Search the wire for the device's address.
      if(sensors.getAddress(temperature_device_address, i)){
        Serial.print("Temperature for device ");
        sensor_id = lookup_table[returnAddress(temperature_device_address)];
        Serial.print(sensor_id);
        Serial.print(": ");
        // Print the data
        temp = sensors.getTempC(temperature_device_address);
        strcat(message, "\"");
        strcat(message, sensor_id);
        strcat(message, "\":{\"value\":");
        dtostrf(temp, 3, 2, temp_string);
        strcat(message, temp_string);
        if (i < number_of_devices - 1){
          strcat(message, "},");
        }
        Serial.print(temp);
        Serial.println("C");
      }
    }
    strcat(message, "}}}");
  
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      //Start connection with server.
      HTTPClient http;
      http.begin(server_name);
      // Send HTTP POST request.
      http.addHeader("Content-Type", "application/json");
      Serial.print("Sending json: ");
      Serial.println(message);
      int httpResponseCode = http.POST(message);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      http.end();
      Serial.print("Time to send: ");
      Serial.println(millis() - start);
      Serial.println();
      
    } else {
      // Reconnect to wifi.
      Serial.println("WiFi Disconnected.");
      Serial.print("Please reattempt to connect via WiFi access point.");
      while(WiFi.status() != WL_CONNECTED) {
        handle_wifi();
      }
      Serial.println("Connected to WiFi network with IP Address: ");
      Serial.println(WiFi.localIP());
      Serial.println();
    }

  }
}
