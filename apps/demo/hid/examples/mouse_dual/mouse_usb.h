#ifndef __MOUSE__DUAL
#define __MOUSE__DUAL
#include "typedef.h"

#define     MOUSE_USB_PACKET_LEN      6

void mouse_set_soft_reset(void *priv);
void mouse_usb_start();
void mouse_usb_data_send(uint8_t report_id, uint8_t *packet, uint16_t size);
void mouse_usb_set_go_updata(void);
extern void go_mask_usb_updata();
#endif
