#ifndef  __USB_STD_CLASS_DEF_H__
#define  __USB_STD_CLASS_DEF_H__

#include "usb_common_def.h"

#define     USB_MALLOC_ENABLE           0
#define     USB_HOST_ASYNC              0
#define     USB_H_MALLOC_ENABLE         0

#define     USB_DEVICE_CLASS_CONFIG (SPEAKER_CLASS|MIC_CLASS|HID_CLASS|MASSSTORAGE_CLASS)

///////////MassStorage Class

#define     MSD_BULK_EP_OUT             1
#define     MSD_BULK_EP_IN              1


#define     MAXP_SIZE_BULKOUT           64
#define     MAXP_SIZE_BULKIN            64

#define     IAP_STR_INDEX               7

///////////HID class
#define     HID_EP_IN                   2

#define     HID_EP_OUT                  0  //不定义ep out

#define     MAXP_SIZE_HIDOUT            8
#define     MAXP_SIZE_HIDIN             8
/* #define     MAXP_SIZE_HIDOUT            64 */
/* #define     MAXP_SIZE_HIDIN             64 */



/////////////Audio Class
#define     UAC_ISO_INTERVAL            1
//speaker class
//suport 8K 11.025 12K 16K 22.050K 24K 32K 44.1K 48K 64K 88.2K 96K
#define     SPK_AUDIO_RATE              48000
#define     SPK_AUDIO_RES               24

#define     SPK_CHANNEL                 2
#define     SPK_FRAME_LEN               (((SPK_AUDIO_RATE) * SPK_AUDIO_RES / 8 * SPK_CHANNEL)/1000)

#define     SPK_PCM_Type                (SPK_AUDIO_RES >> 4)                // 0=8 ,1=16
#define     SPK_AUDIO_TYPE              (0x02 - SPK_PCM_Type)           // TYPE1_PCM16


#define     SPK_ISO_EP_OUT              3

#define     SPEAKER_STR_INDEX           0

#define     SPK_INPUT_TERMINAL_ID       1
#define     SPK_FEATURE_UNIT_ID         2
#define     SPK_OUTPUT_TERMINAL_ID      3

/////////////Microphone Class

#define     MIC_SamplingFrequency       1

#if MIC_SamplingFrequency   == 1
//suport 8K 11.025 12K 16K 22.050K 24K 32K 44.1K 48K
#define     MIC_AUDIO_RATE              48000

#else

#define     MIC_AUDIO_RATE              192000
#define     MIC_AUDIO_RATE_1            44100
#define     MIC_AUDIO_RATE_2            48000
#define     MIC_AUDIO_RATE_4            96000

#endif

#define     MIC_AUDIO_RES               16

#define     MIC_CHANNEL                 2

#define     MIC_FRAME_LEN               ((MIC_AUDIO_RATE * MIC_AUDIO_RES / 8 * MIC_CHANNEL)/1000)

#define     MIC_PCM_TYPE                (MIC_AUDIO_RES >> 4)                // 0=8 ,1=16
#define     MIC_AUDIO_TYPE              (0x02 - MIC_PCM_TYPE)



#define     MIC_ISO_EP_IN               3

#define     MIC_STR_INDEX               0

#define     MIC_INPUT_TERMINAL_ID       4
#define     MIC_FEATURE_UNIT_ID         5
#define     MIC_OUTPUT_TERMINAL_ID      6
#define     MIC_SELECTOR_UNIT_ID        7

#ifndef     TCFG_USB_APPLE_DOCK_EN
#define     TCFG_USB_APPLE_DOCK_EN      0
#endif

////////////CDC Class
#ifndef CDC_DATA_EP_IN
#define CDC_DATA_EP_IN              1
#endif
#ifndef CDC_DATA_EP_OUT
#define CDC_DATA_EP_OUT             1
#endif
#ifndef CDC_INTR_EP_IN
#define CDC_INTR_EP_IN              5
#endif
#ifndef MAXP_SIZE_CDC_BULKIN
#define MAXP_SIZE_CDC_BULKIN        64
#endif
#ifndef MAXP_SIZE_CDC_BULKOUT
#define MAXP_SIZE_CDC_BULKOUT       64
#endif
#ifndef MAXP_SIZE_CDC_INTRIN
#define MAXP_SIZE_CDC_INTRIN        8
#endif
#ifndef CDC_INTR_EP_ENABLE
#define CDC_INTR_EP_ENABLE          0
#endif

///////////CUSTOM_HID class
#ifndef CUSTOM_HID_EP_IN
#define CUSTOM_HID_EP_IN            1
#endif
#ifndef CUSTOM_HID_EP_OUT
#define CUSTOM_HID_EP_OUT           1
#endif
#ifndef MAXP_SIZE_CUSTOM_HIDIN
#define MAXP_SIZE_CUSTOM_HIDIN      64
#endif
#ifndef MAXP_SIZE_CUSTOM_HIDOUT
#define MAXP_SIZE_CUSTOM_HIDOUT     64
#endif

#endif  /*USB_STD_CLASS_DEF_H*/
