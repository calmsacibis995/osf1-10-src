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
 *	@(#)$RCSfile: aif.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:57:15 $
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
#ifdef SEC_BASE
#ifndef _AIF_H
#define _AIF_H



/*
 * aif.h - ASCII user interface data structures, macros, etc.
 */

#include	<values.h>
#include	<sys/types.h>
#include	"gl_defs.h"

typedef struct {	/* SCREEN DECORATIONS */
	uchar *title,		/* title (line 3) */
	     *lh,			/* left header (line 1) */
	     *ch,			/* center header (line 1) */
	     *rh,			/* right header (line 1) */
	     *lf,			/* left footer (line 1) */
	     *cf,			/* center footer (line 1) */
	     *rf,			/* right footer (line 1) */
	     *c3,			/* command prompt line # 3 */
	     *c2,			/* command prompt line # 2 */
	     *c1;			/* command prompt line # 1 */
} Scrn_hdrs;

/*
 * various variables used in headers, footers, titles - those with
 * strings starting with a '$' are runtime variables changed on the fly
 */
GLOBAL uchar
	main_menu_t []	INIT1 ("MAIN MENU"),
	cur_date []	INIT1 ("$DATE"),
	cur_time []	INIT1 ("$TIME"),
	cur_user []	INIT1 ("$USER"),
	cur_group []	INIT1 ("$GROUP"),
	cur_uid []	INIT1 ("$UID"),
	cur_gid []	INIT1 ("$GID"),
	cur_dev []	INIT1 ("$DEV"),
	sec_officer []	INIT1 ("Security Officer"),
	sys_admin []	INIT1 ("System Administrator"),
	hot_dog []	INIT1 ("Sys Adm. & Sec. Officer"),
	runner []	INIT1 ("$RUNNER"),
	nada []		INIT1 ("");

/*
 * data storage for runtime variables
 */

GLOBAL uchar *gl_user INIT1 ({NULL});		/* global user name */
GLOBAL uchar *gl_group INIT1 (NULL);		/* global group name */
GLOBAL long gl_uid INIT1 (NULL);		/* global user id */
GLOBAL long gl_gid INIT1 (NULL);		/* global group id */
GLOBAL uchar *gl_device INIT1 (NULL);		/* global device name */
GLOBAL uchar *gl_host INIT1 (NULL);		/* global host name */
GLOBAL uchar *gl_printer INIT1 (NULL);		/* global printer name */
GLOBAL uchar *gl_tape INIT1 (NULL);		/* global tape name */
GLOBAL uchar *gl_terminal INIT1 (NULL);		/* global terminal name */
GLOBAL uchar *gl_runner INIT1 (NULL);		/* who's running the program */

/*
 * command explanations that go at bottom of screen (c[123] in Scrn_hdrs)
 */

GLOBAL uchar
	cmds_line1 []
		INIT1 ("RET=Execute  ESC=Leave menu  ^Y=Item help  ^B=Quit Program"),
	cmds_altline1 []
		INIT1 ("RET=Check  ESC=Leave menu  ^Y=Item help  ^B=Quit Program"),
	cmds_short []
		INIT1 ("ESC=Leave menu  ^B=Quit Program"),
	cmds_line2 [] INIT1 ( { NULL } ),
	cmds_line3 [] INIT1 ( { NULL } ),
	cmds_screen []
		INIT1 ("^N=Next screen  ^P=Previous screen"),
	cmds_yesno []
		INIT1 ("SPACE=Select"),
	cmds_tgroup []
		INIT1 ("SPACE=Select"),
	cmds_tuser []
		INIT1 ("SPACE=Select"),
	cmds_tcgroup []
		INIT1 ("<SPACE=Select  ^A=Add new group"),
	cmds_titem []
		INIT1 ("SPACE=Select"),
	cmds_anykey []
		INIT1 ("Press any key to continue");

/*
 * main header declaration
 */
GLOBAL Scrn_hdrs main_hdrs INIT10 (
	main_menu_t,
	cur_date,
	runner,
	cur_time,
	nada,
	nada,
	nada,
	nada,
	cmds_titem,
	cmds_line1
);

#define BIGNEGNUM	-(MAXINT - 1)
#define BOGUS_ID	BIGNEGNUM		/* uid/gid not filled yet */

#endif /* _AIF_H */
#endif /* SEC_BASE */
