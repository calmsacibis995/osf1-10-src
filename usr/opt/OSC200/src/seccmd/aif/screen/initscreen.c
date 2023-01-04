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
static char	*sccsid = "@(#)$RCSfile: initscreen.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:57:58 $";
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



/* Copyright (c) 1988, 1990 SecureWare, Inc.
 *   All rights reserved
 */

#include "userif.h"
#include "kitch_sink.h"
#include "logging.h"

#ifdef DEBUG
FILE *logfp;
#define	LOGFILE "logfile"
#else DEBUG
#endif DEBUG

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
	sp->toprow = sp->leftcol = sp->nbrrows = sp->nbrcols = 0;

	switch (sp->scrntype) {
	case SCR_MENU :
		if (sp->ndescs != 0 || forceit) {
			for ( ; sd < sp->sd + sp->ndescs; sd++) {
				if (++sp->nbrrows > MENU_ROWS)
					break;		/* SKIP THE REST */
				if (sd->prompt) {
					len = strlen (sd->prompt);
					sp->nbrcols = MAX (sp->nbrcols, len);
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
	case SCR_MENUPROMPT :
		if (sp->ndescs != 0 || forceit) {
			for ( ; sd < sp->sd + sp->ndescs; sd++) {
				if (old_row != sd->row) {
					sp->nbrrows = MAX (old_row, sd->row);
					if (sp->nbrrows > MENU_ROWS)
						goto SMP_CLEAN;	/* SKIP REST */
					len = 0;
					old_row = sd->row;
				}
				sd->col = len;
				if (sd->len > 0) {
					len += sd->len;
				} else if (sd->prompt) {
					len += strlen (sd->prompt);
				}
				sp->nbrcols = MAX (sp->nbrcols, len);
			}
SMP_CLEAN:
			sp->nbrrows++;
			sp->nbrrows = MIN (sp->nbrrows, MENU_ROWS);
			sp->toprow = (LINES - sp->nbrrows) / 2 - 1;
			adj_cols (sp);

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
				sp->nbrrows = MAX(sp->nbrrows, row);

				len = sd->col;
				if (sd->len > 0) {
					if (sd->type == FLD_TEXT) {
						sd->s_itemsperline = 1;
						sd->s_spaces = 0;
					}
					if (sd->s_itemsperline)
						len = MAX(len +
							(sd->s_itemsperline*
							sd->len)+(sd->s_spaces*
							(sd->s_itemsperline-1)),
							len);
					else
						len += sd->len;
				} else if (sd->prompt) {
					len += strlen (sd->prompt);
				}
				sp->nbrcols = MAX (sp->nbrcols, len);
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
				sp->nbrcols = MAX(sp->nbrcols, len);
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
		sp->w = newwin (sp->nbrrows, sp->nbrcols, sp->toprow, sp->leftcol);
		fixwin (sp->w);
	}
	EXITFUNC ("initscreen");
}



/*
 * adj_cols - adjust # columns
 */

adj_cols (sp)
Scrn_parms *sp;
{
	sp->nbrcols = MIN (sp->nbrcols + 1, MENU_COLS);
	if (sp->nbrcols < MENU_COLS)
		sp->leftcol = (COLS - sp->nbrcols) / 2 - 1;
	else
		sp->leftcol = 0;
}
