#include <Wire.h>                             // Include wire library for I2C
#include <SparkFun_GridEYE_Arduino_Library.h> // AMG8833 Grideye Thermopile
#include <Adafruit_LIS3DH.h>                  // Accelerometer sensor
#include <Adafruit_Sensor.h>    // Adafruit sensor library used for accelerometer(?)
#include <RTClib.h>   // PCF8523 RTC
#include <SparkFun_Qwiic_OpenLog_Arduino_Library.h>     // Openlog

/* --- Controls --- */
boolean debug = TRUE;   // Serial Debug mode

OpenLog myLog;      // Create log instance
GridEYE grideye;    // Init grideye
Adafruit_LIS3DH lis = Adafruit_LIS3DH();    // init accelerometer
RTC_PCF8523 rtc;        // Init RTC

char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};    // Array to hold days of the week


void setup(void)
{
  Wire.begin();       // Init I2C
  Serial.begin(9600);
  grideye.begin();
  delay(50);    // Allow grid eye time to init
  //Serial.print("TEST");
  rtc.begin();
  lis.setRange(LIS3DH_RANGE_4_G);     // 2, 4 ,8, or 16 G
  lis.setDataRate(LIS3DH_DATARATE_10_HZ);      // 1, 10, 25, 50, 100, 200, 400 Hz
  myLog.begin();        //Open connection to OpenLog (no pun intended)

  /* Header Info*/
  DateTime now = rtc.now();
  myLog.println("-- Time --");
  myLog.print("Power-on timestamp = ");
  myLog.print(daysOfTheWeek[now.dayOfTheWeek()]);
  myLog.print(", ");
  myLog.print(now.month(), DEC);
  myLog.print("/");
  myLog.print(now.day(), DEC);
  myLog.print("/");
  myLog.print(now.year(), DEC);
  myLog.print(" ");
  myLog.print(now.hour(), DEC);
  myLog.print(":");
  myLog.print(now.minute(), DEC);
  myLog.print(":");
  myLog.println(now.second(), DEC);
  myLog.print("Unix Time= ");
  myLog.println(now.unixtime());
  myLog.println();

  /* Accelerometer */
  myLog.println("-- Accelerometer--");
  myLog.print ("Data Rate = ");

  /* Cannot get this switch case to work. lis.getDataRate() only ever returns
    15 regardless of what the data rate is set to */
  /*
    switch (lis.getDataRate()) {

     case LIS3DH_DATARATE_1_HZ: Serial.println("1 Hz"); break;
     case LIS3DH_DATARATE_10_HZ: Serial.println("10 Hz"); break;
     case LIS3DH_DATARATE_25_HZ: Serial.println("25 Hz"); break;
     case LIS3DH_DATARATE_50_HZ: Serial.println("50 Hz"); break;
     case LIS3DH_DATARATE_100_HZ: Serial.println("100 Hz"); break;
     case LIS3DH_DATARATE_200_HZ: Serial.println("200 Hz"); break;
     case LIS3DH_DATARATE_400_HZ: Serial.println("400 Hz"); break;

     case LIS3DH_DATARATE_POWERDOWN: Serial.println("Powered Down"); break;
     case LIS3DH_DATARATE_LOWPOWER_5KHZ: Serial.println("5 Khz Low Power"); break;
     case LIS3DH_DATARATE_LOWPOWER_1K6HZ: Serial.println("16 Khz Low Power"); break;
    }
  */
  myLog.println();
  myLog.println("------");
  myLog.println();
  myLog.println("Frame Avg. Temp | Frame Std. Dev | Frame Change Avg. | Min | Max | X | Y | Z");
  myLog.println();
}

void loop()
{
  float pixelLast[64];
  float sum = 0.0;  //(Σ xi)^2 Sum of all temps in frame for Std. Dev. calculation
  float sumQ = 0.0; // Σ(xi2)  Sum of square of all temps in frame for Std. Dev. calculation
  float frameMin;   //lowest temp in frame
  float frameMax;   //highest temp in frame
  float frameAvg;   //avg temp of frame
  float frameStdDev;
  float temps[64];
  float delta[64];    // Array to keep the differenes between current pixel and previous pixel
  float frameChangeAvg;
  DateTime now = rtc.now();   // Variable for storing current time

/*  Debug   */
  /* Store pixel snapshot into array */
  if (debug == TRUE)
  {
    for (unsigned char i = 0; i < 64; i++)
    {
      temps [i] = grideye.getPixelTemperature(i);   // Populate temp array with sensor array readings

      /* Serial Debug Block to display temp array for frame */
      byte x;
      byte y;
      Serial.print(temps[i]);
      Serial.print(" ");
      x++;
      if (x == 8)
      {
        Serial.println();
        x = 0;
        y++;
      }
      if (y == 8)
      {
        Serial.println();
        Serial.println();
        y = 0;
      }
    }

  }
/*  /Debug  */


  /* Once array is full, process stats */
  if (sizeof(temps) == 64)
  {
    float deltaSum = 0;   //variable for average-of-pixel-change function

    /* Go through each pixel in array and do some mathing */
    for (int q = 0; q < 64; q++)
    {
      delta[q] = max(temps[q], pixelLast[q]) - min(temps[q], pixelLast[q]);   //subtract whatever is most from whatever is least
      frameAvg += temps[q];   // sum of temps for finding avg. temp of frame
      sum += temps[q];    // sum of temps for finding standard deviation, consider combining
      sumQ += sq(temps[q]);   // Sum of the square of every temp in frame for finding std. dev
      frameMin = min(temps[q], frameMin); // Find min temp in frame array
      frameMax = max(temps[q], frameMax);  // Find max temp in frame array

      /* on the last pixel, do this: */
      if (q == 63)
      {
        sum = sq(sum); // Square the sum of all temps
        sum /= 64;
        frameStdDev = sumQ - sum; // Finish running Std. Dev.
      }
      pixelLast[q] = temps[q]; // Observed reading is now previous reading
    }

    /* After all entire frame is processed, do this: */
    for (int i = 0; i < 64; i++)
    {
      deltaSum += delta[i];   // sum of delta between i & i-1
    }
    frameChangeAvg = deltaSum / 64;   // Find average amount of change in pixel temp beteen current and last frame
    frameAvg /= 64;    //Average temp of frame is recorded
  }

  /* prepare accelerometer reading for logging */
  sensors_event_t event;
  lis.getEvent(&event);


  /* Write to log */
  /* myLog.print(now.unixtime());
    myLog.print(",");
    myLog.print(frameAvg);
    myLog.print(",");
    myLog.print(frameStdDev);
    myLog.print(",");
    myLog.print(frameChangeAvg);
    myLog.print(",");
    myLog.print(frameMin);
    myLog.print(",");
    myLog.print(frameMax);
    myLog.print(",");
    myLog.print(event.acceleration.x);
    myLog.print(",");
    myLog.print(event.acceleration.y);
    myLog.print(",");
    myLog.println(event.acceleration.z);
    myLog.syncFile(); */

  /* Test block before logging*/
  /* Serial.print(now.unixtime());
    Serial.print(",");
    Serial.print(frameAvg);
    Serial.print(",");
    Serial.print(frameStdDev);
    Serial.print(",");
    Serial.print(frameChangeAvg);
    Serial.print(",");
    Serial.print(frameMin);
    Serial.print(",");
    Serial.print(frameMax);
    Serial.print(",");
    Serial.print(event.acceleration.x);
    Serial.print(",");
    Serial.print(event.acceleration.y);
    Serial.print(",");
    Serial.println(event.acceleration.z);*/

}
