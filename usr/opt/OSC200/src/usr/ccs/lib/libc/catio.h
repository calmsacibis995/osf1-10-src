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
 *	@(#)$RCSfile: catio.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:39:03 $
 */
/*
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/* @(#)$RCSfile: catio.h,v $ $Revision: 4.2.5.2 $ (OSF) $Date: 1993/06/07 22:39:03 $ */
/*
 * COMPONENT_NAME: CMDMSG
 *
 * FUNCTIONS: catio.h 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.13  com/lib/c/msg/catio.h, libcmsg, bos320, bosarea.9125 6/19/91 14:11:23
 */


#include <mesg.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _catalog_rmutex;

#define LOCK		TS_LOCK(&_catalog_rmutex)
#define	UNLOCK		TS_UNLOCK(&_catalog_rmutex)
#define RETURN(e,s)	return (_Seterrno(e), (s))

#else /* _THREAD_SAFE */
#define LOCK		while(0)
#define UNLOCK		while(0)	/* Dummy stmt */
#define	RETURN(e,s)	return(_Seterrno(e), (s))

#endif /* _THREAD_SAFE */

extern  int     _cat_do_open(nl_catd);

#define PATH_FORMAT     "/usr/lib/nls/msg/%L/%N"


#define FILE_UNUSED     (-1)	/* Open failed or short-circuited */
#define FILE_DUMMY      (-2)
#define FILE_CLOSED	(-3)	/* catalog not opened yet */
