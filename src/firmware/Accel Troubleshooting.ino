
// Basic demo for accelerometer readings from Adafruit LIS3DH

#include <Wire.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <SparkFun_Qwiic_OpenLog_Arduino_Library.h>
OpenLog myLog; // Create instance

// software SPI
// Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS, LIS3DH_MOSI, LIS3DH_MISO, LIS3DH_CLK);
// hardware SPI
// Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS);
// I2C
Adafruit_LIS3DH lis = Adafruit_LIS3DH();
unsigned long counter = 0;

void setup(void)
{
    Wire.begin();
    myLog.begin(0x1E); // Open connection to OpenLog (no pun intended)
    Serial.begin(115200);
    while (!Serial)
        delay(10); // will pause Zero, Leonardo, etc until serial console opens

    Serial.println("LIS3DH test!");

    if (!lis.begin(0x18))
    { // change this to 0x19 for alternative i2c address
        Serial.println("Couldnt start");
        while (1)
            yield();
    }
    Serial.println("LIS3DH found!");

    lis.setRange(LIS3DH_RANGE_4_G); // 2, 4, 8 or 16 G!

    Serial.print("Range = ");
    Serial.print(2 << lis.getRange());
    Serial.println("G");

    lis.setDataRate(LIS3DH_DATARATE_400_HZ);
    Serial.print("Data rate set to: ");
    switch (lis.getDataRate())
    {
    case LIS3DH_DATARATE_1_HZ:
        Serial.println("1 Hz");
        break;
    case LIS3DH_DATARATE_10_HZ:
        Serial.println("10 Hz");
        break;
    case LIS3DH_DATARATE_25_HZ:
        Serial.println("25 Hz");
        break;
    case LIS3DH_DATARATE_50_HZ:
        Serial.println("50 Hz");
        break;
    case LIS3DH_DATARATE_100_HZ:
        Serial.println("100 Hz");
        break;
    case LIS3DH_DATARATE_200_HZ:
        Serial.println("200 Hz");
        break;
    case LIS3DH_DATARATE_400_HZ:
        Serial.println("400 Hz");
        break;

    case LIS3DH_DATARATE_POWERDOWN:
        Serial.println("Powered Down");
        break;
    case LIS3DH_DATARATE_LOWPOWER_5KHZ:
        Serial.println("5 Khz Low Power");
        break;
    case LIS3DH_DATARATE_LOWPOWER_1K6HZ:
        Serial.println("16 Khz Low Power");
        break;
    }
    myLog.print("X");
    myLog.print(",");
    myLog.print("Y");
    myLog.print(",");
    myLog.print("Z");
    myLog.println();
}

void loop()
{
    unsigned long milliStart = millis();
    lis.read(); // get X Y and Z data at once

    /*
    // Then print out the raw data
    Serial.print("X:  "); Serial.print(lis.x);
    Serial.print("  \tY:  "); Serial.print(lis.y);
    Serial.print("  \tZ:  "); Serial.print(lis.z);

    /* Or....get a new sensor event, normalized */
    // sensors_event_t event;
    // lis.getEvent(&event);

    /* Display the results (acceleration is measured in m/s^2)
    Serial.print("\t\tX: "); Serial.print(event.acceleration.x);
    Serial.print(" \tY: "); Serial.print(event.acceleration.y);
    Serial.print(" \tZ: "); Serial.print(event.acceleration.z);
    Serial.println(" m/s^2 ");
  */
    unsigned long millisEnd = millis();
    unsigned long elapsed = milliStart - millisEnd;
    myLog.print(elapsed);
    myLog.print(",");
    myLog.print(lis.x);
    myLog.print(",");
    myLog.print(lis.y);
    myLog.print(",");
    myLog.print(lis.z);
    myLog.println();
    myLog.syncFile();

    // delay(200);
}