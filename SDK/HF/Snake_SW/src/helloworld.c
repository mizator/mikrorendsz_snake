/*
SNEK
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
#include "font/font.h"

#define MAP_WIDTH 34
#define MAP_HEIGHT 22
#define MAPSIZE MAP_WIDTH*MAP_HEIGHT
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

void drawframe(uint16_t width, uint16_t height, uint16_t * array);
void drawframe2(uint16_t width, uint16_t height, uint8_t * array);
void mapupdate(uint16_t * map);

int8_t snakecheck(uint16_t * map);
uint8_t almagen(uint16_t * map);
void placealma(uint16_t * map);
void maplcdconv(uint16_t width, uint16_t height, uint16_t * map,
				uint16_t lcdwidth, uint16_t lcdheight, uint8_t * framebuffer);

inline void drawpixel(uint16_t loc_x, uint16_t loc_y, uint16_t value, uint8_t * framebuffer);

void printchar(uint8_t row, uint8_t col, uint8_t * array, char ch);
void printstring(uint8_t row, uint8_t col, uint8_t * array, char * string);
void printnum(uint8_t row, uint8_t col, uint8_t * array, char * num);
void num2string(char num, char * string);

uint8_t setlevel(uint8_t input);



void timer_int_handler(void *instance_Ptr)
{
	if (update != 0){
	update--;
	}
	// clear interrupt flag
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

    uint8_t * framebuffer = NULL;
    framebuffer = malloc(LCD_SIZE * sizeof(uint8_t));

    if (framebuffer == NULL){
    	print ("not enough free memory\r\n");
    	return 0;
    }
    else print("framebuffer OK\r\n");

    uint16_t * snake = NULL;
    snake = malloc(MAPSIZE * sizeof(uint16_t));

    if (snake == NULL){
    	print ("not enough free memory\r\n");
    	return 0;
    }
    else print("map OK\r\n");

while(1){

	drawframe2(LCD_WIDTH, LCD_HEIGHT, framebuffer);
    drawframe(MAP_WIDTH, MAP_HEIGHT, snake);

	headpos_x = MAP_WIDTH / 2;
	headpos_y = MAP_HEIGHT / 2;
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
		snake[headpos_y*MAP_WIDTH + headpos_x - dummy] = snake_size - dummy;
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

	maplcdconv(MAP_WIDTH, MAP_HEIGHT, snake, LCD_WIDTH, LCD_HEIGHT, framebuffer);

	LcdArrayConv(framebuffer);

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



			if(update == 0 && midbutton == 0) {
				 if (snakecheck(snake) != 0) { //collision detected, game over
					running = 0;
					printstring(26, 24, framebuffer, "Game over!");
					uint8_t printpos = 33;
					char scorebuf[4];
					int16_t score = snake_size - SNAKESIZE_START;

					// print the score
					if (score > 999) {
						num2string(score / 1000 , &scorebuf[0]);
						score = score % 1000;
						num2string(score / 100 , &scorebuf[1]);
						score = score % 100;
						num2string(score / 10 , &scorebuf[2]);
						num2string(score % 10 , &scorebuf[3]);
						scorebuf[4] = 0;
						printpos = 23;

					}
					else if(score > 99) {
						num2string(score / 100 , &scorebuf[0]);
						score = score % 100;
						num2string(score / 10 , &scorebuf[1]);
						num2string(score % 10 , &scorebuf[2]);
						scorebuf[3] = 0;
						printpos = 26;
					}
					else if(score > 9){
						num2string(score / 10 , &scorebuf[0]);
						num2string(score % 10 , &scorebuf[1]);
						scorebuf[2] = 0;
						printpos = 29;
					}
					else {
						num2string(score, &scorebuf[0]);
						scorebuf[1] = 0;
						printpos = 32;
					}

					printstring(35, printpos, framebuffer, "Score:");
					printstring(35, printpos + 36, framebuffer, scorebuf);
					LcdArrayConv(framebuffer);
					while (NavswR() != PUSH );
					break;
				 }

					if (!snake_grow) {	//food not picked up
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

					maplcdconv(MAP_WIDTH, MAP_HEIGHT, snake, LCD_WIDTH, LCD_HEIGHT, framebuffer);
					LcdArrayConv(framebuffer);
					update = setlevel(level);	//set delay
			}
		}

	}
  return 0;
}


void drawframe(uint16_t width, uint16_t height, uint16_t * array){	// draw frame on map (invisible, only for collision detection)
	uint16_t i;

    for (i=0; i < width * height;i++ ){
    	if (i < width || (i % width) == 0 || i > width * height - width || (i % (width)) == width - 1){
    		array[i] = 0xffff; // frame pixels have 0xff value for easy recognition (no value decrement on map update)
    	}
    	else {
    		array[i] = 0;
    	}
    }
}

void drawframe2(uint16_t width, uint16_t height, uint8_t * array){	// draw frame on the LCD
	uint16_t i;
    for (i=0; i < width * height; i++ ){
    	if (i < 2*width - 1 || (i % width) < 3 || i > width * height - 2*width || (i % (width)) >= width - 3){
    		array[i] = 0xff;
    	}

    	else {
    		array[i] = 0;
    	}
    }
}


int8_t snakecheck(uint16_t * map){ // collision detection
	uint8_t ret=1;

	activedirection_x = direction_x;
	activedirection_y = direction_y;

	headpos_x = headpos_x + activedirection_x;
	headpos_y = headpos_y + activedirection_y;

	if (headpos_y == alma_y && headpos_x == alma_x){ 	// ate food
		snake_grow = 1;
		snake_size += 1;
		ret = 0;
	}
	else { 												// no collision
		snake_grow = 0;
		ret = 0;
	}

	if (map[headpos_y*MAP_WIDTH + headpos_x] && snake_grow == 0){	// collision detected
		running = 0;
		ret = -1;
	}

return ret;
}

void mapupdate(uint16_t * map){							// decrease all map element values by one (this makes the tail disappear)
	int i;
	for (i=0; i< (MAPSIZE);i++ ){
	    	if (map[i] != 0xffff){ 						// if not part of the edge
	    		if (i != alma_y*MAP_WIDTH + alma_x){ 	// if does not match the food position
	    			if (map[i] != 0){
	    				map[i] -= 1;
	    			}
	    		}
	    	}
	}
	map[headpos_y*MAP_WIDTH + headpos_x] = snake_size;	// give maximum value to the head
}

void maplcdconv(uint16_t width, uint16_t height, uint16_t * map,
				uint16_t lcdwidth, uint16_t lcdheight, uint8_t * framebuffer){ // convert the small map to the LCD resolution
	int x, y;
	for (y = 1; y < height - 1; y++){
		for (x = 1; x < width - 1; x++){
			//framebuffer[(y*3*lcdwidth + 1 + 3*x] = map[y*width + x]; // only draws the middle points
			drawpixel(1+3*x, y*3, map[y*width + x], framebuffer);
		}
	}
}

inline void drawpixel(uint16_t loc_x, uint16_t loc_y, uint16_t value, uint8_t * framebuffer){ // draw the other 9 pixels around the middle coordinate
	uint16_t x, y;
	for (y = loc_y - 1; y <= loc_y + 1; y++){
		for (x = loc_x - 1; x <= loc_x + 1; x++){
			framebuffer[y * LCD_WIDTH + x] = value;
		}
	}
}

uint8_t almagen(uint16_t * map){ // generate food
	alma_x = 1 + (rand() % (MAP_WIDTH-1));
	alma_y = 1 + (rand() % (MAP_HEIGHT-1));

	if (map[alma_y*MAP_WIDTH + alma_x]){
		return 1;
	}
	else return 0;
}

void placealma(uint16_t * map){ // place generated food
	map[alma_y*MAP_WIDTH + alma_x] = snake_size;
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

void printchar(uint8_t row, uint8_t col, uint8_t * array, char ch) { // print character to framebuffer
	unsigned char buf[5];
	uint8_t i, b;

	for (i = 0; i < 5; i++)	{
		buf[i] = font5x8[5*ch + i]; // read from character font array
	}

	for (i = 0; i < 5; i++)	{
		for (b = 0; b < 8; b++) {
			array[row*LCD_WIDTH + b*LCD_WIDTH + col + i] = (buf[i] & 1<<b); // copy character to framebuffer
		}
	}

	for (b = 0; b < 8; b++) {
		array[row*LCD_WIDTH + b*LCD_WIDTH + col + 5] = 0; // clear screen between characters (1 vertical line)
	}
}

void printstring(uint8_t row, uint8_t col, uint8_t * array, char * string) { // print string to framebuffer
	unsigned char buf;
	while(*string != 0) {
		buf = *string;
		printchar(row,col,array, buf - 32);
		col += 6; // 1 empty line between characters
		string++;
	}
}

void num2string(char num, char * string) {
	*string = num + 48 ;
}

void TimerInit(void){
	// register interrupt routines
	XIntc_RegisterHandler(
			XPAR_MICROBLAZE_0_INTC_BASEADDR,
			XPAR_MICROBLAZE_0_INTC_AXI_TIMER_0_INTERRUPT_INTR,  // timer interrupt
			(XInterruptHandler)timer_int_handler,
			NULL
	);

	// enable interrupts
	// configure the handler
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
