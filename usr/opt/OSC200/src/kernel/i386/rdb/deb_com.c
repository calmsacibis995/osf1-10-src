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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: deb_com.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:12:40 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1986,1990
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

#ifndef lint
#endif

/* #include "sa.h" */
#include "i386/rdb/com.h"

/*=========== ALL CODE BELOW INCLUDED FOR SERIAL DEBUG VERSION ==============*/
/*									   
 * This code allows all debugger i/o over a serial line to a 3101 terminal.
 * We accomplish this by overridding the normal getchar/putchar while
 * in the debugger with these routines.
 */


static kbd_init;

/*
 * Local getchar routine that uses the COM1 serial line interface.
 * Used to get all input for debugger over a serial line.
 */
com_getchar()
{
	register int c;

	if (!kbd_init)
		init_kbd();
/*	printf("com_getchar\n");	/* DEBUG - WEW */
	while ((in(COM1_LSR) & 01) == 0);
	c = in(COM1_RXB) & 0177;
/*	com_putchar(c);	*/
	return (c);
}

int com_baud = COM_2400;
/*
 * Local init_kbd routine that actually inits a serial line for 
 * input and output to debugger.
 */
static init_kbd()
{
	register int x;

	if (in(COM1_LCR) == 0xff) {
		printf("debug: no COM card at address %x \n", COM1);
		return;
	}
	out(COM1_MCR, 0x03);		/* set DTR, RTS */	
	out(COM1_LCR, 0x80);		/* enable for output to set baud rate */
	out(COM1_LSB, com_baud);	/* 9600 baud */
	out(COM1_MSB, 0x00);		/* set msb */
	out(COM1_LCR, 0x03);		/* 8 bits, 1 stop, no parity */
	out(COM1_IER, 0x00);		/* disable interrupts */
	x = in(COM1_RXB);
	++kbd_init;
}

/*
 * Local putchar routine that uses the COM1 serial line
 */
com_putchar(c)
	register int c;
{
	register int i;
	register int save_c;

	if (!kbd_init)
		init_kbd();
	/* for 3101 newline send both CR and LF	*/ 
	if (c == '\r' || c == '\n') {
		while ((in(COM1_LSR) & 040) == 0);  /* wait for ready */
		out(COM1_TXB, '\r');
		while ((in(COM1_LSR) & 040) == 0);  /* wait for ready */
		out(COM1_TXB, '\n');
	}
	else {
		if(c == '\t')
		{
		  save_c = c;
		  c = ' ';
		  for(i=0;i<5;i++)
		  {
		    while ((in(COM1_LSR) & 040) == 0);  /* wait for ready */
		    out(COM1_TXB, c);		    /* output the character */
		  }
		  c = save_c;
	        }
		else
		{
		    while ((in(COM1_LSR) & 040) == 0);  /* wait for ready */
		    out(COM1_TXB, c);		    /* output the character */
		}
	}
	return (c);

}

