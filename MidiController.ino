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


/**
 * Setup Function
 * Configure the System
 */
void setup(void) {
  // Display Init
  u8g2.begin();

  // Encoder Init
  pinMode(ECL, INPUT);
  pinMode(EDT, INPUT);
  pinMode(ESW, INPUT_PULLUP);
  nELCL = digitalRead(ECL);
}

/**
 * Loop Function
 */
void loop(void) {
  // Encoder
  encoderRead(false);
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