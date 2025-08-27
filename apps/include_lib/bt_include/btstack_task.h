#ifndef BTSTACK_TASK_H
#define BTSTACK_TASK_H

typedef enum {
    SET_DUT_MODE = 1,
    SET_DUT_API_MODE
} dut_set_mode;

int btstack_init();
int btstack_exit();
void ble_bqb_test_thread_init(void);
void df_aoa_broadcast_test_open(void);
void df_aoa_tx_connected_test_open(void);
void aes128_test();
void aes_ccm_test(void);

/*!
 *  \brief      设置DUT模式
 *  \param      [in] dut_set_mode
                    SET_DUT_MODE为DUT测试模式,默认上电进初始化
                    SET_DUT_API_MODE为DUT 测试api 模式, 需要自行调用api测试,见ble_test_api.c
 *  \return     void
 */
//蓝牙初始化前调用
void user_sele_dut_mode(dut_set_mode set);

//蓝牙初始化前后,运行后也可调用i
//调用前先关闭蓝牙ADV/SCAN/INIT/CONNECT等状态操作,bt处在idle状态
void user_enter_dut_mode(dut_set_mode set);

int get_dut_mode(void);
#endif
