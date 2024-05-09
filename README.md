# QHS Util 

Simple utility to read QHS support status from Qualcomm BT modules via HCI commands

## Usage
```console
$ g++ -O3 qhs-util.cpp -o qhs-util -lbluetooth
$ sudo ./qhs-util
```

Also runs on Android (as root) if built via `m qhs-util` inside AOSP tree. 
Bluetooth needs to be disabled first.

