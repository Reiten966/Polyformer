# EBB42: Firmware Deployment Guide
---

The firmware files are located [here](https://github.com/Reiten966/Polyformer/tree/main/Electronics/EBB42%20%2B%20Alexware%20(PF%20Kit)/Firmware), the EBB42 has no firmware installed by default when shipped out. You can manage the firmware using dfu-util, Arduino IDE and STM32CubeProgrammer.<!-- , Particle CLI, Particle Cloud, Ymodem and OpenOCD. -->


* [Using STM32Programmer](https://youtu.be/_FELCN8CbWA?t=385): STM32Programmer may be easier if you prefer a graphical interface. This video(watch 6:24-8:06) by Eddie shows you how to install any firmware on EBB42, you just need to load the PolyFormerFW_Vx.x.bin file instead.

* [Using dfu-util](#dfu-util_installation_guide.md): You can update all of the internal flash memory, system firmware and any other binary files if you need, except the bootloader itself.

* [Using Arduino IDE](#using-arduino-ide): You can update system firmware and  user application. If you have a ST-LINK in hand, you can even update the bootloader.





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
