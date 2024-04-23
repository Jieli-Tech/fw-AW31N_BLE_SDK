
#include "app_config.h"
#include "includes.h"
#include "app_config.h"
/* #include "update_loader_download.h" */

////////////////////////////update control///////////////////////////////////////////
const int CONFIG_UPDATE_STORAGE_DEV_EN = 0;
const int CONFIG_UPDATE_TESTBOX_UART_EN = 1;
const int CONFIG_UPDATE_APP_OTA_EN = 0;
const int CONFIG_UPDATE_TESTBOX_BLE_EN = 1;

//是否采用双备份升级方案:0-单备份;1-双备份
#if CONFIG_DOUBLE_BANK_ENABLE
const int support_dual_bank_update_en = 1;
#else
const int support_dual_bank_update_en = 0;
#endif  //CONFIG_DOUBLE_BANK_ENABLE

//TODO
/* #if OTA_TWS_SAME_TIME_NEW       //使用新的同步升级流程 */
/* const int support_ota_tws_same_time_new =  1; */
/* #else */
const int support_ota_tws_same_time_new =  0;
/* #endif */
//是否支持升级之后保留vm数据
const int support_vm_data_keep = 0;

//是否支持外挂flash升级,需要打开Board.h中的TCFG_NOR_FS_ENABLE
const int support_norflash_update_en  = 0;

//支持从外挂flash读取ufw文件升级使能
const int support_norflash_ufw_update_en = 0;

/////////////////////////////////////////////////////////////////////////////////////

const char log_tag_const_i_UPDATE AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1);
const char log_tag_const_d_UPDATE AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1);
const char log_tag_const_e_UPDATE AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1);
const char log_tag_const_c_UPDATE AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1);


#if 0//NFIG_APP_OTA_ENABLE

#ifdef CONFIG_256K_FLASH
const int config_update_mode = UPDATE_UART_EN;
#else
const int config_update_mode = UPDATE_BT_LMP_EN | UPDATE_BLE_TEST_EN | UPDATE_APP_EN | UPDATE_UART_EN;
#endif

//是否采用双备份升级方案:0-单备份;1-双备份
#if CONFIG_DOUBLE_BANK_ENABLE
const int support_dual_bank_update_en = 1;
#else
const int support_dual_bank_update_en = 0;
#endif

//是否支持外挂flash升级,需要打开Board.h中的TCFG_NOR_FS_ENABLE
const int support_norflash_update_en  = 0;

//支持从外挂flash读取ufw文件升级使能
const int support_norflash_ufw_update_en = 0;

#if OTA_TWS_SAME_TIME_NEW       //使用新的同步升级流程
const int support_ota_tws_same_time_new =  1;
#else
const int support_ota_tws_same_time_new =  0;
#endif

//是否支持升级之后保留vm数据
const int support_vm_data_keep = 1;

const char log_tag_const_v_UPDATE AT(.LOG_TAG_CONST) = LIB_DEBUG &  FALSE;
const char log_tag_const_i_UPDATE AT(.LOG_TAG_CONST) = LIB_DEBUG &  FALSE;
const char log_tag_const_d_UPDATE AT(.LOG_TAG_CONST) = LIB_DEBUG &  FALSE;
const char log_tag_const_w_UPDATE AT(.LOG_TAG_CONST) = LIB_DEBUG &  TRUE;
const char log_tag_const_e_UPDATE AT(.LOG_TAG_CONST) = LIB_DEBUG &  TRUE;
#endif

