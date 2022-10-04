#include <PID_v1.h>
#include "pidautotuner.h"
unsigned long  pidTime = 0;
int pidLoopInterval = 100; //milliseconds
long milliseconds;
PIDAutotuner tuner = PIDAutotuner();

//Define Variables we'll be connecting to
double Input, Output;
//float Kp = 33.54, Ki = 0.89, Kd = 1285.85;

//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint, menuKp.getAsFloatingPointValue(), menuKi.getAsFloatingPointValue(), menuKd.getAsFloatingPointValue(), P_ON_M, DIRECT); //P_ON_M specifies that Proportional on Measurement be used
//P_ON_E (Proportional on Error) is the default behavior

void pidSetup()
{
  //initialize the variables we're linked to
  Input = therm.getTemp();
  Setpoint = menuTemperature.getAsFloatingPointValue();

  //apply PID gains
  pidChange();

  //turn the PID on
  myPID.SetMode(AUTOMATIC);
}

void pidLoop()
{
  if (runSystem) {
    if (millis() > pidTime)
    {
      Input = therm.getTemp();
      myPID.Compute();
      analogWrite(heaterPin, Output);
      pidTime += 100;
    }
  } else {
    analogWrite(heaterPin, 0);
  }
}

void pidChange() {
  //apply PID gains
  Serial.print("Kp: "); Serial.print(tuner.getKp()); Serial.print(" ");
  Serial.print("Ki: "); Serial.print(tuner.getKi()); Serial.print(" ");
  Serial.print("Kd: "); Serial.println(tuner.getKd());
  myPID.SetTunings(menuKp.getAsFloatingPointValue(), menuKi.getAsFloatingPointValue(), menuKd.getAsFloatingPointValue());
}

void pidTune() {
  runSystem = false; //Stop system before running tune process
  pidTuneRun = true; //Mark PID tuning as a process
  //  PIDAutotuner tuner = PIDAutotuner();

  // Set the target value to tune to
  // This will depend on what you are tuning. This should be set to a value within
  // the usual range of the setpoint. For low-inertia systems, values at the lower
  // end of this range usually give better results. For anything else, start with a
  // value at the middle of the range.
  tuner.setTargetInputValue(menuTemperature.getAsFloatingPointValue());

  // Set the loop interval in milliseconds
  // This must be the same as the interval the PID control loop will run at
  tuner.setpidLoopInterval(pidLoopInterval);

  // Set the output range
  // These are the minimum and maximum possible output values of whatever you are
  // using to control the system (Arduino analogWrite, for example, is 0-255)
  tuner.setOutputRange(0, 255);

  // Set the Ziegler-Nichols tuning mode
  // Set it to either PIDAutotuner::ZNModeBasicPID, PIDAutotuner::ZNModeLessOvershoot,
  // or PIDAutotuner::ZNModeNoOvershoot. Defaults to ZNModeNoOvershoot as it is the
  // safest option.
  tuner.setZNMode(PIDAutotuner::ZNModeNoOvershoot);

  // here is a brief example of how to show a dialog, usually for information
  // or yes/no answers.
//  auto* dlg = renderer.getDialog();
//    if (dlg && !dlg->isInUse()) {
//      dlg->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
//      dlg->show(pgmTuning, false);
//      dlg->copyIntoBuffer("Proceed with Tune?");
//    }

  // This must be called immediately before the tuning loop
  // Must be called with the current time in milliseconds
  tuner.startTuningLoop(millis());
  //  pidTuneLoop();

  // Run a loop until tuner.isFinished() returns true
  while (!tuner.isFinished()) { //-----------------------------------------------While loop
    // This loop must run at the same speed as the PID control loop being tuned
    long prevMilliseconds = milliseconds;
    milliseconds = millis();

    // Get input value here (temperature, encoder position, velocity, etc)
    Input = therm.getTemp();

    // Call tunePID() with the input value and current time in milliseconds
    double output = tuner.tunePID(Input, milliseconds);

    // Set the output - tunePid() will return values within the range configured
    // by setOutputRange(). Don't change the value or the tuning results will be
    // incorrect.
    analogWrite(heaterPin, output);

    // here is a brief example of how to show a dialog, usually for information
    // or yes/no answers.

    Serial.print("Temperature: "); Serial.print(Input); Serial.print(" Output: "); Serial.println(output);

    // This loop must run at the same speed as the PID control loop being tuned
    while (millis() - milliseconds < pidLoopInterval) delayMicroseconds(1);
  }

  auto* dlg = renderer.getDialog();
  if (dlg && !dlg->isInUse()) {
    dlg->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
    dlg->show(pgmTuning, false);
    dlg->copyIntoBuffer("Tuned");
  }

  //-----------------------------------------------While loop

  // Turn the output off here.
  analogWrite(heaterPin, 0);

  Serial.print("Kp: "); Serial.print(tuner.getKp()); Serial.print(" ");
  Serial.print("Ki: "); Serial.print(tuner.getKi()); Serial.print(" ");
  Serial.print("Kd: "); Serial.println(tuner.getKd());

  //Set PID Values to RAM
  menuKp.setFromFloatingPointValue(tuner.getKp());
  menuKi.setFromFloatingPointValue(tuner.getKi());
  menuKd.setFromFloatingPointValue(tuner.getKd());
  pidChange();
}

//void pidTuneLoop() {
//  // Run a loop until tuner.isFinished() returns true
//  //    while (!tuner.isFinished()) { //-----------------------------------------------While loop
//
//  // This loop must run at the same speed as the PID control loop being tuned
//  long prevMilliseconds = milliseconds;
//  milliseconds = millis();
//
//  // Get input value here (temperature, encoder position, velocity, etc)
//  Input = therm.getTemp();
//
//  // Call tunePID() with the input value and current time in milliseconds
//  double output = tuner.tunePID(Input, milliseconds);
//
//  // Set the output - tunePid() will return values within the range configured
//  // by setOutputRange(). Don't change the value or the tuning results will be
//  // incorrect.
//  analogWrite(heaterPin, output);
//
//  Serial.print("Temperature: "); Serial.print(Input); Serial.print(" Output: "); Serial.println(output);
//
//  // This loop must run at the same speed as the PID control loop being tuned
//  while (millis() - milliseconds < pidLoopInterval) delayMicroseconds(1);
//  //    }
//
//  if (!tuner.isFinished()) { //Update PID tuning in RAM
//
//  // here is a brief example of how to show a dialog, usually for information
//  // or yes/no answers.
//  auto* dlg = renderer.getDialog();
//  if (dlg && !dlg->isInUse()) {
//    dlg->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
//    dlg->show(pgmTuning, false);
//    dlg->copyIntoBuffer("Tuned");
//  }
//
//    // Turn the output off here.
//    analogWrite(heaterPin, 0);
//
//    //Set PID Values to RAM
//    menuKp.setFromFloatingPointValue(tuner.getKp());
//    menuKi.setFromFloatingPointValue(tuner.getKi());
//    menuKd.setFromFloatingPointValue(tuner.getKd());
//    pidTuneRun = false; //We have finished PID tuning now
//    pidChange();
//  }
//}
