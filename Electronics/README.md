# Firmware Setup

### Its a good idea to flash the firmware for Pi and MCU **BEFORE** you follow the part 4 video and install them on the machine.

## Option1: (Klipper: Standalone Pi + Compatible MCU)

- Insert SD card into your PC, download and install Pios on the SD card. [Download Pios here](https://www.raspberrypi.com/software/)
- Before you install the OS, please configure your pi's username, password, and network(SSIDs are upper case sensitive!!!) in the setting.
- It takes sometime if the Pi is first time booting up upon an install. Please be patient for 5 minutes and 5 minutes only.

![image](https://user-images.githubusercontent.com/55605342/174422910-c1eb13dd-0ef4-4d33-b69c-427f8a600450.png)

## Klipper installation on Raspberry

First, you’ll need a SSH client such as [PuTTY](https://www.putty.org/) or use your PC’s terminal. We’ll use it to
connect our PC to the Raspberry.

- Start with locating Raspberry in your local network.

  - You can use [Advanced IP Scanner](https://www.advanced-ip-scanner.com/) to find it.

  ![image](https://user-images.githubusercontent.com/70567811/177376907-3577eb64-083f-4808-a2c1-a83d572cbb41.png)

  - Or if you can plug a keyboard, a mouse, and a screen to your Raspberry:

    - Once plugged, turn the Raspberry on and wait until the main screen appears
    - Click on the terminal icon on the task bar

    ![image](https://user-images.githubusercontent.com/70567811/177376910-1aa73b31-3b85-47ce-a4ec-49d466582111.png)

    - Type `ifconfig` and find the IP address between 192.168.0.1-254 or 192.168.1.1-254

    ![image](https://user-images.githubusercontent.com/70567811/177376911-fbb90e5b-1170-4c53-82ca-c45b79e705e5.png)

- SSH connection

  - If you chose PuTTY, paste the copied IP from the previous step. Click on Open.

  ![image](https://user-images.githubusercontent.com/70567811/177376914-165df54b-5ade-4ea4-be4c-90715925ee19.png)

  - This window may appear, click on Yes

  ![image](https://user-images.githubusercontent.com/70567811/177376915-be8dd046-dc96-44cd-a432-43971041b246.png)

  - Then login in the terminal with your username and password.

  * Else, if you chose your PC’s terminal, you can type `ssh pi@192.168.0.210` to connect to the Raspberry ((replace pi by the username you set on Raspberry Setup and the IP by the one you found on the previous step) and type your password.

- GIT installation

  - Type `sudo apt-get install git -y` in the terminal (you might be asked to enter your password)
  - Once the installation is complete, do the followind commands:

    - `cd ~`
    - `git clone https://github.com/th33xitus/kiauh.git`
    - `./kiauh/kiauh.sh`
    - This window will show up

    ![image](https://user-images.githubusercontent.com/70567811/177376916-d7a5dd19-9ed7-4477-96a6-fe764b00c57a.png)

    - Use the interface to install Klipper, Moonraker and Mainsail.

## Klipper installation on SKR Pico

If you can't get Klipper working with the UART in Option 2, try the USN which should be easier.

- Option 1 : USB setup

![image](https://user-images.githubusercontent.com/70567811/177376918-b372b667-78d7-4ad8-bbe6-86553759aca7.png)

- Option 2 : UART setup

![image](https://user-images.githubusercontent.com/70567811/177376923-ffb21f26-5516-459a-9268-902e88696b82.png)

- Build firmware image

   1. SSH into your pi and type
      ```
      cd ~/klipper/
      make menuconfig
      ```

   2. Building the micro controller with the configuration shown below.
      * [*] Enable extra low-level configuration options
      * Micro-controller Architecture = `Raspberry Pi RP2040`
      * IF USE USB
         * Communication interface = `USB`
             ![klipper_menuconfig](https://user-images.githubusercontent.com/55605342/178121718-b5b6baf3-de33-4c4a-b05d-788bdb36a745.png)
      * ElSE IF USE UART0
         * Communication interface = `Serial (on UART0 GPIO1/GPIO0)`
             ![image](https://user-images.githubusercontent.com/55605342/178122140-dbb68adf-d975-424e-9f00-9be30a87eba8.png)


   3. Once the configuration is selected, press `q` to exit,  and "Yes" when  asked to save the configuration.
   4. Run the command `make`
   5. The `klipper.uf2` file will be generated in the folder `home/pi/kliiper/out` when the `make` command completed. Use your windows PC to copy the file from the Pi's folder using [winscp](https://winscp.net/download/WinSCP-5.21.1-Setup.exe) to your PC.

## Firmware Installation

1. Insert a jumper on the `Boot` pins of the motherboard and click the `Reset` button to enter the burn mode (Note: If you want to use USB to power the motherboard, you need to insert a jumper on `USB Power`. When there is 12V / 24V power supply, it is best to remove the jumper)
   ![boot](https://user-images.githubusercontent.com/55605342/178121734-eed02614-2aa9-4460-aae4-94c4eef7b908.png)
2. Connect USB-C to computer, then you will see a USB flash drive named `RPI-PR2`, copy `klipper.uf2` compiled by yourself to the USB flash drive, the motherboard will automatically reboot and update the firmware, the computer will re-identify this USB flash drive means the firmware update
is complete, unplug the `boot jumper` and click the `Reset` button to enter normal working mode

   ![msc](https://user-images.githubusercontent.com/55605342/178121736-f8d809a8-eca1-463c-b515-508fa1a1187f.png) 
3. **(note: this test is only needed for USB configuration)** you can confirm that the flash was successful, by running `ls /dev/serial/by-id`.  if the flash was successful, this should now show a klipper device, similar to:

   ![rp2040_id](https://user-images.githubusercontent.com/55605342/178121739-78c09143-54f3-41ff-b319-ec3552a6ac16.png)



## Configure the printer parameters
### Basic configuration
1. On your PC, open a web browser and go to the IP address we gained for SSH within the same network to access Mainsail.
2. Replace the printer.cfg file from the mainsail interface with [printer.cfg](https://github.com/Reiten966/Polyformer/blob/main/Electronics/Firmware/printer.cfg).
3. If you use USB to communicate with raspberry pi, ssh into the pi and run the `ls /dev/serial/by-id/*` command in raspberry pi to get the correct ID number of the motherboard, and set the correct ID number in `printer.cfg`. And wiring reference [here](#raspberry-pi-is-powered-by-an-external-5v-adapter-and-communicates-with-skr-pico-v10-via-usb)
    ```
    [mcu]
    serial: /dev/serial/by-id/usb-Klipper_rp2040_E66094A027831922-if00
    ```
4. If you use UART0 to communicate with raspberry pi, you need to modify the following files by inserting the SD card into the computer or by SSH command. And wiring reference [here](#raspberry-pi-is-powered-by-the-motherboard-5v-and-communicates-with-skr-pico-v10-via-uart)
   * Remove `console=serial0,115200` in `/boot/cmdline.txt`
   * Add `dtoverlay=pi3-miniuart-bt` at the end of file `/boot/config.txt`
   * Modify the configuration of `[mcu]` in `printer.cfg` to `serial: /dev/ttyAMA0` and enable `restart_method: command` by SSH
     ```
     [mcu]
     serial: /dev/ttyAMA0
     restart_method: command
     ```

## Option2: (Klipper: Voron printer + Compatible MCU)

- Plug the SKR pico to the pi via USB on your printer, and add the polyformer.cfg into your printer's config as below
  ![image](https://user-images.githubusercontent.com/55605342/166185969-eca3ac38-87a4-4fdb-8bb0-ddc4806e66f2.png)
- And then include the polyformer.cfg in your main printer.cfg as below
  ![image](https://user-images.githubusercontent.com/55605342/166186076-9d54991d-e156-47dc-b81a-397d029f8e0a.png)

- Follow the Klipper installation on SKR Pico above.

## Option3: (Marlin: Compatible MCU + Screen)

Guide in progress, please ask in the discord.

# Wiring Guide

- UART setup
  ![Polyformer-Electronics-RPi-SKR-Pico1 0](https://user-images.githubusercontent.com/55605342/173913205-4c073081-22fa-4f03-8d1d-62b9d70ea6fb.png)

- USB setup
  ![image](https://user-images.githubusercontent.com/70567811/177376903-14f4bb92-6adb-45ea-b97c-070cf620d8c9.png)

# PID Calibration Procedure

This procedure is for calibrating the heater cartridge.

**Reference documentation**

- [Bigtree SKR Pico v1.0 + RPi setup](https://3dwork.io/en/skr-pico-analysis-and-complete-guide-of-this-great-little-controller)
- [Basic Klipper Configuration](https://3dwork.io/en/skr-pico-analysis-and-complete-guide-of-this-great-little-controller/#basic-klipper-configuration)
- [Mainsail](https://docs.mainsail.xyz/)
- [Installing MainsailOS](https://docs.mainsail.xyz/setup/mainsail-os)
- [Klipper3D](https://www.klipper3d.org)

## Pre-reqs

The following instructions assume the following:

- Your hardware setup matches the diagram the provided diagram `Polyformer-Electronics-RPi-SKR-Pico1.0-Working.png`
- Your RaspberryPi has been configured for UART integration. If not, follow this guide [Basic Klipper Configuration](https://3dwork.io/en/skr-pico-analysis-and-complete-guide-of-this-great-little-controller/#basic-klipper-configuration)
- _MainsailOS_ (_Mainsail_, _klipper_, and _Moonracker_) are install and running on the _RaspberryPi_
- Your _SKR Pico v1.0_ control board has been flashed with the latest stable firmware and has `UART` support. The name of your firware image should be `klipper-UART0.uf2`
- You are using the attached klipper config `printer.cfg`
- _Mainsail_ web UI is acceesible and there are no errors reported by klipper on the _Mainsail_ dashboard.

**IMPORTANT:** ALL pre-qeqs MUST be satified in order to proceed.

## Instructions

1. Open a web brower and go to the Mainsail website that is running on your RapsberryPi
2. Open the _G-Code console_ on the Mainsail web UI
3. Run the following g-code command to calibrate. This will take about 5 minutes to complete:

```
PID_CALIBRATE HEATER=extruder TARGET=210
```

4. Run this command to save the PID calibration coeffs to the 'printer.cfg' file

```
SAVE_CONFIG
```

5. Click the _Save and Restart_ button to reconfigure _Klipper_
6. Verify temperature stabiity
