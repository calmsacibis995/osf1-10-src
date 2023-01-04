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
static char rcsid[] = "@(#)$RCSfile: _sethl.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:48:54 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "_sethl.c  1.6  com/lib/curses,3.1,8943 10/16/89 23:00:24";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _sethl
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
 * NAME:        _sethl
 */

_sethl ()
{
#ifdef DEBUG
	if(outf) fprintf(outf,
		"_sethl().  SP->phys_gr=%o, SP->virt_gr %o\n",
			SP->phys_gr, SP->virt_gr);
#endif
#ifdef	 	VIDEO
	if (SP->phys_gr == SP->virt_gr)
		return;
	vidputs(SP->virt_gr, _outch);
	SP->phys_gr = SP->virt_gr;
	/* Account for the extra space the cookie takes up */
	if (magic_cookie_glitch >= 0)
		SP->phys_x += magic_cookie_glitch;
#endif 		/* VIDEO */
}
