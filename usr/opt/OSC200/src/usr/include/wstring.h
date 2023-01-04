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
 *	@(#)$RCSfile: wstring.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:19:54 $
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
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _WSTRING_H_
#define _WSTRING_H_

#include <wchar.h>

extern double	 wstrtod(wchar_t *, wchar_t **);
extern double	 watof(wchar_t *);
extern int	 watoi(wchar_t *);
extern long	 watol(wchar_t *);
extern long	 wstrtol(wchar_t *, wchar_t **, int);

#ifdef _KJI
extern wchar_t
	*wstrdup(wchar_t *),
	*wstrcpy(wchar_t *, wchar_t *),
	*wstrncpy(wchar_t *, wchar_t *, int),
	*wstrcat(wchar_t *, wchar_t *),
	*wstrncat(wchar_t *, wchar_t *, int),
	*wstrchr(wchar_t *, int),
	*wstrrchr(wchar_t *, int),
	*wstrpbrk(wchar_t *, wchar_t *),
	*wstrtok(wchar_t *, wchar_t *);

extern size_t
	wstrlen(wchar_t *),
	wstrspn(wchar_t *, wchar_t *),
	wstrcspn(wchar_t *, wchar_t *);

extern int
	wstrcmp(wchar_t *, wchar_t *),
	wstrncmp(wchar_t *, wchar_t *, int);
#endif /* _KJI */

#endif /* _WSTRING_H_ */
