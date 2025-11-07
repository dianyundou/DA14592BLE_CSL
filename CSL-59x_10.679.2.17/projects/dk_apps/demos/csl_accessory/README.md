Renesas Crowd-Sourced Locationing (CSL) accessory demo application {#csla}
=============================================================

## Overview

This is a demo application for Renesas CSL accessory combining both Apple Find My Network (FMN) and Google Find My Device Network (FMDN) extension of Google Fast Pair. Implementation is based on Apple specification R2, Google Fast Pair specification v3.1 and Find My Device Network Accessory specification v1.3, respectively.

## Command Line Interface (CLI)

You can use Command Line Interface. Available commands:
* help - list all commands
* reset [factory] - to do simple device reset or factory reset (erase all stored keys)
* pairmode - to put device into pairing mode
* stopring - to stop ringing
* userconsent - to enable user consent mode (i.e. BLE serial number lookup for Apple FMN)
* advertise stop | start - to stop or start advertising
* motion - to simulate motion detection event
* serial - to print accessory serial number

## Finder Network Pairing

* Reboot accessory.
* Apple Find My Network pairing mode is enabled for 10 minutes.
* Long-press K1 for 4s (until red LED starts to blink) to put device into Google Fast Pair pairing mode.
* @iphone run "Find My" app
  * tap "Add Item"
  * select "Other Supported Item"
  * DA1459x accessory should be found, tap "Connect" and continue with the pairing procedure
* @android phone wait for notification pop-up, then tap "Connect"
  * Use "Find My Device" app to localize the device or start ringing

