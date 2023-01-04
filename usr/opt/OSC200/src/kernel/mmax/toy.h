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
 *	@(#)$RCSfile: toy.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:43:24 $
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *        Copyright 1985 Encore Computer Corporation
 *
 * ALL RIGHTS RESERVED. Licensed Material - Property of Encore Computer
 * Corporation. This software is made available solely pursuant to the
 * terms of a software license agreement which governs its use. 
 * Unauthorized duplication, distribution or sale are strictly prohibited.
 *
 * Include file description:
 *
 * This file defines the toy data structure.  The structure is used to
 * 'SET' and 'GET' time from the RTC on the SCC.
 */


/* The toy structure is used to 'SET' and 'GET' the contents of the
 * real time clock data on the SCC board.  In addition, the SCC uses
 * battery backed up memory to maintain the high two decimal digits of
 * the year and the offset from Greenwich mean.
 */

typedef	struct toy {
 	char toy_tenths_sec;   	/* Note that the tenths are not settable */
	char toy_unit_sec;	/* each char toy_contains the digit in the */
        char toy_ten_sec;	/* lower four bits. */
        char toy_unit_min;
        char toy_ten_min;
        char toy_unit_hour;
        char toy_ten_hour;
        char toy_unit_day;
        char toy_ten_day;
        char toy_unit_month;
        char toy_ten_month;
        char toy_unit_year;
        char toy_ten_year;
        char toy_day_of_week;

/* data not present on real time clock chip */

	char toy_hun_year;
	char toy_thou_year;

	short toy_offset;		/* minutes from Greenwich mean */
} TOY;

