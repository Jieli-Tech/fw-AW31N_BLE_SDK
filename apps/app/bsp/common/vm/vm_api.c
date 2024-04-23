#include "vm.h"
#include "app_modules.h"
#include "app_config.h"

#if (SYS_MEMORY_SELECT == USE_OLD_VM)
#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

#define LABEL_INDEX_LEN_CRC_SIZE        (4)
static u8 vm_buff[sizeof(VM_INDEX_BUFF) + LABEL_INDEX_LEN_CRC_SIZE];
u16 vm_buff_alloc(u8 **buf)
{
    if (buf == NULL) {
        return 0;
    }

    *buf = vm_buff;
    return sizeof(vm_buff);
}
#endif
