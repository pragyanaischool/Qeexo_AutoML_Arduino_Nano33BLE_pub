#!/bin/bash

# install arduino-cli tool
# Please refer to https://arduino.github.io/arduino-cli/installation/
# Get OS Type
SYSTEM=`uname -s`
SYSTEM=${SYSTEM%_*}
echo $SYSTEM
export BINDIR=/usr/local/bin
ARDUINO_CLI_FILEPATH=$BINDIR/arduino-cli

if [ $SYSTEM = "Linux" ] ; then
echo "Intalling on Linux"
elif [ $SYSTEM = "Darwin" ] ; then
echo "Intalling on MAC"
brew uninstall arduino-cli
brew install arduino-cli
elif [ $SYSTEM = "MINGW64" ] || [ $SYSTEM = "MINGW32" ] ; then
echo "Intalling on Windows"
export BINDIR=$HOME/bin
ARDUINO_CLI_FILEPATH=$BINDIR/arduino-cli.exe
mkdir -p $BINDIR
./tools/install.sh
else
echo "Error Unknow system"
exit
fi

# update the index of available boards
arduino-cli core update-index
#and download the core needed with specified version
arduino-cli core install arduino:samd@1.8.6

#We need to do same for the libraries
arduino-cli lib update-index

#Check if installation went ok
output_string=$(arduino-cli core list)

if [[ $output_string != *"arduino:samd"* ]]; then
	echo -e "\e[31m"
  	echo "ERROR: Failed to install core arduino:samd"
	echo -e "\e[39m"
fi

echo "If no errors occurred, the Arduino compile environment for Qeexo AutoML installed successfully"
