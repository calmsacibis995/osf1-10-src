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
 * @(#)$RCSfile: rpc.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/07/21 14:35:11 $
 */

/*
 * Alpha/OSF used to include rpc/auth.h & rpc/auth_unix.h, but the former
 * typedefs u_int32 like we do, but the compiler squawks about it.
 * Therefore, we define stuff to keep rpc/rpc_msg.h happy.
 *
 * While we're at it, we make opaque_auth look more like what's
 * on the wire.  This is the first place where we have to start decoding
 * variable sized data.
 */
#define	_rpc_auth_h
#define	_rpc_auth_unix_h

struct opaque_auth {
	enum_t	oa_flavor;	/* flavor of auth */
	u_int	oa_length;	/* not to exceed MAX_AUTH_BYTES */
	/* variable amount of data here */
};

enum auth_stat {		/* Trimmed, but enough to compile */
	AUTH_OK=0
};
