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
/*	
 *	@(#)$RCSfile: cram.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:08:11 $
 */ 
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
 * cram.h
 */
 
/*
 **********************************************************************
 * Copyright (C) 1988 by Olivetti.  All rights reserved.              *
 **********************************************************************
 */

/* 
 * outb(CMOS_ADDR, addr);
 * result = inb(CMOS_DATA);
 * 
 * where "addr" tells what value you want to read (some are listed 
 * below).  Interrupts should be disabled while you do this.
 */

/* I/O ports */

#define CMOS_ADDR	0x70		/* port for CMOS ram address */
#define CMOS_DATA	0x71		/* port for CMOS ram data */


/* Addresses, related masks, and potential results */

#define CMOS_EB		0x14		/* read Equipment Byte */
#define CM_SCRMSK	0x30	/* mask for EB query to get screen */
#define CM_EGA_VGA	0x00	/* "not CGA or MONO" */
#define CM_CGA_40	0x10
#define CM_CGA_80	0x20
#define CM_MONO_80	0x30

