import processing.serial.*; //<>// //<>// //<>// //<>// //<>// //<>// //<>//

String myString = null;
Serial myPort;  // The serial port
int comPort = 1;
float [] temps = new float [64];
Table thermTable;    // Table for outputting GridEye sensor readings
Table accTable;    // Table for outputting accelerometer data 

//done
float frameMin;    //lowest temp in frame
float frameMax;    //highest temp in frame
float frameAvg;    //avg temp of frame
float frameStdDev;    //Std. Dev. of frame Σ(xi2)-(Σ xi)2/n Sum of square shortcut
// int hottestPixel;    //XY of hottest pixel in frame

void setup() {
  size(400, 400);  // Size must be the first statement
  noStroke();
  frameRate(30);

  printArray(Serial.list());     // Print a list of connected serial devices in the console
  myPort = new Serial(this, Serial.list()[comPort], 115200);    //Active comport changed here
  myPort.clear();    // Throw out the first chunk in case we caught it in the middle of a frame
  myString = myPort.readStringUntil(13);    //Read until carriage return
  myString = null;
  colorMode(HSB, 360, 100, 100);    // change to HSB color mode, this will make it easier to color code the temperature data

  /* Setup table for thermopile data */
  thermTable = new Table();
  thermTable.addColumn("Timestamp");
  thermTable.addColumn("Avg. Temp");
  thermTable.addColumn("Std. Dev.");
  thermTable.addColumn("Min. Temp");
  thermTable.addColumn("Max Temp");

  /* Setup table for accelerometer data */
  accTable = new Table();
  accTable.addColumn("Timestamp");
  accTable.addColumn("X Axis");
  accTable.addColumn("Y Axis");
  accTable.addColumn("Z Axis");
}

void draw() { 

  float sum = 0.0;    //(Σ xi)^2 Sum of all temps in frame for Std. Dev. calculation
  float sumQ = 0.0;    // Σ(xi2)  Sum of square of all temps in frame for Std. Dev. calculation


  // When there is a sizeable amount of data on the serial port
  // read everything up to the first linefeed
  if (myPort.available() > 64) {
    myString = myPort.readStringUntil(13);

    // generate an array of strings that contains each of the comma
    // separated values

    if (myString != null) {
      myString = trim(myString);
      String splitString[] = splitTokens(myString, ",");
      //printArray(splitString);

      /* Once String is full of temp data, start splitting processing */
      if (splitString.length == 64) {    
        for (int q = 0; q < 64; q++) {    
          frameAvg += float(splitString[q]);
          sum += float(splitString[q]);    // (Σ xi)^2 Sum of all temps in frame
          sumQ += sq(float(splitString[q]));    // Σ(xi^2)  Sum of square of all temps in frame
          println("Temp", q, "=", splitString[q]);



          if (q == 63) {
            println("sum = ", sum);
            sum = sq(sum);    // Square the sum of all temps
            println("sum^2 = ", sum);
            println("sumQ = ", sumQ);
            sum /= 64;
            println("sum / q =", sum);
            frameStdDev = sumQ - sum;    // Finish running Std. Dev.
            //println("Σ(xi2) - (Σ xi)2", frameStdDev);            
            println("Std. Dev. = ", frameStdDev);
          }

          temps[q] = map(float(splitString[q]), 33, 39, 100, 360);    //temps get mapped into color scaled values at this point
        }
        frameMin = min(float(splitString));    // Find min temp in frame array
        frameMax = max(float(splitString));    // Find max temp in frame array
      }
      frameAvg /= float(splitString.length);    //Average temp of frame is recorded
    }
  }


  /*Prepare variables needed to draw our heatmap*/
  int x = 0;
  int y = 0;
  int i = 0;
  background(0);   // Clear the screen with a black background


  /* each GridEYE pixel will be represented by a 50px square: 
   because 50 x 8 = 400, we draw squares until our y location
   is 400 */

  while (y < 400) {


    // for each increment in the y direction, draw 8 boxes in the 
    // x direction, creating a 64 pixel matrix
    while (x < 400) {
      // before drawing each pixel, set our paintcan color to the 
      // appropriate mapped color value
      fill(temps[i], 100, 100);
      rect(x, y, 50, 50);
      x = x + 50;
      i++;
    }

    y = y + 50;
    x = 0;
  }

  filter(BLUR, 10);    // Add a gaussian blur to the canvas in order to create a rough visual interpolation between pixels.

  /* time stamp variables that will get replaced by RTC */
  int ms = millis();
  int s = second();
  int m = minute();
  int h = hour();
  String time = h + ":" + m + ":" + s + ":" + ms;



  /* Write statistics for thermal to table and save */
  TableRow thermRow = thermTable.addRow();
  thermRow.setString("Timestamp", time);    //Finish when you get RTC all figured out
  thermRow.setFloat("Avg. Temp", frameAvg);    // Record average of frame in table 
  thermRow.setFloat("Std. Dev.", frameStdDev);    // Record standard deviation of frame in table
  thermRow.setFloat("Min. Temp", frameMin);    // Record min temp in frame
  thermRow.setFloat("Max Temp", frameMax);    // Record max temp in frame
  saveTable(thermTable, "Thermal.csv");   //Commented for debugging. Consider putting in global boolean controlled if

  /* Write values for accelerometer to table and save */

  /* 
   Finish when you get accelerometer working
   TableRow accRow = accTable.addRow();
   accRow.setFloat("X Axis");
   accRow.setFloat("Y Axis");
   accRow.setFloat("Z Axis"); 
   */
} 
