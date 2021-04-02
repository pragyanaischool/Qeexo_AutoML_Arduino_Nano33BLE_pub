#!/bin/bash
PROJECT=embedded_ml-arduino-nano-33-ble-sense
BOARD=arduino:mbed:nano33ble
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
echo MBED_PATH:$MBED_PATH
WINPTY=winpty
#exit
else
echo "Error Unknow system"
exit
fi

MBED_PATH="$COMPILE_PATH/mbed-os"

if [ "$COMMAND" = "--build" ] || [ "$COMMAND" = "-b" ] || [ "$COMMAND" = "--build-with-battery" ] || [ "$COMMAND" = "-bb" ];
then
	echo "Building $PROJECT"
if [ "$COMMAND" = "--build" ] || [ "$COMMAND" = "-b" ]
then
    cp automl-arduino-nano-33ble-sense-no-battery.ino.qeexo automl-arduino-nano-33ble-sense.ino
fi

if [ "$COMMAND" = "--build-with-battery" ] || [ "$COMMAND" = "-bb" ]
then
    cp automl-arduino-nano-33ble-sense-battery.ino.qeexo automl-arduino-nano-33ble-sense.ino
fi
    STATIC_LIB_PATH=$2
    if [ "$STATIC_LIB_PATH" = "" ]
    then
        STATIC_LIB_PATH={compiler.demo_root}/libs/libQxClassifyEngine.a
    fi
	$WINPTY arduino-cli compile --fqbn  $BOARD --verbose --libraries ./libs $COMPILE_PATH --build-path $COMPILE_PATH/output  \
    --build-properties "compiler.demo_root=$COMPILE_PATH"\
    --build-properties "compiler.mbed='$STATIC_LIB_PATH' '{compiler.demo_root}/libs/libQxSensorHal.a' '{compiler.demo_root}/libs/mbed-core-ARDUINO_NANO33BLE.a' '{compiler.demo_root}/libs/libarm_cortexM4lf_math.a'"\
    --build-properties 'compiler.mbed.cflags={compiler.demo_root}/variants/cflags.txt'\
    --build-properties 'compiler.mbed.cxxflags={compiler.demo_root}/variants/cxxflags.txt'\
    --build-properties 'compiler.mbed.ldflags={compiler.demo_root}/variants/ldflags.txt'\
    --build-properties 'compiler.mbed.extra_ldflags=-lstdc++ -lsupc++ -lm -lc -lgcc -lnosys -mfloat-abi=hard'\
    --build-properties "recipe.c.o.pattern='{compiler.path}{compiler.c.cmd}' {compiler.c.flags} -DARDUINO={runtime.ide.version} -DARDUINO_{build.board} -DARDUINO_ARCH_{build.arch}  {build.extra_flags} {compiler.c.extra_flags} '-I{build.core.path}/api/deprecated' {includes} '-iprefix{build.core.path}' '@{compiler.mbed.includes}' '-I$COMPILE_PATH/inc' -o '{object_file}' '{source_file}'"\
    --build-properties "recipe.cpp.o.pattern='{compiler.path}{compiler.cpp.cmd}' {compiler.cpp.flags} -DARDUINO={runtime.ide.version} -DARDUINO_{build.board} -DARDUINO_ARCH_{build.arch} {includes} {build.extra_flags} {compiler.cpp.extra_flags} '-I{build.core.path}/api/deprecated' '-iprefix{build.core.path}' '@{compiler.mbed.includes}' '-I$COMPILE_PATH/inc' '{source_file}' -o '{object_file}' "\
    --build-properties 'build.float-abi=hard'
    
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
      rm automl-arduino-nano-33ble-sense.ino
elif [ "$COMMAND" = "--flash" ] || [ "$COMMAND" = "-f" ];
then
    INPUT_FILE=$2
    if [ "$INPUT_FILE" = "" ]
    then
        INPUT_FILE=$COMPILE_PATH/output/automl-arduino-nano-33ble-sense.ino.bin
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
if [ $SYSTEM = "Darwin" ] ; then
    cat $(arduino-cli board list | grep Arduino | cut -d ' ' -f1)
else
    $WINPTY python -u tools/debuglog.py -p $(arduino-cli board list | grep Arduino | cut -d ' ' -f1)
fi
elif [ "$COMMAND" = "--mbedcore" ] || [ "$COMMAND" = "-m" ];
then
    BUILD_PATH=BUILD/ARDUINO_NANO33BLE/GCC_ARM
if [ $SYSTEM = "Darwin" ] ; then
    mbed config root $MBED_PATH
    PATH=/Users/$USER/Library/Arduino15/packages/arduino/tools/arm-none-eabi-gcc/7-2017q4/bin:$PATH
    python3 -u $MBED_PATH/tools/make.py -t GCC_ARM -m ARDUINO_NANO33BLE --source $MBED_PATH  --build $COMPILE_PATH/$BUILD_PATH -v
else
    /c/mbed-cli/mbed-cli/mbed-cli-1.8.3/mbed/mbed.py config root  $MBED_PATH
    /c/mbed-cli/mbed-cli/mbed-cli-1.8.3/mbed/mbed.py toolchain GCC_ARM

    PATH=/c/Users/$USERNAME/AppData/Local/Arduino15/packages/arduino/tools/arm-none-eabi-gcc/7-2017q4/bin:$PATH
    $WINPTY python -u mbed-os/tools/make.py -t GCC_ARM -m ARDUINO_NANO33BLE --source mbed-os  --build $BUILD_PATH -v
fi
    # remove old libmbed.a and generate new library
    rm -rf ./libs/mbed-core-ARDUINO_NANO33BLE.a
    tr  ' ' '\n' < $BUILD_PATH/.link_options.txt | grep '\.o' \
     | grep -v '/main\.o' | xargs arm-none-eabi-ar rcs \
     ./libs/mbed-core-ARDUINO_NANO33BLE.a
elif [ "$COMMAND" = "--help" ] || [ "$COMMAND" = "-h" ] || [ "$COMMAND" = "" ];
then
	echo 'useage: automl-build.sh [-h] [-b <STATIC_LIB_PATH>] [-f] [-m] [-d]
     for example:
         automl-build.sh -b   //build arduino sketch demo using default static library filepath
         automl-build.sh -b  ~/nano-xgb-1.0-static.a  //build arduino sketch demo using gived static library filepath
     -h, --help           Show this help message and exit
     -b, --build          Build Arduino Nano 33 BLE binary
        <STATIC_LIB_PATH>  the static library filepath to link, the default filepath is: ./libs/libQxClassifyEngine.a
     -bb, --build-with-battery    Build Arduino Nano 33 BLE binary with Qeexo battery board
        <STATIC_LIB_PATH>  the static library filepath to link, the default filepath is: ./libs/libQxClassifyEngine.a
     -f, --flash          Flash binary to Arduino Nano 33BLE device, please make sure the device is
                           connected to USB and has been in bootloader mode. (by double clicking button)
     -d, --debuglog       Print devices debug log from USB
     -m, --mbedcore       Build mBed OS library for Arduino Nano 33BLE
     '
fi
