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
static char rcsid[] = "@(#)$RCSfile: talk.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/11 19:12:25 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/* 
 * COMPONENT_NAME: TCPIP talk.c
 * 
 * FUNCTIONS: MSGSTR, Mtalk 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
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
 */
/* talk.c	1.5  com/sockcmd/talk,3.1,9021 4/4/90 11:26:06 */
/*
char copyright[] =
" Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
 "talk.c	5.4 (Berkeley) 6/29/88";
*/

#include "talk.h"
#include <locale.h>
#include "talk_msg.h" 

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TALK,n,s) 

/*
 * talk:	A visual form of write. Using sockets, a two way 
 *		connection is set up between the two people talking. 
 *		With the aid of curses, the screen is split into two 
 *		windows, and each users text is added to the window,
 *		one character at a time...
 *
 *		Written by Kipp Hickman
 *		
 *		Modified to run under 4.1a by Clem Cole and Peter Moore
 *		Modified to run between hosts by Peter Moore, 8/19/82
 *		Modified to run under 4.1c by Peter Moore 3/17/83
 */

main(argc, argv)
	int argc;
	char *argv[];
{

	setlocale(LC_ALL,"");
	catd = catopen(MF_TALK,NL_CAT_LOCALE);
	get_names(argc, argv);
	init_display();
	open_ctl();
	open_sockt();
	start_msgs();
	if (!check_local() )
		invite_remote();
	end_msgs();
	set_edit_chars();
	talk();
}
