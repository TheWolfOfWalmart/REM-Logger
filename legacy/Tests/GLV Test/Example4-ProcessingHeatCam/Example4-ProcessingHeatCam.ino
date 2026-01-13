/*
  Visualizing the Panasonic Grid-EYE Sensor Data using Processing
  By: Nick Poole
  SparkFun Electronics
  Date: January 12th, 2018
  
  MIT License: Permission is hereby granted, free of charge, to any person obtaining a copy of this 
  software and associated documentation files (the "Software"), to deal in the Software without 
  restriction, including without limitation the rights to use, copy, modify, merge, publish, 
  distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the 
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or 
  substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING F
*/

#include <Wire.h>
#include <SparkFun_GridEYE_Arduino_Library.h>

GridEYE grideye;

void setup() {

  Wire.begin();
  // Library assumes "Wire" for I2C but you can pass something else with begin() if you like
  grideye.begin();
  // Pour a bowl of serial
  Serial.begin(115200);

}

void loop() {

  // Print the temperature value of each pixel in floating point degrees Celsius
  // separated by commas 
  float temp = 10.00;
  for(unsigned char i = 0; i < 64; i++){
    Serial.print(temp, 2);
    //Serial.print(grideye.getPixelTemperature(i));
    Serial.print(",");
    temp += 0.25;
  } 

  // End each frame with a linefeed
  Serial.println();

  // Give Processing time to chew
  delay(100);

}
