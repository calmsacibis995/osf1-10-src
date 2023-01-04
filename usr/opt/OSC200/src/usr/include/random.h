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
 *	@(#)$RCSfile: random.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/12/15 22:13:54 $
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
 * defintions for the reentrant versions of random()
 */
#ifndef	_RANDOM_H_
#define	_RANDOM_H_

/*
 * random_r.h
 *
 * Include file for thread-safe version of random.
 */

 typedef struct random_data {
 	int	*fptr;
	int	*rptr;
	int	*state;
	int	rand_type;
	int	rand_deg;
	int	rand_sep;
	int	*end_ptr;
} RANDOMD;

/* functions */

#ifdef _NO_PROTO
extern	int	srandom_r();
extern	int	initstate_r();
extern	char	*setstate_r();
extern	int	random_r();
#else
#ifdef __cplusplus
extern "C" {
#endif
extern	int	srandom_r(unsigned, RANDOMD *);
extern	int	initstate_r(unsigned, char *, int, RANDOMD *);
extern	char	*setstate_r(char *, RANDOMD *);
extern	int	random_r(RANDOMD *, int *);
#ifdef __cplusplus
}
#endif
#endif /* _NO_PROTO */

#endif	/* _RANDOM_H_ */
