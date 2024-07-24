#ifndef __if_decoder_ctrl_h
#define __if_decoder_ctrl_h

#include "string.h"

#include "audio_typedef.h"
//#define DECODE_NORMAL  0x00
//#define DECODE_FF      0x01
//#define DECODE_FR      0x02
//#define DECODE_STOP    0x03

#define CMD_SET_CONTINUE_BK   0x90
#define CMD_SET_FADEOUT       0x93


typedef struct _AUDIO_FADE_PARA {
    u32 mode;                 //0跟之前一样，1代表要fade到0
} AUDIO_FADE_PARA;


#define PLAY_MOD_NORMAL   0x00
#define PLAY_MOD_FF       0x01
#define PLAY_MOD_FB       0x02


//play control
#define PLAY_FILE       0x80000000
#define PLAY_CONTINUE   0x80000001
#define PLAY_NEXT       0x80000002

#define AUDIO_BK_EN

typedef struct if_decoder_io IF_DECODER_IO;


#endif
