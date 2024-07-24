#ifndef __JL_CH_LOADER_DOWNLOAD_H__
#define __JL_CH_LOADER_DOWNLOAD_H__

#include "typedef.h"
#include "rcsp_msg.h"

#define JL_OPCODE_GET_DEVICE_UPDATE_FILE_INFO_OFFSET    0xe1
#define JL_OPCODE_INQUIRE_DEVICE_IF_CAN_UPDATE          0xe2
#define JL_OPCODE_ENTER_UPDATE_MODE                     0xe3
#define JL_OPCODE_EXIT_UPDATE_MODE                      0xe4
#define JL_OPCODE_SEND_FW_UPDATE_BLOCK                  0xe5
#define JL_OPCODE_GET_DEVICE_REFRESH_FW_STATUS          0xe6
#define JL_OPCODE_SET_DEVICE_REBOOT                     0xe7
#define JL_OPCODE_NOTIFY_UPDATE_CONENT_SIZE             0xe8

#if 0
enum RCSP_MSG_T {
    MSG_JL_GET_DEV_UPDATE_FILE_INFO_OFFSET = RCSP_MSG_END,
    MSG_JL_INQUIRE_DEVEICE_IF_CAN_UPDATE,
    MSG_JL_LOADER_DOWNLOAD_START,
    MSG_JL_UPDATE_START,
    MSG_JL_ENTER_UPDATE_MODE,
    MSG_JL_DEV_DISCONNECT,
    MSG_JL_BLE_UPDATE_START,
    MSG_JL_SPP_UPDATE_START,
};
#endif

void JL_rcsp_update_cmd_receive_resp(void *priv, u8 OpCode, u8 status, u8 *data, u16 len);
void JL_rcsp_update_cmd_resp(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len);
void set_curr_update_type(u8 type);
u8 get_jl_update_flag(void);
void set_jl_update_flag(u8 flag);
void JL_rcsp_msg_deal(RCSP_MSG msg, int argc, int *argv);
void app_update_msg_handle(int msg);
void rcsp_update_jump_to_loader_handle(void *priv);
void app_ota_update_handle(void);

#endif
