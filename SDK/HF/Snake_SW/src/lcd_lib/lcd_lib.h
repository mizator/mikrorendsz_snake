//////////////////////////////////////////////////////////////////////////////////
// Company: 		Mikrorendszerek Tervezese
// Engineer: 
// 
// Create Date:    	2017-10-28 
// Design Name: 	LCD
// Module Name:    	
// Project Name: 	Snake
// Target Devices: 	EADOGS102N-6	LCD
// Description: 	LCD_LIB.H
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
#ifndef __LCD_LIB_H_
#define __LCD_LIB_H_

#include <inttypes.h>

//	SPI CONTROL REGISTER
//	[ SS_reg [11] | Global EN [10] | Interrupt EN [9] | Interrupt Clear [8] | Baudrate [7:0] ]
#define SPICR 			0x00
#define SS_REG(bit)		(bit << 11)
#define GLOBAL_EN(bit)	(bit << 10)
#define INT_EN(bit)		(bit << 9)
#define INT_CLR(bit)	(bit << 8)

//	SPI STATUS
// [ IRQreg [10] | BUSY reg [9] | LCD CMDn/DATA [8] | Data [7:0] ]
#define SPISR 			0x04
#define IRQ_REG(bit)	(bit << 10)
#define BUSY_REG(bit)	(bit << 9)
#define CMDn_DAT(bit)	(bit << 8)

#define LCD_CONTROLREG 	MEM32(XPAR_LCD_SPI_0_BASEADDR + SPICR)
#define LCD_STATUSREG 	MEM32(XPAR_LCD_SPI_0_BASEADDR + SPISR)

//2: display start line set (lower 6 bits select first line on lcd from 64 lines in memory)
#define LCD_START_LINE        0x40

//3: Page address set (lower 4 bits select one of 8 pages)
#define LCD_PAGE_ADDRESS      0xB0

//4: column address (lower 4 bits are upper / lower nibble of column address)
#define LCD_COL_ADDRESS_MSB   0x10
#define LCD_COL_ADDRESS_LSB   0x00  //second part of column address

#define LCD_RESET_CMD         0xE2

void LcdBusy(void);
uint32_t LcdCntrl(uint32_t cntrl);
void LcdCmd(uint8_t cmd);
void LcdData(uint8_t data);
void LcdSelect(void);
void LcdDeSelect(void);
void LcdInit(void);
void LcdReset(void);
void LcdSetFirstLine(uint8_t line);
void LcdSetColumnAddress(uint8_t col);
void LcdSetPageAddress(uint8_t page);
void LCDGoToXY(uint8_t col, uint8_t page);
void LcdEn(void);

#endif
