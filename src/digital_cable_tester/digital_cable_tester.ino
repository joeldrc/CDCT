/**
 ******************************************************************************
   @file    Digital Cable Tester
   @date    CERN - Geneva(CH), March 2017.
   @author  Joel Daricou  <joel.daricou@cern.ch>
   @brief   This program allows you to test the Burndy connectors 50 pin thanks to 'use of the proper master system (Arduino Mega 2560 + shield) and Slave.

            The pins depart from number 1 up to 50:
            The odd numbers are used as the reading pin.
            The even numbers are used as enable pin.

 ******************************************************************************
   Board:
    - Developed and tested on Arduino MEGA 2560.

   Info programming:
    - Arduino IDE Download:                               https://www.arduino.cc/en/Main/Software
    - Arduino Mega Official page:                         https://store.arduino.cc/arduino-mega-2560-rev3
    - Arduino LCD Screen:                                 https://store.arduino.cc/arduino-lcd-screen

   References C++:

   Scheme:

 ******************************************************************************
 */

#include <TFT.h>
#include <SPI.h>

//define tft library
static const uint8_t CS = 53;  // Control slave select
static const uint8_t DC = 58;
static const uint8_t RESET = 59;
static const uint8_t BL = 57;
static const uint8_t LCDROTATION = 3; //1 or 3

TFT myScreen = TFT(CS, DC, RESET);

//define pin test GND
static const uint8_t gndPin = 62;

//define pin button
static const uint8_t buttonA = 54;
static const uint8_t buttonB = 55;
static const uint8_t ledButtonA = 56;

// pin not connected ( number 99 == not connected)
static const uint8_t NOT_CONNECTED = 99;

// physical number of pins
static const uint8_t nPin = 50; 


// *** GLOBAL VARIABLE *** =====================================================

uint8_t selectConnector = 11;   //type of connector burndy 0 to 11
uint8_t pinToTest[12] = {   2 , 4, 8, 12, 18, 19, 26, 27, 28, 48, 49, 50  };  //number of pin burndy possible to test

// number of pins used during the test
uint8_t nPinUsed = 50;

//array to store pin value
uint8_t pinSelect[50] = {   2, 22,  3, 23,  4, 24,  5, 25,  6, 26,    //pin Burndy  1-10
                            7, 27,  8, 28,  9, 29, 10, 30, 11, 31,    //pin Burndy 11-20
                            12, 33, 13, 35, 14, 37, 15, 39, 16, 41,   //pin Burndy 21-30
                            17, 43, 18, 45, 19, 47, 20, 49, 21, 42,   //pin Burndy 31-40
                            32, 44, 34, 46, 36, 48, 38, 68, 40, 69    //pin Burndy 41-50
                        };
                        
//array to store pin value after digitalRead();
boolean pinValue[nPin];

//matrix for store final value
uint8_t finalValue[2][nPin];

//multiple contacts
boolean multipleContacts = false;

// short circuit to gnd if is to 0 to 49
byte shortCircuitGnd = NOT_CONNECTED;

// *** SETUP *** ===============================================================
void setup(void) {
  //init serial comunication
  Serial.begin(9600);

  //init button
  pinMode(buttonA, INPUT_PULLUP);
  pinMode(buttonB, INPUT_PULLUP);
  
  //init pins
  pinMode(ledButtonA, OUTPUT);
  digitalWrite(ledButtonA, LOW);
  
  // enable lcd tft
  pinMode(BL, OUTPUT);
  digitalWrite(BL, HIGH);
  
  // init pin GND
  pinMode(gndPin, INPUT_PULLUP);
  
  // init pin burndy
  for (byte i = 0; i < nPin; i++) {
    pinMode(pinSelect[i], INPUT_PULLUP);
  }

  //init tft screen
  myScreen.begin();
  myScreen.setRotation(LCDROTATION);
  delay(1000);

  Serial.println("DIGITAL CABLE TESTER V.2.0");
  Serial.println("CERN, Geneva (CH)");
  Serial.println("Created by Joel Daricou");
  Serial.println("");
  myScreen.background(0, 255, 255);
  myScreen.stroke(0, 0, 0);
  myScreen.setTextSize(6);
  myScreen.text("CERN", 10, 20);
  myScreen.setTextSize(3);
  myScreen.text("BE-RF-IS", 10, 90);
  delay(3000);

  Serial.println("CONNECT GROUND");
  Serial.println("");
  myScreen.background(0, 255, 255);
  myScreen.fill(255, 255, 0);
  myScreen.rect(2, 8, 156, 86);
  myScreen.setTextSize(3);
  myScreen.text("CONNECT", 10, 20);
  myScreen.text("GROUND >", 10, 60);
  
  // button
  buttonContinue();
  
  // wait command
  inputEvent();
  delay(500);
}


// *** LOOP *** ================================================================
void loop(void) {
  char printCharValue[4];
  String convertToString;

  // First Page -----------------------------------------------------
  myScreen.fill(255, 255, 255);
  myScreen.rect(2, 8, 156, 86);
  myScreen.setTextSize(3);
  myScreen.text("CABLE", 30, 20);
  convertToString = String(pinToTest[selectConnector]);
  convertToString.toCharArray(printCharValue, 4);
  myScreen.text(printCharValue, 25, 60);
  myScreen.text("PIN", 80, 60);
  Serial.println("--> DO YOU WANT TO CONTINUE WITH TEST ? (y/n)");
  Serial.println("");
  //clear button
  myScreen.fill(0, 255, 255);
  myScreen.noStroke();
  myScreen.rect(79, 100, 2, 26);
  myScreen.stroke(0, 0, 0);
  //button
  myScreen.setTextSize(2);
  myScreen.fill(255, 255, 255);
  myScreen.rect(2, 100, 77, 26);
  myScreen.text("<PIN>", 10, 106);
  myScreen.rect(81, 100, 77, 26);
  myScreen.text("TEST", 90, 106);
  // ----------------------------------------------------------------
  
  // wait for command 
  // if you press the button A
  if (inputEvent()) {
    
    // Test Page ----------------------------------------------------
    Serial.println("--> START CABLE TEST ");
    Serial.println("");
    myScreen.fill(255, 255, 255);
    myScreen.rect(2, 8, 156, 86);
    myScreen.setTextSize(2);
    myScreen.text("TESTING", 40, 20);
    myScreen.text(printCharValue, 15, 40);
    myScreen.text("PIN", 45, 40);
    myScreen.text("CABLE", 90, 40);
    // change color button
    myScreen.noStroke();
    myScreen.fill(0, 255, 255);
    myScreen.rect(2, 100, 77, 26);
    myScreen.stroke(0, 0, 0);
    // button
    myScreen.fill(0, 255, 0);
    myScreen.rect(81, 100, 77, 26);
    myScreen.text("TEST", 90, 106);
    // --------------------------------------------------------------

    // start test cable: if the test is ok
    if (testCable() == true) {
      Serial.println("-> TEST: OK");
      Serial.println("");
      // Test Page OK -------------------------------------------------
      myScreen.fill(0, 255, 0);
      myScreen.rect(2, 8, 156, 86);
      myScreen.setTextSize(4);
      myScreen.text("TEST", 35, 20);
      myScreen.text("OK!", 50, 55);
      // button
      buttonContinue();
      // wait command
      inputEvent();
      // --------------------------------------------------------------
      delay(500);
    }
    
    // if the test is bad
    else {
      Serial.println("-> TEST: ERROR");
      Serial.println("");
      // Test Page ERROR ----------------------------------------------
      myScreen.fill(255, 255, 0);
      myScreen.rect(2, 8, 156, 86);
      myScreen.setTextSize(4);
      myScreen.text("TEST", 35, 20);
      myScreen.text("ERROR", 20, 55);
      // button
      buttonContinue();
      //wait command
      inputEvent();
      // --------------------------------------------------------------
      delay(500);

      // SHOW ERROR PAGE ----------------------------------------------
      myScreen.background(0, 255, 255);
      myScreen.setTextSize(1);
      // display errors value
      displayError();
      // button
      Serial.println("--> CONTINUE ? (y/n)");
      Serial.println("");
      myScreen.stroke(0, 0, 0);
      myScreen.fill(255, 255, 255);
      myScreen.rect(2, 113, 156, 13);
      myScreen.text("CONTINUE", 54, 116);
      // wait command
      inputEvent();
      // --------------------------------------------------------------
      delay(500);

      // reset background
      myScreen.background(0, 255, 255);
    }

    Serial.println("--> END CABLE TEST");
    Serial.println("");
  }
  else {
    // change the number of pin to test
    if (selectConnector < 11) {
      // for now not enabled
      //selectConnector += 1;
    }
    else {
      // for now not enabled
      //selectConnector = 0;
    }
    delay(500);
  }
}


// *** button continue *** =====================================================
void buttonContinue(void) {
  Serial.println("--> CONTINUE ? (y/n)");
  Serial.println("");
  myScreen.fill(255, 255, 255);
  myScreen.rect(2, 100, 156, 26);
  myScreen.setTextSize(2);
  myScreen.text("CONTINUE", 34, 106);
}


// *** inputEvent* ** ==========================================================
boolean inputEvent() {
  char commandData = 0;
  //wait for command
  while (1) {
    if (Serial.available()) {
      commandData = Serial.read();
    }
    if (((commandData == 'y') || (commandData == 'Y')) || (digitalRead(buttonA) == 0)) {
      return true;
    }
    else if (((commandData == 'n') || (commandData == 'N')) || (digitalRead(buttonB) == 0)) {
      return false;
    }
  }
}


// *** testCable *** ===========================================================
boolean testCable() {
  // initialize variables and pins
  boolean testResult = true;
  multipleContacts = false;
  shortCircuitGnd = NOT_CONNECTED;

  for (byte i = 0; i < nPin; i++) {
    finalValue[0][i] = NOT_CONNECTED;
    finalValue[1][i] = NOT_CONNECTED;
    pinValue[i] = 1;
    pinMode(pinSelect[i], INPUT_PULLUP);
  }

  // test ground insulation
  for (byte i = 0; i < nPin; i++) {
    pinMode(pinSelect[i], OUTPUT);
    digitalWrite(pinSelect[i], LOW);
    delay(5);

    boolean groundValue = digitalRead(gndPin);
    
    if (groundValue == 0) {
      Serial.println(i);
      Serial.println("--> C.C with GROUND");
      Serial.println("");
      shortCircuitGnd = i;
    }
    delay(5);

    pinMode(pinSelect[i], INPUT_PULLUP);
  }

  // start pin test
  for (byte i = 0; i < nPin; i += 2) {
    // set pin out
    pinMode(pinSelect[i], OUTPUT);
    digitalWrite(pinSelect[i], LOW);

    // print serial
    Serial.println("");
    Serial.print("-> TEST PIN: ");
    Serial.print(i);
    Serial.println("");
    Serial.println("Npin | pinValue");

    byte j = 0;

    // read pin
    for (byte x = 0; x < nPin; x++) {
      if (x == i) {
        pinValue[i] = 1;
      }
      else {
        pinValue[x] = digitalRead(pinSelect[x]);
      }
      // control multiple contacts
      if (pinValue[x] == 0) {
        if (j < 2) {
          finalValue[j][i] = x;
          j++;
        }
        else {
          multipleContacts = true;
        }
      }

      // serial print Value
      Serial.print(x);
      if (x < 10)
        Serial.print("  | ");
      else
        Serial.print(" | ");
      Serial.print(pinValue[x]);
      if (pinValue[x] == 0)
        Serial.println(" <--");
      else
        Serial.println("");
        
      //delay(5);
    }

    // set pin to input
    pinMode(pinSelect[i], INPUT_PULLUP);

    // blink led button on TEST
    digitalWrite(ledButtonA, HIGH);

    // display loading bar -----------------------------------------------------
    char printCharValue[4];
    myScreen.fill(255, 255, 255);
    myScreen.rect(30, 65, 100, 20);
    myScreen.fill(0, 255, 0);
    myScreen.rect(30, 65, (i * 2) + 4, 20);
    myScreen.fill(255, 255, 255);
    String convertToString = String((i * 2) + 4);
    convertToString.toCharArray(printCharValue, 4);
    myScreen.text(printCharValue, 60, 68);
    myScreen.text("%", 95, 68);
    // -------------------------------------------------------------------------

    // blink led button on TEST
    digitalWrite(ledButtonA, LOW);
  }

  // print serial chart
  Serial.println("");
  Serial.println("--> TEST RESULT:");
  Serial.println("");
  Serial.println("Npin | pin A | pin B ");
  
  for (byte i = 0; i < nPin; i += 2) {
    Serial.print(i);
    if (i < 10)
      Serial.print("  | ");
    else
      Serial.print(" | ");
    Serial.print(finalValue[0][i]);
    if (finalValue[0][i] < 10)
      Serial.print("  | ");
    else
      Serial.print(" | ");
    Serial.println(finalValue[1][i]);
  }
  Serial.println("");

  // preliminary error check
  for (byte i = 0; i < nPin; i += 2) {
    // *** TEST IF PIN AFTER AND PIN BEFORE ARE CORRECT ***
    if ((i == 0) && (finalValue[0][i] == (i + 1)) && (finalValue[1][i] == (nPin - 1))) {
      //testResult = true;
    }
    else if ((finalValue[0][i] == (i - 1)) && (finalValue[1][i] == (i + 1))) {
      //testResult = true;
    }
    else {
      testResult = false;
    }
  }

  // preliminary error check
  if (multipleContacts == true) {
    testResult = false;
  }
  else if (shortCircuitGnd != NOT_CONNECTED) {
    testResult = false;
  }

  return testResult;
}


// *** testVerify *** ==========================================================
void displayError() {
  for (uint8_t i = 0; i < (nPin - 1); i += 2) {
    if ((finalValue[0][i] == NOT_CONNECTED) && (finalValue[1][i] == NOT_CONNECTED)) {
      Serial.print(i);
      Serial.print(" -> ");
      Serial.println("!");
      printErrors(i, NOT_CONNECTED, i);
    }
    else {
      Serial.print(i);
      Serial.print(" -> ");
      Serial.println(i);
      printErrors(i, i, i);
    }

    if (i == 0) {
      Serial.print(i + 1); // add one to read 1 to 50
      Serial.print(" -> ");
      Serial.println(finalValue[0][i]);

      printErrors(i + 1, finalValue[0][i], i + 1);
    }
    else {
      Serial.print(i + 1);
      Serial.print(" -> ");
      Serial.println(finalValue[1][i]);

      printErrors(i + 1, finalValue[1][i], i + 1);
    }
  }
}


// *** printErrors *** =========================================================
void printErrors(byte valueA, byte valueB, byte index) {
  String convertToString;
  char printCharValueA[4];
  char printCharValueB[4];
  byte x = 3;
  byte y = 3;

  convertToString = String(valueA);
  convertToString.toCharArray(printCharValueA, 4);

  if (valueB == NOT_CONNECTED) {
    convertToString = String("!");
    convertToString.toCharArray(printCharValueB, 4);
  }
  else {
    convertToString = String(valueB);
    convertToString.toCharArray(printCharValueB, 4);
  }

  if (index == 0) {
    //
  }
  else if (index < 10) {
    y += index * 10;
  }
  else if (index == 10) {
    x *= 10;
  }
  else if (index < 20) {
    x *= 10;
    y += (index - 10) * 10;
  }
  else if (index == 20) {
    x *= 21;
  }
  else if (index < 30) {
    x *= 21;
    y += (index - 20) * 10;
  }
  else if (index == 30) {
    x *= 32;
  }
  else if (index < 40) {
    x *= 32;
    y += (index - 30) * 10;
  }
  else if (index == 40) {
    x *= 43;
  }
  else if (index < 50) {
    x *= 43;
    y += (index - 40) * 10;
  }

  // print on lcd
  myScreen.text(printCharValueA, x, y);

  if (valueA != valueB) {
    myScreen.stroke(255, 0, 0);
  }

  if (index < 10) {
    if (valueA != valueB) {
      myScreen.stroke(255, 0, 0);
    }
    myScreen.text(">", x + 5, y);
    myScreen.text(printCharValueB, x + 11, y);
  }
  else {
    if (valueA != valueB) {
      myScreen.stroke(255, 0, 0);
    }
    myScreen.text(">", x + 11, y);
    myScreen.text(printCharValueB, x + 17, y);
  }
  myScreen.stroke(0, 0, 0);


  // print gnd value
  if (shortCircuitGnd != NOT_CONNECTED) {
    myScreen.stroke(255, 0, 0);
    myScreen.text("GND INSULATION: ERROR", 3, 103);
    char printCharValueC[4];
    convertToString = String(shortCircuitGnd);
    convertToString.toCharArray(printCharValueC, 4);
    myScreen.text(printCharValueC, 135, 103);
    myScreen.stroke(0, 0, 0);
  }
  else if (multipleContacts == true) {
    myScreen.stroke(255, 0, 0);
    myScreen.text("MULTIPLE CONTACTS ERROR", 3, 103);
    myScreen.stroke(0, 0, 0);
  }
  else {
    myScreen.text("GND INSULATION: OK", 3, 103);
  }
}
