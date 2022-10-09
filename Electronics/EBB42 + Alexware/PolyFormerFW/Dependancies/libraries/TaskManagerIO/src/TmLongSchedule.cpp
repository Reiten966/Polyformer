/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry)..
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "TmLongSchedule.h"

#define HOURS_TO_MILLIS 3600000UL
#define MINUTES_TO_MILLIS 60000UL
#define SIX_MINUTES_TO_MILLIS 360000UL

uint32_t makeHourSchedule(int hours, int minutes, int seconds, int millis) {
    return (hours * HOURS_TO_MILLIS) + (minutes * MINUTES_TO_MILLIS) + (seconds * 1000) + millis;
}

uint32_t makeDaySchedule(int days, int hours) {
    return (days * 24UL * HOURS_TO_MILLIS) + (hours * MINUTES_TO_MILLIS);
}

TmLongSchedule::TmLongSchedule(uint32_t milliScheduleNext, Executable* toExecute) : milliSchedule(milliScheduleNext),
        fnCallback(nullptr), theExecutable(toExecute), lastScheduleTime(0) { }

TmLongSchedule::TmLongSchedule(uint32_t milliScheduleNext, TimerFn toExecute) : milliSchedule(milliScheduleNext),
        fnCallback(toExecute), theExecutable(nullptr), lastScheduleTime(0) { }

void TmLongSchedule::exec() {
    lastScheduleTime = millis();

    // never set last schedule time as 0. It is an invalid state.
    if(lastScheduleTime == 0) lastScheduleTime = 1;

    if(theExecutable != nullptr) {
        theExecutable->exec();
    }
    else if(fnCallback != nullptr) {
        fnCallback();
    }

}

uint32_t TmLongSchedule::timeOfNextCheck() {
    // initial state when schedule time is zero, we need avoid running the task at start up and most certainly should
    // not call millis in a global constructor, who knows what's initialised at that point.
    if(lastScheduleTime == 0) lastScheduleTime = millis();

    // Do not modify this code without fully understanding clock roll and unsigned values.
    uint32_t alreadyTaken = (millis() - lastScheduleTime);
    auto millisFromNow = (milliSchedule < alreadyTaken) ? 0 : ((milliSchedule - alreadyTaken));
    if(millisFromNow == 0) {
        // time to trigger, set the event as ready to fire and we'll wait out a full cycle
        setTriggered(true);
        millisFromNow = milliSchedule;
    }

    // we'll wait a maximum of six minutes in between testing again.
    return (millisFromNow > SIX_MINUTES_TO_MILLIS) ? SIX_MINUTES_TO_MILLIS : millisFromNow;
}
