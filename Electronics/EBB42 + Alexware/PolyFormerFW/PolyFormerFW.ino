//Install the following libraries from the arduino Library manager
//TCMenu 2.3.1
//U8g2lib 2.33.15
//TMCStepper 0.7.3
//GyverNTC 1.2.0
//PID 1.2.0

//Add STM32Duino
//https://github.com/stm32duino/wiki/wiki/Getting-Started
//Use version 2.4.0 (If not available yet, use the main branch found here https://github.com/stm32duino/Arduino_Core_STM32), this adds support for the EBB42


#include "PolyFormerFW_menu.h"
#include "TMCStepper.h"           // TMCstepper - https://github.com/teemuatlut/TMCStepper
const char pgmCommittedToRom[] PROGMEM = "Saved to ROM";
const char pgmCommittedToRomMessage[] PROGMEM = "just so you know";
const char pgmTuning[] PROGMEM = "Tuning PID";

//Hardware Variables
#define baudrate               9600
#define REFERENCE_RESISTANCE   4700
#define NOMINAL_RESISTANCE     100000
#define NOMINAL_TEMPERATURE    25
#define B_VALUE                3950

#define fwVersion              0.09

//System Variables
int error = 0; //Start with a clean sheet and no error /* 0=no error; 1=Heater decoupled during rising; 2=Heater decoupled during hold; 10=Over Temp; 11=Thermistor short; 12=No thermistor */
bool runSystem = false;
bool pidTuneRun = false;
double Setpoint;

//BlackPill
//#define SDAPin          PB9
//#define SCLPin          PB8
//#define stepperStepPin  PB0
//#define stepperDirPin   PB7
//#define stepperEnPin    PB5
//#define thermistorPin   PA2
//#define heaterPin
//#define meltzoneFanPin  PA6


////EBB42 v1.1
#define SDAPin          PB4
#define SCLPin          PB3
#define stepperEnPin    PD2
#define thermistorPin   PA3
#define heaterPin       PA2
#define meltzoneFanPin  PA0
#define encA            PB5
#define encB            PB7
#define encSW           PB6



void setup() {
  // If you use i2c and serial devices, be sure to start wire / serial.
  Wire.setSDA(SDAPin); //BlackPill
  Wire.setSCL(SCLPin); //BlackPill
  Wire.begin();

  // This is added by tcMenu Designer automatically during the first setup.
  setupMenu();

  // lastly we load state from EEPROM.
  menuMgr.load();
  serialSetup();
  stepperSetup();
  fanSetup();
  pidSetup();
}

void loop() {
  taskManager.runLoop();
  SerialLoop();
  fanLoop();
  //  // Run a loop until tuner.isFinished() returns true
  //  if (pidTuneRun) {
  //    pidTuneLoop();
  //  }else{
  pidLoop();
  //  }
}


void CALLBACK_FUNCTION onSaveSettings(int id) {
  menuMgr.save();

  // here is a brief example of how to show a dialog, usually for information
  // or yes/no answers.
  auto* dlg = renderer.getDialog();
  if (dlg && !dlg->isInUse()) {
    dlg->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
    dlg->show(pgmCommittedToRom, false);
    dlg->copyIntoBuffer(pgmCommittedToRomMessage);
  }
}

void CALLBACK_FUNCTION onPIDTune(int id) {
  // TODO - your menu change code
  pidTune();
}

void CALLBACK_FUNCTION onStart(int id) {
  runSystem = !runSystem;
  stepperSppeed();
}

void CALLBACK_FUNCTION onGearboxChange(int id) {
  stepperSppeed();
}


void CALLBACK_FUNCTION onNameChanged(int id) {
  // TODO - your menu change code
}



void CALLBACK_FUNCTION onMotorCurrent(int id) {
  stepperCurrent();
}


void CALLBACK_FUNCTION onPidChange(int id) {
  pidChange();
}


void CALLBACK_FUNCTION onTemperatureChange(int id) {
  Setpoint = menuTemperature.getAsFloatingPointValue();
}
