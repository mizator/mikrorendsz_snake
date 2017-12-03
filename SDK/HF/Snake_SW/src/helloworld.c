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
#include "io_lib/io_lib.h"
//#include <dogm-graphic.h>

#define TRUE 1
#define FALSE 0

volatile uint8_t counter = 0;

void timer_int_handler(void *instance_Ptr)
{
	if (counter < 99){
	counter++;
	}
	else {
		counter = 0;
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
			XPAR_AXI_TIMER_0_CLOCK_FREQ_HZ / 4
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

    DispW(54);

    NavswR();






    LcdInit();


    uint8_t snake[LCD_HEIGHT*LCD_WIDTH];

    int i;

    for (i=0; i< (LCD_HEIGHT*LCD_WIDTH);i++ ){
    	snake[i] = 1;
    }

    snake[6400]=0;

uint8_t snakemizu[LCD_WIDTH*LCD_PAGENUM];

    LcdArrayConv(snake, snakemizu);
    LcdArrayOut(snakemizu);


   /* uint8_t snake2[LCD_HEIGHT*LCD_WIDTH];

    int i;

    for (i=0; i< (8*LCD_WIDTH);i++ ){
    	snake2[i] = 0;
    }

    for (i=0; i<LCD_WIDTH; i++){
        snake2[3*i]=1;
    }



    LcdArrayOut(snake2);*/

    while(1) DispW(counter);

    while(1);
    return 0;
}


