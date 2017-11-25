`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company:         Mikrorendszerek Tervezése
// Engineer: 		Attila Gránicz
// 
// Create Date:     2017-10-28 
// Design Name:     LCD
// Module Name:     lcd_spi
// Project Name:    Snake
// Target Devices:  EADOGS102N-6   LCD
// Description: 
// Additional Comments: 
//	Prefixes:
//		i_ : input
//		o_ : output
//		r_ : register
//		w_ : wire
//
//////////////////////////////////////////////////////////////////////////////////
module sckgen(
	input 			i_sysclk, 		// System Clock
	input 			i_sysrst, 		// System Reset
	input 			i_en, 			// Enable Clock Generator
	input 	[7:0] 	i_baudrate, 	// Baudrate Divider
	output 			o_sck, 			// SPI SCK
	output 			o_sck_rise, 	// SCK Rising Edge
	output 			o_sck_fall 		// SCK Falling Edge
);

//---------------------------------------------
// Counter
//---------------------------------------------
reg [7:0] r_cntr;
always @ (posedge i_sysclk)
begin
	if (i_sysrst)
		r_cntr <= 8'b0;
	else if(i_en) begin
		if(r_cntr == i_baudrate)
			r_cntr <= 8'b0;
		else
			r_cntr <= r_cntr + 1'b1;
	end
	else 
		r_cntr <= 8'b0;
end
//---------------------------------------------

//---------------------------------------------
// SCK Register
//---------------------------------------------
reg r_sck;
always @ (posedge i_sysclk)
begin
	if (i_sysrst)
		r_sck <= 1'b0;
	else if(~i_en)
		r_sck <= 1'b0;
	else if(r_cntr == i_baudrate)
		r_sck <= ~r_sck;
end
//---------------------------------------------

//---------------------------------------------
// Output Signal Generation		
//---------------------------------------------
assign o_sck 		= ( r_sck) & (i_en);
assign o_sck_rise 	= (~r_sck) & (r_cntr == i_baudrate) & (i_en);
assign o_sck_fall 	= ( r_sck) & (r_cntr == i_baudrate) & (i_en);
//---------------------------------------------

endmodule
