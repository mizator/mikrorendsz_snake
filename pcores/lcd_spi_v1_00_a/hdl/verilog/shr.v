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
module shr(
	input 			i_sysclk,
	input 			i_sysrst,

	input 			i_din, 		// Data In
	input 			i_sh, 		// Shift Signal
	input 			i_ld,		// Load Signal
	input 	[7:0] 	i_ld_data, 	// Data to Load

	output 			o_dout, 	// Data Out
	output 	[7:0] 	o_dstr 		// Data to Store
);

//---------------------------------------------
// Shift-Register Block
//---------------------------------------------
reg [7:0] r_shr;
always @ (posedge i_sysclk)
begin
	if(i_sysrst)
		r_shr <= 8'b0;
	else if(i_ld)
		r_shr <= i_ld_data;
	else if(i_sh)
		r_shr <= {r_shr[6:0], i_din};
end
//---------------------------------------------

//---------------------------------------------
// Output Signal Generation
//---------------------------------------------
assign o_dstr = r_shr;
assign o_dout = r_shr[7];
//---------------------------------------------
endmodule
