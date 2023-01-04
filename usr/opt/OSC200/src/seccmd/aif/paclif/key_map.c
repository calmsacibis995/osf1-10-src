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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: key_map.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:12:03 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	key_map.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:04:30  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  16:59:21  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:19:46  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:52:31  marquard]
 * 
 * $OSF_EndLog$
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (c) 1989-90 SecureWare, Inc.
 *   All rights reserved.
 *
 * Based on OSF version:
 *	@(#)key_map.c	2.7 16:31:10 5/17/91 SecureWare
 */

/* #ident "@(#)key_map.c	1.1 11:17:01 11/8/91 SecureWare" */

/* This file contains key mapping data structures & routines.
 */

#include "If.h"
#include "AIf.h"
#include "scrn_local.h"

/* key definitions - depends on curses */


/*
	key_conv array - maps additional keys to standard names

	Variable entries go at start of array, final entry MUST
	be a pair of NULLs.
*/

typedef struct {
	int	in;
	int	out;
} key_map;

static key_map key_conv[] = {

#define BS_INDEX 0
	{NULL,		BACKSPACE},
	{DELETE,	BACKSPACE},
	{'\r',		EXECUTE},
	{'\n',		DOWN},
	{CTRL('V'),	INSTOGGLE},
	{CTRL('A'),	INSFIELD},
	{CTRL('R'),	INSFIELD},
	{CTRL('J'),	DOWN},
	{CTRL('K'),	UP},
	{CTRL('N'),	SCROLLDOWN},
	{CTRL('P'),	SCROLLUP},
	{ESCAPE,	QUITMENU},
	{KEY_ENTER,	EXECUTE},
	{NULL,		NULL}
};


void init_key_conv (erase_char)
int erase_char;
{
	key_conv[BS_INDEX].in = erase_char;
	DUMPLVARS ("erase char = <%x>", key_conv[BS_INDEX].in, 0, 0);
}



int get_key_conv (w)
WINDOW *w;
{
	int ch;		/* char input */
	int cch;	/* converted char */
	register key_map *kcp;


	/* flush readahead to avoid re-execute screen problem */
	/* (This is deleted to speed up input.  Left here in case
	 *  problems reoccur.)
	flushinp ();
	 */

	while ((ch = wgetch (w)) == 0) {
		/* toss any bogus input from interrupt */
	}

	DUMPVARS("get_key_conv: received 0%o 0x%x", ch, ch, NULL);

/*
	set cch to key pressed; if a match is found, stick correct key in cch
*/

	cch = ch;
	for (kcp = &(key_conv[0]); kcp->in != NULL; kcp++) {
		if (kcp->in == ch) {
			cch = kcp->out;
			break;
		}
	}
	return cch;
}
