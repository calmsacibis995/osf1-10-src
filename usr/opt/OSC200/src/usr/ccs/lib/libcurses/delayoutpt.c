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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: delayoutpt.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 21:17:47 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "delayoutpt.c  1.5  com/lib/curses,3.1,8943 10/16/89 23:11:36";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   delay_output
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Code for various kinds of delays.  Most of this is nonportable and
 * requires various enhancements to the operating system, so it won't
 * work on all systems.  It is included in curses to provide a portable
 * interface, and so curses itself can use it for function keys.
 */


#include "cursesext.h"
#include <signal.h>

/*
 * NAME:        delay_output
 *
 * FUNCTION:
 *
 *      Delay the output for ms milliseconds.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Note that this is NOT the same as a high resolution sleep.  It will
 *      cause a delay in the output but will not necessarily suspend the
 *      processor.  For applications needing to sleep for 1/10th second,
 *      this is not a usable substitute.  It causes a pause in the displayed
 *      output, for example, for the eye wink in snake.  It is
 *      disrecommended for "delay" to be much more than 1/2 second,
 *      especially at high baud rates, because of all the characters it
 *      will output.  Note that due to system delays, the actual pause
 *      could be even more.
 *
 *      Some games won't work decently with this routine.
 */

delay_output(ms)
int ms;
{
	extern int _outchar();		/* it's in putp.c */

	return _delay(ms*10, _outchar);
}
