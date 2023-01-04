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
static char *rcsid = "@(#)$RCSfile: Errors.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:10:50 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	Errors.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:52:13  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  17:50:15  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:19:07  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:51:29  marquard]
 * 
 * $OSF_EndLog$
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (c) 1990, SecureWare, Inc.
 *   All rights reserved
 * 
 * Based on OSF Version:
 *	"@(#)Errors.c	1.5 16:30:35 5/17/91 SecureWare"
 */

/* #ident "@(#)Errors.c	1.1 11:15:19 11/8/91 SecureWare" */

/*
 * Support routines to display fatal and non-fatal error messages
 */

void 
ErrorMessageOpen(tag, class, i_index, CharPtr)
	int         tag;
	char      **class;
	int         i_index;
	char       *CharPtr; /* client_data */
{
	DispMessage(class, i_index, CharPtr);
}


void 
SystemErrorMessageOpen(tag, class, i_index, CharPtr)
	int         tag;
	char        **class;
	int         i_index;
	char        *CharPtr;
{
	ErrorMessageOpen(tag, class, i_index, CharPtr);
	exit(1);
}
