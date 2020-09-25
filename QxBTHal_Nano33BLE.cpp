#include "QxBTHal.h"
#include <stdio.h>
#include "QxOS.h"
#include <ArduinoBLE.h>

#define serviceUUID  "00618b72-a321-389d-8849-cd74f9f0f4eb"
#define characteristicUUID "10618b72-a321-389d-8849-cd74f9f0f4eb"

BLEService automlService(serviceUUID); // create service

// create button characteristic and allow remote device to get notifications
BLEStringCharacteristic buttonCharacteristic(characteristicUUID, BLERead | BLENotify, 125);

tQxStatus QxBTHal_Initialize() {
    // begin initialization
    if (!BLE.begin()) {
        QxOS_DebugPrint("starting BLE failed!");
        return QxErr;
    }

    // set the local name peripheral advertises
    BLE.setLocalName("QX-Nano");
    // set the UUID for the service this peripheral advertises:
    BLE.setAdvertisedService(automlService);

    // add the characteristics to the service
    automlService.addCharacteristic(buttonCharacteristic);

    // add the service
    BLE.addService(automlService);

    // start advertising
    BLE.advertise();

    QxOS_DebugPrint("Bluetooth device active, waiting for connections...");


    Serial.print("automlService uuid = ");
    Serial.print(automlService.uuid());

    return QxOK;
}


void QxBTHal_Write(char *buf, int length) {
    if (BLE.connected()) {
        buttonCharacteristic.writeValue(buf);
    } else {
        //QxOS_DebugPrint("QxBTHal_Write error, BT not connected.");
    }
}

extern "C"
bool QxBTHal_Connected()
{
    return BLE.connected();
}
