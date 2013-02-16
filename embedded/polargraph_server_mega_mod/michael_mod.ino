// MICHAEL IRELAND
const int pinResetSD = 22;
const int pinExecComm = 24;
const int pinExecHome = 28;
const int pinExecPen = 30;
const int pinExecPause = 18;
const int pinStatusReady = 23;
const int pinStatusBusy = 25;
const int intExecPen = 4;
const int intExecPause = 5;

int BTEnabled;
int currentResetSDState;
int currentExecCommState;
int currentExecHomeState;
int currentPenState;
volatile int pausePressed;
int pauseState;
int int_last_btn_pressed = 0;
int currentPauseState;

void Michael_Init()
{
  pinMode( pinResetSD, INPUT );
  pinMode( pinExecComm, INPUT );
  pinMode( pinExecHome, INPUT );
  pinMode( pinExecPen, INPUT );
  pinMode( pinExecPause, INPUT );
  pinMode( pinStatusReady, OUTPUT );
  pinMode( pinStatusBusy, OUTPUT );
  pinMode( 13, OUTPUT );
  
  currentResetSDState = 0;
  currentExecCommState = 0;
  currentExecHomeState = 0;
  currentPenState = 0;
  currentPauseState = false;
  pausePressed = false;
  pauseState = false;
  BTEnabled = 1;
  
  if( BTEnabled == 1 )
    Serial3.begin(57600);
  
  //attachInterrupt( intExecPause, Michael_ExecPause, RISING );
}

int Michael_ProcessInterrupts(void)
{
  pausePressed = (digitalRead( pinExecPause ) == HIGH);
  if( pausePressed == true && currentPauseState == false )
  {
    Michael_SerialPrintln( F("MI - PROCESSING PAUSE") );
    int btn_pressed = millis();
    if( true ) //btn_pressed > int_last_btn_pressed + 500 )
    {
      if( pauseState == true )
      {
        pauseState = false;
        Michael_SerialPrintln( F("MI - PAUSED") );
      }
      else
      {
        pauseState = true;
        Michael_SerialPrintln( F("MI - RESUME") );
      }
      
      //Michael_ProcessPenButton(); // Check for pen
      
      int_last_btn_pressed = btn_pressed;
    }
  }
  
  currentPauseState = pausePressed;
  
  Michael_ProcessPenButton(); // Check for pen
  
  return pauseState;
}

int Michael_IsPaused( void )
{
  return pauseState;
}

void Michael_SetStatusBusy( int CurrentState )
{
  if( CurrentState == 1 )
  {
    Michael_SerialPrintln( F("MI - BUSY") );
    digitalWrite( pinStatusBusy, HIGH );
    digitalWrite( pinStatusReady, LOW );
    digitalWrite( 13, LOW );
  }
  else
  {
    Michael_SerialPrintln( F("MI - READY") );
    digitalWrite( pinStatusBusy, LOW );
    digitalWrite( pinStatusReady, HIGH );
    digitalWrite( 13, HIGH );
  }
}

void Michael_ProcessPenButton( void )
{
  int ButtonState = 0;
      
  ButtonState = digitalRead( pinExecPen );
  if ( ButtonState == HIGH )
  {
    currentPenState = 1;
  }
  else
  {
    if ( currentPenState == 1 )
    {
      Michael_SerialPrintln( F("MI - Pen released") );
      currentPenState = 0;
      if (isPenUp == false)
      {
        Michael_SerialPrintln( F("MI - PEN UP") );
        penlift_penUp();
      }
      else
      {
        Michael_SerialPrintln( F("MI - PEN DOWN") );
        penlift_penDown();
      }
    }
  }
}

String Michael_Execute( String input )
{
    int ButtonState = 0;
    ButtonState = digitalRead( pinResetSD );
    if ( ButtonState == HIGH )
    {
      currentResetSDState = 1;
    }
    else
    {
      if ( currentResetSDState == 1 )
      {
        Michael_SerialPrintln( F("MI - ResetSD released") );
        currentResetSDState = 0;
        sd_initSD();
      }
    }
    
    ButtonState = 0;
    ButtonState = digitalRead( pinExecComm );
    if ( ButtonState == HIGH )
    {
      currentExecCommState = 1;
    }
    else
    {
      if ( currentExecCommState == 1 )
      {
        Michael_SerialPrintln( F("MI - Comm released") );
        currentExecCommState = 0;
        return "C34,comm.txt,END";
      }
    }
        
    ButtonState = 0;
    ButtonState = digitalRead( pinExecHome );
    if ( ButtonState == HIGH )
    {
      currentExecHomeState = 1;
    }
    else
    {
      if ( currentExecHomeState == 1 )
      {
        Michael_SerialPrintln( F("MI - Home released") );
        currentExecHomeState = 0;
        return "C34,home.txt,END";
      }
    }
    
    Michael_ProcessPenButton(); // Check for pen
    Michael_ProcessInterrupts();
    
    return input;
}

void Michael_ExecPause( void )
{
  pausePressed = true;
}

// Serial functions / Macros

#define __Michael_SerialPrint( input ) \
  Serial.print( input ); \
  if ( BTEnabled == 1 ) \
    Serial1.print( input ); \

#define __Michael_SerialPrintx2( input1, input2 ) \
  Serial.print( input1, input2 ); \
  if ( BTEnabled == 1 ) \
    Serial1.print( input1, input2 ); \


void Michael_SerialPrintln()
{
  Serial.println();
  if( BTEnabled == 1 )
    Serial1.println();
}

#define __Michael_SerialPrintln( input ) \
  Serial.println( input ); \
  if( BTEnabled == 1 ) \
    Serial3.println( input ); \

#define __Michael_SerialPrintlnx2( input1, input2 ) \
  Serial.println( input1, input2 ); \
  if( BTEnabled == 1 ) \
    Serial3.println( input1, input2 ); \


String Michael_SerialRead()
{
  // check if data has been sent from the computer:
  char inString[INLENGTH+1];
  int inCount = 0;
  
  if( Serial.available() > 0 )
  {
    while (Serial.available() > 0)
    {
      char ch = Serial.read();       // get it
      delay(15);
      inString[inCount] = ch;
      if (ch == INTERMINATOR)
      {
        Serial.flush();
        break;
      }
      inCount++;
    }
  }
  else if( (Serial3.available() > 0) && ( BTEnabled == 1 ) )
  {
    while (Serial3.available() > 0)
    {
      char ch = Serial3.read();       // get it
      delay(15);
      inString[inCount] = ch;
      if (ch == INTERMINATOR)
      {
        Serial3.flush();
        break;
      }
      inCount++;
    }
  }
  
  inString[inCount] = 0; // Terminate string
  
  String result = inString;
  return result;
}



// Handle specific data types

void Michael_SerialPrint( String input )
{
  __Michael_SerialPrint( input );
}
void Michael_SerialPrint( __FlashStringHelper* input )
{
  __Michael_SerialPrint( input );
}
void Michael_SerialPrint( const __FlashStringHelper* input )
{
  __Michael_SerialPrint( input );
}
void Michael_SerialPrint( byte input )
{
  __Michael_SerialPrint( input );
}
void Michael_SerialPrint( long input1, int input2 )
{
  __Michael_SerialPrintx2( input1, input2 );
}
void Michael_SerialPrint( long input )
{
  __Michael_SerialPrint( input );
}
void Michael_SerialPrint( int input )
{
  __Michael_SerialPrint( input );
}
void Michael_SerialPrint( float input )
{
  __Michael_SerialPrint( input );
}
void Michael_SerialPrint( uint32_t input )
{
  __Michael_SerialPrint( input );
} 


void Michael_SerialPrintln( String input )
{
  __Michael_SerialPrintln( input );
}
void Michael_SerialPrintln( __FlashStringHelper* input )
{
  __Michael_SerialPrintln( input );
}   
void Michael_SerialPrintln( const __FlashStringHelper* input )
{
  __Michael_SerialPrintln( input );
}   
void Michael_SerialPrintln( byte input )
{
  __Michael_SerialPrintln( input );
}   
void Michael_SerialPrintln( long input1, int input2 )
{
  __Michael_SerialPrintlnx2( input1, input2 );
}
void Michael_SerialPrintln( long input )
{
  __Michael_SerialPrintln( input );
}   
void Michael_SerialPrintln( int input )
{
  __Michael_SerialPrintln( input );
}   
void Michael_SerialPrintln( float input )
{
  __Michael_SerialPrintln( input );
}  
void Michael_SerialPrintln( uint32_t input )
{
  __Michael_SerialPrintln( input );
} 

