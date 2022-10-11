#include <AUnit.h>
#include <IoAbstractionWire.h>
#include <MockIoAbstraction.h>

test(testMockIoAbstractionRead) {
    MockedIoAbstraction ioDevice;

    for(int i=0; i<16; i++) {
        ioDevicePinMode(&ioDevice, i, INPUT);
    }

    ioDevice.setValueForReading(0, 0xeeff);
    ioDevice.setValueForReading(1, 0x1100);
    ioDevice.setValueForReading(2, 0xaa55);
    ioDevice.setValueForReading(3, 0x1100);

    assertTrue(ioDeviceDigitalRead(&ioDevice, 1));
    ioDeviceSync(&ioDevice);
    assertFalse(ioDeviceDigitalRead(&ioDevice, 1));
    ioDeviceSync(&ioDevice);

    assertEqual((uint8_t)0x55, ioDeviceDigitalReadPort(&ioDevice, 1));
    assertEqual((uint8_t)0xaa, ioDeviceDigitalReadPort(&ioDevice, 9));

    ioDeviceSync(&ioDevice);

    assertEqual((uint8_t)0x00, ioDeviceDigitalReadPort(&ioDevice, 1));
    assertEqual((uint8_t)0x11, ioDeviceDigitalReadPort(&ioDevice, 9));
    
    // now test error handling.
    assertEqual(ioDevice.getErrorMode(), NO_ERROR);

    ioDevicePinMode(&ioDevice, 1, OUTPUT);
    ioDevicePinMode(&ioDevice, 9, OUTPUT);
    ioDeviceDigitalRead(&ioDevice, 1);
    assertEqual(ioDevice.getErrorMode(), READ_NOT_INPUT);
    ioDevice.clearError();
    ioDeviceDigitalReadPort(&ioDevice, 1);
    assertEqual(ioDevice.getErrorMode(), READ_NOT_INPUT);
    ioDevice.clearError();
    ioDeviceDigitalReadPort(&ioDevice, 9);
    assertEqual(ioDevice.getErrorMode(), READ_NOT_INPUT);

}

test(testMockIoAbstractionWrite) {
    MockedIoAbstraction ioDevice;

    for(int i=0; i<16; i++) {
        ioDevicePinMode(&ioDevice, i, OUTPUT);
    }
 
    // test port manipulation.
    ioDeviceDigitalWritePort(&ioDevice, 0, 0xaa);
    ioDeviceDigitalWritePortS(&ioDevice, 9, 0x55);
    assertEqual((uint16_t)0x55aa, ioDevice.getWrittenValue(1));
    ioDeviceDigitalWritePortS(&ioDevice, 9, 0xbb);
    assertEqual((uint16_t)0xbbaa, ioDevice.getWrittenValue(2));
    ioDeviceDigitalWritePortS(&ioDevice, 0, 0xdd);
    assertEqual((uint16_t)0xbbdd, ioDevice.getWrittenValue(3));
    assertEqual(3, ioDevice.getNumberOfRunLoops());

    // test pin manipulation.
    for(int i = 0; i<16; i++) {
        ioDeviceDigitalWrite(&ioDevice, i, HIGH);
    }
    assertEqual((uint16_t)0xffff, ioDevice.getWrittenValue(3));

    // test error handling for writes
    assertEqual(ioDevice.getErrorMode(), NO_ERROR);
    ioDevicePinMode(&ioDevice, 1, INPUT);
    ioDevicePinMode(&ioDevice, 9, INPUT);
    ioDeviceDigitalWrite(&ioDevice, 1, HIGH);
    assertEqual(ioDevice.getErrorMode(), WRITE_NOT_OUTPUT);

    ioDevice.clearError();
    ioDeviceDigitalWritePort(&ioDevice, 1, 0xff);
    assertEqual(ioDevice.getErrorMode(), WRITE_NOT_OUTPUT);

    ioDevice.clearError();
    ioDeviceDigitalWritePort(&ioDevice, 9, 0xff);
    assertEqual(ioDevice.getErrorMode(), WRITE_NOT_OUTPUT);
}

MockedIoAbstraction ioDevice1;
MockedIoAbstraction ioDevice2;
MultiIoAbstraction multiIo(100);
void setupMultiTest()  {
    ioDevice1.setValueForReading(0, 0x0001);
    ioDevice1.setValueForReading(1, 0x0002);
    ioDevice1.setValueForReading(2, 0x0003);
    ioDevice2.setValueForReading(0, 0x0010);
    ioDevice2.setValueForReading(1, 0x0020);
    ioDevice2.setValueForReading(2, 0x0030);

    multiIo.addIoExpander(&ioDevice1, 20);
    multiIo.addIoExpander(&ioDevice2, 20);
}

test(testMultiIoPassThrough) {
    setupMultiTest();

    for(int i=0; i<8; i++) {
        ioDevicePinMode(&multiIo, 100 + i, INPUT);
        ioDevicePinMode(&multiIo, 120 + i, INPUT);
        ioDevicePinMode(&multiIo, 108 + i, OUTPUT);
        ioDevicePinMode(&multiIo, 128 + i, OUTPUT);
    }

    assertTrue(ioDeviceDigitalRead(&multiIo, 100));
    assertFalse(ioDeviceDigitalRead(&multiIo, 120));
    assertEqual((uint8_t)0x01, ioDeviceDigitalReadPort(&multiIo, 101));
    assertEqual((uint8_t)0x10, ioDeviceDigitalReadPort(&multiIo, 121));
    assertFalse(ioDeviceDigitalReadS(&multiIo, 100));
    assertTrue(ioDeviceDigitalRead(&multiIo, 101));
    assertEqual((uint8_t)0x30, ioDeviceDigitalReadPortS(&multiIo, 121));
    assertTrue(ioDeviceDigitalRead(&multiIo, 100));
    assertTrue(ioDeviceDigitalRead(&multiIo, 101));

    assertEqual(2, ioDevice1.getNumberOfRunLoops());
    assertEqual(2, ioDevice2.getNumberOfRunLoops());

    ioDevice1.resetIo();
    ioDevice2.resetIo();

    ioDeviceDigitalWritePort(&multiIo, 109, 0xaa);
    ioDeviceDigitalWritePortS(&multiIo, 129, 0x55);
    assertEqual((uint16_t)0xaa00, ioDevice1.getWrittenValue(0));
    assertEqual((uint16_t)0x5500, ioDevice2.getWrittenValue(0));

    ioDeviceDigitalWritePort(&multiIo, 109, 0x99);
    ioDeviceDigitalWritePortS(&multiIo, 129, 0x88);
    assertEqual((uint16_t)0x9900, ioDevice1.getWrittenValue(1));
    assertEqual((uint16_t)0x8800, ioDevice2.getWrittenValue(1));

    assertEqual(2, ioDevice1.getNumberOfRunLoops());
    assertEqual(2, ioDevice2.getNumberOfRunLoops());

    assertEqual(ioDevice1.getErrorMode(), NO_ERROR);
    assertEqual(ioDevice2.getErrorMode(), NO_ERROR);
}