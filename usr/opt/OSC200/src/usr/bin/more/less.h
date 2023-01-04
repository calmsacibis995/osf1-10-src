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
/*
static char rcsid[] = "@(#)$RCSfile: less.h,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/09/02 18:40:21 $";
 */
/*
 * HISTORY
 */
/*
 * HISTORY
 * $OSF_Log:	less.h,v $
 * Revision 1.1.1.1  93/01/07  08:45:00  devrcs
 *  *** OSF1_1_2B07 version ***
 * 
 * Revision 1.1.2.2  1992/08/24  18:17:08  tom
 * 	New more for POSIX.2/XPG4.
 * 	[1992/08/24  17:30:27  tom]
 *
 * $OSF_EndLog$
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
 *
 *	less.h	5.9 (Berkeley) 6/1/90
 */

#include <sys/types.h>
#include "more_msg.h"

#ifdef SILVER
#define iswprint(c) isprint(c)
#define iswcntrl(c) iscntrl(c)
#define wcwidth(c)  1

#ifndef _WINT_T
#define _WINT_T
        typedef unsigned int wint_t;         /* Wide character */
#endif
#endif

#define	NULL_POSITION	((off_t)(-1))

#define	EOI		(-1)
#define	READ_INTR	(-2)

/*
 * Special chars used to tell put_line() to do something special.
 * Always preceded by ESC_CHAR if they are real control codes.
 * ESC_CHAR is escaped with itself if it is in input stream.
 */
#define	ESC_CHAR	'\001'		/* character preceding all above */
#define	UL_CHAR		'\002'		/* Enter underline mode */
#define	UE_CHAR		'\003'		/* Exit underline mode */
#define	BO_CHAR		'\004'		/* Enter boldface mode */
#define	BE_CHAR		'\005'		/* Exit boldface mode */

#define	CONTROL_CHAR(c)		(iscntrl(c))
#define	CARAT_CHAR(c)		((c == '\177') ? '?' : (c | 0100))

#define	TOP		(0)
#define	TOP_PLUS_ONE	(1)
#define	BOTTOM		(-1)
#define	BOTTOM_PLUS_ONE	(-2)
#define	MIDDLE		(-3)

#define	A_INVALID		-1

#define	A_AGAIN_B_SEARCH	1
#define	A_AGAIN_F_SEARCH	2
#define	A_B_LINE		3
#define	A_B_SCREEN		4
#define	A_B_SCROLL		5
#define	A_B_SEARCH		6
#define	A_DIGIT			7
#define	A_EXAMINE		8
#define	A_FREPAINT		9
#define	A_F_LINE		10
#define	A_F_SCREEN		11
#define	A_F_SCROLL		12
#define	A_F_SEARCH		13
#define	A_GOEND			14
#define	A_GOLINE		15
#define	A_GOMARK		16
#define	A_HELP			17
#define	A_NEXT_FILE		18
#define	A_PERCENT		19
#define	A_PREFIX		20
#define	A_PREV_FILE		21
#define	A_QUIT			22
#define	A_REPAINT		23
#define	A_SETMARK		24
#define	A_STAT			25
#define	A_VISUAL		26
#define	A_TAGFILE		27
#define	A_FILE_LIST		28
#define	A_SF_SCROLL		29
#define	A_SKIP			30
#define	A_SHELL			31

extern nl_catd catd;
#define MSGSTR(Num, Str)	catgets(catd, MS_MORE, Num, Str)

/*
 * prototypes
 */

/* ch.c */
extern int 	ch_seek(off_t);
extern int	ch_seek_byte(off_t);
extern int	ch_end_seek(void);
extern int	ch_beg_seek(void);
extern off_t	ch_length(void);
extern off_t	ch_tell(void);
extern off_t	ch_byte(off_t);
extern wint_t	ch_forw_get(void);
extern wint_t	ch_back_get(void);
extern void	ch_init(int, int);
extern int	ch_addbuf(int);

/* command.c */
extern void	start_mca(int, char *);
extern int	prompt(void);
extern void	commands(void);
extern void	editfile(void);
extern void 	showlist(void);

/* decode.c */
extern void	noprefix(void);
extern int	cmd_decode(int);
extern int 	cmd_search(char *, char *);

/* help.c */
extern void	help(void);

/* input.c */
extern off_t	forw_line(off_t);
extern off_t	back_line(off_t);

/* line.c */
extern void	prewind(void);
extern int	pappend(wint_t);
extern off_t	forw_raw_line(off_t);
extern off_t	back_raw_line(off_t);

#define LINE_LENGTH 1024

/* linenum.c */
extern void	clr_linenum(void);
extern void	add_lnum(int, off_t);
extern int 	find_linenum(off_t);
extern int	currline(int);

/* main.c */
extern int 	edit(char *);
extern void	next_file(int);
extern void 	prev_file(int);
extern char	*save(char *);
extern void	quit(void);

/* option.c */
extern int	option(int, char **);

/* os.c */
extern void	lsystem(char *);
extern int	iread(int, unsigned char *, int);
extern void	intread(void);
extern char	*mglob(char *);
extern char 	*bad_file(char *, char *, u_int);
extern void	strtcpy(char *, char *, int);

/* output.c */
extern void	put_line(void);
extern void	flush(void);
extern void	purge(void);
extern void	putchr(int);
extern void	putwchr(wchar_t);
extern void	putstr(char *);
extern void	error(char *);
extern void	ierror(char *);

/* position.c */
extern off_t	position(int);
extern void	add_forw_pos(off_t);
extern void 	add_back_pos(off_t);
extern void	copytable(void);
extern void	pos_clear(void);
extern int	onscreen(off_t);

/* prim.c */
extern void	eof_check(void);
extern void	squish_check(void);
extern void	forw(int, off_t, int);
extern void	back(int, off_t, int);
extern void	forward(int, int);
extern void	backward(int, int);
extern void	prepaint(off_t);
extern void	repaint(void);
extern void	jump_forw(void);
extern void	jump_back(int);
extern void	jump_percent(int);
extern void	jump_loc(off_t);
extern void	init_mark(void);
extern void	setmark(int);
extern void	lastmark(void);
extern void	gomark(int);
extern int	get_back_scroll(void);
extern int	search(int, char *, int, int);

/* screen.c */
extern void	raw_mode(int);
extern void	get_term(void);
extern void	init(void);
extern void	deinit(void);
extern void	home(void);
extern void	add_line(void);
extern void	lower_left(void);
extern void	bell(void);
extern void	clear(void);
extern void	clear_eol(void);
extern void	so_enter(void);
extern void	so_exit(void);
extern void	ul_enter(void);
extern void	ul_exit(void);
extern void	bo_enter(void);
extern void	bo_exit(void);
extern void	backspace(void);
extern void	putbs(void);

/* signal.c */
extern void	init_signals(int);
extern void 	winch(int);
extern void	psignals(void);

/* tags.c */
extern void	findtag(char *);
extern int	tagsearch(void);

/* ttyin.c */
extern void	open_getchr(void);
extern int	getchr(void);

