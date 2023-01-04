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
 *	@(#)$RCSfile: sec_stream.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:59:42 $
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
#ifndef __SEC_STREAM__
#define __SEC_STREAM__

#include <sys/secdefines.h>

#if SEC_BASE

/*
 * Copyright (c) 1990 SecureWare, Inc.  All rights reserved.
 *
 * @(#)sec_stream.h	3.1 08:52:14 2/26/91 SecureWare, Inc.
 *
 * This Module contains Proprietary Information of SecureWare, Inc.
 * and should be treated as Confidential.
 *
 * General Purpose Defines and Macros for Secure Network Modules
 */

/*
 * The following bit definitions are used in the masks for the 
 * tnet_sec_attr, tnet_full_attr and tnet_full_token structures.  The
 * attributes MUST be defined in the same order as the CIPSO_... bit masks
 * in the file sec_tnip.h, but do not need the same bit values.
 */
enum tnet_mask_values {
#if SEC_MAC
	TNETSenLabel,
#endif
#if SEC_NCAV
	TNETNatCaveats,
#endif
#if SEC_ILB
	TNETInfoLabel,
#endif
	TNETPrivileges,
	TNETLuid,
	TNETIDs,
	TNETProcessID
};

#define tnet_mask(value)	(1<<(value))
#define tnet_value(value)	(int)(value)

#if SEC_MAC
#define TNET_SEN_LABEL			tnet_value(TNETSenLabel)
#define TNET_SW_SEN_LABEL		tnet_mask(TNETSenLabel)
#endif

#if SEC_NCAV
#define TNET_NAT_CAVEATS		tnet_value(TNETNatCaveats)
#define TNET_SW_NAT_CAVEATS		tnet_mask(TNETNatCaveats)
#endif

#if SEC_ILB
#define TNET_INFO_LABEL			tnet_value(TNETInfoLabel)
#define TNET_SW_INFO_LABEL		tnet_mask(TNETInfoLabel)
#endif

#define TNET_PRIVILEGES			tnet_value(TNETPrivileges)
#define TNET_SW_PRIVILEGES		tnet_mask(TNETPrivileges)

#define TNET_LUID			tnet_value(TNETLuid)
#define TNET_SW_LUID			tnet_mask(TNETLuid)

#define TNET_IDS			tnet_value(TNETIDs)
#define TNET_SW_IDS			tnet_mask(TNETIDs)

#define TNET_PID			tnet_value(TNETProcessID)
#define TNET_SW_PID			tnet_mask(TNETProcessID)

#define TNET_UID			(TNET_PID+1)
#define TNET_GID			(TNET_UID+1)
#define TNET_GROUPS			(TNET_GID+1)
#define TNET_MAX_ATTR_ID		(TNET_GROUPS+1)

/*
 * The tnet_sec_attr structure is used for passing security attributes
 * between the kernel and user space.
 *
 * The mask field specifies which attributes are present.  The attributes
 * to be passed immediately follow the structure in order of their bit
 * positions in the mask, starting with the least significant bit.
 * Attributes that are not present do not occupy any space.  An attribute
 * whose size is not known to the kernel (an internal representation of a
 * security policy label, for example) begins with a u_long word specifying
 * the length in bytes (excluding the length word) of the attribute.  
 * On systems that support supplementary groups, the attribute that contains
 * effective user and group IDs also contains a u_long word specifying the
 * number of supplementary groups, followed by the groups themselves, stored
 * as type gid_t.  Each attribute begins on a longword boundary; attributes
 * that end before a longword boundary are padded with bytes of unspecified 
 * value.  The attributes that can be specified (subject to security policy
 * configuration) are as follows:
 *
 *	Attribute			Format
 *	---------			------
 *	Effective privileges:		privvec_t privs;
 *
 *	Process ID:			pid_t	pid;
 *
 *	Login user ID:			uid_t	luid;
 *
 *	Effective identity:		uid_t	uid;
 *					gid_t	gid;
 * #if SEC_GROUPS
 *					u_long	gcnt;
 *					gid_t	groups[gcnt];
 # endif
 *
 * #if SEC_MAC
 *	Sensitivity label:		u_long	length;
 *					char	slabel[length];
 * #endif
 *
 * #if SEC_ILB
 *	Information label:		u_long	length;
 *					char	ilabel[length];
 * #endif
 *
 * #if SEC_NCAV
 *	Nationality caveats:		u_long	length;
 *					char	ncav[length];
 * #endif
 */

struct tnet_sec_attr {
	u_short	attr_type;	/* Constant: SEC_EXTENDED_RIGHTS */
	u_short	length;		/* Total length, including header */
	u_long 	mask;		/* Which attributes are present */
/*	char	attr[0];	/* Attributes */
};

/*
 * Internal format of security attributes associated with
 * streams messages.
 */
struct str_sec_attr {
	u_long		mask;	/* Which attributes are present */
	privvec_t	privs;	/* privileges */
	pid_t		pid;	/* process ID */
	uid_t		luid;	/* login user ID */
	uid_t		uid;	/* effective user ID */
	gid_t		gid;	/* effective group ID */
#if SEC_GROUPS
	u_long		g_len;	/* number of supplementary groups */
	gid_t		groups[SEC_GROUPS];	/* supplementary groups */
#endif
#if SEC_ARCH
	tag_t		tags[SEC_TAG_COUNT];
#endif
};

#endif /* SEC_BASE */
#endif /* __SEC_STREAM__ */
