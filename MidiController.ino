//#################################################################################################
//##                                    ------ MIDI-C ------                                     ##
//##                           Midi Controller using Arduino Mega2560                            ##
//#################################################################################################

#include <U8g2lib.h>
#include <Wire.h>

// Display Config
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);


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