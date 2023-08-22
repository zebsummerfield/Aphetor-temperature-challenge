
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into GPIO 25
const int one_wire_bus = 13;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire one_wire(one_wire_bus);

// Pass our oneWire reference to Dallas Temperature
DallasTemperature sensors(&one_wire);

// Number of temperature devices found
int number_of_devices;

// Variable to handle any address found
DeviceAddress temperature_device_address; 

void setup(){
  // start serial port
  Serial.begin(115200);
  
  // Start up the library
  sensors.begin();
  
  // Grab a count of devices on the wire
  number_of_devices = sensors.getDeviceCount();
  
  // Locate devices on the bus
  Serial.println();
  Serial.println();
  Serial.print("Locating devices...");
  Serial.print("found ");
  Serial.print(number_of_devices, DEC);
  Serial.println(" devices.");

  // Loop through each device, print out address
  for(int i=0;i<number_of_devices; i++){
    if(sensors.getAddress(temperature_device_address, i)){
      Serial.print("Found device ");
      Serial.print(i, DEC);
      Serial.print(" with address: ");
      printAddress(temperature_device_address);
      Serial.println();
    } else {
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address, check power and cabling.");
    }
  }
}

void loop(){ 
  // Send the command to get temperatures
  sensors.requestTemperatures();
  
  // Loop through each device, print out temperature data
  for(int i=0;i<number_of_devices; i++){
    // Search the wire for address
    if(sensors.getAddress(temperature_device_address, i)){
      // Output the device ID
      Serial.print("Temperature for device ");
      Serial.print(i,DEC);
      Serial.print(": ");
      // Print the data
      float tempC = sensors.getTempC(temperature_device_address);
      Serial.print(tempC);
      Serial.println("C");
    }
  }
  delay(1000);
}

// function to print a device address
void printAddress(DeviceAddress device_address) {
  for (uint8_t i = 0; i < 8; i++){
    if (device_address[i] < 16) Serial.print("0");
      Serial.print(device_address[i], HEX);
  }
}
