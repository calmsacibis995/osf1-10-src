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
 * @(#)$RCSfile: patlocal.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/07 23:32:43 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#ifndef __H_PATLOCAL
#define __H_PATLOCAL
/*
 * COMPONENT_NAME: (LIBCPAT) Internal Regular Expression
 *
 * FUNCTIONS: 
 *
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.2  com/lib/c/pat/patlocal.h, , bos320, 9134320 8/13/91 14:48:33 
 */

#include <sys/types.h>
#include <sys/localedef.h>


/************************************************************************/
/*									*/
/* Function prototypes							*/
/*									*/
/************************************************************************/

extern	int	_mbce_lower(char *, size_t, char *);

extern	wchar_t	_mbucoll(_LC_collate_t *, char *, char **);

/************************************************************************/
/*									*/
/* Macros to determine collation weights				*/
/*									*/
/************************************************************************/

#define MAX_PC		phdl->co_wc_max		/* max process code	*/

#define MIN_PC		phdl->co_wc_min		/* min process code	*/

#define MAX_UCOLL	phdl->co_col_max 	/* max unique weight	*/

#define MIN_UCOLL	phdl->co_col_min 	/* min unique weight	*/

#define	MAX_NORDS	phdl->co_nord		/* # collation weights	*/

#define CLASS_SIZE	32			/* [: :] max length	*/

#endif /* __H_PATLOCAL */
