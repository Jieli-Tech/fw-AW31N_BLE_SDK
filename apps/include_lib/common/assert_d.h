#ifndef  __ASSERT_D_H__
#define  __ASSERT_D_H__


extern const int config_asser;
extern void __local_irq_disable();
extern const unsigned char config_exception_record_info;

/* cpu_assert_debug(); \ */
/* system_reset(ASSERT_FLAG); \ */

#define ASSERT_R(a,...)   \
    do { \
        if(config_asser){\
            if(!(a)){ \
                printf("cpu %d file:%s, line:%d",current_cpu_id(), __FILE__, __LINE__); \
                printf("ASSERT-FAILD: "#a" "__VA_ARGS__); \
                chip_reset();\
            } \
        }else {\
            if(!(a)){ \
                chip_reset();\
            }\
        }\
    }while(0);

#define assert(a,...)   \
    do { \
        if(config_asser){\
            if(!(a)){ \
                printf("cpu %d file:%s, line:%d",current_cpu_id(), __FILE__, __LINE__); \
                printf("ASSERT-FAILD: "#a" "__VA_ARGS__); \
                local_irq_disable();\
                while(1); \
            } \
        }else {\
            if(!(a)){ \
                if (config_exception_record_info) { \
                    local_irq_disable();\
                    while(1); \
                }\
                chip_reset();\
            }\
        }\
    }while(0);

//#define ASSERT ASSERT_R
#define ASSERT assert

#endif  /*ASSERT_H*/
