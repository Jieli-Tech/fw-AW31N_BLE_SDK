#ifndef  __USB_STACK_H__
#define  __USB_STACK_H__
#include "typedef.h"
#include "usb/usb.h"
#include "usb/ch9.h"
#include "usb/usb_phy.h"
#include "usb/otg.h"

#define      USB_ROOT2  0

#define     MAX_INTERFACE_NUM       5

#define     USB_SETUP_SIZE         (512)

#if 0
#define     USB_ATTACHED        BIT(0)
#define     USB_POWERED         BIT(1)
#define     USB_DEFAULT         BIT(2)
#define     USB_ADDRESS         BIT(3)
#define     USB_CONFIGURED      BIT(4)
#define     USB_SUSPENDED       BIT(5)
#else
enum {
    USB_ATTACHED,
    USB_POWERED,
    USB_DEFAULT,
    USB_ADDRESS,
    USB_CONFIGURED,
    USB_SUSPENDED
};
#endif

struct usb_device_t ;

typedef u32(*itf_hander)(struct usb_device_t *usb_device, struct usb_ctrlrequest *);
typedef void(*itf_reset_hander)(struct usb_device_t *, u32 itf);
typedef void(*usb_interrupt)(struct usb_device_t *, u32 ep);
typedef u32(*desc_config)(const usb_dev usb_id, u8 *ptr, u32 *cur_itf_num);

struct usb_device_t {
    u8 baddr;
    u8 bsetup_phase;          //ep0 setup状态机
    u16 wDataLength;    //ep0 setup data stage数据长度

    u8 *setup_buffer;   //本次传输的bufer地址
    u8 *setup_ptr;      //当前传输的位置
    u32(*setup_hook)(struct usb_device_t *, struct usb_ctrlrequest *);
    u32(*setup_recv)(struct usb_device_t *, struct usb_ctrlrequest *);
    u32(*usb_g_set_intr_hander)(const usb_dev usb_id, u32 ep, usb_interrupt hander);

    u8 bDeviceStates;
    u8 bDataOverFlag;    //ep0 0包标识
    u8 bRemoteWakup: 1;
    u8 res: 7;
    u8 wDeviceClass;    // 设备类

    u8 baddr_config: 1;
};

struct usb_device_descriptor_t {
    const u8 *device_descriptor;
    const u8 *language_str;
    const u8 *manufacture_str;
    const u8 *product_str;
    const u8 *iserialnumber_str;
    const u8 *config_desc;
};


struct usb_setup_t {
    struct usb_device_t usb_device;
    struct usb_ctrlrequest request;
    itf_hander interface_hander[MAX_INTERFACE_NUM];
    itf_reset_hander reset_hander[MAX_INTERFACE_NUM];
} __attribute__((aligned(4)));

const usb_dev usb_device2id(const struct usb_device_t *usb_device);
struct usb_device_t *usb_id2device(const usb_dev usb_id);
void usb_control_transfer(struct usb_device_t *usb_device);
void usb_device_set_class(struct usb_device_t *usb_device, u32 class_config);
u32 usb_g_set_intr_hander(const usb_dev usb_id, u32 ep, usb_interrupt hander);
u32 usb_set_interface_hander(const usb_dev usb_id, u32 itf_num, itf_hander hander);
void usb_add_desc_config(const usb_dev usb_id, u32 index, const desc_config desc);
const u8 *usb_get_config_desc();
void usb_device_set_desc(const usb_dev usb_id, const struct usb_device_descriptor_t *desc);
u32 usb_set_reset_hander(const usb_dev usb_id, u32 itf_num, itf_reset_hander hander);
void usb_reset_interface(struct usb_device_t *usb_device);
void usb_set_setup_recv(struct usb_device_t *usb_device, void *recv);
void usb_set_setup_hook(struct usb_device_t *usb_device, void *hook);
int usb_device_mode(const usb_dev usb_id, const u32 class);
//void usb_otg_sof_check_init(const usb_dev id);
void usb_setup_init(const usb_dev usb_id, void *ptr, u8 *setup_buffer);
u32 usb_setup_release(const usb_dev usb_id);
u8 *usb_set_data_payload(struct usb_device_t *usb_device, struct usb_ctrlrequest *req, const void *data, u32 len);
void usb_set_setup_phase(struct usb_device_t *usb_device, u8 setup_phase);
void dump_setup_request(const struct usb_ctrlrequest *request);
void user_setup_filter_install(struct usb_device_t *usb_device);
void usb_ep_enable(const usb_dev usb_id, u32 ep, u32 is_enable);
void *usb_get_setup_buffer(const struct usb_device_t *usb_device);
u32 usb_root2_testing();

extern void usb_start();
extern void usb_stop();
extern void usb_pause();

/* #define usb_add_desc_config(fn) \                                    */
/*     const desc_config usb_desc_config##fn sec(.usb.desc_config) = fn */

#endif  /*USB_STACK_H*/
