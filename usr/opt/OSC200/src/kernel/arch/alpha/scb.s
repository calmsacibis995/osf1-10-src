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
	.rdata
	.asciiz "@(#)$RCSfile: scb.s,v $ $Revision: 1.2.2.7 $ (DEC) $Date: 1992/08/28 09:15:53 $"
	.text

/*
 * Modification History:
 *
 * 13-Sep-91 prm (Peter Mott)
 *      Add a shadow scb, _Xscb, for use as a data structure to
 *      redirect interrupts through a generic insterrupt service
 *      routine, vector_dispatch.
 *
 * 24-Sep-90 rjl
 *
 *	Initial version for alpha
 *
 *	The alpha scb is an 8kb block of entries.  Each entry consist of
 *	the address of an exception handler and a quad word parameter.
 *	Pal_code vectors through this data structure to reach the handler.
 *
 */

#include <machine/trap.h>

#define SE(routine,parameter)	.quad	_X/**/routine; .quad	parameter
#define STRAY(parameter) .quad stray; .quad parameter;

	.text
	.globl	_scb
_scb:
/* 000 */	STRAY(0);		STRAY(0x10);
/* 020 */	STRAY(0x20);		STRAY(0x30);
/* 040 */	STRAY(0x40);		STRAY(0x50);
/* 060 */	STRAY(0x60);		STRAY(0x70);
/* 080 */	STRAY(0x80);		STRAY(0x90);
/* 0a0 */	STRAY(0xa0);		STRAY(0xb0);
/* 0c0 */	STRAY(0xc0);		STRAY(0xd0);
/* 0e0 */	STRAY(0xe0);		STRAY(0xf0);

/* 100 */	STRAY(0x100);		STRAY(0x110);
/* 120 */	STRAY(0x120);		STRAY(0x130);
/* 140 */	STRAY(0x140);		STRAY(0x150);
/* 160 */	STRAY(0x160);		STRAY(0x170);
/* 180 */	STRAY(0x180);		STRAY(0x190);
/* 1a0 */	STRAY(0x1a0);		STRAY(0x1b0);
/* 1c0 */	STRAY(0x1c0);		STRAY(0x1d0);
/* 1e0 */	STRAY(0x1e0);		STRAY(0x1f0);

/* 200 */	STRAY(0x200);		STRAY(0x210);
/* 220 */	STRAY(0x220);		STRAY(0x230);
/* 240 */	STRAY(0x240);		STRAY(0x250);
/* 260 */	STRAY(0x260);		STRAY(0x270);
/* 280 */	STRAY(0x280);		STRAY(0x290);
/* 2a0 */	STRAY(0x2a0);		STRAY(0x2b0);
/* 2c0 */	STRAY(0x2c0);		STRAY(0x2d0);
/* 2e0 */	STRAY(0x2e0);		STRAY(0x2f0);
/* 300 */	STRAY(0x300);		STRAY(0x310);
/* 320 */	STRAY(0x320);		STRAY(0x330);
/* 340 */	STRAY(0x340);		STRAY(0x350);
/* 360 */	STRAY(0x360);		STRAY(0x370);
/* 380 */	STRAY(0x380);		STRAY(0x390);
/* 3a0 */	STRAY(0x3a0);		STRAY(0x3b0);
/* 3c0 */	STRAY(0x3c0);		STRAY(0x3b0);
/* 3e0 */	STRAY(0x3e0);		STRAY(0x3d0);

/* 400 */	STRAY(0x400);		STRAY(0x410);
/* 420 */	STRAY(0x420);		STRAY(0x430);
/* 440 */	STRAY(0x440);		STRAY(0x450);
/* 460 */	STRAY(0x460);		STRAY(0x470);
/* 480 */	STRAY(0x480);		STRAY(0x490);
/* 4a0 */	STRAY(0x4a0);		STRAY(0x4b0);
/* 4c0 */	STRAY(0x4c0);		STRAY(0x4d0);
/* 4e0 */	STRAY(0x4e0);		STRAY(0x4f0);

/* 500 */	STRAY(0x500);		STRAY(0x510);

/* 520 */	STRAY(0x520);		STRAY(0x530);
/* 540 */	STRAY(0x540);		STRAY(0x550);
/* 560 */	STRAY(0x560);		STRAY(0x570);
/* 580 */	STRAY(0x580);		STRAY(0x590);
/* 5a0 */	STRAY(0x5a0);		STRAY(0x5b0);
/* 5c0 */	STRAY(0x5c0);		STRAY(0x5d0);
/* 5e0 */	STRAY(0x5e0);		STRAY(0x5f0);

/* 600 */	STRAY(0x600);		STRAY(0x610);
/* 620 */	STRAY(0x620);		STRAY(0x630);
/* 640 */	STRAY(0x640);		STRAY(0x650);
/* 660 */	STRAY(0x660);		STRAY(0x670);
/* 680 */	STRAY(0x680);		STRAY(0x690);
/* 6a0 */	STRAY(0x6a0);		STRAY(0x6b0);
/* 6c0 */	STRAY(0x6c0);		STRAY(0x6d0);
/* 6e0 */	STRAY(0x6e0);		STRAY(0x6f0);

/* 700 */	STRAY(0x700); 		STRAY(0x710);
/* 720 */	STRAY(0x720);		STRAY(0x730);
/* 740 */	STRAY(0x740); 		STRAY(0x750);
/* 760 */	STRAY(0x760);		STRAY(0x770);
/* 780 */	STRAY(0x780); 		STRAY(0x790);
/* 7a0 */	STRAY(0x7a0);		STRAY(0x7b0);
/* 7c0 */	STRAY(0x7c0); 		STRAY(0x7d0);
/* 7e0 */	STRAY(0x7e0);		STRAY(0x7f0);

/*
 * I/O Space vectors
 */
	.globl	_scb_io
_scb_io:

/* 800 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 840 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 880 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 8c0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* 900 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 940 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 980 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 9c0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* a00 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* a40 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* a80 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* ac0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* b00 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* b40 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* b80 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* bc0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* c00 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* c40 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* c80 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* cc0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* d00 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* d40 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* d80 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* dc0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* e00 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* e40 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* e80 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* ec0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* f00 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* f40 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* f80 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* fc0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* 1000 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1040 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1080 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 10c0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* 1100 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1140 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1180 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 11c0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* 1200 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1240 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1280 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 12c0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* 1300 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1340 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1380 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 13c0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* 1400 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1440 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1480 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 14c0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* 1500 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1540 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1580 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 15c0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* 1600 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1640 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1680 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 16c0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* 1700 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1740 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1780 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 17c0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* 1800 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1840 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1880 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 18c0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* 1900 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1940 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1980 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 19c0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* 1a00 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1a40 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1a80 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1ac0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* 1b00 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1b40 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1b80 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1bc0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* 1c00 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1c40 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1c80 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1cc0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* 1d00 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1d40 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1d80 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1dc0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* 1e00 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1e40 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1e80 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1ec0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);

/* 1f00 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1f40 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1f80 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
/* 1fc0 */ STRAY(-1); STRAY(-1); STRAY(-1); STRAY(-1);
	.globl _scbend
_scbend:

