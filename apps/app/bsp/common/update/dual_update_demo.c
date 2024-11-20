
#include "board_config.h"
#include "includes.h"
#include "asm/power_interface.h"
#include "app_config.h"
#include "dual_bank_updata_api.h"
#include "my_malloc.h"
#include "uart_v2.h"
#include "app_main.h"
#include "msg.h"
#include "flash_init.h"

#define LOG_TAG         "[DUAL_OTA]"
#include "log.h"

#if (CONFIG_DOUBLE_BANK_ENABLE && CONFIG_DB_UPDATE_DATA_GENERATE_EN && CONFIG_ONLY_GRENERATE_ALIGN_4K_CODE)

/*******************************************************************************
 * #define CONFIG_DOUBLE_BANK_ENABLE            1
 * #define CONFIG_FLASH_SIZE                    FLASH_SIZE_512K //配置FLASH大小
 * #define CONFIG_DB_UPDATE_DATA_GENERATE_EN    1
 * #define CONFIG_ONLY_GRENERATE_ALIGN_4K_CODE  1
 *
 * 检查isd_config.ini文件
 * BR22_TWS_DB = YES;         // dual bank flash framework enable
 * FLASH_SIZE = 0x100000;     // flash_size cfg
 * BR22_TWS_VERSION = 0;      // default fw version
 * DB_UPDATE_DATA = YES;      // generate db_update_data.bin
 * FORCE_4K_ALIGN = YES;      // force aligin with 4k bytes
 * SPECIAL_OPT = 0;           // only generate one flash.bin
 *
 * const int support_dual_bank_update_en = 1;
 * const int support_vm_data_keep = 1;
 *******************************************************************************/

// #define MAX_PACKET_LEN   (512 * 4)  //最大包长，至少512，最好4K，需要自行组包
// #define MAX_PACKET_LEN   (512 * 1)  //最大包长，至少512，最好4K，需要自行组包
#define MAX_PACKET_LEN   (256 * 1)  //最大包长512
//0:CRC-16/XMODEM Standard 杰理默认的
//1:客户自行选择crc16的算法类型，重定义 dual_ota_crc_calc()
#define CRC_TYPE               0

enum {
    DUAL_OTA_NOTIFY_SUCCESS = 0,
    DUAL_OTA_NOTIFY_FAIL,
    DUAL_OTA_NOTIFY_CONTINUE,
};

typedef enum {
    DUAL_OTA_STEP_START = 0,
    DUAL_OTA_STEP_WRITE,
    DUAL_OTA_STEP_VERIFY,
} DUAL_OTA_STEP;

typedef struct {
    u32 file_size;     // db_update_data.bin
    u32 file_crc;      // CRC_TYPE 的对应值
    u32 max_pkt_len;   // MAX_PACKET_LEN
} dual_ota_info;

u8 *bin_buf;

NOT_KEEP_RAM
u8 db_update_packet[300];

static dual_ota_info ota_info;
static DUAL_OTA_STEP dual_ota_step = DUAL_OTA_STEP_START;
static int dual_ota_written_size = 0;
static u8 dual_ota_idle = 1;  // 置 1 可进睡眠, 置 0 不可进睡眠
int dual_ota_notify_msg_to_app(u32 msg);

__attribute__((weak))
u16 dual_ota_crc_calc(u8 *buf, u32 len, u16 init)
{
    log_debug("%s weak func", __func__);
    //CRC16-XMODEM
    u16 poly = 0x1021;
    u16 crc = init;
    while (len--) {
        u16 data = *buf++;
        crc ^= data << 8; //逆序
        for (u8 i = 0; i < 8; ++i) {
            crc = crc & 0x8000 ? (crc << 1) ^ poly : crc << 1; //逆序
        }
    }
    return crc;
}

static u32 read_32(u8 *p)
{
    return ((u32)(p[3]) << 24) | ((u32)(p[2]) << 16) | ((u32)(p[1]) << 8) | (p[0]);
}

static void dual_ota_cpu_reset(void *priv)
{
    log_info("%s", __func__);
    cpu_reset();
}

static void dual_ota_fail_exit(void)
{
    log_info("%s[step:%d]", __func__, dual_ota_step);
    if (bin_buf) {
        my_free(bin_buf);
        bin_buf = NULL;
    }
    memset(&ota_info, 0x00, sizeof(ota_info));
    dual_ota_step = DUAL_OTA_STEP_START;
    dual_ota_written_size = 0;
    dual_ota_idle = 1;
    dual_bank_passive_update_exit(NULL);
    /* sys_timeout_add(NULL, dual_ota_cpu_reset, 2000); */
    /* 通知上位机升级失败，退出升级 */
    dual_ota_notify_msg_to_app(DUAL_OTA_NOTIFY_FAIL);
}

static void dual_ota_success_reset(void)
{
    log_info("%s", __func__);
    sys_timeout_add(NULL, dual_ota_cpu_reset, 2000);
    /* 通知上位机升级完整，准备复位 */
    dual_ota_notify_msg_to_app(DUAL_OTA_NOTIFY_SUCCESS);
}

static int dual_ota_boot_info_cb(int err)
{
    dual_bank_passive_update_exit(NULL);
    /* 开启flash写保护 */
    flash_code_set_protect();
    /* 允许进行低功耗 */
    sleep_run_check_enalbe(1);

    if (err == 0) {
        dual_ota_success_reset();
    } else {
        dual_ota_fail_exit();
    }
    return 0;
}

static int dual_ota_verify_result(int res)            //1:success 0:fail
{
    log_info("%s[res:%d %s]", __func__, res, res ? "success" : "fail");
    if (res) {
        /* 校验成功 */
        dual_bank_update_burn_boot_info(dual_ota_boot_info_cb);
    } else {
        /* 校验失败 */
        dual_ota_fail_exit();
    }
    return 0;
}

static void dual_ota_crc_init_cb(void)
{
    log_info("%s", __func__);
}

static u32 dual_ota_crc_calc_cb(u32 init_crc, const void *data, u32 len)
{
    u32 crc16 = 0;
    log_info("&data = 0x%x", data);
    put_buf((u8 *)data, len);
    crc16 = dual_ota_crc_calc((u8 *)data, len, init_crc);
    log_debug("%s[init_crc:0x%x len:%d crc:0x%x]", __func__, init_crc, len, crc16);
    return crc16;
}

static int dual_ota_update_verify(void)
{
    log_info("%s[file_crc:0x%x]", __func__, ota_info.file_crc);
    if (ota_info.file_crc) {
        if (CRC_TYPE) {
            dual_bank_update_verify(dual_ota_crc_init_cb, dual_ota_crc_calc_cb,
                                    dual_ota_verify_result);
        } else {
            dual_bank_update_verify(NULL, NULL, dual_ota_verify_result);
        }
    } else {
        int ret = dual_bank_update_verify_without_crc();
        log_info("%s[verify_without_crc ret:%d]", __func__, ret);
        dual_ota_verify_result(ret ? 0 : 1);
    }
    return 0;
}

static int dual_ota_write_cb(void *priv)
{
    int ret = (int)priv;
    if (ret) {
        log_info("%s -> fail", __func__);
        dual_ota_fail_exit();
        return -1;
    } else {
        log_info("%s -> success", __func__);
        /* 通知上位机发送下一组数据 */
        dual_ota_notify_msg_to_app(DUAL_OTA_NOTIFY_CONTINUE);
        return 0;
    }
}

static int dual_ota_update_init(u32 file_size, u32 file_crc, u32 pkt_len)
{
    log_info("%s[size:%d crc:0x%x ptk_len:%d]", __func__, file_size, file_crc, pkt_len);
    if (bin_buf) {
        return -1;
    }
    bin_buf = my_malloc(MAX_PACKET_LEN, 0);
    if (bin_buf == NULL) {
        return -2;
    }
    if ((u32)bin_buf % 4) { //4byte对齐
        ASSERT(0, "%s: bin_buf addr unaligned !!!!", __func__);
    }
    memset(bin_buf, 0x00, MAX_PACKET_LEN);
    int ret = 0;
    ret = dual_bank_passive_update_init(file_crc, file_size, pkt_len, NULL);
    if (ret == 0) {
        ret = dual_bank_update_allow_check(file_size);
        if (ret == 0) {
            dual_ota_idle = 0;
            dual_ota_written_size = 0;
            ota_info.file_size = file_size;
            ota_info.file_crc  = file_crc;
            ota_info.max_pkt_len = pkt_len;
            /* 可以升级，需要通知上位机准备发包 */
            dual_ota_notify_msg_to_app(DUAL_OTA_NOTIFY_CONTINUE);
        }
    }
    log_debug("%s[ret:%d]", __func__, ret);
    if (ret) {
        dual_ota_fail_exit();  /* 无法升级 */
        return -3;
    }
    return ret;
}

static int dual_ota_update_deal(u8 step, u8 *buf, u32 len) //升级流程处理
{
    int ret = 0;
    log_info("%s[step:%d]", __func__, step);
    switch (step) {
    case DUAL_OTA_STEP_START:
        /* u32 *data = (u32 *)buf; //buf 地址需要aligned(4) */
        u32 file_size = buf[0];
        u32 file_crc  = buf[1];
        /* 禁止进行低功耗 */
        sleep_run_check_enalbe(0);
        /* 解锁flash写保护 */
        flash_code_set_unprotect();
        ret = dual_ota_update_init(file_size, file_crc, MAX_PACKET_LEN);
        break;

    case DUAL_OTA_STEP_WRITE:
        dual_bank_update_write(buf, len, dual_ota_write_cb);
        dual_ota_written_size += len;
        break;

    case DUAL_OTA_STEP_VERIFY:
        dual_ota_update_verify();
        break;
    }
    return ret;
}

/********************************************************************************
 * 跟上位机通信接口
 * 自行按照协议，自行修改，这个写法只是个demo
 * 数据来源：串口、USB、BLE等
 * 数据包很小，需要缓存成一包大包：MAX_PACKET_LEN，才能调用dual_bank_update_write
 * MAX_PACKET_LEN 定义成 小包的整数倍 ： MAX_PACKET_LEN == len * n
 * 必须在低优先级任务执行，不能在中断函数、蓝牙回调函数执行
 * 参考：https://gitee.com/Jieli-Tech/fw-AW31N_BLE_SDK/issues/IAPXPS
 *********************************************************************************/
int dual_ota_app_data_deal(u32 msg, u8 *buf, u32 len)
{
    if ((buf == NULL) || (len == 0)) {
        log_error("%s para is error", __func__);
        return -1;
    }
    log_info("%s[msg:%d len:%d]", __func__, msg, len);
    /* log_debug_hexdump(buf, len); */
    int ret = 0;
    static u32 size = 0;
    if (dual_ota_step == DUAL_OTA_STEP_WRITE) {
        if ((size < ota_info.max_pkt_len) && (size + len <= ota_info.max_pkt_len)) {
            memcpy(bin_buf + size, buf, len);
            size += len;
        } else {
            log_error("%s size + len over MAX_PACKET_LEN", __func__);
        }
        if ((size == ota_info.max_pkt_len)
            || (size + dual_ota_written_size >= ota_info.file_size)) {
            if (size + dual_ota_written_size >= ota_info.file_size) {
                dual_ota_step = DUAL_OTA_STEP_VERIFY;
            }
            dual_ota_update_deal(DUAL_OTA_STEP_WRITE, bin_buf, size);
            size = 0;
        }
    } else {
        if ((buf[0] == 0x55) && (buf[1] == 0xAA)) {
            u32 data[2];
            data[0] = read_32(buf + 2); //file_size
            data[1] = read_32(buf + 6); //file_crc
            ret = dual_ota_update_deal(DUAL_OTA_STEP_START, (u8 *)data, sizeof(data));
            if (ret == 0) {
                dual_ota_step = DUAL_OTA_STEP_WRITE;
            }
        }
        if ((buf[0] == 0xAA) && (buf[1] == 0x55)) {
            dual_ota_update_deal(DUAL_OTA_STEP_VERIFY, NULL, 0);
        }
    }
    return ret;
}

int dual_ota_notify_msg_to_app(u32 msg)
{
    int ret = 0;
    char *str = NULL;
    log_info("%s[msg:%d]", __func__, msg);
    switch (msg) {
    case DUAL_OTA_NOTIFY_SUCCESS:  //成功
        str = "success\r\n";
        break;

    case DUAL_OTA_NOTIFY_FAIL:     //失败
        str = "fail\r\n";
        break;

    case DUAL_OTA_NOTIFY_CONTINUE: //上一组数据处理成功，继续下一组
        str = "continue\r\n";
        break;
    }
    // 返回升级信息，eg:串口返回
#if TCFG_COMMON_UART_ENABLE
    uart_send_bytes(COMMON_UART_INDEX, (u8 *)str, strlen(str));
#endif
    return ret;
}

void app_db_updata_event_to_user(u8 event, u32 value)
{
    struct sys_event *e = event_pool_alloc();
    if (e == NULL) {
        log_info("Memory allocation failed for sys_event");
        return;
    }
    e->type = SYS_DEVICE_EVENT;
    e->arg  = (void *)DEVICE_EVENT_FROM_DUAL_UPDATE_UART;
    e->u.dev.event = event;
    e->u.dev.value = value;
    main_application_operation_event(NULL, e);
}

static u8 dual_ota_idle_query(void)
{
    if (dual_ota_idle) {
        return 1;
    } else {
        return 0;
    }
}

REGISTER_LP_TARGET(dual_ota_lp_target) = {
    .name = "dual_ota_lp",
    .is_idle = dual_ota_idle_query,
};

#endif



