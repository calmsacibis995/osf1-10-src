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
 * @(#)$RCSfile: pathname.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/29 18:53:36 $
 */
/*	@(#)pathname.h	2.3 89/03/20 NFSSRC4.0 from 2.6 86/07/16 SMI	*/

#ifndef _PATHNAME_H_
#define _PATHNAME_H_
/*
 * Pathname structure.
 * System calls which operate on path names gather the
 * pathname from system call into this structure and reduce
 * it by peeling off translated components.  If a symbolic
 * link is encountered the new pathname to be translated
 * is also assembled in this structure.
 */
struct pathname {
	char	*pn_buf;		/* underlying storage */
	char	*pn_path;		/* remaining pathname */
	u_int	pn_pathlen;		/* remaining length */
};

#define PN_STRIP 0x00		/* Strip next component off pn */
#define PN_PEEK	0x01  		/* Only peek at next pn component */
#define pn_peekcomponent(PNP, COMP) pn_getcomponent(PNP, COMP, PN_PEEK)
#define pn_stripcomponent(PNP, COMP) pn_getcomponent(PNP, COMP, PN_STRIP)

#define	pn_peekchar(PNP)	((PNP)->pn_pathlen!=0?*((PNP)->pn_path):(char)0)
#define pn_pathleft(PNP)	((PNP)->pn_pathlen)
#define pn_getpath(PNP)		((PNP)->pn_path)
#define pn_copy(PNP1, PNP2)	(pn_set(PNP2, pn_getpath(PNP1)))

extern int	pn_alloc();		/* allocat buffer for pathname */
extern int	pn_get();		/* allocate buf and copy path into it */
#ifdef notneeded
extern int	pn_getchar();		/* get next pathname char */
#endif
extern int	pn_set();		/* set pathname to string */
extern int	pn_combine();		/* combine to pathnames (for symlink) */
extern int	pn_getcomponent();	/* get next component of pathname */
extern void	pn_skipslash();		/* skip over slashes */
extern void	pn_free();		/* free pathname buffer */
extern int	pn_append();		/* Append string to pathname */
extern int	pn_getlast();		/* Get last component of pathname */ 

#endif
