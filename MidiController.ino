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


/**
 * Setup Function
 * Configure the System
 */
void setup(void) {
  // Display Init
  u8g2.begin();
}

/**
 * Loop Function
 */
void loop(void) {

}