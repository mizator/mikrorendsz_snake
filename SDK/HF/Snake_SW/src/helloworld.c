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

int init_lcd [] = {	0x40, 	// Fuggoleges gorgetes		Az elso megjelenitett sor a 0
					0xA0, 	// SEG irany beallitas		Normal iranyu oszlopcimzes
					0xC8,	// COM irany beallitas		Forditott iranyu sorcimzes
					0xA4,	// Minden pixel be 			Az SRAM tartalom megjelenitese
					0xA6,	// Inverz kijelzes 			Az inverz megjelenites tiltasa
					0xA2,	// LCD bias beallitas		1/9 LCD bias
					0x2F,	// Tapellatas vezerles		A tapellatas bekapcsolasa
					0x27,	// Tapellatas vezerles		A kontraszt beallitasa
					0x81,	// VEV beallitas			A kontraszt beallitasa
					0x10,	// VEV beallitas			A kontraszt beallitasa
					0xFA,	// APC0 regiszter irasa		Homerseklet kompenzacio
					0x90,	// APC0 regiszter irasa		Homerseklet kompenzacio
					0xAF	// Kijelzo engedelyezes		A megjelenites bekapcsolasa
					};

void print(char *str);

void LCD_wr(int data);
void lcd_data(uint8_t data);
void lcd_command(uint8_t cmd);
void lcd_sel();
void lcd_desel();

int main()
{
    init_platform();

    print("Hello World\n\r");

    return 0;
}

void lcd_data(uint8_t data) {
	int out = data;

	data = data | CMDn_DAT(1)



  LCD_SELECT();
  LCD_DRAM();
  spi_write(data);
  LCD_UNSELECT();
  lcd_inc_column(1);
  }

/******************************************************************************
 * Writes one command byte
 * cmd           - the command byte
 */
void lcd_command(int cmd) {
	int reg = MEM32(XPAR_LCD_SPI_0_BASEADDR + SPISR);
  LCD_SELECT();
  LCD_CMD();
  spi_write(cmd);
  LCD_UNSELECT();
  }

void lcd_sel(){

	// TODO : check busy bit

	int cfg = MEM32(XPAR_LCD_SPI_0_BASEADDR + SPICR)
	cfg = cfq | SS_REG(l);
	MEM32(XPAR_LCD_SPI_0_BASEADDR + SPICR) = cfg;
}

void lcd_desel(){

	// TODO : check busy bit

	int cfg = MEM32(XPAR_LCD_SPI_0_BASEADDR + SPICR)
	cfg = cfq & SS_REG(0);
	MEM32(XPAR_LCD_SPI_0_BASEADDR + SPICR) = cfg;
}
