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
 *	"@(#)clock.h	9.1	(ULTRIX/OSF)	10/21/91"
 */
/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * Modification History: alpha/clock.h
 *
 * 26-Apr-91 -- afd
 *	Created this file for Alpha support.
 */


/*
 * Time define constants
 */
#define SECMIN		((unsigned)(60))		/* seconds per min */
#define SECHOUR		((unsigned)(60*SECMIN))		/* seconds per hr  */
#define	SECDAY		((unsigned)(24*SECHOUR))	/* seconds per day */
#define SECFEB		((unsigned)(28*SECDAY))		/* per Feb	   */
#define SECLFEB		((unsigned)(29*SECDAY))		/* per Feb (leap)  */
#define SECJUN		((unsigned)(30*SECDAY))		/* per short month */
#define SECJAN		((unsigned)(31*SECDAY))		/* per long month  */
#define	SECYR		((unsigned)(365*SECDAY))	/* per common year */
#define SECQYR		((unsigned)(4*SECYR+SECDAY))	/* per typical 4yr */
#define	UNITSPERSEC	((unsigned)(10000000))		/* 100ns units/sec */

/*
 * TODRZERO is what the TODR should contain when the ``year'' begins.
 * The TODR should always contain a number between [TODRZERO] and
 * [TODRZERO + "number of 100ns time units in a year"].
 * On Alpha systems, the TODR (AT) is kept as the number of 100ns time units.
 * The value 0x185C22CE4000 is the number of 100ns time units in 31 days.
 * We use this as a base value in the TODR to detect if the TODR has been reset.
 */
#define	TODRZERO	0x185c22ce4000

#define	YRREF		1970
#define	LEAPYEAR(year)	((year)%4==0)		/* good till the year 2400 */
