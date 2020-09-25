/**
  ******************************************************************************
  * @file    QxSensorHal_Nano33E.h
  * @author  Qeexo Kernel Development team
  * @version V1.0.0
  * @date    30-Sep-2020
  * @brief   Header of Nano33 BLE Sensor Api definition
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

#ifndef QXSENSORHAL_NANO_33BLE_H_
#define QXSENSORHAL_NANO_33BLE_H_

#include <PDM.h>
#include <Wire.h>


#ifdef __cplusplus
extern "C" {
#endif

//  Sensor register defines
#define LSM9DS1_SLAVE_ADDR   0x6B
#define LSM9DS1_REG_OUT_X_G  0x18
#define LSM9DS1_REG_OUT_X_XL 0x28
#define LSM9DS1_REG_FIFO_SRC 0x2f

#define LSM9DS1_MAG_ADDR 0x1E
#define LSM9DS1_STATUS_REG_M 0x27
#define LSM9DS1_OUT_X_L_M 0x28

#define MAG_BUFF_MAX (1*6)
#define PRESS_BUFF_MAX (1*4) // 4bytes
#define HUMIDITY_BUFF_MAX (1*2) //2bytes
#define TEMPERATURE_BUFF_MAX (1*2) //2bytes
#define PROXIMITY_BUFF_MAX (1*1)
#define LIGHT_BUFF_MAX (4*2)
#define MICROPHONE_BUFF_MAX (160*2) //16khz


tQxStatus QxOS_InitializeBSP();

void QxOS_ClassifyBTPrint(const char *format, ...);
tQxStatus QxBTHal_Initialize();

// Sensor I2C read/write interfaces
tQxStatus Sensor_I2CReadReg(uint8_t slave_addr, uint8_t reg, uint8_t *data,  uint16_t len);

tQxStatus lsm9ds1_acc_init(struct QxSensorDevice_t *dev);
tQxStatus lsm9ds1_acc_enable(struct QxSensorDevice_t *dev, float fs, float odr, BOOL fifo);

tQxStatus lsm9ds1_gyro_init(struct QxSensorDevice_t *dev);
tQxStatus lsm9ds1_gyro_enable(struct QxSensorDevice_t *dev, float fs, float odr, BOOL fifo);

tQxStatus mp34dt05_microphone_init(struct QxSensorDevice_t *dev);
tQxStatus mp34dt05_microphone_enable(struct QxSensorDevice_t *dev, float fs, float odr, BOOL fifo);
tQxStatus QxAudioHal_GetPCMBuf(int16_t *pBuf, uint32_t numOfSamples);

tQxStatus lsm9ds1_mag_init(struct QxSensorDevice_t *dev);
tQxStatus lsm9ds1_mag_enable(struct QxSensorDevice_t *dev, float fs, float odr, BOOL fifo);

#ifdef __cplusplus
}
#endif

#endif

