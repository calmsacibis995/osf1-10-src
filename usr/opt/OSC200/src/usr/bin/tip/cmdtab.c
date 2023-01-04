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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: cmdtab.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/09/07 15:36:08 $";
#endif
/*
cmdtab.c	1.3  com/cmd/tip,3.1,9013 10/15/89 10:41:49";
 */
/* 
 * COMPONENT_NAME: UUCP cmdtab.c
 * 
 * FUNCTIONS: MSGSTR 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* static char sccsid[] = "cmdtab.c	5.3 (Berkeley) 5/5/86"; */

#include "tip.h"

extern	int shell(), getfl(), sendfile(), chdirectory();
extern	int finish(), help(), pipefile(), pipeout(), consh(), variable();
extern	int cu_take(), cu_put(), dollar(), genbrk(), suspend();

esctable_t etable[] = {
	{ '!',	NORM,		NULL, shell }, 
	{ '<',	NORM,		NULL, getfl }, 
	{ '>',	NORM,		NULL, sendfile }, 
	{ 't',	NORM,		NULL, cu_take }, 
	{ 'p',	NORM,		NULL, cu_put }, 
	{ '|',	NORM,		NULL, pipefile }, 
	{ '$',	NORM,		NULL, pipeout }, 
	{ 'C',  NORM,		NULL, consh }, 
	{ 'c',	NORM,		NULL, chdirectory }, 
	{ '.',	NORM,		NULL, finish }, 
	{CTRL('d'),NORM,	NULL, finish }, 
	{CTRL('y'),NORM,	NULL, suspend }, 
	{CTRL('z'),NORM,	NULL, suspend }, 
	{ 's',	NORM,		NULL, variable }, 
	{ '?',	NORM,		NULL, help }, 
	{ '#',	NORM,		NULL, genbrk }, 
	{ 0, 0, 0 }
};

char *etable_default[] = {
	"shell",
	"receive file from remote host",
	"send file to remote host",
	"take file from remote UNIX",
	"put file to remote UNIX",
	"pipe remote file",
	"pipe local command to remote host",
	"connect program to remote host",
	"change directory",
	"exit from tip",
	"exit from tip",
	"suspend tip (local+remote)",
	"suspend tip (local only)",
	"set variable",
	"get this summary",
	"send break",
	NULL
};
