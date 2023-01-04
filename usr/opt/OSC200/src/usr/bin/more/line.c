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
static char rcsid[] = "@(#)$RCSfile: line.c,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/10/07 17:58:13 $";
#endif
/*
 * HISTORY
 */
/*
 * HISTORY
 * $OSF_Log:	line.c,v $
 * Revision 1.1.1.1  93/01/07  08:45:00  devrcs
 *  *** OSF1_1_2B07 version ***
 * 
 * Revision 1.1.2.3  1992/09/03  19:54:39  tom
 * 	Fix small problem with exiting bold.
 * 	[1992/09/03  19:24:04  tom]
 *
 * Revision 1.1.2.2  1992/08/24  18:17:17  tom
 * 	New more for POSIX.2/XPG4.
 * 	[1992/08/24  17:30:31  tom]
 * 
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1988 Mark Nudleman
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char sccsid[] = "@(#)line.c	5.5 (Berkeley) 7/24/91";
#endif /* not lint */

/*
 * Routines to manipulate the "line buffer".
 * The line buffer holds a line of output as it is being built
 * in preparation for output to the screen.
 * We keep track of the PRINTABLE length of the line as it is being built.
 */

#include <sys/types.h>
#include <ctype.h>
#include <wchar.h>
#include "less.h"

static wchar_t linebuf[LINE_LENGTH];	/* Buffer which holds the
					   current output line */
static wchar_t *curr;		/* Pointer into linebuf */
static int column;		/* Printable length, accounting for
				   backspaces, etc. */
int null_count ;
int esc_count;

/*
 * A ridiculously complex state machine takes care of backspaces.  The
 * complexity arises from the attempt to deal with all cases, especially
 * involving long lines with underlining, boldfacing or whatever.  There
 * are still some cases which will break it.
 *
 * There are a lot of states:
 *	LN_NORMAL is the normal state (not in underline mode).
 *	LN_UNDERLINE means we are in underline mode.  We expect to get
 *		either a sequence like "_\bX" or "X\b_" to continue
 *		underline mode, or anything else to end underline mode.
 *	LN_BOLDFACE means we are in boldface mode.  We expect to get sequences
 *		like "X\bX\b...X\bX" to continue boldface mode, or anything
 *		else to end boldface mode.
 *	LN_UL_X means we are one character after LN_UNDERLINE
 *		(we have gotten the '_' in "_\bX" or the 'X' in "X\b_").
 *	LN_UL_XB means we are one character after LN_UL_X 
 *		(we have gotten the backspace in "_\bX" or "X\b_";
 *		we expect one more ordinary character, 
 *		which will put us back in state LN_UNDERLINE).
 *	LN_UL_XBB means we are one character after LN_UL_XB
 *		(we have gotten the multicolumn char 'AA' in "_\bAA\b_")
 *	LN_UL_XBBU means we are one character after LN_UL_XBB
 *		(we have gotten the 2nd backspace in "_\bAA\b_";
 *		we expect one more '_',
 *		which will put us back in state LN_UNDERLINE).
 *	LN_UL_U means we are one character after LN_UNDERLINE2
 *		(we have gotten the '_' in "__\b\bAA").
 *	LN_UL_UU means we are one character after LN_UL_U
 *		(we have gotten the 2nd '_' in "__\b\bAA").
 *	LN_UL_UUB means we are one character after LN_UL_UU
 *		(we have gotten the '\b' in "__\b\bAA").
 *	LN_UL_UUBB means we are one character after LN_UL_UUB
 *		(we have gotten the 2nd '\b' in "__\b\bAA";
 *		we expect one more multicolumn character, 
 *		which will put us back in state LN_UNDERLINE2).
 *	LN_BO_X means we are one character after LN_BOLDFACE
 *		(we have gotten the 'X' in "X\bX").
 *	LN_BO_XB means we are one character after LN_BO_X
 *		(we have gotten the backspace in "X\bX";
 *		we expect one more 'X' which will put us back
 *              in state LN_BOLDFACE) or
 *		(we have gotten the 1st '\b' in "AA\b\bAA")
 *	LN_BO_XBB means we are one character after LN_BO_XB
 *		(we have gotten the 2nd '\b' in "AA\b\bAA";
 *		we expect one more multicolumn character,
 *		which will put us back in state LN_BOLDFACE).
 */
static int ln_state;		/* Currently in normal/underline/bold/etc mode? */
#define	LN_NORMAL	0	/* Not in underline, boldface or whatever mode */
#define	LN_UNDERLINE	1	/* In underline, need next char */
#define	LN_UL_X		2	/* In underline, got char, need \b */
#define	LN_UL_XB	3	/* In underline, got char & \b, need one more */
#define LN_UL_XBB 	4	/* In underline, got multicolumn char 'AA' in "_\bAA\b_" */
#define LN_UL_XBBU 	5	/* In underline, got the 2nd \b in "_\bAA\b_" */
#define	LN_BOLDFACE	6	/* In boldface, need next char */
#define	LN_BO_X		7	/* In boldface, got char, need \b */
#define	LN_BO_XB	8	/* In boldface, got char & \b, need same char */
#define LN_BO_XBB	9	/* In boldface, got 2nd \b in "AA\b\bAA" */
#define	LN_UNDERLINE2	10	/* In underline, need next char */
#define LN_UL_U		11	/* In underline, got '_' in "__\b\bAA" */
#define LN_UL_UU	12	/* In underline, got the 2nd '_' in "__\b\bAA" */
#define LN_UL_UUB	13	/* In underline, got '\b' in "__\b\bAA" */
#define LN_UL_UUBB	14	/* In underline, got the 2nd '\b' in "__\b\bAA" */
#define LN_UL_UUBBX	15	/* In underline, got 'AA' in "__\b\bAA" */

int	do_not_buffer = 0;	/* Do not buffer chars into the line buffer. */
wchar_t *line;			/* Pointer to the current line.
				   Usually points to linebuf. */
extern int bs_mode;
extern int tabstop;
extern int bo_width, be_width;
extern int ul_width, ue_width;
extern int sc_width, sc_height;
extern int show_all_opt, show_opt;
extern int mbcodeset;
extern int fold_opt;

/*
 * Number of elements in array 'a'.
 */
#define NUM_ELEMENTS(a) (sizeof (a) / sizeof (a)[0])

/*
 * Rewind the line buffer.
 */
void
prewind(void)
{
	line = curr = linebuf;
	ln_state = LN_NORMAL;
	column = 0;
}

/*
 * Append a character to the line buffer.
 * Expand tabs into spaces, handle underlining, boldfacing, etc.
 * Returns 0 if ok, 1 if couldn't fit in buffer.
 */
#define	NEW_COLUMN(addon) \
	if (fold_opt && \
	    column + addon + (ln_state ? ue_width : 0) > sc_width) \
		return(1); \
	else \
		column += addon

int
pappend(wint_t c)
{
	static int escape = 0;
	int w;

	if (c == L'\0') {

	    if (curr > linebuf + NUM_ELEMENTS(linebuf) - 12)
		/*
		 * This will protect against huge
		 * lines that might be encountered
		 * if you more an executable command.
		 */			/* GA001 */
		return(1);


		/*
		 * Terminate any special modes, if necessary.
		 * Append a L'\0' to the end of the line.
		 */
		switch (ln_state) {
		case LN_UL_X:
		case LN_UL_XBB:
			curr[1] = curr[-1];
			curr[-1] = ESC_CHAR;
			curr[0] = UE_CHAR;
			curr +=2;
			break;
		case LN_BO_X:
			curr[1] = curr[-1];
			curr[-1] = ESC_CHAR;
			curr[0] = BE_CHAR;
			curr +=2;
			break;
		case LN_UNDERLINE:
			do_not_buffer = 0;
		case LN_UL_XB:
		case LN_UL_XBBU:
		case LN_UNDERLINE2:
		case LN_UL_U:
		case LN_UL_UU:
		case LN_UL_UUB:
		case LN_UL_UUBB:
		case LN_UL_UUBBX:
			*curr++ = ESC_CHAR;
			*curr++ = UE_CHAR;
			break;
		case LN_BOLDFACE:
			do_not_buffer = 0;
		case LN_BO_XB:
		case LN_BO_XBB:
			*curr++ = ESC_CHAR;
			*curr++ = BE_CHAR;
			break;
		/* Don't throw away NULL code */
		case LN_NORMAL:
			*curr = L'\0';
			curr++;
			null_count++;
			break;
		}
		ln_state = LN_NORMAL;
		*curr = L'\0';
		return(0);
	}

	if (curr > linebuf + NUM_ELEMENTS(linebuf) - 12)
		/*
		 * Almost out of room in the line buffer.
		 * Don't take any chances.
		 * {{ Linebuf is supposed to be big enough that this
		 *    will never happen, but may need to be made 
		 *    bigger for wide screens or lots of backspaces. }}
		 */
		return(1);

	if (!bs_mode) {
		/*
		 * Advance the state machine.
		 */
		switch (ln_state) {
		case LN_NORMAL:
			if (curr <= linebuf + 1
			    || curr[-1] != L'\b')
				break;
			/* Here we have curr[-1] = '\b' */
			column -= 2;
			/* Check the seq. "X\bX" */
			if (c == curr[-2])
				goto enter_boldface;
			/* Check the seq. "AA\b\bAA" */
			if (c == curr[-3] && curr[-2] == L'\b')
				goto enter_multicol_bold;
			/* Check the seq. "__\b\bAA" */
			if (curr[-4]==L'_' && curr[-3]==L'_' && curr[-2]==L'\b')
	                        goto enter_multicol_ul;
			/* Check the seq. "\b_" or "_\bX" */
			if (c == L'_' || curr[-2] == L'_' && c != L'\b')
				goto enter_underline;
			if (curr[-2] != L'_' &&
			    !(wcwidth(curr[-2]) == 2 || curr[-2] == L'\b'))
				curr -= 2;
			break;

enter_boldface:
			/*
			 * We have "X\bX" (including the current char).
			 * Switch into boldface mode.
			 */
			column--;
			if (column + bo_width + be_width + 1 >= sc_width)
				/*
				 * Not enough room left on the screen to 
				 * enter and exit boldface mode.
				 */
				return (1);

			if (bo_width > 0 && curr > linebuf + 2
			    && curr[-3] == L' ') {
				/*
				 * Special case for magic cookie terminals:
				 * if the previous char was a space, replace 
				 * it with the "enter boldface" sequence.
				 */
				curr[-1] = curr[-2];
				curr[-2] = BO_CHAR;
				curr[-3] = ESC_CHAR;
				column += bo_width-1;
				curr++;
			} else {
				curr[0] = curr[-2];
				curr[-2] = ESC_CHAR;
				curr[-1] = BO_CHAR;
				column += bo_width;
				curr += 2;
			}
			goto ln_bo_xb_case;

enter_multicol_bold:
			/*
			 * We have "AA\b\bAA" (including the current char).
			 * Switch into boldface mode.
			 */
			column -= 2;
			if (column + bo_width + be_width + 1 >= sc_width)
				/*
				 * Not enough room left on the screen to 
				 * enter and exit boldface mode.
				 */
				return (1);

			if (bo_width > 0 && curr > linebuf + 2
			    && curr[-3] == L' ') {
				/*
				 * Special case for magic cookie terminals:
				 * if the previous char was a space, replace 
				 * it with the "enter boldface" sequence.
				 */
				curr[-2] = c;
				curr[-3] = BO_CHAR;
				curr[-4] = ESC_CHAR;
				column += bo_width-1;
			} else {
				curr[-3] = ESC_CHAR;
				curr[-2] = BO_CHAR;
				curr[-1] = c;
				column += bo_width;
				curr += 2;
			}
                        goto ln_bold_xbb_case;

enter_underline:
			/*
			 * We have either "_\bX" or "X\b_" (including
			 * the current char).  Switch into underline mode.
			 */
			column--;
			if (column + ul_width + ue_width + 1 >= sc_width)
				/*
				 * Not enough room left on the screen to 
				 * enter and exit underline mode.
				 */
				return (1);

			if (ul_width > 0 && 
			    curr > linebuf + 2 && curr[-3] == L' ')
			{
				/*
				 * Special case for magic cookie terminals:
				 * if the previous char was a space, replace 
				 * it with the "enter underline" sequence.
				 */
				curr[-1] = curr[-2];
				curr[-2] = UL_CHAR;
				curr[-3] = ESC_CHAR;
				column += ul_width-1;
				curr++;
			} else
			{
				curr[0] = curr[-2];
				curr[-1] = UL_CHAR;
				curr[-2] = ESC_CHAR;
				column += ul_width;
				curr +=2;
			}
			goto ln_ul_xb_case;
			/*NOTREACHED*/
enter_multicol_ul:
			/*
			 * We have "__\b\bAA" (including the multicolumn
			 * character 'AA'). Switch into underline mode.
			 */
			column -= 2;
			if (column + ul_width + ue_width + 1 >= sc_width)
				/*
				 * Not enough room left on the screen to 
				 * enter and exit underline mode.
				 */
				return (1);

			if (ul_width > 0 && 
			    curr > linebuf + 2 && curr[-5] == L' ')
			{
				/*
				 * Special case for magic cookie terminals:
				 * if the previous char was a space, replace 
				 * it with the "enter underline" sequence.
				 */
				curr[-3] = c;
				curr[-4] = UL_CHAR;
				curr[-5] = ESC_CHAR;
				column += ul_width-1;
				curr += 2;
			} else {
				curr[-2] = c;
				curr[-3] = UL_CHAR;
				curr[-4] = ESC_CHAR;
				column += ul_width;
				curr += 2;
			}
			goto ln_ul_uubb_case;

		case LN_UL_XB:
			/*
			 * Termination of a sequence "_\bX" or "X\b_".
			 */
			if (c != L'_' && curr[-2] != L'_' && c == curr[-2])
			{
				/*
				 * We seem to have run on from underlining
				 * into boldfacing - this is a nasty fix, but
				 * until this whole routine is rewritten as a
				 * real DFA, ...  well ...
				 */
				curr[2] = curr[-2];
				curr[-2] = ESC_CHAR;
				curr[-1] = UE_CHAR;
				curr[0] = ESC_CHAR;
				curr[1] = BO_CHAR;
				curr += 4; /* char & non-existent backspace */
				ln_state = LN_BO_XB;
				goto ln_bo_xb_case;
			}
ln_ul_xb_case:
			if (c == L'_')
				c = curr[-2];
			curr -= 2;

			if(wcwidth(c) == 2)
				ln_state = LN_UL_XBB;
			else
				ln_state = LN_UNDERLINE;
			break;
		case LN_UL_XBB:
			if(c == L'\b'){
				do_not_buffer++;
				ln_state = LN_UL_XBBU;
			}
			break;
		case LN_UL_XBBU:
			if(c == L'_'){
				do_not_buffer++;
				ln_state = LN_UNDERLINE;
			}
			break;
		case LN_UL_U:
			if(c == L'_'){
				ln_state = LN_UL_UU;
			}
			break;
		case LN_UL_UU:
			if(c == L'\b'){
				ln_state = LN_UL_UUB;
			}
			break;
		case LN_UL_UUB:
			if(c == L'\b'){
				ln_state = LN_UL_UUBB;
			}
			break;
		case LN_UL_UUBB:
ln_ul_uubb_case:
			/*
			 * Termination of a sequence "__\b\bAA".
			 */
			curr -= 4;
			ln_state = LN_UNDERLINE2;
			break;
		case LN_BO_XB:
ln_bo_xb_case:
			/*
			 * Termination of a sequnce "X\bX".
			 */
			if (c != curr[-2] && (c == L'_' || curr[-2] == L'_'))
			{
				/*
				 * We seem to have run on from
				 * boldfacing into underlining.
				 */
				curr[2] = curr[-2];
				curr[-2] = ESC_CHAR;
				curr[-1] = BE_CHAR;
				curr[0] = ESC_CHAR;
				curr[1] = UL_CHAR;
				curr += 4; /* char & non-existent backspace */
				ln_state = LN_UL_XB;
				goto ln_ul_xb_case;
			}
                        if (c == L'\b')
				ln_state = LN_BO_XBB;
			else{
				curr -= 2;
				ln_state = LN_BOLDFACE;
			}
			break;
		case LN_BO_XBB:
ln_bold_xbb_case:
			if(c == curr[-3]){
				ln_state = LN_BOLDFACE;
				curr -= 3;
			}
			break;
		case LN_UNDERLINE:
			do_not_buffer = 0;
			if (column + ue_width + bo_width + 1 + be_width >= sc_width)
				/*
				 * We have just barely enough room to 
				 * exit underline mode and handle a possible
				 * underline/boldface run on mixup.
				 */
				return (1);
			ln_state = LN_UL_X;
			break;
		    case LN_UNDERLINE2:
			if (column + ue_width + bo_width + 1 + be_width >= sc_width)
				/*
				 * We have just barely enough room to 
				 * exit underline mode and handle a possible
				 * underline/boldface run on mixup.
				 */
				return (1);
			if (c == L'_')
				ln_state = LN_UL_U;
			else {
				/*
				 * Exit underline mode.
				 * We have to shuffle the chars a bit
				 * to make this work.
				 */
				curr[0] = ESC_CHAR;
				curr[1] = UE_CHAR;
				curr[2] = c;
				column += ue_width;
				curr++;
				if (ue_width > 0 && curr[0] == L' ')
					/*
					 * Another special case for magic
					 * cookie terminals: if the next
					 * char is a space, replace it
					 * with the "exit underline" sequence.
					 */
					column--;
				else
					curr++;
				ln_state = LN_NORMAL;
			} 
			break;
		case LN_BOLDFACE:
			if (c == L'\b'){
				ln_state = LN_BO_XB;
				break;
			}
			if (column + be_width + ul_width + 1 + ue_width >= sc_width)
				/*
				 * We have just barely enough room to 
				 * exit underline mode and handle a possible
				 * underline/boldface run on mixup.
				 */
				return (1);
			ln_state = LN_BO_X;
			break;
		case LN_UL_X:
			if (c == L'\b' || c == L'_' && curr[-1] == L'\b')
				ln_state = LN_UL_XB;
			else {
				/*
				 * Exit underline mode.
				 * We have to shuffle the chars a bit
				 * to make this work.
				 */
				curr[1] = curr[-1];
				curr[-1] = ESC_CHAR;
				curr[0] = UE_CHAR;
				column += ue_width;
				curr++;
				if (ue_width > 0 && curr[0] == L' ')
					/*
					 * Another special case for magic
					 * cookie terminals: if the next
					 * char is a space, replace it
					 * with the "exit underline" sequence.
					 */
					column--;
				else
					curr++;
				ln_state = LN_NORMAL;
			} 
			break;
		case LN_BO_X:
			if (c == L'\b')
				ln_state = LN_BO_XB;
			else
			{
				/*
				 * Exit boldface mode.
				 * We have to shuffle the chars a bit
				 * to make this work.
				 */
				curr[1] = curr[-1];
				curr[-1] = ESC_CHAR;
				curr[0] = BE_CHAR;
				column += be_width;
				curr++;
				if (be_width > 0 && curr[0] == L' ')
					/*
					 * Another special case for magic
					 * cookie terminals: if the next
					 * char is a space, replace it
					 * with the "exit boldface" sequence.
					 */
					column--;
				else
					curr++;
				ln_state = LN_NORMAL;
			} 
			break;
		}
	}

	if(do_not_buffer)
		return(0);

	if (c == L'\t') {
		/*
		 * Expand a tab into spaces.
		 */
		do {
			NEW_COLUMN(1);
		} while (!show_all_opt && (column % tabstop) != 0);
		*curr++ = L'\t';
		return (0);
	}
	
	if (c == L'\b') {
		if (ln_state == LN_NORMAL)
			NEW_COLUMN(2);
		else
			column--;
		*curr++ = L'\b';
		return(0);
	} 
	
	if (c == ESC_CHAR) {
		if (!escape) {
			NEW_COLUMN(2);	/* assuming ESC_CHAR is a control */
			*curr++ = ESC_CHAR;
			escape = 1;
			return(0);
		} else {
			/*
			 * This is a literal ESC_CHAR.
			 * Already did the NEW_COLUMN above.
			 */
			escape = 0;
			*curr++ = ESC_CHAR;
			return(0);
		}
	}
	
	/*
	 * Make sure non-printable don't wrap lines.
	 * Three cases:
	 * 	ctrl-X (^X)	    == 2 columns
	 * 	meta-X:	(M-X)	    == 3 columns
	 * 	meta-ctrl-X: M-^X   == 4 columns
	 * 
	 * put_line() prints them out.
	 */
	if (show_opt) {
		if (!iswprint(c)) {
			if (iswcntrl(toascii(c)))
				NEW_COLUMN(4);		/* M-^X */
			else
				NEW_COLUMN(3);		/* M-X */
			*curr++ = c;
			return(0);
		} else if (iswcntrl(c)) {
			NEW_COLUMN(2);			/* ^X */
			*curr++ = c;
			return(0);
		}
	}
	else {
		if (!iswprint(c)) {
			*curr++ = c;
			return(0);
		}
	}
	
	/*
	 * Ordinary character.  Just put it in the buffer.
	 */
	if (mbcodeset) {
		if ((w = wcwidth(c)) < 0)
			return(1);
		NEW_COLUMN(w);
	}
	else {
		NEW_COLUMN(1);
	}
	*curr++ = c;
	return(0);
}
check_line()
{
	int count;

	count = check_esc(linebuf);
	if( count > esc_count){
		column -= (count - esc_count);
	}
	else  return(0);
	esc_count = count;
	return(1);
}

/*
 * Analogous to forw_line(), but deals with "raw lines":
 * lines which are not split for screen width.
 * {{ This is supposed to be more efficient than forw_line(). }}
 */
off_t
forw_raw_line(off_t curr_pos)
{
	register wchar_t *p;
	register wint_t c;
	off_t new_pos, ch_tell();

	if (curr_pos == NULL_POSITION || ch_seek(curr_pos) ||
		(c = ch_forw_get()) == EOI)
		return (NULL_POSITION);

	p = linebuf;

	for (;;)
	{
		if (c == L'\n' || c == EOI)
		{
			new_pos = ch_tell();
			break;
		}
		if (p >= &linebuf[NUM_ELEMENTS(linebuf)-1])
		{
			/*
			 * Overflowed the input buffer.
			 * Pretend the line ended here.
			 * {{ The line buffer is supposed to be big
			 *    enough that this never happens. }}
			 */
			new_pos = ch_tell() - 1;
			break;
		}
		*p++ = c;
		c = ch_forw_get();
	}
	*p = L'\0';
	line = linebuf;
	return (new_pos);
}

/*
 * Analogous to back_line(), but deals with "raw lines".
 * {{ This is supposed to be more efficient than back_line(). }}
 */
off_t
back_raw_line(off_t curr_pos)
{
	register wchar_t *p;
	register wint_t c;
	off_t new_pos, ch_tell();

	if (curr_pos == NULL_POSITION || curr_pos <= (off_t)0 ||
		ch_seek(curr_pos-1))
		return (NULL_POSITION);

	p = &linebuf[NUM_ELEMENTS(linebuf)];
	*--p = L'\0';

	for (;;)
	{
		c = ch_back_get();
		if (c == L'\n')
		{
			/*
			 * This is the newline ending the previous line.
			 * We have hit the beginning of the line.
			 */
			new_pos = ch_tell() + 1;
			break;
		}
		if (c == EOI)
		{
			/*
			 * We have hit the beginning of the file.
			 * This must be the first line in the file.
			 * This must, of course, be the beginning of the line.
			 */
			new_pos = (off_t)0;
			break;
		}
		if (p <= linebuf)
		{
			/*
			 * Overflowed the input buffer.
			 * Pretend the line ended here.
			 */
			new_pos = ch_tell() + 1;
			break;
		}
		*--p = c;
	}
	line = p;
	return (new_pos);
}
