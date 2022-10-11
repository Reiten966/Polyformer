/**
 * A simple example of how to use either XPT2046 or FT6206 libraries for touch screen with IoAbstraction.
 * This example shows how to use the adafruit FT6206 or XPT2046 library with this touch screen.
 * This example takes the readings and reports them to the serial port every few hundred millis.
 * It's possible to extend the touch base class too and be event-driven, but that's not discussed here.
 *
 * Why use this rather than the libraries directly:
 *  * You're using the touch screen with tcMenu.
 *  * You need to handle more complex cases such as when an item is held (with repeat).
 *  * You need calibration or more complex integrations offered by extending the Touch Manager class.
 */

#include <IoLogging.h>
#include <SwitchInput.h>
#include <ResistiveTouchScreen.h>

// largest possible value returned from library (usually 4096).
#define KNOWN_DEVICE_TOUCH_RANGE 4096.0F

//
// Pick a library from the two below and uncomment the lines for that library. This includes the appropriate library
// header, and creates an instance of the library touch object.
//

// For Paul Stoffregen's touch screen XPT2046 (or ThingPulse fork)
#include <XPT2046_Touchscreen.h>
#define TOUCH_CLASS XPT2046_Touchscreen
#define CS_PIN 5
XPT2046_Touchscreen touchDevice(CS_PIN, 0xFF);
//end XPT2046

// For Adafruit's FT6206 touch screen library
//#include <Adafruit_FT6206>
//#define TOUCH_CLASS Adafruit_FT6206
//Adafruit_FT6206 touchDevice;
// end FT6206


using namespace iotouch;

/**
 * Here we implement the glue between the IoAbstraction touch screen and the library. It is essentially one method
 * that is called frequently to determine the state of the touch screen called `internalProcessTouch`. We create a
 * single instance of this class and pass it to the `ResistiveTouchScreen` when it's created.
 */
class AdaLibTouchInterrogator : public TouchInterrogator {
private:
    TOUCH_CLASS& theTouchDevice;
public:
    AdaLibTouchInterrogator(TOUCH_CLASS& touchLibRef) : theTouchDevice(touchLibRef) {}

    // This method is called frequently by the touch manager, it is basically asking "is there a touch right now".
    // How this method should be implemented:
    // * if there is no touch, it should return NOT_TOUCHED.
    // * if there is a touch, the reading should be obtained, and converted into floating points value between 0 and 1
    // * these x and y values should be run through the calibrator and used to set the pointers to x and y passed in
    // * lastly you should return TOUCHED to indicate a touch as taken place.
    TouchState internalProcessTouch(float *ptrX, float *ptrY, TouchRotation rotation, const CalibrationHandler& calib) {
        if(theTouchDevice.touched() == 0) return NOT_TOUCHED;

        TS_Point pt = theTouchDevice.getPoint();

        *ptrX = calib.calibrateX(float(pt.x) / KNOWN_DEVICE_TOUCH_RANGE, false);
        *ptrY = calib.calibrateY(float(pt.y) / KNOWN_DEVICE_TOUCH_RANGE, false);
        return TOUCHED;
    }
} interrogator(touchDevice);

/**
 * Now we create the resistive touch screen instance, this is the class within IoAbstraction that handles the touch
 * interface. In the simplest case you can use the ValueStoringResistiveTouchScreen, but you can also extend from
 * ResistiveTouchScreen, see the reference documentation for more on this.
 *
 * Notice that we pass in the above created "glue" interrogator and the desired rotation.
 */
ValueStoringResistiveTouchScreen touchScreen(interrogator, TouchInterrogator::PORTRAIT);

void setup() {
    Serial.begin(115200);
    // first start the underlying touch library
    touchDevice.begin();

    // step 1. run with calibration off and get the actual min and max values if corrections need to be made
    // step 2. put the corrections into the value below, xmin, xmax, ymin, ymax and try the program again.
    touchScreen.calibrateMinMaxValues(0.15F, 0.75F, 0.06F, 0.91F);
    touchScreen.start();

    taskManager.scheduleFixedRate(500, [] {
        // be sure to enable IO logging, see IoLogging.h
        serdebugF4("Touch: x, y, touched", touchScreen.getLastX(), touchScreen.getLastY(), touchScreen.getTouchPressure());
        serdebugF2("touchMode: ", touchScreen.getTouchState());
    });
}

void loop() {
    taskManager.runLoop();
}