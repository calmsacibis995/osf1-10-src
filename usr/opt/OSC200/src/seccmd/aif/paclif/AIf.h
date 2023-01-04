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
 * @(#)$RCSfile: AIf.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:10:43 $
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	AIf.h,v $
 * Revision 1.1.1.3  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  16:20:14  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  16:19:26  hosking]
 *
 * Revision 1.1.2.5  1992/04/21  13:32:44  marquard
 * 	Fixed gl_uid and gl_gid initializations.
 * 	[1992/04/20  21:07:13  marquard]
 * 
 * Revision 1.1.2.4  1992/04/08  20:28:02  marquard
 * 	Fixed casting problem.
 * 	[1992/04/05  22:11:42  marquard]
 * 
 * Revision 1.1.2.3  1992/04/05  18:19:04  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:51:23  marquard]
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
/* @(#)AIf.h	1.1 11:15:10 11/8/91 SecureWare */
/* Copyright (c) 1988-90 SecureWare, Inc.
 *   All rights reserved
 *
 * Based on OSF version:
 * 	@(#)AIf.h	2.33 16:30:28 5/17/91 SecureWare
 */

#ifndef __AIF_H__
#define __AIF_H__

#ifdef SEC_BASE

/*
 *	User interface structure definitions, macros, and constants.
 *
 */

/*  NOTE:  all screen positions are 0-based, top left corner of screen
 *	   is at row=0, col=0 or (0, 0).
 */

/*
 *	this following mess make sure we get SysV curses
 *	even with BSD-ish code elsewhere.
 */

#ifdef _BSD
#define _BSD_WAS_DEFINED
#undef _BSD
#endif

#include	<curses.h>
#ifdef _BSD_WAS_DEFINED
#define _BSD
#undef _BSD_WAS_DEFINED
#endif
#include	<term.h>
#include	<values.h>	/* for MAXINT */
#include "aif_msg.h"
nl_catd catd;

/* Just to get rid of the definition in ../../common/If.h */
#ifdef MSGSTR
#undef MSGSTR
#endif

#define MSGSTR(n,s) catgets(catd,MS_AIF,n,s)

/*  For underlining, has_underline is set by the underline() call
 *  the first time through.
 */
extern	int	has_underline;


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
		s_itemsperline,	/* number of items on each line */
			/*  various state information */
		valid_on_leave;	/* validate when leaving field by "select" */
	int	index;		/* index into FLD_SCROLL or FLD_SCRTOG array */
			/* These fields are used internally by getscreen */
	struct	scrn_struct	*scrnstruct;	/* structure for this field */
	char	*which_ss;	/* current screen val for SCRTOG */
	ushort	s_topleft;	/* item occupying top left entry of region */
};

typedef struct scrn_desc	Scrn_desc;

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
#define	FLD_SKIP	9	/* vert. placeholder - ignore it */
#define	FLD_BLANK	10	/* horiz. placeholder - ignore it */
#define	FLD_TOGGLE	11	/* toggle field */
#define	FLD_SCRTOG	12	/* scrolling toggle field */
#define	FLD_TEXT	13	/* multiline text field */

/* the required field can be either yes or no */

#ifndef YES
#define YES 1
#endif
#ifndef NO
#define NO 0
#endif

/* if TOGGLE field is 1 char long, these are ON & OFF */

#define TOGGLEON	((uchar) '*')
#define TOGGLEOFF	((uchar) '_')

/*
 * for longer TOGGLE/SCRTOG fields, these surround the field
 */
#define TOGGLECL	'['
#define TOGGLECR	']'

/* indicators of whether to display, solicit, or both */
#define FLD_INPUT	1	/* input-only field */
#define FLD_OUTPUT	2	/* output-only field */
#define FLD_BOTH	3	/* both input and output to field */

/*
 * Header structure for screens, including the surrounding head/foot lines
 */

typedef struct {
	char *title,			/* title (line 3) */
	     *lh,			/* left header (line 1) */
	     *ch,			/* center header (line 1) */
	     *rh,			/* right header (line 1) */
	     *lf,			/* left footer (line 1) */
	     *cf,			/* center footer (line 1) */
	     *rf,			/* right footer (line 1) */
	     *c3,			/* command prompt line # 3 */
	     *c2,			/* command prompt line # 2 */
	     *c1,			/* command prompt line # 1 */
	     *help;			/* help string */
} Scrn_hdrs;

/*  The screen structure holds parameters about the screen itself, including
 *  the type of screen, how much space it occupies, and where.
 */

struct	scrn_parms  {
	uchar	scrntype,	/* see #defines below  */
		toprow,		/* top row occupied by screen */
		leftcol,	/* leftmost column */
		nbrrows,	/* number of rows  */
		nbrcols,	/* number of columns */
		ndescs,		/* number of scrn_desc's describing screen */
		inuse,		/* screen in use- y=1, n=0 */
		si;		/* ss is initialized? y=1, n=0 */
	struct	scrn_desc *sd;	/* table of scrn_desc's */
	struct	menu_scrn *ms;	/* table of menu choices for screen */
	WINDOW	*w;		/* window for this screen (subwin after top) */
	char    **text;		/* text for plain text screen */
	Scrn_hdrs *sh;		/* screen decorations */
	struct	scrn_struct *ss;	/* 1st scrn_struct (if inuse) */
	int	(*setup)();	/* do any setup for screen */
	int	(*init)();	/* do scrn_struct init for screen */
	int	(*free)();	/* free any malloc'd data */
	char	*fillin;	/* fillin data struct ptr */
	uchar	skip;		/* don't stop on way back up */
	uchar	first_desc;	/* first scrn_desc to move to when requesting */
};

typedef struct scrn_parms Scrn_parms;

/*  screen types:  */
#define	SCR_MENU	1	/*  menu screen */
#define SCR_FILLIN	2	/*  fill in the blanks screen */
#define SCR_MSGONLY	4	/*  message only (press return to cont.) */
#define SCR_NOCHANGE	5	/*  can move around but not change fields */
#define SCR_TEXT	6	/*  like MSG_ONLY, but simpler */

/*  empty win - blanks all but hdr/ftr lines */

GLOBAL WINDOW *overlay_win INIT1 (NULL);

/* formula for calculating number of scrn_descs in a table */
#define NUMDESCS(sd)	(sizeof (sd) ? (sizeof(sd) / sizeof (struct scrn_desc)) : 0)

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
	char	*state;		/* array of states for FLD_SCRTOG */
	int	(*validate)();	/* for screen functions after getscreen */
	int	(*val_act)();	/* validate on action within a field
				(currently in TEXT, TOGGLE, SCRTOG) */
	uchar	desc;		/* which scrn_desc corresponds to this */
	uchar	changed;	/* set if user changed field */
	ushort	filled;		/* set if field filled in by program */
};

typedef struct scrn_struct	Scrn_struct;

/* if alphanumeric field, char string */
#define	s_alpha(sp)		((sp)->pointer)

/* if numeric field, int containing number */
#define s_number(sp)	((long *) (sp)->pointer)

/* yes/no field, contained in one byte */
#define s_yesno(sp)	((uchar *) (sp)->pointer)

/* confirm field, yes/no in a byte */
#define s_confirm(sp)	((uchar *) (sp)->pointer)

/* popup field, yes/no in a byte */
#define s_popup(sp)	((uchar *) (sp)->pointer)

/* scrolling region table of strings */
#define s_scrollreg(sp)	((char **) ((sp)->pointer))

/* toggle field */
#define s_toggle(sp)	((uchar *) (sp)->pointer)

/* scrolling toggle field */
#define s_scrtog(sp)	((char **) ((sp)->pointer))

/* scrolling toggle field state */
#define s_ststate(sp)	((sp)->state)

/* if text field, char string */
#define	s_text(sp)	((char **) (sp)->pointer)

/*  The relationship between a tree of menu and menu-prompt screens
 *  is described by a table that is associated with the screen
 *  description for a SCR_MENU screen.  This table
 *  tells what to do when the user picks one of the items off the
 *  menu.  The action can be either to run a program, to pop up a new
 *  menu, or to call a routine.  A routine is called with the number of
 *  choice taken and an argv[]-like list of parameters.  A program is
 *  called with argv[1] equal to the number of the choice.
 *  Menus are stepped through until a program or routine is called.
 */

struct	menu_scrn {
	int	choice;			/* which one was picked */
	int	type;			/* the type of action to take */
	struct	scrn_parms *next_menu;	/* if next menu screen */
	int 	(*next_routine)();	/* if next is routine to call */
	char	*next_program;		/* if next is program to run */
	int	nparams;		/* number of parameters to pass */
	struct	scrn_parms *help;	/* if help menu screen avail. */
};

typedef struct menu_scrn	Menu_scrn;

/*
 * action type
 */
#define M_MENU	  1
#define M_ROUTINE 2

/* argument to putscreen */

#define	CLEAR	1	/* clear screen before displaying window */
#define POP	2	/* pop window on screen to display window */

/* Screen return values */

struct	scrn_ret {
	unsigned short	flags;
	uchar	item;		/* last item on the screen that changed */
};

typedef struct scrn_ret		Scrn_ret;

#define	R_CONFIRM 	0x01	/* a confirm entry was changed */
#define R_EXECUTE 	0x02	/* user said to execute the screen */
#define R_CHANGED 	0x04	/* screen was changed */
#define R_ABORTED 	0x08	/* user aborted screen entry */
#define R_QUIT		0x10	/* user wants to quit entirely */
#define R_POPUP		0x20	/* popup entry was selected */
#define R_HELP		0x40	/* help was selected */
#define R_ACTHELP	0x80	/* action help was selected */

/* Exit Codes (was: reasons that signals were caught): */
#define	INTERRUPT	1
#define QUIT		2	/* result of QUITPROG or exit top screen */
#define HANGUP		3
#define ABORT		4	/* result of QUITMENU */
#define CONTINUE	5

/* for debugging malloc problems - routines are in dbmalloc.c (not used now) */

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

/*
 * MENU_DESC()	- M_MENU menu_scrn entry
 * ROUT_DESC()	- M_ROUTINE menu_scrn entry
 */
#define MENU_DESC(NUM, NAM, HLP)	{NUM, M_MENU, NAM, 0, 0, 0, HLP}
#define ROUT_DESC(NUM, NAM, NARG, HLP)	{NUM, M_ROUTINE, 0, NAM, 0, NARG, HLP}

/*
 * SCRN_PARMS() - simple scrn_parms data structure
 * SKIP_PARMS() - simple scrn_parms data structure - leave on execute
 * SCRN_PARMF() - scrn_parms data structure with scrnfuncs
 * SKIP_PARMF() - scrn_parms data structure with scrnfuncs - leave on execute
 */
#define SCRN_PARMS(SCR, TYP, DESC, MENU, TEXT, DECS)\
	Scrn_parms SCR = {TYP, 0, 0, 0, 0, NUMDESCS (DESC), 0, 0, DESC, \
		MENU, NULL, TEXT, DECS, NULL, NULL, \
		NULL, NULL, NULL, 0, (uchar) -1}

#define SKIP_PARMS(SCR, TYP, DESC, MENU, TEXT, DECS)\
	Scrn_parms SCR = {TYP, 0, 0, 0, 0, NUMDESCS (DESC), 0, 0, DESC, \
		MENU, NULL, TEXT, DECS, NULL, NULL, \
		NULL, NULL, NULL, 1, (uchar) -1}

#define SCRN_PARMF(SCR, TYP, DESC, MENU, TEXT, DECS, R_SETUP, R_INIT, R_FREE)\
	Scrn_parms SCR = {TYP, 0, 0, 0, 0, NUMDESCS (DESC), 0, 0, DESC, \
		MENU, NULL, TEXT, DECS, NULL, \
		R_SETUP, R_INIT, R_FREE, NULL, 0, (uchar) -1}

#define SKIP_PARMF(SCR, TYP, DESC, MENU, TEXT, DECS, R_SETUP, R_INIT, R_FREE)\
	Scrn_parms SCR = {TYP, 0, 0, 0, 0, NUMDESCS (DESC), 0, 0, DESC, \
		MENU, NULL, TEXT, DECS, NULL, \
		R_SETUP, R_INIT, R_FREE, NULL, 1, (uchar) -1}

/* header, footer, title & command line locations */

GLOBAL int HDR_LINE	 INIT1 (0);
GLOBAL int TITLE_LINE	 INIT1 (2);
GLOBAL int MENU_LINE1	 INIT1 (4);
GLOBAL int FTR_LINE	 INIT1 (0);
GLOBAL int CMDS_LINE1	 INIT1 (0);
GLOBAL int CMDS_LINE2	 INIT1 (0);
GLOBAL int CMDS_LINE3	 INIT1 (0);

/* messages window position/size data */

GLOBAL int WMSGROW	 INIT1 (0);
GLOBAL int WMSGCOL	 INIT1 (0);
GLOBAL int WMSGLEN	 INIT1 (0);

/* menu window size data */

GLOBAL int MENU_ROWS	 INIT1 (0);
GLOBAL int MENU_COLS	 INIT1 (0);

GLOBAL  Scrn_parms *TopScrn;

/* cursor states */

#define NORMALCURSOR	0
#define BLOCKCURSOR	1
#define INVISCURSOR	2
#define UNDERCURSOR	3

GLOBAL int CURSOR_STATE INIT1 ( NORMALCURSOR );

GLOBAL char MT[] INIT1 ("");	/* empty string for inits */
GLOBAL char SMT[] INIT1 ("");	/* empty string for inits */

/*	traverse() read/write parameter values	*/

#define TRAV_RW	0	/* putscreen() & getscreen() */
#define TRAV_WO	1	/* putscreen() only */


/* constants used by utility routines */

#ifndef ON
#define ON 	1
#define OFF	0
#endif /* ON */

/* Screen state used by screen i/o routines */

struct	state {
	uchar	curfield;	/* which scrn_desc is the current input */
	uchar	firstfield;	/* which scrn_desc is first input */
	uchar	lastfield;	/* which scrn_desc is last input */
	uchar	itemchanged;	/* whether item currently editing changed */
	uchar	columninfield;	/* current column in field */
	uchar	insert;		/* whether in insert mode */
	uchar	message;	/* whether message field on screen */
	struct	scrn_struct	*structp;	/* current struct table */
	struct	scrn_parms	*screenp;	/* current screen */
	WINDOW	*window;			/* current window */
	struct	scrn_ret	ret;		/* for return value */
	ushort	scrollitem;	/* in scrolling region, field currently in */
	ushort	topscroll;	/* upper left field is this offset in table */
	char	scrnrep[80];	/* screen representation of current field */
};

/*  error messages */

GLOBAL char *REQUIREDERROR	INIT1 ("Field must be filled");
GLOBAL char *NOTNUMBERERROR	INIT1 ("Digits only in a number field");
GLOBAL char *YESNOERROR		INIT1 ("Answer (y)es or (n)o in this field");
GLOBAL char *NOPROMPT		INIT1 ("Tried to move to prompt field!!");

GLOBAL char *PRESSRETURN	INIT1 ("Press <RETURN> to continue");
GLOBAL char *PRESSSHORT		INIT1 ("<RETURN>");
GLOBAL char *NOHELP		INIT1 ("No help available for this field");
GLOBAL char *BLANKSCROLL	INIT1 ("No blank fields in scrolling regions");
GLOBAL char *FULLSCROLL		INIT1 ("The scrolling region is full");

#define LONGESTMSG		strlen(BLANKSCROLL)

GLOBAL char *screen_copyr INIT1 ("Copyright (c) 1991 SecureWare, Inc.");


#define	TERMINFODIR	"/usr/lib/terminfo"
#define TTYTYPE_FILE	"/etc/ttytype"

/* convert the current field in the state structure to a scrn_desc */
#define		sttosd(stp)	&((stp)->screenp->sd[(stp)->curfield])
#define		lastsd(stp)	&((stp)->screenp->sd[(stp)->lastfield])
#define		firstsd(stp)	&((stp)->screenp->sd[(stp)->firstfield])


/* Help directory which stores the root of the help tree */
GLOBAL char *HelpDir
# ifdef STD_UNIX
	INIT1 ("/tcb/files/auth/help/");
# else   /* STD_UNIX */
	INIT1 ("/usr/share/lib/sechelp/");
# endif  /* STD_UNIX */

/*
 * various variables used in headers, footers, titles - those with
 * strings starting with a '$' are runtime variables changed on the fly
 */

GLOBAL char
	cur_date []	INIT1 ("$DATE"),
	cur_time []	INIT1 ("$TIME"),
	cur_user []	INIT1 ("$USER"),
	cur_group []	INIT1 ("$GROUP"),
	cur_uid []	INIT1 ("$UID"),
	cur_gid []	INIT1 ("$GID"),
	cur_dev []	INIT1 ("$DEV"),
	*sec_officer	INIT1 ("Security Officer"),
	*sys_admin	INIT1 ("System Administrator"),
	*hot_dog	INIT1 ("Sys Adm. & Sec. Officer"),
	runner []	INIT1 ("$RUNNER");

/*
 * data storage for runtime variables
 */

GLOBAL char *gl_user INIT1 ({NULL});		/* global user name */
GLOBAL char *gl_group INIT1 (NULL);		/* global group name */
GLOBAL long gl_uid INIT1 (0L);		/* global user id */
GLOBAL long gl_gid INIT1 (0L);		/* global group id */
GLOBAL char *gl_device INIT1 (NULL);		/* global device name */
GLOBAL char *gl_host INIT1 (NULL);		/* global host name */
GLOBAL char *gl_printer INIT1 (NULL);		/* global printer name */
GLOBAL char *gl_tape INIT1 (NULL);		/* global tape name */
GLOBAL char *gl_terminal INIT1 (NULL);		/* global terminal name */
GLOBAL char *gl_runner INIT1 (NULL);		/* who's running the program */

#define BIGNEGNUM	-(MAXINT - 1)
#define BOGUS_ID	BIGNEGNUM		/* uid/gid not filled yet */

#endif /* SEC_BASE */
#endif /* __AIF_H__ */
