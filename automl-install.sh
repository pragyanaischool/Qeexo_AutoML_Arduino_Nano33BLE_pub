#!/bin/bash
# Get OS Type
SYSTEM=`uname -s`
SYSTEM=${SYSTEM%_*}
echo $SYSTEM
export BINDIR=/usr/local/bin
ARDUINO_CLI_FILEPATH=$BINDIR/arduino-cli

#First download mbed-os source and make patch
git clone https://github.com/ARMmbed/mbed-os.git mbed-os
cd mbed-os
isContinue='y'
echo "Warning!! continue to remove all files in mbbed-os folder, please make sure you have committed your changes before enter 'y'"
read -p 'continue?[y/n]' isContinue
if [ "y" !=  ${isContinue} ];
then
	exit
fi

# temporary remove this part to save time for test
rm -rf *
rm .gitmodules
git checkout origin/mbed-os-5.15.3-rc1 -b mbed-os-5.15.3-rc1_qeexo_automl_arduino_nano_33_ble$RANDOM
git reset --hard
git apply ../automl-arduino-nano-33ble-sense.patch

# remove soft float built libraries
rm features/cryptocell/FEATURE_CRYPTOCELL310/TARGET_MCU_NRF52840/TOOLCHAIN_GCC_ARM/libcc_310_ext.a  features/cryptocell/FEATURE_CRYPTOCELL310/TARGET_MCU_NRF52840/TOOLCHAIN_GCC_ARM/libcc_310_trng.a  features/cryptocell/FEATURE_CRYPTOCELL310/binaries/TOOLCHAIN_GCC_ARM/libcc_310_core.a
cd ..

#Second Install mbed requirements
if [ $SYSTEM = "Linux" ] ; then 
echo "Intalling on Linux"
elif [ $SYSTEM = "Darwin" ] ; then
echo "Intalling on MAC"
brew install mercurial
brew install bash
python3 -m pip install mbed-cli
pip3 install -r ./mbed-os/requirements.txt
elif [ $SYSTEM = "MINGW64" ] || [ $SYSTEM = "MINGW32" ] ; then
echo "Intalling on Windows"
export BINDIR=$HOME/bin
ARDUINO_CLI_FILEPATH=$BINDIR/arduino-cli.exe
python -m pip install --upgrade pip
python -m pip install -r ./mbed-os/requirements.txt
else
echo "Error Unknow system"
exit
fi

#Third install arduino-cli tool
#Please refer to https://arduino.github.io/arduino-cli/installation/
#We need to remove old installed arduino-cli to avoid wrong version problem
rm $ARDUINO_CLI_FILEPATH
mkdir -p $BINDIR
./install.sh

#Then we update the index of available boards
arduino-cli core update-index
#and download the core needed with specified version
arduino-cli core install arduino:mbed@1.1.6

#We need to do same for the libraries
arduino-cli lib update-index

#Check if installation went ok
output_string=$(arduino-cli core list)

if [[ $output_string != *"arduino:mbed"* ]]; then
	echo -e "\e[31m"
  	echo "ERROR: Failed to install core arduino:mbed"
	echo -e "\e[39m"
fi

# Build libmbed.a 
./automl-build.sh -m
echo "If no errors occurred, the Arduino compile environment for Qeexo AutoML installed successfully"
