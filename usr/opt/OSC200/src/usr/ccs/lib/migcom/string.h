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
 *	@(#)$RCSfile: string.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:22:54 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * ABSTRACT:
 *   Header file for string utility programs (string.c)
 */

#ifndef	_STRING_H_
#define	_STRING_H_

#include <strings.h>

typedef char *string_t;
typedef string_t identifier_t;

extern char	charNULL;
#define	strNULL		&charNULL

extern string_t strmake(/* char *string */);
extern string_t strconcat(/* string_t left, right */);
extern void strfree(/* string_t string */);

#define	streql(a, b)	(strcmp((a), (b)) == 0)

extern char *strbool(/* boolean_t bool */);
extern char *strstring(/* string_t string */);

#endif	/* _STRING_H_ */
