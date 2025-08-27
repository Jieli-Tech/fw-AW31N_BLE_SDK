#ifndef __JIFFIES_H__
#define __JIFFIES_H__

#define HZ				100L
#define MSEC_PER_SEC	1000L
#define USEC_PER_MSEC	1000L
#define NSEC_PER_USEC	1000L
#define NSEC_PER_MSEC	1000000L
#define USEC_PER_SEC	1000000L
#define NSEC_PER_SEC	1000000000L
#define FSEC_PER_SEC	1000000000000000LL

extern volatile u32 jiffies;

#define typecheck(type,x) \
    ({  type __dummy; \
        typeof(x) __dummy2; \
        (void)(&__dummy == &__dummy2); \
        1; \
    })


#define time_after(a,b)					((int)(b) - (int)(a) < 0)
#define time_before(a,b)				time_after(b,a)

#define DEF_JIFFIES_MS                  (10) //fixed

#define msecs_to_jiffies(msec) 		    ((msec)/DEF_JIFFIES_MS)
#define jiffies_to_msecs(j) 		    ((j)*DEF_JIFFIES_MS)

// #define msecs_to_jiffies(msec) 		    ((msec)/jiffies_unit)
// #define jiffies_to_msecs(j) 		    ((j)*jiffies_unit)
// #define msecs_to_jiffies(msec) 		    ((msec)/jiffies)
// #define jiffies_to_msecs(j) 		    ((j)*jiffies)

void delay(u32 i);
void delay_10ms(u32 tick);
void os_time_dly(u32 tick);

// #define delay_nops  delay


u32 get_jiffies(void);
void set_jiffies(u32 cnt);

unsigned long jiffies_msec(void);
int jiffies_msec2offset(unsigned long begin_msec, unsigned long end_msec);
unsigned long jiffies_offset2msec(unsigned long begin_msec, int offset_msec);

unsigned long jiffies_usec(void);
int jiffies_usec2offset(unsigned long begin, unsigned long end);
unsigned long jiffies_offset2usec(unsigned long base_usec, int offset_usec);

#endif


