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
 *	@(#)$RCSfile: NLxio.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:13:58 $
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
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Library Support
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _NLXIO_H_
#define _NLXIO_H_

/*
 *
 * FUNCTION:	This file is designed to be included by NLxio.c
 * and any programs using _NLxin/_NLxout
 */

struct NLxtbl {
	char source[16];
	char target[16];
	char chrs[256];
};

#ifdef   _NO_PROTO
int NLxout();
int NLxin();
#else  /*_NO_PROTO */
int NLxin(char *,char *,int );
int NLxout(char *,char *,int );
#endif /*_NO_PROTO */

extern struct NLxtbl *_NLxitbl;
extern struct NLxtbl *_NLxotbl;

#define _NLxin(c)	(_NLxitbl->chrs[c])
#define _NLxout(c)	(_NLxotbl->chrs[c])

#endif /* _NLXIO_H_ */
