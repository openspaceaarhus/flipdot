# Getting started #

Get prerequisites: CMake, Make etc

and arduino-cli
on ubuntu this can be installed with:

sudo snap install arduino-cli

## Configure the build ##
The SSID / Password  is set as variables in CMakeLists.txt

The WEMOS_PORT is where the wemos is connected

You can change them by editing your local copy, giving them as parameters for cmake.


## Setup a build ##
We need to create the Makefiles and insert values into the `Config.h` header
```
cd ~/flipdot/flimos
cmake -B build -DSSID=osaa-iot -DPASSWORD=hunter2 -DWEMOS_PORT="/dev/usb0"
```

## Prepare the wemos environment ##
Install all the wemos libraries we need

```
make -C build  flipmos_init
```

## Build the flipmos binary ##
This steps compiles
```
make -C build  flipmos
```
## deploy the flipmos binary ##
This will upload to `WEMOS_PORT`

```
make -C build flipmos_upload
```


