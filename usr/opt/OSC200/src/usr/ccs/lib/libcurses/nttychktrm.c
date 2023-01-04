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
static char rcsid[] = "@(#)$RCSfile: nttychktrm.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/12 22:14:21 $";
#endif
/*
 * HISTORY
 */
/*** "nttychktrm.c  1.7  com/lib/curses,3.1,9008 12/4/89 21:02:36"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _new_tty, chk_trm
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

static int chk_trm();		/* predefine to remove warnings -- TEJ */

char *_c_why_not = NULL;
static char *
_stupid = "Sorry, I don't know how to deal with your '%s' terminal.\r\n";
static char *
_unknown =
"Sorry, I need to know a more specific terminal type than '%s'.\r\n";

/*
 * NAME:        _new_tty
 */

struct screen *
_new_tty(type, fd)
char	*type;
FILE	*fd;
{
	int retcode;
	char *calloc();

#ifdef DEBUG
	if(outf) fprintf(outf, "__new_tty: type %s, fd %x\n", type, fd);
#endif
	/*
	 * Allocate an SP structure if there is none, or if SP is
	 * still pointing to an old structure from a previous call
	 * to this routine.  But don't allocate one if we are being
	 * called from a higher level curses routine.  Since it's our
	 * job to initialize the phys_scr field and higher level routines
	 * aren't supposed to, we check that field to figure which to do.
	 */
	if (SP == NULL || SP->cur_body!=0)
		SP = (struct screen *) calloc(1, sizeof (struct screen));
	SP->term_file = fd;
	if (type == 0)
		type = "unknown";
	_setbuffered(fd);
	setupterm(type, fileno(fd), &retcode);
	if (retcode < 0) {
		/*
		 * This happens if /usr/lib/terminfo doesn't exist, there is
		 * no such terminal type, or the file is corrupted.
		 * This would be a good place to print an error message.
		 */
		return NULL;
	}
	savetty();	/* as a "useful default" - hanging up is nasty. */
	if (chk_trm() == ERR)
		return NULL;
	SP->tcap = cur_term;
	SP->doclear = 1;
	SP->cur_body=(struct line **) calloc(lines+2, sizeof(struct line *));
	SP->std_body=(struct line **) calloc(lines+2, sizeof(struct line *));
#ifdef KEYPAD
	SP->kp = (struct map *)_init_keypad();
#endif
	SP->input_queue = (_SHORT *) calloc(20, sizeof (_SHORT));
	SP->input_queue[0] = -1;

	SP->virt_x = 0;		/* X and Y coordinates of the SP->curptr */
	SP->virt_y = 0;		/* between updates. */
	_init_costs();
	return SP;
}

/*
 * NAME:        chk_trm
 */

static int
chk_trm()
{
#ifdef DEBUG
	if(outf) fprintf(outf, "chk_trm().\n");
#endif

	if (generic_type) {
		_c_why_not = _unknown;
		return ERR;
	}
	if (clear_screen == 0 || hard_copy || over_strike) {
		_c_why_not = _stupid;
		return ERR;
	}
	if (cursor_address) {
		/* we can handle it */
	} else 
	if ( (cursor_up || cursor_home)	/* some way to move up */
	    && cursor_down		/* some way to move down */
	    && (cursor_left || carriage_return)	/* ... move left */
					/* printing chars moves right */
		) {
		/* we can handle it */
	} else {
		_c_why_not = _stupid;
		return ERR;
	}
	return OK;
}
