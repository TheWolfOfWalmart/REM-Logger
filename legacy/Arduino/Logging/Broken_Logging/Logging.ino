
#include <SparkFun_GridEYE_Arduino_Library.h>  // AMG8833 Grideye Thermopile
GridEYE grideye;                         // Setup Grid Eye
float delta[64];     // Array to keep the differenes between current pixel and previous pixel

void setup()
{
    
//  pinMode(ledPin, OUTPUT);

 grideye.begin(); // Start Grid Eye
 Serial.begin(115200); //9600bps is used for debug statements
  Serial.print("LIS3DH test!");

  /*if (!lis.begin(0x18))
  { // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1)
      yield();
  }
  Serial.println("LIS3DH found!");

  lis.setRange(LIS3DH_RANGE_4_G); // 2, 4, 8 or 16 G!
  lis.setDataRate(LIS3DH_DATARATE_1_HZ);      // Set dataRate as set above- 1, 10, 25, 50, 100, 200, 400

  /* Header Info */

  /* Time */
  /*DateTime now = rtc.now();
  myLog.print("-- Time --");
  myLog.println("Power-on timestamp =");
  myLog.print("/t");
  myLog.print(daysOfTheWeek[now.dayOfTheWeek()]);
  myLog.print(", ");
  myLog.print(now.month(), DEC);
  myLog.print("/");
  myLog.print(now.day(), DEC);
  myLog.print("/");
  myLog.print(now.hour(), DEC);
  myLog.print("/");
  myLog.print(now.minute(), DEC);
  myLog.print("/");
  myLog.print(now.second(), DEC);
  myLog.println("Unix Time=");
  myLog.print("/t");
  //mylog.print(now.unixtime());

  /* Accelerometer */
 /* myLog.println("-- Accelerometer--");
  myLog.println("Data Rate =");
  myLog.print("/t");
  switch (lis.getDataRate())
  {
  case LIS3DH_DATARATE_1_HZ:
    myLog.println("1 Hz");
    break;
  case LIS3DH_DATARATE_10_HZ:
    myLog.println("10 Hz");
    break;
  case LIS3DH_DATARATE_25_HZ:
    myLog.println("25 Hz");
    break;
  case LIS3DH_DATARATE_50_HZ:
    myLog.println("50 Hz");
    break;
  case LIS3DH_DATARATE_100_HZ:
    myLog.println("100 Hz");
    break;
  case LIS3DH_DATARATE_200_HZ:
    myLog.println("200 Hz");
    break;
  case LIS3DH_DATARATE_400_HZ:
    myLog.println("400 Hz");
    break;

  case LIS3DH_DATARATE_POWERDOWN:
    myLog.println("Powered Down");
    break;
  case LIS3DH_DATARATE_LOWPOWER_5KHZ:
    myLog.println("5 Khz Low Power");
    break;
  case LIS3DH_DATARATE_LOWPOWER_1K6HZ:
    myLog.println("16 Khz Low Power");
    break;
  }*/
  Serial.print("setup end");
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
    float frameChangeAvg;

    //DateTime now = rtc.now(); // Stores curent time for use later in loop
    //lis.read();   // Get raw data for XYZ at once. access w/ lis.x/y/z

    /* Get sensor reading but in m/s^2 using arduino sensor library
      Accessed w. event.acceleration.x/y/z */
   // sensors_event_t event;
   // lis.getEvent(&event);

    /* Store pixel snapshot into array */
    for (unsigned char i = 0; i < 64; i++)
    {
    temps [i] = grideye.getPixelTemperature(i);
    }

    if (sizeof(temps) == 64)
    {
      float deltaSum = 0; //variable for average-of-pixel-change function

      for (int q = 0; q < 64; q++)
      {
        delta[q] = max(temps[q], pixelLast[q]) - min(temps[q], pixelLast[q]);
        frameAvg += temps[q];
        sum += temps[q];
        sumQ += sq(temps[q]);
        frameMin = min(temps[q], frameMin); // Find min temp in frame array
        frameMax = max(temps[q], frameMax);  // Find max temp in frame array

        if (q == 63)
        {
          sum = sq(sum); // Square the sum of all temps
          sum /= 64;
          frameStdDev = sumQ - sum; // Finish running Std. Dev.
        }

        pixelLast[q] = temps[q]; // Observed reading is now previous reading
      }
      for (int i = 0; i < 64; i++)
      {
        deltaSum += delta[i];
      }
      frameChangeAvg = deltaSum / 64;
      //frameAvg /= float(splitString.length);    //Average temp of frame is recorded
    }


   
    /* Write to log */
 /*   myLog.print(frameAvg);
    myLog.print(",");
    myLog.print(frameStdDev);
    myLog.print(",");
    myLog.print(frameChangeAvg);
    myLog.print(",");
    myLog.print(frameMin);
    myLog.print(",");
    myLog.print(frameMax);
    myLog.print(","); */
  }
