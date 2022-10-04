#define DRIVER_ADDRESS    0b00    // TMC2209 Driver address
#define R_SENSE           0 //.11f   // SilentStepStick series use 0.11
int microstep;

//#define RMS_CURRENT       600     // Motor RMS current in mA
//#define MICROSTEPS        0       // Microsteps; note that MicroPlyer will interpolate to 256
#define SPREADCYCLE       false   // Spreadcycle can have higher RPM but is louder


HardwareSerial Serial2(PA15, PA14_ALT1);

TMC2209Stepper driver(&Serial2, R_SENSE, DRIVER_ADDRESS);   // Create TMC driver

void stepperSetup()
{
  pinMode(stepperEnPin, OUTPUT);

  Serial2.begin(115200);
  USART2->CR1 &= ~USART_CR1_UE; // UE = 0... disable USART
  USART2->CR2 |= USART_CR2_SWAP; //Swap TX/RX pins2
  USART2->CR3 |= USART_CR3_HDSEL; //Set Half-duplex selection
  USART2->CR1 |= USART_CR1_UE; // UE = 1... Enable USART

  digitalWrite(stepperEnPin, LOW);            // Enable TMC2209 board

  driver.begin();
  driver.toff(5);                       // Enables driver in software
  driver.internal_Rsense(true);         // User Internal sense resistors.
  //  driver.rms_current(RMS_CURRENT);      // Set motor RMS current (mA)
  stepperMicrosteps();
  stepperCurrent();
  driver.en_spreadCycle(SPREADCYCLE);   // Toggle spreadCycle on TMC2208/2209/2224
  driver.pwm_autoscale(true);           // Needed for stealthChop
}

void stepperSppeed()
{ 
  driver.shaft(menuReverse.getCurrentValue()); // SET DIRECTION
  stepperMicrosteps();
  int32_t dest_speed;
  if (runSystem) {
    if (microstep > 0) {
      dest_speed = microstep * menuFeed.getAsFloatingPointValue() * menuSettingsMotorSteps.getAsFloatingPointValue() * menuSettingsGearboxRatio.getAsFloatingPointValue() / (3.1459 * menuSettingsSpoolRadius.getAsFloatingPointValue());
    } else {
      dest_speed =             menuFeed.getAsFloatingPointValue() * menuSettingsMotorSteps.getAsFloatingPointValue() * menuSettingsGearboxRatio.getAsFloatingPointValue() / (3.1459 * menuSettingsSpoolRadius.getAsFloatingPointValue());
    }
  } else {
    dest_speed = 0;
  }
  Serial.println("dest_speed: ");
  Serial.println(dest_speed);
  driver.VACTUAL(dest_speed);
}

void stepperMicrosteps() {
  //  uint16_t currentValue = menuItem.getCurrentValue();
  switch (menuSettingsGearboxMicrosteps.getCurrentValue()) {
    case 1:
      microstep = 2;
      // statements
      break;
    case 2:
      microstep = 4;
      // statements
      break;
    case 3:
      microstep = 8;
      // statements
      break;
    case 4:
      microstep = 16;
      // statements
      break;
    case 5:
      microstep = 32;
      // statements
      break;
    case 6:
      microstep = 64;
      // statements
      break;
    default:
      microstep = 0;
      // statements
      break;
  }
  Serial.print("Microsteps: ");
  Serial.println(microstep);
  driver.microsteps(microstep);        // Set microsteps
}

void stepperCurrent() {
  driver.rms_current(menuSettingsGearboxMotorCurrent.getAsFloatingPointValue());      // Set motor RMS current (mA)
}
