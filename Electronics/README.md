# Firmware Setup

### Its a good idea to flash the firmware for Pi and MCU **BEFORE** you follow the part 4 video to install them on the machine.



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
