/*
 Example sketch for the Xbox ONE USB library - by guruthree, based on work by
 Kristian Lauszus.
 */

#include <XBOXONE.h>
#include <Wire.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

#define JOY1_X 0
#define JOY1_Y 1
#define JOY2_X 2
#define JOY2_Y 3

USB Usb;
XBOXONE Xbox(&Usb);

byte RegAddr[] = {0x00, 0x10, 0x60, 0x70};
byte ButtonList[] = {2, 3, 4, 5};
byte oldA;

void setAnalogJoy(int value, byte channel)
{ 
  Wire.beginTransmission(0x2E);
  Wire.write(RegAddr[channel]);
  Wire.write(byte((value/256) + 128));
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1); //halt
  }
  Serial.print(F("\r\nXBOX ONE USB Library Started"));

  Wire.begin();
  Wire.setClock(400000);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite (2, LOW);
  digitalWrite (3, LOW);
  digitalWrite (4, LOW);
  digitalWrite (5, LOW);
}
void loop() {
  Usb.Task();
  if (Xbox.XboxOneConnected) {
    //Don't do anything unless Xbox controller is connected

    //set stick values
    setAnalogJoy (Xbox.getAnalogHat(LeftHatX), JOY1_X);
    setAnalogJoy (~Xbox.getAnalogHat(LeftHatY), JOY1_Y);
    //Y axis is inverted from Xbox, use ~ instead of - to avoid 2's complement asymmetry 
    setAnalogJoy (Xbox.getAnalogHat(RightHatX), JOY2_X);     
    setAnalogJoy (~Xbox.getAnalogHat(RightHatY), JOY2_Y);     
    
    if (Xbox.getButtonPress(A))
      pinMode (2, OUTPUT);
    else
      pinMode (2, INPUT);
    if (Xbox.getButtonPress(B))
      pinMode (3, OUTPUT);
    else
      pinMode (3, INPUT);
    if (Xbox.getButtonPress(X))
      pinMode (4, OUTPUT);
    else
      pinMode (4, INPUT);
    if (Xbox.getButtonPress(Y))
      pinMode (5, OUTPUT);
    else
      pinMode (5, INPUT);
    
    
    if (Xbox.getButtonPress(RB))
      pinMode (2, OUTPUT);
    else
      pinMode (2, INPUT);
    if (Xbox.getButtonPress(LB))
      pinMode (3, OUTPUT);
    else
      pinMode (3, INPUT);

    if (Xbox.getButtonPress(LT))
      pinMode (4, OUTPUT);
    else
      pinMode (4, INPUT);

    if (Xbox.getButtonPress(RT))
      pinMode (5, OUTPUT);
    else
      pinMode (5, INPUT);
//    if (Xbox.getButtonClick(UP))
//      Serial.println(F("Up"));
//    if (Xbox.getButtonClick(DOWN))
//      Serial.println(F("Down"));
//    if (Xbox.getButtonClick(LEFT))
//      Serial.println(F("Left"));
//    if (Xbox.getButtonClick(RIGHT))
//      Serial.println(F("Right"));
//
//    if (Xbox.getButtonClick(START))
//      Serial.println(F("Start"));
//    if (Xbox.getButtonClick(BACK))
//      Serial.println(F("Back"));
//    if (Xbox.getButtonClick(XBOX))
//      Serial.println(F("Xbox"));
//    if (Xbox.getButtonClick(SYNC))
//      Serial.println(F("Sync"));
//    if (Xbox.getButtonClick(SHARE))
//      Serial.println(F("Share"));
//
//    if (Xbox.getButtonClick(LB))
//      Serial.println(F("LB"));
//    if (Xbox.getButtonClick(RB))
//      Serial.println(F("RB"));
//    if (Xbox.getButtonClick(LT))
//      Serial.println(F("LT"));
//    if (Xbox.getButtonClick(RT))
//      Serial.println(F("RT"));
//    if (Xbox.getButtonClick(L3))
//      Serial.println(F("L3"));
//    if (Xbox.getButtonClick(R3))
//      Serial.println(F("R3"));
//
//    if (oldA != Xbox.getButtonClick(A))
//    {
//      oldA = Xbox.getButtonClick(A);
//      if (Xbox.getButtonClick(A))
//      {
//        Serial.println(F("A_down"));
//        setButtonDown (0);
//      }
//      else
//      {
//        Serial.println(F("A_down"));
//        setButtonUp (0);
//      }
//    }
//    if (Xbox.getButtonClick(B))
//      Serial.println(F("B"));
//    if (Xbox.getButtonClick(X))
//      Serial.println(F("X"));
//    if (Xbox.getButtonClick(Y))
//      Serial.println(F("Y"));
  }
}
