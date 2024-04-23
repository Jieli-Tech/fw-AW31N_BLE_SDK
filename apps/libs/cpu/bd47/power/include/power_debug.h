#ifndef __POWER_DEBUG_H__
#define __POWER_DEBUG_H__

#define     LP_P11_IO_DEBUG_0(i,x)       {P11_PORT->PB_OUT &= ~BIT(x), P11_PORT->PB_DIR &= ~BIT(x), P11_PORT->PB_SEL |= BIT(x);}
#define     LP_P11_IO_DEBUG_1(i,x)       {P11_PORT->PB_OUT |=  BIT(x), P11_PORT->PB_DIR &= ~BIT(x), P11_PORT->PB_SEL |= BIT(x);}

#define     LP_MSYS_IO_DEBUG_0(i,x)       {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT &= ~BIT(x);}
#define     LP_MSYS_IO_DEBUG_1(i,x)       {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT |= BIT(x);}

#if PCONFIG_PDEBUG_IO

#define		LP_IO_DEBUG_ENTER_H()	LP_MSYS_IO_DEBUG_1(A, 2)
#define		LP_IO_DEBUG_ENTER_L()	LP_MSYS_IO_DEBUG_0(A, 2)

#define		LP_IO_DEBUG_EXIT_H()	LP_MSYS_IO_DEBUG_1(A, 2)
#define		LP_IO_DEBUG_EXIT_L()	LP_MSYS_IO_DEBUG_0(A, 2)

#define		LP_IO_DEBUG_CLOCK_H()	LP_MSYS_IO_DEBUG_1(A, 5)
#define		LP_IO_DEBUG_CLOCK_L()	LP_MSYS_IO_DEBUG_0(A, 5)

#define 	LP_IO_DEBUG_RESET_H()	//LP_MSYS_IO_DEBUG_1(A, 3)
#define 	LP_IO_DEBUG_RESET_L()	//LP_MSYS_IO_DEBUG_0(A, 3)

#define		LP_IO_DEBUG_RESUME_H()	//LP_MSYS_IO_DEBUG_1(A, 4)
#define		LP_IO_DEBUG_RESUME_L()	//LP_MSYS_IO_DEBUG_0(A, 4)

#define		LP_IO_DEBUG_WAKUP_H()	LP_MSYS_IO_DEBUG_1(A, 7)
#define		LP_IO_DEBUG_WAKUP_L()	LP_MSYS_IO_DEBUG_0(A, 7)

#define 	LP_IO_DEBUG_REBOOT_H()	//LP_MSYS_IO_DEBUG_1(A, 5)
#define 	LP_IO_DEBUG_REBOOT_L()	//LP_MSYS_IO_DEBUG_0(A, 5)

#define 	LP_IO_DEBUG_PER_ENTER_H()	LP_MSYS_IO_DEBUG_1(A, 4)
#define 	LP_IO_DEBUG_PER_ENTER_L()	LP_MSYS_IO_DEBUG_0(A, 4)

#define 	LP_IO_DEBUG_PER_EXIT_H()	LP_MSYS_IO_DEBUG_1(A, 5)
#define 	LP_IO_DEBUG_PER_EXIT_L()	LP_MSYS_IO_DEBUG_0(A, 5)

#else

#define		LP_IO_DEBUG_ENTER_H()
#define		LP_IO_DEBUG_ENTER_L()

#define		LP_IO_DEBUG_EXIT_H()
#define		LP_IO_DEBUG_EXIT_L()

#define		LP_IO_DEBUG_CLOCK_H()
#define		LP_IO_DEBUG_CLOCK_L()

#define 	LP_IO_DEBUG_RESET_H()
#define 	LP_IO_DEBUG_RESET_L()

#define		LP_IO_DEBUG_RESUME_H()
#define		LP_IO_DEBUG_RESUME_L()

#define		LP_IO_DEBUG_WAKUP_H()
#define		LP_IO_DEBUG_WAKUP_L()

#define 	LP_IO_DEBUG_REBOOT_H()
#define 	LP_IO_DEBUG_REBOOT_L()

#define 	LP_IO_DEBUG_PER_ENTER_H()
#define 	LP_IO_DEBUG_PER_ENTER_L()

#define 	LP_IO_DEBUG_PER_EXIT_H()
#define 	LP_IO_DEBUG_PER_EXIT_L()

#endif

//-------------------------------------------------------------------
/*调试pdown进不去的场景，影响低功耗流程
 * 打印蓝牙和系统分别可进入低功耗的时间(msec)
 * 打印当前哪些模块处于busy,用于蓝牙已经进入sniff但系统无法进入低功耗的情况，如果usr_timer处于busy则会打印对应的func地址
 */
extern const char debug_is_idle;

//-------------------------------------------------------------------
/* 调试快速起振信息，不影响低功耗流程
 */
extern const bool pdebug_xosc_resume;

//-------------------------------------------------------------------
/* 调试低功耗流程
 */
//出pdown打印信息，不影响低功耗流程
extern const bool pdebug_pdown_info;

//使能串口调试低功耗，在pdown、soff模式保持串口, pdebug_pubyte_pdown\pdebug_lp_dump_ram\pdebug_putbyte_soff\log_debug
extern const u32 pdebug_uart_lowpower;
extern const u32 pdebug_uart_port;

//使能串口putbyte调试pdown流程
extern const bool pdebug_putbyte_pdown;

//使能串口putbyte调试soff流程
extern  const bool pdebug_putbyte_soff;

//使能串口pdown/poff/soff打印所有的寄存器
extern const bool pdebug_lp_dump_ram;

//使能uart_flowing
extern const bool pdebug_uart_flowing;

//使能低功耗耗时检查
extern const bool pdebug_reserve_time;

void p_putbyte(char c);
void p_putnbyte(char c);
void p_put_u4hex(u8 dat);
void p_printf(char *str, u32 value);
void lp_dump_ram(int arg);

#define PDEBUG_UART_FLOWING()	\
				if(pdebug_uart_flowing)		\
					log_info("fun: %s, %d", __FUNCTION__, __LINE__)

//pdown流程使用改宏
#define P_PUTBYTE_PD(byte)			\
		if (pdebug_putbyte_pdown)	\
			p_putbyte(byte);

//只有soff使用该宏控制
#define P_PUTNBYTE(byte)				\
		if (pdebug_putbyte_soff)		\
			p_putnbyte(byte);

//------------------------------------------------------------------------------------------
/*在多个地方调用，允许printf在ram
 */
#define GPIO_DUMP(printf_tmp)	\
		printf_tmp("JL_PORTA->DIR : 0x%x", JL_PORTA->DIR);       \
    	printf_tmp("JL_PORTA->DIE : 0x%x", JL_PORTA->DIE);       \
    	printf_tmp("JL_PORTA->DIEH: 0x%x", JL_PORTA->DIEH);      \
    	printf_tmp("JL_PORTA->PU0 : 0x%x", JL_PORTA->PU0);       \
    	printf_tmp("JL_PORTA->PU1 : 0x%x", JL_PORTA->PU1);       \
    	printf_tmp("JL_PORTA->PD0 : 0x%x", JL_PORTA->PD0);       \
    	printf_tmp("JL_PORTA->PD1 : 0x%x", JL_PORTA->PD1);       \
    	printf_tmp("JL_PORTA->HD0 : 0x%x", JL_PORTA->HD0);       \
    	printf_tmp("JL_PORTA->HD1 : 0x%x", JL_PORTA->HD1);       \
																 \
    	printf_tmp("JL_PORTF->DIR : 0x%x", JL_PORTF->DIR);       \
    	printf_tmp("JL_PORTF->DIE : 0x%x", JL_PORTF->DIE);       \
    	printf_tmp("JL_PORTF->DIEH: 0x%x", JL_PORTF->DIEH);      \
    	printf_tmp("JL_PORTF->PU0 : 0x%x", JL_PORTF->PU0);       \
    	printf_tmp("JL_PORTF->PU1 : 0x%x", JL_PORTF->PU1);       \
    	printf_tmp("JL_PORTF->PD0 : 0x%x", JL_PORTF->PD0);       \
    	printf_tmp("JL_PORTF->PD1 : 0x%x", JL_PORTF->PD1);       \
    	printf_tmp("JL_PORTF->HD0 : 0x%x", JL_PORTF->HD0);       \
    	printf_tmp("JL_PORTF->HD1 : 0x%x", JL_PORTF->HD1);       \
		   	    	   									         \
    	printf_tmp("JL_PORTUSB->DIR : 0x%x", JL_PORTUSB->DIR);   \
    	printf_tmp("JL_PORTUSB->DIE : 0x%x", JL_PORTUSB->DIE);   \
    	printf_tmp("JL_PORTUSB->DIEH: 0x%x", JL_PORTUSB->DIEH);  \
    	printf_tmp("JL_PORTUSB->PU0 : 0x%x", JL_PORTUSB->PU0);   \
    	printf_tmp("JL_PORTUSB->PU1 : 0x%x", JL_PORTUSB->PU1);   \
    	printf_tmp("JL_PORTUSB->PD0 : 0x%x", JL_PORTUSB->PD0);   \
    	printf_tmp("JL_PORTUSB->PD1 : 0x%x", JL_PORTUSB->PD1);   \
    	printf_tmp("JL_PORTUSB->HD0 : 0x%x", JL_PORTUSB->HD0);   \
    	printf_tmp("JL_PORTUSB->HD1 : 0x%x", JL_PORTUSB->HD1);	 \
    	printf_tmp("JL_PORTUSB->CON : 0x%x", JL_PORTUSB->CON);	 \
    	printf_tmp("JL_USB->CON : 0x%x", 	 JL_USB->CON0)


#define P11_GPIO_DUMP(printf_tmp)	\
		printf_tmp("P11_PORT->PB_SEL : 0x%x", P11_PORT->PB_SEL ); \
		printf_tmp("P11_PORT->PB_DIR : 0x%x", P11_PORT->PB_DIR ); \
		printf_tmp("P11_PORT->PB_DIE : 0x%x", P11_PORT->PB_DIE ); \
		printf_tmp("P11_PORT->PB_DIEH: 0x%x", P11_PORT->PB_DIEH); \
		printf_tmp("P11_PORT->PB_PU0 : 0x%x", P11_PORT->PB_PU0 ); \
		printf_tmp("P11_PORT->PB_PU1 : 0x%x", P11_PORT->PB_PU1 ); \
		printf_tmp("P11_PORT->PB_PD0 : 0x%x", P11_PORT->PB_PD0 ); \
		printf_tmp("P11_PORT->PB_PD1 : 0x%x", P11_PORT->PB_PD1 )

#define LPTMR_DUMP(printf_tmp)                                              \
    	printf_tmp("P3_LP_RSC00: 0x%x", p33_rx_1byte(P3_LP_RSC00));         \
    	printf_tmp("P3_LP_RSC01: 0x%x", p33_rx_1byte(P3_LP_RSC01));         \
    	printf_tmp("P3_LP_RSC02: 0x%x", p33_rx_1byte(P3_LP_RSC02));         \
    	printf_tmp("P3_LP_RSC03: 0x%x", p33_rx_1byte(P3_LP_RSC03));         \
    	printf_tmp("P3_LP_PRD00: 0x%x", p33_rx_1byte(P3_LP_PRD00));         \
    	printf_tmp("P3_LP_PRD01: 0x%x", p33_rx_1byte(P3_LP_PRD01));         \
    	printf_tmp("P3_LP_PRD02: 0x%x", p33_rx_1byte(P3_LP_PRD02));         \
    	printf_tmp("P3_LP_PRD03: 0x%x", p33_rx_1byte(P3_LP_PRD03));         \
    	printf_tmp("P3_LP_RSC10: 0x%x", p33_rx_1byte(P3_LP_RSC10));         \
    	printf_tmp("P3_LP_RSC11: 0x%x", p33_rx_1byte(P3_LP_RSC11));         \
    	printf_tmp("P3_LP_RSC12: 0x%x", p33_rx_1byte(P3_LP_RSC12));         \
    	printf_tmp("P3_LP_RSC13: 0x%x", p33_rx_1byte(P3_LP_RSC13));         \
    	printf_tmp("P3_LP_PRD10: 0x%x", p33_rx_1byte(P3_LP_PRD10));         \
    	printf_tmp("P3_LP_PRD11: 0x%x", p33_rx_1byte(P3_LP_PRD11));         \
    	printf_tmp("P3_LP_PRD12: 0x%x", p33_rx_1byte(P3_LP_PRD12));         \
    	printf_tmp("P3_LP_PRD13: 0x%x", p33_rx_1byte(P3_LP_PRD13));         \
                                                                            \
    	printf_tmp("P3_LP_TMR0_CLK: 0x%x", p33_rx_1byte(P3_LP_TMR0_CLK));   \
    	printf_tmp("P3_LP_TMR1_CLK: 0x%x", p33_rx_1byte(P3_LP_TMR1_CLK));   \
    	printf_tmp("P3_LP_TMR0_CON: 0x%x", p33_rx_1byte(P3_LP_TMR0_CON));   \
    	printf_tmp("P3_LP_TMR1_CON: 0x%x", p33_rx_1byte(P3_LP_TMR1_CON));	\
    	printf_tmp("P3_PMU_CON5: 0x%x", p33_rx_1byte(P3_PMU_CON5));







#define P33_IO_EDGE_WKUP_DUMP(printf_tmp)                                       \
    	printf_tmp("P3_PORT_SEL0 : 0x%x", p33_rx_1byte(P3_PORT_SEL0));			\
    	printf_tmp("P3_PORT_SEL1 : 0x%x", p33_rx_1byte(P3_PORT_SEL1));          \
    	printf_tmp("P3_PORT_SEL2 : 0x%x", p33_rx_1byte(P3_PORT_SEL2));          \
    	printf_tmp("P3_PORT_SEL3 : 0x%x", p33_rx_1byte(P3_PORT_SEL3));          \
    	printf_tmp("P3_PORT_SEL4 : 0x%x", p33_rx_1byte(P3_PORT_SEL4));          \
    	printf_tmp("P3_PORT_SEL5 : 0x%x", p33_rx_1byte(P3_PORT_SEL5));          \
    	printf_tmp("P3_PORT_SEL6 : 0x%x", p33_rx_1byte(P3_PORT_SEL6));          \
    	printf_tmp("P3_PORT_SEL7 : 0x%x", p33_rx_1byte(P3_PORT_SEL7));          \
    	printf_tmp("P3_PORT_SEL8 : 0x%x", p33_rx_1byte(P3_PORT_SEL8));          \
    	printf_tmp("P3_PORT_SEL9 : 0x%x", p33_rx_1byte(P3_PORT_SEL9));          \
    	printf_tmp("P3_PORT_SEL10: 0x%x", p33_rx_1byte(P3_PORT_SEL10));         \
    	printf_tmp("P3_PORT_SEL11: 0x%x", p33_rx_1byte(P3_PORT_SEL11));         \
                                                                                \
                                                                                \
    	printf_tmp("P3_WKUP_FLT_EN0: 0x%x", p33_rx_1byte(P3_WKUP_FLT_EN0));     \
    	printf_tmp("P3_WKUP_P_IE0  : 0x%x", p33_rx_1byte(P3_WKUP_P_IE0));       \
    	printf_tmp("P3_WKUP_N_IE0  : 0x%x", p33_rx_1byte(P3_WKUP_N_IE0));       \
    	printf_tmp("P3_WKUP_LEVEL0 : 0x%x", p33_rx_1byte(P3_WKUP_LEVEL0));      \
    	printf_tmp("P3_WKUP_P_CPND0: 0x%x", p33_rx_1byte(P3_WKUP_P_CPND0));     \
    	printf_tmp("P3_WKUP_N_CPND0: 0x%x", p33_rx_1byte(P3_WKUP_N_CPND0));     \
    	printf_tmp("P3_WKUP_P_PND0 : 0x%x", p33_rx_1byte(P3_WKUP_P_PND0));      \
    	printf_tmp("P3_WKUP_N_PND0 : 0x%x", p33_rx_1byte(P3_WKUP_N_PND0));      \
    	printf_tmp("P3_WKUP_FLT_EN1: 0x%x", p33_rx_1byte(P3_WKUP_FLT_EN1));     \
    	printf_tmp("P3_WKUP_P_IE1  : 0x%x", p33_rx_1byte(P3_WKUP_P_IE1));       \
    	printf_tmp("P3_WKUP_N_IE1  : 0x%x", p33_rx_1byte(P3_WKUP_N_IE1));       \
    	printf_tmp("P3_WKUP_LEVEL1 : 0x%x", p33_rx_1byte(P3_WKUP_LEVEL1));      \
    	printf_tmp("P3_WKUP_P_CPND1: 0x%x", p33_rx_1byte(P3_WKUP_P_CPND1));     \
    	printf_tmp("P3_WKUP_N_CPND1: 0x%x", p33_rx_1byte(P3_WKUP_N_CPND1));     \
    	printf_tmp("P3_WKUP_P_PND1 : 0x%x", p33_rx_1byte(P3_WKUP_P_PND1));      \
    	printf_tmp("P3_WKUP_N_PND1 : 0x%x", p33_rx_1byte(P3_WKUP_N_PND1));      \
    	printf_tmp("P3_WKUP_CLK_SEL: 0x%x", p33_rx_1byte(P3_WKUP_CLK_SEL))


#define LP_DUMP(printf_tmp)	\

#define PINR_DUMP(printf_tmp) \
    	printf_tmp("P3_PINR_CON:  0x%x", p33_rx_1byte(P3_PINR_CON));   \
    	printf_tmp("P3_PINR_SAFE: 0x%x", p33_rx_1byte(P3_PINR_SAFE));  \
    	printf_tmp("P3_PORT_SEL0: 0x%x", p33_rx_1byte(P3_PORT_SEL0));  \

#define LVD_DUMP(printf_tmp) \

#define P33_ANA_DUMP(printf_tmp) \






#endif
