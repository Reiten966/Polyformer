# EBB42: Firmware Deployment Guide
---

The EBB42 has no firmware installed by default when shipped out. You can manage the firmware using dfu-util, Arduino IDE and STM32CubeProgrammer.<!-- , Particle CLI, Particle Cloud, Ymodem and OpenOCD. -->

**More instructions to update the EBB42 using dfu-util, please refer to the [Firmware Deployment Guide](#using-dfu-util).**

* [Using dfu-util](#using-dfu-util): You can update all of the internal flash memory, system firmware and any other bianry files if you need, except the bootloader itself.

* [Using Arduino IDE](#using-arduino-ide): You can update system firmware and  user application. If you have a ST-LINK in hand, you can even update the bootloader.


## <span id="using-dfu-util">Using dfu-util</span>

Before using the dfu-util, you have to install it first. Please follow the [dfu-util Installation Guide](dfu-util_installation_guide.md) to install it on your computer.

For Windows users, if you havn't installed the DFU USB driver for EBB42, please follow the [Windows Driver Installation Guide](windows_driver_installation_guide.md) to install the driver first.

**All of the firmware images are available on this [GitHub Repository](https://github.com/Reiten966/Polyformer/tree/main/Electronics/EBB42%20%2B%20Alexware).** Just download what you need.

dfu-util command line options:

* `-d` : specify the DFU device's USB ID. For EBB42 it is "df11:0483"
* `-a` : specify the interface of the memory. For EBB42 it has three interfaces.
    - `0` : access the internal memory space, except the bootloader memory space
    - `1` : access the application DCT (not the entire DCT) memory space
    - `2` : access the external SPI flash memory space
* `-s` : specify the offset address of the memory space to store the binary file from
* `-D` : the binary (.bin) file specified is going to be downloaded to the DFU device
* `-U` : the binary (.bin) file specified is going to store the raw data that is dunmped from DFU device
* `:leave` : append this option to the offset address will make device exit DFU Mode, e.g. `-s 0x80C0000:leave`.

Now make your EBB42 enter DFU Mode:

1. Hold down BOTH buttons
2. Release only the RST button, while holding down the BOOT button.
3. Wait for 1 second
4. Release the BOOT button

#### Update System Firmware

* Update system:    
`dfu-util -d df11:0483 -a 0 -s 0x8000000 -D 
PolyFormerFW_Vx.x.bin`

#### Sample Output
    dfu-util 0.11

    Copyright 2005-2009 Weston Schmidt, Harald Welte and OpenMoko Inc.
    Copyright 2010-2021 Tormod Volden and Stefan Schmidt
    This program is Free Software and has ABSOLUTELY NO WARRANTY
    Please report bugs to http://sourceforge.net/p/dfu-util/tickets/

    dfu-util: Warning: Invalid DFU suffix signature
    dfu-util: A valid DFU suffix will be required in a future dfu-util release
    Opening DFU capable USB device...
    Device ID 0483:df11
    Device DFU version 011a
    Claiming USB DFU Interface...
    Setting Alternate Interface #0 ...
    Determining device status...
    DFU state(2) = dfuIDLE, status(0) = No error condition is present
    DFU mode device DFU version 011a
    Device returned transfer size 1024
    DfuSe interface name: "Internal Flash   "
    Downloading element to address = 0x08000000, size = 111724
    Erase           [=========================] 100%       111724 bytes
    Erase    done.
    Download        [=========================] 100%       111724 bytes
    Download done.
    File downloaded successfully


<!-- ## <span id="using-arduino-ide">Using Arduino IDE</span>

If this is your first time playing with the EBB42 using Arduino IDE, you are recommended to follow the [Getting Started with Arduino IDE](getting_started_with_arduino_ide.md) to set up the Arduino environment first. -->

<!-- #### Update User Application (aka. Arduino sketch)

To upload your sketch, simply click on the ![image](images/Upload_icon.png) icon.

#### Update System Firmware
##### 1. via Native USB Port

If you connect your EBB42 directly to the computer, you can update the EBB42's system firmware, by using the "**EBB42 FW Uploader**" programmer.  The factory reset application will also be updated.

- Connect your EBB42 to computer and put it in DFU mode:

    - Hold down BOTH buttons
    - Release only the RST button, while holding down the BOOT button.
    - Wait for 1 second
    - Release the SETUP button

- Select the board: "Tools > Board: EBB42 (Native USB Port)"

- Select the programmer:  "Tools > Programmer: EBB42 FW Uploader"

- Click on "Tools > Burn Bootloader" to update the system firmware.

- After the burn bootloader operation completed, the on-board blue LED start blinking rapidly, since it has also downloaded a blink application, in case that your old application is not compatible with the updated system firmware.

##### 2. via ST-LINK USB Port

If you mount your EBB42 onto ST-LINK and connect the ST-LINK to your computer, you can update the EBB42's bootloader and its system firmware by using the "**ST-LINK**" programmer.

- Mount your EBB42 (be aware of the orientation) onto ST-LINK and connect the ST-LINK to your computer

- Select the board: "Tools > Board: RedBear EBB42 (ST-LINK USB Port)"

- Select the programmer:  "Tools > Programmer: ST-LINK"

- Click on "Tools > Burn Bootloader" to update the bootloader and system firmware.

- After the burn bootloader operation completed, the on-board blue LED start blinking rapidly, since it has also downloaded a blink application, in case that your old application is not compatible with the updated system firmware. -->


## References

* [EBB42 Introduction](EBB42_introduction.md)
* [Firmware Architecture Overview](firmware_architecture_overview.md)
* [System Firmware Change-log](system_firmware_changelog.md)
* [dfu-util Installation Guide](dfu-util_installation_guide.md)
* [Windows Driver Installation Guide](windows_driver_installation_guide.md)
* [Polyformer Disord](https://discord.gg/JUNUWZkG)


## Resources

* [Firmware Binaries](https://github.com/Reiten966/Polyformer/tree/main/Electronics/EBB42%20%2B%20Alexware)


## License

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
