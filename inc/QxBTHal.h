/**
  ******************************************************************************
  * @file    QxBTHal.h
  * @author  Qeexo Kernel Development team
  * @version V1.0.0
  * @date    30-Sep-2019
  * @brief   Header of BT Api definition which Qeexo AutoML package requires.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 Qeexo Co.
  * All rights reserved.</center></h2>
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



#ifndef MIDDLEWARES_QEEXO_INCLUDE_QXBTHAL_H_
#define MIDDLEWARES_QEEXO_INCLUDE_QXBTHAL_H_

#include "QxTypeDefs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define   QX_DC_LABEL_NAME_LEN 20

typedef struct {
    uint32_t    dc_delay_ms;
    uint32_t    dc_duration;
    uint32_t    len_label;
    char         label[QX_DC_LABEL_NAME_LEN + 1];
} s_qx_bt_ctrl;
/*Defined as BLE_spec.pdf*/
typedef enum {
    APK_CMD_STANDBY = 0x0,
    APK_CMD_SEND_KEY = 0x1,
    APK_CMD_START_DC = 0x2,
    APK_CMD_START_TRAINING = 0x3,
    APK_CMD_START_INFERENCE = 0x4,
    APK_CMD_STOP = 0x5,
    APK_CMD_CLEAR_ERR = 0x6,
    APK_CMD_DISCONNECT = 0xF,
} e_apk_cmd_bits_rx;
typedef enum {
    DEV_RPL_WAIT_KEY = 0x10,
    DEV_RPL_STANDBY = 0x11,
    DEV_RPL_DC_START = 0x12,
    DEV_RPL_DC_INST = 0x13,
    DEV_RPL_TRAINING = 0x14,
    DEV_RPL_INFERENCE = 0x15,
    DEV_RPL_DISCONNECTED = 0x1E,
    DEV_RPL_ERR = 0x1F,
} e_apk_cmd_bits_tx;
/**    
 * @brief Initialize BT device
 * @return tQxStatus : Status of initializing BT device.
 */
tQxStatus QxBTHal_Initialize();

/**    
 * @brief Transfer data out through BT.
 * @param[in] *buf the data pointer to transfer.
 * @param[in] length The data size to transfer.
 * @return int : Return how many bytes has been transferred.
 */
void QxBTHal_Write(char *buf, int length);

bool QxBTHal_Connected();

#ifdef __cplusplus
}
#endif

#endif /* MIDDLEWARES_QEEXO_INCLUDE_QXBTHAL_H_ */
