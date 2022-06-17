# Wiring Guide

![Polyformer-Electronics-RPi-SKR-Pico1 0](https://user-images.githubusercontent.com/55605342/173913205-4c073081-22fa-4f03-8d1d-62b9d70ea6fb.png)

# Firmware Setup


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
PID_CALIBRATE HEATER=extruder TARGET=210 WRITE_FILE=1
```
4. Run this command to save the PID colibration coeffs to the 'printer.cfg' file
```
SAVE_CONFIG
```
5. Edit `printer.cfg`, set the PID coeffs (`pid_Kp`, `pid_Ki`, `pid_Kd`) to the values that were just calculated. The computed PID coeffs are appended to the `printer.cfg` in a block comment
4. Click the _Save and Restart_ button to reconfigure _Klipper_
6. Verify temperature stabiity
