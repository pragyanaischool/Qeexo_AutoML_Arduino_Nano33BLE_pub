/**
  ******************************************************************************
  * @file    QxOS.h
  * @author  Qeexo Kernel Development team
  * @version V1.0.0
  * @date    30-Sep-2020
  * @brief   Header of OS Api definition which Qeexo AutoML package requires.
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


#ifndef MIDDLEWARES_QEEXO_INCLUDE_QXOS_H_
#define MIDDLEWARES_QEEXO_INCLUDE_QXOS_H_

#include "QxTypeDefs.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Thread structure for Qeexo AutoML framework.
*/
typedef struct {
	char *name; /*!< The name of thread */
	void *pData; /*!< The pointer maintaining user data */
} tQxThread;

/**
 * Mutex structure for Qeexo AutoML framework.
*/
typedef struct {
	char *name; /*!< The name of mutex */
	BOOL isLocked; /*!< The variable indicates if the mutex is locked. */
} tQxMutex;

/**
 * Thread priority defintions. 
 * @note it is compatible with osPriority of CMSIS FreeRTOS.
*/
typedef enum {
	QxPriorityIdle = -3,         /*!< priority: Idle (lowest) */
	QxPriorityLow = -2,          /*!< priority: Low */
	QxPriorityBelowNormal = -1,  /*!< priority: Below normal */
	QxPriorityNormal = 0,        /*!< priority: Normal (default) */
	QxPriorityAboveNormal = 1,   /*!< priority: Above normal */
	QxPriorityHigh = 2,          /*!< priority: High */
	QxPriorityRealtime = 3,      /*!< priority: Realtime (highest) */
	QxPriorityError = 0x8F,      /*!< system cannot determine priority or thread has illegal priority */
} tQxPriority;


/**
 * Device definition of streaming devices 
 * 
*/
typedef enum {
	QxStreamDeviceNone = 0,      /*!< No device */
	QxStreamDeviceUSB,           /*!< USB device. */
	QxStreamDeviceBT,            /*!< Bluetooth */
	QxStreamDeviceSD,            /*!< SDCard */
	QxStreamDeviceWifi,          /*!< Wify */
	QxStreamDeviceArbitrator,    /*!< Vendor defined device */
	QxStreamDeviceMax
} tQxStreamDevice;

/** 
 * Callback prototype of thread
 *
 * @param *userdata The user data pointer, which is passed by QxOS_CreateThread(), will return for user's thread function .
 */
typedef void (*tQxThreadFunc) (const void *userdata);


/** 
 * Callback function defintion of data input from stream device. 
 *
 * @param[in] device_type The device type transferring data.
 * @param[in] data The data pointer from incomming device. 
 * @param[in] length The data size from incomming device.
 */
typedef void (*tQxStreamDataInCallback) (tQxStreamDevice device_type, const uint8_t* data, uint16_t length, void *userdata);


/**    
 * @brief Initialize BSPs for tareget device including clocks, SPI, I2C, and all about system configurations.    
 * @return tQxStatus : status of initializing BSP.
 */
tQxStatus QxOS_InitializeBSP(void);

/**    
 * @brief Create a new thread.    
 * @param[in] *name the name of a created thread.   
 * @param[in] func the main function of thread.     
 * @param[in] prio the priority of this thread. 
 * @param[in] stacksz the stack size of this thread.
 * @param[in] userdata the memory pointer. It will be passed into the thread main function.  
 * @return tQxThread : Memory pointer of a creatted thread. 
 */
tQxThread* QxOS_CreateThread(const char* name, tQxThreadFunc func, tQxPriority prio, uint32_t stacksz, void* userdata);

/**    
 * @brief Create a mutex.    
 * @param[in] *name The name of mutex.     
 * @return tQxMutex : The memory pointer of the created mutex.
 * @note This function is matched with the use of osThreadCreate() in CMSIS and the sample implementation codes give how to fill this HAL code with osThreadCreate().
 */
tQxMutex* QxOS_CreateMutex(const char* name);

/**    
 * @brief Lock the cretical section with the mutex.    
 * @param[in] *mutex The memory pointer of mutex.
 * @return tQxStatus : Status of mutex lock.
 * @note In side of the sample codes, there is some code how to write with osMutexWait() of CMSIS.
 */
tQxStatus QxOS_LockMutex(tQxMutex* mutex);

/**
 * @brief Lock the critical section with the mutex.
 * @param[in] *mutex The memory pointer of mutex.
 * @param[in] millisec Timeout value.
 * @return tQxStatus : Status of mutex lock.
 * @note In side of the sample codes, there is some code how to write with osMutexWait() of CMSIS.
 */
tQxStatus QxOS_LockMutex_Wait(tQxMutex* mutex, uint32_t millisec);

/**    
 * @brief Unlock the cretical section with the mutex.    
 * @param[in] *mutex The memory pointer of mutex.
 * @return tQxStatus : Status of mutex unlock.
 * @note In side of the sample codes, there is some code how to write with osMutexRelease() of CMSIS.
 */
tQxStatus QxOS_UnLockMutex(tQxMutex* mutex);

/**    
 * @brief Start OS Kernel for QxSensor framework.    
 * @return tQxStatus : Status of starting OS kernel.
 * @note This function is matched with osKernelStart() of CMSIS and you can see this in the samples codes.
 */
tQxStatus QxOS_StartKernel(void);

/**
 * @brief Get millisecond delay.    
 * @param[in] msec Millisecond delay value.
 * @return tQxStatus : Status of getting OS delay.
 * @note This function is matched with osDelay() of CMSIS.
 */
tQxStatus QxOS_Delay(uint32_t msec);

/**    
 * @brief Get tick counter in millisecond unit.
 * @return uint32_t : Millisecond tick counter.
 * @note This function is matched with osKernelSysTick() of CMSIS
 */
uint32_t QxOS_GetTick();

/**    
 * @brief Print log out for the debug purpose of AutoML package.    
 * @param[in] *format The format of log message.    
 * @param[in] ... The relevant variables or values for format to print. 
 */
void QxOS_DebugPrint(const char *format, ...);

/**    
 * @brief Print log out for the classify result purpose of AutoML package.    
 * @param[in] *format The format of log message.    
 * @param[in] ... The relevant variables or values for format to print. 
*/
void QxOS_ClassifyPrint(const char *format, ...);

/**    
 * @brief Print log out over bluetooth for the classify result purpose of AutoML package.    
 * @param[in] *format The format of log message.    
 * @param[in] ... The relevant variables or values for format to print. 
*/
void QxOS_ClassifyBTPrint(const char *format, ...);

/**    
 * @brief Transfer data out through the device_type.
 * @param[in] device_type The device type transferring data.
 * @param[in] *data The data pointer of memory to transfer.
 * @param[in] data_len The data length to transfer.
 * @param[in] timeout The millisecond unit of timeout.
 * @return int : Return how many bytes has been transferred.
 */
int QxOS_StreamDataOut(tQxStreamDevice device_type, void* data, uint16_t data_len, uint32_t timeout);

/**    
 * @brief Flush out if data is being buffered on the transferring device or device driver.    
 * @param[in] device_type The device type transferring data.
 * @param[in] timeout The millisecond unit of timeout.
 * @return int : Return how many bytes has been transferred.
 */
int QxOS_FlushDataOut(tQxStreamDevice device_type, uint32_t timeout);

/**    
 * @brief Register callack function of stream data input    
 * @param[in] callback The call function point.
 * @return tQxStatus : The status of registering callback function.
 */
tQxStatus QxOS_RegisterStreamDataInCallback(tQxStreamDataInCallback callback, void *userdata);

/**    
 * @brief Notify stream data input to callbacks.
 * @param[in] device_type stream device type.
 * @param[in] *data the data pointer
 * @param[in] data_len the lenth of data.
 * @return tQxStatus : The status of registering callback function.
 */
tQxStatus QxOS_NotifyStreamDataIn(tQxStreamDevice device_type, const uint8_t* data, uint16_t length);

/**    
 * @brief Write data to sensor register.
 * @param[in] slave_addr slave address of device for I2C and it can be a device unique id for SPI.
 * @param[in] reg The register address to store.
 * @param[in] *data Data array to store into sensor register.
 * @param[in] data_len The lenth of data.
 * @return tQxStatus : Status of writing data into sensor.
 */
tQxStatus QxOS_SensorWriteReg(uint8_t slave_addr, uint8_t reg, uint8_t *data,  uint16_t len);

/**    
 * @brief Write a single byte of data to sensor register.
 * @param[in] slave_addr Slave address of device for I2C and it can be a device unique id for SPI.
 * @param[in] reg The register address to store.
 * @param[in] data One byte data to store into sensor register.
 * @return tQxStatus : Status of writing data into sensor.
 */
tQxStatus QxOS_SensorWriteRegSingle(uint8_t slave_addr, uint8_t reg, uint8_t data);

/**    
 * @brief Read data from sensor register.
 * @param[in] slave_addr Slave address of device for I2C and it can be a device unique id for SPI.
 * @param[in] reg The register address to read.
 * @param[in] *data Buffer to read data from sensor register.
 * @param[in] data_len The lenth of data.
 * @return tQxStatus : Status of reading data from sensor.
 */
tQxStatus QxOS_SensorReadReg(uint8_t slave_addr, uint8_t reg, uint8_t *data,  uint16_t len);



/**    
 * @brief Handle the case of assert.
 * @param[in] cond condition; .
 */
void QxOS_Assert(BOOL cond);

#ifdef __cplusplus
}
#endif

#endif //MIDDLEWARES_QEEXO_INCLUDE_QXOS_H_
