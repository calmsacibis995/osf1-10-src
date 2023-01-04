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
static char rcsid[] = "@(#)$RCSfile: chktypeahd.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 21:04:07 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "chktypeahd.c  1.5  com/lib/curses,3.1,8943 10/16/89 23:06:01";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _chk_typeahead
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
extern	int InputPending;

/*
 * NAME:        _chk_typeahead
 *
 * FUNCTION:
 *
 *      If it's been long enough, check to see if we have any typeahead
 *      waiting.  If so, we quit this update until next time.
 */

_chk_typeahead()
{

#ifdef FIONREAD
# ifdef DEBUG
if(outf) fprintf(outf,
"end of _id_char: --SP->check_input %d, InputPending %d, chars buffered %d: ",
SP->check_input-1, InputPending, (SP->term_file->_ptr-SP->term_file->_base));
# endif
	if(--SP->check_input<0 && !InputPending &&
	    ((SP->term_file->_ptr - SP->term_file->_base) > 20)) {
		__cflush();
		if (SP->check_fd >= 0)
			ioctl(SP->check_fd, FIONREAD, &InputPending);
		else
			InputPending = 0;
		SP->check_input = SP->baud / 2400;
# ifdef DEBUG
		if(outf) fprintf(outf,
		"flush, ioctl returns %d, SP->check_input set to %d\n",
			InputPending, SP->check_input);
# endif
	}
# ifdef DEBUG
	if(outf) fprintf(outf, ".\n");
# endif
#endif
}
