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
 * @(#)$RCSfile: paclif.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:13:10 $
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	paclif.h,v $
 * Revision 1.1.1.1  92/05/12  01:45:00  devrcs
 *  *** OSF1_1B29 version ***
 * 
 * Revision 1.1.2.3  1992/04/05  18:20:15  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:53:17  marquard]
 *
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1991 SecureWare, Inc.  All rights reserved.
 * @(#)paclif.h	1.1 11:17:55 11/8/91 SecureWare
 */

#ifndef __Paclif__
#define __Paclif__

struct paclif {
	char	filename [_POSIX_PATH_MAX];
	int	is_ipc_object;
	char	creator_name [NUSERNAME];
	char    creator_group [NGROUPNAME];
	char	owner_name [NUSERNAME];
	char	owner_group [NGROUPNAME];
	char	user_perms [4];
	char	backing_user_perms [4];
	char	group_perms [4];
	char	backing_group_perms [4];
	char	group_eff [4];
	char	other_perms [4];
	char	backing_other_perms [4];
	char	mask_perms [4];
	char	backing_mask_perms [4];
	int	number_users;
	char	**user;  
	char	**backing_user;
	int	number_groups;
	char	**group;
	char	**backing_group;
	char	mask_recalculate;
};

struct acc_ids {
	char	username [NUSERNAME];
	int	number_groups;
	char	**group;
	char	perms [4];
};

/* 
 * The width of one of the entry lines (acl_user and acl_group).
 * 8 chars + blank + perms + blank + perms 
 */

#define ENTRY_WIDTH	16

/*
 * Special case flags necessary for the read_paclif and write_paclif 
 * routines:
 */

#define PACLIF_DEFAULT_ENTRY	1
#define PACLIF_DELETE_ACL	2
#define PACLIF_NULL_ACL		3
/*
 * The initial contents of the fields to display user and group fields.
 */

#define INITIAL_LINE	"         --- ---"

/*
 * Utility routines
 */

int	isblank();
char	*trim_name();

#endif
