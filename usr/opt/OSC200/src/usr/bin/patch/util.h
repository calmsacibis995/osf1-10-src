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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/* @(#)$RCSfile: util.h,v $ $Revision: 1.1.3.2 $ (OSF) $Date: 1993/06/11 21:16:16 $ */

/* Header: util.h,v 2.0 86/09/17 15:40:06 lwall Exp
 *
 * Log:	util.h,v
 * Revision 2.0  86/09/17  15:40:06  lwall
 * Baseline for netwide release.
 * 
 */

/* and for those machine that can't handle a variable argument list */

#ifdef CANVARARG

#define say1 say
#define say2 say
#define say3 say
#define say4 say
#define ask1 ask
#define ask2 ask
#define ask3 ask
#define ask4 ask
#define fatal1 fatal
#define fatal2 fatal
#define fatal3 fatal
#define fatal4 fatal

#else /* hope they allow multi-line macro actual arguments */

#ifdef lint

#define say1(a) say(a, 0, 0, 0)
#define say2(a,b) say(a, (b)==(b), 0, 0)
#define say3(a,b,c) say(a, (b)==(b), (c)==(c), 0)
#define say4(a,b,c,d) say(a, (b)==(b), (c)==(c), (d)==(d))
#define ask1(a) ask(a, 0, 0, 0)
#define ask2(a,b) ask(a, (b)==(b), 0, 0)
#define ask3(a,b,c) ask(a, (b)==(b), (c)==(c), 0)
#define ask4(a,b,c,d) ask(a, (b)==(b), (c)==(c), (d)==(d))
#define fatal1(a) fatal(a, 0, 0, 0)
#define fatal2(a,b) fatal(a, (b)==(b), 0, 0)
#define fatal3(a,b,c) fatal(a, (b)==(b), (c)==(c), 0)
#define fatal4(a,b,c,d) fatal(a, (b)==(b), (c)==(c), (d)==(d))

#else /* lint */
    /* if this doesn't work, try defining CANVARARG above */
#define say1(a) say(a, Nullch, Nullch, Nullch)
#define say2(a,b) say(a, b, Nullch, Nullch)
#define say3(a,b,c) say(a, b, c, Nullch)
#define say4 say
#define ask1(a) ask(a, Nullch, Nullch, Nullch)
#define ask2(a,b) ask(a, b, Nullch, Nullch)
#define ask3(a,b,c) ask(a, b, c, Nullch)
#define ask4 ask
#define fatal1(a) fatal(a, Nullch, Nullch, Nullch)
#define fatal2(a,b) fatal(a, b, Nullch, Nullch)
#define fatal3(a,b,c) fatal(a, b, c, Nullch)
#define fatal4 fatal

#endif /* lint */

/* if neither of the above work, join all multi-line macro calls. */
#endif

EXT char serrbuf[BUFSIZ];		/* buffer for stderr */

char *fetchname();
int move_file();
void copy_file();
void say();
void fatal();
void ask();
char *savestr();
void set_signals();
void ignore_signals();
void makedirs();
