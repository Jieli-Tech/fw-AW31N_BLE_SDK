#ifndef __INIT_H__
#define __INIT_H__

void system_init(void);
extern void exception_analyze(unsigned int *sp);

typedef void (*uninitcall_t)(void);

#define platform_uninitcall(fn) \
	const uninitcall_t __uninitcall_##fn sec_used(.platform.uninitcall) = fn

#define __do_uninitcall(prefix) \
	do { \
		uninitcall_t *uninit; \
		extern uninitcall_t prefix##_begin[], prefix##_end[]; \
		for (uninit=prefix##_begin; uninit<prefix##_end; uninit++) { \
			(*uninit)();	\
		} \
	}while(0)

#define do_platform_uninitcall()  __do_uninitcall(platform_uninitcall)

#endif
