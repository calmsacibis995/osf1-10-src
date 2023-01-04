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
static char	*sccsid = "@(#)$RCSfile: xmsg.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/09/23 18:29:15 $";
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

/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: xmsg
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */


/*
 * FUNCTION: Print an error message based on errno.
 *
 * RETURN VALUE DESCRIPTIONS:
 *		fatal()
 */
/*
	Call fatal with an appropriate error message
	based on errno.  If no good message can be made up, it makes
	up a simple message.
	The second argument is a pointer to the calling functions
	name (a string); it's used in the manufactured message.
*/


#ifdef MSG
#include "pw_msg.h"
nl_catd	catd;
#define MSGSTR(Num, Str) catgets(catd, MS_PW, Num, Str)
#else
#define MSGSTR(Num, Str) Str
#endif

# include	"errno.h"
# include	"sys/types.h"
# include	"macros.h"

xmsg(file,func)
char *file, *func;
{
	register char *str;
	char buf[sizeof Error];
	extern int errno, sys_nerr;
	extern char *sys_errlist[];
	extern char *dname();

#ifdef MSG
	catd = catopen(MF_PW, NL_CAT_LOCALE);
#endif
	switch (errno) {
	case ENFILE:
#ifdef MSG
		str = MSGSTR(XTABFULL, "file table full (ut3)");
#else
		str = "file table full (ut3)";
#endif
		break;
	case ENOENT:
#ifdef MSG
		sprintf(str = Error, MSGSTR(XNOFILE, 
			"`%s' does not exist (ut4)"), file);
#else
		sprintf(str = Error,"`%s' does not exist (ut4)", file);
#endif
		break;
	case EACCES:
		copy(file,buf);
#ifdef MSG
		sprintf(str = Error, MSGSTR(XNODWRT,
			"directory `%s' unwritable (ut2)"), dname(buf));
#else
		sprintf(str = Error,"directory `%s' unwritable (ut2)",
			dname(buf));
#endif
		break;
	case ENOSPC:
#ifdef MSG
		str = MSGSTR(XNOSPC, "no space! (ut10)");
#else
		str = "no space! (ut10)";
#endif
		break;
	case EFBIG:
#ifdef MSG
		str = MSGSTR(XTOOBIG, "file too big (ut8)");
#else
		str = "file too big (ut8)";
#endif
		break;
	default:
		if ((unsigned)errno < sys_nerr)
			str = sys_errlist[errno];
		else
#ifdef MSG
			sprintf(str = buf, MSGSTR(XERRNO, "errno = %d"),
				errno);
		sprintf(Error, MSGSTR(XFUNC, "%s, function = `%s' (ut11)"),
			str, func);
#else
			sprintf(str = buf,"errno = %d",errno);
		sprintf(Error,"%s, function = `%s' (ut11)", str, func);
#endif
		str = Error;
		break;
	}
	return(fatal(str));
}
