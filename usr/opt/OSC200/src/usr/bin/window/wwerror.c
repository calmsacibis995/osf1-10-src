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
static char	*sccsid = "@(#)$RCSfile: wwerror.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:13:35 $";
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
 * 	wwerror.c	3.5 (Berkeley) 6/29/88
 */


#include "defs.h"

char *
wwerror()
{
	extern errno;
	extern char *sys_errlist[];

	switch (wwerrno) {
	case WWE_NOERR:
		return (MSGSTR(NOERROR, "No error"));
	case WWE_SYS:
		return sys_errlist[errno];
	case WWE_NOMEM:
		return (MSGSTR(OUTOFMEM, "Out of memory"));
	case WWE_TOOMANY:
		return (MSGSTR(TOOMANYWIN, "Too many windows"));
	case WWE_NOPTY:
		return (MSGSTR(NOPTTYS, "Out of pseudo-terminals"));
	case WWE_SIZE:
		return (MSGSTR(BADSIZE, "Bad window size"));
	case WWE_BADTERM:
		return (MSGSTR(BADTYPE, "Unknown terminal type"));
	case WWE_CANTDO:
		return (MSGSTR(NODICE, "Can't run window on this terminal"));
	default:
		return (MSGSTR(UNKNOWNE, "Unknown error"));
	}
}
