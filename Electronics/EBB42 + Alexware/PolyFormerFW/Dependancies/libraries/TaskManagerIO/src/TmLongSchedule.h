/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry)..
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TASKMANGER_IO_TMLONGSCHEDULE_H
#define TASKMANGER_IO_TMLONGSCHEDULE_H

#include <TaskManagerIO.h>

/**
 * A task manager task that can be scheduled safely in hours and days. If you need more than this, you'll probably need to
 * write your own event based on a real time clock's time of day. You could use this event as the starting point.
 */
class TmLongSchedule : public BaseEvent {
private:
    const uint32_t milliSchedule;
    const TimerFn fnCallback;
    Executable *const theExecutable;
    uint32_t lastScheduleTime;
public:
    /** Create a schedule that will call back a TimerFn functional callback.
     * @param milliSchedule the schedule to call back on
     * @param callee the functional callback
     */
    TmLongSchedule(uint32_t milliSchedule, TimerFn callee);
    /**
     * Create schedule that will call the exec() method on an Executable
     * @param milliSchedule the schedule to call back on
     * @param callee the object extending from Executable
     */
    TmLongSchedule(uint32_t milliSchedule, Executable* callee);

    void exec() override;

    uint32_t timeOfNextCheck() override;
};

/**
 * Make a schedule based around hours for a TmLongSchedule
 * @param hours the number of hours
 * @param minutes the number of minutes within the hour
 * @param seconds the number of seconds within the minute
 * @param millis  the number of millis within the second
 * @return the schedule value
 */
uint32_t makeHourSchedule(int hours, int minutes = 0, int seconds = 0, int millis = 0);

/**
 * Make a schedule for daily execution in days
 * @param days the number of days.
 * @return the schedule value
 */
uint32_t makeDaySchedule(int days, int hours = 0);

#endif //TASKMANGER_IO_TMLONGSCHEDULE_H
