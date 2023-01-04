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
static char rcsid[] = "@(#)$RCSfile: nonl.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/12 22:11:49 $";
#endif
/*
 * HISTORY
 */
/*** "nonl.c  1.7  com/lib/curses,3.1,9008 1/17/90 14:25:17"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   nonl
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        "cursesext.h"

/*
 * NAME:        nonl
 */

nonl()	
{


#ifdef USG
	(cur_term->Nttyb).c_iflag &= ~ICRNL;
	(cur_term->Nttyb).c_oflag &= ~ONLCR;

# ifdef DEBUG
	if(outf) fprintf(outf,
		"nonl(), file %x, SP %x, flags %x,%x\n",
		SP->term_file, SP, cur_term->Nttyb.c_iflag,
		cur_term->Nttyb.c_oflag);
# endif
#else
	(cur_term->Nttyb).sg_flags &= ~CRMOD;
# ifdef DEBUG
	if(outf) fprintf(outf,
		"nonl(), file %x, SP %x, flags %x\n",
		SP->term_file, SP, cur_term->Nttyb.sg_flags);
# endif
#endif
	_reset_prog_mode();					/* 001 */
}
