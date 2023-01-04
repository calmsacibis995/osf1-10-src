/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/*
** File: 
**
**   libpip.c -- Support routines for Picture In a Picture Option Hardware.
**
**		Copyright(c) 1990, RasterOps, Inc.
**
** Author: 
**
**   RasterOps folk
**
** Revisions:
**
**   20.09.91 Carver
**     - added code to use new driver interface for pip waiting if it exists
**     - fixed gray scaled parameters
**
**   18.09.91 Carver
**     - added two new procedures: pip_src_area and pip_dst_area: xvrop.c uses.
**     - now uses field mode to size in y to less than 1/2 signal size
**     - made some very hardware specific adjustments for sizing and alignment
**     - replaced old size mask procs with new ones from Don Fong
**       still had an off by one problem in X made adjustment.
**     - changed video region offset in video source signal to latest values
**       given to me my RasterOps.
**
**   29.08.91 Carver
**     - changed pip_source_type to fix svideo href
**     - added support for gray scaled video
**     - changed all interfaces to take RopPtr as first argument
**       RopPtr is defined in rop.h
**
**   16.08.91 Carver
**     - fixed spin wait problem
**     - fixed sizing problem
**
**   04.06.91 Carver
**   22.04.91 Carver
**     - added TC1_CSR2_SYNC_NOT_HREF that should be define in pmagro.h
**     - added variable to manage i2c delay; reduced delay to 5
**     - added variable to shadow DCSC source type since DCSC is write only
**     - added timeout stuff
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/tty.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/workstation.h>
#include <sys/pmagro.h>
#include <sys/i2c.h>

#include "misc.h" /* GET MAXSCREENS SYMBOL */
#include "rop.h" /* RopPtr STRUCT DEFINITION SHARED WITH ROP.C */
#include "libpip.h"

extern RopRec ropInfo[];
extern int wsFd;

#define SET_MAP_REG(off) { *map_reg = (*map_reg & 0xfffffff0) | (off);}

/*
*/

#define HALFWAY         ((unsigned long)1<<31)
#define AFTER(ta, tb)   ((tb) - (ta) >=  HALFWAY)
#define BEFORE(ta, tb)  ((ta) - (tb) >=  HALFWAY)

static int pipDCSCSourceType = DCSC_COMPOSITE_LUT;
static int i2cDelay = 5;

#ifndef TC1_CSR2_SYNC_NOT_HREF
#define TC1_CSR2_SYNC_NOT_HREF 0x01
#endif

/* END DCC MOD */

/*
 *	Address pointer union used to alias a location to more than
 *	one width
 */
typedef union
{
	u_char		*byte;		/* Address of an 8-bit entity. */
	u_short		*half_word;	/* Address of a 16-bit entity. */
	u_int		integer;	/* Address value as an integer. */
	u_int		*word;		/* Address of a 32-bit entity. */
}	Access;

static int	exit_on_error = 0;

/*"i2c_clock"
 *
 *	Set the i2c bus clock signal to high or low state. This is used by diagnostics
 *	to get the i2c bus into a known state.
 */
i2c_clock(port, state)
	u_char	*port;		/* -> port for i2c bus. */
	int		state;	/* = 1 for high, 0 for low. */
{
	if ( state )
	{
		I2C_CLOCK_HIGH;
	}
	else
	{
		I2C_CLOCK_LOW;
	}
}

/*"i2c_clock_delay"
 *
 *	Delay an interval to let i2c bus clock signal settle.
 */
i2c_clock_delay()
{
    int i;
/* DCC MOD --- SUBSTITUTED VARIABLE FOR FIXED NUMBER --- 04.06.91 */
    for (i = 0; i < i2cDelay; i++) ;
/* END DCC MOD */
    return i;
}

/*"i2c_data"
 *
 *	Set the i2c bus data signal to high or low state. This is used by diagnostics
 *	to get the i2c bus into a known state.
 */
i2c_data(port, state)
	u_char	*port;		/* -> port for i2c bus. */
	int		state;	/* = 1 for high, 0 for low. */
{
	if ( state )
	{
		I2C_DATA_HIGH;
	}
	else
	{
		I2C_DATA_LOW;
	}
}

/*"i2c_data_delay"
 *
 *	Delay an interval to let i2c bus clock signal settle.
 */
i2c_data_delay()
{
    int i;
/* DCC MOD --- SUBSTITUTED VARIABLE FOR FIXED NUMBER --- 04.06.91 */
    for (i = 0; i < i2cDelay; i++) ;
/* END DCC MOD */
    return i;
}
	
/*"i2c_exit_on_error"
 *
 *	Set quick exit bit to indicate quick exit on error, or to continue processing.
 */
i2c_exit_on_error( flag )
	int		flag;		/*  = value to set quick exit flag to. */
{
	extern int	exit_on_error;	/* Flag indicating if quick exit to be taken on error. */

	exit_on_error = flag;
}
/*"i2c_read"
 *
 *	Write a slave address and subcomponent request to the i2c bus and read back the
 *	data associated with it.
 *
 *		= 1 if success
 *		= 0 if error detected
 */
i2c_read(port, slave, subaddr, data)
	u_char	*port;		/* -> port for i2c bus. */
	int		slave;		/* =  address of slave device writing to. */
	int		subaddr;	/* =  address of subcomponent writing to. */
	int		*data;		/* -> where to place data read. */
{
	int				delay;		/* Index: number of times tested for clock and data both high. */
	int				retry;		/* Index: number of attempt to get clock and data both high. */

    /*  Make sure the i2c interface is in the proper state (both clock and data
     *  bits are high). If they are not, try toggling the clock and sending
     *  a stop command sequence.
     */
    for ( retry = 0; retry < 20; retry++ )
    {
        for (delay = 0; delay < 100000; delay++)
        {
            if ( (*port & (I2C_CLOCK_ON_DATA_ONE)) == I2C_CLOCK_ON_DATA_ONE ) goto start;
        }
        I2C_DATA_HIGH;
        I2C_CLOCK_LOW;
        I2C_CLOCK_HIGH;
     
        I2C_CLOCK_LOW;
        I2C_DATA_LOW;
        I2C_CLOCK_HIGH;
        I2C_DATA_HIGH;
    }

    if ( (*port & (I2C_CLOCK_ON_DATA_ONE)) != I2C_CLOCK_ON_DATA_ONE )
    {
        fprintf(stderr,"i2c_write(%02x,%02x,%02x): i2c bus not ready!!!!\n", slave, subaddr, data);
        return 0;
    }

    /*  Send out the start pulse signals. 
     */
start:
    I2C_DATA_LOW;
    I2C_CLOCK_LOW;

	/*	Send out the slave device address. If there is an error
	 *	pull down data then clock, then pull both high together,
	 *	then clock high, and wait a while.
	 */
	if ( !i2c_write_byte(port, slave) )
	{
		fprintf(stderr,"i2c_read(%02x,%02x): unable to send slave byte in set address correctly\n", slave, subaddr);
		I2C_DATA_LOW;
		I2C_CLOCK_LOW;
		I2C_CLOCK_DATA_HIGH;
		I2C_CLOCK_HIGH;
		return 0;
	}

	/*	Send out the slave subcomponent address. If there is an error
	 *	pull down data then clock, then pull both high together,
	 *	then clock high, and wait a while.
	 */
	if ( !i2c_write_byte(port, subaddr) )
	{
		fprintf(stderr,"i2c_read(%02x,%02x): unable to send subaddress byte correctly \n", slave, subaddr);
		I2C_DATA_LOW;
		I2C_CLOCK_LOW;
		I2C_CLOCK_DATA_HIGH;
		I2C_CLOCK_HIGH;
		return 0;
	}

	/*	Initiate a new sequence by setting the data value low while clock is high.
	 *	(The data value was already high from the last byte write acknowledge
	 *	cycle - see "i2c_write_byte" below.)
	 *	Send out the slave device address with a read indication. 
	 *	If there is an error pull down data then clock, then pull both high 
	 *	together, then clock high, and wait a while.
	 */
	I2C_CLOCK_LOW;
	I2C_CLOCK_HIGH;
	I2C_DATA_LOW;
	if ( !i2c_write_byte(port, slave | I2C_READ_BIT) )
	{
		fprintf(stderr,"i2c_read(%02x,%02x): unable to send slave byte in read cmd correctly \n", slave, subaddr);
		I2C_DATA_LOW;
		I2C_CLOCK_LOW;
		I2C_CLOCK_DATA_HIGH;
		I2C_CLOCK_HIGH;
		return 0;
	}

	/*	Read back the data byte. If there is an error
	 *	pull down data then clock, then pull both high together,
	 *	then clock high, and wait a while.
	 */
	if ( !i2c_read_byte(port, data) )
	{
		fprintf(stderr,"i2c_read(%02x,%02x): unable to read data byte correctly \n", slave, subaddr);
		I2C_DATA_LOW;
		I2C_CLOCK_LOW;
		I2C_CLOCK_DATA_HIGH;
		I2C_CLOCK_HIGH;
		return 0;
	}

	return 1;
}

/*"i2c_read_byte"
 *
 *  Toggle a byte value off the i2c bus and wait for an acknowledgment.
 *  This routine assumes that the i2c interface is properly prepared to
 *  send data. Each data bit is transmitted by:
 *      a) setting the clock bit low
 *      b) setting the data bit to the appropriate sense (0 or 1)
 *      c) setting the clock bit high
 *
 *      = 1 if successful
 *      = 0 if error detected
 */
i2c_read_byte(port, data)
    u_char  *port;      /* -> i2c port to read from. */
    int     *data;      /* -> where to place data read. */
{
    int     bit;        /* Index: next bit to be transmitted. */
    int     delay;      /* How long to loop waiting for i2c to become ready. */
 
 
	/*	Loop reading in the bits, assembling them into a byte.
	 */
    for (bit = 0, *data = 0; bit < 8; bit++)
    {
        I2C_CLOCK_LOW;
		I2C_DATA_HIGH;	/* #### make sure slave is driving bus, (I'm not!). */
        I2C_CLOCK_HIGH;
		*data |= (*port & I2C_DATA_ONE) >> bit;
    }
 
	/*	We only read a single byte, so don't send an acknowledgement, rather
	 *	send the stop sequence.
	 */
	I2C_CLOCK_LOW;
    I2C_DATA_LOW;
    I2C_CLOCK_HIGH;
    I2C_DATA_HIGH;
}

/*"i2c_write"
 *
 *      = 1 if success
 *      = 0 if error detected
 */
i2c_write(port, slave, subaddr, data)
    u_char  *port;      /* -> port for i2c bus. */
    int     slave;      /* =  address of slave device writing to. */
    int     subaddr;    /* =  address of subcomponent writing to. */
    int     data;       /* =  data to write. */
{
	int		delay;		/* Index: number of times tested for clock and data both high. */
	int		retry;		/* Index: number of attempt to get clock and data both high. */

    /*  Make sure the i2c interface is in the proper state (both clock and data
     *  bits are high). If they are not, try toggling the clock and sending
     *  a stop command sequence.
     */
    for ( retry = 0; retry < 20; retry++ )
    {
        for (delay = 0; delay < 100000; delay++)
        {
            if ( (*port & (I2C_CLOCK_ON_DATA_ONE)) == I2C_CLOCK_ON_DATA_ONE ) goto start;
        }
        I2C_DATA_HIGH;
        I2C_CLOCK_LOW;
        I2C_CLOCK_HIGH;
     
        I2C_CLOCK_LOW;
        I2C_DATA_LOW;
        I2C_CLOCK_HIGH;
        I2C_DATA_HIGH;
    }

	if ( (*port & (I2C_CLOCK_ON_DATA_ONE)) != I2C_CLOCK_ON_DATA_ONE )
	{
		fprintf(stderr,"i2c_write(%02x,%02x,%02x): i2c bus not ready!!!!\n", slave, subaddr, data);
		return 0;
	}

	/*	Send out the start pulse signals. 
	 */
start:
    I2C_DATA_LOW;
    I2C_CLOCK_LOW;
 
    /*  Send out the slave device address. If there is an error
     *  pull down data then clock, then pull both high together,
     *  then clock high, and wait a while.
     */
    if ( !i2c_write_byte(port, slave) )
    {
        fprintf(stderr,"i2c_write(%02x,%02x,%02x): unable to send slave byte correctly\n", slave, subaddr, data);
        I2C_DATA_LOW;
        I2C_CLOCK_LOW;
        I2C_CLOCK_DATA_HIGH;
        I2C_CLOCK_HIGH;
        return 0;
    }
 
    /*  Send out the slave subcomponent address. If there is an error
     *  pull down data then clock, then pull both high together,
     *  then clock high, and wait a while.
     */
    if ( !i2c_write_byte(port, subaddr) )
    {
        fprintf(stderr,"i2c_write(%02x,%02x,%02x): unable to send subaddress byte correctly\n", slave, subaddr, data);
        I2C_DATA_LOW;
        I2C_CLOCK_LOW;
        I2C_CLOCK_DATA_HIGH;
        I2C_CLOCK_HIGH;
        return 0;
    }
 
    /*  Send out the data byte. If there is an error
     *  pull down data then clock, then pull both high together,
     *  then clock high, and wait a while.
     */
    if ( !i2c_write_byte(port, data) )
    {
        fprintf(stderr,"i2c_write(%02x,%02x,%02x): unable to send data byte correctly\n", slave, subaddr, data);
        I2C_DATA_LOW;
        I2C_CLOCK_LOW;
        I2C_CLOCK_DATA_HIGH;
        I2C_CLOCK_HIGH;
        return 0;
    }
 
	/*	All done with the transmission, so make sure i2c bus is in a known stopped
	 *	state and return success to our caller.
	 */
	I2C_DATA_LOW;
	I2C_CLOCK_HIGH;
	I2C_DATA_HIGH;
    return 1;
}

/*"i2c_write_byte"
 *
 *	Toggle a byte value into the i2c bus and wait for an acknowledgment.
 *	This routine assumes that the i2c interface is properly prepared to
 *	accept data. Each data bit is transmitted by:
 *		a) setting the clock bit low
 *		b) setting the data bit to the appropriate sense (0 or 1)
 *		c) setting the clock bit high
 *
 *		= 1 if successful
 *		= 0 if error detected
 */
i2c_write_byte(port, data)
	u_char	*port;		/* -> i2c port to write to. */
	int		data;		/* =  data to be written. */
{
	int			bit;			/* Index: next bit to be transmitted. */
	int			delay;			/* How long to loop waiting for i2c to become ready. */
	extern int	exit_on_error;	/* Flag indicating if quick exit to be taken on error. */


	for (bit = 0; bit < 8; bit++)
	{
		I2C_CLOCK_LOW;
		*port = (*port & I2C_DATA_ZERO) | ( (data << bit) & I2C_DATA_ONE );
		I2C_DATA_DELAY;
		I2C_CLOCK_HIGH;
		I2C_CLOCK_DELAY;
	}

	/*	Set the clock is low, set the data bit high and look for the acknowledge from 
	 *	the slave device we have written to. When we receive the acknowledge
	 *	bring the clock back high then low.
	 */
	I2C_CLOCK_LOW;
	I2C_DATA_HIGH;
	I2C_CLOCK_DATA_HIGH;
	for (delay = 0; delay < 100000; delay++)
	{
		if ( (*port & I2C_DATA_ONE) == 0 ) break;
	}
	if ( (*port & (I2C_DATA_ONE)) != 0 ) 
	{
		fprintf(stderr,"i2c_write_byte(%02x): i2c bus not ready!!!!\n", data);
		if ( exit_on_error ) 
		{
			exit(1); 
		}
		else 
		{
			return 0;
		}
	}
	I2C_CLOCK_LOW;
	I2C_DATA_HIGH;		/*####*/
	return 1;
}

/*"i2c_write_bytes"
 *
 *	Toggle a byte value into the i2c bus and wait for an acknowledgment.
 *	This routine assumes that the i2c interface is properly prepared to
 *	accept data. Each data bit is transmitted by:
 *		a) setting the clock bit low
 *		b) setting the data bit to the appropriate sense (0 or 1)
 *		c) setting the clock bit high
 *
 *		= 1 if successful
 *		= 0 if error detected
 */
i2c_write_bytes(port, count, data)
	u_char	*port;		/* -> i2c port to write to. */
	int		count;		/* =  number of data bytes to write. */
	u_char	*data;		/* =  data to be written. */
{
	int			bit;			/* Index: next bit to be transmitted. */
	int			data_i;			/* Index: next data byte to write. */
	int			delay;			/* How long to loop waiting for i2c to become ready. */
	extern int	exit_on_error;	/* Flag indicating if quick exit to be taken on error. */


	for ( data_i = 0; data_i < count; data_i++ )
	{
		for (bit = 0; bit < 8; bit++)
		{
			I2C_CLOCK_LOW;
			*port = (*port & I2C_DATA_ZERO) | 
				( (data[data_i] << bit) & I2C_DATA_ONE );
			I2C_DATA_DELAY;
			I2C_CLOCK_HIGH;
			I2C_CLOCK_DELAY;
		}

		/*	Set the clock is low, set the data bit high and look for the acknowledge from 
		 *	the slave device we have written to. When we receive the acknowledge
		 *	bring the clock back high then low.
		 */
		I2C_CLOCK_LOW;
		I2C_DATA_HIGH;
		I2C_CLOCK_DATA_HIGH;
		for (delay = 0; delay < 100000; delay++)
		{
			if ( (*port & I2C_DATA_ONE) == 0 ) break;
		}
		if ( (*port & (I2C_DATA_ONE)) != 0 ) 
		{
			fprintf(stderr,"i2c_write_byte(%02x): i2c bus not ready!!!!\n", data);
			if ( exit_on_error ) 
			{
				exit(1); 
			}
			else 
			{
				return 0;
			}
		}
	}
	I2C_CLOCK_LOW;
	I2C_DATA_HIGH;		/*####*/
	return 1;
}

/*"i2c_write_list"
 *
 *      = 1 if success
 *      = 0 if error detected
 */
i2c_write_list(port, slave, subaddr, count, data)
    u_char  *port;      /* -> port for i2c bus. */
    int     slave;      /* =  address of slave device writing to. */
    int     subaddr;    /* =  address of subcomponent writing to. */
	int		count;		/* =  count of data to be written. */
    u_char  *data;      /* =  data to write. */
{
	int		delay;		/* Index: number of times tested for clock and data both high. */
	int		retry;		/* Index: number of attempt to get clock and data both high. */

    /*  Make sure the i2c interface is in the proper state (both clock and data
     *  bits are high). If they are not, try toggling the clock and sending
     *  a stop command sequence.
     */
    for ( retry = 0; retry < 20; retry++ )
    {
        for (delay = 0; delay < 100000; delay++)
        {
            if ( (*port & (I2C_CLOCK_ON_DATA_ONE)) == I2C_CLOCK_ON_DATA_ONE ) goto start;
        }
        I2C_DATA_HIGH;
        I2C_CLOCK_LOW;
        I2C_CLOCK_HIGH;
     
        I2C_CLOCK_LOW;
        I2C_DATA_LOW;
        I2C_CLOCK_HIGH;
        I2C_DATA_HIGH;
    }

	if ( (*port & (I2C_CLOCK_ON_DATA_ONE)) != I2C_CLOCK_ON_DATA_ONE )
	{
		fprintf(stderr,"i2c_write_list(%02x,%02x): i2c bus not ready!!!!\n", slave, subaddr);
		return 0;
	}

	/*	Send out the start pulse signals. 
	 */
start:
    I2C_DATA_LOW;
    I2C_CLOCK_LOW;
 
    /*  Send out the slave device address. If there is an error
     *  pull down data then clock, then pull both high together,
     *  then clock high, and wait a while.
     */
    if ( !i2c_write_byte(port, slave) )
    {
        fprintf(stderr,"i2c_write(%02x,%02x): unable to send slave byte correctly\n", slave, subaddr);
        I2C_DATA_LOW;
        I2C_CLOCK_LOW;
        I2C_CLOCK_DATA_HIGH;
        I2C_CLOCK_HIGH;
        return 0;
    }
 
    /*  Send out the slave subcomponent address. If there is an error
     *  pull down data then clock, then pull both high together,
     *  then clock high, and wait a while.
     */
    if ( !i2c_write_byte(port, subaddr) )
    {
        fprintf(stderr,"i2c_write(%02x,%02x,%02x): unable to send subaddress byte correctly\n", slave, subaddr, data);
        I2C_DATA_LOW;
        I2C_CLOCK_LOW;
        I2C_CLOCK_DATA_HIGH;
        I2C_CLOCK_HIGH;
        return 0;
    }
 
    /*  Send out the data bytes. If there is an error
     *  pull down data then clock, then pull both high together,
     *  then clock high, and wait a while.
     */
    if ( !i2c_write_bytes(port, count, data) )
    {
        fprintf(stderr,"i2c_write(%02x,%02x): unable to send data byte correctly\n", slave, subaddr);
        I2C_DATA_LOW;
        I2C_CLOCK_LOW;
        I2C_CLOCK_DATA_HIGH;
        I2C_CLOCK_HIGH;
        return 0;
    }
 
	/*	All done with the transmission, so make sure i2c bus is in a known stopped
	 *	state and return success to our caller.
	 */
	I2C_DATA_LOW;
	I2C_CLOCK_HIGH;
	I2C_DATA_HIGH;
    return 1;
}


#ifdef PIP_WAIT

/* USE DRIVER INTERFACE FOR WAITING IF IT IS AVAILABLE */

int pip_which_screen(prop)
     RopPtr prop;
{
  int i;
  for(i = 0; i < MAXSCREENS; i++) {
    if(prop == &ropInfo[i])
      return i;
  }
  fprintf(stderr, "libpip.c: which_screen: screen not found!\n", i);
  return -1;
}

pip_wait_off(prop)
     RopPtr prop;
{
  rop_wait_ioctl rip;

  if ((rip.screen = pip_which_screen(prop)) < 0) return 0;
  rip.command = PIP_WAIT_OFF;
  if (ioctl(wsFd, PIP_WAIT, &rip) != 0) {
    return 0;
  }
  return 1;
}

pip_wait_on(prop)
     RopPtr prop;
{
  rop_wait_ioctl rip;

  if ((rip.screen = pip_which_screen(prop)) < 0) return 0;
  rip.command = PIP_WAIT_ON;
  if (ioctl(wsFd, PIP_WAIT, &rip) != 0) {
    return 0;
  }
  return 1;
}
#else

/* IF DRIVER INTERFACE NOT AVAILABLE THEN JUST RETURN 0 */

pip_wait_off(prop)
     RopPtr prop;
{
  return 0;
}

pip_wait_on(prop)
     RopPtr prop;
{
  return 0;
}
#endif

/* DCC MOD  --- NEW PROCEDURE TO DETERMINE IF PIP IS ACTIVE --- 24.05.91 */

pip_active(prop)
     RopPtr prop;
{

  Pip_Device_Regs	*pregs;
  pregs = (Pip_Device_Regs *)prop->pipRegisters;

  if (pregs->control_status & TC1_CSR0_PIP_IS_ACTIVE)
    return 1;

  return 0;

}

/* DCC MOD --- NEW PIP PROCEDURE FROM RASTEROPS --- 04.06.91 */

/*"pip_hue"
 *
 *  Set the color hue for the incoming pip signal. This is 
 *  controlled by a register in the DMSD.
 */
void pip_hue(prop, level)
     RopPtr prop;
     int level;	                /* =  level of hue (0 to 255). */
{
  Pip_Device_Regs	*pregs;
  pregs = (Pip_Device_Regs *)prop->pipRegisters;
  i2c_write( &pregs->control_status1, I2C_ADDR_DMSD, 0x07, level&0xff );
}

pip_priority(prop, prio)
     RopPtr prop;
     unsigned int prio;
{

  if (prio > 0xB0) prio = 0xB0;
  *prop->dutyCycle = prio;

}

/* DCC MOD --- CHANGED TO USE LIBPIP.H SYMBOLS --- 04.06.91 */

/*"pip_init"
 *
 *	Initialize pip hardware to an operable state. 
 */
RopPtr
pip_init(index, type)
     int index;
     int		type;	/* =  type of video source: see libpip.h */
{
  /*
   *	Set default source to composite, with pip turned off. Disable 
   *	all special effects (e.g., inverted fields, flipped video).
   *	Set pitch register to 1280.
   */
  Pip_Device_Regs	*pregs;	/* -> pip device register area. */
  RopPtr prop;

  prop = &ropInfo[index];
  pregs = (Pip_Device_Regs *)prop->pipRegisters;

  if (!pip_installed(prop)) return (RopPtr)NULL;

  pregs->control_status = 0;
  pregs->control_status1 = 0xC0;
  pregs->control_status2 = 0;
  
  pregs->fb_pitch_hi = 0x05;
  pregs->fb_pitch_low = 0x00;
  
  /*
   *	Set signal source size to encompass the source type specified
   *	by the caller.
   */
  switch( type & PIP_SOURCE_TIMING )
    {
    default:
      fprintf(stderr,
	      "pip_init: unknown source type %d; defaulting to NTSC\n", type);

    case PIP_NTSC:
      pregs->x_source_start_hi =  0x00;
      pregs->x_source_start_low = 0x0c;
      pregs->x_source_end_hi =    0x02;
      pregs->x_source_end_low =   0x8c;
      
      pregs->x_source_scale_hi =  0x03;
      pregs->x_source_scale_low = 0xff;
      
      pregs->y_source_start_hi =  0x00;
      pregs->y_source_start_low = 0x07;
      pregs->y_source_end_hi =    0x00;
      pregs->y_source_end_low =   0xF7;
      
      pregs->y_source_scale_hi =  0x01;
      pregs->y_source_scale_low = 0xff;
      break;
    case PIP_PAL:
    case PIP_SECAM:
      pregs->x_source_start_hi =  0x00;
      pregs->x_source_start_low = 0x0a;
      pregs->x_source_end_hi =    0x03;
      pregs->x_source_end_low =   0x09;
      
      pregs->x_source_scale_hi =  0x03;
      pregs->x_source_scale_low = 0xff;
      
      pregs->y_source_start_hi =  0x00;
      pregs->y_source_start_low = 0x0b;
      pregs->y_source_end_hi =    0x01;
      pregs->y_source_end_low =   0x28;
      
      pregs->y_source_scale_hi =  0x01;
      pregs->y_source_scale_low = 0xff;
      break;
    }
  
  /*
   *	Initialize the DMSD and dcsc chips, which reside on the I2C bus.
   */
  pip_init_dmsd(&pregs->control_status1, type);
  pip_init_dcsc(&pregs->control_status1, 16, 240);

  return prop;

}

/*"pip_init_dcsc"
 *
 *  Initialize the Digital Color Space Convertor chip with a color ramp 
 *  for "digitizing" data received from the DMSD chip. Note that the 
 *  DCSC is left in composite video configuration, use pip_source to
 *  change to another one if that is appropriate.
 */
pip_init_dcsc(port, _min, _max)
     u_char *port;	/* -> port containing I2C bus interface. */
     int _min;		/* =  min value expected from DMSD chip for pixel. */
     int _max;		/* =  max value expected from DMSD chip for pixel. */
{
  float	lut_delta;	/* DCSC lookup table inter-entry increment. */
  int	lut_index;	/* DCSC lookup table value to load next. */
  float	lut_value;	/* DCSC lookup table value to load next. */
  
  if ( _max-_min > 0) lut_delta = 255. / (_max-_min);
  
  i2c_write(port, I2C_ADDR_DCSC, I2C_SUBA_DCSC_CONTROL, DCSC_LOAD_LUT);
  for ( lut_index = 0; lut_index < _min; lut_index++ )
    {
      i2c_write(port, I2C_ADDR_DCSC, I2C_SUBA_DCSC_DATA, 0);
    }
  for ( lut_value = 0; lut_index < _max; lut_index++, lut_value+=lut_delta )
    {
      i2c_write(port, I2C_ADDR_DCSC, I2C_SUBA_DCSC_DATA, (int)lut_value);
    }
  for ( ; lut_index < 256; lut_index++ )
    {
      i2c_write(port, I2C_ADDR_DCSC, I2C_SUBA_DCSC_DATA, 240);
    }
  i2c_write(port, I2C_ADDR_DCSC, I2C_SUBA_DCSC_CONTROL, pipDCSCSourceType);
}

/* DCC MOD --- 04.06.91 
**   - changed to use libpip.h symbols
**   - broke out type and timing init into different procedure
*/

/*"pip_init_dmsd"
 *
 *  Initialize the Phillips Digital Multi-Standard Decoder chip's parameters. 
 *  Send data serially over the on-board I2C bus to set up the DMSD chip to 
 *  provide scan conversion of NTSC and PAL signals.
 *
 */
pip_init_dmsd(port, type)
     u_char	*port;		/* -> port containing I2C bus interface. */
     int	type;		/* =  type of timing regimen see libpip.h */
{
  i2c_write( port, I2C_ADDR_DMSD, 0x00, 0x50 );
  i2c_write( port, I2C_ADDR_DMSD, 0x01, 0x30 );
  i2c_write( port, I2C_ADDR_DMSD, 0x02, 0x00 );
  i2c_write( port, I2C_ADDR_DMSD, 0x03, 0xE8 );
  i2c_write( port, I2C_ADDR_DMSD, 0x04, 0xB6 );
  
  i2c_write( port, I2C_ADDR_DMSD, 0x07, 0x00 );
  i2c_write( port, I2C_ADDR_DMSD, 0x08, 0xFE );
  i2c_write( port, I2C_ADDR_DMSD, 0x09, 0xF0 );
  i2c_write( port, I2C_ADDR_DMSD, 0x0A, 0xFE );
  i2c_write( port, I2C_ADDR_DMSD, 0x0B, 0xE0 );
  i2c_write( port, I2C_ADDR_DMSD, 0x0C, 0x20 );
  
  i2c_write( port, I2C_ADDR_DMSD, 0x10, 0x00 );
  i2c_write( port, I2C_ADDR_DMSD, 0x11, 0x80 );
  
  i2c_write( port, I2C_ADDR_DMSD, 0x14, 0x3A );
  i2c_write( port, I2C_ADDR_DMSD, 0x15, 0x06 );
  i2c_write( port, I2C_ADDR_DMSD, 0x16, 0xFA );
  i2c_write( port, I2C_ADDR_DMSD, 0x17, 0xD6 );
  i2c_write( port, I2C_ADDR_DMSD, 0x18, 0x20 );

  pip_init_dmsd_type(port, type);

}

pip_init_dmsd_type(port, type)
     u_char	*port;		/* -> port containing I2C bus interface. */
     int	type;		/* =  type of timing regimen see libpip.h */
{

  switch(type & PIP_SOURCE_TIMING)
    {
    default:
    case PIP_NTSC: 
      i2c_write( port, I2C_ADDR_DMSD, 0x05, 0xF2 ); 
      i2c_write( port, I2C_ADDR_DMSD, 0x0D, 0x80 );
      i2c_write( port, I2C_ADDR_DMSD, 0x0F, 0x58 ); 
      break;
    case PIP_PAL:  
      i2c_write( port, I2C_ADDR_DMSD, 0x05, 0x34 ); 
      i2c_write( port, I2C_ADDR_DMSD, 0x0D, 0x80 );
      i2c_write( port, I2C_ADDR_DMSD, 0x0F, 0x39 );
      break;
    case PIP_SECAM:
      i2c_write( port, I2C_ADDR_DMSD, 0x05, 0x34 ); 
      i2c_write( port, I2C_ADDR_DMSD, 0x0D, 0x81 );
      i2c_write( port, I2C_ADDR_DMSD, 0x0F, 0x3F );
      break;
    } 

  switch(type & PIP_SOURCE_TYPE)
    {
    default:
    case PIP_COMPOSITE: 
      if (type & PIP_GRAY)
	{
	  i2c_write( port, I2C_ADDR_DMSD, 0x06, 0x13 );
	  i2c_write( port, I2C_ADDR_DMSD, 0x0E, 0x37 );
	}
      else
	{
	  i2c_write( port, I2C_ADDR_DMSD, 0x06, 0x13 );
	  i2c_write( port, I2C_ADDR_DMSD, 0x0E, 0x33 );
	}
      pipDCSCSourceType = DCSC_COMPOSITE_LUT;
      break;
    case PIP_SVIDEO:  
      if (type & PIP_GRAY)
	{
	  i2c_write( port, I2C_ADDR_DMSD, 0x06, 0x13 );
	  i2c_write( port, I2C_ADDR_DMSD, 0x0E, 0x33 );
	}
      else
	{
	  i2c_write( port, I2C_ADDR_DMSD, 0x06, 0x93 );
	  i2c_write( port, I2C_ADDR_DMSD, 0x0E, 0x33 );
	}
      pipDCSCSourceType = DCSC_S_VIDEO_LUT;
      break;
    case PIP_RGB:
      i2c_write( port, I2C_ADDR_DMSD, 0x06, 0x13 );
      i2c_write( port, I2C_ADDR_DMSD, 0x0E, 0x30 );
      pipDCSCSourceType = DCSC_RGB_LUT;
      break;
    } 

}

/* DCC MOD --- NEW PROCEDURE FOR XV --- 24.5.91 */

pip_input_connected(prop)
     RopPtr prop;
{
  Pip_Device_Regs	*pregs;	/* -> pip device register area. */

  pregs = (Pip_Device_Regs *)prop->pipRegisters;

  if (pregs->control_status1 & TC1_CSR1_INPUT_CONNECTED)
    return 1;

  return 0;

}

pip_installed(prop)
     RopPtr prop;
{
  Pip_Device_Regs	*pregs;	/* -> pip device register area. */

  pregs = (Pip_Device_Regs *)prop->pipRegisters;
  return (!(pregs->control_status1 & TC1_CSR1_NO_PIP));
}

/* END DCC MOD */

/*"pip_ioctl"
 *
 *	I/O Control Interface support for Picture In a Picture hardware.
 *	This routine is called from the normal ioctl code for the frame
 *	buffer device driver.
 */
pip_ioctl(dev, softc, cmd, data)
	int		dev;	/* =  device this ioctl is for. */
	caddr_t	softc;	/* -> per unit software characteristics area. */
	int		cmd;	/* =  ioctl command to execute. */
	caddr_t	data;	/* -> where to get or place data associated with this ioctl. */
{
	switch( cmd )
	{
		default:
		break;
	}
}

/* DCC MOD --- LOAD DCSC --- 04.06.91 */

/*"pip_load_dcsc"
 *
 *  Set the Digital Color Space Convertor chip load
 */
pip_load_dcsc(prop, values)
     RopPtr prop;
     unsigned char *values;     /* array[0..255] of values */
{
  u_char *port;	        /* -> port containing I2C bus interface. */
  int lut_index;	/* DCSC lookup table value to load next. */
  unsigned char *lut_value;       /* pointer to current value */
  Pip_Device_Regs	*pregs;	/* -> pip device register area. */

  pregs = (Pip_Device_Regs *)prop->pipRegisters;

  port = (u_char *)&pregs->control_status1;
  lut_value = values;

  i2c_write(port, I2C_ADDR_DCSC, I2C_SUBA_DCSC_CONTROL, DCSC_LOAD_LUT);
  for ( lut_index = 0; lut_index < 255; lut_index++ )
    {
      i2c_write(port, I2C_ADDR_DCSC, I2C_SUBA_DCSC_DATA, *lut_value++);
    }
  i2c_write(port, I2C_ADDR_DCSC, I2C_SUBA_DCSC_CONTROL, pipDCSCSourceType);
}

/* END DCC MOD */

/* DCC MOD for Xv --- 22.4.91 */

/*"pip_off"
 *
 *	Turn off pip and return its previous operational state.
 */
pip_off(prop, wait)
     RopPtr prop;
     int wait;	/* =  1 if should wait for pip to actually turn off. */
{
  unsigned long entryTime,currentTime;
  int old_state;	/* Previous state of pip operations. */
  struct timeval to;
  Pip_Device_Regs	*pregs;	/* -> pip device register area. */

  pregs = (Pip_Device_Regs *)prop->pipRegisters;

  old_state = (pregs->control_status & TC1_CSR0_PIP_IS_ON) ? 1 : 0;

  pregs->control_status = pregs->control_status & 
    ~(TC1_CSR0_TURN_PIP_ON|TC1_CSR0_PIP_IS_ON);
  pregs->control_status = pregs->control_status & 
    ~(TC1_CSR0_TURN_PIP_ON|TC1_CSR0_PIP_IS_ON);

  if ( wait )
    {

      if (!(pregs->control_status &
	    (TC1_CSR0_PIP_IS_ON | TC1_CSR0_PIP_IS_ACTIVE))) return old_state;

      if (!pip_wait_off(prop))
	{
	  entryTime = currentTime = GetTimeInMillis();
	  while (BEFORE(currentTime, entryTime+100))
	    {
	      if (!(pregs->control_status &
		    (TC1_CSR0_PIP_IS_ON | TC1_CSR0_PIP_IS_ACTIVE))) 
		return old_state;
	      currentTime = GetTimeInMillis();
	      to.tv_sec = 0;             /* Sleep for 1 millisecond */
	      to.tv_usec = 1000;         /* (actually 4 because of */
	      select(0,0,0,0,&to);       /* scheduler granularity)   */
	    }
	}
    }
  return old_state;
}

/*"pip_on"
 *
 *	Turn on pip and return its previous operational state.
 */
pip_on(prop)
     RopPtr prop;
{
  int old_state;	/* Previous state of pip operations. */
  Pip_Device_Regs	*pregs;	/* -> pip device register area. */

  pregs = (Pip_Device_Regs *)prop->pipRegisters;

  old_state = (pregs->control_status & TC1_CSR0_PIP_IS_ON) ? 1 : 0;
  pregs->control_status = pregs->control_status | TC1_CSR0_TURN_PIP_ON;
  return old_state;
}

/* DCC MOD FOR Xv --- 28.5.91 */

/* pip_off_with_timeout --- added by dcc
 *
 *	Turn off pip and return true if it turned on; otherwise false
 */
pip_off_with_timeout(prop, wait)
     RopPtr prop;
     int wait;	/* =  1 if should wait for pip to actually turn off. */
{
  unsigned long entryTime,currentTime;
  struct timeval to;
  Pip_Device_Regs	*pregs;	/* -> pip device register area. */

  pregs = (Pip_Device_Regs *)prop->pipRegisters;

  if (!pip_input_connected(prop)) return 0;

  pregs->control_status = pregs->control_status & 
    ~(TC1_CSR0_TURN_PIP_ON|TC1_CSR0_PIP_IS_ON);
  pregs->control_status = pregs->control_status & 
    ~(TC1_CSR0_TURN_PIP_ON|TC1_CSR0_PIP_IS_ON);

  if ( wait )
    {
      if (!(pregs->control_status & 
	    (TC1_CSR0_PIP_IS_ON | TC1_CSR0_PIP_IS_ACTIVE))) return 1;

      if(!pip_wait_off(prop))
	{
	  entryTime = currentTime = GetTimeInMillis();
	  while (BEFORE(currentTime, entryTime+100))
	    {
	      if (!(pregs->control_status &
		    (TC1_CSR0_PIP_IS_ON | TC1_CSR0_PIP_IS_ACTIVE))) return 1;
	      currentTime = GetTimeInMillis();
	      to.tv_sec = 0;             /* Sleep for 1 millisecond */
	      to.tv_usec = 1000;         /* (actually 4 because of */
	      select(0,0,0,0,&to);       /* scheduler granularity)   */
	    }
	  
	  return 0;
	}
      
      if (!(pregs->control_status &
	    (TC1_CSR0_PIP_IS_ON | TC1_CSR0_PIP_IS_ACTIVE))) return 1;
      
      return 0;

    }

  return 1;

}

/* pip_on_with_timeout --- added by dcc
 *
 *	Turn on pip and return its previous operational state.
 */
pip_on_with_timeout(prop,wait)
     RopPtr prop;
     int wait;
{
  unsigned long entryTime,currentTime;
  struct timeval to;
  Pip_Device_Regs	*pregs;	/* -> pip device register area. */

  pregs = (Pip_Device_Regs *)prop->pipRegisters;

  if (!pip_input_connected(prop)) return 0;

  pregs->control_status = pregs->control_status | TC1_CSR0_TURN_PIP_ON;

  if ( wait )
    {

      if (pregs->control_status & TC1_CSR0_PIP_IS_ACTIVE) return 1;

      if (!pip_wait_on(prop))
	{
	  entryTime = currentTime = GetTimeInMillis();
	  while (BEFORE(currentTime, entryTime+100))
	    {
	      if (pregs->control_status & TC1_CSR0_PIP_IS_ACTIVE) return 1;
	      currentTime = GetTimeInMillis();
	      to.tv_sec = 0;             /* Sleep for 1 millisecond */
	      to.tv_usec = 1000;         /* (actually 4 because of */
	      select(0,0,0,0,&to);       /* scheduler granularity)   */
	    }
	  return 0;
	}

      if (pregs->control_status & TC1_CSR0_PIP_IS_ACTIVE) return 1;

      return 0;

    }

  return 1;

}

/* END DCC MOD */

/* DCC MOD --- NEW PROCEDURE FOR STILL FRAMES --- 24.05.91 */

pip_one_shot(prop,wait)
     RopPtr prop;
     int wait;
{
  unsigned long entryTime,currentTime;
  struct timeval to;
  Pip_Device_Regs	*pregs;	/* -> pip device register area. */

  pregs = (Pip_Device_Regs *)prop->pipRegisters;

  if (!pip_input_connected(prop)) return 0;

  pregs->control_status = pregs->control_status | TC1_CSR0_PIP_ONE_SHOT;

  if ( wait )
    {

      if (pregs->control_status & TC1_CSR0_PIP_IS_ACTIVE) return 1;

      if (!pip_wait_on(prop))
	{
	  entryTime = currentTime = GetTimeInMillis();
	  while (BEFORE(currentTime, entryTime+100))
	    {
	      if (pregs->control_status & TC1_CSR0_PIP_IS_ACTIVE) return 1;
	      currentTime = GetTimeInMillis();
	      to.tv_sec = 0;             /* Sleep for 1 millisecond */
	      to.tv_usec = 1000;         /* (actually 4 because of */
	      select(0,0,0,0,&to);       /* scheduler granularity)   */
	    }
	  return 0;
	}

      if (pregs->control_status & TC1_CSR0_PIP_IS_ACTIVE) return 1;

      return 0;

    }
  return 1;
}

/* END DCC MOD */

/*"pip_origin"
 *
 *	Set the origin of the pip image (upper left corner.)
 *	Note the hardware expects an offset three rows before 
 *	where we want things to show up on the screen.
 */
pip_origin(prop, left, top)
     RopPtr prop;
     int left;	/* =  x coordinate of pip image start. */
     int top;	/* =  y coordinate of pip image start. */
{
  u_int	offset;			/* Offset into frame buffer of x and y. */
  Pip_Device_Regs	*pregs;	/* -> pip device register area. */

  pregs = (Pip_Device_Regs *)prop->pipRegisters;

  offset = (left + (1280*(top-3)));
  pregs->start_offset_hi = (offset & 0xff0000) >> 16;
  pregs->start_offset_mid = (offset & 0x00ff00) >> 8;
  pregs->start_offset_low = offset & 0xff;
}

/* DCC MOD --- NEW PIP PROCEDURE FROM RASTEROPS --- 04.06.91 */

/*"pip_saturation"
 *
 *  Set the color saturation level for the incoming pip
 *  signal. This is controlled by a register in the DMSD.
 */
void pip_saturation(prop, level)
     RopPtr prop;
     int level;	                /* =  level of saturation (0 to 255). */
{
  Pip_Device_Regs	*pregs;	/* -> pip device register area. */

  pregs = (Pip_Device_Regs *)prop->pipRegisters;
  i2c_write( &pregs->control_status1, I2C_ADDR_DMSD, 0x11, level&0xff );
}

/* END DCC MOD */

/*"pip_size"
 *
 *	Set the size of the image in the x and y directions.
 *	Note if the image will be less than half size in y use
 *	field only mode to reduce the y size and provide a
 *	better image.
 */
void pip_size(prop, x, y)
     RopPtr prop;
     u_long x;		/* =  x size of image in pixels. */
     u_long y;		/* =  y size of image in pixels. */
{
  u_long on_off;		/* Current setting of live video. */
  int x_signal;	/* Width of current x video signal. */
  u_short x_size;		/* X size mask. */
  int y_signal;	/* Height of current y video signal. */
  u_short y_size;		/* Y size mask. */
  Pip_Device_Regs	*pregs;	/* -> pip device register area. */

  pregs = (Pip_Device_Regs *)prop->pipRegisters;

  /*
   *	Save the state of control register zero and turn off the pip.
   */
  on_off = pip_off(prop, 0);

  /*	Calculate mask for x size and load it in. 
   */

/* DCC MOD --- 30.5.91 --- added + 1 */
  x_signal = (int)X_Source_End - (int)X_Source_Start + 1;
/* END DCC MOD */

  x_size = pip_x_size_mask(x_signal, x);
  pregs->x_source_scale_hi = (x_size & 0xff00) >> 8;
  pregs->x_source_scale_low = (x_size & 0xff);
  
  /*	Calculate mask for y size and load it in. 
   */
  /* DCC MOD --- 30.5.91 --- added + 1 */
  y_signal = 2*(((int)Y_Source_End - (int)Y_Source_Start) + 1);
  /* END DCC MOD */

  /* DCC MOD --- 16.9.91 --- added code from Ricky */
  if(y <= y_signal/2) 
    {
      pregs->control_status1 |= TC1_CSR1_FIELD_ONLY;
      y_size = pip_y_size_mask(y_signal, 2*y);
    } 
  else 
    {
      pregs->control_status1 &= ~TC1_CSR1_FIELD_ONLY;
      y_size = pip_y_size_mask(y_signal, y);
    }
  /* END DCC MOD */

  pregs->y_source_scale_hi = (y_size & 0xff00) >> 8;
  pregs->y_source_scale_low = (y_size & 0xff);
  
  if ( on_off ) pip_on(prop);
}

/*"pip_source_area"
 *
 *	Set the portion of the source signal to be sampled.
 *	Note the y specification is in scan lines, but the registers
 *	take scan line pairs, so we divide by 2!
 */
void pip_source_area(prop, x_start, x_end, y_start, y_end)
     RopPtr prop;
     int	x_start;	/* =  start of signal in x direction. */
     int	x_end;	    /* =  end of signal in x direction. */
     int	y_start;	/* =  start of signal in y_ direction. */
     int	y_end;		/* =  start of signal in y_ direction. */
{
  u_long	on_off;	/* Current setting of control status register 0. */
  Pip_Device_Regs *pregs;	/* -> pip device register area. */

  pregs = (Pip_Device_Regs *)prop->pipRegisters;
  
  on_off = pip_off(prop, 0);

  /* ADJUST VALUES FOR LOCATION OF VIDEO IMAGE INSIDE SIGNAL */

  x_start += 12;
  x_end += 12;
  y_start += 14;
  y_end += 14;

  y_end /= 2;
  y_start /= 2;
  
  pregs->x_source_start_hi = (x_start & 0xff00) >> 8;
  pregs->x_source_start_low = (x_start & 0xff);
  pregs->x_source_end_hi = (x_end & 0xff00) >> 8;
  pregs->x_source_end_low = (x_end & 0xff);
  
  pregs->y_source_start_hi = (y_start & 0xff00) >> 8;
  pregs->y_source_start_low = (y_start & 0xff);
  pregs->y_source_end_hi = (y_end & 0xff00) >> 8;
  pregs->y_source_end_low = (y_end & 0xff);
  
  if ( on_off ) pip_on(prop);
}

/* DCC MOD --- 18.09.91 --- ADDED NEW PROCEDURES */

/*"pip_src_area"
 *
 *	Set the portion of the source signal to be sampled.
 *	Note the y specification is in scan lines, but the registers
 *	take scan line pairs, so we divide by 2!
 */
void pip_src_area(prop, x, y, w, h)
     RopPtr prop;
     int	x;
     int	y;
     int        w;
     int        h;
{
  Pip_Device_Regs *pregs;	/* -> pip device register area. */
  u_long	on_off;	/* Current setting of control status register 0. */
  int	x_end;
  int	y_end;

  pregs = (Pip_Device_Regs *)prop->pipRegisters;
  
  on_off = pip_off(prop, 0);

  /* ADJUST VALUES FOR LOCATION OF VIDEO IMAGE INSIDE SIGNAL */

  x += 12;
  x_end = x + w - 1;
  y += 14;
  y_end = y + h - 1;

  y /= 2;
  y_end /= 2;
  
  pregs->x_source_start_hi = (x & 0xff00) >> 8;
  pregs->x_source_start_low = (x & 0xff);
  pregs->x_source_end_hi = (x_end & 0xff00) >> 8;
  pregs->x_source_end_low = (x_end & 0xff);
  
  pregs->y_source_start_hi = (y & 0xff00) >> 8;
  pregs->y_source_start_low = (y & 0xff);
  pregs->y_source_end_hi = (y_end & 0xff00) >> 8;
  pregs->y_source_end_low = (y_end & 0xff);
  
  if ( on_off ) pip_on(prop);
}


void pip_dst_area(prop, x, y, w, h)
     RopPtr prop;
     int	x;
     int	y;
     int        w;
     int        h;
{
  Pip_Device_Regs *pregs;	/* -> pip device register area. */
  u_long	on_off;	/* Current setting of control status register 0. */
  int x_end;
  int y_end;
  int signal_w;
  int signal_h;
  u_short x_size;		/* X size mask. */
  u_short y_size;		/* Y size mask. */
  u_int offset;
  int adjustment;

  pregs = (Pip_Device_Regs *)prop->pipRegisters;
  
  on_off = pip_off(prop, 0);

  /* GET THE CURRENT SOURCE SIZE */

  signal_w = (int)X_Source_End - (int)X_Source_Start + 1;
  signal_h = 2*(((int)Y_Source_End - (int)Y_Source_Start) + 1);

  /* IF WE ARE USING FIELD MODE THEN CHANGE THE OFFSET ADJUSTMENT */

  if(h <= signal_h/2 && h >= signal_h/4) adjustment = 2;
  else adjustment = 3;

  /* SET THE ORIGIN */

  offset = (x + (1280*(y-adjustment)));
  pregs->start_offset_hi = (offset & 0xff0000) >> 16;
  pregs->start_offset_mid = (offset & 0x00ff00) >> 8;
  pregs->start_offset_low = offset & 0xff;

  x_size = pip_x_size_mask(signal_w, w);
  pregs->x_source_scale_hi = (x_size & 0xff00) >> 8;
  pregs->x_source_scale_low = (x_size & 0xff);
  
  if(h <= signal_h/2) 
    {
      pregs->control_status1 |= TC1_CSR1_FIELD_ONLY;
      y_size = pip_y_size_mask(signal_h, 2*h);
    } 
  else 
    {
      pregs->control_status1 &= ~TC1_CSR1_FIELD_ONLY;
      y_size = pip_y_size_mask(signal_h, h);
    }

  pregs->y_source_scale_hi = (y_size & 0xff00) >> 8;
  pregs->y_source_scale_low = (y_size & 0xff);

}


/* END DCC MOD */

/* DCC MOD --- USE pip_init_dmsd_type AND libpip.h symbols --- 04.06.91 */

/*"pip_source_type"
 *
 *	Set the type of source being used. This involves setting the source
 *	bit in pip control status register 0, and changing certain dmsd and
 *	dcsc chip values.
 */
void pip_source_type(prop, type)
     RopPtr prop;
     int type;                  /* =  type of source. */
{
  u_char			*port;	/* I2C port. */
  int csr0;
  Pip_Device_Regs *pregs;	/* -> pip device register area. */

  pregs = (Pip_Device_Regs *)prop->pipRegisters;
  
  port = (u_char *)&pregs->control_status1;

  pip_init_dmsd_type(port, type);

  i2c_write( port, I2C_ADDR_DCSC, I2C_SUBA_DCSC_CONTROL, pipDCSCSourceType);

  csr0 = pregs->control_status & ~TC1_CSR0_SOURCE_TYPE_MASK;

  switch(type & PIP_SOURCE_TYPE)
    {
    default:
    case PIP_COMPOSITE: 
      csr0 |= TC1_CSR0_COMPOSITE_SOURCE;
      pregs->control_status2 &= ~TC1_CSR2_SYNC_NOT_HREF;
      break;
    case PIP_SVIDEO:  
      csr0 |= TC1_CSR0_S_VIDEO_SOURCE;
      /* THE FOLLOWIN LINE ISN'T WHAT RASTEROPS WANTED THERE, BUT IT WORKS */
      pregs->control_status2 &= ~TC1_CSR2_SYNC_NOT_HREF;
      break;
    case PIP_RGB:
      csr0 |= TC1_CSR0_RGB_SOURCE;
      pregs->control_status2 &= ~TC1_CSR2_SYNC_NOT_HREF;
      break;
    } 

  pregs->control_status = csr0;

}
	
/* END DCC MOD */

/* DCC MOD --- 16.9.91 --- Added code from Don Fong */

int
pip_x_size_mask(signal, view)
	int signal, view;
{
# define PIP_X_SCALE_REG_BITS	10

# define PIP_X_OVERRUN_SLOP	1

	if (view < signal)
		view -= PIP_X_OVERRUN_SLOP;

	return compute_scale_reg(view, PIP_X_SCALE_REG_BITS, signal);
}

int
pip_y_size_mask(signal, view)
	int signal, view;
{
# define PIP_Y_SCALE_REG_BITS	9
/*
# define PIP_Y_OVERRUN_SLOP	2
	if (view < signal)
		view -= PIP_Y_OVERRUN_SLOP;
 */
	/* y scale reg refers to line pairs */
	return compute_scale_reg(view/2, PIP_Y_SCALE_REG_BITS, signal/2);
}


/*
 * compute_scale_reg --
 * Given a size q, find a PIP scaling register value
 * to enable exactly q out of M (horizontal or vertical)
 * lines in the picture.
 *
 * The lines of the picture are numbered from [0..M-1].
 * The bits of the scaling register are numbered from [1..Mbits].
 * Each bit of the scaling register enables a (disjoint)
 * subset of lines in the picture.  The n-th bit enables
 * lines with numbers L_i = A_n/2 - 1 + i*A_n, where A_n = 2^n,
 * and i is any non-negative integer such that L_i < M.
 * Rearranging, we find
 *	i < (M-A_n/2+1)/A_n
 * so the number of L_i's in the n-th set is
 *	ceil((M-A/2+1)/A) = floor((M+A/2)/A)
 * since A_n is always a power of 2, the division may be accomplished
 * using shifts.
 */
int
compute_scale_reg(q, Mbits, M)
	int q; int Mbits; int M;
{
# define subsetsize(n, M)	(((M) + (1<<((n)-1))) >> (n))
	int mask; int n; int s;

	mask = 0;
	for (n = 1; n <= Mbits; n++)
		if ((s = subsetsize(n, M)) <= q)
			{ mask |= 1<<(Mbits-n); q -= s; }
	/* NOTE: q should always be 0 here unless q > M originally */
	return mask;
}

/* END DCC MOD */

#ifdef nomore /* THE CODE BELOW HAS AN OFF BY PROBLEM WHICH I HAVEN'T FIXED */

/*"pip_x_size_mask"
 *
 *	Calculate the mask value for the pip x size register that will produce a proper-sized 
 *	viewing image, given the size of the video signal image. The sizing mechanism is
 *	implemented with a rate multiplier, each bit in the mask reduces the original image
 *	by one over a power of two (i.e. 1/2, 1/4, 1/8, 1/16, etc.)
 *
 *		= mask value to use
 */
pip_x_size_mask(signal, view)
    int signal;	/* = current size of signal image in pixels in x direction. */
    int view;	/* = desired size of viewing image in pixels in x direction. */
{

/* 
**
**  FORMULA
**
**    n = mask bit
**    i = 10-n (n=9..0)
**
**    BITS ENABLED ACCORDING TO HARDWARE SPEC
**    a IS FIRST BIT ASSERTED; b IS THE INCREMENT TO THE NEXT BIT ASSERTED
**
**    a = 2**(i-1)-1
**    b = 2**i
**
**    pixels IS THE NUMBER OF PIXELS THAT MASK BIT n ENABLES FOR THE
**    GIVEN SIGNAL SIZE signal; NOTE THERE ARE HOLES
**
**    pixels = (signal - 1 - a)/b + 1
**
**    FOR PURPOSES OF ITERATION NOTE THAT
**
**    a[i] = b[i-1] - 1; THUS
**
**    pixels[i] = (signal - 1 - (b[i-1]-1))/b[i] + 1 OR
**
**    pixels[i] = (signal - b[i-1])/b[i] + 1 AND LET
**
**    t = (signal - b[i-1])
*/

  int mask;	/* Mask value to return to our caller. */
  int b,t;
  int pixels;
  int blog;     /* blog is log2(b) so we can use shifts instead of div */
  
  if (view > signal) view = signal;

  /* THIS IS TO MAKE SURE THAT RATE MULTIPLIER ERRORS DON'T CAUSE BOUNDS
     TO BE OVERRUN */

  if (view < signal) view-=2;
  
  t = signal - 1;
  b = 2;
  mask = 0;
  
  for (blog = 1; blog < 11; blog++)
    {
      mask <<= 1;

      pixels = (t >> blog) + (b-1 < signal);
      t = signal - b;
      b <<= 1;
      
      if ( /* pixels && */ view >= pixels )
        {
	  mask |= 1;
	  view -= pixels;
        }
      
    }
  
  return mask;

}

/*"pip_y_size_mask"
 *
 *	Calculate the mask value for the pip y size register that will produce a proper-sized 
 *	viewing image, given the size of the video signal image. The sizing mechanism is
 *	implemented with a rate multiplier, each bit in the mask reduces the original image
 *	by one over a power of two (i.e. 1/2, 1/4, 1/8, 1/16, etc.)
 *
 *		= mask value to use
 */
pip_y_size_mask(signal, view)
    int signal; /* = current size of signal image in pixels in y direction. */
    int view;	/* = desired size of viewing image in pixels in y direction. */
{

/* 
**
**  FORMULA
**
**    n = mask bit
**    i = 9-n (n=8..0)
**
**    BITS ENABLED ACCORDING TO HARDWARE SPEC
**    a IS FIRST BIT ASSERTED; b IS THE INCREMENT TO THE NEXT BIT ASSERTED
**
**    a = 2**(i-1)-1
**    b = 2**i
**
**    pixels IS THE NUMBER OF PIXELS THAT MASK BIT n ENABLES FOR THE
**    GIVEN SIGNAL SIZE signal; NOTE THERE ARE HOLES
**
**    pixels = (signal - 1 - a)/b + 1
**
**    FOR PURPOSES OF ITERATION NOTE THAT
**
**    a[i] = b[i-1] - 1; THUS
**
**    pixels[i] = (signal - 1 - (b[i-1]-1))/b[i] + 1 OR
**
**    pixels[i] = (signal - b[i-1])/b[i] + 1 AND LET
**
**    t = (signal - b[i-1])
*/

  int mask;	/* Mask value to return to our caller. */
  int b,t;
  int pixels;
  int blog;     /* blog is log2(b) so we can use shifts instead of div */
  
  if (view > signal) view = signal;

  signal >>= 1;
  view >>= 1;

  /* THIS IS TO MAKE SURE THAT RATE MULTIPLIER ERRORS DON'T CAUSE BOUNDS
     TO BE OVERRUN */

/*  if (view < signal) view-=2; */

  t = signal - 1;
  b = 2;
  mask = 0;
  
  for (blog = 1; blog < 10; blog++)
    {
      
      mask <<= 1;

      pixels = (t >> blog) + (b-1 < signal);
      t = signal - b;
      b <<= 1;
      
      if ( /* pixels && */ view >= pixels )
        {
	  mask |= 1;
	  view -= pixels;
        }
      
    }

  return mask;

}
#endif nomore






