#include "app_config.h"
#include "cpu.h"
#include "usb_config.h"
#include "usb/device/descriptor.h"
#include "usb/device/usb_stack.h"
#include "usb/device/usb_suspend_resume.h"
#include "irq.h"
#include "gpio.h"
#include "clock.h"
#include "fusb_pll_trim.h"
#define LOG_TAG_CONST       USB
#define LOG_TAG             "[USB]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "log.h"


#define EP0_DMA_SIZE    (64+4)
#define HID_DMA_SIZE    (64+4)
#define AUDIO_DMA_SIZE  (SPK_FRAME_LEN + MIC_FRAME_LEN + 4 * 2)
#define MSD_DMA_SIZE    ((64+4)*2)

#define     MAX_EP_TX   5
#define     MAX_EP_RX   5
static usb_interrupt usb_interrupt_tx[USB_MAX_HW_NUM][MAX_EP_TX];// SEC(.usb_g_bss);
static usb_interrupt usb_interrupt_rx[USB_MAX_HW_NUM][MAX_EP_RX];// SEC(.usb_h_bss);
static usb_interrupt usb_sof_interrupt_tx[USB_MAX_HW_NUM][MAX_EP_TX];// SEC(.usb_g_bss);

NOT_KEEP_RAM
static u8 ep0_dma_buffer[EP0_DMA_SIZE]     __attribute__((aligned(4)));
NOT_KEEP_RAM
static u8 ep1_msd_dma_buffer[2][MSD_DMA_SIZE]  __attribute__((aligned(4)));
NOT_KEEP_RAM
static u8 ep2_hid_dma_buffer[HID_DMA_SIZE]  __attribute__((aligned(4)));
NOT_KEEP_RAM
static u8 ep3_spk_dma_buffer[AUDIO_DMA_SIZE]   __attribute__((aligned(4)));
NOT_KEEP_RAM
static u8 ep3_mic_dma_buffer[AUDIO_DMA_SIZE]   __attribute__((aligned(4)));

struct usb_config_var_t {
    u8 usb_setup_buffer[USB_SETUP_SIZE];
    struct usb_ep_addr_t usb_ep_addr;
    struct usb_setup_t usb_setup;
};

static struct usb_config_var_t *usb_config_var[USB_MAX_HW_NUM] = {NULL};

static struct usb_config_var_t _usb_config_var[USB_MAX_HW_NUM] SEC(.usb_config_var);

__attribute__((always_inline_when_const_args))
void *usb_get_ep_buffer(const usb_dev usb_id, u32 ep)
{
    u8 *ep_buffer = NULL;
    u32 _ep = ep & 0xf;
    if (ep & USB_DIR_IN) {
        switch (_ep) {
        case 0:
            ep_buffer = ep0_dma_buffer;
            break;
        case 1:
            ep_buffer = ep1_msd_dma_buffer[0];
            break;
        case 2:
            ep_buffer = ep2_hid_dma_buffer;
            break;
        case 3:
            ep_buffer = ep3_mic_dma_buffer;
            break;
        }
    } else {
        switch (_ep) {
        case 0:
            ep_buffer = ep0_dma_buffer;
            break;
        case 1:
            ep_buffer = ep1_msd_dma_buffer[1];
            break;
        case 2:
            ep_buffer = NULL;
            break;
        case 3:
            ep_buffer = ep3_spk_dma_buffer;
            break;
        }
    }
    return ep_buffer;
}

void usb_isr(const usb_dev usb_id)
{
    u32 intr_usb, intr_usbe;
    u32 intr_tx, intr_txe;
    u32 intr_rx, intr_rxe;

    __asm__ volatile("ssync");
    usb_read_intr(usb_id, &intr_usb, &intr_tx, &intr_rx);
    usb_read_intre(usb_id, &intr_usbe, &intr_txe, &intr_rxe);
    struct usb_device_t *usb_device = usb_id2device(usb_id);

    intr_usb &= intr_usbe;
    intr_tx &= intr_txe;
    intr_rx &= intr_rxe;

    if (intr_usb & INTRUSB_SUSPEND) {
        log_error("usb suspend\n");
        usb_suspend_interface(usb_device);
        usb_slave_suspend(usb_id);
    }
    if (intr_usb & INTRUSB_RESET_BABBLE) {
        log_error("usb reset\n");
        usb_reset_interface(usb_device);
        usb_slave_reset(usb_id);
    }
    if (intr_usb & INTRUSB_RESUME) {
        log_error("usb resume\n");
        usb_slave_resume(usb_id);
    }

    if (intr_tx & BIT(0)) {
        if (usb_interrupt_rx[usb_id][0]) {
            usb_interrupt_rx[usb_id][0](usb_device, 0);
        } else {
            usb_control_transfer(usb_device);
        }
    }

    for (int i = 1; i < MAX_EP_TX; i++) {
        if (intr_tx & BIT(i)) {
            if (usb_interrupt_tx[usb_id][i]) {
                usb_interrupt_tx[usb_id][i](usb_device, i);
            }
        }
    }

    for (int i = 1; i < MAX_EP_RX; i++) {
        if (intr_rx & BIT(i)) {
            if (usb_interrupt_rx[usb_id][i]) {
                usb_interrupt_rx[usb_id][i](usb_device, i);
            }
        }
    }
    __asm__ volatile("csync");
}
SET_INTERRUPT
void usb0_g_isr()
{
    usb_isr(0);
}
SET_INTERRUPT
void usb0_sof_isr()
{
    const usb_dev usb_id = 0;
    usb_sof_clr_pnd(usb_id);
    //debug代码,用于测试收到sof包,1000个sof包打印一次
    /* static u32 sof_count = 0; */
    /* if ((sof_count++ % 1000) == 0) { */
    /*     log_info("sof 1s isr frame:%d", usb_read_sofframe(usb_id)); */
    /* } */

    struct usb_device_t *usb_device = usb_id2device(usb_id);
    for (int i = 1; i < MAX_EP_RX; i++) {
        if (usb_sof_interrupt_tx[usb_id][i]) {
            usb_sof_interrupt_tx[usb_id][i](usb_device, i);
        }
    }
}

__attribute__((always_inline_when_const_args))
u32 usb_g_set_intr_hander(const usb_dev usb_id, u32 ep, usb_interrupt hander)
{
    if (ep & USB_DIR_IN) {
        usb_interrupt_tx[usb_id][ep & 0xf] = hander;
    } else {
        usb_interrupt_rx[usb_id][ep] = hander;
    }
    return 0;
}
__attribute__((always_inline_when_const_args))
u32 usb_g_set_sof_intr_hander(const usb_dev usb_id, u32 ep, usb_interrupt hander)
{

    struct usb_device_t *usb_device = usb_id2device(usb_id);
    //sof 中断里进行数据发送, 只考虑发数据
    if (ep & USB_DIR_IN) {
        usb_sof_interrupt_tx[usb_id][ep & 0xf] = hander;
        //允许重复注册sof中断
        usb_sof_isr_reg(usb_id, 5, 0);
        usb_sofie_enable(usb_id);
    }
    return 0;
}
u32 usb_g_sof_intr_hander_check(const usb_dev usb_id, u32 ep, usb_interrupt hander)
{
    u32 ret = 1;
    if (usb_sof_interrupt_tx[usb_id][ep & 0xf] == hander) {
        ret = 0;
    }
    return ret;
}
void usb_g_isr_reg(const usb_dev usb_id, u8 priority, u8 cpu_id)
{
    request_irq(IRQ_USB_CTRL_IDX, priority, usb0_g_isr, cpu_id);
}
void usb_sof_isr_reg(const usb_dev usb_id, u8 priority, u8 cpu_id)
{
    request_irq(IRQ_USB_SOF_IDX, priority, usb0_sof_isr, cpu_id);
}
u32 usb_device_config(const usb_dev usb_id)
{
    memset(usb_interrupt_rx[usb_id], 0, sizeof(usb_interrupt_rx[usb_id]));
    memset(usb_interrupt_tx[usb_id], 0, sizeof(usb_interrupt_tx[usb_id]));

    log_info("zalloc: usb_config_var[%d] = %x\n", usb_id, usb_config_var[usb_id]);
    if (!usb_config_var[usb_id]) {
        memset(&_usb_config_var, 0, sizeof(_usb_config_var));
        usb_config_var[usb_id] = &_usb_config_var[usb_id];
    }
    log_info("zalloc: usb_config_var[%d] = %x\n", usb_id, usb_config_var[usb_id]);

    usb_var_init(usb_id, &(usb_config_var[usb_id]->usb_ep_addr));
    usb_setup_init(usb_id, &(usb_config_var[usb_id]->usb_setup), usb_config_var[usb_id]->usb_setup_buffer);
    /* usb_device_set_desc(usb_id, usb_get_desc_config()); */
    struct usb_device_t *usb_device = usb_id2device(usb_id);
    usb_device->usb_g_set_intr_hander = usb_g_set_intr_hander;
    usb_slave_suspend_resume_init(usb_id);
    return 0;
}

u32 usb_release(const usb_dev usb_id)
{
    log_info("free zalloc: usb_config_var[%d] = %x\n", usb_id, usb_config_var[usb_id]);
    usb_var_init(usb_id, NULL);
    usb_setup_init(usb_id, NULL, NULL);

    usb_config_var[usb_id] = NULL;
    /* usb_device_set_desc(usb_id, NULL); */
    usb_slave_suspend_resume_deinit(usb_id);

    return 0;
}
