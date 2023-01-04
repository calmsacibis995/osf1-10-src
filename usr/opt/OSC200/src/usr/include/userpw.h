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
 *	@(#)$RCSfile: userpw.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:19:37 $
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
 * COMPONENT_NAME: (userpw.h) header to access user passwd info 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _USERPW_H_    
#define _USERPW_H_

#include <standards.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/limits.h>

/* function declarations	*/
#ifndef _NO_PROTO
	extern	struct userpw  	*getuserpw (char *);
	extern	int		putuserpw (struct userpw *);
	extern	int		setpwdb (int);
	extern	int		endpwdb (void);
#else	
	extern	struct userpw  	*getuserpw ();
	extern	int		putuserpw ();
	extern	int		setpwdb ();
	extern	int		endpwdb ();
#endif


/* manifest constants and limits	*/
#define	MAX_PASS	PASS_MAX

/* flags for user password */
#define	PW_NOCHECK	0x1
#define	PW_ADMCHG	0x2
#define	PW_ADMIN	0x4

#define PW_PASSLEN	8		/* maximum length of a password */
#define PW_NAMELEN	32		/* max length of a user's name */
#define PW_CRYPTLEN	16		/* length of the encrypted password */

struct	userpw
{
	char	upw_name[PW_NAMELEN];	/* user's name */
	char	*upw_passwd;		/* user's passwd */
	ulong	upw_flags;		/* flags of restrictions */
	ulong	upw_lastupdate;		/* date of last passwd update */
};

#endif /* _USERPW_H_ */
