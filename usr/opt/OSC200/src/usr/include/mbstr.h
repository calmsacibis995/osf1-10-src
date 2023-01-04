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
 *	@(#)$RCSfile: mbstr.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/08 01:02:18 $
 */ 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 *   COMPONENT_NAME: LIBCNLS
 *
 *   FUNCTIONS: none
 *
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   Copyright (c) 1984 AT&T
 *   All Rights Reserved
 *
 *   THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
 *   The copyright notice above does not evidence any
 *   actual or intended publication of suc
 */
/* 1.5  com/inc/mbstr.h, libcnls, bos320, 9125320 5/16/91 15:19:59 */
/*
 * NAME: mbstr.h
 */                                                                   

#ifndef _MBSTR_H_
#define _MBSTR_H_

#include <standards.h>
#include <sys/types.h>

#define MBMAXLEN	4

typedef unsigned int mbchar_t;

#if defined(__cplusplus)
extern "C" {
#endif
extern char *mbsadvance __((const char *));
extern char *mbscat __((char *, char *));
extern char *mbschr __((const char *, const mbchar_t));
extern int  mbscmp __((char *, char *));
extern char *mbscpy __((char *, char *));
extern char *mbsinvalid __((const char *));
extern size_t       mbslen __((const char *));
extern char *mbsncat __((char *, const char *, size_t));
extern int  mbsncmp __((char *, char *, size_t));
extern char *mbsncpy __((char *, char *, size_t));
extern char *mbspbrk __((char *, char *));
extern char *mbsrchr __((char *, mbchar_t));
extern mbchar_t     mbstomb __((const char *mbs));
extern int  mbswidth __((const char *, size_t ));
extern int  mbstoint __((char *));
#if defined(__cplusplus)
}
#endif

#endif  /* _MBSTR_H_ */
