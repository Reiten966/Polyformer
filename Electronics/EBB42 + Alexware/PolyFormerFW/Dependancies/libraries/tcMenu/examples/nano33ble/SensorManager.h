#ifndef TCLIBRARYDEV_SENSORMANAGER_H
#define TCLIBRARYDEV_SENSORMANAGER_H

#include <TaskManager.h>
#include <Arduino_HTS221.h>
#include <Arduino_LPS22HB.h>
#include <MenuItems.h>
#include "nano33ble_menu.h"

/**
 * Here we have a class that extends `Executable`, meaning that the `exec()` method is called every time the event
 * is scheduled by task manager. We read the data from the sensors and set them onto menu items.
 */
class SensorManager : public Executable {
private:
    bool initialised{};
public:
    void initialise() {
        initialised = HTS.begin()  != 0;
        initialised = initialised && BARO.begin();
    }

    void exec() override {
        menuTemp.setFromFloatingPointValue(HTS.readTemperature());
        menuHumidity.setFromFloatingPointValue(HTS.readHumidity());
        menuBPressure.setFromFloatingPointValue(BARO.readPressure());
    }
};

#endif //TCLIBRARYDEV_SENSORMANAGER_H
