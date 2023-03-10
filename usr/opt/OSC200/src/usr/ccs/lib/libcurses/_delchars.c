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
static char rcsid[] = "@(#)$RCSfile: _delchars.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:31:53 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "_delchars.c	1.6  com/lib/curses,3.1,8943 10/16/89 22:54:48";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _delchars
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

extern	int	_outch();

/*
 * NAME:        _delchars
 *
 * FUNCTION:
 *
 *      Delete n characters.
 *
 */

_delchars (n)
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_delchars(%d).\n", n);
#endif
	if (enter_delete_mode) {
		if (strcmp(enter_delete_mode, enter_insert_mode) == 0) {
			if (SP->phys_irm == 0) {
				tputs(enter_delete_mode,1,_outch);
				SP->phys_irm = 1;
			} 
		}
		else {
			if (SP->phys_irm == 1) {
				tputs(exit_insert_mode,1,_outch);
				SP->phys_irm = 0;
			}
			tputs(enter_delete_mode,1,_outch);
		}
	}
	while (--n >= 0) {
		/* Only one line can be affected by our delete char */
		tputs(delete_character, 1, _outch);
	}
	if (exit_delete_mode) {
		if (strcmp(exit_delete_mode, exit_insert_mode) == 0)
			SP->phys_irm = 0;
		else
			tputs(exit_delete_mode, 1, _outch);
	}
}
