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
/*	
 *	@(#)$RCSfile: userif.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:54:42 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#ifndef __USERIF__
#define __USERIF__

/* Copyright (c) 1988-1990 SecureWare, Inc.
 *   All rights reserved
 *

 */

/*
 *	User interface structure definitions, macros, and constants.
 *
 */

/*  NOTE:  all screen positions are 0-based, top left corner of screen
 *	   is at row=0, col=0 or (0, 0).
 */

#include	<curses.h>
#include	<term.h>

/*  For underlining, has_underline is set by the underline() call
 *  the first time through.
 */
extern	int	has_underline;

/*  For lower level screens, set this value so higher level setjmps
 *  will stay in their own routines rather than return (which would be
 *  TWO levels.
 */
extern	int	LowerLevel;

/*  The scrn_desc structure holds information about each field that
 *  appears on the screen.  A table of these, pointed to by the screen
 *  definition structure, defines the fields on the screen.
 */

struct	scrn_desc  {
	uchar	row,		/* row on screen that field starts */
		col,		/* column */
		type,		/* see #defines below */
		len,		/* length of field, in characters */
		inout,		/* see #defines below (input, output, both */
		required;	/* is this field required?  */
	char	*prompt;	/* if menu item or merely display field */
	char	*help;		/* if help available, file with screen text */
			/*  Scrolling region parameters */
	uchar	s_lines,	/* number of lines in region */
		s_spaces,	/* number of spaces between items */
		s_itemsperline;	/* number of items on each line */
			/* These fields are used internally by getscreen */
	ushort	s_topleft;	/* item occupying top left entry of region */
	struct	scrn_struct	*scrnstruct;	/* structure for this field */
};

/* types of scrn_desc's */

#define FLD_PROMPT	1	/* just text */
#define FLD_ALPHA	2	/* alphanumeric (isprint) */
#define FLD_NUMBER	3	/* number field (isdigit) */
#define FLD_YN		4	/* yes/no field */
#define	FLD_POPUP	5	/* popup field (answering yes causes
				   popup window to appear */
#define FLD_SCROLL	6	/* scrolling region */
#define FLD_CONFIRM	7	/* confirmation required */
#define FLD_CHOICE	8	/* one of choices on menu screen */

/* the required field can be either yes or no */

#ifndef YES
#define YES 1
#endif
#ifndef NO
#define NO 0
#endif

/* indicators of whether to display, solicit, or both */
#define FLD_INPUT	1	/* input-only field */
#define FLD_OUTPUT	2	/* output-only field */
#define FLD_BOTH	3	/* both input and output to field */

/*  The screen structure holds parameters about the screen itself, including
 *  the type of screen, how much space it occupies, and where.
 */

struct	scrn_parms  {
	uchar	scrntype,	/*  see #defines below  */
		toprow,		/*  top row occupied by screen */
		leftcol,	/*  leftmost column */
		nbrrows,	/*  number of rows  */
		nbrcols,	/*  number of columns */
		ndescs;		/*  number of scrn_desc's describing screen */
	struct	scrn_desc *sd;	/*  table of scrn_desc's */
	struct	menu_scrn *ms;	/*  table of menu choices for screen */
};

/*  screen types:  */
#define	SCR_MENU	1	/*  menu screen */
#define SCR_FILLIN	2	/*  fill in the blanks screen */
#define SCR_MENUPROMPT	3	/*  menu screen with prompts */
#define SCR_MSGONLY	4	/*  message only (press return to cont.) */
#define SCR_NOCHANGE	5	/*  can move around but not change fields */

/* formula for calculating number of scrn_descs in a table */
#define NUMDESCS(sd)	(sizeof(sd) / sizeof (struct scrn_desc))

/*  Each screen has a table of data descriptions that describe the
 *  fields for input and output items.  Each of these will typically
 *  point to members of a structure defining the data items on the
 *  screen.
 *  The intent is to relate each of these to a prompt field on the
 *  screen.  Need to maintain a relationship between the descriptions
 *  in the scrn_desc table and the prompt elements.
 */

struct	scrn_struct  {
	char	*pointer;
	uchar	changed;	/* set if user changed field */
	ushort	filled;		/* set if field filled in by program */
	uchar	desc;		/* which scrn_desc corresponds to this */
	int	(*validate)();	/* for screen functions after getscreen */
};

/* if alphanumeric field, char string */
#define	s_alpha(sp)		(sp->pointer)
/* if numeric field, int containing number */
#define s_number(sp)	((long *) sp->pointer)
/* yes/no field, contained in one byte */
#define s_yesno(sp)	((uchar *) sp->pointer)
/* confirm field, yes/no in a byte */
#define s_confirm(sp)	((uchar *) sp->pointer)
/* popup field, yes/no in a byte */
#define s_popup(sp)	((uchar *) sp->pointer)
/* scrolling region table of strings */
#define s_scrollreg(sp)	((char **) (sp->pointer))

/*  The relationship between a tree of menu and menu-prompt screens
 *  is described by a table that is associated with the screen
 *  description for a SCR_MENU or SCR_MENUPROMPT screen.  This table
 *  tells what to do when the user picks one of the items off the
 *  menu.  The action can be either to run a program, to pop up a new
 *  menu, or to call a routine.  A routine is called with the number of
 *  choice taken and an argv[]-like list of parameters.  A program is
 *  called with argv[1] equal to the number of the choice, and the rest
 *  of the arguments equal to the strings entered into the MENUPROMPT
 *  fields.  Menus are stepped through until a program or routine is called.
 */

struct	menu_scrn {
	int	choice;			/* which one was picked */
	int	type;			/* the type of action to take */
	struct	scrn_parms *next_menu;	/* if next menu screen */
	int 	(*next_routine)();	/* if next is routine to call */
	char	*next_program;		/* if next is program to run */
	int	nparams;		/* number of parameters to pass */
};

#define M_MENU	  1
#define M_ROUTINE 2
#define M_PROGRAM 3

/* argument to putscreen */

#define	CLEAR	1	/* clear screen before displaying window */
#define POP	2	/* pop window on screen to display window */

/* Screen return values */

struct	scrn_ret {
	uchar	item;		/* last item on the screen that changed */
	unsigned short	flags;	/* Plexus warns against <4 byte structures */
};
#define	R_CONFIRM 	0x01	/* a confirm entry was changed */
#define R_EXECUTE 	0x02	/* user said to execute the screen */
#define R_CHANGED 	0x04	/* screen was changed */
#define R_ABORTED 	0x08	/* user aborted screen entry */
#define R_QUIT		0x10	/* user wants to quit entirely */
#define R_POPUP		0x20	/* popup entry was selected */

/* reasons that signals were caught: */
#define	INTERRUPT	1
#define QUIT		2
#define HANGUP		3
#define ABORT		4
#define CONTINUE	5

/* signal handling globals: */
extern	int	hup_caught;	/* set when hangup signal caught */
extern	void	hup_catch();	/* routine that sets hup_caught */

/* declarations for screen handling routines */

extern struct	scrn_ret	getscreen();
extern WINDOW	*putscreen();

/* screen messages use these print conventions */

#define YESCHAR		'Y'
#define NOCHAR		'N'
#define DEFAULTCHAR	'D'

/* declarations for utility routines for screen functions */

extern struct	scrn_desc *copy_desc();
extern struct	scrn_struct *copy_struct();
extern int	userif_zero(), userif_one();
extern void	userif_void();
/* constant width table manipulation routines */
extern char	**expand_cw_table(),
		**alloc_cw_table();
extern void	free_cw_table(),
		sort_cw_table();
/* popen's that capture both stdout and stderr */
extern FILE	*popen_all_output();
extern void	pclose_all_output();

/* for debugging malloc problems - routines are at the bottom of scrnsubs.c */

#ifdef DEBUG_MALLOC
extern char *dbmalloc(), *dbrealloc(), *dbcalloc(), *dbstrdup();
extern void dbfree();
#define malloc(bytes)  dbmalloc(bytes)
#define realloc(pointer,bytes)	dbrealloc(pointer,bytes)
#define free(pointer)		dbfree(pointer)
#define calloc(nelem,elsize)	dbcalloc(nelem,elsize)
#define strdup(pointer)		dbstrdup(pointer)
#else
extern char *malloc(), *realloc(), *calloc(), *strdup();
extern void free();
#endif

#define COPYSCRN_DISP(fillinstruct,name)				\
  static								\
  struct	scrn_parms *						\
  name (fill, parms_template)						\
  struct	fillinstruct	*fill;					\
  struct	scrn_parms	*parms_template;			\
  {									\
	struct	scrn_parms	*sp;					\
 									\
	if (sp = (struct scrn_parms *) malloc (sizeof (*sp))) {		\
		*sp = *parms_template;					\
		sp->scrntype = SCR_MSGONLY;				\
		fill->ndescs = parms_template->ndescs;			\
	} else {							\
		pop_msg ("Not enough memory to allocate screen parameters.", \
		"Please report problem and re-run program.");		\
	}								\
	return (sp);							\
  }

#define COPYSCRN_NOCHANGE(fillinstruct,name)				\
  static								\
  struct	scrn_parms *						\
  name (fill, parms_template)						\
  struct	fillinstruct	*fill;					\
  struct	scrn_parms	*parms_template;			\
  {									\
	struct	scrn_parms	*sp;					\
 									\
	if (sp = (struct scrn_parms *) malloc (sizeof (*sp))) {		\
		*sp = *parms_template;					\
		sp->scrntype = SCR_NOCHANGE;				\
		fill->ndescs = parms_template->ndescs;			\
	} else {							\
		pop_msg ("Not enough memory to allocate screen parameters.", \
		"Please report problem and re-run program.");		\
	}								\
	return (sp);							\
  }

#define COPYSCRN_UPDATE(fillinstruct,name)				\
  static								\
  struct	scrn_parms *						\
  name (fill, parms_template) 						\
  struct	fillinstruct	*fill; 					\
  struct	scrn_parms	*parms_template; 			\
  { 									\
	struct	scrn_parms	*sp; 					\
 									\
	if (sp = (struct scrn_parms *) malloc (sizeof (*sp))) { 	\
		*sp = *parms_template; 					\
		fill->ndescs = parms_template->ndescs;			\
	} else {							\
		pop_msg ("Not enough memory to allocate screen parameters.", \
		"Please report problem and re-run program.");		\
	}								\
	return (sp);							\
  }

#define	BDESC_DISP(fillinstruct, name)					\
  static								\
  struct	scrn_desc *						\
  name (fill, desc_template, struct_template)				\
  struct	fillinstruct	*fill;					\
  struct	scrn_desc	*desc_template;				\
  struct	scrn_struct	*struct_template;			\
  {									\
	int	i;							\
	struct	scrn_desc	*sd;					\
									\
	if (sd = (struct scrn_desc *) calloc (fill->ndescs, sizeof (*sd))) \
		for (i = 0; i < fill->ndescs; i++)  {			\
			sd[i] = desc_template[i];			\
			sd[i].inout = FLD_OUTPUT;			\
		}							\
	else {								\
		pop_msg ("Not enough memory to allocate screen descriptors.", \
		"Please report problem and re-run program.");		\
	}								\
	return (sd);							\
  }
#endif
