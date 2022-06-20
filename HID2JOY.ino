//HID2JOY - Xbox One Controller (and other USB) to Analog IBM PC Gameport Adapter
//by Zuofu, requires USB Host Shield 2.0 Library
//Example sketch for the Xbox ONE USB library - by guruthree, based on work by
//Kristian Lauszus.

#include <XBOXONE.h>
#include <EEPROM.h>
#include <Wire.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

#define JOY1_X         0
#define JOY1_Y         1
#define JOY2_X         2
#define JOY2_Y         3

#define DUAL_STICK     0
#define THRUSTMASTER   1
#define CH_FLIGHTSTICK 2

#define THROTTLE_SLEW  512 //larger number means slower throttle slew 

#define DPOT_I2C_ADDR  0x2E
#define MODE_ROM_ADDR  0x00

USB Usb;
XBOXONE Xbox(&Usb);

const byte RegAddr[] = {0x00, 0x10, 0x60, 0x70};
byte mode;

void setAnalogJoy(int value, byte channel)
{ 
  Wire.beginTransmission(DPOT_I2C_ADDR);
  Wire.write(RegAddr[channel]);
  Wire.write(byte((value/256) + 128));
  Wire.endTransmission();
}

//pulse the controller rumble, useful for devices such as Xbox One which don't have programmable LEDs
void controllerPulse (byte times)
{
  for (byte i = 0; i < times; i++)
  {
    Xbox.setRumbleOn(128, 128, 64, 64);
    delay(50);
    Xbox.setRumbleOff(); 
    delay(50);
  }
}

void indicateMode (byte mode)
{
  //This can be made fancier later with (e.g.) colors on the PS4/PS5 controller
  
  switch (mode)
  {
    case DUAL_STICK:
      Serial.print(F("\r\nDual Stick Mode"));
      break;
    case THRUSTMASTER:
      Serial.print(F("\r\nThrustmaster Mode"));
      break;
    case CH_FLIGHTSTICK:
      Serial.print(F("\r\nCH Flightstick Mode"));
      break;
  }
  controllerPulse (mode + 1);
}

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1); //halt
  }
  Serial.print(F("\r\nXBOX ONE USB Library Started"));

  Wire.begin(); //Initialize I2C
  Wire.setClock(400000);
  
  pinMode(2, OUTPUT); //Configure GPIOs
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite (2, LOW);
  digitalWrite (3, LOW);
  digitalWrite (4, LOW);
  digitalWrite (5, LOW);

  mode = EEPROM.read(MODE_ROM_ADDR);
  if (mode > CH_FLIGHTSTICK) //if not a valid mode
  {
    mode = DUAL_STICK;
    EEPROM.write(MODE_ROM_ADDR, mode); //write a default mode
  }
  indicateMode (mode);
}

void loop() 
{
  Usb.Task();
  
  if (Xbox.XboxOneConnected)  //Don't do anything unless Xbox controller is connected
  {
    static unsigned long modeDownTime;
    static byte          modeDownPrev = 0;
    static signed long   throttleValue = 0;
    unsigned int         X2_val;
    byte                 buttonState[6] = {0, 0, 0, 0, 0, 0}; //holds button state to set in a single place, assume all are off for now

    if (Xbox.getButtonPress(VIEW) && !modeDownPrev) //Xbox button pressed down
    {
      modeDownTime = millis(); //record time & set down flag
      modeDownPrev = 1;
    }
    else if (!Xbox.getButtonPress(VIEW) && modeDownPrev) //button was previously pressed but now released
    {  
      modeDownPrev = 0;
    }
    else if (Xbox.getButtonPress(VIEW) && (millis() - modeDownTime > 3000) ) //Xbox button pressed for > 3 seconds
    {
      modeDownPrev = 0;
      
      //modify the mode
      if (mode < CH_FLIGHTSTICK)
      mode ++;
      else
      mode = DUAL_STICK;
      
      indicateMode(mode);
      EEPROM.write(MODE_ROM_ADDR, mode); //write mode to EEPROM
    }

    //Do some common button mapping
    buttonState[2] = Xbox.getButtonPress(A);
    buttonState[3] = Xbox.getButtonPress(B);
    buttonState[4] = Xbox.getButtonPress(X);
    buttonState[5] = Xbox.getButtonPress(Y);

    //Allow LB and RB to also turn on buttons, but not turn off (they will automatically go off) 
    if (Xbox.getButtonPress(RB))
      buttonState[2] = 1;
    if (Xbox.getButtonPress(LB))
      buttonState[3] = 1;
      
    //Handle mode specific behavior
    switch (mode)
    {
      case DUAL_STICK: 
        //Conventional dual stick mode
        
        //set stick values
        setAnalogJoy (Xbox.getAnalogHat(LeftHatX), JOY1_X);
        setAnalogJoy (~Xbox.getAnalogHat(LeftHatY), JOY1_Y);
        //Y axis is inverted from Xbox, use ~ instead of - to avoid 2's complement asymmetry 
        setAnalogJoy (Xbox.getAnalogHat(RightHatX), JOY2_X);     
        setAnalogJoy (~Xbox.getAnalogHat(RightHatY), JOY2_Y);     
  
        //triggers are just overloaded as buttons
        if (Xbox.getButtonPress(LT))
          buttonState[4] = 1;
        else
          buttonState[4] = 1;
    
        if (Xbox.getButtonPress(RT))
          buttonState[5] = 1;
        else
          buttonState[5] = 0;
      break;

      case THRUSTMASTER:
        //Thrustmaster, encode POV hat using 4th axis use triggers for X2 (usually rudder)
        X2_val = (signed(Xbox.getButtonPress(RT)*16) - signed(Xbox.getButtonPress(LT)*16));
        setAnalogJoy (Xbox.getAnalogHat(RightHatX), JOY1_X);
        setAnalogJoy (~Xbox.getAnalogHat(RightHatY), JOY1_Y);
        setAnalogJoy (X2_val, JOY2_X);
  
        //Process POV hat
        if (Xbox.getButtonPress(LEFT))
        {
          setAnalogJoy(-6553, JOY2_Y);
        }
        else if (Xbox.getButtonPress(DOWN))
        {
          setAnalogJoy(-13107, JOY2_Y);
        }
        else if (Xbox.getButtonPress(RIGHT))
        {
          setAnalogJoy(-19659, JOY2_Y);
        }
        else if (Xbox.getButtonPress(UP))
        {
          setAnalogJoy(-26212, JOY2_Y);
        }
        else
        {
          setAnalogJoy(0, JOY2_Y);
        }
      break;

      case CH_FLIGHTSTICK:
        //CH Flightstick, encode POV hat as chords, use Y2 as throttle
        //set stick values
        setAnalogJoy (Xbox.getAnalogHat(RightHatX), JOY1_X);
        setAnalogJoy (~Xbox.getAnalogHat(RightHatY), JOY1_Y);
        setAnalogJoy ((signed(Xbox.getButtonPress(RT)*32) - signed(Xbox.getButtonPress(LT)*32)), JOY2_X); //Used for rudder, convert 10->16 bit
        //Keep track of throttle using triggers, make sure it doesn't overflow
        if (Xbox.getAnalogHat(LeftHatY) > 10000 || Xbox.getAnalogHat(LeftHatY) < -10000) //only change throttle if deadzone exceeded
        {
          throttleValue += ( signed(Xbox.getAnalogHat(LeftHatY)) / THROTTLE_SLEW); 
          if (throttleValue > 32767)
            throttleValue = 32767;
          if (throttleValue < -32768)
            throttleValue = -32768; 
          setAnalogJoy (~throttleValue, JOY2_Y);
        }
    
        if (Xbox.getButtonPress(RIGHT) || Xbox.getButtonPress(LEFT) || Xbox.getButtonPress(UP) || Xbox.getButtonPress(DOWN))
        {
          //If any HAT button pressed, clear previous button reads
           buttonState[2] = 0;
           buttonState[3] = 0;
           buttonState[4] = 0;
           buttonState[5] = 0;
        }
        if (Xbox.getButtonPress(RIGHT)) 
        {
           buttonState[2] = 1;
           buttonState[3] = 1;
           buttonState[5] = 1;
        }
        else if (Xbox.getButtonPress(LEFT))
        {
           buttonState[2] = 1;
           buttonState[3] = 1;
        }
        else if (Xbox.getButtonPress(UP))
        {
           buttonState[2] = 1;
           buttonState[3] = 1;
           buttonState[4] = 1;
           buttonState[5] = 1;
        }
        else if (Xbox.getButtonPress(DOWN))
        {
           buttonState[2] = 1;
           buttonState[3] = 1;
           buttonState[4] = 1;
        }
        
      break;
    }
    
    //Send out button presses based on state
    for (int i = 2; i < 6; i++)
    {
      if(buttonState[i])
       pinMode (i, OUTPUT);
      else
       pinMode (i, INPUT);
    }
    
  }
}
