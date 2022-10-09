

#ifndef TCMENUEXAMPLE_NTPTIMEEVENT_H
#define TCMENUEXAMPLE_NTPTIMEEVENT_H

#include <mbed.h>
#include <ctime>
#include <EthernetInterface.h>
#include <TaskManagerIO.h>

/**
 * Provides an easy way to get the time from an NTP server as an event, the exec method is not implemented on this
 * so that you can implement it how you see fit. This class works by starting a thread to acquire the time from an
 * NTP server, and when the time is obtained, it triggers the event.
 */
class NTPTimeEvent : public BaseEvent {
public:
    /**
     * Create the event for a given interface, server and port.
     * @param interface the interface to use
     * @param timeServer the NTP server name
     * @param timePort the port to connect on
     */
    NTPTimeEvent(NetworkInterface* interface, const char* timeServer, int timePort);

    uint32_t timeOfNextCheck() override;

private:
    const time_t EPOCH_CONVERT_OFFSET = (time_t)2208988800UL;
    const char* const  timeServer;
    const int volatile timePort;
    NetworkInterface* const interface;
    Thread ntpThread;

    void acquireNtpOnThread();
    friend void acquireNtpTimeThreadProc(NTPTimeEvent *ntpTime);
protected:
    volatile time_t _presentValue;
};


#endif //TCMENUEXAMPLE_NTPTIMEEVENT_H
