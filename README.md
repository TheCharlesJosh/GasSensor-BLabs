Bayanihan Labs - Gas Sensor (WIP)
===============

## Introduction

Another addition to the Internet of Things, this time utilizing an MQ-6 Gas Sensor, a Neocene 2T3542142 Bipolar Stepper Motor, a RN-XV WiFly Module, and an Arduino Uno. This detects the LPG (or propane, butane) concentration (parts per million) within the vicinity, pushes data online real-time, and switches the gas valve automatically, online, or through an app.

## Milestones reached

1. Calibrate the sensor to properly detect the LPG/propane/butane concentration.
2. Pass values to m2m.io through WiFly module.
3. Finish the circuitry needed to weave all the modules together to the Uno.

## Milestones to-do

1. Build a 3D model to connect the stepper motor to the gas valve.
2. Create a casing for the whole device to appear aesthetically pleasing.
3. Calibrate the stepper rotation to properly switch on/off the gas valve.
4. Create a web & mobile application to properly interpret the data, as well as to remotely manipulate the device.