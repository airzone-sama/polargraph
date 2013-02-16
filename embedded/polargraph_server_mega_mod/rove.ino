/**
*  Polargraph Server for ATMEGA1280+ 
*  Written by Sandy Noble
*  Released under GNU License version 3.
*  http://www.polargraph.co.uk
*  http://code.google.com/p/polargraph/

Specific features for Polarshield / arduino mega.
Rove.

Commands that will set and modify the rove area, and the features that use
the rove area heavily.

*/

void rove_setRoveArea()
{
  rove1x = stepsPerMM * asLong(inParam1);
  rove1y = stepsPerMM * asLong(inParam2);
  roveWidth = stepsPerMM * asLong(inParam3);
  roveHeight = stepsPerMM * asLong(inParam4);
  
  if (rove1x > pageWidth)
    rove1x = pageWidth / 2;
  else if (rove1x < 1)
    rove1x = 1;
    
  if (rove1y > pageHeight)
    rove1y = pageHeight / 2;
  else if (rove1y < 1)
    rove1y = 1;
  
  if (roveWidth+rove1x > pageWidth)
    roveWidth = pageWidth - rove1x;
    
  if (roveHeight+rove1y > pageHeight)
    roveHeight = pageHeight - rove1y;
  
  useRoveArea = true;

  Michael_SerialPrintln("Set rove area (steps):");
  Michael_SerialPrint("X:");
  Michael_SerialPrint(rove1x);
  Michael_SerialPrint(",Y:");
  Michael_SerialPrint(rove1y);
  Michael_SerialPrint(",width:");
  Michael_SerialPrint(roveWidth);
  Michael_SerialPrint(",height:");
  Michael_SerialPrintln(roveHeight);

  Michael_SerialPrintln("Set rove area (mm):");
  Michael_SerialPrint("X:");
  Michael_SerialPrint(rove1x * mmPerStep);
  Michael_SerialPrint("mm, Y:");
  Michael_SerialPrint(rove1y * mmPerStep);
  Michael_SerialPrint("mm, width:");
  Michael_SerialPrint(roveWidth * mmPerStep);
  Michael_SerialPrint("mm, height:");
  Michael_SerialPrint(roveHeight * mmPerStep);
  Michael_SerialPrintln("mm.");

}
void rove_startText()
{
  if (useRoveArea)
  {
    long tA = multiplier(asLong(inParam1));
    long tB = multiplier(asLong(inParam2));
    
    if (rove_inRoveArea(tA, tB))
    {
      Michael_SerialPrintln("Target position is in rove area.");
      penlift_penUp();
      changeLength(tA, tB);
      textRowSize = multiplier(asInt(inParam3));
      textCharSize = textRowSize * 0.8;
      globalDrawDirection = asInt(inParam4);
      Michael_SerialPrintln("Text started.");
    }
    else
    {
      Michael_SerialPrint("Target position (");
      Michael_SerialPrint(tA);
      Michael_SerialPrint(",");
      Michael_SerialPrint(tB);
      Michael_SerialPrintln(") not in rove area.");
    }
  }
  else
  {
    Michael_SerialPrintln("Rove area must be defined to start text.");
  }
}

boolean rove_inRoveArea(float a, float b)
{
  // work out cartesian position of pen
  float cX = getCartesianXFP(a, b);
  float cY = getCartesianYFP(cX, a);
  
//  Serial.print("Input cX: ");
//  Serial.println(cX);
//  Serial.print("Input cY: ");
//  Serial.println(cY);
//  
//  Serial.print("Rove origin: ");
//  Serial.print(rove1x);
//  Serial.print(", ");
//  Serial.println(rove1y);
//
//  Serial.print("Rove size: ");
//  Serial.print(roveWidth);
//  Serial.print(", ");
//  Serial.println(roveHeight);
  
  if (cX < rove1x || cX > rove1x+roveWidth || cY < rove1y || cY > rove1y+roveHeight)
    return false;
  else
    return true;
}


/**
Method that works out where the next line should start, based on pen position, line
width and rove area
*/
boolean rove_moveToBeginningOfNextTextLine()
{
  Michael_SerialPrintln("Move to beginning of next line.");
  Michael_SerialPrint("Global draw direction is ");
  Michael_SerialPrintln(globalDrawDirection);
  long xIntersection;
  long yIntersection;
  boolean result = false;


  
  if (globalDrawDirection == DIR_SE) // 2
  {
    long nextLine = motorB.currentPosition() + textRowSize;
    Michael_SerialPrint("Next line:");
    Michael_SerialPrintln(nextLine);

    // greater than the far corner or less than the near corner
    if (sq(nextLine) > sq(rove1y+roveHeight) + sq(pageWidth-rove1x)
      || sq(nextLine) < sq(rove1y) + sq(pageWidth-(rove1x+roveWidth)))
    {
      Michael_SerialPrintln("No space for lines!");
      // no lines left!
    }
    else if (sq(nextLine) <= sq(rove1y) + sq(pageWidth-rove1x))
    {
      Michael_SerialPrintln("On the top edge.");
      // measure on the top edge of the rove area
      xIntersection = pageWidth-sqrt(sq(nextLine) - sq(rove1y));
      yIntersection = rove1y;

      Michael_SerialPrint("nextline:");
      Michael_SerialPrint(nextLine * mmPerStep);
      Michael_SerialPrint(",rove1x:");
      Michael_SerialPrint(rove1x * mmPerStep);
      Michael_SerialPrint(",rove1y:");
      Michael_SerialPrintln(rove1y * mmPerStep);

      result = true;
    }
    else
    {
      Michael_SerialPrintln("On the left edge.");
      // measure on the left edge of the rove area
      xIntersection = rove1x;
      yIntersection = sqrt(sq(nextLine) - sq(pageWidth - rove1x));
      result = true;
    }
  }
  else if (globalDrawDirection == DIR_NW) // 4
  {
  }
  else if (globalDrawDirection == DIR_SW) //3
  {
  }
  else //(drawDirection == DIR_NE) // default //1
  {
  }

  if (result)
  {
    long pA = getMachineA(xIntersection, yIntersection);
    long pB = getMachineB(xIntersection, yIntersection);
    changeLength(pA, pB);
  }
  
  return result;
}

/**
*  This is a good one - hoping to draw something like the
*  Norwegian Creations machine.  This uses a very short wavelength
*  and a relatively wide amplitude.  Using a wavelength this short
*  isn't practical for interactive use (too many commands)
*  so this first attempt will do it on-board.  In addition this 
*  should cut out an awful lot of the complexity involved in 
*  creating lists of commands, but will probably result in some
*  fairly dirty code.  Apologies in advance.
*/
void rove_drawNorwegianFromFile()
{
  if (useRoveArea)
  {
    // get parameters
    String filename = inParam1;
    int maxAmplitude = multiplier(asInt(inParam2));
    int wavelength = multiplier(asInt(inParam3));
    
    // Look up file and open it
    if (!sd_openPbm(filename))
    {
      Michael_SerialPrint("Couldn't open that file - ");
      Michael_SerialPrintln(filename);
      return;
    }
    else
    {
      Michael_SerialPrint("image size "); 
      Michael_SerialPrint(pbmWidth, DEC);
      Michael_SerialPrint(", ");
      Michael_SerialPrintln(pbmHeight, DEC);
      Michael_SerialPrint("(roveWidth:");
      Michael_SerialPrint(roveWidth);
      Michael_SerialPrintln(")");
      pbmScaling = float(roveWidth) / float(pbmWidth);
      Michael_SerialPrint("Scaling factor:");
      Michael_SerialPrintln(pbmScaling);
      Michael_SerialPrint("Rove width:");
      Michael_SerialPrintln(roveWidth);
      Michael_SerialPrint("Image offset:");
      Michael_SerialPrintln(pbmImageoffset);
    }
    
    // Pen up and move to start corner (top-right)
    penlift_penUp();
    // Move to top of first row:
    // x2 - amplitude

    // set roveHeight so that it is the same shape as the image.  
    roveHeight = roveWidth * pbmAspectRatio;
    long rove2x = rove1x + roveWidth;
    long rove2y = rove1y + roveHeight;
    
    // work out the distance from motor B to the closest corner of the rove area
    float row = getMachineB(rove2x,rove1y);
    
    // so the first row will be that value plus half of maxAmplitude
    row += (maxAmplitude / 2);
    
    changeLength(getMachineA(rove2x, rove1y), row);
    penlift_penDown();
    
    // and figure out where the arc with this radius intersects the top edge
    long xIntersection;
    long yIntersection;
    boolean finished = false;
  
    float tA = motorA.currentPosition();
    float tB = motorB.currentPosition();
    
  //  tA = getMachineA(rove2x, rove2y);
  //  tB = row;
    
    int pixels = 0;
    
    while (!finished)
    {
      while( Michael_ProcessInterrupts() == true )
      {
        delay( 100 );
      }
      if (!rove_inRoveArea(tA, tB))
      {
        Michael_SerialPrintln("Outside rove area. Making new line.");
        penlift_penUp();
        // increment row
        row += maxAmplitude;
        tB = row;
        
        // greater than the far corner or less than the near corner
        if (sq(row) > sq(rove2y) + sq(pageWidth-rove1x)
          || sq(row) < sq(rove1y) + sq(pageWidth-(rove2x)))
        {
          Michael_SerialPrintln("No space for rows!");
          // no lines left!
          finished = true;
        }
        else if (sq(row) <= sq(rove1y) + sq(pageWidth-rove1x))
        {
          Michael_SerialPrintln("On the top edge.");
          // measure on the top edge of the rove area
          xIntersection = pageWidth-sqrt(sq(row) - sq(rove1y));
          yIntersection = rove1y;
          
          Michael_SerialPrint("New row starts at (mm) x:");
          Michael_SerialPrint(rove1x * mmPerStep);
          Michael_SerialPrint(",Y:");
          Michael_SerialPrint(rove1y * mmPerStep);
      
          // move      
          tA = getMachineA(xIntersection, yIntersection);
          tB = getMachineB(xIntersection, yIntersection);
      
          finished = false;
        }
        else
        {
          Michael_SerialPrintln("On the left edge.");
          // measure on the left edge of the rove area
          xIntersection = rove1x;
          yIntersection = sqrt(sq(row) - sq(pageWidth - rove1x));
  
          Michael_SerialPrint("New row starts at (mm) x:");
          Michael_SerialPrint(rove1x * mmPerStep);
          Michael_SerialPrint(",Y:");
          Michael_SerialPrint(rove1y * mmPerStep);
    
          // move      
          tA = getMachineA(xIntersection, yIntersection);
          tB = getMachineB(xIntersection, yIntersection);
    
          finished = false;
        }      
  
        delay(1000);
      }
      else
      {
        Michael_SerialPrintln("In area.");
      }
      
      if (!finished)
      {
        changeLength(tA,tB);
        penlift_penDown();
        pixels++;
        Michael_SerialPrint("Pixel ");
        Michael_SerialPrintln(pixels);
        // draw pixel
        // Measure cartesian position at that point.
        float cX = getCartesianXFP(tA, tB);
        float cY = getCartesianYFP(cX, tA);
        
        cX -= rove1x;
        cY -= rove1y;
        
        Michael_SerialPrint("Drawing pixel on page at x:");
        Michael_SerialPrint(cX); //* mmPerStep);
        Michael_SerialPrint(", y:");
        Michael_SerialPrintln(cY);// * mmPerStep);
        // Scale down to cartesian position in bitmap
        cX = cX / pbmScaling;
        cY = cY / pbmScaling;

        Michael_SerialPrint("Drawing pixel from file at pixel x:");
        Michael_SerialPrint(cX);
        Michael_SerialPrint(", y:");
        Michael_SerialPrintln(cY);
        
        if (int(cY) > pbmHeight || int(cX) > pbmWidth)
        {
          Michael_SerialPrintln("Out of pixels. Cancelling");
          finished = true;
        }
        else
        {
          // Get pixel brightness at that position
          byte brightness = sd_getBrightnessAtPixel(cX, cY);
          
          if (brightness < 0)
          {
            Michael_SerialPrintln("No brightness value found. Cancelling.");
            finished = true;
          }
          else
          {
            // Scale pixel amplitude to be in range 0 to <maxAmplitude>,
            // where brightest = 0 and darkest = <maxAmplitude>
            byte amplitude = brightness;
            amplitude = pixel_scaleDensity(amplitude, pbmDepth, maxAmplitude);
  
            // Draw the wave:
            float halfWavelength = float(wavelength) / 2.0;
            float halfAmplitude = float(amplitude) / 2.0;
            changeLength(tA+halfWavelength, tB-halfAmplitude);
            changeLength(tA+halfWavelength, tB+halfAmplitude);
            changeLength(tA+wavelength, tB);
            tA += wavelength;
//            changeLength(tA, tB);
          }
        }
      }
      else
      {
        Michael_SerialPrintln("Finished!!");
        // finished
      }
    }
    penlift_penUp();
  }
  else
  {
    Michael_SerialPrintln("Rove area must be chosen for this operation.");
  }
}

void rove_drawRoveAreaFittedToImage()
{
  if (useRoveArea)
  {
    // get parameters
    String filename = inParam1;

    // Look up file and open it
    if (!sd_openPbm(filename))
    {
      Michael_SerialPrint("Couldn't open that file - ");
      Michael_SerialPrintln(filename);
      return;
    }
    else
    {
      Michael_SerialPrint("image size "); 
      Michael_SerialPrint(pbmWidth, DEC);
      Michael_SerialPrint(", ");
      Michael_SerialPrintln(pbmHeight, DEC);
      Michael_SerialPrint("(roveWidth:");
      Michael_SerialPrint(roveWidth);
      Michael_SerialPrintln(")");
      pbmScaling = roveWidth / pbmWidth;
      Michael_SerialPrint("Scaling factor:");
      Michael_SerialPrintln(pbmScaling);
      Michael_SerialPrint("Rove width:");
      Michael_SerialPrintln(roveWidth);
      Michael_SerialPrint("Image offset:");
      Michael_SerialPrintln(pbmImageoffset);
    }
    
    // set roveHeight so that it is the same shape as the image.  
    roveHeight = roveWidth * pbmAspectRatio;
    long rove2x = rove1x + roveWidth;
    long rove2y = rove1y + roveHeight;

    Michael_SerialPrint("rove2x:");
    Michael_SerialPrint(rove2x);
    Michael_SerialPrint("rove2y:");
    Michael_SerialPrintln(rove2y);

    
    // go to first point, top-left
//    Serial.println("Point 1.");
    float mA = motorA.currentPosition();
    float mB = motorB.currentPosition();
    float tA = getMachineA(rove1x, rove1y);
    float tB = getMachineB(rove1x, rove1y);
//    Serial.print("now a:");
//    Serial.print(mA);
//    Serial.print(",b:");
//    Serial.print(mB);
//    Serial.print(", target a:");
//    Serial.print(tA);
//    Serial.print(", b:");
//    Serial.print(tB);
    exec_drawBetweenPoints(mA, mB, tA, tB, 20);
        
//    Serial.println("Point 2.");
    mA = motorA.currentPosition();
    mB = motorB.currentPosition();
    tA = getMachineA(rove2x, rove1y);
    tB = getMachineB(rove2x, rove1y);
//    Serial.print("now a:");
//    Serial.print(mA);
//    Serial.print(",b:");
//    Serial.print(mB);
//    Serial.print(", target a:");
//    Serial.print(tA);
//    Serial.print(", b:");
//    Serial.println(tB);
    exec_drawBetweenPoints(mA, mB, tA, tB, 20);

//    Serial.println("Point 3.");
    mA = motorA.currentPosition();
    mB = motorB.currentPosition();
    tA = getMachineA(rove2x, rove2y);
    tB = getMachineB(rove2x, rove2y);
//    Serial.print("now a:");
//    Serial.print(mA);
//    Serial.print(",b:");
//    Serial.print(mB);
//    Serial.print(", target a:");
//    Serial.print(tA);
//    Serial.print(", b:");
//    Serial.println(tB);
    exec_drawBetweenPoints(mA, mB, tA, tB, 20);

//    Serial.println("Point 4.");
    mA = motorA.currentPosition();
    mB = motorB.currentPosition();
    tA = getMachineA(rove1x, rove2y);
    tB = getMachineB(rove1x, rove2y);
//    Serial.print("now a:");
//    Serial.print(mA);
//    Serial.print(",b:");
//    Serial.print(mB);
//    Serial.print(", target a:");
//    Serial.print(tA);
//    Serial.print(", b:");
//    Serial.println(tB);
    exec_drawBetweenPoints(mA, mB, tA, tB, 20);

//    Serial.println("Point 5.");
    mA = motorA.currentPosition();
    mB = motorB.currentPosition();
    tA = getMachineA(rove1x, rove1y);
    tB = getMachineB(rove1x, rove1y);
//    Serial.print("now a:");
//    Serial.print(mA);
//    Serial.print(",b:");
//    Serial.print(mB);
//    Serial.print(", target a:");
//    Serial.print(tA);
//    Serial.print(", b:");
//    Serial.println(tB);
    exec_drawBetweenPoints(mA, mB, tA, tB, 20);
    exec_drawBetweenPoints(float(motorA.currentPosition()), float(motorB.currentPosition()), getMachineA(rove1x, rove1y), getMachineB(rove1x, rove1y), 20);
//    Serial.println("Done.");
    
  }
  else
  {
    Michael_SerialPrintln("Rove area must be chosen for this operation.");
  }
}

/**
*  This moves to a random positions inside the rove area.
*/
void  rove_moveToRandomPositionInRoveArea()
{
  long x = random(rove1x, rove1x+roveWidth);
  long y = random(rove1y, rove1y+roveHeight);
  float a = getMachineA(x,y);
  float b = getMachineB(x,y);
  
  penlift_penUp();
  changeLength(a,b);
}

void rove_swirl()
{
  motorA.run();
  motorB.run();
  
  if (motorA.distanceToGo() == 0)
  {
    long x = random(rove1x, rove1x+roveWidth);
    long y = random(rove1y, rove1y+roveHeight);
    float a = getMachineA(x,y);
    motorA.moveTo(a);
  }

  if (motorB.distanceToGo() == 0)
  {
    long x = random(rove1x, rove1x+roveWidth);
    long y = random(rove1y, rove1y+roveHeight);
    float b = getMachineB(x,y);
    motorB.moveTo(b);
  }
}

void rove_controlSwirling()
{
  if (asInt(inParam1) == 0)
  {
    swirling = false;
  }
  else 
  {
    if (useRoveArea)
    {
      swirling = true;
    }
    else
    {
      Michael_SerialPrintln("Rove area must be defined to swirl.");
    }
  }
}



