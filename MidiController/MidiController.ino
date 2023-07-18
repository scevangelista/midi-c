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

// Button Variables
int nFS;   // Button to configure
int nFSA;  // Action to configure

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

  // Load EEPROM Data
  sysLoad();

  delay(200);
}

/**
 * Loop Function
 */
void loop(void) {
  // Return to HomeScreen after 60 secs
  //  without encoder action
  if ((nSysI + 60000) < millis()) {
    nSysI = millis();
    screen(0, 0);
  }

  // Encoder
  encoderRead(false);

  // Execute a screen
  screen(nRoute, nPos);

  // Execute a Midi
  midiExec();
}


/////////////////////////////////////////////////////////////////////////////////////
// Display //////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Screen Router
 * param int nNewRoute Route
 * param int nPosSet Selected option
 */
void screen(int nNewRoute, int nPosSet) {
  // New route 
  if (nNewRoute != nLRoute) {
    nLRoute = nRoute;
    nRoute = nNewRoute;
    bRefresh = true;
    nEVL = nPos = nPosSet;
    bESW = false;
  }

  // Routing
  switch (nRoute) {
    case 0:
      homeScreen();
      break;

    case 1:
      configScreen();
      break;

    case 10:
      buttonsScreen();
      break;

    case 100:
      buttonScreen();
      break;

    case 101:
      buttonConfScreen();
      break;

    case 11:
      loadScreen();
      break;

    case 12:
      saveScreen();
      break;

    case 13:
      languageScreen();
      break;
  }

  bRefresh = false;
}

/**
 * Generate home screen
 */
void homeScreen() {
  // Route to Menu
  if (bESW) {
    screen(1, 0);
    return;
  }

  // Draw
  if (bRefresh) {
    u8g2.clearBuffer();
    u8g2.setFontPosTop();
    u8g2.setFontMode(1);
    u8g2.setDrawColor(2);
    u8g2.setFont(u8g2_font_maniac_tf);
    u8g2.drawBox(0, 16, 128, 30);
    u8g2.drawUTF8(19, 20, "MIDI C");
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawUTF8(50, 53, "1.0.0");
    u8g2.sendBuffer();
  }
}

/**
 * Generate config menu
 */
void configScreen() {
  char *aLMenu[5];
  char *cTitleM;

  // Language
  switch (nSysL) {
    case 0:  // PT-BR
      cTitleM = "Configurações";
      aLMenu[0] = "Configurar Botões";
      aLMenu[1] = "Carregar Preset";
      aLMenu[2] = "Salvar Preset";
      aLMenu[3] = "Idioma";
      aLMenu[4] = "Sair";
      break;
    case 1:  // EN-US
      cTitleM = "Configurations";
      aLMenu[0] = "Configure FSW";
      aLMenu[1] = "Load Preset";
      aLMenu[2] = "Save Preset";
      aLMenu[3] = "Language";
      aLMenu[4] = "Exit";
  }

  // Route to
  if (bESW) {
    switch (nPos) {
      case 0:  // Button Configure
        screen(10, 0);
        break;
      case 1:  // Load Preset
        screen(11, nSysP);
        break;
      case 2:  // Save Preset
        screen(12, nSysP);
        break;
      case 3:  // Language
        screen(13, nSysL);
        break;
      case 4:  // Home
        screen(0, 0);
        break;
    }
    return;
  }

  // Draw
  if (bRefresh || encoderStatus()) {
    u8g2.clearBuffer();
    u8g2.setFontPosTop();
    printTitle(cTitleM);
    printOptions(aLMenu, 5, -1);
    u8g2.sendBuffer();

    bRefresh = false;
  }
}

/**
 * Generate language config
 */
void languageScreen() {
  char *aLList[3];
  char *cTitleL;

  // Language
  switch (nSysL) {
    case 0:  // PT-BR
      cTitleL = "Idioma";
      aLList[0] = "Português";
      aLList[1] = "Inglês";
      aLList[2] = "Voltar";
      break;
    case 1:  // EN-US
      cTitleL = "Language";
      aLList[0] = "Portuguese";
      aLList[1] = "English";
      aLList[2] = "Back";
      break;
  }

  // Change Language
  if (bESW) {
    switch (nPos) {
      case 0:  // PT-BR
        if (printConfirm()) {
          nSysL = 0;
          EEPROM.update(0, 0);
          printSaved(1, 3);
          return;
        }
        break;
      case 1:  // EN-US
        if (printConfirm()) {
          nSysL = 1;
          EEPROM.update(0, 1);
          printSaved(1, 3);
          return;
        }
        break;
      default:
        // Return to Menu
        screen(1, 3);
        return;
    }

    bRefresh = true;
  }

  // Draw
  if (bRefresh || encoderStatus()) {
    u8g2.clearBuffer();
    u8g2.setFontPosTop();
    printTitle(cTitleL);
    printOptions(aLList, 3, nSysL);
    u8g2.sendBuffer();

    bRefresh = false;
  }
}

/**
 * Generate load preset
 */
void loadScreen() {
  char *aPList[6];
  char *cTitleP;

  // Language
  switch (nSysL) {
    case 0:  // PT-BR
      cTitleP = "Carregar Preset";
      aPList[0] = "Usuário 1";
      aPList[1] = "Usuário 2";
      aPList[2] = "Usuário 3";
      aPList[5] = "Voltar";
      break;
    case 1:  // EN-US
      cTitleP = "Load Preset";
      aPList[0] = "User 1";
      aPList[1] = "User 2";
      aPList[2] = "User 3";
      aPList[5] = "Back";
      break;
  }
  aPList[3] = "Valeton GP200";
  aPList[4] = "POD X3 Live";

  // Load Preset
  if (bESW) {
    if (nPos != 5) {
      if (printConfirm()) {
        loadPreset(nPos);
        printSaved(1, 1);
        return;
      }
    }
    screen(1, 1);
    return;
  }

  // Draw
  if (bRefresh || encoderStatus()) {
    u8g2.clearBuffer();
    u8g2.setFontPosTop();
    printTitle(cTitleP);
    printOptions(aPList, 6, nSysP);
    u8g2.sendBuffer();

    bRefresh = false;
  }
}

/**
 * Generate save preset
 */
void saveScreen() {
  char *aSList[4];
  char *cTitleS;

  // Language
  switch (nSysL) {
    case 0:  // PT-BR
      cTitleS = "Salvar Preset";
      aSList[0] = "Usuário 1";
      aSList[1] = "Usuário 2";
      aSList[2] = "Usuário 3";
      aSList[3] = "Voltar";
      break;
    case 1:  // EN-US
      cTitleS = "Save Preset";
      aSList[0] = "User 1";
      aSList[1] = "User 2";
      aSList[2] = "User 3";
      aSList[3] = "Back";
      break;
  }

  // Save Preset
  if (bESW) {
    if (nPos != 3) {
      if (printConfirm()) {
        savePreset(nPos);
        printSaved(1, 2);
        return;
      }
    }
    screen(1, 2);
    return;
  }

  // Draw
  if (bRefresh || encoderStatus()) {
    u8g2.clearBuffer();
    u8g2.setFontPosTop();
    printTitle(cTitleS);
    printOptions(aSList, 4, nSysP);
    u8g2.sendBuffer();

    bRefresh = false;
  }
}

/**
 * Generate FSW List
 */
void buttonsScreen() {
  char *aFList[7];
  char *cTitleF;


  // Language
  switch (nSysL) {
    case 0:  // PT-BR
      cTitleF = "Configurar FSW";
      aFList[6] = "Voltar";
      break;
    case 1:  // EN-US
      cTitle = "FSW Configure";
      aFList[6] = "Back";
  }

  // Reset FSW to configure
  nFS = -1;

  // FSW List
  aFList[0] = "FSW 1";
  aFList[1] = "FSW 2";
  aFList[2] = "FSW 3";
  aFList[3] = "FSW 4";
  aFList[4] = "FSW 5";
  aFList[5] = "FSW 6";

  // Select FSW or Return
  if (bESW) {
    if (nPos != 6) {
      nFS = nPos;
      screen(100, 0);
      return;
    }
    screen(1, 0);
    return;
  }

  // Draw
  if (bRefresh || encoderStatus()) {
    u8g2.clearBuffer();
    u8g2.setFontPosTop();
    printTitle(cTitleF);
    printOptions(aFList, 7, -1);
    u8g2.sendBuffer();

    bRefresh = false;
  }
}

/**
 * Generate FSW config actions
 */
void buttonScreen() {
  char *aOList[3];
  char *cTitleO;

  // Language
  switch (nSysL) {
    case 0:  // PT-BR
      cTitleO = "Ações do FSW";
      aOList[2] = "Voltar";
      break;
    case 1:  // EN-US
      cTitleO = "Actions of FSW";
      aOList[2] = "Back";
  }
  aOList[0] = "Off";
  aOList[1] = "On";

  // Select action to config
  if (bESW) {
    if (nPos != 2) {
      nFSA = nPos;
      screen(101, 0);
      return;
    }
    screen(10, nFS);
    return;
  }

  // Draw
  if (bRefresh || encoderStatus()) {
    u8g2.clearBuffer();
    u8g2.setFontPosTop();
    printTitle(cTitleO);
    printOptions(aOList, 3, -1);
    u8g2.sendBuffer();

    bRefresh = false;
  }
}

/**
 * Generate FSW action config
 */
void buttonConfScreen() {
  char *aMList[3];
  char *cTitleM;
  char *cTypeM[2];
  int nPosX = nPos;

  // Type
  cTypeM[1] = "PC";
  cTypeM[0] = "CC";

  // Language
  switch (nSysL) {
    case 0:  // PT-BR
      aMList[0] = "Canal";
      aMList[1] = "Tipo do Comando";
      aMList[2] = "CC";
      aMList[3] = "Valor";
      aMList[4] = "Voltar";
      break;
    case 1:  // EN-US
      aMList[0] = "Channel";
      aMList[1] = "Command Type";
      aMList[2] = "Controller CC";
      aMList[3] = "Value";
      aMList[4] = "Back";
  }

  // Title based in action
  if (nFSA == 0) {
    cTitle = "FSW ON ";
  } else {
    cTitle = "FSW OFF";
  }

  // Configure MIDI
  if (bESW) {
    switch (nPos) {
      case 0:  // Channel   
        printValue((aMList[nPos]), nCH[nFS + (nFSA * 6)], 0, 16);
        Serial.println((String) "Valor Retornado: " + nEIVL);
        //nCH[nFS + (nFSA * 6)] = nEIVL;
        bRefresh = true;
        break;

      case 1: // Type
        printInternalOptions((aMList[nPos]), cTypeM, 2, nCT[nFS + (nFSA * 6)]);
        Serial.println((String) "Valor Retornado: "+ nEIVL);
        Serial.println((nFS + (nFSA * 6)));
        bRefresh = true;
        break;

      case 2:  // CC
        printValue((aMList[nPos]), nCC[nFS + (nFSA * 6)], 0, 254);
        Serial.println((String) "Valor Retornado: "+ nEIVL);
        //Serial.println((String) "Posição: "+ (nFS + (nFSA * 6)));
        //nCC[nFS + (nFSA * 6)] = nEIVL;
        bRefresh = true;
        break;

      case 3:  // Value
        printValue((aMList[nPos]), nVL[nFS + (nFSA * 6)], 0, 254);
        //nVL[nFS + (nFSA * 6)] = nEIVL;
        bRefresh = true;
        break;

      default: // Return
        screen(100, nFSA);
        return;
        break;
    }
    nPos = nPosX;
  }

  // Draw
  if (bRefresh || encoderStatus()) {
    u8g2.clearBuffer();
    u8g2.setFontPosTop();
    printTitle(cTitleM);
    printOptions(aMList, 5, -1);
    u8g2.sendBuffer();

    bRefresh = false;
  }
}

/////////////////////////////////////////////////////////////////////////////////////
// Screen functions /////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Draw tittle
 * param char* title Title of screen
 */
void printTitle(char *title) {
  // Draw
  u8g2.setFontMode(1);
  u8g2.setDrawColor(2);
  u8g2.setFont(u8g2_font_8x13_tf);
  u8g2.drawUTF8(0, 2, title);
  u8g2.drawBox(0, 15, 128, 1);
}

/**
 * Draw a options list - OK
 * param char** aOptions Array of options
 * param int nOptions Number of options in array
 * param int nOptSel Number of option selected
 */
void printOptions(char **aList, int nOptions, int nOptSel) {
  int aOShow[3] = { 0, 1, 2 }; // Only 3 lines
  int nLenOpt;
  nOptions--;

  // Position
  if (nPos != nEVL) {
    if (nEVL > nOptions) {
      nEVL = nPos = nOptions;
    } else {
      if (nEVL < 0) {
        nEVL = nPos = 0;
      } else {
        nPos = nEVL;
      }
    }
  }

  // Options to show
  if (nPos > 2) {
    aOShow[0] = (nPos - 2);
    aOShow[1] = (nPos - 1);
    aOShow[2] = nPos;
  }

  // Draw
  u8g2.setFont(u8g2_font_7x13_tf);
  for (int i = 0; i <= min(nOptions, 2); i++) {
    if (nPos == aOShow[i]) {
      u8g2.drawBox(0, (16 * (i + 1)), 128, 16);
    }
    u8g2.drawUTF8(2, (16 * (i + 1) + 2), aList[aOShow[i]]);
    if (nOptSel == aOShow[i]) {
      if ((nLenOpt = strlen(aList[aOShow[i]])) < 15) {
        u8g2.drawUTF8(2 + (nLenOpt * 7), (16 * (i + 1) + 2), " <<");
      }
    }
  }
}

/**
 * Print a confirm screen
 * return bool Confirm
 */
bool printConfirm() {
  bool r = true;
  char *cTitleC;
  char *cMensC;
  char *cYes;
  char *cNo;
  int nIOpt = 1;

  // Language
  switch (nSysL) {
    case 0:  // PT-BR
      cTitleC = "Confirmar";
      cMensC = "Tem certeza?";
      cYes = "Sim";
      cNo = "Não";
      break;
    case 1:  // EN-US
      cTitleC = "Confirm";
      cMensC = "Are you sure?";
      cYes = "Yes";
      cNo = "No";
      break;
  }

  // Internal loop
  while (r) {
    encoderRead(true);

    if (nEIVL >= 1) {
      nEIVL = 1;
    } else {
      nEIVL = 0;
    }

    // Draw
    if (nEIVL != nIOpt) {
      nIOpt = nEIVL;

      u8g2.clearBuffer();
      u8g2.setFontPosTop();
      printTitle(cTitleC);
      u8g2.setFont(u8g2_font_7x13_tf);
      u8g2.drawUTF8(2, 22, cMensC);
      u8g2.drawUTF8(24, 47, cNo);
      u8g2.drawUTF8(84, 47, cYes);

      // Selected Box
      if (nIOpt == 0) {
        u8g2.drawBox(20, 45, 30, 16);
      } else {
        u8g2.drawBox(80, 45, 30, 16);
      }
      u8g2.sendBuffer();
    }

    // Exit of loop
    if (bESW) {
      r = bESW = false;
    }
  }

  // Return
  if (nIOpt == 1) {
    return true;
  } else {
    return false;
  }
}

/**
 * Draw Value Selector
 * param char* cMens Mensagem
 * param int nValA Valor atual
 * param int nValI Valor mínimo
 * param int nValE Valor máximo
 */
void printValue(char *cMens, int nValA, int nValI, int nValE) {
  bool r = true;
  char *cTitleV;
  char cVal[3];
  int nIVal = nValA;
  bool lInit = true;

  // Language
  switch (nSysL) {
    case 0:  // PT-BR
      cTitleV = "Escolha o Valor";
      break;
    case 1:  // EN-US
      cTitleV = "Inform Value";
      break;
  }

  // Loop Interno
  while (r) {
    encoderRead(true);

    // Stop
    if (bESW) {
      r = bESW = false;
    }

    //Limit values
    if (nEIVL >= nValE) {
      nEIVL = nValE;
    } else {
      if (nEIVL < nValI) {
        nEIVL = nValI;
      }
    }

    // Draw
    if (lInit || nEIVL != nIVal && r) {
      nIVal = nEIVL;
      strcpy(cVal, u8g2_u8toa(nIVal, 3));

      u8g2.clearBuffer();
      u8g2.setFontPosTop();
      printTitle(cTitleV);
      u8g2.setFont(u8g2_font_7x13_tf);
      u8g2.drawUTF8(2, 22, cMens);
      u8g2.drawBox(45, 45, 30, 16);
      u8g2.drawUTF8(50, 47, cVal);
      u8g2.sendBuffer();
      lInit = false;
    }
  }
}

/**
 * Draw Internal Options
 * param char* cMens Message
 * param int nValA Valor atual
 * param int nValI Valor inicial
 * param int nValE Valor final
 */
int printInternalOptions(char *cMensI, char **aListI, int nOptionsI, int nOptSelI) {
  bool r = true;
  char *cTitleI;
  int nPosI = nOptSelI;
  int aOShowI[2] = { 0, 1 };
  nOptionsI--;
  bool lRefI = true;

  // Language
  switch (nSysL) {
    case 0:  // PT-BR
      cTitleI = "Escolha a Opção";
      break;
    case 1:  // EN-US
      cTitleI = "Select a Option";
      break;
  }

  // Loop Interno
  while (r) {
    encoderRead(true);

    // Return
    if(bESW){
      r = bESW = false;
      return;
    }

    // Show positions
    if (nPosI != nEIVL) {
      if (nEIVL > nOptionsI) {
        nEIVL = nPosI = nOptionsI;
      } else {
        if (nEIVL < 0) {
          nEIVL = nPosI = 0;
        } else {
          nPosI = nEIVL;
        }
      }
      lRefI = true;
    }

    // Draw
    if (lRefI) {
      if (nPosI > 1) {
        aOShowI[0] = (nPosI - 1);
        aOShowI[1] = nPosI;
      }

      u8g2.clearBuffer();
      u8g2.setFontPosTop();
      printTitle(cTitleI);
      u8g2.setFont(u8g2_font_7x13_tf);
      u8g2.drawUTF8(2, 17, cMensI);
      for (int y = 0; y <= min(nOptionsI, 1); y++) {
        if (nPosI == aOShowI[y]) {
          u8g2.drawBox(0, (16 * (y + 2)), 128, 16);
        }
        u8g2.drawUTF8(2, (16 * (y + 2) + 2), aListI[aOShowI[y]]);        
      }
      u8g2.sendBuffer();
    }
    lRefI = false;
  }

  return nEIVL;
}

/**
 * Draw saved screen
 * param nNRoute
 * param nNPos
 */
void printSaved(int nNRoute, int nNPos) {
  char *cMensS;

  // Language
  switch (nSysL) {
    case 0:  // PT-BR
      cMensS = "Salvo";
      break;
    case 1:  // EN-US
      cMensS = "Saved";
      break;
  }

  // Draw
  u8g2.clearBuffer();
  u8g2.setFontPosTop();
  u8g2.setFont(u8g2_font_8x13_tf);
  u8g2.drawUTF8(44, 25, cMensS);
  u8g2.sendBuffer();

  // Route to Screen
  delay(1000);
  screen(nNRoute, nNPos);
  return;
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
// Data functions ///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Load the configuration
 */
bool sysLoad() {
  // Read Language
  nSysL = EEPROM.read(0);

  // If not exists a language value
  //  burn default contents into EEPROM
  if (nSysL == 255) {
    sysDefault();
  }

  // Read Preset
  nSysP = EEPROM.read(1);

  //Serial.println("Loaded data");
  return true;
}

/**
 * Set all default values to system
 * return bool Sucess
 */
bool sysDefault() {
  int nValeton[] = {0,1,48,0,0,1,49,0,0,1,51,0,0,1,54,0,0,1,55,0,0,1,56,0,0,1,48,64,0,1,49,64,0,1,51,64,0,1,54,64,0,1,55,64,0,1,56,64};
  int nPodX3[]   = {0,1,48,0,0,1,49,0,0,1,51,0,0,1,54,0,0,1,55,0,0,1,56,0,0,1,48,64,0,1,49,64,0,1,51,64,0,1,54,64,0,1,55,64,0,1,56,64};

  // Language 0 - PT-BR
  nSysL = 0;
  EEPROM.update(0, nSysL);

  // Preset selected
  nSysP = 0;
  EEPROM.update(1, nSysP);

  // Presets do usuário
  for(int i = 0; i < 144; i++){
    EEPROM.update((i + 2), 0);
  }

  // Valeton
  for(int i = 0; i < 48; i++){
    EEPROM.update((i + 146), nValeton[i]);
  }

  // Valeton
  for(int i = 0; i < 48; i++){
    EEPROM.update((i + 194), nPodX3[i]);
  }

  //Serial.println("Reseted data");
  return true;
}

/**
 * Save the atual config to preset
 * param int p Preset to save
 */
bool savePreset(int p) {
  int x;

  // Set preset in EEPROM
  EEPROM.update(1, p);

  // Set FS data
  for (int b = 0; b <= 5; b++) {
    for (int a = 0; a <= 1; a++) {
      x = (b * 4 + (a * 24)) + (p * 48) + 2;
      EEPROM.update(x, nCH[b + (a * 6)]);
      EEPROM.update(x + 1, nCT[b + (a * 6)]);
      EEPROM.update(x + 2, nCC[b + (a * 6)]);
      EEPROM.update(x + 3, nVL[b + (a * 6)]);
    }
  }

  // Set memory preset
  nSysP = p;
  return true;
}

/**
 * Load preset data
 * param int p Preset to load
 */
bool loadPreset(int p) {
  int x;

  // Load FS data
  for (int b = 0; b <= 5; b++) {
    for (int a = 0; a <= 1; a++) {
      x = (b * 4 + (a * 24)) + (p * 48) + 2;
      nCH[b + (a * 6)] = EEPROM.read(x);
      nCT[b + (a * 6)] = EEPROM.read(x + 1);
      nCC[b + (a * 6)] = EEPROM.read(x + 2);
      nVL[b + (a * 6)] = EEPROM.read(x + 3);
    }
  }

  return true;
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