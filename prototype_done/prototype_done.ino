#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <map>
#include "wifi_autoconnect.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLDED dispay dimensions and declaration.
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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
const char* hub_id = "h08";

// Wifi and server constants and variables.
const char* server_name ="https://api-http.telemetry.aphetor.org/temperatures";
unsigned int timer_delay = 3000;
// start is an unsigned long as it will quickly become a bigger number than can be stored in an int.
unsigned long start = 0;

// Dictionary mapping address onto id.
std::map<String, char*> lookup_table = {
  //{"285B5956B5013C2F", "h01-s01"},
  //{"28640F56B5013CEF", "h01-s02"},
  //{"28091056B5013C75", "h01-s03"},
  //{"2882EE56B5013C63", "h01-s04"},
  //{"284FD856B5013CAE", "h01-s05"},
  {"28480E56B5013CE9", "h02-s01"},
  {"28ECC856B5013CD7", "h02-s02"},
  {"28422156B5013CB1", "h02-s03"},
  {"28B63956B5013C5D", "h02-s04"},
  {"28625F56B5013CC8", "h02-s05"},
  {"28D36956B5013CE0", "h03-s01"},
  {"28004056B5013C76", "h03-s02"},
  {"28EB8456B5013C9A", "h03-s03"},
  {"2895FD56B5013C8F", "h03-s04"},
  {"28EC9856B5013C42", "h03-s05"},
  {"28047B56B5013C5E", "h04-s01"},
  {"28391056B5013C98", "h04-s02"},
  {"28A19256B5013CC0", "h04-s03"},
  {"2833A156B5013CD5", "h04-s04"},
  {"28BA3456B5013CCC", "h04-s05"},
  {"28C7F956B5013CD0", "h05-s01"},
  {"28BD4356B5013C75", "h05-s02"},
  {"28B81A56B5013CF8", "h05-s03"},
  {"289D4856B5013CB3", "h05-s04"},
  {"28155B56B5013CCA", "h05-s05"},
  {"28DD1156B5013CA0", "h06-s01"},
  {"28513B56B5013C72", "h06-s02"},
  {"288F8E56B5013C38", "h06-s03"},
  {"28961556B5013C32", "h06-s04"},
  {"28955056B5013C50", "h06-s05"},
  {"28753F56B5013C07", "h07-s01"},
  {"28569356B5013CFA", "h07-s02"},
  {"28184956B5013C7F", "h07-s03"},
  {"28A82A56B5013C27", "h07-s04"},
  {"28C6CF56B5013CFF", "h07-s05"},
  {"2882EE56B5013C63", "h08-s01"},
  {"28640F56B5013CEF", "h08-s02"},
  {"285B5956B5013C2F", "h08-s03"},
  {"284FD856B5013CAE", "h08-s04"},
  {"28091056B5013C75", "h08-s05"}
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
  
  // start the serial port, the sensors and the display.
  Serial.begin(115200);
  sensors.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64 screen
    Serial.println(F("SSD1306 allocation failed"));
  }
  delay(1000);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  // Get a count of the number of devices on the wire.
  number_of_devices = sensors.getDeviceCount();
  Serial.println();
  Serial.println();
  Serial.print("Locating devices...");
  Serial.print("found ");
  Serial.print(number_of_devices, DEC);
  Serial.println(" devices.");
  Serial.println();
  display.print("Found ");
  display.print(number_of_devices, DEC);
  display.println(" devices.");

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
  char wifi_apid[32] = "temp_hub_";
  strcat(wifi_apid, hub_id);
  Serial.println();
  Serial.println("Setting up WiFi with AutoConnect.");
  Serial.print("Connect via SSID: ");
  Serial.print(wifi_apid);
  display.setCursor(0, 16);
  display.println("Connect via SSID: ");
  display.println(wifi_apid);
  display.display();
  setup_wifi(wifi_apid);
  display.display();
  while(WiFi.status() != WL_CONNECTED) {
    handle_wifi();
  }
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void loop(){ 

  // Makes the time of each loop consistent to timer_delay.
  if (millis() - start > timer_delay) {
    start = millis();

    // Clear Screen.
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    
    // Reset message.
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
        display.print(sensor_id);
        display.print(" : ");
        display.print(temp_string);
        display.println("C");
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

      display.print("SSID: ");
      display.println(WiFi.SSID());
      display.print("RSSI: ");
      display.println(WiFi.RSSI()); 
      display.display();
      
    } else {
      // Reconnect to wifi.
      display.println("WiFi Disconnected.");
      display.display();
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
