#ifndef	_SPI_HW_H_
#define _SPI_HW_H_

#include "typedef.h"
#include "ioctl.h"

#define SUPPORT_SPI0  0   //是否使能SPI0 系统占用
#define SUPPORT_SPI1  1   //是否使能SPI1
#define SPI1_SUPPORT_UNIDIR_4BIT  1   //spi 4bit
#define SUPPORT_SPI2  0   //无spi2
#define SPI2_SUPPORT_UNIDIR_4BIT  0   //spi 4bit

//spi1:任意io

typedef enum spi_index {
    HW_SPI0, //SPI0系统占用
    HW_SPI1,
    HW_SPI_MAX_NUM,
} hw_spi_dev;

enum spi_bit_mode {
    SPI_FIRST_BIT_MSB,  //7,6,5,4,3,2,1,0
    SPI_FIRST_BIT_LSB,  //0,1,2,3,4,5,6,7
    SPI_FIRST_BIT_BIT3, //3,2,1,0,7,6,5,4
    SPI_FIRST_BIT_BIT4, //4,5,6,7,0,1,2,3
};
#endif

