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
 * @(#)$RCSfile: secdefines.h,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/04/05 19:46:32 $
 */
#ifndef __SECDEFINES__
#define __SECDEFINES__

/*
 * @OSF_COPYRIGHT@
 */
/*
 * Copyright (c) 1990 SecureWare, Inc.  All Rights Reserved.
 *
 * @(#)secdefines.h	2.1 16:06:16 4/20/90 SecureWare
 *
 * This header file configures the conditional compilation symbols
 * used to control the inclusion of security features.
 */
/*
 * HISTORY
 * $OSF_Log:	secdefines.h,v $
 * Revision 1.1.1.2  92/03/31  01:45:00  devrcs
 *  *** OSF1_1B26 version ***
 * 
 * Revision 1.3.2.2  1992/03/02  21:59:25  hosking
 * 	bug 4436: partially get rid of #includes of SEC_CMW, SEC_NCAV, SEC_SHW
 * 		to help speed up kernel builds
 * 	[1992/03/02  21:58:26  hosking]
 *
 * Revision 1.3  1990/10/07  14:53:24  devrcs
 * 	Fixed up EndLog Marker.
 * 	[90/09/30  16:08:55  gm]
 * 
 * 	Added EndLog Marker.
 * 	[90/09/28  11:44:01  gm]
 * 
 * Revision 1.2  90/06/22  20:53:39  devrcs
 * 	Added from SecureWare.
 * 	[90/06/09  23:58:46  seiden]
 * 
 * $OSF_EndLog$
 */

#include <standards.h>

#if defined(KERNEL) && defined(_OSF_SOURCE)
#include <sec_base.h>
#include <sec_priv.h>
#include <sec_mac_ob.h>
#include <sec_cmw.h>
#include <sec_shw.h>
#include <sec_acl_posix.h>
#include <sec_ncav.h>
#endif /* KERNEL && _OSF_SOURCE */

/*
 * This enum & set of defines is for SEC_SWITCH_CONF.  Keep it in sync
 * with the resolution of definitions which follow.
 */

enum SEC_CONF_bits {
	SEC_CONF_base = 0,	/* SEC_BASE */
	SEC_CONF_priv,		/* SEC_PRIV */
	SEC_CONF_mac_ob,	/* SEC_MAC_OB */
	SEC_CONF_cmw,		/* SEC_CMW */
	SEC_CONF_shw,		/* SEC_SHW */
	SEC_CONF_acl_sware,	/* SEC_ACL_SWARE */
	SEC_CONF_acl_posix,	/* SEC_ACL_POSIX */
	SEC_CONF_ncav,		/* SEC_ACL_NCAV */
	/* new ones go here */
	SEC_CONF_groups		/* SEC_GROUPS, dummy entry */
};

#define	SEC_CONF_BASE		(1ul<<((int)SEC_CONF_base))
#define	SEC_CONF_PRIV		(1ul<<((int)SEC_CONF_priv))
#define	SEC_CONF_MAC_OB		(1ul<<((int)SEC_CONF_mac_ob))
#define	SEC_CONF_CMW		(1ul<<((int)SEC_CONF_cmw))
#define	SEC_CONF_SHW		(1ul<<((int)SEC_CONF_shw))
#define	SEC_CONF_ACL_SWARE	(1ul<<((int)SEC_CONF_acl_sware))
#define	SEC_CONF_ACL_POSIX	(1ul<<((int)SEC_CONF_acl_posix))
#define	SEC_CONF_NCAV		(1ul<<((int)SEC_CONF_ncav))
/* #define	SEC_CONF_GROUPS		(1ul<<((int)SEC_CONF_groups)) */

/* defines for the composite features */

#define	SEC_CONF_ILB		(SEC_CONF_CMW | SEC_CONF_SHW)
#define	SEC_CONF_ENCODINGS	SEC_CONF_ILB
#define	SEC_CONF_MAC		(SEC_CONF_ILB | SEC_CONF_MAC_OB)
#define	SEC_CONF_ACL		(SEC_CONF_ACL_SWARE | SEC_CONF_ACL_POSIX)
#define	SEC_CONF_ARCH		(SEC_CONF_ACL | SEC_CONF_MAC | SEC_CONF_NCAV)
#define	SEC_CONF_FSCHANGE	(SEC_CONF_ARCH | SEC_CONF_PRIV)

#if SEC_BASE

/*
 * Resolve mutually exclusive base conditionals
 */

#if SEC_CMW
#undef SEC_MAC_OB
#undef SEC_SHW
#endif

#if SEC_SHW
#undef SEC_MAC_OB
#endif

#if SEC_ACL_POSIX
#undef SEC_ACL_SWARE
#endif

#if defined(_OSF_SOURCE) || defined(AUX)

#define	SEC_AUDIT_SYMLINKS	1	/* system supports symbolic links */
#define	SEC_PTY			1
#define	SEC_SOCKET		1

#include <sys/param.h>
#if NGROUPS
#define	SEC_GROUPS	NGROUPS
#else
#undef	SEC_GROUPS
#endif

#endif /* _OSF_SOURCE || AUX */

#ifdef SYSV_3

#undef	SEC_AUDIT_SYMLINKS
#undef	SEC_PTY
#undef	SEC_SOCKET
#undef	SEC_GROUPS

#endif /* SYSV_3 */


/*
 * The following definitions derive various conditional compilation
 * symbols from the base conditionals that are specified on the
 * compiler command line.
 */

#if SEC_MAC_OB || SEC_SHW || SEC_CMW
#define	SEC_MAC	1
#else
#undef	SEC_MAC
#endif

#if SEC_ACL_SWARE || SEC_ACL_POSIX
#define	SEC_ACL	1
#else
#undef	SEC_ACL
#endif

#if SEC_SHW || SEC_CMW
#define	SEC_ILB	1
#define	SEC_ENCODINGS	1
#else
#undef	SEC_ILB
#undef	SEC_ENCODINGS
#endif

#if SEC_ACL || SEC_MAC || SEC_NCAV
#define	SEC_ARCH	1
#else
#undef	SEC_ARCH
#endif

#if SEC_ARCH || SEC_PRIV
#define	SEC_FSCHANGE	1
#else
#undef	SEC_FSCHANGE
#endif

#else /* !SEC_BASE */

#if defined(_OSF_SOURCE) || defined(AUX)

#include <sys/param.h>
#if NGROUPS
#define	SEC_GROUPS	NGROUPS
#else
#undef	SEC_GROUPS
#endif

#endif /* _OSF_SOURCE || AUX */

#undef	SEC_BASE
#undef	SEC_PRIV
#undef	SEC_MAC_OB
#undef	SEC_SHW
#undef	SEC_CMW
#undef	SEC_ACL_SWARE
#undef	SEC_ACL_POSIX
#undef	SEC_NCAV
#undef	SEC_ACL
#undef	SEC_MAC
#undef	SEC_ILB
#undef	SEC_ENCODINGS
#undef	SEC_ARCH
#undef	SEC_FSCHANGE
#undef	SEC_PTY
#undef	SEC_SOCKET
#undef	SEC_AUDIT_SYMLINKS

#endif /* !SEC_BASE */
#endif /* __SECDEFINES__ */
