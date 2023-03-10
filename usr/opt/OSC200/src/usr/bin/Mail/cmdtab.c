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
static char	*sccsid = "@(#)$RCSfile: cmdtab.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/09/29 13:26:53 $";
#endif 
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/* 
 * COMPONENT_NAME: CMDMAILX cmdtab.c
 * 
 * FUNCTIONS: MSGSTR 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	cmdtab.c     5.3 (Berkeley) 9/15/85
 *
 * Modification History
 *
 * 01 gaudet Tue Nov 05 19:07:29 EST 1991
 *	Changed the local subroutine pipe() to mpipe() because it
 *	interfered with pipe(2). Caused shell variable evaluation to
 *	fail and Mail/mailx to exit. Fixes OSF_QAR #01665.
 */

#include "def.h"

#include "Mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

/*
 * Mail -- a mail program
 *
 * Define all of the command names and bindings.
 */

extern int type(), preserve(), delete(), undelete(), next(), shell(), schdir();
extern int save(), help(), headers(), pdot(), strace(), respond(), editor();
extern int edstop(), rexit(), pcmdlist(), sendmail(), from(), copycmd();
extern int messize(), psalloc(), deltype(), unset(), set(), source();
extern int pversion(), group(), top(), core(), null(), stouch(), visual();
extern int swrite(), dosh(), file(), echo(), Respond(), scroll(), ifcmd();
extern int elsecmd(), endifcmd(), mboxit(), clobber(), alternates(), unalias();
extern int local(), folders(), igfield(), Type(), retfield(), more(), More();
extern int unread();	/* , Header(); */
extern int Save(), Copy();
extern int followup(), Followup();
extern int dopipe();

struct cmd cmdtab[] = {
    {	"next",		next,		NDMLIST,	0,	MMNDEL },
    {	"alias",	group,		M|RAWLIST,	0,	1000 },
    {	"print",	type,		MSGLIST,	0,	MMNDEL },
    {	"type",		type,		MSGLIST,	0,	MMNDEL },
    {	"Type",		Type,		MSGLIST,	0,	MMNDEL },
    {	"Print",	Type,		MSGLIST,	0,	MMNDEL },
    {	"visual",	visual,		I|MSGLIST,	0,	MMNORM },
    {	"top",		top,		MSGLIST,	0,	MMNDEL },
    {	"touch",	stouch,		W|MSGLIST,	0,	MMNDEL },
    {	"preserve",	preserve,	W|MSGLIST,	0,	MMNDEL },
    {	"delete",	delete,		W|P|MSGLIST,	0,	MMNDEL },
    {	"dp",		deltype,	W|MSGLIST,	0,	MMNDEL },
    {	"dt",		deltype,	W|MSGLIST,	0,	MMNDEL },
    {	"undelete",	undelete,	P|MSGLIST,	MDELETED,MMNDEL },
    {	"unset",	unset,		M|RAWLIST,	1,	1000 },
    {	"mail",		sendmail,	R|M|I|STRLIST,	0,	0 },
    {	"mbox",		mboxit,		W|MSGLIST,	0,	0 },
    {	"more",		more,		MSGLIST,	0,	MMNDEL },
    {	"page",		more,		MSGLIST,	0,	MMNDEL },
    {	"More",		More,		MSGLIST,	0,	MMNDEL },
    {	"Page",		More,		MSGLIST,	0,	MMNDEL },
    {	"unread",	unread,		MSGLIST,	0,	MMNDEL },
    {	"Unread",	unread,		MSGLIST,	0,	MMNDEL },
    {	"new",		unread,		MSGLIST,	0,	MMNDEL },
    {	"New",		unread,		MSGLIST,	0,	MMNDEL },
    {	"!",		shell,		I|STRLIST,	0,	0 },
    {	"copy",		copycmd,	M|STRLIST,	0,	0 },
    {   "Copy",         Copy,           M|MSGLIST,      0,      0 }, /*SVID-2 */
    {	"chdir",	schdir,		M|STRLIST,	0,	0 },
    {	"cd",		schdir,		M|STRLIST,	0,	0 },
    {	"save",		save,		STRLIST,	0,	0 },
    {   "Save",         Save,           MSGLIST,        0,      0 }, /*SVID-2 */
    {	"source",	source,		M|STRLIST,	0,	0 },
    {	"set",		set,		M|RAWLIST,	0,	1000 },
    {	"shell",	dosh,		I|NOLIST,	0,	0 },
    {	"version",	pversion,	M|NOLIST,	0,	0 },
    {	"group",	group,		M|RAWLIST,	0,	1000 },
    {	"write",	swrite,		STRLIST,	0,	0 },
    {	"from",		from,		MSGLIST,	0,	MMNORM },
    {   "followup",     followup,       R|I|MSGLIST,    0,      MMNDEL},/*SVID*/
    {   "Followup",     Followup,       R|I|MSGLIST,    0,      MMNDEL},/*SCID*/
    {	"file",		file,		T|M|RAWLIST,	0,	1 },
    {	"folder",	file,		T|M|RAWLIST,	0,	1 },
    {	"folders",	folders,	T|M|RAWLIST,	0,	1 },
    {	"?",		help,		M|NOLIST,	0,	0 },
    {	"z",		scroll,		M|STRLIST,	0,	0 },
    {	"headers",	headers,	MSGLIST,	0,	MMNDEL },
    {	"help",		help,		M|NOLIST,	0,	0 },
    {	"=",		pdot,		NOLIST,		0,	0 },
    {	"Reply",	Respond,	R|I|MSGLIST,	0,	MMNDEL },
    {	"Respond",	Respond,	R|I|MSGLIST,	0,	MMNDEL },
    {	"reply",	respond,	R|I|MSGLIST,	0,	MMNDEL },
    {	"respond",	respond,	R|I|MSGLIST,	0,	MMNDEL },
    {	"edit",		editor,		I|MSGLIST,	0,	MMNORM },
    {	"echo",		echo,		M|RAWLIST,	0,	1000 },
    {	"quit",		edstop,		NOLIST, 	0,	0 },
    {	"list",		pcmdlist,	M|NOLIST,	0,	0 },
    {	"local",	local,		M|RAWLIST,	0,	1000 },
    {	"xit",		rexit,		M|NOLIST,	0,	0 },
    {	"exit",		rexit,		M|NOLIST,	0,	0 },
    {	"size",		messize,	MSGLIST,	0,	MMNDEL },
    {	"hold",		preserve,	W|MSGLIST,	0,	MMNDEL },
    {	"if",		ifcmd,		F|M|RAWLIST,	1,	1 },
    {	"else",		elsecmd,	F|M|RAWLIST,	0,	0 },
    {	"endif",	endifcmd,	F|M|RAWLIST,	0,	0 },
    {	"alternates",	alternates,	M|RAWLIST,	0,	1000 },
    {	"ignore",	igfield,	M|RAWLIST,	0,	1000 },
    {	"discard",	igfield,	M|RAWLIST,	0,	1000 },
    {	"retain",	retfield,	M|RAWLIST,	0,	1000 },
    {	"unalias",	unalias,	M|RAWLIST,	0,	1000 },
/*    {	"Header",	Header,		STRLIST,	0,	1000 },	*/
/*    {	"core",		core,		M|NOLIST,	0,	0 },      */
    {	"#",		null,		M|NOLIST,	0,	0 },
    {   "pipe",         dopipe,         STRLIST,        0,      0 }, /*SVID-2 */
    {   "|",            dopipe,         STRLIST,        0,      0 }, /*SVID-2 */


/*    {	"clobber",	clobber,	M|RAWLIST,	0,	1 },      */
    {	0,		0,		0,		0,	0 }
};
