/*
 * Copyright (c) 2009-2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include <xparameters.h>
#include <xintc_l.h>
#include <xtmrctr_l.h>
#include <mb_interface.h>
#include "platform.h"
#include "lcd_lib/lcd_lib.h"
//#include <dogm-graphic.h>

#define MEM8(addr)   (*(volatile unsigned char *)(addr))
#define MEM16(addr)  (*(volatile unsigned short *)(addr))
#define MEM32(addr)  (*(volatile unsigned long *)(addr))

#define TRUE 1
#define FALSE 0

//	SPI CONTROL REGISTER
//	[ SS_reg [11] | Global EN [10] | Interrupt EN [9] | Interrupt Clear [8] | Baudrate [7:0] ]
#define SPICR 0x00
#define SS_REG(bit)		(bit << 11)
#define GLOBAL_EN(bit)	(bit << 10)
#define INT_EN(bit)		(bit << 9)
#define INT_CLR(bit)	(bit << 8)

//	SPI STATUS
// [ IRQreg [10] | BUSY reg [9] | LCD CMDn/DATA [8] | Data [7:0] ]
#define SPISR 0x04
#define IRQ_REG(bit)	(bit << 10)
#define BUSY_REG(bit)	(bit << 9)
#define CMDn_DAT(bit)	(bit << 8)



void print(char *str);


void lcd_cntrl(int cntrl);
void lcd_data(int data);
void lcd_command(int cmd);
void lcd_sel();
void lcd_desel();

int main()
{
    init_platform();



    LcdInit();
    LcdCmd(0xA5);





    while(1);
    return 0;
}


