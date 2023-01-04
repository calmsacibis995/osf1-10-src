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
 *      @(#)$RCSfile: mount.h,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1993/05/29 17:30:38 $
 */
/*
 */

/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 *	Portions of this software have been licensed to 
 *	Digital Equipment Company, Maynard, MA.
 *	Copyright (c) 1986 Sun Microsystems, Inc.  ALL RIGHTS RESERVED.
 */

/*
 *
 *   Modification history:
 *
 * 21-Jul-89 -- lebel
 *  Changed exports structure to accomodate allowing exports
 *  options at the directory level.  Removed expdir structure.
 *
 * 09-Mar-88 -- logcher
 *	Added a pointer to the new expdir structure in exports and
 *	added the expdir structure which allows the mountd to use
 *	a 2x2 matrix of exports rather than 1x1.
 *
 * 14-Jul-87 -- logcher
 *	Changed ex_flags in exports structure from short to u_int for 
 *	continuity with m_flags in mount structure.
 */

#include <nfs/nfs.h>

#define MOUNTPROG 100005
#define MOUNTPROC_MNT 1
#define MOUNTPROC_DUMP 2
#define MOUNTPROC_UMNT 3
#define MOUNTPROC_UMNTALL 4
#define MOUNTPROC_EXPORT 5
#define MOUNTPROC_EXPORTALL 6
#define MOUNTVERS_ORIG 1
#define MOUNTVERS 1

#ifndef svc_getcaller
#define svc_getcaller(x) (&(x)->xp_raddr)
#endif

bool_t xdr_path();
bool_t xdr_fhandle();
bool_t xdr_fhstatus();
bool_t xdr_mountlist();
bool_t xdr_exports();

struct mountlist {		/* what is mounted */
	char *ml_name;
	char *ml_path;
	struct mountlist *ml_nxt;
};

struct fhstatus {
	int fhs_status;
	fhandle_t fhs_fh;
};

/*
 * List of exported directories.
 * An export entry with ex_groups NULL indicates an entry which is 
 * exported to the world. 
 * 
 */
struct exports {
	dev_t   ex_dev;
	ino_t   ex_ino;
	uint_t  ex_gen;
	char    *ex_name; 		/* path */
	int     ex_flags;
	uid_t   ex_rootmap;
        uid_t   ex_anon;
        struct exaddrlist ex_rootaddrs; /* Ip addrs allowed rootmap privs */
        struct exaddrlist ex_writeaddrs; /* Ip addrs allowed rw privs */
	struct groups	 *ex_groups;  /* groups allowed to mount this entry */
	struct exports	 *ex_next;    /* next entry for this dev */
	struct exports   *ex_devptr;  /* pointer to next dev entry if this 
					 is the top entry for this dev */
};

struct groups {
	char		*g_name;
	struct groups	*g_next;
/* private to mountd to identify hostnames */
	int 		g_hbit;
};
