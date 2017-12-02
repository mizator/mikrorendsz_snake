//////////////////////////////////////////////////////////////////////////////////
// Company: 		Mikrorendszerek Tervezese
// Engineer: 
// 
// Create Date:    	2017-10-28 
// Design Name: 	LCD
// Module Name:    	
// Project Name: 	Snake
// Target Devices: 	EADOGS102N-6	LCD
// Description: 	LCD_LIB.C
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
#include "io_lib.h"
#include <stdio.h>
#include <xparameters.h>
#include <inttypes.h>

const unsigned char bin2sevenseg[] = {
		0x3f,
		0x06,
		0x5b,
		0x4f,
		0x66,
		0x6d,
		0x7d,
		0x07,
		0x7f,
		0x6f
};


inline void Disp1W(uint8_t dat){
	O_DISP1 = dat;
}

inline void Disp2W(uint8_t dat){
	O_DISP2 = dat;
}

void DispW(uint8_t dat){
	if (dat>99) dat = 66;
	Disp2W(bin2sevenseg[dat / 10]);
	Disp1W(bin2sevenseg[dat % 10]);
}

uint8_t DipswR(void){
	uint8_t dat;
	dat = (I_DIPSW & 0x00ff0000) >> 16;
	return dat;
}

uint8_t NavswR(void){
	uint8_t dat;
	dat = (I_NAVSW & 0x00ff0000) >> 16;
	return dat;
}
