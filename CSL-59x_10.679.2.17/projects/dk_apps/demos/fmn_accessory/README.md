Apple Find My Network (FMN) accessory demo application {#afmn}
=============================================================

## Overview

This is a demo application for Apple Find My Network (FMN) accessory. Implementation is based on Apple specification R2.

## Command Line Interface (CLI)

You can use Command Line Interface. Available commands:
* help - list all commands
* reset [factory] - to do simple device reset or factory reset (erase all stored keys)
* pairmode - to put device into pairing mode (10 minutes)
* userconsent - to enable BLE serial number lookup for 5 minutes
* motion - to simulate motion detection event
* serial - to print accessory serial number

## FindMy Pairing

* Reboot accessory, pair mode for 10 minutes. 
* @iphone run "Find My" app
* tap "Add Item"
* select "Other Supported Item"
* DA1459x accessory should be found, tap "Connect" and continue with the pairing procedure
