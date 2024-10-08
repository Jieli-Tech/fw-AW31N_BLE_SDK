#include "includes.h"
#include "app_config.h"
#include "btcontroller_config.h"
/* #include "bt_common.h" */
//#include "avctp_user.h"

#define LOG_TAG     "[BT-CFG]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
/* #include "debug.h" */
#include "log.h"

/*注意hid_conn_depend_on_dev_company置2之后，默认不断开HID连接 */
const u8 hid_conn_depend_on_dev_company = 2;
const u8 sdp_get_remote_pnp_info = 0;

const u8 a2dp_mutual_support = 0;
const u8 more_addr_reconnect_support = 0;
const u8 more_hfp_cmd_support = 0;
const u8 more_avctp_cmd_support = 0;
const u8 hci_inquiry_support = 0;
const u8 btstack_emitter_support  = 0;  /*定义用于优化代码编译*/
const u8 adt_profile_support = 0;
const u8 pbg_support_enable = 0;

