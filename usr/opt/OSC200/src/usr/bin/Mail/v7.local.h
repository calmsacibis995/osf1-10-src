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
/*
static char rcsid[] = "@(#)$RCSfile: v7.local.h,v $ $Revision: 4.2.8.3 $ (DEC) $Date: 1993/09/07 18:16:24 $";
 */
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0.1
 */
/* 
 * COMPONENT_NAME: CMDMAILX v7.local.h
 * 
 * FUNCTIONS: 
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
 *      v7.local.h  5.2 (Berkeley) 9/19/85
 */

/*
 * Declarations and constants specific to an installation.
 *
 * Vax/Unix version 7.
 */

#define	GETHOST				/* System has gethostname syscall */
#ifdef	GETHOST
#define	LOCAL		EMPTYID		/* Dynamically determined local host */
#else
#define	LOCAL		'Self'		/* Local host id */
#endif /*GETHOST*/

#define MYDOMAIN        ".uucp"         /* Appended to local host name -SVID-2*/

#define MAILBOX		"/usr/spool/mail"	/* system mailbox dir */
#define	MAIL		"/usr/sbin/sendmail"	/* Name of mail sender */
#define SENDMAIL	"/usr/sbin/sendmail"
					/* Name of classy mail deliverer */
#ifdef ASIAN_I18N
#define I18NPATH	"/usr/i18n"	/* I18N path */
#define MCODESET	"/usr/lib/mail-codesets"
#define MB_MAX		4	/* maximum number of byte per char */
#endif  /* ASIAN_I18N */
#define	EDITOR		"/usr/bin/ex"	/* Name of text editor */
#define	VISUAL		"/usr/bin/vi"	/* Name of display editor */
#define	MORE		"/usr/bin/more"	/* Standard output pager */
#define	LS		(value("LISTER") ? value("LISTER") : "ls")
					/* Name of directory listing prog*/
#define	SHELL		"/bin/sh"	/* Standard shell */
#define	POSTAGE		"/var/adm/maillog"
					/* Where to audit mail sending */
#define	UIDMASK		0177777		/* Significant uid bits */
#define	MASTER		"/usr/share/lib/Mail.rc"
#define	APPEND				/* New mail goes to end of mailbox */
#define CANLOCK				/* Locking protocol actually works */
#define	UTIME				/* System implements utime(2) */

#ifdef ASIAN_I18N
#define ISSPACE(wc, cp, mb) ((mb=mbtowc(&wc, cp, MB_MAX))!=-1 && iswspace(wc))
#define NOCHANGE 0
#define SET_CODESET 1
#define SET_EXCODE 2
#endif
