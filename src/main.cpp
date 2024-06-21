#include <Arduino.h>
#include <Wire.h> 
#include <WiFi.h>
#define address 99

char computerdata[20]; 
byte received_from_computer = 0;
byte serial_event = 0;
byte code = 0; 
char ph_data[32];
byte in_char = 0;
byte i = 0; 
int time_ = 815;
float ph_float; 

#define WIFI_SSID "enter ssid here"
#define WIFI_PASSWORD "enter password here"


void setup() 
{
  Serial.begin(9600);  
  Wire.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}


void serialEvent() {                                                         
  received_from_computer = Serial.readBytesUntil(13, computerdata, 20);        
  computerdata[received_from_computer] = 0;                                 
  serial_event = true;                                            
}


void loop() {                                                                     //the main loop.
  if (serial_event == true) {                                                     //if a command was sent to the EZO device.
    for (i = 0; i <= received_from_computer; i++) {                               //set all char to lower case, this is just so this exact sample code can recognize the "sleep" command.
      computerdata[i] = tolower(computerdata[i]);                                 //"Sleep" ≠ "sleep"
    }
    i=0;                                                                          //reset i, we will need it later 
    if (computerdata[0] == 'c' || computerdata[0] == 'r')time_ = 815;             //if a command has been sent to calibrate or take a reading we wait 815ms so that the circuit has time to take the reading.
    else time_ = 250;                                                             //if any other command has been sent we wait only 250ms.
    

    Wire.beginTransmission(address);                                              //call the circuit by its ID number.
    Wire.write((uint8_t*)computerdata, 20);                                                     //transmit the command that was sent through the serial port.
    Wire.endTransmission();                                                       //end the I2C data transmission.


    if (strcmp(computerdata, "sleep") != 0) {  	                                  //if the command that has been sent is NOT the sleep command, wait the correct amount of time and request data.
                                                                                  //if it is the sleep command, we do nothing. Issuing a sleep command and then requesting data will wake the circuit.

      delay(time_);								                                                //wait the correct amount of time for the circuit to complete its instruction.

      Wire.requestFrom(address, 32, 1); 		                                      //call the circuit and request 32 bytes (this may be more than we need)
      code = Wire.read();               		                                      //the first byte is the response code, we read this separately.

      switch (code) {							          //switch case based on what the response code is.
        case 1:                         		//decimal 1.
          Serial.println("Success");    		//means the command was successful.
          break;                        		//exits the switch case.

        case 2:                         		//decimal 2.
          Serial.println("Failed");     		//means the command has failed.
          break;                        		//exits the switch case.

        case 254:                       		//decimal 254.
          Serial.println("Pending");    		//means the command has not yet been finished calculating.
          break;                        		//exits the switch case.

        case 255:                       		//decimal 255.
          Serial.println("No Data");    		//means there is no further data to send.
          break;                        		//exits the switch case.
      }





      while (Wire.available()) {         		//are there bytes to receive.
        in_char = Wire.read();           		//receive a byte.
        ph_data[i] = in_char;					      //load this byte into our array.
        i += 1;                          		//incur the counter for the array element.
        if (in_char == 0) {              		//if we see that we have been sent a null command.
          i = 0;                         		//reset the counter i to 0.
          break;                         		//exit the while loop.
        }
      }

      Serial.println(ph_data);          		//print the data.
    }
    serial_event = false;                   //reset the serial event flag.
  }
  ph_float=atof(ph_data);
}
