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
`uselib lib=unisims_ver
`uselib lib=proc_common_v3_00_a

module user_logic#(
	parameter C_NUM_REG                      = 2,		//Az IPIF által dekódolt 32 bites regiszterek száma.
   	parameter C_SLV_DWIDTH                   = 32		//Az adatbusz szélessége bitekben.
)(
	// General Input Signals
	input  wire                      Bus2IP_Clk,		//Órajel.
   	input  wire                      Bus2IP_Resetn,		//Aktív alacsony reset jel.
   	input  wire [C_SLV_DWIDTH-1:0]   Bus2IP_Data,		//Írási adatbusz.
   	input  wire [C_SLV_DWIDTH/8-1:0] Bus2IP_BE,			//Bájt engedélyezõ jelek (csak írás esetén érvényesek).
   	input  wire [C_NUM_REG-1:0]      Bus2IP_RdCE,		//A regiszterek olvasás engedélyezõ jelei.
   	input  wire [C_NUM_REG-1:0]      Bus2IP_WrCE,		//A regiszterek írás engedélyezõ jelei.
   	output reg [C_SLV_DWIDTH-1:0]   IP2Bus_Data,		//Olvasási adatbusz.
   	output wire                      IP2Bus_RdAck,		//Az olvasási mûveletek nyugtázó jele.
   	output wire                      IP2Bus_WrAck,		//Az írási mûveletek nyugtázó jele.
   	output wire                      IP2Bus_Error,		//Hibajelzés.

	output wire			o_irq,		// Interrupt Request

	// SPI Signals
  	output wire 		o_spi_miso,	// Master-In-Slave-Out / LCD CMD/DATA
  	output wire			o_spi_mosi,	// Master-Out-Slave-In
  	output wire			o_spi_sck,	// Bus-Clock
  	output wire			o_spi_ss_n	// Slave-Select
);

//---------------------------------------------
// Net Alias
//---------------------------------------------
wire 		i_sysclk =  Bus2IP_Clk;		// System Clock
wire 		i_sysrst = ~Bus2IP_Resetn;	// System Reset
wire [31:0]	i_din 	 =  Bus2IP_Data; 	// Data from Bus
wire [3:0]	i_byte_en=  Bus2IP_BE;
wire 		i_cmd_rd =  Bus2IP_RdCE[1];
wire 		i_cmd_wr =  Bus2IP_WrCE[1];
wire 		i_dat_rd =  Bus2IP_RdCE[0];
wire 		i_dat_wr =  Bus2IP_WrCE[0];

wire 		o_ack_wr;
wire 		o_ack_rd;

wire [31:0]	o_dout; 	// Data to Bus
wire		o_ack; 		// Acknowledge

//---------------------------------------------
// olvasási multiplexer
//---------------------------------------------
reg [31:0] rd_mux;
always @(*)
begin
	case(Bus2IP_RdCE)
		2'b10: IP2Bus_Data <= {20'b0, r_SPICR};
		2'b01: IP2Bus_Data <= {20'b0, r_SPISR};
		default: IP2Bus_Data <= 0;
	endcase
end

//Az IPIF felé menõ jelek meghajtása.
//assign o_dout = rd_mux;
assign o_ack_wr = ( r_ack_wr_c | r_ack_wr_d);
assign o_ack_rd = ( r_ack_rd_c | r_ack_rd_d);

//assign IP2Bus_Data = o_dout;
assign IP2Bus_Error = 1'b0;
//assign IP2Bus_WrAck = o_ack_wr;
//assign IP2Bus_RdAck = o_ack_rd;

assign IP2Bus_WrAck = |Bus2IP_WrCE;
assign IP2Bus_RdAck = |Bus2IP_RdCE;



//---------------------------------------------
// Registers for SPI
//---------------------------------------------
//	SPI CONTROL REGISTER 		
//	[ SS_reg [11] | Global EN [10] | Interrupt EN [9] | Interrupt Clear [8] | Baudrate [7:0] ]
reg [11:0] r_SPICR;
//---------------------------------------------
//	SPI STATUS REGISTER 		
// [ IRQreg [10] | BUSY reg [9] | LCD CMDn/DATA [8] | Data [7:0] ]
reg [11:0]  r_SPISR;		
//---------------------------------------------

//---------------------------------------------
// Control Register Value Settings
//---------------------------------------------
reg r_ack_wr_c;
reg r_ack_rd_c;
always @ (posedge i_sysclk)
begin
	if(i_sysrst) begin
		r_SPICR <= 12'b0;
		r_ack_wr_c <= 1'b0;
		r_ack_rd_c <= 1'b0;
		r_ack_rd_d <= 1'b0;
	end
	else if(i_cmd_wr && (i_byte_en == 4'b1111)) begin
		r_SPICR    <= i_din[11:0];
		r_ack_wr_c <= 1'b1;
		r_ack_rd_c <= 1'b0;
		r_ack_rd_d <= 1'b0;
	end
	else if(i_cmd_rd) begin
		r_ack_wr_c <= 1'b0; 
		r_ack_rd_c <= 1'b1;
		r_ack_rd_d <= 1'b0;
	end
	else if (i_dat_rd) begin
		r_ack_wr_c <= 1'b0; 
		r_ack_rd_c <= 1'b0;
		r_ack_rd_d <= 1'b1; 
	end
	else begin
		r_ack_wr_c <= 1'b0;
		r_ack_rd_c <= 1'b0;
		r_ack_rd_d <= 1'b0; 
		if (~r_SPISR[10])
			r_SPICR[8] <= 1'b0;	
	end
end





//---------------------------------------------
// State Machine
//---------------------------------------------
// 0 	- Idle
// 1 	- Start - SS Down, SCK Enable
// 2:9 	- Transmission
// 10 	- Stop  - SS Up, SCK Disable
//---------------------------------------------
reg [3:0] r_state = 0;
reg 	  r_ack_wr_d;
reg 	  r_ack_rd_d;
reg 	  r_sck_en;
always @ (posedge i_sysclk)
begin
 if(i_sysrst) begin
	r_state <= 4'b0;
	r_ack_wr_d <= 0;
	r_sck_en <= 0;
	r_SPISR <= 12'b0;						// Status Register Clear
 end
 else begin
 	if (r_state == 4'h0) begin 				// Idle
	 	if (i_dat_wr && (r_SPICR[10]) 
	 		&& (i_byte_en == 4'b1111)) 
	 	begin
	 		r_state <= 4'h1;
	 		r_sck_en <= 0;
			r_SPISR[9] <= 1; 				// BUSY Flag Write
			r_SPISR[8] <= i_din[8];			// LCD CMDn/DATA
			r_SPISR[7:0] <= i_din[7:0]; 	// Data from cpu bus
			r_ack_wr_d <= 1;				// Acknowledge
		end
	end
	else if  (r_state == 4'h1)  begin 		// Start
		r_state <= 4'h2;
		r_ack_wr_d <= 0;
		r_sck_en <= 1;
	end
	else if ((r_state >= 4'h2) && 			// Transmission
			((r_state <= 4'h9))) 
	begin 
		if (w_spi_sck_fall) begin 			// State Change on Falling Edge
			r_state <= r_state + 1;
		end
	end
	else if  (r_state == 4'hA)  begin 		// Stop
		r_state <= 4'h0;
		if (r_SPICR[9] == 1) begin 			// If Interrupt Enable Set
			r_SPISR[10] <= 1; 				// Then Interrupt Request
		end
		r_SPISR[9] <= 0; 					// BUSY Flag Clear
		r_SPISR[7:0] <= spi_shr_dout;
		r_sck_en <= 0;
	end
	if (r_SPICR[8])
		r_SPISR[10] <= 1'b0;
 end
end

assign o_spi_miso = r_SPISR[8];

//---------------------------------------------
// Output Signal Generation		
//---------------------------------------------
assign o_irq = r_SPISR[10];
assign o_spi_ss_n = (~r_SPICR[11]);
assign o_spi_sck = (w_sck & r_sck_en);
//---------------------------------------------
// Internal Signal Generation		
//---------------------------------------------
wire w_sck_en;
assign w_sck_en = (r_sck_en && r_SPICR[11]);
assign w_shr_ld = (r_state == 4'h1);
assign w_shr_sh = ((r_state >= 4'h2) && (r_state <= 4'hA) && w_spi_sck_fall && (~o_spi_ss_n));
//---------------------------------------------

//---------------------------------------------
// SCK frequency divider module instantiation
//---------------------------------------------
wire w_spi_sck_rise, w_spi_sck_fall, w_sck;
sckgen spi_sckgen (
	.i_sysclk(i_sysclk),
	.i_sysrst(i_sysrst),
	.i_en(w_sck_en),
	.i_baudrate(r_SPICR[7:0]),
	.o_sck(w_sck),
	.o_sck_rise(w_spi_sck_rise),
	.o_sck_fall(w_spi_sck_fall)
);
//---------------------------------------------

//---------------------------------------------
// SPI shift register module instantiation
//---------------------------------------------
wire [7:0] spi_shr_dout;
shr spi_shr (
	.i_sysclk(i_sysclk),
	.i_sysrst(i_sysrst),
	.i_din(o_spi_miso),
	.i_sh(w_shr_sh),
	.i_ld(w_shr_ld),
	.i_ld_data(r_SPISR[7:0]),
	.o_dout(o_spi_mosi),
	.o_dstr(spi_shr_dout)
);
//---------------------------------------------



//---------------------------------------------
endmodule
