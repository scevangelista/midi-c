//#################################################################################################
//##                                    ------ MIDI-C ------                                     ##
//##                           Midi Controller using Arduino Mega2560                            ##
//#################################################################################################

#include <U8g2lib.h>
#include <Wire.h>

// Display Config
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);

// Encoder Config
#define ECL 23  // PIN CLK
#define EDT 24  // PIN DT
#define ESW 22  // PIN SW

// Encoder Variables
int nEVL = 0;    // Current Global Value
int nEIVL = 0;   // Current Internal Value
int nECL;        // Current Position of CLK
int nELCL;       // Last Position of CLK
bool bESW = 0;   // Current position of SW
bool bLESW = 0;  // Last position of SW

// Display Variables
int nRoute = 0;     // Current Screen Route
int nLRoute = -1;   // Last Screen Route
int nPos = 0;       // List Position
bool bRefresh = 0;  // Refresh Screen

// System Variables
int nSysL = 0;            // Language: 0-Português, 1-English
int nSysP = 0;            // Current Preset
double nSysI = millis();  // Last Interaction

// Midi Variables
int nCH[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // Channel
int nCT[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // Type
int nCC[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // Controller
int nVL[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // Value

int nBT[] = { 40, 41, 42, 43, 44, 45 }; // SW PIN
int nLD[] = { 30, 31, 32, 33, 34, 35 }; // LED PIN
int valIni[] = { 0, 0, 0, 0, 0, 0 }; // Initial value of SW
int valNow[] = { 0, 0, 0, 0, 0, 0 }; // Current value of SW
int altAte[] = { 0, 0, 0, 0, 0, 0 }; // Reads with new SW value
int nBtn = 6; // Number of SWs
int nAte = 10; // Number of reads to change SW value

/**
 * Setup Function
 * Configure the System
 */
void setup(void) {
  // Display Init
  u8g2.begin();

  // MIDI Setup
  Serial.begin(31250);

  // Foot Switchs
  for (int i = 0; i < nBtn; i++) {
    pinMode(nBT[i], INPUT);
    pinMode(nLD[i], OUTPUT);
    valNow[i] = valIni[i] = digitalRead(nBT[i]);
  }

  // Encoder Init
  pinMode(ECL, INPUT);
  pinMode(EDT, INPUT);
  pinMode(ESW, INPUT_PULLUP);
  nELCL = digitalRead(ECL);

  // Initial LED sequence
  ledInit();

  delay(200);
}

/**
 * Loop Function
 */
void loop(void) {
  // Encoder
  encoderRead(false);

  // Execute a Midi
  midiExec();
}



/////////////////////////////////////////////////////////////////////////////////////
// Encoder Functions ////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Read the rotary encoder with SW
 * param bool internal
 */
void encoderRead(bool internal) {
  // Rotate Function
  nECL = digitalRead(ECL);

  if (nECL != nELCL && nECL == 1) {
    if (digitalRead(EDT) != nECL) {
      if (internal) {
        nEIVL--;
      } else {
        nEVL--;
      }
    } else {
      if (internal) {
        nEIVL++;
      } else {
        nEVL++;
      }
    }
    nSysI = millis();
  }

  nELCL = nECL;

  // Button Function
  if (digitalRead(ESW) == LOW) {
    if (bLESW) {
      bESW = 0;
    } else {
      bLESW = bESW = 1;
    }
  } else {
    if (bLESW == 1) {
      nSysI = millis();
      bLESW = 0;
    }
  }
}

/**
 * Verify if encoder has changed
 * return bool Changed
 */
bool encoderStatus() {
  return (nPos != nEVL || bESW);
}


/////////////////////////////////////////////////////////////////////////////////////
// LED functions ///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Update led status
 * param int nLed Number of led
 * param int nStatus On or Off
 */ 
void led(int nLed, int nStatus) {
  digitalWrite(nLed, nStatus);
}

/**
 * Execute a sequential LED on or off
 * param int nSStatus On or Off
 */
void ledS(int nSStatus){
  for (int i = 0; i < nBtn; i++) {
    led(nLD[i], nSStatus);
    delay(200);
  }
}

/**
 * LED Initial Sequence and Reset
 */
void ledInit() {
  // Sequence On
  ledS(1);

  // Default configuration if encoder is pressed
  //  in LED Init
  encoderRead(false);
  if(bESW){
    bESW = 0;
    if (printConfirm()) {
      nSysL = 0;
      sysDefault();
      ledS(0);
      homeScreen();
    }
  }  

  // Blink all leds
  for (int x = 0; x < 4; x++){
    for (int i = 0; i < nBtn; i++) {
      led(nLD[i], 1);
    }
    delay(100);
    for (int i = 0; i < nBtn; i++) {
      led(nLD[i], 0);
    }
    delay(100);
  }
}


/////////////////////////////////////////////////////////////////////////////////////
// MIDI functions ///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Execute a Midi Functions
 */
void midiExec(){
  // Read All SWs  
  for (int i = 0; i < nBtn; i++) {
    int pinVal = digitalRead(nBT[i]);
    if (pinVal != valNow[i]){
      // Diminui falso positivo
      if (altAte[i] < nAte){
        altAte[i]++;
        continue;
      }
      altAte[i] = 0;

      if (valIni[i] != pinVal){        
        led(nLD[i], 1);
        midiSend(nCH[i], nCT[i], nCC[i], nVL[i]);
      }
      else{
        led(nLD[i], 0);
        midiSend(nCH[i + 6], nCT[i + 6], nCC[i + 6], nVL[i + 6]);
      }

      valNow[i] = pinVal;
    }
  }
}

/**
 * Send Midi Comand
 * param int ch Channel
 * param int ct Command Type 1-PC 2-CC
 * param int cc Controller
 * param int vl Value
 */ 
void midiSend(int ch, int ct, int cc, int vl) {
  int data1;

  if(ct == 1){
    data1 = 0xC0 + ch; //PC
  }
  else{
    data1 = 0xB0 + ch; //CC
  }
  Serial.write(data1);
  Serial.write(cc);
  Serial.write(vl);
}