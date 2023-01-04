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
static char rcsid[] = "@(#)$RCSfile: defs.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 15:15:31 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 */

#include	<sys/limits.h>
#include	<sys/types.h>
#include 	<setjmp.h>
#include	"mode.h"
#include	"name.h"

int	_open_max;

/* temp files and io */

int		output = 2;
int		ioset;
struct ionod	*iotemp;	/* files to be deleted sometime */
struct ionod	*fiotemp;	/* function files to be deleted sometime */
struct ionod	*iopend;	/* documents waiting to be read at NL */
struct fdsave	*fdmap;

/* substitution */
int		dolc;
uchar_t		**dolv;
struct dolnod	*argfor;
struct argnod	*gchain;


/* name tree and words */
int		wdval;
int		wdnum;
int		fndef;
int		nohash;
struct argnod	*wdarg;
int		wdset;
BOOL		reserv;

/* special names */
uchar_t		*pcsadr;
uchar_t		*pidadr;
uchar_t		*cmdadr;

/* transput */ 
uchar_t 	*tempname;
unsigned int 	serial; 
unsigned int 	peekc;
unsigned int 	peekn;
uchar_t 	*comdiv;

long		flags;
int		rwait;	/* flags read waiting */

/* error exits from various parts of shell */
jmp_buf		subshell;
jmp_buf		errshell;

/* fault handling */
BOOL		trapnote;
BOOL		mailalarm;

/* execflgs */
int		exitval;
int		retval;
BOOL		execbrk;
int		loopcnt;
int		breakcnt;
int 		funcnt;

int		wasintr;	/* used to tell if break or delete is hit
	   			   while executing a wait
				*/

int		eflag;

/* The following stuff is from stak.h	*/

uchar_t		*stakbas;
uchar_t		*staktop;
uchar_t		*stakbot;
uchar_t		*stakbsy;
uchar_t		*brkend;
