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
static char *rcsid = "@(#)$RCSfile: prscr.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:13:27 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	prscr.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:05:08  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  16:59:54  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:20:23  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:53:34  marquard]
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
 * Copyright (c) 1989, 1990 SecureWare, Inc.  All rights reserved.
 *
 * Based on OSF version:
 *	@(#)prscr.c	1.5 16:31:16 5/17/91 SecureWare
 */

/* #ident "@(#)prscr.c	1.1 11:18:12 11/8/91 SecureWare" */

/* print screen routines */

#include "If.h"
#include "AIf.h"
#include "scrn_local.h"

extern FILE *fopen();

#define LINE_WIDTH 80

#ifdef PRINTSCR

static void print_prompt();
static void print_field();
static void print_scroll();
static void print_choice();
static void print_toggle();
static void print_text();

/* routine that prints a screen into the file "SWscreens" in current dir */

printscreen(screenp)
Scrn_parms *screenp;
{
	static FILE *psfp;
	static char **scr_mem;
	int i;
	register Scrn_desc *sdp;
	WINDOW *oldwin;

	ENTERFUNC ("printscreen");
	DUMPVARS ("screen title='%s'", screenp->sh->title, NULL, NULL);
	if (psfp == (FILE *) 0) {

		/* open output file */

		psfp = fopen("SWscreens", "w");
		if (psfp == (FILE * ) 0) {
			fprintf(stderr,
			  MSGSTR(PRSCR_1, "Could not open SWscreens for writing\n"));
			exit(1);
		}

		/* allocate memory to store screen */

		scr_mem = alloc_cw_table(LINES, LINE_WIDTH + 1);

		if (scr_mem == (char **) 0)
			MemoryError();

	}

	/* put spaces in entire screen buffer */

	for (i = 0; i < LINES; i++)
		memset(scr_mem[i], ' ', LINE_WIDTH);

	/* put beginning of screen marker */

	putc('B', psfp);
	for (i = 0; i < LINE_WIDTH - 2; i++)
		putc('-', psfp);
	putc('B', psfp);
	putc('\n', psfp);


	/*
	 * initialize the screen (sets up number rows, etc.)
	 * avoid curses problems by setting window to invalid value
	 */

	oldwin = screenp->w;
	screenp->w = (WINDOW *) 1;
	initscreen(screenp, 0);
	screenp->w = oldwin;

	/* adjust to the amount centered by the screen */

	for (i = 0; i < screenp->ndescs; i++)
		screenp->sd[i].col += screenp->leftcol;

	/* put screen title */

	if (screenp->sh->title) {
		int len = strlen(screenp->sh->title);

		memcpy(scr_mem[0] + (LINE_WIDTH - len) / 2, screenp->sh->title,
			strlen(screenp->sh->title));
	}

	/* put screen descs */

	sdp = screenp->sd;
	for (i = 0; i < screenp->ndescs; i++, sdp++) {
		char *row_ptr = scr_mem[sdp->row + 2];
		switch (sdp->type) {
		case FLD_PROMPT:
			print_prompt(row_ptr, sdp);
			break;
		case FLD_ALPHA:
		case FLD_NUMBER:
			print_field(row_ptr, sdp);
			break;
		case FLD_SCROLL:
		case FLD_SCRTOG:
			print_scroll(scr_mem, sdp);
			break;
		case FLD_CHOICE:
			print_choice(row_ptr, sdp);
			break;
		case FLD_SKIP:
			break;
		case FLD_TOGGLE:
			print_toggle(row_ptr, sdp);
			break;
		case FLD_TEXT:
			print_text(scr_mem, sdp);
			break;
		}
	}


	/* print the lines at the bottom of the screen */

	if (screenp->sh->c3)
		memcpy(scr_mem[2 + screenp->nbrrows + 0] +
			(LINE_WIDTH - strlen(screenp->sh->c3)) / 2,
		       screenp->sh->c3, strlen(screenp->sh->c3));
	if (screenp->sh->c2)
		memcpy(scr_mem[2 + screenp->nbrrows + 1] +
			(LINE_WIDTH - strlen(screenp->sh->c2)) / 2,
		       screenp->sh->c2, strlen(screenp->sh->c2));
	if (screenp->sh->c1)
		memcpy(scr_mem[2 + screenp->nbrrows + 2] +
			(LINE_WIDTH - strlen(screenp->sh->c1)) / 2,
		       screenp->sh->c1, strlen(screenp->sh->c1));

	/* now print the contents of the screen */

	for (i = 0; i < screenp->nbrrows + 5; i++) {
		fputs(scr_mem[i], psfp);
		putc('\n', psfp);
	}

	/* put end of screen marker */

	putc('E', psfp);
	for (i = 0; i < LINE_WIDTH - 2; i++)
		putc('-', psfp);
	putc('E', psfp);
	putc('\n', psfp);

	EXITFUNC ("printscreen");
}

static void
print_prompt(row_ptr, sdp)
	char *row_ptr;
	Scrn_desc *sdp;
{
	memcpy(row_ptr + sdp->col, sdp->prompt, strlen(sdp->prompt));
}

static void
print_field(row_ptr, sdp)
	char *row_ptr;
	Scrn_desc *sdp;
{
	memset(row_ptr + sdp->col, '_', sdp->len);
}

static void
print_scroll(rows, sdp)
	char **rows;
	Scrn_desc *sdp;
{
	int	temprow, tempcol;
	int	i, j;
	char	**strings;
	int	filled;
	int	item;

	temprow = sdp->row;

	/* if a scrolling region is filled in, use the values */

	if (sdp->scrnstruct != (Scrn_struct *) 0) {
		strings = s_scrollreg(sdp->scrnstruct);
		filled = sdp->scrnstruct->filled;
	} else {
		strings = (char **) 0;
		filled = 0;
	}

	item = 0;
	for (i = 0; i < sdp->s_lines; i++)  {
		tempcol = sdp->col;
		for (j = 0; j < sdp->s_itemsperline; j++) {

			/* if there is a filled in item, print it */

			if (filled && item < filled)
				memcpy(rows[temprow + 2] + tempcol,
				       strings[item],
				       strlen(strings[item]));

			/* if there are no filled in items, print underscore */

			else if (!filled)
				memset(rows[temprow + 2] + tempcol,
				       '_', sdp->len);

			tempcol += sdp->len + sdp->s_spaces;
			item++;
		}
		temprow++;
	}
}

static void
print_choice(row_ptr, sdp)
	char *row_ptr;
	Scrn_desc *sdp;
{
	memcpy(row_ptr + sdp->col, sdp->prompt, strlen(sdp->prompt));
}

static void
print_toggle(row_ptr, sdp)
	char *row_ptr;
	Scrn_desc *sdp;
{
	if (sdp->len == 1)
		row_ptr[sdp->col] = '_';
	else
		memcpy(row_ptr + sdp->col, sdp->prompt,
				strlen(sdp->prompt));
}

static void
print_text(rows, sdp)
	char **rows;
	Scrn_desc *sdp;
{
	int i;

	for (i = 0; i < sdp->s_lines; i++) {

		char *row = rows[sdp->row + i + 2];

		memset(row + sdp->col, '_', sdp->len);
	}
}

#endif /* PRINTSCR */
