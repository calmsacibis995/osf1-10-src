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
/*	
 *	@(#)$RCSfile: valid.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:02:48 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#ifdef SEC_BASE
#ifndef __VALID__
#define __VALID__

/* Copyright (c) 1988, 1990 SecureWare, Inc.
 *   All rights reserved
 */



/*  User account validation utilities
 */

#include	"gl_defs.h"

int invaliduser();
FILE *lockpwfile();
void badgroup(), baduser(), noshell(), userexists();

void InvUserMsg(), UIDExistsMsg(), UserExistsMsg();

/* file names for rewriting control files (not defined anywhere!) */

GLOBAL char opasswd[]		INIT1	("/etc/opasswd");
GLOBAL char opasswd_pag[]	INIT1	("/etc/opasswd.pag");
GLOBAL char opasswd_dir[]	INIT1	("/etc/opasswd.dir");

/*
 * the following files are the same as used by vipw and mkpasswd
 */
GLOBAL char temp[]		INIT1	("/etc/ptmp");
GLOBAL char temp_pag[]		INIT1	("/etc/ptmp.pag");
GLOBAL char temp_dir[]		INIT1	("/etc/ptmp.dir");
GLOBAL char passwd[]		INIT1	("/etc/passwd");
GLOBAL char passwd_pag[]	INIT1	("/etc/passwd.pag");
GLOBAL char passwd_dir[]	INIT1	("/etc/passwd.dir");

#define	PASSWD_LEN	sizeof("/etc/passwd")

#define MAXGID		99999

GLOBAL char groupfn[]		INIT1	("/etc/group");
GLOBAL char groupnewfn[]	INIT1	("/etc/group-t");

#define	GROUP_LEN	sizeof("/etc/group")

/* file names for skeleton startup files for new user accounts. */

GLOBAL char skel_dir[]		INIT1	("/tcb/files/skel/");

/* programs that need to be exec'ed from this file */

GLOBAL char copy_program[]	INIT1	("/bin/cp");

/* modes on files that this program creates. */

#define HOMEDIRMODE	0750

#endif /* __VALID __ */
#endif /* SEC_BASE */
