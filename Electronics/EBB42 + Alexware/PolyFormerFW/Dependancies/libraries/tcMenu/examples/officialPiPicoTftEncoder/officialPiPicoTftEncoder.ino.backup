#include "officialPiPicoTftEncoder_menu.h"
#include <PlatformDetermination.h>
#include <IoLogging.h>
#include <TaskManagerIO.h>

// TFT_eSPI setup is "Setup60_RP2040_ILI9341.h".

void setup() {
    Serial.begin(115200);
    while(!Serial);

    setupMenu();
}

void loop() {
    taskManager.runLoop();
}


void CALLBACK_FUNCTION onRestart(int /*id*/) {
    serdebugF("Restart selected");
}


void CALLBACK_FUNCTION onVolumeChanged(int id) {
    serdebugF2("Volume changed ", menuVolume.getCurrentValue());
}
