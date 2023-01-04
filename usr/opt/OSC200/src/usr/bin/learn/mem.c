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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: mem.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:38:39 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#if !defined(lint) && !defined(_NOIDENT)

#endif

/*
 * COMPONENT_NAME: (CMDMAN) commands that allow users to read online
 * documentation
 *
 * FUNCTIONS: action, setdid, unsetdid, already, tellwhich, load_keybuff
 *
 * ORIGINS: 26, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * mem.c	1.2  com/cmd/man/learn,3.1,9021 9/14/89 06:40:47
 */

# include "stdio.h"
# include "lrnref.h"

#include "learn_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LEARN,n,s) 

# define SAME 0

#define KWDLEN 30  /* length of the k_wd field in the keys struct below */
static int keybuff_loaded = 1;  /* have we loaded the array yet? */
#define load_array(string) (void) NLstrncpy((void*)keybuff[keybuff_loaded++].k_wd, (void*)string, KWDLEN-1)


struct keys {
	char k_wd[KWDLEN];
	int k_val;
} keybuff[] = {
	{"ready",	READY},
	{"answer",	READY},
	{"#print",	PRINT},
	{"#copyin",	COPYIN},
	{"#uncopyin",	UNCOPIN},
	{"#copyout",	COPYOUT},
	{"#uncopyout",	UNCOPOUT},
	{"#pipe",	PIPE},
	{"#unpipe",	UNPIPE},
	{"#succeed",	SUCCEED},
	{"#fail",	FAIL},
	{"bye",		BYE},
	{"chdir",	CHDIR},
	{"cd",		CHDIR},
	{"learn",	LEARN},
	{"#log",	LOG},
	{"yes",		YES},
	{"no",		NO},
	{"again",	AGAIN},
	{"#mv",		MV},
	{"#user",	USER},
	{"#next",	NEXT},
	{"skip",	SKIP},
	{"where",	WHERE},
	{"#match",	MATCH},
	{"#bad",	BAD},
	{"#create",	CREATE},
	{"#cmp",	CMP},
	{"hint",	HINT},
	{"#once",	ONCE},
	{"#",		NOP},
	{NULL,		0}
};

int *action(s)
char *s;
{
	struct keys *kp;
	if (!keybuff_loaded) /* load array with national language? */
		load_keybuff();
	for (kp=keybuff; *(kp->k_wd); kp++)
		if (STRCMP(kp->k_wd, s) == SAME)
			return(&(kp->k_val));
	return(NULL);
}

# define NW 100
# define NWCH 800
struct whichdid {
	char *w_less;
	int w_seq;
} which[NW];
int nwh = 0;
char whbuff[NWCH];
char *whcp = whbuff;
static struct whichdid *pw;

setdid(lesson, sequence)
char *lesson;
int sequence;
{
	if (already(lesson)) {
		pw->w_seq = sequence;
		return;
	}
	pw = which+nwh++;
	if (nwh >= NW) {
		fprintf(stderr, MSGSTR(LTOOMNYLESS, "Setdid:  too many lessons\n")); /*MSG*/
		tellwhich();
		wrapup(1);
	}
	pw->w_seq = sequence;
	pw->w_less = whcp;
	while (*whcp++ = *lesson++);
	if (whcp >= whbuff + NWCH) {
		fprintf(stderr, MSGSTR(LLESSNMTOOLNG, "Setdid:  lesson names too long\n")); /*MSG*/
		tellwhich();
		wrapup(1);
	}
}

unsetdid(lesson)
char *lesson;
{
	if (!already(lesson))
		return;
	nwh = pw - which;	/* pretend the rest have not been done */
	whcp = pw->w_less;
}

already(lesson)
char *lesson;
{
	for (pw=which; pw < which+nwh; pw++)
		if (STRCMP(pw->w_less, lesson) == SAME)
			return(1);
	return(0);
}

tellwhich()
{
	for (pw=which; pw < which+nwh; pw++)
		PRINTF(MSGSTR(LLESSSEQ, "%3d lesson %7s sequence %3d\n"), /*MSG*/
			pw-which, pw->w_less, pw->w_seq);
}

load_keybuff() 
{
	load_array(MSGSTR(LREADY, "ready")); /*MSG*/
	load_array(MSGSTR(LANSWER, "answer")); /*MSG*/
	load_array(MSGSTR(LPPRINT, "#print")); /*MSG*/
	load_array(MSGSTR(LPCOPYIN, "#copyin")); /*MSG*/
	load_array(MSGSTR(LPUNCOPYIN, "#uncopyin")); /*MSG*/
	load_array(MSGSTR(LPCOPYOUT, "#copyout")); /*MSG*/
	load_array(MSGSTR(LPUNCOPYOUT, "#uncopyout")); /*MSG*/
	load_array(MSGSTR(LPPIPE, "#pipe")); /*MSG*/
	load_array(MSGSTR(LPPUNPIPE, "#unpipe")); /*MSG*/
	load_array(MSGSTR(LPPSUCCEED, "#succeed")); /*MSG*/
	load_array(MSGSTR(LPFAIL, "#fail")); /*MSG*/
	load_array(MSGSTR(LBYE, "bye")); /*MSG*/
	load_array(MSGSTR(LCHDIR, "chdir")); /*MSG*/
	load_array(MSGSTR(LCD, "cd")); /*MSG*/
	load_array(MSGSTR(LLEARN, "learn")); /*MSG*/
	load_array(MSGSTR(LPLOG, "#log")); /*MSG*/
	load_array(MSGSTR(LYES, "yes")); /*MSG*/
	load_array(MSGSTR(LNO, "no")); /*MSG*/
	load_array(MSGSTR(LAGAIN, "again")); /*MSG*/
	load_array(MSGSTR(LPMV, "#mv")); /*MSG*/
	load_array(MSGSTR(LPUSER, "#user")); /*MSG*/
	load_array(MSGSTR(LPNEXT, "#next")); /*MSG*/
	load_array(MSGSTR(LSKIP, "skip")); /*MSG*/
	load_array(MSGSTR(LWHERE, "where")); /*MSG*/
	load_array(MSGSTR(LPMATCH, "#match")); /*MSG*/
	load_array(MSGSTR(LPBAD, "#bad")); /*MSG*/
	load_array(MSGSTR(LPCREATE, "#create")); /*MSG*/
	load_array(MSGSTR(LPCMP, "#cmp")); /*MSG*/
	load_array(MSGSTR(LHINT, "hint")); /*MSG*/
	load_array(MSGSTR(LPONCE, "#once")); /*MSG*/
}
