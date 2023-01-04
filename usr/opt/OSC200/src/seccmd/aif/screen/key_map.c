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
static char	*sccsid = "@(#)$RCSfile: key_map.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:58:05 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1989 SecureWare, Inc.  All rights reserved.
 */



/* This file contains key mapping data structures & routines.
 */

#include	"userif.h"
#include	"key_map.h"
#include	"logging.h"

#ifdef DEBUG
extern	FILE	*logfp;
#endif DEBUG

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


void init_key_conv (erase)
int erase;
{
	key_conv[BS_INDEX].in = erase;
	DUMPLVARS ("erase char = <%x>", key_conv[BS_INDEX].in, 0, 0);
}



int get_key_conv (w)
WINDOW *w;
{
	int ch;		/* char input */
	int cch;	/* converted char */
	register key_map *kcp;

	/* flush readahead to avoid re-execute screen problem */
	flushinp ();
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
