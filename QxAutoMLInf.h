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

#ifndef __QXAUTOMLINF__
#define __QXAUTOMLINF__

#include <Arduino.h>
#include "QxClassifyEngine.h"
#include "QxSensorHal_Nano33Iot.h"

#define configTASK_PRIORITY_CLASSIFY QxPriorityAboveNormal
#define configTASK_STACK_CLASSIFY 1536

#define configTASK_PRIORITY_DATA_LOG QxPriorityAboveNormal
#define configTASK_STACK_DATA_LOG 2048

#define configTASK_PRIORITY_SENSOR QxPriorityHigh
#define configTASK_STACK_SENSOR 2048

class QxAutoMLInf
{

public:
    
  QxAutoMLInf(void*, void*);
  void InitEngine();
  void sensorInit();
  void SetDataFrame(PredictionFrame *dataframe);
  void FillDataLoop(const void *);
  void FillDataFrame();
  int Classify();
  int GetInterval();
  void CopyDataToSensorData(void* data, SensorData *sensor, int data_len);


private:
  pPredictionFrame   mPred;
  int mNumOfClasses;
  float mEngineSensitivity[50];
  int mPredictionInterval;

  SensorData  *mAccelData = NULL;
  SensorData  *mGyroData = NULL; 
  SensorData  *mMagData = NULL; 

  SensorData  *mPCMData = NULL; 

};

#endif // __QXAUTOMLINF__
