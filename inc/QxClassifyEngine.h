/**
 ******************************************************************************
 * @file    QxAutoMLInf.h
 * @author  Qeexo Kernel Development team
 * @version V1.0.0
 * @date    30-Sep-2020
 * @brief   Auto ML module for Inference 
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2020 Qeexo Co.
 * All rights reserved.
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
#ifndef _QXCLASSIFYENGINE_H_
#define _QXCLASSIFYENGINE_H_


#include "QxOS.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifdef QXO_GLOBALS
#define QXO_EXTERN
#else
#define QXO_EXTERN extern
#endif

/* sensor type definitions inside inference Engine. 
 * Customer should define the sensor index as the same sequence regarding to 
 * fill the prediction data buffer with multiple sensors*/
typedef enum {
  SENSOR_TYPE_NONE = 0, /*!< None defined sensor */
  SENSOR_TYPE_ACCEL, /*!< Default accelerometer sensor */
  SENSOR_TYPE_GYRO, /*!< Default gyroscope sensor */
  SENSOR_TYPE_MAG, /*!< Megnotometer sensor */
  SENSOR_TYPE_PRESSURE, /*!< Pressure sensor */
  SENSOR_TYPE_TEMPERATURE, /*!< Temperature sensor */
  SENSOR_TYPE_HUMIDITY, /*!< Humidity sensor */
  SENSOR_TYPE_MICROPHONE, /*!< Microphone sensor */
  
  SENSOR_TYPE_ACCEL_LOWPOWER, /*!< Additional lowpower accelometer sensor */
  SENSOR_TYPE_ACCEL_HIGHSENSITIVE, /*!< Additional high sensitive accelometer sensor */
  SENSOR_TYPE_TEMPERATURE_EXT1, /*!< Additional temperature sensor */
  SENSOR_TYPE_PROXIMITY, /*!< Proximity sensor */
  SENSOR_TYPE_AMBIENT, /*!< Ambient light sensor */
  SENSOR_TYPE_LIGHT,  /*!<Light sensor with one axis>*/
  SENSOR_TYPE_MAX
}QXOSensorType;

typedef enum {
  MLENGINE_OK = 0,
  MLENGINE_ERROR_UNKNOWN,
} MLEngineStatus_t;

typedef struct {
  QXOSensorType sensor_type;
  uint32_t buff_end;
  uint32_t buff_max;
  uint8_t* buff_ptr;
}SensorData;

typedef enum {
  ONDEVICE_INIT = 0,
  ONDEVICE_DC,
  ONDEVICE_TRAINING,
  ONDEVICE_CLASSIFY,
} MLOnDeviceModelStatus_t;


typedef struct {
	uint8_t mEnabledSensorCount;
  uint8_t mEventClassificationStatus;
	SensorData* mSensorData;
  tQxMutex* mFrameMutex;
  float mProbs[50];
  MLOnDeviceModelStatus_t mOnDeviceModelStatus;
}PredictionFrame, *pPredictionFrame;

/* Initialize Engine. Allocate memory for buffering sensor data for prediction purpose */

QXO_EXTERN pPredictionFrame QXO_MLEngine_Init(void);

/* Deallocate all ML Engine resources. */
QXO_EXTERN MLEngineStatus_t QXO_MLEngine_DeInit(pPredictionFrame frame);

/* This function performs the inference work. It takes PredictionFrame for input and output the classification result*/
QXO_EXTERN int QXO_MLEngine_Work(pPredictionFrame frame, int mEvClsStatus);

/* Gets engine preferred prediction interval in millionseconds*/
QXO_EXTERN int QXO_MLEngine_GetPredictionInterval(void);

#ifdef __cplusplus
}
#endif

#endif