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
#include "lcd_lib.h"
#include <xparameters.h>
#include <inttypes.h>

inline void LcdBusy(void){
	uint32_t reg;
	do{
		reg = LCD_STATUSREG;
	} while (reg & BUSY_REG(1));
}

inline uint32_t LcdCntrl(uint32_t cntrl){
	LcdBusy();
	if (cntrl != 0)
		LCD_CONTROLREG = (cntrl);
	return LCD_CONTROLREG;
}

inline void LcdCmd(uint8_t cmd){
	uint32_t reg;
	LcdBusy();
	reg = cmd & (~CMDn_DAT(1));
	LCD_STATUSREG = reg;
}

inline void LcdData(uint8_t data){
	uint32_t reg;
	LcdBusy();
	reg = data | CMDn_DAT(1);
	LCD_STATUSREG = reg;
}

inline void LcdSelect(void){
	uint32_t cfg;
	LcdBusy();
	cfg = LCD_CONTROLREG;
	cfg = cfg | SS_REG(1);
	LCD_CONTROLREG = cfg;
}

inline void LcdDeSelect(void){
	uint32_t cfg;
	LcdBusy();
	cfg = LCD_CONTROLREG;
	cfg = cfg & (~SS_REG(1));
	LCD_CONTROLREG = cfg;
}

inline void LcdEnable(void){
	uint32_t cfg;
	LcdBusy();
	cfg = LCD_CONTROLREG;
	cfg = cfg | GLOBAL_EN(1);
	LCD_CONTROLREG = cfg;
}

inline void LcdDisable(void){
	uint32_t cfg;
	LcdBusy();
	cfg = LCD_CONTROLREG;
	cfg = cfg & (~GLOBAL_EN(1));
	LCD_CONTROLREG = cfg;
}

void LcdInit(void)
{
	uint8_t init_seq [] = 
					{	0x40, 	// Fuggoleges gorgetes		Az elso megjelenitett sor a 0
						0xA0, 	// SEG irany beallitas		Normal iranyu oszlopcimzes
						0xC8,	// COM irany beallitas		Forditott iranyu sorcimzes
						0xA4,	// Minden pixel be 			Az SRAM tartalom megjelenitese
						0xA6,	// Inverz kijelzes 			Az inverz megjelenites tiltasa
						0xA2,	// LCD bias beallitas		1/9 LCD bias
						0x2F,	// Tapellatas vezerles		A tapellatas bekapcsolasa
						0x24,	// Tapellatas vezerles		A kontraszt beallitasa
						0x81,	// VEV beallitas			A kontraszt beallitasa
						0x2C,	// VEV beallitas			A kontraszt beallitasa
						0xFA,	// APC0 regiszter irasa		Homerseklet kompenzacio
						0x90,	// APC0 regiszter irasa		Homerseklet kompenzacio
						0xAF,	// Kijelzo engedelyezes		A megjelenites bekapcsolasa
						0x00	};

	LcdEnable();
	LcdSelect();

    uint8_t i;
    for(i=0; init_seq[i] != 0; i++){
    	LcdCmd(init_seq[i]);
    }
}

inline void LcdReset(void){
	LcdCmd(LCD_RESET_CMD);
}

inline void LcdSetFirstLine(uint8_t line){
	LcdCmd(LCD_START_LINE | ((line) & 0x3F));
}

inline void LcdSetColumnAddress(uint8_t col){
	LcdCmd(LCD_COL_ADDRESS_MSB | ((col>>4) & 0x0F));
	LcdCmd(LCD_COL_ADDRESS_LSB | ((col) & 0x0F));
}

inline void LcdSetPageAddress(uint8_t page){
 	LcdCmd(LCD_PAGE_ADDRESS | ((page) & 0x0F));
}

inline void LcdGoToXY(uint8_t col, uint8_t page){
	LcdSetColumnAddress(0x1e + col);
	LcdSetPageAddress(page);
}


inline void LcdArrayOut(uint8_t *data){
	uint8_t page, col;

	for(page = 0; page < LCD_PAGENUM; page++){
		for(col = 0; col < LCD_WIDTH;col++){
			LcdGoToXY(col,page);
			LcdData(data[page*LCD_WIDTH + col]);
		}
	}
}

inline void LcdLineOut(uint8_t data, uint8_t page, uint8_t x){
	LcdGoToXY(x,page);
	LcdData(data);
}

void LcdArrayConv(uint16_t *data){
	uint8_t y, x, bit;
	uint8_t dataout = 0;

	for(y = 0; y < LCD_PAGENUM; y++){
		for (x = 0; x < LCD_WIDTH; x++){
			for (bit = 0; bit < 8; bit++){
				if (data[LCD_WIDTH*8*y+bit*LCD_WIDTH+x]){
					dataout |= (1 << bit);
				}
				else {
					dataout &= (~(1 << bit));
				}
			}
			LcdLineOut(dataout,y, x);
		}
	}
}
