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
#ifndef __IO_LIB_H_
#define __IO_LIB_H_

#include <inttypes.h>

#define DISPREG1 			0x01 // 8bit W
#define DISPREG2 			0x02 // 8bit W
#define DIPSWITCH 			0x06 // 8bit R
#define NAVSWITCH 			0x0A // 8bit R

#define MEM8(addr)   (*(volatile uint8_t *)(addr))
#define MEM16(addr)  (*(volatile unsigned short *)(addr))
#define MEM32(addr)  (*(volatile unsigned long *)(addr))

#define O_DISP1 	MEM8(XPAR_LOGSYS_AXI_SP6_SIMPLEIO_0_BASEADDR + DISPREG1)
#define O_DISP2 	MEM8(XPAR_LOGSYS_AXI_SP6_SIMPLEIO_0_BASEADDR + DISPREG2)
#define I_DIPSW 	MEM32(XPAR_LOGSYS_AXI_SP6_SIMPLEIO_0_BASEADDR + DIPSWITCH)
#define I_NAVSW 	MEM32(XPAR_LOGSYS_AXI_SP6_SIMPLEIO_0_BASEADDR + NAVSWITCH)

#define UP 			0x01
#define DOWN 		0x02
#define RIGHT		0x08
#define LEFT		0x04
#define PUSH 		0x10

extern const unsigned char bin2sevenseg[];


void Disp1W(uint8_t datd1);
void Disp2W(uint8_t datd2);
void DispW(uint8_t dat);
uint8_t DipswR(void);
uint8_t NavswR(void);


#endif
