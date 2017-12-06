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

#define MAPSIZE LCD_WIDTH*LCD_HEIGHT
#define SNAKESIZE_START 10
#define LVLTHR 5

volatile uint8_t counter = 0;
volatile uint8_t update = 0;

int8_t headpos_x = 0;
int8_t headpos_y = 0;
int8_t direction = RIGHT;
int8_t direction_x = 0;
int8_t direction_y = 0;
int8_t activedirection_x = 0;
int8_t activedirection_y = 0;
int16_t snake_size = 0;
uint8_t running = 0;
uint8_t alma_x;
uint8_t alma_y;
uint8_t snake_grow = 0;

void TimerInit(void);

void initmap(uint16_t * map);
void mapupdate(uint16_t * map);

int8_t snakecheck(uint16_t * map);
uint8_t almagen(uint16_t * map);
void placealma(uint16_t * map);

uint8_t setlevel(uint8_t input);



void timer_int_handler(void *instance_Ptr)
{
	if (update != 0){
	update--;
	}
	// a megszakítás jelzõbit törlése
	unsigned long csr;
	csr = XTmrCtr_GetControlStatusReg(XPAR_AXI_TIMER_0_BASEADDR, 0);
	XTmrCtr_SetControlStatusReg(XPAR_AXI_TIMER_0_BASEADDR, 0, csr);
}

int main()
{

    init_platform();
    TimerInit();

	microblaze_enable_interrupts();

    LcdInit();

    //map array

    uint16_t * snake = NULL;
    snake = malloc(MAPSIZE * sizeof(uint16_t));

    if (snake == NULL){
    	print ("elcseszett\r\n");
    	return 0;
    }
    else print("okeka\r\n");


while(1){

    initmap(snake);

	headpos_x = 51;
	headpos_y = 32;
	direction_x = 1;
	direction_y = 0;
	snake_size = SNAKESIZE_START;
	uint8_t running = 1;
	uint8_t midbutton = 0;
	uint8_t level;

	int64_t dummy64 = 0;

	uint8_t dummy;

	// initial snake
	for (dummy = 0; dummy < snake_size; dummy++){
		snake[headpos_y*LCD_WIDTH + headpos_x - dummy] = snake_size - dummy;
	}

	while(NavswR() != PUSH); // wait for buttonpress

	dummy64 = XTmrCtr_GetTimerCounterReg(XPAR_AXI_TIMER_0_BASEADDR,0);
	srand(dummy64); // random init

	level = DipswR();
	update = setlevel(level);

	// place food
	uint8_t almacheck;

		do {
		almacheck = almagen(snake);
		} while (almacheck);

		placealma(snake);

		LcdArrayConv(snake);

	// game cycle

		while (running){

		// btn read
		switch(NavswR()){
			case 0x01: 	if (activedirection_y != 1){
								direction_x = 0;
								direction_y = -1;
						} break;

			case 0x02: 	if (activedirection_y != -1){
								direction_x = 0;
								direction_y = 1;
						} break;

			case RIGHT: if (activedirection_x != -1){
								direction_x = 1;
								direction_y = 0;
						} break;

			case LEFT: 	if (activedirection_x != 1){
								direction_x = -1;
								direction_y = 0;
						} break;

			case PUSH: 	midbutton = 1; break;
		}



		DispW(snake_size - SNAKESIZE_START);
		LedW(level);

		if (midbutton){
			update = 4;
			while(update);

			while (NavswR() != PUSH );

			update = 4;
			while(update);

			midbutton = 0;
			level = DipswR();
			update = 0;
		}



			if(update == 0 && midbutton == 0){

				 if (snakecheck(snake) != 0){ //collision detection
					 //TODO pontszám kiírás
					 //TODO szöveg

					 running = 0;

				 }

					if (!snake_grow){	//food not picked up
						mapupdate(snake);
					}

					else { //food picked up
						do {
							almacheck = almagen(snake);

							} while (almacheck);

							placealma(snake);

							if ((snake_size - SNAKESIZE_START) % LVLTHR == 0){ //increase level after LVLTHR points
								if (level < 8) {
									level++;
								}
							}
					}

				LcdArrayConv(snake); //write lcd
				update = setlevel(level); //set delay
			}
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

void mapupdate(uint16_t * map){
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

uint8_t setlevel(uint8_t input){
	uint8_t ret = 0;
	switch(input){
		case 0: ret = 10;	break;
		case 1: ret = 8;	break;
		case 3: ret = 7;	break;
		case 4: ret = 6;	break;
		case 5: ret = 4;	break;
		case 6: ret = 2;	break;
		case 7: ret = 1;	break;
		case 8: ret = 0;	break;
	}

	return ret;
}

void TimerInit(void){
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
			XPAR_AXI_TIMER_0_CLOCK_FREQ_HZ / 20 - 2 //50 ms interrupt
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
}
