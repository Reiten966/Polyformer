//
// Created by David Cherry on 25/06/2020.
//

#include <IoLogging.h>
#include "NTPTimeEvent.h"

void acquireNtpTimeThreadProc(NTPTimeEvent *ntpTime) {
    ntpTime->acquireNtpOnThread();
}

NTPTimeEvent::NTPTimeEvent(NetworkInterface *nwInterface, const char *serverName, int port)
        : timeServer(serverName), timePort(port), interface(nwInterface),
          ntpThread(), _presentValue(0) {
    ntpThread.start(callback(acquireNtpTimeThreadProc, this));
}

void NTPTimeEvent::acquireNtpOnThread() {
    int retriesLeft = 20;
    while(--retriesLeft > 0) {
        // wait a second before trying to avoid a tight loop.
        ThisThread::sleep_for((20 - retriesLeft) * 1000);

        serdebugF2("Starting ntp loop retries = ", retriesLeft);

        SocketAddress serverAddr;
        if (interface->gethostbyname(timeServer, &serverAddr) < 0) {
            continue;
        }
        serverAddr.set_port(timePort);

        serdebugF("Found NTP host");

        int32_t ntpRx[12] = {0};
        char ntpTx[48] = {0};
        ntpTx[0] = 0x1b;

        UDPSocket socket;
        socket.open(interface);
        socket.set_blocking(true);
        socket.set_timeout(10);

        int sendCount = 0;
        nsapi_size_or_error_t sendRet;
        while((sendRet = socket.sendto(serverAddr, ntpTx, sizeof(ntpTx))) <= 0) {
            if(sendRet != NSAPI_ERROR_WOULD_BLOCK || ++sendCount > 20) break;
            ThisThread::sleep_for(500);
        }

        serdebugF("Sent NTP message");

        SocketAddress sourceAddr;
        sendCount = 0;
        while(sendRet > 0 && ++sendCount < 20) {
            int n = socket.recvfrom(&sourceAddr, ntpRx, sizeof(ntpRx));

            if (n > 10) {
                uint32_t tmNowRaw = ntpRx[10];
                uint32_t ret = (tmNowRaw & 0xffU) << 24U;
                ret |= (tmNowRaw & 0xff00U) << 8U;
                ret |= (tmNowRaw & 0xff0000UL) >> 8U;
                ret |= (tmNowRaw & 0xff000000UL) >> 24U;
                _presentValue = ret - EPOCH_CONVERT_OFFSET;
                markTriggeredAndNotify();
                serdebugF2("Time was set to ", (unsigned long) _presentValue);
                return;
            }
        }
        // finally close the socket
        socket.close();
    }
}

uint32_t NTPTimeEvent::timeOfNextCheck() {
    return secondsToMicros(1);
}
