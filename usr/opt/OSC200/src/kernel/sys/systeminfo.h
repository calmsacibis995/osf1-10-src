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
 * @(#)$RCSfile: systeminfo.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/05/27 21:37:25 $
 */
/*
 *  Module Name:
 *	systeminfo.h
 *
 *  Description:
 *	header file for SVR4 sysinfo() system call
 */
#ifndef _SYS_SYSTEMINFO_H
#define _SYS_SYSTEMINFO_H

/*
 * Commands to sysinfo()
 */

#define SI_SYSNAME		1	/* return name of operating system */
#define SI_HOSTNAME		2	/* return name of node */
#define SI_RELEASE 		3	/* return release of operating system */
#define SI_VERSION		4	/* return version field of utsname */
#define SI_MACHINE		5	/* return kind of machine */
#define SI_ARCHITECTURE		6	/* return instruction set arch */
#define SI_HW_SERIAL		7	/* return hardware serial number */
#define SI_HW_PROVIDER		8	/* return hardware manufacturer */
#define SI_SRPC_DOMAIN		9	/* return secure RPC domain */
/*
 * These commands are unpublished interfaces to sysinfo().
 */
#define SI_SET_HOSTNAME		258	/* set name of node */
					/*  -unpublished option */
#define SI_SET_SRPC_DOMAIN	265	/* set secure RPC domain */
					/* -unpublished option */
#define SI_SET_SYSNAME		259	/* set name of system */
					/*  -unpublished option */

#ifdef __alpha
/*
 * Definitions for the system V sysinfo system call: sysv_sysinfo()
 * These used to be in utsname.h but that file seems to be gone in 
 * OSF/1 for the alpha.
 */

#define ARCHITECTURE		"alpha"	/* architecture type */
#define HW_PROVIDER_NAME	"Digital"
#endif /* __alpha */

#if defined(__STDC__) && !defined(_KERNEL)
int sysinfo(int, char *, long);
#endif

#endif	/* _SYS_SYSTEMINFO_H */
