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
 *	@(#)$RCSfile: nl_types.h,v $ $Revision: 4.2.8.4 $ (DEC) $Date: 1993/11/09 23:55:53 $
 */ 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: INC
 *
 * FUNCTIONS: nl_types.h
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _NL_TYPES_H_
#define _NL_TYPES_H_

#include <standards.h>

typedef int __nl_item;

#if defined(_XOPEN_SOURCE) || defined(_OSF_SOURCE)
typedef struct __catalog_descriptor *nl_catd;
#endif /* _XOPEN_SOURCE || _OSF_SOURCE */

#ifdef _XOPEN_SOURCE
typedef __nl_item nl_item;

#define NL_CAT_LOCALE	1
#define NL_SETD 	1  				

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */
extern nl_catd catopen __((const char *, int ));
extern char  *catgets __((nl_catd , int , int , const char *));
extern int catclose __((nl_catd ));
#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif /* _XOPEN_SOURCE */

#ifdef _OSF_SOURCE
typedef struct __catalog_descriptor CATD;

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */
extern nl_catd NLcatopen __((char *, int));
extern char *NLgetamsg __((char *, int, int, char *));
extern char *NLcatgets __((nl_catd, int, int, char *));
#if defined(__cplusplus)
}
#endif /* __cplusplus */

#include <mesg.h>
#endif /* _OSF_SOURCE */

#endif /* _NL_TYPES_H_ */
