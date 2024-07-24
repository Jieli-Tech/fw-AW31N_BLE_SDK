//*********************************************************************************//
// Module name : p33_interface.h
// Description : common use subroutine                                             //
// By Designer : Nelson.long
// Dat changed :                                                                   //
//*********************************************************************************//
#ifndef __P33_INTERFACE_
#define __P33_INTERFACE_

#define p33_cs_h            do{local_irq_disable();JL_PMU->SPI_CON  |= BIT(0);}while(0)
#define p33_cs_l            do{JL_PMU->SPI_CON  &= ~BIT(0);local_irq_enable();}while(0)

#define LP_KST              JL_PMU->PMU_CON   |= BIT(6)

#define P33_OR              0b001
#define P33_AND             0b010
#define P33_XOR             0b011

extern u8 p33_buf(u8 buf);
extern void p33_xor_1byte(u16 addr, u32 data0);
extern void p33_and_1byte(u16 addr, u32 data0);
extern void p33_or_1byte(u16 addr, u32 data0);
extern void p33_tx_1byte(u16 addr, u32 data0);
extern u8 p33_rx_1byte(u16 addr);
extern void p33_rx_nbyte(u16 addr, u8 *buf_ptr, u8 len);
extern void p33_tx_1byte_fast(u16 addr, u32 data0);
extern void P33_CON_SET(u16 addr, u8 start, u8 len, u8 data);

#define P33_INTERFACE 1
#endif

//*********************************************************************************//
//                                                                                 //
//                               end of this module                                //
//                                                                                 //
//*********************************************************************************//

