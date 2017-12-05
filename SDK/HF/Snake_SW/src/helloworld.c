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
#include <stdlib.h>
#include <xparameters.h>
#include <xintc_l.h>
#include <xtmrctr_l.h>
#include <mb_interface.h>
#include "platform.h"
#include "lcd_lib/lcd_lib.h"
#include "io_lib/io_lib.h"
//#include <dogm-graphic.h>

#define TRUE 1
#define FALSE 0

#define MAPSIZE LCD_WIDTH*LCD_HEIGHT

volatile uint8_t counter = 0;

int8_t headpos_x = 0;
int8_t headpos_y = 0;
int8_t direction = RIGHT;
int8_t direction_x = 0;
int8_t direction_y = 0;
int8_t activedirection_x = 0;
int8_t activedirection_y = 0;
int snake_size = 0;
uint8_t running = 0;

uint8_t alma_x;
uint8_t alma_y;

uint8_t snake_grow = 0;




volatile uint8_t update = 0;


void initmap(uint16_t * map);
int8_t snakecheck(uint16_t * map);
void snakeupdate(uint16_t * map);
uint8_t almagen(uint16_t * map);
void placealma(uint16_t * map);

void timer_int_handler(void *instance_Ptr)
{
	if (update == 0){
	update = 1;
	}


	// a megszakítás jelzõbit törlése
	unsigned long csr;
	csr = XTmrCtr_GetControlStatusReg(XPAR_AXI_TIMER_0_BASEADDR, 0);
	XTmrCtr_SetControlStatusReg(XPAR_AXI_TIMER_0_BASEADDR, 0, csr);
}

int main()
{
    init_platform();

	//Megszakításkezelõ rutinok regisztrálása
	XIntc_RegisterHandler(
			XPAR_MICROBLAZE_0_INTC_BASEADDR,                  	//INTC báziscíme
			XPAR_MICROBLAZE_0_INTC_AXI_TIMER_0_INTERRUPT_INTR,  //Megszakítás azonosító
			(XInterruptHandler)timer_int_handler,
			NULL
	);

	// megszakítások engedélyezése
	//A megszakítás vezérlõ konfigurálása.
	XIntc_MasterEnable(XPAR_MICROBLAZE_0_INTC_BASEADDR);
	XIntc_EnableIntr(XPAR_MICROBLAZE_0_INTC_BASEADDR,
			XPAR_AXI_TIMER_0_INTERRUPT_MASK
	);

	XTmrCtr_SetLoadReg(
			XPAR_AXI_TIMER_0_BASEADDR,
			0,
			XPAR_AXI_TIMER_0_CLOCK_FREQ_HZ /5 //ide kell kevesebb
	);

	XTmrCtr_SetControlStatusReg(
			XPAR_AXI_TIMER_0_BASEADDR,
			0,
			XTC_CSR_INT_OCCURED_MASK |
			XTC_CSR_LOAD_MASK
	);

	XTmrCtr_SetControlStatusReg(
			XPAR_AXI_TIMER_0_BASEADDR,0,
			XTC_CSR_ENABLE_TMR_MASK |
			XTC_CSR_ENABLE_INT_MASK |
			XTC_CSR_AUTO_RELOAD_MASK |
			XTC_CSR_DOWN_COUNT_MASK
	);

	//A megszakítások engedélyezése a processzoron.
	microblaze_enable_interrupts();


	uint16_t snake[MAPSIZE];



    LcdInit();
    initmap(snake);

	headpos_x = 51;
	headpos_y = 32;
	direction = RIGHT;
	direction_x = 1;
	direction_y = 0;
	snake_size = 1;
	uint8_t running = 1;

	snake[headpos_y*LCD_WIDTH + headpos_x] = 1;


    //startgame gomb
    //TODO rand init
	srand(154);
	uint8_t almacheck;

do {
		almacheck = almagen(snake);

	} while (almacheck);

	placealma(snake);

    LcdArrayConv(snake);

// jatek ciklus
    while (running){



    // gomb read
    switch(NavswR()){
    	case 0x01: 	if (activedirection_y != 1){
    						direction = UP;
    					    direction_x = 0;
    					    direction_y = -1;
    				} break;

    	case 0x02: 	if (activedirection_y != -1){
    	    				direction = DOWN;
    	    				direction_x = 0;
    	    				direction_y = 1;
    	    		} break;

    	case RIGHT: if (activedirection_x != -1){
    	    				direction = RIGHT;
    	    				direction_x = 1;
    	    				direction_y = 0;
    	    		} break;

    	case LEFT: 	if (activedirection_x != 1){
    	    				direction = LEFT;
    	    				direction_x = -1;
    	    				direction_y = 0;
    	    		} break;
    }

		if(update == 1){
		snakecheck(snake);

			if (!snake_grow){
				snakeupdate(snake);
			}

			else {
				do {
					almacheck = almagen(snake);

					} while (almacheck);

					placealma(snake);
			}

		LcdArrayConv(snake);
		update = 0;
		}
    }
    return 0;
}


void initmap(uint16_t * map){
	int i;
    for (i=0; i< (MAPSIZE);i++ ){
    	if (i < LCD_WIDTH || (i % LCD_WIDTH) == 0 || i > MAPSIZE - LCD_WIDTH || (i % (LCD_WIDTH)) == 101){
    		map[i] = 1;
    	}

    	else {
    		map[i] = 0;
    	}
    }
}

int8_t snakecheck(uint16_t * map){
	uint8_t ret=1;
	activedirection_x = direction_x;
	activedirection_y = direction_y;
	headpos_x = headpos_x + activedirection_x;
	headpos_y = headpos_y + activedirection_y;

	if (headpos_y == alma_y && headpos_x == alma_x){
		snake_grow = 1;
		snake_size += 1;
		ret = 0;
	}
	else {
		snake_grow = 0;
		ret = 0;
	}

	if (map[headpos_y*LCD_WIDTH + headpos_x] && snake_grow == 0){
		running = 0;
		ret = -1;
	}

return ret;
}

void snakeupdate(uint16_t * map){
	int i;
	for (i=0; i< (MAPSIZE);i++ ){
	    	if (!(i < LCD_WIDTH || (i % LCD_WIDTH) == 0 || i > MAPSIZE - LCD_WIDTH || (i % (LCD_WIDTH)) == 101)){
	    		if (i != alma_y*LCD_WIDTH + alma_x){
	    			if (map[i] != 0){
	    				map[i] -= 1;
	    			}
	    		}
	    	}
	}
	map[headpos_y*LCD_WIDTH + headpos_x] = snake_size;
}

uint8_t almagen(uint16_t * map){
	alma_x = 1 + (rand() % (LCD_WIDTH-1));
	alma_y = 1 + (rand() % (LCD_HEIGHT-1));

	if (map[alma_y*LCD_WIDTH + alma_x]){
		return 1;
	}
	else return 0;
}

void placealma(uint16_t * map){
	map[alma_y*LCD_WIDTH + alma_x] = snake_size;
}


