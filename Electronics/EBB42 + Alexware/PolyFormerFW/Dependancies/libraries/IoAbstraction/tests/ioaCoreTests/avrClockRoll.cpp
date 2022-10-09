#include <AUnit.h>

#if defined(__AVR__)

#include <util/atomic.h>
#include "IoAbstraction.h"
#include "MockIoAbstraction.h"

using namespace aunit;

// We can only reset the clock to a new value on AVR, this is very useful and allows us to ensure the
// rollover cases work properly at least for milliseconds. As millisecond and microsecond logic are very
// similar it gives some degree of confidence that it's working properly.
//
// Keep this test on it's own in this package. It messes around with the millisecond counter.

void setMillis(unsigned long ms)
{
    extern unsigned long timer0_millis;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        timer0_millis = ms;
    }
}

void dumpTaskTiming() {
    Serial.println("Showing task timings");
    TimerTask* task = taskManager.getFirstTask();
    while(task) {
        Serial.print(" - Timing "); Serial.println(task->microsFromNow());
        task = task->getNext();
    }
}

int avrCount1 = 0;
int avrCount2 = 0;

//
// this test only runs on AVR - it sets the timer near to overflow and schedules some tasks
//
test(testClockRollover) {
    avrCount1 = avrCount2 = 0;

    // set the clock so that it will roll
    uint32_t oldMillis = millis();
    setMillis(0xfffffe70UL);

    taskManager.scheduleOnce(1, [] {
        avrCount1++;
    }, TIME_SECONDS);

    taskManager.scheduleFixedRate(250, [] {
        avrCount2++;
    }, TIME_MICROS);

    // make sure it's still to wrap.
    assertTrue(millis() > 100000000UL);

    // now run the loop
    dumpTaskTiming();
    unsigned long start = millis();
    while(avrCount1 == 0 && (millis() - start) < 5000) {
        taskManager.yieldForMicros(10000);
    }

    dumpTaskTiming();

    // the one second task should have executed exactly once.
    assertEqual(avrCount1, 1);
    assertMore(avrCount2, 1000);

    // make sure millis has wrapped now.
    assertTrue(millis() < 10000UL);

    // and make sure the microsecond job is still going..
    int avrCount2Then = avrCount2;
    taskManager.yieldForMicros(10000);
    assertTrue(avrCount2Then != avrCount2);

    // reset the millisecond timer where it was before.
    setMillis(oldMillis);
}
#endif // __AVR__
