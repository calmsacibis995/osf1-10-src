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
static char rcsid[] = "@(#)$RCSfile: _c_clean.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:27:59 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "_c_clean.c	1.6  com/lib/curses,3.1,8943 10/16/89 22:53:28";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _c_clean
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
 * NAME:        _c_clean
 *
 * FUNCTION:    Clear screen.
 *
 */

_c_clean ()
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_c_clean().\n");
#endif
	_hlmode (0);
	_kpmode(0);
	SP->virt_irm = 0;
	_window(0, lines-1, 0, columns-1);
	_syncmodes();
	tputs(exit_ca_mode, 0, _outch);
	tputs(cursor_normal, 0, _outch);
}
