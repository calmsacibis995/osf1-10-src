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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/* @(#)$RCSfile: crypt.h,v $ $Revision: 4.2.5.3 $ (OSF) $Date: 1993/12/15 22:13:16 $ */

/*
 * definitions for the reentrant versions of crypt()
 */

#ifndef	_CRYPT_H_
#define	_CRYPT_H_

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
typedef	struct crypt_data {
	char C[28], D[28];
	char E[48];
	char KS[16][48];
	char block[66], iobuf[16];
} CRYPTD;

#ifdef _NO_PROTO
extern	char	*crypt_r();
extern	int	setkey_r();
extern	void	encrypt_r();
#else
#ifdef __cplusplus
extern "C" {
#endif
extern	char	*crypt_r(const char *, const char *, CRYPTD *);
extern	int	setkey_r(char *, CRYPTD *);
extern	void	encrypt_r(char *, int, CRYPTD *);
#ifdef __cplusplus
}
#endif
#endif /* _NO_PROTO */
#endif
#endif	/* _CRYPT_H_ */
