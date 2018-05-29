// Wire Slave Sender
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Sends data as an I2C/TWI slave device
// Refer to the "Wire Master Reader" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>



   const int buttonPin = 2;     //
   int buttonState = 0;
void setup() {
  Wire.begin(8);                // join i2c bus with address #8
   // register event
   pinMode(buttonPin, INPUT);
}

void loop() {


   buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    // turn LED on:
    Wire.write("Ola ");
    delay(100);
  }
  
 
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  Wire.write("Ola "); // respond with message of 6 bytes
  // as expected by master
}
