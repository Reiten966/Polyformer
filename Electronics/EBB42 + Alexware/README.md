# PolyFormerFW
Firmware for the PolyFormer stand alone pulling system

Compilation instructions

1.  Install Arduino IDE
    You get the latest Arduino IDE here:
    http://arduino.cc/en/Main/Software

2.  Install STM32DUINO
	Once Arduino IDE is installed, launch Arduino IDE then go to File > Preferences
	In “Additional Boards Manager URLs”, enter the following URL : https://raw.githubusercontent.com/stm32duino/BoardManagerFiles/master/STM32/package_stm_index.json
	Enter “OK”
	Then : Tools > Board : ___ > Boards Manager...
	Enter in the search bar “STM32” or “stm” and download the package by clicking on install.

3.  Install STM32CubeIDE
	Download and install STM32CubeIDE by following URL: https://www.st.com/en/development-tools/stm32cubeide.html
	Warning : it is necessary to create a my.st.com account.
	Launch Arduino IDE with a STM32 plugged in USB on the computer.
	Go to Tools > Board: ___ > STM32 boards groups... > and choose "Generic STM32G0 series"
	Then go to Tools > Board part number: ___ and choose "Generic G0B1RBTx"
	Go to Tools > USB support:___ and choose "CDC (generic 'Serial' supersede U(S)ART)"
	In Tools > Upload method: ___ choose STM32CubeProgrammer (DFU)
	Then : Tools > Port: ___ and choose the corresponding port to your STM32 board.

4.  Install the required libraries
	Go to Sketch > Library and choose "Manage Libraries..."
	Search for, and install the following:
	"NTC_Thermistor"
	"PID"
	"Bounce2"
	"U8g2"

5.  Start Arduino IDE and open the file "PolyFormerFW.ino"

5.  Select the board and port for upload in the arduino ide.

6.  Check PolyFormerFW.ino for hints, if something needs to be checked or modified.

7.  Upload the firmware with the upload button (right arrow in toolbar).