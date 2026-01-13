/**
 * @file Logger.ino
 * @brief Firmware for the Lucidity REM Sleep Tracker (v2.0 - Tuned).
 * 
 * This sketch handles high-frequency data acquisition from:
 * - AMG8833 Grid-EYE Thermal Camera (I2C)
 * - LIS3DH Accelerometer (I2C)
 * - PCF8523 Real Time Clock (I2C)
 * 
 * Features:
 * - Exponential Moving Average (EMA) & Simple Moving Average (SMA) filtering.
 * - Gravity Compensation & Normalized Accelerometer output.
 * - Start-up delay to avoid calibration noise.
 * 
 * @author TheWolfOfWalmart
 * @date 2021-11-23 (Updated)
 */

#include <Wire.h>
#include <SparkFun_GridEYE_Arduino_Library.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <RTClib.h>
#include <SparkFun_Qwiic_OpenLog_Arduino_Library.h> 
#include <Ewma.h>
#include <SPI.h>
#include <SD.h>
#include <MovingAverage.h>

// --- Configuration Constants ---
#define SD_CS_PIN 4      // Chip Select for SD card
#define LED_PIN   13     // Onboard LED for status/error blinking
#define ERROR_PIN 8      // Additional error indicator pin

/* --- Controls & Settings --- */
const bool  DEBUG_SERIAL   = true;   // Output to serial console
const bool  DEBUG_LOGGER   = false;  // Extra debug info in log file
const bool  WRITE_TO_LOG   = true;   // Enable SD card logging
const int   LOG_RATE_MS    = 0;      // Sampling interval (0 = max speed)

const float ACC_ALPHA      = 0.04;   // EMA Alpha for Accelerometer
const float TEMP_ALPHA     = 0.01;   // EMA Alpha for Temperature
const int   SMA_PERIOD     = 100;    // Window size for Simple Moving Average
const int   GRAV_COMP      = 1300;   // Gravity compensation value (for LIS3DH range)
const unsigned long START_DELAY_MS = 60000; // 60s warm-up delay

// --- Global Objects ---
Ewma accNormF(ACC_ALPHA);                     // EMA filter for normalized value from accelerometer
Ewma emaXF(ACC_ALPHA);                        // EMA filter for accelerometer X axis
Ewma emaYF(ACC_ALPHA);                        // EMA filter for accelerometer Y axis
Ewma emaZF(ACC_ALPHA);                        // EMA filter for accelerometer Z axis
Ewma emaStdDevF(TEMP_ALPHA);                  // EMA filter for temperature readings
Ewma emaChangeAvgF(TEMP_ALPHA);               // EMA filter for temperature readings
MovingAverage<long> smaStdDevF(SMA_PERIOD);    // SMA for temp readings
MovingAverage<long> smaChangeAvgF(SMA_PERIOD); // SMA filter for temperature readings

File sleepLog;                           
GridEYE grideye;                         
Adafruit_LIS3DH lis = Adafruit_LIS3DH(); 
RTC_PCF8523 rtc;                         

// --- State Variables ---
const char DAYS_OF_WEEK[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
int pixelLast[64];
unsigned long unixLast;
int subSecCounter;
char currentLog[15] = "/ANALOG00.TXT"; 
unsigned long startTime = 0; // Timestamp of first sample


/**
 * @brief Blinks out an error code using the onboard LED.
 */
void error(uint8_t errno)
{
  while (true)
  {
    for (uint8_t i = 0; i < errno; i++)
    {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
    for (uint8_t i = errno; i < 10; i++)
    {
      delay(200);
    }
  }
}

/**
 * @brief Writes the CSV header and configuration metadata to the log file.
 */
void writeLogHeader()
{
  DateTime now = rtc.now();
  sleepLog.print("Power-on time = ");
  sleepLog.print(DAYS_OF_WEEK[now.dayOfTheWeek()]);
  sleepLog.print(", ");
  sleepLog.print(now.month(), DEC);
  sleepLog.print("/");
  sleepLog.print(now.day(), DEC);
  sleepLog.print("/");
  sleepLog.print(now.year(), DEC);
  sleepLog.print(" ");
  sleepLog.print(now.hour(), DEC);
  sleepLog.print(":");
  sleepLog.print(now.minute(), DEC);
  sleepLog.print(":");
  sleepLog.println(now.second(), DEC);
  sleepLog.println();

  sleepLog.println("--Accelerometer--");
  sleepLog.print("Data Rate = 50Hz, Range = ");
  sleepLog.print(2 << lis.getRange());
  sleepLog.println("G\n");

  sleepLog.println("--Filter Settings--");
  sleepLog.print("Acc Alpha = "); sleepLog.println(ACC_ALPHA);
  sleepLog.print("Temp Alpha = "); sleepLog.println(TEMP_ALPHA);
  sleepLog.println();

  // CSV Columns
  sleepLog.println("Unix time,Milli,Duration,Avg. Temp,Std Dev,Std Dev_SMA,Std Dev_EMA,Change Avg.,Change_SMA,Change_EMA,X_Raw,Y_Raw,Z_Raw,X_EMA,Y_EMA,Z_EMA,Acc. Norm");
}

void setup(void)
{
  Wire.begin(); 
  Serial.begin(115200);
  grideye.begin();
  delay(50); 
  rtc.begin();

  if (!lis.begin(0x18))
  {
    Serial.println("Could not find LIS3DH accelerometer!");
    error(4);
  }
  
  lis.setRange(LIS3DH_RANGE_16_G);
  lis.setDataRate(LIS3DH_DATARATE_50_HZ);

  if (DEBUG_SERIAL)
  {
    Serial.println("LIS3DH Initialized.");
    Serial.print("Range = "); Serial.print(2 << lis.getRange()); Serial.println("G");
  }

  /* -- SD Card Initialization -- */
  if (!SD.begin(SD_CS_PIN))
  {
    Serial.println("Card init. failed!");
    error(2);
  }

  char filename[15];
  strcpy(filename, "/ANALOG00.TXT"); 

  for (uint8_t i = 0; i < 100; i++)
  {
    filename[7] = '0' + i / 10;
    filename[8] = '0' + i % 10;
    if (!SD.exists(filename))
    {
      strcpy(currentLog, filename);
      break;
    }
  }

  sleepLog = SD.open(currentLog, FILE_WRITE);
  if (!sleepLog)
  {
    Serial.println("Could not create log file.");
    error(3);
  }

  pinMode(LED_PIN, OUTPUT);
  pinMode(ERROR_PIN, OUTPUT);
  Serial.println("Ready!");

  if (WRITE_TO_LOG) 
  {
    writeLogHeader();
  }
  sleepLog.close();
}

uint8_t i = 0;

void loop()
{
  unsigned long startMillis = millis(); 

  float sum = 0.0;     
  float sumOfSq = 0.0; 
  float frameAvg;      
  float frameStdDev;
  float temps[64];
  float delta[64]; 
  float deltaSum = 0.0;     
  DateTime now = rtc.now(); 

  // --- Step 1: Data Acquisition (Grid-EYE) ---
  for (uint8_t i = 0; i < 64; i++) 
  {
    temps[i] = grideye.getPixelTemperature(i); 

    if (DEBUG_SERIAL)
    {
      Serial.print(temps[i]);
      Serial.print(" ");
      if ((i + 1) % 8 == 0) Serial.println();
    }
  }
  if (DEBUG_SERIAL) Serial.println();

  // --- Step 2: Statistical Processing ---
  for (uint8_t q = 0; q < 64; q++) 
  {
    delta[q] = abs(temps[q] - pixelLast[q]);
    sum += temps[q];
    sumOfSq += sq(temps[q]);
    pixelLast[q] = temps[q]; 
  }

  frameAvg = sum / 64.0;
  float sumSquaredDivN = sq(sum) / 64.0;
  frameStdDev = sumOfSq - sumSquaredDivN; 

  for (uint8_t i = 0; i < 64; i++)
  {
    deltaSum += delta[i];
  }
  float frameChangeAvg = deltaSum / 64.0;

  // Filters for Temp Stats
  int stdDevInt = (int)(frameStdDev * 100);       
  int changeAvgInt = (int)(frameChangeAvg * 100); 

  int emaStdDev = emaStdDevF.filter(stdDevInt);          
  int emaChangeAvg = emaChangeAvgF.filter(changeAvgInt); 

  smaStdDevF.push(stdDevInt);       
  smaChangeAvgF.push(changeAvgInt); 
  int smaStdDev = smaStdDevF.get(); 
  int smaChangeAvg = smaChangeAvgF.get();

  // --- Step 3: Accelerometer & Gravity Compensation ---
  lis.read();
  int rawX = lis.x;
  int rawY = lis.y;
  int rawZ = lis.z;

  int accNorm = 0;
  int axisData[3] = {rawX, rawY, rawZ};

  // Calculate Normalized Acceleration (Magnitude above gravity)
  for (int i = 0; i < 3; i++)
  {
    if (abs(axisData[i]) >= GRAV_COMP)
    {
      accNorm += abs(axisData[i]) - GRAV_COMP;
    }
  }
  
  // Filters for Accelerometer
  int emaNorm = accNormF.filter(accNorm); 
  int emaX = emaXF.filter(rawX);
  int emaY = emaYF.filter(rawY);
  int emaZ = emaZF.filter(rawZ);

  // --- Step 4: Time Tracking ---
  unsigned long unixNow = now.unixtime();
  if (unixNow != unixLast) subSecCounter = 0;
  else subSecCounter++;
  unixLast = unixNow;

  if (startTime == 0)
  {
    startTime = unixNow;
  }
  unsigned long duration = unixNow - startTime;

  // --- Step 5: Logging ---
  if (WRITE_TO_LOG) 
  {
    // Skip logging if within start-up delay window
    if (millis() < START_DELAY_MS)
    {
      if (DEBUG_SERIAL) Serial.println("Waiting for warm-up...");
      delay(LOG_RATE_MS);
      return; 
    }
    
    sleepLog = SD.open(currentLog, FILE_WRITE);
    if (sleepLog)
    {
      sleepLog.print(unixNow);          sleepLog.print(",");
      sleepLog.print(subSecCounter);    sleepLog.print(",");
      sleepLog.print(duration);         sleepLog.print(",");
      sleepLog.print(frameAvg);         sleepLog.print(",");
      sleepLog.print(frameStdDev);      sleepLog.print(",");
      sleepLog.print(smaStdDev);        sleepLog.print(",");
      sleepLog.print(emaStdDev);        sleepLog.print(",");
      sleepLog.print(frameChangeAvg);   sleepLog.print(",");
      sleepLog.print(smaChangeAvg);     sleepLog.print(",");
      sleepLog.print(emaChangeAvg);     sleepLog.print(",");
      sleepLog.print(rawX);             sleepLog.print(",");
      sleepLog.print(rawY);             sleepLog.print(",");
      sleepLog.print(rawZ);             sleepLog.print(",");
      sleepLog.print(emaX);             sleepLog.print(",");
      sleepLog.print(emaY);             sleepLog.print(",");
      sleepLog.print(emaZ);             sleepLog.print(",");
      sleepLog.print(emaNorm);          sleepLog.println();
      
      sleepLog.close();
    }
  }

  if (DEBUG_SERIAL)
  {
    Serial.print("Loop Time: ");
    Serial.print(millis() - startMillis);
    Serial.println("ms");
  }

  delay(LOG_RATE_MS);
}
