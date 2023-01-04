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
static char *rcsid = "@(#)$RCSfile: initscreen.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:11:52 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	initscreen.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:04:17  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  16:59:10  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:19:41  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:52:21  marquard]
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
 * Copyright (c) 1988, 1990 SecureWare, Inc.
 *   All rights reserved
 *
 * Based on OSF version:
 *	@(#)initscreen.c	2.12 16:31:06 5/17/91 SecureWare
 */

/* #ident "@(#)initscreen.c	1.1 11:16:49 11/8/91 SecureWare" */

#include "If.h"
#include "AIf.h"
#include "scrn_local.h"

/*
 * initscreen (Scrn_parms sp, int forceit) - initialize screen data structures
 */

initscreen (spp, forceit)
Scrn_parms *spp;
int forceit;
{
	register Scrn_parms *sp = spp;
	register Scrn_desc *sd = sp->sd;
	register Scrn_hdrs *sh = sp->sh;
	int i;
	int len = 0;
	int old_row = 0;
	char *s;

	ENTERFUNC ("initscreen");

	/*
	 * All screen types re-compute these screen parameters except
	 * the SCR_TEXT screen type, which may pass in sp->nbrcols.
	 */

	if (sp->scrntype != SCR_TEXT)
		sp->toprow = sp->leftcol = sp->nbrrows = sp->nbrcols = 0;

	switch (sp->scrntype) {
	case SCR_MENU :
		if (sp->ndescs != 0 || forceit) {
			for ( ; sd < sp->sd + sp->ndescs; sd++) {
				if (++sp->nbrrows > MENU_ROWS)
					break;		/* SKIP THE REST */
				if (sd->prompt) {
					len = strlen (sd->prompt);
					if (len > sp->nbrcols)
						sp->nbrcols = len;
				}
			}
			sp->toprow = (LINES - sp->nbrrows) / 2 - 1;
			adj_cols (sp);

			{ register int row = 0;
				for (sd = sp->sd; sd < sp->sd + sp->ndescs; sd++) {
					sd->row = row++;
					sd->col = 0;
				}
			}
		}
		break;
	case SCR_FILLIN :
	case SCR_MSGONLY :
	case SCR_NOCHANGE :
		if (sp->ndescs != 0 || forceit) {
			for ( ; sd < sp->sd + sp->ndescs; sd++) {
				int row;

				if (sd->s_lines)
					row = sd->row + sd->s_lines - 1;
				else
					row = sd->row;
				if (row > sp->nbrrows)
					sp->nbrrows = row;

				len = sd->col;
				if (sd->len > 0) {
					if (sd->type == FLD_TEXT) {
						sd->s_itemsperline = 1;
						sd->s_spaces = 0;
					}
					if (sd->s_itemsperline) {
						int tlen;

						tlen = len +
				(sd->s_itemsperline * sd->len) + 
				(sd->s_spaces * (sd->s_itemsperline - 1));
						if (tlen > len)
							len = tlen;
					} else
						len += sd->len;
				} else if (sd->prompt) {
					len += strlen (sd->prompt);
				}
				if (len > sp->nbrcols)
					sp->nbrcols = len;
			}
SF_CLEAN:
			sp->nbrrows++;
			if (sp->nbrrows > MENU_ROWS)
				sp->nbrrows = MENU_ROWS;
			sp->toprow = (LINES - sp->nbrrows) / 2 - 1;
			adj_cols (sp);
			sp->nbrcols = MENU_COLS;
			sp->leftcol = 0;

		}
		break;
	case SCR_TEXT:
		if (sp->text) {
			/*
			 * If the number of rows specified is non-zero,
			 * use that as the count.  Otherwise assume a
			 * NULL-terminated array and count them
			 */

			if (sp->nbrrows == 0)
				for (i = 0; sp->text[i]; i++)
					sp->nbrrows++;

			/* enforce the maximum size */
			if (sp->nbrrows > MENU_ROWS)
				sp->nbrrows = MENU_ROWS;

			/* find the widest column */
			for (i = 0; i < sp->nbrrows; i++) {
				len = strlen(sp->text[i]);
				if (len > sp->nbrcols)
					sp->nbrcols = len;
			}

			/* center the text on the screen */
			sp->toprow = (LINES - sp->nbrrows) / 2 - 1;
			adj_cols(sp);
		}
		break;
	}
	DUMPDETI (" r=<%d> c=<%d>", sp->nbrrows, sp->nbrcols, NULL);
	DUMPDETI (" y=<%d> x=<%d>", sp->toprow, sp->leftcol, NULL);
	if (!sp->w) {
		sp->w = newwin(sp->nbrrows, sp->nbrcols,
				sp->toprow, sp->leftcol);
		fixwin(sp->w);
	}
	EXITFUNC ("initscreen");
}



/*
 * adj_cols - adjust # columns
 */

adj_cols (sp)
Scrn_parms *sp;
{
	sp->nbrcols++;
	if (MENU_COLS < sp->nbrcols)
		sp->nbrcols = MENU_COLS;
	if (sp->nbrcols < MENU_COLS)
		sp->leftcol = (COLS - sp->nbrcols) / 2 - 1;
	else
		sp->leftcol = 0;
}
