
# dfu-util Installation Guide
[dfu-util](http://dfu-util.sourceforge.net/) is a command line tool for Device Firmware Upgrade via USB port. When the EBB42 is in DFU mode, we can use dfu-util to download firmware to the EBB42, as well as upload firmware from the EBB42 to local binary file. 

dfu-util uses libusb 1.0 to access your device, so on Windows you have to register the device with the WinUSB driver (alternatively libusb-win32 or libusbK), please see the [libusb wiki](https://github.com/libusb/libusb/wiki/Windows#How_to_use_libusb_on_Windows) for more details.

**Note: Make sure you are installing the dfu-util 0.8 or later**

## Installation

### Windows

* Download the [dfu-util](http://dfu-util.sourceforge.net/releases/dfu-util-0.8-binaries/win32-mingw32/dfu-util-static.exe) to your local system, e.g., under `C:\dfu-util`.

* Rename it to `dfu-util.exe`

* Append the path of the `dfu-util.exe` to the system environment variable `Path`: "My Computer" > "Properties" > "Advanced" > "Environment Variables" > "Path". Please note that paths in the variable `Path` are seperated by semicolon `;`. This will allow dfu-util to be executed globally in command prompt.

### OSX

Use "brew" to install or follow the instructions from [the official website](http://dfu-util.sourceforge.net/).

* Install [brew](http://brew.sh/) or start the Terminal to install it directly:

        $ ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

* In the Teminal, type this command to install the dfu-util:

        $ brew install dfu-util

* Add the path of `dfu-util` to `PATH`:

        export PATH=$PATH:{PATH_TO_DFU_UTIL}

    This will allow dfu-util to be executed globally in command prompt.


### Linux

* Download the [dfu-util](http://dfu-util.sourceforge.net/releases/dfu-util-0.8-binaries/linux-i386/) or use the package manager of your distribution to get the latest version:

        $ sudo apt update && sudo apt install dfu-util

* Add UDEV rule:

	    $ sudo nano /etc/udev/rules.d/50-usb-conf.rules

    Simply add these lines:

	    SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="df11", GROUP="users", MODE="0666"

    `Ctrl + s`, Followed by `Ctrl + x` to save the file

    Reload the udev rules:

        $ sudo udevadm control --reload-rules && sudo udevadm trigger

If it prompts "Cannot open DFU device", just reboot to try again.


## Build from Source ([origin](http://dfu-util.sourceforge.net/build.html))

### Prerequisites for building from git

* Mac OS X

    First install MacPorts (and if you are on 10.6 or older, the Java Developer Package) and then run:

        sudo port install libusb-devel git-core

* FreeBSD

        sudo pkg_add -r git pkgconf

* Ubuntu and Debian and derivatives

        sudo apt-get build-dep dfu-util
	    sudo apt-get install libusb-1.0-0-dev

### Get the source code and build it

*The first time you will have to clone the git repository:

    git clone git://git.code.sf.net/p/dfu-util/dfu-util
	cd dfu-util

If you later want to update to latest git version, just run this:

    make maintainer-clean
	git pull

To build the source:

	./autogen.sh
	./configure  # on most systems
	make

If you are building on Mac OS X, replace the ./configure command with:

	./configure --libdir=/opt/local/lib --includedir=/opt/local/include  # on MacOSX only

Your dfu-util binary will be inside the src folder. Use it from there, or install it to /usr/local/bin by running "sudo make install".

### Cross-building for Windows

Windows binaries can be built in a [MinGW](http://mingw.org/) environment, on a Windows computer or cross-hosted in another OS. To build it on a Debian or Ubuntu host, first install build dependencies:

	sudo apt-get build-dep libusb-1.0-0 dfu-util
	sudo apt-get install mingw32

The below example builds dfu-util 0.8 and libusb 1.0.20 from unpacked release tarballs. If you instead build from git, you will have to run "./autogen.sh" before running the "./configure" steps.

    mkdir -p build
    cd libusb-1.0.20
    PKG_CONFIG_PATH=$PWD/../build/lib/pkgconfig ./configure --host=i586-mingw32msvc --prefix=$PWD/../build

    make
    make install
    cd ..

    cd dfu-util-0.8
    PKG_CONFIG_PATH=$PWD/../build/lib/pkgconfig ./configure --host=i586-mingw32msvc --prefix=$PWD/../build

    make
    make install
    cd ..

The build files will now be in build/bin.

### Building on Windows using MinGW-w64 from MSYS2

This assumes using release tarballs or having run ./autogen.sh on the git sources.

First install MSYS2 from the [MSYS2 installer](http://msys2.github.io/) home page.

To avoid interference from other software on your computer, set a clean path in the MSYS window before running the upgrade commands:

    PATH=/usr/local/bin:/usr/bin:/bin:/opt/bin

    update-core
    pacman -Su
    pacman -Sy

Close all MSYS windows and open a new one to install the toolchain:

    PATH=/usr/local/bin:/usr/bin:/bin:/opt/bin

    pacman -S mingw-w64-x86_64-gcc
    pacman -S make

Now open a MINGW64 shell to build libusb and dfu-util:

    PATH=/mingw64/bin:/usr/local/bin:/usr/bin:/bin

    cd libusb-1.0.20
    ./configure --prefix=$PWD/../build
    make
    make install
    cd ..

    cd dfu-util-0.9
    ./configure USB_CFLAGS="-I$PWD/../build/include/libusb-1.0" USB_LIBS="-L $PWD/../build/lib -lusb-1.0" --prefix=$PWD/../build
    make
    make install
    cd ..

To link libusb statically into dfu-util.exe use instead of only "make":

make LDFLAGS=-static

The built executables (and DLL) will now be in the build/bin folder.


## Check dfu-util Version

* Type in command line terminal:
		
		$ dfu-util --version

* Sample output:

		dfu-util 0.8

		Copyright 2005-2009 Weston Schmidt, Harald Welte and OpenMoko Inc.
		Copyright 2010-2014 Tormod Volden and Stefan Schmidt
		This program is Free Software and has ABSOLUTELY NO WARRANTY
		Please report bugs to dfu-util@lists.gnumonks.org


## List EBB42 Devices

* Firstly make your EBB42 enter DFU Mode:  

    - Connect the jumper across VBUS (next to the USB port)
    - Connect the EBB42 to your computer via the USB port.
    - Hold down BOTH buttons (RST & BOOT)
    - Release only the RST button, while holding down the BOOT button.
    - Wait for one second
    - Release the BOOT button

* (**Windows only**) Follow this [Windows Driver Installation Guide](windows_driver_installation_guide.md) to install the DFU USB driver

* Type in command line terminal:

        $ dfu-util -l

* Sample output:

        Found DFU: [0483:df11] ver=0200, devnum=76, cfg=1, intf=0, path="1-1", alt=2, name="@Internal Flash   /0x08000000/64*02Kg", serial="206B327A5841"
        Found DFU: [0483:df11] ver=0200, devnum=76, cfg=1, intf=0, path="1-1", alt=1, name="@Internal Flash   /0x08000000/64*02Kg", serial="206B327A5841"
        Found DFU: [0483:df11] ver=0200, devnum=76, cfg=1, intf=0, path="1-1", alt=0, name="@Internal Flash   /0x08000000/64*02Kg", serial="206B327A5841"



## What's Next

[Firmware Deployment Guide](firmware_deployment_guide.md)


## References

* [Polyformer Introduction](polyformer_introduction.md)
* [Windows Driver Installation Guide](windows_driver_installation_guide.md)
* [dfu-util Home Page](http://dfu-util.sourceforge.net/)
* [Polyformer Disord](https://discord.gg/JUNUWZkG)


## Resources

* [dfu-util](http://dfu-util.sourceforge.net/releases/)


## License

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
