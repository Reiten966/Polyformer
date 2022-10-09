#ifndef TCLIBRARYDEV_MOTIONDETECTION_H
#define TCLIBRARYDEV_MOTIONDETECTION_H

#include <TaskManager.h>
#include <Arduino_LSM9DS1.h>
#include "nano33ble_menu.h"

/**
 * Here we create a polling event that checks if the acceleration / magnetic data is available, and whenever it is
 * it triggers the event. This tidies up the class slightly by avoiding if statements in the exec() methods.
 * In short, the timeOfNextCheck method is called every `eventFrequency` micros, and we tell taskManager that the
 * event is triggered when both readings are available. At this point `exec()` is called and we take the readings
 * and put then into menu items.
 */
class MotionDetection : public BaseEvent {
private:
    uint32_t eventFrequency = 1000UL;
public:
    void initialise() {
        IMU.begin();
        auto imuRate = IMU.magneticFieldSampleRate();
        eventFrequency = min(int(imuRate), 100) * 1000UL;
    }

    uint32_t timeOfNextCheck() override {
        setTriggered(IMU.magneticFieldAvailable() && IMU.accelerationAvailable());

        return eventFrequency;
    }

    void exec() override {
        float x, y, z;
        IMU.readMagneticField(x, y, z);
        menuAccelerometerMagX.setFloatValue(x);
        menuAccelerometerMagY.setFloatValue(y);
        menuAccelerometerMagZ.setFloatValue(z);

        IMU.readAcceleration(x, y, z);
        menuAccelerometerAccelX.setFloatValue(x);
        menuAccelerometerAccelY.setFloatValue(y);
        menuAccelerometerAccelZ.setFloatValue(z);
    }
};

#endif //TCLIBRARYDEV_MOTIONDETECTION_H
