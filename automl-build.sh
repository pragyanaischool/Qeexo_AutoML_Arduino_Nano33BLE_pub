#!/bin/bash
PROJECT=embedded_ml-arduino-nano-33-iot
BOARD=arduino:samd:nano_33_iot
COMMAND=$1

COMPILE_PATH="$(PWD)"
WINPTY=

# Get OS Type
SYSTEM=`uname -s`
SYSTEM=${SYSTEM%_*}
echo $SYSTEM
ARDUINO_CLI_FILEPATH=/usr/bin/arduino-cli
BINDIR=/usr/bin
if [ $SYSTEM = "Linux" ] ; then 
echo "Running on Linux"
elif [ $SYSTEM = "Darwin" ] ; then
echo "Running on MAC"
elif [ $SYSTEM = "MINGW64" ] || [ $SYSTEM = "MINGW32" ] ; then
echo "Running on Windows"
COMPILE_PATH1=${COMPILE_PATH:1:1}
COMPILE_PATH2=${COMPILE_PATH:2}
COMPILE_PATH=$COMPILE_PATH1:$COMPILE_PATH2
echo COMPILE_PATH:$COMPILE_PATH

WINPTY=winpty
#exit
else
echo "Error Unknow system"
exit
fi
if [ "$COMMAND" = "--build" ] || [ "$COMMAND" = "-b" ];
then
	echo "Building $PROJECT"

    STATIC_LIB_PATH=$2
    if [ "$STATIC_LIB_PATH" = "" ]
    then
        STATIC_LIB_PATH={compiler.demo_root}/libs/libQxClassifyEngine.a
    fi
	arduino-cli compile --fqbn  $BOARD --verbose --libraries ./libraries $COMPILE_PATH --build-path $COMPILE_PATH/output  \
    --build-properties "compiler.demo_root=$COMPILE_PATH"\
    --build-properties "build.extra_flags=-mfloat-abi=soft -DQXO_ARDUINO -DUSE_FIXEDPOINT -mthumb -mcpu=cortex-m0plus \
    				-DARM_MATH_CM0PLUS '-I$COMPILE_PATH/inc' \
    			    --specs=nano.specs --specs=nosys.specs -DQXO_BUILD_CLASSIFY -DCRYSTALLESS -D__SAMD21G18A__ {build.usb_flags}"\
    --build-properties "compiler.arm.cmsis.ldflags=-L./libraries -larm_cortexM0l_math  -lQxClassifyEngine -lQxSensorHal"
    
	pid=$! # Process Id of the previous running command
	while kill -0 $pid 2>/dev/null
	do
		echo "Still building..."
		sleep 2
	done
	wait $pid
	ret=$?
	if [ $ret -eq 0 ]; then
		echo "Building $PROJECT done"
	else
		exit "Building $PROJECT failed"
	fi
elif [ "$COMMAND" = "--flash" ] || [ "$COMMAND" = "-f" ];
then
    INPUT_FILE=$2
    if [ "$INPUT_FILE" = "" ]
    then
        INPUT_FILE=$COMPILE_PATH/output/automl-arduino-nano-33iot.ino.bin
        echo "Flashing $INPUT_FILE to $BOARD"
    else
        isContinue='y'
        echo "Warning!! Flashing $INPUT_FILE to $BOARD, please confirm device type before enter 'y'"
        read -p 'continue?[y/n]' isContinue
        if [ "y" !=  ${isContinue} ];
        then
            exit
        fi
    fi

	arduino-cli upload -p $(arduino-cli board list | grep Arduino | cut -d ' ' -f1) --fqbn $BOARD --input-file $INPUT_FILE

elif [ "$COMMAND" = "--debuglog" ] || [ "$COMMAND" = "-d" ];
then
    cat $(arduino-cli board list | grep Arduino | cut -d ' ' -f1)
elif [ "$COMMAND" = "--help" ] || [ "$COMMAND" = "-h" ] || [ "$COMMAND" = "" ];
then
	echo 'useage: arduino-build.sh [-h] [-b <STATIC_LIB_PATH>] [-f] [-m] [-d]
     for example:
         arduino-build.sh -b   //build arduino sketch demo using default static library filepath
         arduino-build.sh -b  ~/nano-xgb-1.0-static.a  //build arduino sketch demo using gived static library filepath
     -h, --help           Show this help message and exit
     -b, --build          Build Arduino Nano 33 BLE binary
        <STATIC_LIB_PATH>  the static library filepath to link, the default filepath is: ./libs/libQxClassifyEngine.a
     -f, --flash          Flash binary to Arduino Nano 33BLE device, please make sure the device is
                           connected to USB and has been in bootloader mode. (by double clicking button)
     -d, --debuglog       Print devices debug log from USB
     '
fi
