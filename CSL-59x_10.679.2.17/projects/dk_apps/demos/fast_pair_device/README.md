Google Fast Pair device with Find My Device Network (FMDN) extension demo application {#gfps-fmdn}
=============================================================

## Overview

This is a demo application for Google Fast Pair device with Find My Device Network (FMDN) extension. It covers all features for BLE-only devices from Google Fast Pair specification v3.1 and Find My Device Network Accessory specification v1.3.

## Command Line Interface (CLI)

You can use Command Line Interface. Available commands:
* help - list all commands
* reset [factory] - to do simple device reset or factory reset (erase all stored account keys)
* pairmode - to put device into pairing mode
* stopring - to stop ringing
* userconsent - to enable user consent mode
* advertise stop | start - to stop or start advertising
* motion - to simulate motion detection event

## Android setup

There are 2 Android applications that may be useful for testing/development:
* "Find My Device" 
* "Fast Pair Validator" 

## Fast Pair pairing

* Long-press K1 for 4s (until red LED starts to blink) to put device into pairing mode. 
* Wait for the android notification pop-up, then tap "connect".
* Use "Find My Device" to localize the device or start ringing.

