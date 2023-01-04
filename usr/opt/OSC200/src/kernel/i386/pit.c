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
static char	*sccsid = "@(#)$RCSfile: pit.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:19:34 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0
 */

/*
 *  Copyright 1988, 1989 by Intel Corporation
 *
 *         INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *     This software is supplied under the terms of a license 
 *    agreement or nondisclosure agreement with Intel Corpo-
 *    ration and may not be copied or disclosed except in
 *    accordance with the terms of that agreement.
 */

#include <cputypes.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <sys/table.h>
#include <i386/pit.h>
#include <i386/ipl.h>
#include <i386/handler.h>
#include <i386/dispatcher.h>


int pitctl_port  = PITCTL_PORT;		/* For 386/20 Board */
int pitctr0_port = PITCTR0_PORT;	/* For 386/20 Board */
int pitctr1_port = PITCTR1_PORT;	/* For 386/20 Board */
int pitctr2_port = PITCTR2_PORT;	/* For 386/20 Board */
/* We want PIT 0 in square wave mode */

int pit0_mode = PIT_C0|PIT_SQUAREMODE|PIT_READMODE ;

unsigned int delaycount;		/* loop count in trying to delay for
					 * 1 millisecond
					 */
unsigned long microdata=50;		/* loop count for 10 microsecond wait.
					   MUST be initialized for those who
					   insist on calling "tenmicrosec"
					   it before the clock has been
					   initialized.
					 */
unsigned int clknumb = CLKNUM;		/* interrupt interval for timer 0 */

#ifdef PS2
#include <i386/PS2/abios.h>
static struct generic_request *clock_request_block;
static int     clock_flags;
char cqbuf[200];	/*XXX temparary.. should use kmem_alloc or whatever..*/
#endif

extern  int i386_hardclock();
static ihandler_t clk_handler;
static ihandler_id_t *clk_handler_id;

clkstart()
{
	unsigned int	flags;
	unsigned char	byte;
	int s;
#ifdef PS2
	struct generic_request  temp_request_block;
	int rc;
#endif

	findspeed();
	microfind();
	s = sploff();         /* disable interrupts */
#ifdef PS2
	temp_request_block.r_current_req_blck_len = ABIOS_MIN_REQ_SIZE;
	temp_request_block.r_logical_id = abios_next_LID(SYSTIME_ID,2);
	temp_request_block.r_unit = 0;
	temp_request_block.r_function = ABIOS_LOGICAL_PARAMETER;
	temp_request_block.r_return_code = ABIOS_UNDEFINED;

	abios_common_start(&temp_request_block,0);
	if (temp_request_block.r_return_code != ABIOS_DONE) {
		panic("could not init abios time code!\n");
	}

	/*
	 * now build the clock request for the hardware system clock
	 */
	clock_request_block = (struct generic_request *)cqbuf;
	clock_request_block->r_current_req_blck_len =
				temp_request_block.r_request_block_length;
	clock_request_block->r_logical_id = temp_request_block.r_logical_id;
	clock_request_block->r_unit = 0;
	clock_request_block->r_function = ABIOS_DEFAULT_INTERRUPT;
	clock_request_block->r_return_code = ABIOS_UNDEFINED;
	clock_flags = temp_request_block.r_logical_id_flags;

#endif
	/* Since we use only timer 0, we program that.
	 * 8254 Manual specifically says you do not need to program
	 * timers you do not use
	 */
	outb(pitctl_port, pit0_mode);
#if	defined(MB1) || defined(MB2) || EXL > 0
	clknumb = CLKNUM;
#else	
	clknumb = CLKNUM/hz;
#endif
	byte = clknumb;
	outb(pitctr0_port, byte);
	byte = clknumb>>8;
	outb(pitctr0_port, byte); 

	/* splon does not nest with splhigh (in handler_add). */
	splon(s);         /* restore interrupt state */

	/*
	 * Hardclock interrupts currently are detected in locore.s
	 * and do not go via the dispatcher, but we install it for
	 * sanity's sake anyway. The reasons for the direct handling
	 * are speed plus the stack frame has different requirements
	 * from a normal interrupt routine.
	 *
	 * Note: intpri[0] is statically initialized to SPLHI.
	 */
	clk_handler.ih_level = 0;
	clk_handler.ih_handler = i386_hardclock;
	clk_handler.ih_resolver = (int (*)()) NULL;
	clk_handler.ih_stats.intr_type = INTR_HARDCLK;
	clk_handler.ih_stats.intr_cnt = 0;
	clk_handler.ih_hparam[0].intparam = 0;
	if ((clk_handler_id = handler_add(&clk_handler)) != NULL)
		handler_enable(clk_handler_id);
	else
		panic("Unable to add clock interrupt handler");

#ifdef PS2
	int_enable(0,SPLHI);
#endif
}

#ifdef PS2
ackrtclock()
{
	if (clock_request_block) {
		clock_request_block->r_return_code = ABIOS_UNDEFINED;
		abios_common_interrupt(clock_request_block,clock_flags);
	}
}
#endif

#define COUNT	0x2000

findspeed()
{
	unsigned int flags;
	unsigned char byte;
	unsigned int leftover;
	int i;
	int j;
	int s;

	s = sploff();                 /* disable interrupts */
	/* Put counter in count down mode */
#define PIT_COUNTDOWN PIT_READMODE|PIT_NDIVMODE
	outb(pitctl_port, PIT_COUNTDOWN);
	/* output a count of -1 to counter 0 */
	outb(pitctr0_port, 0xff);
	outb(pitctr0_port, 0xff);
	delaycount = COUNT;
	spinwait(1);
	/* Read the value left in the counter */
	byte = inb(pitctr0_port);	/* least siginifcant */
	leftover = inb(pitctr0_port);	/* most significant */
	leftover = (leftover<<8) + byte ;
	/* Formula for delaycount is :
	 *  (loopcount * timer clock speed)/ (counter ticks * 1000)
	 * 1000 is for figuring out milliseconds 
	 */
	delaycount = (((COUNT * CLKNUM)/1000) * hz) / (0xffff-leftover);
	splon(s);         /* restore interrupt state */
}

spinwait(millis)
	int millis;		/* number of milliseconds to delay */
{
	int i, j;

	for (i=0;i<millis;i++)
		for (j=0;j<delaycount;j++)
			;
}

#define MICROCOUNT	0x2000

microfind()
{
	unsigned int flags;
	unsigned char byte;
	unsigned short leftover;
	int s;


	s = sploff();                 /* disable interrupts */

	/* Put counter in count down mode */
	outb(pitctl_port, PIT_COUNTDOWN);
	/* output a count of -1 to counter 0 */
	outb(pitctr0_port, 0xff);
	outb(pitctr0_port, 0xff);
	microdata=MICROCOUNT;
	tenmicrosec();
	/* Read the value left in the counter */
	byte = inb(pitctr0_port);	/* least siginifcant */
	leftover = inb(pitctr0_port);	/* most significant */
	leftover = (leftover<<8) + byte ;
	/* Formula for delaycount is :
	 *  (loopcount * timer clock speed)/ (counter ticks * 1000)
	 *  Note also that 1000 is for figuring out milliseconds
	 */
	microdata = (MICROCOUNT * CLKNUM) / ((0xffff-leftover)*(100000/hz));
	if (!microdata)
		microdata++;
	splon(s);         /* restore interrupt state */
}

#if	EXL
/* added micro-timer support.   --- csy */

extern unsigned ts_tick_count;

/* don't need these for now.   --- csy
extern time_t lbolt;
extern time_t lsec;
extern time_t time;
*/
static long last_lbolt;
static long last_msec;
static short missed_intr = 0;

stop_watch(p_time_latch)
time_latch *p_time_latch;
{
	unsigned char lsb, status;
	unsigned short msb;

	intr_disable();                   /* disable interrupts */

	outb ( pitctl_port, 0xC2 );       /* latch count & status */
	status = inb(pitctr0_port) & 0x80;    /* read status byte */
	lsb = inb(pitctr0_port);	  /* least signifcant */
	msb = inb(pitctr0_port);	  /* most significant */
	msb = CLKNUM - ((msb<<8) + lsb);  /* what's left in counter */
	msb = ( short ) msb;              /* what's left in .8 micro secs */

	p_time_latch->uticks = msb;
/* don't need secs and epochsecs for now.
 * lbolt is used for different purpose under MACH and ts_tick_count is
 * the equivalent of system V lbolt.   --- csy*/
/*	p_time_latch->secs = lsec;
 *	p_time_latch->epochsecs = time;
 */
	if ( missed_intr == 1 && last_lbolt == ts_tick_count && msb < last_msec )
		p_time_latch->ticks = last_lbolt = ts_tick_count + 1;
	else
		p_time_latch->ticks = last_lbolt = ts_tick_count;

	last_msec = msb;
	missed_intr = ( status == 0x80 ) ? 1 : 0;

	intr_restore();                   /* restore interrupt state */
	
}
#endif	/* EXL */
