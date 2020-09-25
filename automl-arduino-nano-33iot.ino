/*
  Qeexo AutoML Demo

  This example creates a Qeexo AutoML predict demo with two threads that shows how
  to use static library in user application that runs on Arduino Nano 33 BLE


  This example code is in the public domain.
*/
#include <FreeRTOS.h>
#include <DMA.h>
#include <QxAutoML.h>
#include "QxAutoMLInf.h"

const int ledPin = LED_BUILTIN; // set ledPin to on-board LED

uint16_t          LSM6DSMFifoCount      = 0;
uint16_t          QxMlLatency           = 0;
uint32_t          QxMlUpdateTime        = 0;
uint8_t           QxClassificationResult= 0;

QxAutoMLInf  QxAutoMLInf(NULL, NULL);  

 
void ClassifyLoop(const void *) {
 
    while(1) {
        /* Get classification calling interval(ms) from library */
        int interval = QxAutoMLInf.GetInterval();

        /* Get current tick in ms */
        uint32_t tick = QxOS_GetTick();

        /* Call classify periodically */
        QxClassificationResult = (uint8_t)QxAutoMLInf.Classify();
        
        QxMlLatency = (uint16_t)QxOS_GetTick() - tick;
        QxOS_DebugPrint("Result: %d, lantency: %d, interval: %d",
            QxClassificationResult, QxMlLatency, QxAutoMLInf.GetInterval());
        if (QxClassificationResult) {
            QxOS_DebugPrint("set led off");
            digitalWrite(ledPin, LOW);
        } else {
            QxOS_DebugPrint("set led on");
            digitalWrite(ledPin, HIGH);
        }

        if(QxMlLatency < interval) {
            QxOS_Delay(interval - QxMlLatency);
        }
    }
}

void setup() {

    /* use the LED as an output to show the prediction result*/
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    while(!Serial) {
        delay(1);         // will pause the chip until it opens serial console
    }

    /* Call this function to init all necessary preparations for clasification  */
    QxAutoMLInf.InitEngine();

    QxOS_CreateThread("ClassifyLoop", ClassifyLoop, configTASK_PRIORITY_CLASSIFY, configTASK_STACK_CLASSIFY, NULL);
}

void loop()
{
    // This function will never run.
    QxOS_StartKernel();
}
