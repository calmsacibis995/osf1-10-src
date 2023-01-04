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
static char rcsid[] = "@(#)$RCSfile: flushinp.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 21:28:15 $";
#endif
/*
 * HISTORY
 */
/***
 ***  "flushinp.c  1.5  com/lib/curses,3.1,8943 10/16/89 23:18:13";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   flushinp
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cursesext.h"

/*
 * NAME:        flushinp
 */

flushinp()
{
#ifdef DEBUG
	if(outf) fprintf(outf,
		"flushinp(), file %x, SP %x\n", SP->term_file, SP);
#endif
#ifdef USG
	ioctl(cur_term -> Filedes, TCFLSH, 0);
#else
	/* for insurance against someone using their own buffer: */
	ioctl(cur_term -> Filedes, TIOCGETP, &(cur_term->Nttyb));

	/*
	 * SETP waits on output and flushes input as side effect.
	 * Really want an ioctl like TCFLSH but Berkeley doesn't have one.
	 */
	ioctl(cur_term -> Filedes, TIOCSETP, &(cur_term->Nttyb));
#endif
	/*
	 * Have to doupdate() because, if we've stopped output due to
	 * typeahead, now that typeahead is gone, so we'd better catch up.
	 */
	doupdate();
}
