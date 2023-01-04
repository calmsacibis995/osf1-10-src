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
/*#ifndef lint
*static char *rcsid = "@(#)$RCSfile: pathnames.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/08/04 21:19:05 $";
*#endif
*/

#include <paths.h>

#ifndef _PATH_DEFPATH
#define	_PATH_DEFPATH	"/usr/bin:"
#endif
#define	_PATH_HUSHLOGIN	".hushlogin"
#define	_PATH_LASTLOG	"/var/adm/lastlog"
#define	_PATH_LASTTLC	"/var/adm/lasttlc"
#define	_PATH_MAILDIR	"/var/spool/mail"
#define	_PATH_MOTDFILE	"/etc/motd"
#define	_PATH_NOLOGIN	"/etc/nologin"
