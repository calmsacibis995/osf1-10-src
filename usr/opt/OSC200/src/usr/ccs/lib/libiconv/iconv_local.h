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
 *	@(#)$RCSfile: iconv_local.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/08 00:45:25 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/* 
 *
 * COMPONENT_NAME: (LIBICONV)
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.7  com/inc/iconv.h, , bos320, 9134320c 8/19/91 21:45:14
 */

/*
 *	iconv_local.h
 */

#ifndef	_ICONV_LOCAL_H
#define	_ICONV_LOCAL_H

#include <sys/types.h>
#include <errno.h>
#include <sys/localedef.h>

/*
 *	definition of iconv_t type.
 */

typedef struct	__iconv_rec	iconv_rec, *iconv_t;
struct	__iconv_rec	{
	_LC_object_t	hdr;
	iconv_t	(*open)(const char *to, const char *from);
	int	(*exec)(iconv_t cd, char **inbuf, size_t *inbytesleft,
			char **outbuf, size_t *outbytesleft);
	void	(*close)(iconv_t cd);
};

typedef struct _LC_core_iconv_type	_LC_core_iconv_t;
struct _LC_core_iconv_type {

	_LC_object_t  hdr;

	/* implementation initialization */
	_LC_core_iconv_t	*(*init)();
	int	(*exec)();
	void	(*close)();
};


/*
 *	methods.
 */

#ifdef _ICONV_INTERNAL
extern	iconv_t	__iconv_open(const char *, const char *);
extern	iconv_t	iconv_open(const char *, const char *);
extern	size_t	iconv(iconv_t, char **, size_t *, char **, size_t *);
extern	int 	iconv_close(iconv_t);
#else
extern	iconv_t	__iconv_open(const char *, const char *);
#define iconv_open(to, from)	__iconv_open(to, from)
#define	iconv(cd, ibuf, ilen, obuf, olen) \
	((*(cd)->exec)(cd, ibuf, ilen, obuf, olen))
#define	iconv_close(cd)		((*(cd)->close)(cd))
#endif /* _ICONV_INTERNAL */

/*
 *	return values of iconv()
 */

#define	ICONV_DONE	0
#define	ICONV_TRUNC	1	/* EINVAL */
#define	ICONV_INVAL	2	/* EILSEQ */
#define	ICONV_OVER	3	/* E2BIG */

#define _LC_ICONV     10
#define DEFAULTPATH	"/usr/lib/nls/loc:/etc/nls/loc"

#endif	/* _ICONV_LOCAL_H */
