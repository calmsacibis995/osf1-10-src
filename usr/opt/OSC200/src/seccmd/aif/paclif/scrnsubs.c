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
static char *rcsid = "@(#)$RCSfile: scrnsubs.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:13:51 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	scrnsubs.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:05:31  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  17:00:15  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:20:35  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:53:55  marquard]
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
 *	@(#)scrnsubs.c	2.23 16:31:24 5/17/91 SecureWare
 */

/* #ident "@(#)scrnsubs.c	1.1 11:18:38 11/8/91 SecureWare" */

/*
 * This file contains miscellaneous user interface screen routines
 */

#include "If.h"
#include "AIf.h"
#include "scrn_local.h"

static char *fixalpha();

/* move to a particular field, given an index into scrn_desc table
 * Assume that any cleanup necessary for old field was done.
 */

void
movetofield (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	register int	i;

	if (stp->message != stp->curfield)  {
		rm_message (stp->window);
		stp->message = stp->curfield;
	}
	sdp = sttosd(stp);
	if (!sdp)
		return;			/* NO SCRN_DESC STUFF */
	switch (sdp->type) {
	case FLD_SCROLL:
	case FLD_SCRTOG:
	case FLD_TEXT:
		movetoscrollreg (stp, sdp);
		break;
	case FLD_PROMPT:
		restorescreen();
		printf (NOPROMPT);
		exit (1);
	case FLD_CHOICE:
		/* highlight choice field */
		highlight (stp->window, sdp->row, sdp->col, sdp->prompt);
		if (stp->screenp->scrntype == SCR_MENU || 
		    stp->curfield == stp->screenp->ndescs - 1 ||
		    (sdp+1)->type == FLD_CHOICE || 
		    (sdp+1)->type == FLD_TOGGLE ||
		    (sdp+1)->type == FLD_PROMPT) {
			cursor (INVISCURSOR);
			break;
		}
		/* otherwise, fall into input field for next item */
		stp->curfield++;
		sdp++;
		break;
	case	FLD_TOGGLE:
		stp->columninfield = 0;
		stp->itemchanged = 0;
		if (sdp->len == 1) {
			stp->scrnrep[0] = *(s_toggle(sdp->scrnstruct)) ?
				TOGGLEON : TOGGLEOFF;
			stp->scrnrep[1] = '\0';
			wmove(stp->window, sdp->row, sdp->col);
		} else {
			strcpy(stp->scrnrep, sdp->prompt);
			for (i = strlen (stp->scrnrep); i < sdp->len; i++)
				stp->scrnrep[i] = ' ';
			stp->scrnrep[sdp->len] = '\0';
			highlighttoggle(stp, sdp);
		}
		break;
	default:  /* input field */
		wmove (stp->window, sdp->row, sdp->col);
		stp->columninfield = 0;
		stp->itemchanged = 0;
		/* prepare screen representation of field */
		switch (sdp->type)  {
		case	FLD_ALPHA:
			strcpy (stp->scrnrep, s_alpha(sdp->scrnstruct));
			break;
		case	FLD_NUMBER:
			sprintf (stp->scrnrep,
				 "%ld",
				 *(s_number(sdp->scrnstruct)));
			break;
		case	FLD_YN:
			stp->scrnrep[0] = *(s_yesno(sdp->scrnstruct)) ?
						YES_CHAR : NO_CHAR;
			stp->scrnrep[1] = '\0';
			break;
		case	FLD_POPUP:
			stp->scrnrep[0] = *(s_popup(sdp->scrnstruct)) ?
						YES_CHAR : NO_CHAR;
			stp->scrnrep[1] = '\0';
			break;
		case	FLD_CONFIRM:
			stp->scrnrep[0] = *(s_confirm(sdp->scrnstruct)) ?
						YES_CHAR : NO_CHAR;
			stp->scrnrep[1] = '\0';
			break;
		}
		/* pad out scrnrep to nulls */
		for (i = strlen (stp->scrnrep); i < sizeof (stp->scrnrep); i++)
			stp->scrnrep[i] = '\0';
		break;
	}
}

/*
 * Re-enter a field after having redrawn the screen.
 * Just go back to where we were.
 */

void
reenterfield(stp, sdp)
struct state *stp;
struct scrn_desc *sdp;
{
	uchar row, col;

	switch (sdp->type) {
	case FLD_SCROLL:
	case FLD_TEXT:
		scr_rowcol(stp, sdp, &row, &col);
		wmove(stp->window, row, col + stp->columninfield);
		break;

	case FLD_NUMBER:
	case FLD_ALPHA:
		wmove(stp->window, sdp->row, sdp->col + stp->columninfield);
		break;

	case FLD_YN:
		wmove(stp->window, sdp->row, sdp->col);
		break;

	case FLD_CHOICE:
		highlight (stp->window, sdp->row, sdp->col, sdp->prompt);
		break;

	case FLD_SCRTOG:
	case FLD_TOGGLE:
		highlighttoggle(stp, sdp);
		break;
	}
}

/* leave an input field, and store any changed information from
 * the screen representation of the field into the internal representation.
 *
 * returns	0 on success,
 *		1 if error in the field itself
 *		2 if an action in that field should cause a return
 */

int
leavefield (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	char	*endcp, *begincp, *cp;
	char	thischar;
	char	**stringtab;
	long	atol();
	char	negative;

	sdp = sttosd (stp);
	switch (sdp->type) {
	case FLD_CHOICE:
		unhighlight (stp->window, sdp->row, sdp->col, sdp->prompt);
		return (0);
	case FLD_SCRTOG:
		sdp->index = stp->scrollitem;
		/* fall into . . . */
	case FLD_TOGGLE:
		unhighlighttoggle(stp, sdp);
		break;
	default:
		break;
	}
		
	if (stp->itemchanged == 0 &&
	!((sdp->type == FLD_SCROLL || sdp->type == FLD_TEXT) &&
	sdp->inout == FLD_OUTPUT))
		return (0);

	switch (sdp->type)  {
	case	FLD_SCROLL:

		/*
		 * In a scrolling region, fix the screen to handle
		 * empty slots.  If there is a newly-created one
		 * and the screen allows input, move the rest of
		 * the items in the screen back one slot.
		 */

		stringtab = s_scrollreg(sdp->scrnstruct);
		if (sdp->inout != FLD_OUTPUT) {

			cp = fixalpha(stp->scrnrep, sdp->len);
			strcpy(stringtab[stp->scrollitem], cp);

			/* 
			 * Another kml hack.  Just don't worry about blank 
			 * spots.  We shouldn't have them and we shouldn't move
			 * around lines if we find one of them.
			 */
#if SCROLLING_RIGHT
			/* check for a newly-created null slot */

			if (stp->scrollitem != sdp->scrnstruct->filled - 1 &&
			    stringtab[stp->scrollitem][0] == '\0' &&
			    stringtab[stp->scrollitem+1][0] != '\0') {
				scroll_lshift(stp, sdp);
				return (2);
			}
#endif
		}

		/* remember which field in the region we were last on */

		sdp->which_ss = stringtab[stp->scrollitem];
		break;

	case	FLD_TEXT:
	{
		int i;

		/*
		 * Copy the screen rep into the right slot in the table,
		 * and NULL-fill at the end
		 */

		stringtab = (char **) s_text(sdp->scrnstruct);
		strcpy(stringtab[stp->scrollitem], stp->scrnrep);
		i = strlen(stp->scrnrep);
		memset(&stringtab[stp->scrollitem][i], '\0', sdp->len - i);

		/*
		 * Correct the text representation of the last line
		 * in the string --> either the first line whose length
		 * is less than the width of the field or the last line
		 * in the table.
		 */

		for (i = 0; i < sdp->scrnstruct->filled; i++) {
			if (strlen(stringtab[i]) < sdp->len ||
			    i == sdp->scrnstruct->filled - 1) {
				fixalpha(stringtab[i]);

				/* correct screen image if on screen */

				if (i == stp->scrollitem) {

					int length;

					strcpy(stp->scrnrep, stringtab[i]);
					length = strlen(stp->scrnrep);
					memset(&stp->scrnrep[length], '\0',
						sdp->len - length);
				}

				break;
			}
		}

		sdp->which_ss = stringtab[stp->scrollitem];
		break;
	}
	case	FLD_YN:
		thischar = stp->scrnrep[0];
		if (thischar == 'y' || thischar == 'Y')
			*(s_yesno(sdp->scrnstruct)) = 1;
		else if (thischar == 'n' || thischar == 'N')
			*(s_yesno(sdp->scrnstruct)) = 0;
		else  {
			message (stp, stp->window, stp->screenp,
			  YESNOERROR, NO);
			wmove (stp->window, sdp->row, sdp->col);
			stp->columninfield = 0;
			return (1);
		}
		break;
	case	FLD_CONFIRM:
		thischar = stp->scrnrep[0];
		if (thischar == 'y' || thischar == 'Y')
			*(s_confirm(sdp->scrnstruct)) = 1;
		else if (thischar == 'n' || thischar == 'N')
			*(s_confirm(sdp->scrnstruct)) = 0;
		else  {
			message (stp, stp->window, stp->screenp,
			  YESNOERROR, NO);
			wmove (stp->window, sdp->row, sdp->col);
			stp->columninfield = 0;
			return (1);
		}
		stp->ret.flags |= (R_CONFIRM | R_CHANGED);
		stp->ret.item = stp->curfield;
		sdp->scrnstruct->changed = 1;
		return (2);
		break;
	case	FLD_POPUP:
		thischar = stp->scrnrep[0];
		if (thischar == 'y' || thischar == 'Y')
			*(s_popup(sdp->scrnstruct)) = 1;
		else if (thischar == 'n' || thischar == 'N')
			*(s_popup(sdp->scrnstruct)) = 0;
		else  {
			message (stp, stp->window, stp->screenp,
			  YESNOERROR, NO);
			wmove (stp->window, sdp->row, sdp->col);
			stp->columninfield = 0;
			return (1);
		}
		stp->ret.flags |= (R_POPUP | R_CHANGED);
		stp->ret.item = stp->curfield;
		sdp->scrnstruct->changed = 1;
		return (2);
		break;
	case	FLD_ALPHA:
		/* fix field to remove NULLs, trailing spaces */
		cp = fixalpha (stp->scrnrep, sdp->len);
		if (cp == NULL) {
			s_alpha(sdp->scrnstruct)[0] = '\0';
			return (0);
		}
		strcpy (s_alpha(sdp->scrnstruct), cp);
		break;
	case	FLD_NUMBER:
		begincp = stp->scrnrep;
		/* find last non-null and non-space in field */
		for (endcp = &stp->scrnrep[sdp->len];
		     endcp >= begincp;
		     endcp--)
			if (*endcp && *endcp != SPACE)
				break;
		/* check for nothing in field */
		if (begincp == endcp && *endcp == '\0') {
			*s_number(sdp->scrnstruct) = 0L;
			return (0);
		}
		/* skip leading nulls and spaces */
		for ( ; begincp < endcp; begincp++)
			if (*begincp && *begincp != SPACE)
				break;
		/* check for negative number */
		if (*begincp == '-') {
			negative = 1;
			begincp++;
		} else	negative = 0;
		/* look for non-digits */
		for (cp = begincp; cp <= endcp; cp++)
			if (!isdigit (*cp)) {
				message (stp, stp->window, stp->screenp,
					 NOTNUMBERERROR, NO);
				wmove (stp->window, sdp->row, sdp->col);
				stp->columninfield = 0;
				return (1);
			}
		*s_number(sdp->scrnstruct) =
		  negative ? -atol (begincp) : atol (begincp);
		break;
	default:
		break;
	}  /* end switch */
	stp->ret.flags |= R_CHANGED;
	stp->ret.item = stp->curfield;
	sdp->scrnstruct->changed = 1;
	stp->itemchanged = 0;
	return (0);
}

/* modify an FLD_ALPHA's screen representation such that all internal
 * NULLs are turned to spaces.
 * All trailing spaces are ignored.
 * Returns a pointer to the beginning of the fixed string, or NULL
 * if there were nothing but spaces and NULLs in the string
 */


static char *
fixalpha (string, len)
char	*string;
int	len;
{
	char	*begincp, *endcp, *cp;

	begincp = string;
	for (endcp = &string[len]; endcp >= string; endcp--)
		if (*endcp && *endcp != SPACE)
			break;
	/* check if nothing in field */
	if (begincp == endcp && *endcp == '\0')
		return (NULL);
	/* place '\0' at end of string (change all spaces to '\0') */
	for (cp = endcp + 1; cp < &string[len]; cp++)
		*cp++ = '\0';
	/* replace '\0' with spaces  - retain leading spaces */
	for (begincp = string; begincp < endcp; begincp++)
		if (*begincp == '\0')
			*begincp = SPACE;
	return (string);
}

/* zeros an input field in its internal representation */
void
clearfield (sdp)
struct	scrn_desc	*sdp;
{
	char	*cp;
	int	i;

	switch (sdp->type)  {
	case	FLD_ALPHA:
		for (cp = s_alpha(sdp->scrnstruct), i = 0; i < sdp->len; i++)
			*cp++ = '\0';
		break;
	case	FLD_NUMBER:
		*(s_number(sdp->scrnstruct)) = 0L;
		break;
	case	FLD_YN:
		*(s_yesno(sdp->scrnstruct)) = '\0';
		break;
	case	FLD_POPUP:
		*(s_popup(sdp->scrnstruct)) = '\0';
		break;
	case	FLD_CONFIRM:
		*(s_confirm(sdp->scrnstruct)) = '\0';
		break;
	}
	return;
}



/* delete n characters from a field on the screen
 * Leaves the cursor in the same screen position
 */

void
delchars (stp, sdp, n)
register struct	state		*stp;	/* get columninfield and window */
register struct	scrn_desc	*sdp;	/* which screen description */
int	n;			/* how many columns */
{
	register int	i;
	char	*string, *start;
	char	scrnbuf[80];
	uchar	row, col;

	start = &stp->scrnrep[stp->columninfield];
	string = start;
	for (i = stp->columninfield; i < sdp->len - n; i++)  {
		*string = *(string + n);
		string++;
	}
	/* pad out rest of string with '\0' */
	for (i = 0; i < n; i++)
		*string++ = '\0';
	UNDERLINE (stp->window, ON);
	wpadstring (stp->window,
		    start,
		    sdp->len - stp->columninfield);
	UNDERLINE (stp->window, OFF);
	if (sdp->type == FLD_SCROLL || sdp->type == FLD_SCRTOG ||
	sdp->type == FLD_TEXT)
		scr_rowcol (stp, sdp, &row, &col);
	else  {
		row = sdp->row;
		col = sdp->col;
	}
	wmove (stp->window, row, col + stp->columninfield);
	return;
}

/* move a partial field n characters to the right, starting at
 * the current columninfield
 */
void
moveright (stp, sdp, n)
register struct	scrn_desc	*sdp;	/* which screen description */
register struct	state		*stp;	/* get columninfield and window */
int	n;			/* how many columns */
{
	register char	*start;
	register int	i;
	uchar	row, col;

	start = &stp->scrnrep[sdp->len - 1];
	for (i = sdp->len - 1; i >= stp->columninfield + n; i--) {
		*start = *(start - n);
		start--;
	}
	start = &stp->scrnrep[stp->columninfield];
	for (i = 0; i < n; i++)
		*start++ = SPACE;
	UNDERLINE (stp->window, ON);
	wpadstring (stp->window, &stp->scrnrep[stp->columninfield],
		sdp->len - stp->columninfield);
	UNDERLINE (stp->window, OFF);
	if (sdp->type == FLD_SCROLL || sdp->type == FLD_SCRTOG ||
	sdp->type == FLD_TEXT)
		scr_rowcol (stp, sdp, &row, &col);
	else  {
		row = sdp->row;
		col = sdp->col;
	}
	wmove (stp->window, row, col + stp->columninfield);
	return;
}

/* find the choice field corresponding to the current input field */

void
findchoice (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;

	sdp = sttosd (stp);
	while (sdp->type != FLD_CHOICE)  {
		sdp--;
		stp->curfield--;
	}
	return;
}

/* find the item on the previous line that is closest to the current column
 * returns the descriptor offset, or -1 if there wasn't one
 */

int
findupitem (stp, sdp)
register struct	state	*stp;
register struct	scrn_desc	*sdp;
{
	struct	scrn_desc	*foundsdp;
	int	founddesc;
	register int	i;
	uchar	row, col;
	int wrapping = 0;

	if (sdp->type == FLD_SCROLL || sdp->type == FLD_SCRTOG ||
	sdp->type == FLD_TEXT)
		scr_rowcol (stp, sdp, &row, &col);
	else {
		row = sdp->row;
		col = sdp->col;
	}
	/* find first non-prompt on a previous line that is not output only */
	foundsdp = NULL;	/* set when found one on previous line */
	founddesc = -1;		/* ditto */
	if(stp->curfield > stp->firstfield) {
		i = stp->curfield - 1;
		sdp--;
	} else {
		i = stp->lastfield;
		sdp = lastsd (stp);
	}
loop:	for ( ; i >= stp->firstfield && i <= stp->lastfield; i--, sdp--) {
		if (sdp->type == FLD_PROMPT || sdp->type == FLD_SKIP ||
			sdp->type == FLD_BLANK)
			continue;
		/* find CLOSEST input or both field on a previous line */
		if ((sdp->row < row || wrapping) && sdp->inout != FLD_OUTPUT) {
			if (foundsdp) { /* already have one on previous line */
				/* Was it 1st on that line? */
				if (sdp->row < foundsdp->row)  /* yes */
					break;
				/* it wasn't 1st on that line */
				/* check if closest to column */
				else if (abs (sdp->col - col) <
					    abs (foundsdp->col - col) || wrapping) {
					foundsdp = sdp;
					founddesc = i;
				}
			}
			else  {  /* found a candidate for previous line */
				foundsdp = sdp;
				founddesc = i;
			}
		}
	}
	if (founddesc == -1)  {  /* wrap to bottom line */
		i = stp->curfield = stp->lastfield;
		sdp = sttosd (stp);
		row = sdp->row;
		wrapping = 1;
		goto loop;
	}
	stp->curfield = founddesc;
	sdp = sttosd (stp);
	return (founddesc);
}

/* find the item on the next row that is closest to the column of the
 * current item.
 * return a curfield value if found, o.w. -1.
 */

int
finddownitem (stp, sdp)
struct	state	*stp;
struct	scrn_desc	*sdp;
{
	uchar	row, col;
	struct	scrn_desc	*foundsdp;
	int	founddesc;
	int	i;
	int wrapping = 0;

	if (sdp->type == FLD_SCROLL || sdp->type == FLD_SCRTOG ||
	sdp->type == FLD_TEXT)
		scr_rowcol (stp, sdp, &row, &col);
	else {
		row = sdp->row;
		col = sdp->col;
	}
	/* find first non-prompt on a later line that is not output only */
	foundsdp = NULL;
	founddesc = -1;
	if (stp->curfield < stp->lastfield) {
		i = stp->curfield + 1;
		sdp++;
	} else {
		i = stp->firstfield;
		sdp = firstsd(stp);
	}
loop:	for ( ; i <= stp->lastfield; i++, sdp++) {
		if (sdp->type == FLD_PROMPT || sdp->type == FLD_SKIP ||
			sdp->type == FLD_BLANK)
			continue;
		if ((sdp->row > row || wrapping) && sdp->inout != FLD_OUTPUT) {
			if (foundsdp) {
				if (sdp->row > foundsdp->row)
					break;
				if (abs (sdp->col - col) <
				    abs (foundsdp->col - col) || wrapping) {
					foundsdp = sdp;
					founddesc = i;
				}
			} else {
				foundsdp = sdp;
				founddesc = i;
			}
		}
	}
	if (founddesc == -1)  {  /* wrap to top line */
		i = stp->curfield = stp->firstfield;
		sdp = sttosd (stp);
		row = sdp->row;
		wrapping = 1;
		goto loop;
	}
	stp->curfield = founddesc;
	sdp = sttosd (stp);
	return (founddesc);
}

#ifdef OLD_XENIX
/*
 * the equivalent of strtok for an /etc/default file line.
 */

char *
default_tok (string, chars)
char	*string;
char	*chars;
{
	static	char	*cp;
	register char	*cp1;
	char	*retval;

	if (string != (char *) 0)
		cp = string;
	/* pass up white space first */
	while (*cp == ' ' || *cp == '\t')
		cp++;
	if (*cp == '\0')
		return ((char *) 0);
	if (*cp == '\\' && *(cp + 1) == '\0') {
		cp++;
		return ((char *) 0);
	}
	/* at this point, we have a real token. */
	for (cp1 = cp + 1; ; cp1++) {
		switch (*cp1) {
		case	'\0': /* end of string means we've got the token */
			retval = cp;
			cp = cp1;
			return (retval);
		case	' ':	/* item by itself */
		case	'\t':
			*cp1 = '\0';
			cp1++;
			retval = cp;
			cp = cp1;
			return (retval);
		case	'=':
			cp1++;
			if (*cp1 == '"') { /* string - position at end */
				for (++cp1; *cp1 != '"'; cp1++)
					if (*cp1 == '\0') { /* bad format */
						cp = cp1;
						return ((char *) 0);
					}
				cp1++;
			} else { /* non-string - look for 1st space */
				for ( ; *cp1 != ' ' &&
					*cp1 != '\t' &&
					*cp1 != '\0'; cp1++)
					;
			}
			retval = cp;
			/* end of line logic, leave at end or one past
			 * end of token.
			 */
			if (*cp1 == '\0')
				cp = cp1;
			else {
				*cp1++ = '\0';
				cp = cp1;
			}
			return (retval);
		default:
			break;
		}
	}
}

#endif

enditall(msg)
char *msg;
{
	restorescreen();
	printf (msg);
	exit (1);
}
