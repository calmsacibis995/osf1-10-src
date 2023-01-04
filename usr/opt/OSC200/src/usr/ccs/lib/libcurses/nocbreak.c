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
static char rcsid[] = "@(#)$RCSfile: nocbreak.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/12 22:08:54 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/*** "nocbreak.c  1.5  com/lib/curses,3.1,8943 10/16/89 23:31:45"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   nocbreak
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
 * NAME:        nocbreak
 */

nocbreak()
{
#ifdef USG
	(cur_term->Nttyb).c_lflag |= ICANON;
	(cur_term->Nttyb).c_cc[VEOF] = (cur_term->Ottyb).c_cc[VEOF];
	(cur_term->Nttyb).c_cc[VEOL] = (cur_term->Ottyb).c_cc[VEOL];
# ifdef DEBUG
	if(outf) fprintf(outf,
		"nocrmode(), file %x, SP %x, flags %x\n",
		SP->term_file, SP, cur_term->Nttyb.c_lflag);
# endif
#else
	(cur_term->Nttyb).sg_flags &= ~CBREAK;
# ifdef DEBUG
	if(outf) fprintf(outf,
		"nocrmode(), file %x, SP %x, flags %x\n",
		SP->term_file, SP, cur_term->Nttyb.sg_flags);
# endif
#endif
	SP->fl_rawmode=FALSE;
	_reset_prog_mode();					/* 001 */
}
