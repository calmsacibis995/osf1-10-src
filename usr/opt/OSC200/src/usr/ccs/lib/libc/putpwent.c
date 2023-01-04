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
static char	*sccsid = "@(#)$RCSfile: putpwent.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/03/26 12:07:12 $";
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
 * COMPONENT_NAME: (LIBCADM) Standard C Library System Admin Functions 
 *
 * FUNCTIONS: putpwent 
 *
 * ORIGINS: 3, 27 
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
 * putpwent.c	1.5  com/lib/c/adm,3.1,8943 9/13/89 10:49:03
 */

/*LINTLIBRARY*/
#include <stdio.h>
#include <pwd.h>

/*                                                                    
 * FUNCTION: Update a user description in the files /etc/passwd
 *		and /etc/security/passwd.
 *
 * PARAMETERS: pw - struct passwd *pw;  the description of the users
 *					password entry.
 *
 * RETURN VALUE DESCRIPTIONS: Upon successful conpletion, PUTPWENT returns
 *				a value of 0. If PUTPWENT fails, a nonzero
 *				value is returned.
 *
 */
/*
 *
 * format a password file entry
 */

int
putpwent(p, f)
register struct passwd *p;
register FILE *f;
{
	(void) fprintf(f, "%s:%s", p->pw_name, p->pw_passwd);


	(void) fprintf(f, ":%u:%u:%u:%s:%s:%s:%s",
		p->pw_uid,
		p->pw_gid,
                p->pw_quota,
                p->pw_comment,
		p->pw_gecos,
		p->pw_dir,
		p->pw_shell);
	(void) putc('\n', f);
	return(ferror(f));
}
