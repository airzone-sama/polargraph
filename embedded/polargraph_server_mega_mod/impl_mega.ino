/**
*  Polargraph Server for ATMEGA1280+ 
*  Written by Sandy Noble
*  Released under GNU License version 3.
*  http://www.polargraph.co.uk
*  http://code.google.com/p/polargraph/

Specific features for Polarshield / arduino mega.
Implementation.

So this file is the interface for the extra features available in the 
mega/polarshield version of the polargraph.

*/

/*  Implementation of executeCommand for MEGA-sized boards that
have command storage features. */
void impl_processCommand(String com)
{
  //lcd_echoLastCommandToDisplay(com);
  
  // check for change mode commands
  if (com.startsWith(CMD_MODE_STORE_COMMANDS)
  || com.startsWith(CMD_MODE_LIVE))
  {
    Michael_SerialPrintln("Changing mode.");
    impl_executeCommand(com);
  }
  // else execute / store the command
  else if (storeCommands)
  {
    Michael_SerialPrint(F("Storing command:"));
    Michael_SerialPrintln(com);
    sd_storeCommand(com);
  }
  else
  {
    impl_executeCommand(com);
  }
}

/**  
*  This includes the extra commands the MEGA is capable of handling.
*  It tries to run the command using the core executeBasicCommand
*  first, but if that doesn't work, then it will go through
*  it's own decision tree to try and run one of the additional
*  routines.
*/
void impl_executeCommand(String &com)
{
  Michael_SetStatusBusy( 1 );
  if (exec_executeBasicCommand(com))
  {
    // that's nice, it worked
    Michael_SerialPrintln("Executed basic.");
  }
  else if (com.startsWith(CMD_DRAWCIRCLEPIXEL))
    impl_pixel_drawCircularPixel();
//  else if (com.startsWith(CMD_TESTPATTERN))
//    testPattern();
  else if (com.startsWith(CMD_TESTPENWIDTHSCRIBBLE))
    impl_pixel_testPenWidthScribble();
  else if (com.startsWith(CMD_DRAWDIRECTIONTEST))
    impl_exec_drawTestDirectionSquare();
  else if (com.startsWith(CMD_MODE_STORE_COMMANDS))
    impl_exec_changeToStoreCommandMode();
  else if (com.startsWith(CMD_MODE_LIVE))
    impl_exec_changeToLiveCommandMode();
  else if (com.startsWith(CMD_MODE_EXEC_FROM_STORE))
    impl_exec_execFromStore();
  else if (com.startsWith(CMD_RANDOM_DRAW))
    drawRandom();
  else if (com.startsWith(CMD_SET_ROVE_AREA))
    rove_setRoveArea();
  else if (com.startsWith(CMD_START_TEXT))
    rove_startText();
  else if (com.startsWith(CMD_DRAW_SPRITE))
    sprite_drawSprite();
  else if (com.startsWith(CMD_DRAW_RANDOM_SPRITE))
    sprite_drawRandomPositionedSprite();
  else if (com.startsWith(CMD_CHANGELENGTH_RELATIVE))
    exec_changeLength();
  else if (com.startsWith(CMD_SWIRLING))
    rove_controlSwirling();
  else if (com.startsWith(CMD_DRAW_NORWEGIAN))
    rove_drawNorwegianFromFile();
  else if (com.startsWith(CMD_DRAW_NORWEGIAN_OUTLINE))
    rove_drawRoveAreaFittedToImage();
  else
  {
    comms_unrecognisedCommand(com);
    comms_ready();
  }
  Michael_SetStatusBusy( 0 );
}

/** Polarshield implementation of runBackgroundProcesses. This is basically stuff to do with 
the screen.
*/
void impl_runBackgroundProcesses()
{
  //lcd_checkForInput();
  //lcd_updateDisplay();
      
  long motorCutoffTime = millis() - lastOperationTime;
  if ((automaticPowerDown) && (powerIsOn) && (motorCutoffTime > motorIdleTimeBeforePowerDown))
  {
    Michael_SerialPrintln("Powering down because of inactivity.");
    //lcd_runEndScript();
    //lcd_updateDisplay();
    impl_releaseMotors();
  }
  
  if (swirling)
    rove_swirl();
}

void impl_loadMachineSpecFromEeprom()
{
}

void impl_exec_execFromStore()
{
  String fileToExec = inParam1;
  if (fileToExec != "")
  {
    currentlyDrawingFromFile = true;
    Michael_SerialPrint("Filename to read from: ");
    Michael_SerialPrintln(fileToExec);
    commandFilename = fileToExec;
    impl_exec_execFromStore(commandFilename);
    currentlyDrawingFromFile = true;
  }
  else
  {
    Michael_SerialPrintln("No filename supplied to read from.");
  }
  
}

void impl_exec_execFromStore(String inFilename)
{
  if (inFilename != "")
  {
    String noBlanks = "";
    // remove blanks
    for (int i = 0; i<inFilename.length(); i++)
    {
      if (inFilename[i] != ' ')
        noBlanks = noBlanks + inFilename[i];
    }
    
    char filename[noBlanks.length()+1];
    noBlanks.toCharArray(filename, noBlanks.length()+1);
//    Serial.print("Array to read from: ");
//    Serial.println(filename);
    File readFile = SD.open(filename, FILE_READ);
    if (readFile)
    {
      Michael_SerialPrint("Opened file:");
      Michael_SerialPrintln(noBlanks);
      String command = "";
      while (readFile.available() && currentlyDrawingFromFile)
      {
//        Serial.println("Reading...");
        // poll for input
        char ch = readFile.read();
//        Serial.print(".");
//        Serial.print(ch);
//        Serial.print("-");
        if (ch == 13 || ch == 10)
        {
//          Serial.println("New line");
          // execute the line
          command.trim();
          
          while( Michael_ProcessInterrupts() == true )
          {
            delay( 100 );
          }
          
          boolean commandParsed = comms_parseCommand(command);
          if (commandParsed)
          {
//            Serial.println("Stored command parsed.");
            Michael_SerialPrint(F("Executing command:"));
            Michael_SerialPrintln(command);
            impl_executeCommand(command);
          }
//          else
//            Serial.println("Stored command WAS NOT parsed.");
          command = "";
          //lcd_checkForInput();
        }
        else
          command += ch;
//        Serial.print("Command building:");
//        Serial.println(command);
      }
      Michael_SerialPrintln("Finished with the file.");
      currentlyDrawingFromFile = false;
      readFile.close();
    }
    else
    {
      Michael_SerialPrintln("Couldn't find that file, btw.");
      currentlyDrawingFromFile = false;
    }
  }
  else
  {
    Michael_SerialPrintln("No filename supplied to read from.");
    currentlyDrawingFromFile = false;
  }
}

void impl_exec_changeToStoreCommandMode()
{
  String newfilename = inParam1;
  String newFile = inParam2;
  if (newfilename != "")
  {
    Michael_SerialPrint("Filename for command store: ");
    Michael_SerialPrintln(newfilename);
    storeCommands = true;
    commandFilename = newfilename;
    if (newFile.equals("R"))
    {
      // delete file if it exists
      char filename[newfilename.length()+1];
      newfilename.toCharArray(filename, newfilename.length()+1);
      
      if (SD.exists(filename))
      {
        // file exists
        Michael_SerialPrintln(F("File already exists."));
        boolean removed = SD.remove(filename);
        if (removed)
          Michael_SerialPrintln(F("File removed."));
        
      }
    }
  }
  else
  {
    Michael_SerialPrintln("No filename supplied to write to.");
  }
}

void impl_exec_changeToLiveCommandMode()
{
  Michael_SerialPrintln(F("Changing back to live mode."));
  storeCommands = false;
}

void impl_pixel_drawCircularPixel() 
{
    long originA = multiplier(asLong(inParam1));
    long originB = multiplier(asLong(inParam2));
    int size = multiplier(asInt(inParam3));
    int density = asInt(inParam4);
     Michael_SerialPrint("Density before: ");
     Michael_SerialPrintln(density);
     
    density = pixel_scaleDensity(density, 255, 8);
    Michael_SerialPrint("Density scaled");
    Michael_SerialPrintln(density);
    
    int radius = size / 2;
    int increment = radius / 8;

    if (density > 0)
    {
      Michael_SerialPrint("Density:");
      Michael_SerialPrintln(density);
      impl_drawSpiral(originA, originB, radius, increment, density);
    }
    
    outputAvailableMemory(); 
}

void impl_pixel_testPenWidthScribble()
{
  int rowWidth = multiplier(asInt(inParam1));
  float startWidth = asFloat(inParam2);
  float endWidth = asFloat(inParam3); 
  float incSize = asFloat(inParam4);
  
  boolean ltr = true;
  
  float oldPenWidth = penWidth;
  int iterations = 0;
  
  int posA = motorA.currentPosition();
  int posB = motorB.currentPosition();

  int startColumn = posA;
  int startRow = posB;
  
  for (float pw = startWidth; pw <= endWidth; pw+=incSize)
  {
    iterations++;
    int column = posA;
    
    penWidth = pw;
    int maxDens = pixel_maxDensity(penWidth, rowWidth);
    Michael_SerialPrint(F("Penwidth test "));
    Michael_SerialPrint(iterations);
    Michael_SerialPrint(F(", pen width: "));
    Michael_SerialPrint(penWidth);
    Michael_SerialPrint(F(", max density: "));
    Michael_SerialPrintln(maxDens);
    
    for (int density = maxDens; density >= 0; density--)
    {
      pixel_drawScribblePixel(posA, posB, rowWidth, density);
      posB+=rowWidth;
    }
    
    posA+=rowWidth;
    posB = startRow;
  }
  
  changeLength(long(posA-(rowWidth/2)), long(startRow-(rowWidth/2)));

  penWidth = oldPenWidth;
  
  moveB(0-rowWidth);
  for (int i = 1; i <= iterations; i++)
  {
    moveB(0-(rowWidth/2));
    moveA(0-rowWidth);
    moveB(rowWidth/2);
  }
  
  penWidth = oldPenWidth;
}    

void impl_engageMotors()
{
  motorA.enableOutputs();
  motorB.enableOutputs();
  powerIsOn = true;
  motorA.runToNewPosition(motorA.currentPosition()+32);
  motorB.runToNewPosition(motorB.currentPosition()+32);
  motorA.runToNewPosition(motorA.currentPosition()-32);
  motorB.runToNewPosition(motorB.currentPosition()-32);
  Michael_SerialPrintln("Engaged motors.");
}

void impl_releaseMotors()
{
  motorA.disableOutputs();
  motorB.disableOutputs();
  powerIsOn = false;
  Michael_SerialPrintln("Released motors");
}


void drawRandom()
{
  for (int i = 0; i < 1000; i++)
  {
    Michael_SerialPrint("Drawing:");
    Michael_SerialPrintln(i);
    while (motorA.distanceToGo() != 0 && motorB.distanceToGo() != 0)
    {
      motorA.run();
      motorB.run();
    }

    if (motorA.distanceToGo() == 0)
    {
      int r = random(-2,3);
      motorA.move(r);

      Michael_SerialPrint("Chosen new A target: ");
      Michael_SerialPrintln(r);
    }

    if (motorB.distanceToGo() == 0)
    {
      int r = random(-2,3);
      motorB.move(r);
      Michael_SerialPrint("Chosen new B target: ");
      Michael_SerialPrintln(r);
    }
    
    reportPosition();
  }
}




/* AS220 Drawbot */
float rads(int n) {
  // Return an angle in radians
  return (n/180.0 * PI);
}    

void impl_drawCurve(long x, long y, long fx, long fy, long cx, long cy) {
  // Draw a Quadratic Bezier curve from (x, y) to (fx, fy) using control pt
  // (cx, cy)
  float xt=0;
  float yt=0;

  for (float t=0; t<=1; t+=.0025) {
    xt = pow((1-t),2) *x + 2*t*(1-t)*cx+ pow(t,2)*fx;
    yt = pow((1-t),2) *y + 2*t*(1-t)*cy+ pow(t,2)*fy;
    changeLength(xt, yt);
  }  
}
                                                     

void impl_drawCircle(long centerx, long centery, int radius) {
  // Estimate a circle using 20 arc Bezier curve segments
  int segments =20;
  int angle1 = 0;
  int midpoint=0;
   
//  changeLength(centerx+radius, centery);

  for (float angle2=360/segments; angle2<=360; angle2+=360/segments) {

    midpoint = angle1+(angle2-angle1)/2;

    float startx=centerx+radius*cos(rads(angle1));
    float starty=centery+radius*sin(rads(angle1));
    float endx=centerx+radius*cos(rads(angle2));
    float endy=centery+radius*sin(rads(angle2));
    
    int t1 = rads(angle1)*1000 ;
    int t2 = rads(angle2)*1000;
    int t3 = angle1;
    int t4 = angle2;

    impl_drawCurve(startx,starty,endx,endy,
              centerx+2*(radius*cos(rads(midpoint))-.25*(radius*cos(rads(angle1)))-.25*(radius*cos(rads(angle2)))),
              centery+2*(radius*sin(rads(midpoint))-.25*(radius*sin(rads(angle1)))-.25*(radius*sin(rads(angle2))))
    );
    
    angle1=angle2;
  }
}

void impl_drawSpiral(long centerx, long centery, int maxRadius, int increment, int density) 
{
  Michael_SerialPrintln("Draw spiral.");
  Michael_SerialPrint("Max radius: ");
  Michael_SerialPrintln(maxRadius);
  Michael_SerialPrint("Increment:");
  Michael_SerialPrintln(increment);
  Michael_SerialPrint("densitY:");
  Michael_SerialPrintln(density);
  // Estimate a circle using 20 arc Bezier curve segments
  int segments = 20;

  
  float radius = float(increment);
  // work out how many shells to draw
  for (int turns = 0; turns < density; turns++)
  {
    if (increment < 80)
      segments = radius / 4;
    if (segments < 4)
      segments == 4;

    float anglePerSegment = 360/segments;
    
    float radiusIncrementPerSegment = float(increment) / float(segments);
  
    int angle1 = 0;
    int midpoint=0;
    boolean firstMove = true;
    for (float angle2=anglePerSegment; angle2<=360; angle2+=anglePerSegment) 
    {
      midpoint = angle1+(angle2-angle1)/2;
  
      float startx=centerx+radius*cos(rads(angle1));
      float starty=centery+radius*sin(rads(angle1));
      float endx=centerx+radius*cos(rads(angle2));
      float endy=centery+radius*sin(rads(angle2));
      
      int t1 = rads(angle1)*1000 ;
      int t2 = rads(angle2)*1000;
      int t3 = angle1;
      int t4 = angle2;
  
      if (firstMove)
      {
        changeLength(startx, starty);
        firstMove = false;
      }
      usingAcceleration = false;
      impl_drawCurve(startx,starty,endx,endy,
                centerx+2*(radius*cos(rads(midpoint))-.25*(radius*cos(rads(angle1)))-.25*(radius*cos(rads(angle2)))),
                centery+2*(radius*sin(rads(midpoint))-.25*(radius*sin(rads(angle1)))-.25*(radius*sin(rads(angle2))))
      );
      usingAcceleration = true;
      angle1=angle2;
      radius += radiusIncrementPerSegment;
    }
  }
}

void impl_exec_drawTestDirectionSquare()
{
  int rowWidth = multiplier(asInt(inParam1));
  int segments = asInt(inParam2);
  pixel_drawSquarePixel(rowWidth, rowWidth, segments, DIR_SE);
  moveA(rowWidth*2);
  
  pixel_drawSquarePixel(rowWidth, rowWidth, segments, DIR_SW);
  moveB(rowWidth*2);
  
  pixel_drawSquarePixel(rowWidth, rowWidth, segments, DIR_NW);
  moveA(0-(rowWidth*2));
  
  pixel_drawSquarePixel(rowWidth, rowWidth, segments, DIR_NE);
  moveB(0-(rowWidth*2));
  
}


