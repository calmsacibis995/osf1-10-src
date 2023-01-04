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
static char rcsid[] = "@(#)$RCSfile: raw.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/12 22:20:15 $";
#endif
/*
 * HISTORY
 */
/*** 1.7  com/lib/curses/raw.c, , bos320, 9134320 7/19/91 16:06:48 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   raw
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
#include <unistd.h>		/* for POSIX_VDISABLE */

/*
 * NAME:        raw
 */

raw()
{
#ifdef USG
	/* Disable interrupt characters */
	(cur_term->Nttyb).c_cc[VINTR] = _POSIX_VDISABLE;
	(cur_term->Nttyb).c_cc[VQUIT] = _POSIX_VDISABLE;
	/* Allow 8 bit input/output */
	(cur_term->Nttyb).c_iflag &= ~ISTRIP;
	(cur_term->Nttyb).c_cflag &= ~CSIZE;
	(cur_term->Nttyb).c_cflag |= CS8;
	(cur_term->Nttyb).c_cflag &= ~PARENB;
	crmode();
#else
	(cur_term->Nttyb).sg_flags|=RAW;
#ifdef DEBUG
	if(outf) fprintf(outf,
		"raw(), file %x, SP %x, flags %x\n",
		SP->term_file, SP, cur_term->Nttyb.sg_flags);
#endif
	SP->fl_rawmode=TRUE;
#endif
	_reset_prog_mode();					/* 001 */
}
