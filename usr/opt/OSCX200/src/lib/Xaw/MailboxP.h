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
 * $XConsortium: MailboxP.h,v 1.20 91/07/19 21:52:57 rws Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Jim Fulton, MIT X Consortium
 */

#ifndef _XawMailboxP_h
#define _XawMailboxP_h

#include <X11/Xaw/Mailbox.h>
#include <X11/Xaw/SimpleP.h>

#ifdef SYSV
#define MAILBOX_DIRECTORY "/usr/mail"
#else
#ifdef SVR4
#define MAILBOX_DIRECTORY "/var/mail"
#else
#define MAILBOX_DIRECTORY "/usr/spool/mail"
#endif
#endif

typedef struct {			/* new fields for mailbox widget */
    /* resources */
    int update;				/* seconds between updates */
    Pixel foreground_pixel;		/* color index of normal state fg */
    String filename;			/* filename to watch */
    String check_command;		/* command to exec for mail check */
    Boolean flipit;			/* do flip of full pixmap */
    int volume;				/* bell volume */
    Boolean once_only;			/* ring bell only once on new mail */
    /* local state */
    GC gc;				/* normal GC to use */
    long last_size;			/* size in bytes of mailboxname */
    XtIntervalId interval_id;		/* time between checks */
    Boolean flag_up;			/* is the flag up? */
    struct _mbimage {
	Pixmap bitmap, mask;		/* depth 1, describing shape */
	Pixmap pixmap;			/* full depth pixmap */
	int width, height;		/* geometry of pixmaps */
    } full, empty;
    Boolean shapeit;			/* do shape extension */
    struct {
	Pixmap mask;
	int x, y;
    } shape_cache;			/* last set of info */
} MailboxPart;

typedef struct _MailboxRec {		/* full instance record */
    CorePart core;
    SimplePart simple;
    MailboxPart mailbox;
} MailboxRec;


typedef struct {			/* new fields for mailbox class */
    int dummy;				/* stupid C compiler */
} MailboxClassPart;

typedef struct _MailboxClassRec {	/* full class record declaration */
    CoreClassPart core_class;
    SimpleClassPart simple_class;
    MailboxClassPart mailbox_class;
} MailboxClassRec;

extern MailboxClassRec mailboxClassRec;	 /* class pointer */

#endif /* _XawMailboxP_h */
