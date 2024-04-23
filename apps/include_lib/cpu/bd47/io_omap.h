
//===============================================================================//
//
//      output function define
//
//===============================================================================//
#define FO_GP_OCH0        ((0 << 2)|BIT(1))
#define FO_GP_OCH1        ((1 << 2)|BIT(1))
#define FO_GP_OCH2        ((2 << 2)|BIT(1))
#define FO_GP_OCH3        ((3 << 2)|BIT(1))
#define FO_GP_OCH4        ((4 << 2)|BIT(1))
#define FO_GP_OCH5        ((5 << 2)|BIT(1))
#define FO_GP_OCH6        ((6 << 2)|BIT(1))
#define FO_GP_OCH7        ((7 << 2)|BIT(1))
#define FO_LEDC_DOUT0        ((8 << 2)|BIT(1)|BIT(0))
#define FO_LEDC_DOUT1        ((9 << 2)|BIT(1)|BIT(0))
#define FO_IIC0_SCL        ((10 << 2)|BIT(1)|BIT(0))
#define FO_IIC0_SDA        ((11 << 2)|BIT(1)|BIT(0))
#define FO_UART0_TX        ((12 << 2)|BIT(1)|BIT(0))
#define FO_UART1_TX        ((13 << 2)|BIT(1)|BIT(0))
#define FO_UART2_TX        ((14 << 2)|BIT(1)|BIT(0))
#define FO_MCPWM_H0        ((15 << 2)|BIT(1)|BIT(0))
#define FO_MCPWM_L0        ((16 << 2)|BIT(1)|BIT(0))
#define FO_MCPWM_H1        ((17 << 2)|BIT(1)|BIT(0))
#define FO_MCPWM_L1        ((18 << 2)|BIT(1)|BIT(0))
#define FO_SPI1_CLK        ((19 << 2)|BIT(1)|BIT(0))
#define FO_SPI1_DA0        ((20 << 2)|BIT(1)|BIT(0))
#define FO_SPI1_DA1        ((21 << 2)|BIT(1)|BIT(0))
#define FO_SPI1_DA2        ((22 << 2)|BIT(1)|BIT(0))
#define FO_SPI1_DA3        ((23 << 2)|BIT(1)|BIT(0))

//===============================================================================//
//
//      IO output select sfr
//
//===============================================================================//
typedef struct {
    __RW __u8 PA0_OUT;
    __RW __u8 PA1_OUT;
    __RW __u8 PA2_OUT;
    __RW __u8 PA3_OUT;
    __RW __u8 PA4_OUT;
    __RW __u8 PA5_OUT;
    __RW __u8 PA6_OUT;
    __RW __u8 PA7_OUT;
    __RW __u8 PA8_OUT;
    __RW __u8 PA9_OUT;
    __RW __u8 PA10_OUT;
    __RW __u8 PA11_OUT;
    __RW __u8 USBDP_OUT;
    __RW __u8 USBDM_OUT;
} JL_OMAP_TypeDef;

#define JL_OMAP_BASE      (ls_base + map_adr(0x36, 0x00))
#define JL_OMAP           ((JL_OMAP_TypeDef   *)JL_OMAP_BASE)


