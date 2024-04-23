#ifndef __PMU_WAKEUP_H__
#define __PMU_WAKEUP_H__

struct _pwk_hdl {
    u32 g_filter;
    u32 g_port_used;
    struct _p33_io_wakeup_config g_port[MAX_WAKEUP_PORT];
#if MAX_WAKEUP_ANA_PORT
    u32 a_filter;
    u32 a_port_used;
    struct _p33_io_wakeup_config a_port[MAX_WAKEUP_ANA_PORT];
#endif

    bool init_flag;
};

extern struct _pwk_hdl pwk_hdl;

#define list_for_each_general_wakeup_port(index) \
			for(index = 0; index < MAX_WAKEUP_PORT; index++) \

#define list_for_each_analog_wakeup_port(index) \
			for(index=0; index < MAX_WAKEUP_ANA_PORT; index++) \

#endif
