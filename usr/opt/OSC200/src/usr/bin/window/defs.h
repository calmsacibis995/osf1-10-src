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
 *	@(#)$RCSfile: defs.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/15 17:44:56 $
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
/*
 * defs.h
 *
 *	Revision History:
 *
 * 12-MAr-91	Fred Canter
 *	Multiple include protection.
 *
 */

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	defs.h	3.18 (Berkeley) 6/29/88
 */
#ifndef	_DEFS_H_
#define	_DEFS_H_

#include "ww.h"
#include <sys/time.h>

#define NWINDOW 9

struct timeval starttime;

struct ww *window[NWINDOW];	/* the windows */
struct ww *selwin;		/* the selected window */
struct ww *lastselwin;		/* the last selected window */
struct ww *cmdwin;		/* the command window */
struct ww *framewin;		/* the window for framing */
struct ww *boxwin;		/* the window for the box */
struct ww *fgwin;		/* the last foreground window */

#define isfg(w)		((w)->ww_order <= fgwin->ww_order)

char *shell[128];		/* the shell argv */
char *shellfile;		/* the shell program */
int nbufline;			/* default buffer size for new windows */
char escapec;			/* the escape character */

	/* flags */
char quit;			/* quit command issued */
char terse;			/* terse mode */
char debug;			/* debug mode */
char incmd;			/* in command mode */

#if defined(_NO_PROTO)

struct ww *getwin();
struct ww *openwin();
struct ww *vtowin();
struct ww *openiwin();

#else /* !defined(_NO_PROTO) */

extern struct ww *getwin();
extern struct ww *openwin(int id, int row, int col, int nrow, int ncol,
			  int nline, char *label, char haspty, char hasframe,
			  char *shf, char **sh)
extern struct ww *vtowin(struct value *v, struct ww *w);
extern struct ww *openiwin(int nrow, char *label);

#endif /* defined(_NO_PROTO) */

#include        <nl_types.h>
#include        "window_msg.h"
nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_WINDOW,num,str)

#endif	/* _DEFS_H_ */
