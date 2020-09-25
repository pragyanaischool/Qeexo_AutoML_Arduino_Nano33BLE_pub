/**
  ******************************************************************************
  * @file    QxAutoMLInf.cpp
  * @author  Qeexo Kernel Development team
  * @version V1.0.0
  * @date    30-Sep-2020
  * @brief   Auto ML module for Inference 
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020 Qeexo Co.
  * All rights reserved
  *
  *
  * ALL INFORMATION CONTAINED HEREIN IS AND REMAINS THE PROPERTY OF QEEXO, CO.
  * THE INTELLECTUAL AND TECHNICAL CONCEPTS CONTAINED HEREIN ARE PROPRIETARY TO
  * QEEXO, CO. AND MAY BE COVERED BY U.S. AND FOREIGN PATENTS, PATENTS IN PROCESS,
  * AND ARE PROTECTED BY TRADE SECRET OR COPYRIGHT LAW. DISSEMINATION OF
  * THIS INFORMATION OR REPRODUCTION OF THIS MATERIAL IS STRICTLY FORBIDDEN UNLESS
  * PRIOR WRITTEN PERMISSION IS OBTAINED OR IS MADE PURSUANT TO A LICENSE AGREEMENT
  * WITH QEEXO, CO. ALLOWING SUCH DISSEMINATION OR REPRODUCTION.
  *
  ******************************************************************************
 */

#include "QxAutoMLInf.h"

rtos::Thread sample_thread;

/* Get engine’s sensitivity parameter and the number of inference classes */
extern "C"  void QXO_MLEngine_GetSensitivity(float * pSensitivity, int * pNumOfClasses);

/*
    This function returns accel & gyro fifo sample data count, which is the remainning
    sensor data number, each number's size is 6 bytes 3axis.
    Please refer to https://content.arduino.cc/assets/Nano_BLE_Sense_lsm9ds1.pdf
*/
uint16_t lsm9ds1_read_fifocount()
{
    uint16_t fifocount;
    uint8_t reg = 0;

    Sensor_I2CReadReg(LSM9DS1_SLAVE_ADDR, LSM9DS1_REG_FIFO_SRC, &reg, 1);

    fifocount = (reg & 0x3f);
    bool overwritten = !!(reg & 0x40);
    if (overwritten){
        Serial.println("lsm9ds1 over written");
    }
    return fifocount;
}

/*
    This function read the accel and gyro sensor data to the gived input 'data' buffer, the read number
    dependeds on parameter 'remaining'.
    Please refer to https://content.arduino.cc/assets/Nano_BLE_Sense_lsm9ds1.pdf
 */
tQxStatus lsm9ds1_read_fifoData(uint16_t remaining, int16_t* accel_data, int16_t* gyro_data)
{
    int16_t read_samples = 1;

    while(remaining > 0) {
        //read one accel sample data
        Sensor_I2CReadReg(LSM9DS1_SLAVE_ADDR, LSM9DS1_REG_OUT_X_XL, (uint8_t *)accel_data, 6*read_samples);
        accel_data += 3*read_samples;

        //read one gyro sample data
        Sensor_I2CReadReg(LSM9DS1_SLAVE_ADDR, LSM9DS1_REG_OUT_X_G, (uint8_t *)gyro_data, 6*read_samples);
        gyro_data += 3*read_samples;

        /* The remaining indicates the pair of the accel&gyro sample data*/
        remaining -= read_samples;
    }

    return QxOK;
}

/*
    This funtion configs the format of sensor data output.
    Please change the parameters to what you set in the Qeexo AutoML data collection page.
*/
void QxAutoMLInf::sensorInit(void)
{
    /* init and enable accelerometer @& gyrometer */
    if (mAccelData) {
        lsm9ds1_acc_init(NULL);
        lsm9ds1_acc_enable(NULL,
                          16.0,    /* ACCEL FULL SCALE RANGE */
                          952.0,   /* ACCEL SAMPLING RATE */
                          TRUE);
    }

    if (mGyroData) {
        lsm9ds1_gyro_init(NULL);
        lsm9ds1_gyro_enable(NULL,
                            2000.0, /* GYRO FULL SCALE RANGE */
                            952.0,  /* GYRO SAMPLING RATE */
                            TRUE);
    }

    if (mMagData) {
        lsm9ds1_mag_init(NULL);

         /* MAG's params are fixed values */
        lsm9ds1_mag_enable(NULL, 16.0, 100.0, FALSE);
    }

    if (mPCMData) {
        mp34dt05_microphone_init(NULL);

        /* MIC's params are fixed values */
        mp34dt05_microphone_enable(NULL, 32768.0f, 16000.0f, FALSE);
    }
}

QxAutoMLInf::QxAutoMLInf(void* lsm6dsm, void* lis2mdl):
    _thread_sensor_read(osPriorityISR, 4096, NULL, "sensor_read_thread")
{
    /* Nothing to do */
}

/*
    This funtion initialize all requeirements for classification
*/
void QxAutoMLInf::InitEngine()
{
    Serial.println("Ready to initialize MLEngine!!");

    /* Initialize BSP and Sensor HAL layer for Qeexo AutoML */
    QxOS_InitializeBSP();

    /* Initialize Engine. Allocate memory for buffering sensor data for prediction purpose */
    mPred = QXO_MLEngine_Init();

    mPredictionInterval = QXO_MLEngine_GetPredictionInterval();
    memset(mEngineSensitivity, 0.0, sizeof(mEngineSensitivity));
    QXO_MLEngine_GetSensitivity(&mEngineSensitivity[0], &mNumOfClasses);

    /* In the following section, we assign the prediction sensor data buffer pointers to
        each private pointer variables, then we can feed the sensor data separately.  */
    if(mPred != NULL) {
        for(int i = 0; i < mPred->mEnabledSensorCount; i++) {
            if(mPred->mSensorData[i].sensor_type == SENSOR_TYPE_ACCEL) {
                mAccelData = &mPred->mSensorData[i];
                Serial.print("Init mAccelData.");
                Serial.println(mAccelData->buff_max);
                Serial.println(mAccelData->buff_end);
            }

            if(mPred->mSensorData[i].sensor_type == SENSOR_TYPE_GYRO) {
                mGyroData = &mPred->mSensorData[i];
                Serial.print("Init mGyroData.");
                Serial.println(mGyroData->buff_max);
                Serial.println(mGyroData->buff_end);
            }

            if(mPred->mSensorData[i].sensor_type == SENSOR_TYPE_MICROPHONE) {
                mPCMData = &mPred->mSensorData[i];
                Serial.println("Init mPCMData");
            }
        }
        Serial.println("MLEngine is ready!!");

        /* After private pointer variables is assigned, we call sensorInit()
            to init any sensors that are used by current static classify engine libarary. */
        sensorInit();

        /* Here we create a thread and use ticker & event queue methods to trigger periodically
            sensor data feeding, the feeding interval should be 10ms. */
        _thread_sensor_read.start(mbed::callback(this, &QxAutoMLInf::FillDataLoop));
    } else {
        Serial.println("MLEngine init error!!");
    }
}

void CopyDataToSensorData(void* data, SensorData *sensor, int data_len)
{
    if (data_len > sensor->buff_max) {
        data_len = sensor->buff_max;
    }

    int delta = sensor->buff_max - (data_len + sensor->buff_end);
    if(delta >= 0) {
        memcpy((sensor->buff_ptr+sensor->buff_end), (uint8_t*)data, data_len );
        sensor->buff_end += data_len;
    } else {
        //remove oldest data to make a room for new data
        memmove(sensor->buff_ptr, sensor->buff_ptr+abs(delta), sensor->buff_max + delta );
        memcpy((sensor->buff_ptr+sensor->buff_end + delta), (uint8_t*)data, data_len );
    }
}


#define MAX_FIFO_BUFFER 32 //samples
#define AXIS_NUMBER 3 

#define MIN(a,b) ((a < b) ? a : b)


void QxAutoMLInf::FillDataLoop()
{
    /* Get classification calling interval(ms) from library */
    int interval = 10;

    while(1){
        /* Get current tick in ms */
        uint32_t tick = QxOS_GetTick();

        /* Call classify periodically */
        FillDataFrame();

        uint32_t diff = QxOS_GetTick() - tick;


        if(diff < interval) {
            QxOS_Delay(interval - diff);
        }
    }
}

void QxAutoMLInf::FillDataFrame()
{
   /* 1. read accel and gyro data */
    uint16_t read_samples = 0, remained_samples = 0;

    remained_samples = lsm9ds1_read_fifocount();

    if(remained_samples > 0) {
        static int16_t accel_data[AXIS_NUMBER * MAX_FIFO_BUFFER];
        static int16_t gyro_data[AXIS_NUMBER * MAX_FIFO_BUFFER];

        read_samples = MIN(remained_samples, MAX_FIFO_BUFFER);

        lsm9ds1_read_fifoData(read_samples, accel_data, gyro_data);

        //Serial.print("FillDataFrame:");
        //Serial.println(read_samples);

        if(mAccelData) {
            CopyDataToSensorData(accel_data, mAccelData, read_samples*6);
        }

        if(mGyroData) {
            CopyDataToSensorData(gyro_data, mGyroData, read_samples*6);
        }
     }

    /* 2. read MAG data */
    if(mMagData) {
        static uint8_t data[MAG_BUFF_MAX];
        uint8_t status = 0;
        int data_len = 6;

        Sensor_I2CReadReg(LSM9DS1_MAG_ADDR, LSM9DS1_STATUS_REG_M, &status, 1);
        Sensor_I2CReadReg(LSM9DS1_MAG_ADDR, LSM9DS1_OUT_X_L_M, data, data_len);

        CopyDataToSensorData(data, mMagData, data_len);
    }

    /* 3. read PCM data */
    if(mPCMData) {
        static uint8_t data[MICROPHONE_BUFF_MAX];
        int data_len = MICROPHONE_BUFF_MAX;
        QxAudioHal_GetPCMBuf((int16_t *)data, MICROPHONE_BUFF_MAX/sizeof(int16_t));

        CopyDataToSensorData(data, mPCMData, data_len);
    }
}

int QxAutoMLInf::Classify()
{
    /* Call classification prediction, the input sensor data in 'mPred' is feeding
        in another thread that created by QxAutoMLInf::InitEngine() */
    int cls = 0;
    cls = QXO_MLEngine_Work(mPred, 0);
    const uint8_t buffsize = 125;
    char classifylogBuffer[buffsize];
    int offset = 0;

    offset = sprintf(classifylogBuffer, "PRED: %d", cls);

    for (int i = 0; i < mNumOfClasses; i++) {
        offset += sprintf(classifylogBuffer + offset, ", %.2f", mPred->mProbs[i]);
    }

    Serial.println(classifylogBuffer);

    return cls;
}

int QxAutoMLInf::GetInterval()
{
    /* Gets engine preferred prediction interval in millionseconds*/
    return QXO_MLEngine_GetPredictionInterval();
}
