# STM32CubeProg
---

# Windows | Mac | Linux

The easiest way by far to program the board is to download STM32CubeProg. It's a graphical programmer, and does not require Zadig or special command line invocation. Download the earliest version available from their website after 2.1.0 (e.g. 2.5.0 is fine), otherwise, the writing process will not complete successfully.

You'll need to make a ST.com account is the only downside.

[Download STM32CubeProg(please download 2.50 version)](https://www.st.com/en/development-tools/stm32cubeprog.html)

![Alt text](images/STM32CubeProg1.png)
When you start it up, it'll look like this.

![Alt text](images/STM32CubeProg2.png)
In the top right, below the Not Connected message, find the dropdown to the left of the Connect button
Select "USB"

![Alt text](images/STM32CubeProg3.png)
If the device is plugged in and the bootloader is running, it will show up under the USB configuration pane. If not, enter bootloader mode by holding BOOT and pressing RST (you shoudl see a blue LED on the board). Click the refresh button.
Once it appears as a valid Port, click Connect

![Alt text](images/STM32CubeProg4.png)
You should see the Device info pane in the bottom right is updated with info about what chip was found!

![Alt text](images/STM32CubeProg5.png)
Click the 3-lines below the STM32 logo in the top left, to expand the menu.
Then click Erasing & Programming

![Alt text](images/STM32CubeProg6.png)
Click Browse to open the firmware files you want to program.
You can program .hex or .bin files.
Don't change the Start Address.
Make sure Verify Programming and Run after Programming are clicked, but Skip flash erase is not.
Then click Start Programming.

![Alt text](images/STM32CubeProg7.png)![Alt text](images/STM32CubeProg9.png)![Alt text](images/STM32CubeProg10.png)
It will take a few seconds to erase and reprogram the chip.
It's normal to get a Warning Connection is lost alert.
Click away until you get the File download complete alert.

That's it! You can close STM32 CubeProg.
