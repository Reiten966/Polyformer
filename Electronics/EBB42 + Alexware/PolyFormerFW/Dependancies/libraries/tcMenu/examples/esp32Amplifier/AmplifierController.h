#ifndef TCMENU_ESPAMPLIFIER_AMPLIFIERCONTROLLER_H
#define TCMENU_ESPAMPLIFIER_AMPLIFIERCONTROLLER_H

#include <Arduino.h>
#include <tcMenu.h>
#include <IoLogging.h>
#include "esp32Amplifier_menu.h"

#ifndef MENU_MAGIC_KEY
#define MENU_MAGIC_KEY 0xfade
#endif // MENU_MAGIC_KEY

#ifndef NUM_CHANNELS
#define NUM_CHANNELS 4
#endif // NUM_CHANNELS

#define EEPROM_TRIM_POS 140

//                                             1234567890123456 1234567890123456 1234567890123456 1234567890123456
const char pgmDefaultChannelNames[] PROGMEM = "Turntable\0      Auxiliary\0      USB Audio\0      Storage\0";


class AmplifierController  {
public:
    enum AmplifierStatus {
        WARMING_UP,
        VALVE_WARM_UP,
        RUNNING,
        DC_PROTECTION,
        OVERLOADED,
        OVERHEATED
    };
private:
    bool audioDirect;
    bool muted = true;
    int levelTrims[NUM_CHANNELS];
public:
    explicit AmplifierController() : levelTrims{} {
        menuMute.setBoolean(true);
        setAmpStatus(WARMING_UP);
    }

    void initialise() {
        for(int i=0; i<NUM_CHANNELS; i++) {
            levelTrims[i] = menuMgr.getEepromAbstraction()->read8(EEPROM_TRIM_POS + i);
        }
    }

    uint8_t getChannelInt() {
        uint8_t ch = menuChannels.getCurrentValue();
        if(ch > NUM_CHANNELS) return 0;
        return ch;
    }

    void setAmpStatus(AmplifierStatus status) {
        menuStatusAmpStatus.setCurrentValue(status);
    }

    AmplifierStatus getAmpStatus() {
        return static_cast<AmplifierStatus>(menuStatusAmpStatus.getCurrentValue());
    }

    void onVolumeChanged() {
        auto trim = levelTrims[getChannelInt()];
        auto vol = menuVolume.getCurrentValue() + trim;
        auto volToWrite = menuMute.getBoolean() ? 0 : vol;
        serdebugF2("write volume to bus", volToWrite);
    }

    void onChannelChanged() {
        auto ch = (muted) ? 0 : (1 << getChannelInt());
        onVolumeChanged();
        serdebugF2("write channel to bus", ch);
    }

    void onAudioDirect(bool direct) {
        audioDirect = direct;
    }

    void onMute(bool mute) {
        muted = mute;
    }

    int getTrim(uint8_t channel) {
        if(channel >= NUM_CHANNELS) return 0;
        return levelTrims[channel];
    }

    void setTrim(uint8_t channel, int newTrim) {
        if(channel >= NUM_CHANNELS) return;
        levelTrims[channel] = newTrim;
        menuMgr.getEepromAbstraction()->write8(EEPROM_TRIM_POS + channel, newTrim);
    }

};

#endif //TCMENU_ESPAMPLIFIER_AMPLIFIERCONTROLLER_H
