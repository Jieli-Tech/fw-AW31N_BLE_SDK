#ifndef __RCSP_MSG_H__
#define __RCSP_MSG_H__

#include "typedef.h"

typedef enum __RCSP_MSG {
    // RCSP_MSG_UPDATE_EQ = 0,
    // RCSP_MSG_SET_FMTX_POINT,
    // RCSP_MSG_BS_END,
    // RCSP_MSG_BT_SCAN,
    RCSP_MSG_NORMAL_NORMAL_PROCESS,
    // RCSP_MSG_END,
    // 下面的消息从RCSP_MSG_T合并过来
    // MSG_JL_GET_DEV_UPDATE_FILE_INFO_OFFSET = RCSP_MSG_END
    MSG_JL_GET_DEV_UPDATE_FILE_INFO_OFFSET,
    MSG_JL_INQUIRE_DEVEICE_IF_CAN_UPDATE,
    MSG_JL_LOADER_DOWNLOAD_START,
    MSG_JL_UPDATE_START,
    MSG_JL_ENTER_UPDATE_MODE,
    MSG_JL_DEV_DISCONNECT,
    MSG_JL_BLE_UPDATE_START,
    MSG_JL_SPP_UPDATE_START,
} RCSP_MSG;

int rcsp_msg_post(u8 msg, int argc, ...);
void rcsp_msg_init(void);
int JL_rcsp_event_handler(RCSP_MSG msg, int argc, int *argv);
#endif
