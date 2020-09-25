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

#include <functional>

#include "QxAutoMLInf.h"

#define LSM6DS3_ADDRESS            0x6A
#define LSM6DS3_FIFO_STATUS1       0x3A
/* two consecutive registers for FIFO: L, H */
#define LSM6DS3_FIFO_DATA_OUT      0x3E


// max 6667 samples/sec x 0.01sec = 67samples + 3samples tolerrance @10ms
#define MAX_FIFO_BUFFER 80 //samples
#define AXIS_NUMBER 3
#define ACC_BUFF_MAX (MAX_FIFO_BUFFER*AXIS_NUMBER)
#define GYRO_BUFF_MAX (MAX_FIFO_BUFFER*AXIS_NUMBER)

#define MIN(a,b) ((a < b) ? a : b)

//#define DEBUG_DATA
/* Safe "sizeof_array" macro. http://cnicholson.net/2011/01/stupid-c-tricks-a-better-sizeof_array/ */
namespace detail {
    template< typename T, size_t N > char (&SIZEOF_ARRAY_REQUIRES_ARRAY_ARGUMENT(T (&)[N]))[N];
}
#define sizeof_array(x) sizeof(detail::SIZEOF_ARRAY_REQUIRES_ARRAY_ARGUMENT(x))

/* Get engine’s sensitivity parameter and the number of inference classes */
extern "C"  void QXO_MLEngine_GetSensitivity(float * pSensitivity, int * pNumOfClasses);

/*
    This function returns accel & gyro fifo sample data count, which is the remainning
    sensor data number, each number's size is 6 bytes 3axis.
    Please refer to https://content.arduino.cc/assets/Nano_BLE_Sense_lsm6ds3.pdf
*/
uint16_t lsm6ds3_read_fifocount()
{
    uint16_t fifocount;
    uint8_t status[2];

    if (QxOK!=Sensor_I2CReadRegs(LSM6DS3_ADDRESS, LSM6DS3_FIFO_STATUS1, status, 2)) {
        Serial.println("Error! Sensor_I2CReadRegs return failure");
    }
    fifocount = status[0] | ((status[1] & 0xf) << 8);
    if(status[1] & 0x20)
        Serial.println("lsm6ds3: overflow!");

    return fifocount;
}

/*
    This function read the accel and gyro sensor data to the gived input 'data' buffer, the read number
    dependeds on parameter 'remaining'.
    Please refer to https://content.arduino.cc/assets/Nano_BLE_Sense_lsm6ds3.pdf
 */

static int16_t sample_buf[126];
int lsm6ds3_read_fifoData(uint16_t remaining, int16_t* accel_data, int16_t* gyro_data)
{
    int16_t read_samples = 1;
    int acc_index = 0;
    int gyro_index = 0;
    int fifocount = remaining / 6 * 6;

     while(fifocount) {
         int readcount = fifocount;
         if(readcount > sizeof_array(sample_buf))
             readcount = sizeof_array(sample_buf);

         if (QxOK!=Sensor_I2CReadRegs(LSM6DS3_ADDRESS, LSM6DS3_FIFO_DATA_OUT,
                                        (uint8_t *)sample_buf, readcount * 2)) {
            Serial.println("Error! Sensor_I2CReadRegs return failure");
         }
         int buf_index = 0;
         while(buf_index < readcount) {
             if(gyro_data) {
                 if(gyro_index < GYRO_BUFF_MAX) {
                     memcpy(&gyro_data[gyro_index], &sample_buf[buf_index], 6);
                     gyro_index += 3;
                 }
                 buf_index += 3;
             }
             if(accel_data) {
                 if(acc_index < ACC_BUFF_MAX) {
                     memcpy(&accel_data[acc_index], &sample_buf[buf_index], 6);
                     acc_index += 3;
                 }
                 buf_index += 3;
             }
         }

         fifocount -= readcount;
     }

    return acc_index;
}

template <typename T>
struct Callback;

template <typename ReturnType, typename... Signature>
struct Callback<ReturnType(Signature...)>
{
  template <typename... Args>
  static ReturnType callback(Args... args)
  {
    return func(args...);
  }
  inline static std::function<ReturnType(Signature...)> func;
};

/*
    This funtion configs the format of sensor data output.
    Please change the parameters to what you set in the Qeexo AutoML data collection page.
*/
void QxAutoMLInf::sensorInit(void)
{
    /* init and enable accelerometer @& gyrometer */
    if (mAccelData) {
        lsm6ds3_acc_init(NULL);
        lsm6ds3_acc_enable(NULL,
                           2.0,    /* ACCEL FULL SCALE RANGE */
                           1660.0,   /* ACCEL SAMPLING RATE */
                           TRUE);
    }

    if (mGyroData) {
        lsm6ds3_gyro_init(NULL);
        lsm6ds3_gyro_enable(NULL,
                            125.0, /* GYRO FULL SCALE RANGE */
                            1660.0,  /* GYRO SAMPLING RATE */
                            TRUE);
    }

}

QxAutoMLInf::QxAutoMLInf(void* lsm6dsm, void* lis2mdl)
{
    //Do nothing.
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
    if (mPred != NULL) {
        for (int i = 0; i < mPred->mEnabledSensorCount; i++) {
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
        }
        Serial.println("MLEngine is ready!!");

        /* After private pointer variables is assigned, we call sensorInit()
            to init any sensors that are used by current static classify engine libarary. */
        sensorInit();

        /* Here we create a thread and use ticker & event queue methods to trigger periodically
            sensor data feeding, the feeding interval should be 10ms. */
        Callback<void(const void*)>::func = std::bind(&QxAutoMLInf::FillDataLoop, this, std::placeholders::_1);
        QxOS_CreateThread("ReadSensor", static_cast<tQxThreadFunc>(Callback<void(const void*)>::callback) , 
                            configTASK_PRIORITY_SENSOR, configTASK_STACK_SENSOR, NULL);
    } else {
        Serial.println("MLEngine init error!!");
    }
}

void QxAutoMLInf::CopyDataToSensorData(void* data, SensorData *sensor, int data_len)
{
    if (data_len > sensor->buff_max) {
        data_len = sensor->buff_max;
    }

    QxOS_LockMutex(mPred->mFrameMutex);

    int delta = sensor->buff_max - (data_len + sensor->buff_end);
    if(delta >= 0) {
        memcpy((sensor->buff_ptr+sensor->buff_end), (uint8_t*)data, data_len );
        sensor->buff_end += data_len;
    } else {
        //remove oldest data to make a room for new data
        memmove(sensor->buff_ptr, sensor->buff_ptr+abs(delta), sensor->buff_max + delta );
        memcpy((sensor->buff_ptr+sensor->buff_end + delta), (uint8_t*)data, data_len );
    }

    QxOS_UnLockMutex(mPred->mFrameMutex);
}

void QxAutoMLInf::FillDataLoop(const void *)
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

    remained_samples = lsm6ds3_read_fifocount();

    if(remained_samples > 0) {
        static int16_t accel_data[ACC_BUFF_MAX];
        static int16_t gyro_data[GYRO_BUFF_MAX];

        read_samples = lsm6ds3_read_fifoData(remained_samples, accel_data, gyro_data);

#ifdef DEBUG_DATA
        Serial.print("FillDataFrame:"); Serial.println(read_samples);
#endif

        if(mAccelData) {
#ifdef DEBUG_DATA
            Serial.print("mAccelData:"); Serial.print(accel_data[0]);Serial.print(", ");
            Serial.print(accel_data[1]);Serial.print(", "); Serial.println(accel_data[2]);
#endif
            CopyDataToSensorData(accel_data, mAccelData, read_samples*2);
        }

        if(mGyroData) {
#ifdef DEBUG_DATA
            Serial.print("mGyroData:"); Serial.print(gyro_data[0]);Serial.print(", ");
            Serial.print(gyro_data[1]);Serial.print(", "); Serial.println(gyro_data[2]);
#endif
            CopyDataToSensorData(gyro_data, mGyroData, read_samples*2);
        }
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
