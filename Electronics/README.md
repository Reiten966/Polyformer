# Wiring Guide

![Polyformer-Electronics-RPi-SKR-Pico1 0](https://user-images.githubusercontent.com/55605342/173913205-4c073081-22fa-4f03-8d1d-62b9d70ea6fb.png)

# Firmware Setup

## Option1: (Klipper: Standalone Pi + Compatible MCU)


- Insert SD card into your PC, download and install Pios on the SD card. [Download Pios here](https://www.raspberrypi.com/software/)
- Before you install the OS, please configure your pi's username, password, and network(SSIDs are upper case sensitive!!!) in the setting.
- It takes sometime if the Pi is first time boot up upon an install. Please be patient for 5 minutes.


![image](https://user-images.githubusercontent.com/55605342/174422910-c1eb13dd-0ef4-4d33-b69c-427f8a600450.png)
    


- Please first follow the guide to [install klipper on raspberry pi](https://www.lpomykal.cz/kiauh-installation-guide/) starting from **chapter 2.2**
  
  Copy and right click on the terminal to paste codes.


- And then please follow the guide to [install Klipper on SKR pico](https://github.com/bigtreetech/SKR-Pico/tree/master/Klipper)

- And then add the printer.cfg to the mainsail web interface.





## Option2: (Klipper: Voron printer + Compatible MCU)



- Plug the SKR pico to the pi via USB on your printer, and add the polyformer.cfg into your printer's config as below
![image](https://user-images.githubusercontent.com/55605342/166185969-eca3ac38-87a4-4fdb-8bb0-ddc4806e66f2.png)
- And then include the polyformer.cfg in your main printer.cfg as below
![image](https://user-images.githubusercontent.com/55605342/166186076-9d54991d-e156-47dc-b81a-397d029f8e0a.png)

- [SKR pico installation guide here](https://github.com/bigtreetech/SKR-Pico/tree/master/Klipper)

## Option3: (Marlin: Compatible MCU + Screen)


Guide in progress, please ask in the discord.


# PID Calibration Procedure

This procedure is for calibrating the heater cartridge.

__Reference documentation__

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

__IMPORTANT:__ ALL pre-qeqs MUST be satified in order to proceed.

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
