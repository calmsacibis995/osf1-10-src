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
static char rcsid[] = "@(#)$RCSfile: curshdr.h,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/12/15 22:13:22 $";
 */
/*
 * HISTORY
 */
/* curshdr.h        1.7  com/lib/curses,3.1,9013 12/14/89 10:58:18 */
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   curshdr.h
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _CURSHDR_H_
#define _CURSHDR_H_

/*
 * IC and IL overheads and costs should be set to this
 * value if the corresponding feature is missing
 */
#define INFINITY 500

/*
 * DEFINE SHORT to be "int" on alpha and "short" everwhere else
 */
#if !defined(_SHORT)
#if defined(__alpha)
#define _SHORT int
#else
#define _SHORT short
#endif
#endif

#ifdef PHASE2
#define ATR_TYPE	0x0000000F
#define ATR_BYTELN	0x000000F0
#define ATR_POS		0x00000F00

#define ATR_CODE	0x00000001
#define ATR_ACS		0x00000002
#define ATR_NEXT	0x00000004

#define get_byteln( c )		(((c)&ATR_BYTELN) >> 4)
#define put_byteln( c, a )	((c) &= ~ATR_BYTELN, (c)|= (a) << 4)
#define get_pos( c )		(((c)&ATR_POS) >> 8)
#define put_pos( c, a )		((c) &= ~ATR_POS, (c)|= (a) << 8)
#define IS_NEXTATR( c )		(((c)&ATR_TYPE) == ATR_NEXT)

#endif

struct line
{
	int     hash;		/* hash value for this line, 0 if not known */
	struct line *next;      /* pointer to the next line in list of lines*/
	_SHORT  bodylen;	/* the cost of redrawing this line */
	_SHORT  length;		/* the number of valid characters in line */
	chtype  *body;		/* the actual text of the line */
#ifdef PHASE2
	chtype  *atr_body;	/* the attribute flag of the line data*/
#endif
};

struct costs {
	_SHORT Cursor_address;
	_SHORT Cursor_home;
	_SHORT Carriage_return;
	_SHORT Tab;
	_SHORT Back_tab;
	_SHORT Cursor_left;
	_SHORT Cursor_right;	/* current cost, which is 1 unless in insert
				 * mode, since you can just output the
				 * character you want to move over */
	_SHORT Right_base;	/* actual cost to do cursor_right */
	_SHORT Cursor_down;
	_SHORT Cursor_up;
	_SHORT Parm_left_cursor;
	_SHORT Parm_right_cursor;
	_SHORT Parm_up_cursor;
	_SHORT Parm_down_cursor;
	_SHORT Column_address;
	_SHORT Row_address;
	unsigned ilvar;		/* Insert line varying part * 32 */
	int   ilfixed;		/* Insert line fixed overhead */
	unsigned dlvar;		/* Delete line varying part * 32 */
	int   dlfixed;		/* Delete line fixed overhead */
	unsigned icvar;		/* Insert char varying part * 32 */
	int   icfixed;		/* Insert char fixed overhead */
	unsigned dcvar;		/* Delete char varying part * 32 */
	int   dcfixed;		/* Delete char fixed overhead */
	/* Should have costs for delete char/delete line here too. */
};
#define _cost(field) (SP->term_costs.field)

struct map {
	char label[16];		/* Text the key is labelled with */
	char sends[16];		/* Characters sent when key is pressed */
	_SHORT keynum;		/* "char" we pass back to program */
};

/* forward declaration for C++ */
#ifdef __cplusplus
struct term;
#endif

struct screen {
	unsigned fl_nonl	: 1;	/* we shouldn't output \n */
	unsigned fl_uppercase	: 1;	/* upper case terminal */
	unsigned fl_normtty	: 1;	/* currently in normal tty mode */
	unsigned fl_my_term	: 1;	/* user names his terminal type */
	unsigned fl_echoit	: 1;	/* in software echo mode */
	unsigned fl_rawmode	: 1;	/* in raw or cbreak mode */
	unsigned fl_endwin	: 1;	/* has called endwin */
	unsigned phys_irm	: 1;	/* physically in insert char mode */
	unsigned virt_irm	: 1;	/* want to be in insert char mode */
	unsigned fl_nodelay	: 1;	/* tty is in nodelay mode */
	struct line **cur_body;	/* physical screen image */
	struct line **std_body;	/* desired screen image */
	struct line *freelist;	/* free space list */
	_SHORT phys_x;		/* physical cursor X position */
	_SHORT phys_y;		/* physical cursor Y position */
	_SHORT virt_x;		/* virtual cursor X position */
	_SHORT virt_y;		/* virtual cursor Y position */
	_SHORT phys_top_mgn;	/* physical top margin of scrolling region */
	_SHORT phys_bot_mgn;	/* physical bottom margin of scrolling region */
	_SHORT des_top_mgn;	/* desired top margin of scrolling region */
	_SHORT des_bot_mgn;     /* desired bottom margin of scrolling region*/
	chtype *curptr;		/* pointer to cursor */
	chtype virt_gr;		/* desired highlight state */
	chtype phys_gr;		/* current highlight state */
	_SHORT winsize;		/* no. of lines on which ID operations done */
	_SHORT doclear;		/* flag to say whether screen garbaged */
	_SHORT baud;		/* baud rate of this tty */
	_SHORT check_input;	/* flag for input check */
	_SHORT check_fd;	/* file descriptor for input check */
	struct _win_st *std_scr;/* primary output screen */
	struct _win_st *cur_scr;/* what's physically on the screen */
	struct term *tcap;	/* TERMCAP strings and storage space */
	struct costs term_costs;/* costs of various capabilities */
	_SHORT kp_state;	/* 1 iff keypad is on, else 0 */
	_SHORT ml_above;	/* # memory lines above top of screen */
	SGTTY	save_tty_buf;	/* current state of this tty */
	FILE *term_file;	/* File to write on for output. */
#ifdef		KEYPAD
	struct map *kp;		/* keypad map */
#endif		/* KEYPAD */
	_SHORT *input_queue;	/* a place to put stuff ungetc'ed */
	FILE *input_file;	/* Where to get keyboard input */
#ifdef PHASE2
	chtype *curatr;		/* pointer to attirute of cursor */
#endif
};

extern struct screen *SP;

#ifdef DEBUG
extern	FILE	*outf;
#endif

#endif /* _CURSHDR_H_ */
