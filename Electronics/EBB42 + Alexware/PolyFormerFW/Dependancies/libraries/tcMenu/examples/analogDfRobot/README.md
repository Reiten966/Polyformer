## Simple arduino menu on DfRobot shield

This example assumes you've got an Uno / MEGA with the DF robot board.

It uses a the switches that are connected to analog 0 and tries to keep as much in line with the DF robot spec as possible.

It has two menu items, one to control the PWM output of pin 9, one to read back the value via A1. Other than connecting the DfRobot, the only other requirement is the small PWM circuit connected between the PWM pin and A1.

I've assumed this will be built on either an Uno or MEGA boards, but should work equally well on any other Arduino board with only minor changes.

To get started you can either run as per instructions, or just load the `analogDfRobot.emf` file into the designer, select Code / Generate and change to match your setup.

