#ifndef __POWER_DRIVER_H__
#define __POWER_DRIVER_H__

//
//
//                    power
//
//
//
//******************************************************************

#define ROM_STACK_SIZE      16

struct power {
    struct list_head entry;
    void *priv;
    const struct low_power_operation *ops;
};

struct low_power_group {
    /* struct list_head head_on; */
    struct list_head request_head;
    u32 timeout;
};

struct _pdrv_hdl {
    //power
    struct _phw_dev *pmu_dev;
    void (*p_putbyte)(char);

    //sleep
#if CONFIG_P11_CPU_ENABLE
    u32 p2m_ie;
#endif

    u32 t1;
    u32 t2;
    u32 t3;
    u32 t4;
    u32 T_prp;
    u32 T_rev;
    u32 T_min;

    struct low_power_group group0;  //bluetooth
    struct low_power_group group1;  //System
    enum LOW_POWER_LEVEL level;
    u32 *rom_stack;

    //power
    u8 p11_init;

    //sleep
    u8 config;
};

struct tskTaskControlBlock {
    /*< Points to the location of the last item placed on the tasks stack.  THIS MUST BE THE FIRST MEMBER OF THE TCB STRUCT. */
    volatile int *pxTopOfStack;
};

extern struct _pdrv_hdl pdrv_hdl;




#endif
