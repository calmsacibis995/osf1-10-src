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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/* @(#)$RCSfile: ttyent.h,v $ $Revision: 4.2.5.2 $ (OSF) $Date: 1993/06/08 01:14:52 $ */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *      ttyent.h    5.1 (Berkeley) 5/30/85
 */
#ifndef _TTYENT_H_
#define _TTYENT_H_

#include <standards.h>

struct	ttyent {		/* see getttyent(3) */
    char *ty_name;		/* terminal device name */
    char *ty_getty;		/* command to execute, usually getty */
    char *ty_type;		/* terminal type for termcap (3X) */
    int	ty_status;		/* status flags (see below for defines) */
    char *ty_window;		/* command to start up window manager */
    char *ty_comment;		/* usually the location of the terminal */
};

#define TTY_ON		0x1	/* enable logins (startup getty) */
#define TTY_SECURE	0x2	/* allow root to login */

extern struct ttyent *getttyent __((void));
extern struct ttyent *getttynam __((const char *));
extern int setttyent __((void));
extern void endttyent __((void));

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
extern int getttyent_r __((struct ttyent *, char *, int, FILE **));
extern int getttynam_r __((const char *, struct ttyent *, char *, int));
extern int setttyent_r __((FILE **));
extern void endttyent_r __((FILE **));
#endif	/* _REENTRANT || _THREAD_SAFE */

#endif /* _TTYENT_H_ */
