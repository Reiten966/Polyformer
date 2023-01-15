# Firmware for the PolyFormer stand alone pulling system
---

## Wiring Diagram
![Wiring_Diagram_EBB42](https://user-images.githubusercontent.com/55605342/206941306-63c65b8e-e5c7-4ce9-bf41-2111f33de0be.png)

* [Wiring Guide In-Progress](https://discord.com/channels/969539629176991764/1062541608559579187)



## References

* [Firmware deployment guide](firmware_deployment_guide.md)


## License

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

<!-- # PolyFormerFW
Firmware for the PolyFormer stand alone pulling system

Compilation instructions

1.  Install Arduino IDE
    You get the latest Arduino IDE here:
    http://arduino.cc/en/Main/Software

3.  Install STM32CubeIDE
	Download and install STM32CubeProgrammer from the 'STM32CubePrg' folder here https://drive.google.com/drive/folders/1L-q9MlswSXPIMe1Jg1lsA1I1AFvQ9x0S?usp=sharing.

	(Alternatively you can download by following URL: https://www.st.com/en/development-tools/stm32cubeprog.html
	Warning: it is necessary to create a my.st.com account, if using this link.)

2.  Install STM32Duino
	Once Arduino IDE is installed, launch Arduino IDE then go to File > Preferences
	In “Additional Boards Manager URLs”, enter the following URL : https://raw.githubusercontent.com/stm32duino/BoardManagerFiles/master/STM32/package_stm_index.json
	Enter “OK”
	Then : Tools > Board : ___ > Boards Manager...
	Enter in the search bar “STM32” or “stm” and download the package by clicking on install.

4.  Install the required libraries
	Go to Sketch > Library and choose "Manage Libraries..."
	Search for, and install the following (and any dependancies if prompted):
	"NTC_Thermistor"
	"PID"
	"Bounce2"
	"U8g2"
	"TCMenu"
	"GyverNTC"
	"TMCStepper"

5.  Start Arduino IDE and open the file "PolyFormerFW.ino"

5.  Select the board and port to upload via in the arduino ide.
	Launch Arduino IDE with a STM32 plugged in USB on the computer.
	Go to Tools > Board: ___ > STM32 boards groups... > and choose "Generic STM32G0 series"
	Then go to Tools > Board part number: ___ and choose "Generic G0B1RBTx"
	Go to Tools > USB support:___ and choose "CDC (generic 'Serial' supersede U(S)ART)"
	In Tools > Upload method: ___ choose STM32CubeProgrammer (DFU)
	Then : Tools > Port: ___ and choose the corresponding port to your STM32 board.

6.  Check PolyFormerFW.ino for hints, if something needs to be checked or modified.

7.  Upload the firmware with the upload button (right arrow in toolbar). -->
