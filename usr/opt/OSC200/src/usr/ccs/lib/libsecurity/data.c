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
 *	@(#)$RCSfile: data.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/05/26 15:41:55 $
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
/*
 * Copyright (c) 1988-1990 SecureWare, Inc.  All Rights Reserved.
 */


/*
 * Based on:

 */

/*LINTLIBRARY*/


/*
 * This file contains initialized global data for use in libprot.a .
 */

#include <sys/secdefines.h>

/* #if SEC_BASE */ /*{*/

#include <sys/types.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>


struct namepair secclass[] = {
	"d",			AUTH_D,
	"c1",			AUTH_C1,
	"c2",			AUTH_C2,
	"b1",			AUTH_B1,
	"b2",			AUTH_B2,
	"b3",			AUTH_B3,
	"a1",			AUTH_A1,
	(caddr_t) 0,		0
};



struct namepair sys_priv[] = {
	"suspendaudit",		SEC_SUSPEND_AUDIT,
	"configaudit",		SEC_CONFIG_AUDIT,
	"writeaudit",		SEC_WRITE_AUDIT,
	"execsuid",		SEC_EXECSUID,
	"chmodsugid",		SEC_CHMODSUGID,
	"chown",		SEC_CHOWN,
	"acct",			SEC_ACCT,
	"limit",		SEC_LIMIT,
	"lock",			SEC_LOCK,
	"linkdir",		SEC_LINKDIR,
	"mknod",		SEC_MKNOD,
	"mount",		SEC_MOUNT,
	"sysattr",		SEC_SYSATTR,
	"setprocident",		SEC_SETPROCIDENT,
	"chroot",		SEC_CHROOT,
	"debug",		SEC_DEBUG,
	"shutdown",		SEC_SHUTDOWN,
	"filesys",		SEC_FILESYS,
	"remote",		SEC_REMOTE,
	"kill",			SEC_KILL,
	"owner",		SEC_OWNER,
	"chpriv",		SEC_CHPRIV,
	"allowdacaccess",	SEC_ALLOWDACACCESS,
	"sucompat",		SEC_SUCOMPAT,
	"supropagate",		SEC_SUPROPAGATE,
#if SEC_MAC
	"downgrade",		SEC_DOWNGRADE,
	"writeupclearance",	SEC_WRITEUPCLEARANCE,
	"writeupsyshi",		SEC_WRITEUPSYSHI,
	"multileveldir",	SEC_MULTILEVELDIR,
	"allowmacaccess",	SEC_ALLOWMACACCESS,
#endif
	(caddr_t) 0,		0
};

struct namepair *cmd_priv = (struct namepair *) 0;

struct namepair auth_dev_type[] = {
{ "printer", AUTH_DEV_PRINTER },
{ "terminal", AUTH_DEV_TERMINAL },
{ "tape", AUTH_DEV_TAPE },
{ "remote", AUTH_DEV_REMOTE },
{ "xdisplay", AUTH_DEV_XDISPLAY },
{ (caddr_t) 0, 0 }
};


#if SEC_ARCH /*{*/

struct namepair auth_dev_assign[] = {
#if SEC_MAC
 { "single",  AUTH_DEV_SINGLE },
 { "multi",   AUTH_DEV_MULTI },
#endif
 { "label",   AUTH_DEV_LABEL },
 { "nolabel", AUTH_DEV_NOLABEL },
 { "import",  AUTH_DEV_IMPORT },
 { "export",  AUTH_DEV_EXPORT },
#if SEC_ILB
 { "singleil",  AUTH_DEV_ILSINGLE },
 { "multiil",   AUTH_DEV_ILMULTI },
#endif
 { (caddr_t) 0, 0 }
};

#endif /* } SEC_ARCH */

/* #endif */ /* } SEC_BASE */
