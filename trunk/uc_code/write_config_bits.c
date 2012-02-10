/**************************************************************************
 *   Copyright (C) 2008 by Frans Schreuder                                 *
 *   usbpicprog.sourceforge.net                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 **************************************************************************/

#include "upp.h"
#include "write_config_bits.h"
#ifdef SDCC
#include <pic18f2550.h>
#else
#include <p18cxxx.h>
#endif
#include "typedefs.h"
#include "io_cfg.h"             // I/O pin mapping
#include "prog.h"
#include "interrupt.h"
#include "prog_lolvl.h"

//FIXME: where is P18FXX31

/**
 address points to the first byte of the block
 data contains the data MSB0, LSB0, MSB1, LSB1, etc...
 blocksize is the block syze in BYTES
 */

/**
 program_config_bits writes 2 configuration bytes each time (*data has to be 2 bytes big)
 the address will be 0x300000 + the id location
 before calling this function, make configstate CONFIGSTART
 keep calling this function until configstate==CONFIGSUCCESS
 **/
//PICFAMILY picfamily;
//PICTYPE pictype;

char write_config_bits( PICFAMILY picfamily, PICTYPE pictype, unsigned long address, unsigned char* data,
		char blocksize, char lastblock )
{

	if( lastblock & BLOCKTYPE_FIRST )
		set_vdd_vpp( pictype, picfamily, 1 );
#ifdef TABLE
	if( currDevice.write_config_bits )
		currDevice.write_config_bits( address, data, blocksize, lastblock );
	else
		switch( pictype ) {
#else
		switch( pictype )
		{
			case dsP30F:
			write_config_bits_dsP30F( address, data, blocksize, lastblock );
			break;
			case P18F45J10:
			write_config_bits_P18F45J10( address, data, blocksize, lastblock);
			break;
			case P18F872X:
			case P18FX220:
			case P18FXX39:
			case P18F6X2X:
			case P18F2XXX:
			write_config_bits_P18F2XXX( address, data, blocksize, lastblock );
			break;

			case P18FXX2:
			write_config_bits_P18FXX2( address, data, blocksize, lastblock );
			break;
			case P18F4XK22:
			case P18LF4XK22:
			write_config_bits_P18F4XK22( address, data, blocksize, lastblock );
			break;
			//    case P18F6XKXX:		//FIXME: enable *just not in present
			//    case P18F67KXX:
			//	        write_config_bits_P18F6XKXX( address, data, blocksize, lastblock );
			//	        break;

			/*		case P12F629:
			 if(lastblock&1)
			 {
			 pic_send_14_bits(6,0x00,0x0000);//Execute a Load Configuration command (dataword 0x0000) to set PC to 0x2000.
			 for(i=0;i<((char)address);i++)pic_send_n_bits(6,0x06);   //increment address until ADDRESS is reached
			 }
			 for(blockcounter=0;blockcounter<blocksize;blockcounter+=2)
			 {
			 //load data for config memory
			 if(((((char)address)+(blockcounter>>1))<4))
			 {
			 pic_send_14_bits(6,0x02,(((unsigned int)data[blockcounter]))|   //MSB
			 (((unsigned int)data[blockcounter+1])<<8));//LSB
			 pic_send_n_bits(6,0x08);    //begin programming
			 DelayMs(Tprog);
			 }
			 if(((((char)address)+(blockcounter>>1))==7))      //restore bandgap bits
			 {
			 payload=bandgap|(0x0FFF&((((unsigned int)data[blockcounter+1])<<8)|   //MSB
			 (((unsigned int)data[blockcounter]))));
			 pic_send_14_bits(6,0x02,payload);
			 pic_send_n_bits(6,0x08);    //begin programming
			 DelayMs(Tprog);
			 }
			 //read data from program memory (to verify) not yet impl...
			 pic_send_n_bits(6,0x06);	//increment address
			 }
			 break;*/
			case P16F785:
			case P12F61X:
			write_config_bits_P16F785( address, data, blocksize, lastblock );
			break;
			case P16F87:
			write_config_bits_P16F87( address, data, blocksize, lastblock );
			break;
			case P16F716:
			write_config_bits_P16F716( address, data, blocksize, lastblock );
			break;
			case P16F72:
			case P16F7X:
			case P16F7X7:
			write_config_bits_P16F72( address, data, blocksize, lastblock );
			break;
			case P12F629:
			case P16F88X:
			case P16F87X:
			case P16F91X:
			case P16F81X:
			case P16F84A:
			case P16F87XA:
			case P12F6XX: //same as P16F62X
			case P16F62XA: //same as P16F62X
			case P16F62X:
			case P16F182X:
			write_config_bits_P16F62XA( address, data, blocksize, lastblock );
			break;
			case P12F508:
			case P16F54:
			case P16F57:
			case P16F59:
			case P10F200:
			case P10F202:
			write_config_bits_P16F54( address, data, blocksize, lastblock );
			break;
#endif
		default:
			set_vdd_vpp( pictype, picfamily, 0 );
			return 3;
			break;
		}
	if( lastblock & BLOCKTYPE_LAST )
	{
		set_vdd_vpp( pictype, picfamily, 0 );
		return 1; //ok
	}
	else
	{
		return 2; //ask for next block
	}
}

void write_config_bits_dsP30F( unsigned long address, unsigned char* data, char blocksize, char lastblock )
{
	static char blockcounter;
	unsigned int payload;
	if( lastblock & BLOCKTYPE_FIRST )
	{
		//Step 1: Exit the Reset vector.
		dspic_send_24_bits( 0x000000 ); //NOP
		dspic_send_24_bits( 0x000000 ); //NOP
		dspic_send_24_bits( 0x040100 ); //GOTO 0x100
		dspic_send_24_bits( 0x040100 ); //GOTO 0x100
		dspic_send_24_bits( 0x000000 ); //NOP
		//Step 2: Initialize the write pointer (W7) for the TBLWT instruction.

		dspic_send_24_bits( 0x200007 );//|((address&0xFFFF)<<4));	//MOV #0x0000, W7
	}
	for( blockcounter = 0; blockcounter < blocksize; blockcounter += 2 )
	{
		//Step 3: Set the NVMCON to program 1 Configuration register.
		dspic_send_24_bits( 0x24008A ); //MOV #0x4008, W10
		dspic_send_24_bits( 0x883B0A ); //MOV W10, NVMCON
		//Step 4: Initialize the TBLPAG register.
		//dspic_send_24_bits(0x200000|((blockcounter+address&0xFF0000)>>12));	//MOV #0xF8, W0
		dspic_send_24_bits( 0x200F80 );//|((blockcounter+address&0xFF0000)>>12));	//MOV #0xF8, W0
		dspic_send_24_bits( 0x880190 ); //MOV W0, TBLPAG
		//Step 5: Load the Configuration register data to W6.
		payload = (((unsigned int) data[blockcounter]) | (((unsigned int) data[blockcounter + 1]) << 8));
		dspic_send_24_bits( 0x200006 | ((((unsigned long) payload) & 0xFFFF) << 4) ); //MOV #<CONFIG_VALUE>, W6
		//dspic_send_24_bits(0x200036);	//write 0x0003 in the config register
		dspic_send_24_bits( 0x000000 ); //NOP
		//Step 6: Write the Configuration register data to the write latch and increment the write pointer.
		dspic_send_24_bits( 0xBB1B86 ); //TBLWTL W6, [W7++]
		dspic_send_24_bits( 0x000000 ); //NOP
		dspic_send_24_bits( 0x000000 ); //NOP
		//Step 7: Unlock the NVMCON for programming.
		dspic_send_24_bits( 0x200558 ); //MOV #0x55, W8
		dspic_send_24_bits( 0x883B38 ); //MOV W8, NVMKEY
		dspic_send_24_bits( 0x200AA9 ); //MOV #0xAA, W9
		dspic_send_24_bits( 0x883B39 ); //MOV W9, NVMKEY
		//Step 8: Initiate the write cycle.
		dspic_send_24_bits( 0xA8E761 ); //BSET NVMCON, #WR
		dspic_send_24_bits( 0x000000 ); //NOP
		dspic_send_24_bits( 0x000000 ); //NOP
		DelayMs( 2 ); //Externally time 2 msec
		dspic_send_24_bits( 0x000000 ); //NOP
		dspic_send_24_bits( 0x000000 ); //NOP
		dspic_send_24_bits( 0xA9E761 ); //BCLR NVMCON, #WR
		dspic_send_24_bits( 0x000000 ); //NOP
		dspic_send_24_bits( 0x000000 ); //NOP
		//Step 9: Reset device internal PC.
		dspic_send_24_bits( 0x040100 ); //GOTO 0x100
		dspic_send_24_bits( 0x000000 ); //NOP
	}//Step 10: Repeat steps 3-9 until all 7 Configuration registers are cleared.
}
void write_config_bits_P18F2XXX( unsigned long address, unsigned char* data, char blocksize, char lastblock )
{
	static char blockcounter;

	pic_send( 4, 0x00, 0x8EA6 ); //BSF EECON1, EEPGD
	pic_send( 4, 0x00, 0x8CA6 ); //BSF EECON1, CFGS
	//address=0x300000;
	//start for loop
	for( blockcounter = 0; blockcounter < blocksize; blockcounter += 2 )
	{
		set_address( picfamily, address + ((unsigned int) blockcounter) );
		//LSB first
		pic_send( 4, 0x0F, ((unsigned int) *(data + blockcounter)) | (((unsigned int) *(data + blockcounter))
				<< 8) );
		pic_send_n_bits( 3, 0 );
		PGChigh(); //hold PGC high for P9 (or P9A for 4XF/LFK22 config word)
		DelayMs( P9 );
		PGClow(); //hold PGC low for time P10
		DelayMs( P10 );
		pic_send_word( 0x0000 ); //last part of the nop
		set_address( picfamily, address + ((unsigned int) blockcounter) + 1 );
		pic_send( 4, 0x0F, ((unsigned int) *(data + 1 + blockcounter)) | (((unsigned int) *(data + 1
				+ blockcounter)) << 8) ); //load MSB and start programming
		pic_send_n_bits( 3, 0 );
		PGChigh(); //hold PGC high for P9 (or P9A for 4XF/LFK22 config word)
		DelayMs( P9 );
		PGClow(); //hold PGC low for time P10
		DelayMs( P10 );
		pic_send_word( 0x0000 ); //last part of the nop
	}
}
void write_config_bits_P18FXX2( unsigned long address, unsigned char* data, char blocksize, char lastblock )
{
	static char blockcounter;

	pic_send( 4, 0x00, 0x8EA6 ); //BSF EECON1, EEPGD
	pic_send( 4, 0x00, 0x8CA6 ); //BSF EECON1, CFGS
	//goto 0x100000
	pic_send( 4, 0x00, 0xEF00 );
	pic_send( 4, 0x00, 0xF800 );
	//address=0x300000;
	//start for loop
	for( blockcounter = 0; blockcounter < blocksize; blockcounter += 2 )
	{
		set_address( picfamily, address + ((unsigned int) blockcounter) );
		//LSB first
		pic_send( 4, 0x0F, ((unsigned int) *(data + blockcounter)) | (((unsigned int) *(data + blockcounter))
				<< 8) );
		pic_send_n_bits( 3, 0 );
		PGChigh(); //hold PGC high for P9 (or P9A for 4XF/LFK22 config word)
		DelayMs( P9 );
		PGClow(); //hold PGC low for time P10
		DelayMs( P10 );
		pic_send_word( 0x0000 ); //last part of the nop
		//        pic_send( 4, 0x00, 0x2AF6 ); //incf tblptr			//FIXME: this is deleted in current version???
		set_address( picfamily, address + ((unsigned int) blockcounter) + 1 );
		pic_send( 4, 0x0F, ((unsigned int) *(data + 1 + blockcounter)) | (((unsigned int) *(data + 1
				+ blockcounter)) << 8) ); //load MSB and start programming
		pic_send_n_bits( 3, 0 );
		PGChigh(); //hold PGC high for P9 (or P9A for 4XF/LFK22 config word)
		DelayMs( P9 );
		PGClow(); //hold PGC low for time P10
		DelayMs( P10 );
		pic_send_word( 0x0000 ); //last part of the nop
	}
}
void write_config_bits_P18F4XK22( unsigned long address, unsigned char* data, char blocksize, char lastblock )
{
	static char blockcounter;

	pic_send( 4, 0x00, 0x8EA6 ); //BSF EECON1, EEPGD
	pic_send( 4, 0x00, 0x8CA6 ); //BSF EECON1, CFGS
	pic_send( 4, 0x00, 0x84A6 ); //BSF EECON1, WREN
	//address=0x300000;
	//start for loop
	for( blockcounter = 0; blockcounter < blocksize; blockcounter += 2 )
	{
		set_address( picfamily, address + ((unsigned int) blockcounter) );
		//LSB first
		pic_send( 4, 0x0F, ((unsigned int) *(data + blockcounter)) | (((unsigned int) *(data + blockcounter))
				<< 8) );
		pic_send_n_bits( 3, 0 );
		PGChigh(); //hold PGC high for P9 (or P9A for 4XF/LFK22 config word)
		DelayMs( P9A );
		PGClow(); //hold PGC low for time P10
		DelayMs( P10 );
		pic_send_word( 0x0000 ); //last part of the nop
		set_address( picfamily, address + ((unsigned int) blockcounter) + 1 );
		pic_send( 4, 0x0F, ((unsigned int) *(data + 1 + blockcounter)) | (((unsigned int) *(data + 1
				+ blockcounter)) << 8) ); //load MSB and start programming
		pic_send_n_bits( 3, 0 );
		PGChigh(); //hold PGC high for P9 (or P9A for 4XF/LFK22 config word)
		DelayMs( P9A );
		PGClow(); //hold PGC low for time P10
		DelayMs( P10 );
		pic_send_word( 0x0000 ); //last part of the nop
	}
}
void write_config_bits_P18F6XKXX( unsigned long address, unsigned char* data, char blocksize, char lastblock )
{
	static char blockcounter;

	pic_send( 4, 0x00, 0x8E7F ); //BSF EECON1, EEPGD
	pic_send( 4, 0x00, 0x8C7F ); //BSF EECON1, CFGS
	//address=0x300000;
	//start for loop
	for( blockcounter = 0; blockcounter < blocksize; blockcounter += 2 )
	{
		set_address( picfamily, address + ((unsigned int) blockcounter) );
		//LSB first
		pic_send( 4, 0x0F, ((unsigned int) *(data + blockcounter)) | (((unsigned int) *(data + blockcounter))
				<< 8) );
		pic_send_n_bits( 3, 0 );
		PGChigh(); //hold PGC high for P9 (or P9A for 4XF/LFK22 config word)
		DelayMs( P9 );
		PGClow(); //hold PGC low for time P10
		DelayMs( P10 );
		pic_send_word( 0x0000 ); //last part of the nop
		set_address( picfamily, address + ((unsigned int) blockcounter) + 1 );
		pic_send( 4, 0x0F, ((unsigned int) *(data + 1 + blockcounter)) | (((unsigned int) *(data + 1
				+ blockcounter)) << 8) ); //load MSB and start programming
		pic_send_n_bits( 3, 0 );
		PGChigh(); //hold PGC high for P9 (or P9A for 4XF/LFK22 config word)
		DelayMs( P9 );
		PGClow(); //hold PGC low for time P10
		DelayMs( P10 );
		pic_send_word( 0x0000 ); //last part of the nop
	}
}
void write_config_bits_P18F45J10( unsigned long address, unsigned char* data, char blocksize, char lastblock)
{
	write_code_P18F45J10(address,data,blocksize,lastblock);
}
void write_config_bits_P16F785( unsigned long address, unsigned char* data, char blocksize, char lastblock )
{
	char i;
	static char blockcounter;
	unsigned int payload;
	if( lastblock & BLOCKTYPE_FIRST )
	{
		pic_send_14_bits( 6, 0x00, 0x0000 );//Execute a Load Configuration command (dataword 0x0000) to set PC to 0x2000.
		for( i = 0; i < ((char) address); i++ )
			pic_send_n_bits( 6, 0x06 ); //increment address until ADDRESS is reached
	}
	for( blockcounter = 0; blockcounter < blocksize; blockcounter += 2 )
	{
		//load data for config memory
		if( ((((char) address) + (blockcounter >> 1)) < 4) || ((((char) address) + (blockcounter >> 1)) == 7)
				|| (((((char) address) + (blockcounter >> 1)) == 8) ) )
		{
			payload = (((unsigned int) data[blockcounter]))
					| (((unsigned int) data[blockcounter + 1]) << 8);
			pic_send_14_bits( 6, 0x02, payload ); //load data for programming
			pic_send_n_bits( 6, 0x18 ); //begin programming
			pic_send_n_bits( 6, 0x08 ); //begin programming		//FIXME: bug in original
			DelayMs( Tprog );
			pic_send_n_bits( 6, 0x0A ); //end programming
		}
		//read data from program memory (to verify) not yet impl...
		pic_send_n_bits( 6, 0x06 ); //increment address
	}
}
void write_config_bits_P16F87( unsigned long address, unsigned char* data, char blocksize, char lastblock )
{
	char i;
	static char blockcounter;
	unsigned int payload;
	if( lastblock & BLOCKTYPE_FIRST )
	{
		pic_send_14_bits( 6, 0x00, 0x0000 );//Execute a Load Configuration command (dataword 0x0000) to set PC to 0x2000.
		for( i = 0; i < ((char) address); i++ )
			pic_send_n_bits( 6, 0x06 ); //increment address until ADDRESS is reached
	}
	for( blockcounter = 0; blockcounter < blocksize; blockcounter += 2 )
	{
		//load data for config memory
		if( ((((char) address) + (blockcounter >> 1)) < 4) || ((((char) address) + (blockcounter >> 1)) == 7)
				|| (((((char) address) + (blockcounter >> 1)) == 8) ) )
		{
			payload = (((unsigned int) data[blockcounter]))
					| (((unsigned int) data[blockcounter + 1]) << 8);
			pic_send_14_bits( 6, 0x02, payload ); //load data for programming
			pic_send_n_bits( 6, 0x18 ); //begin programming
			pic_send_n_bits( 6, 0x08 ); //begin programming		//FIXME: bug in original
			DelayMs( Tprog );
			pic_send_n_bits( 6, 0x17 ); //end programming
		}
		//read data from program memory (to verify) not yet impl...
		pic_send_n_bits( 6, 0x06 ); //increment address
	}
}
void write_config_bits_P16F716( unsigned long address, unsigned char* data, char blocksize, char lastblock )
{
	char i;
	static char blockcounter;
	unsigned int payload;
	if( lastblock & BLOCKTYPE_FIRST )
	{
		pic_send_14_bits( 6, 0x00, 0x0000 );//Execute a Load Configuration command (dataword 0x0000) to set PC to 0x2000.
		for( i = 0; i < ((char) address); i++ )
			pic_send_n_bits( 6, 0x06 ); //increment address until ADDRESS is reached
	}
	for( blockcounter = 0; blockcounter < blocksize; blockcounter += 2 )
	{
		//load data for config memory
		if( ((((char) address) + (blockcounter >> 1)) < 4) || ((((char) address) + (blockcounter >> 1)) == 7)
				|| (((((char) address) + (blockcounter >> 1)) == 8) ) )
		{
			payload = (((unsigned int) data[blockcounter]))
					| (((unsigned int) data[blockcounter + 1]) << 8);
			pic_send_14_bits( 6, 0x02, payload ); //load data for programming
			pic_send_n_bits( 6, 0x18 ); //begin programming
			pic_send_n_bits( 6, 0x08 ); //begin programming		//FIXME: bug in original
			DelayMs( Tprog );
			pic_send_n_bits( 6, 0x0E ); //end programming
		}
		//read data from program memory (to verify) not yet impl...
		pic_send_n_bits( 6, 0x06 ); //increment address
	}
}

void write_config_bits_P16F72( unsigned long address, unsigned char* data, char blocksize, char lastblock )
{
	char i;
	static char blockcounter;
	unsigned int payload;
	if( lastblock & BLOCKTYPE_FIRST )
	{
		pic_send_14_bits( 6, 0x00, 0x0000 );//Execute a Load Configuration command (dataword 0x0000) to set PC to 0x2000.
		for( i = 0; i < ((char) address); i++ )
			pic_send_n_bits( 6, 0x06 ); //increment address until ADDRESS is reached
	}
	for( blockcounter = 0; blockcounter < blocksize; blockcounter += 2 )
	{
		//load data for config memory
		if( ((((char) address) + (blockcounter >> 1)) < 4) || ((((char) address) + (blockcounter >> 1)) == 7) )
		{
			payload = (((unsigned int) data[blockcounter]))
					| (((unsigned int) data[blockcounter + 1]) << 8);
			pic_send_14_bits( 6, 0x02, payload ); //load data for programming
			pic_send_n_bits( 6, 0x08 ); //begin programming
			DelayMs( Tprog );
			pic_send_n_bits( 6, 0x0E ); //end programming
		}
		//read data from program memory (to verify) not yet impl...
		pic_send_n_bits( 6, 0x06 ); //increment address
	}
}
void write_config_bits_P16F62XA( unsigned long address, unsigned char* data, char blocksize, char lastblock )
{
	char i;
	static char blockcounter;
	unsigned int payload;
	if( lastblock & BLOCKTYPE_FIRST )
	{
		pic_send_14_bits( 6, 0x00, 0x0000 );//Execute a Load Configuration command (dataword 0x0000) to set PC to 0x2000.
		for( i = 0; i < ((char) address); i++ )
			pic_send_n_bits( 6, 0x06 ); //increment address until ADDRESS is reached
	}
	for( blockcounter = 0; blockcounter < blocksize; blockcounter += 2 )
	{
		//load data for config memory
		if( ((((char) address) + (blockcounter >> 1)) < 4) || ((((char) address) + (blockcounter >> 1)) == 7) )
		{
			payload = (((unsigned int) data[blockcounter]))
					| (((unsigned int) data[blockcounter + 1]) << 8);
			pic_send_14_bits( 6, 0x02, payload ); //load data for programming
			pic_send_n_bits( 6, 0x08 ); //begin programming
			DelayMs( Tprog );
		}
		//read data from program memory (to verify) not yet impl...
		pic_send_n_bits( 6, 0x06 ); //increment address
	}
}
void write_config_bits_P16F54( unsigned long address, unsigned char* data, char blocksize, char lastblock )
{
	static char blockcounter;
	unsigned int payload;

	for( blockcounter = 0; blockcounter < blocksize; blockcounter += 2 )
	{
		payload = (((unsigned int) data[blockcounter])) | (((unsigned int) data[blockcounter + 1]) << 8);
		pic_send_14_bits( 6, 0x02, payload ); //load data for programming
		pic_send_n_bits( 6, 0x08 ); //begin programming
		DelayMs( Tprog );
		pic_send_n_bits( 6, 0x0E ); //end programming
		pic_send_n_bits( 6, 0x06 ); //increment address
	}
}
