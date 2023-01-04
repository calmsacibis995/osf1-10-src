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
 *	@(#)$RCSfile: nan.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:05:43 $
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
/*	@(#)nan.h	3.1	*/
/*		*/
/*	2/26/91	*/
/*	5.2 SID nan.h	1.3	*/
/* Handling of Not_a_Number's (only in IEEE floating-point standard) */

#define KILLFPE()	(void) kill(getpid(), 8)
#if u3b || u3b5
#define NaN(X)	(((union { double d; struct { unsigned :1, e:11; } \
				s; } *)&X)->s.e == 0x7ff)
#define KILLNaN(X)	if (NaN(X)) KILLFPE()
#else
#if n16
union __NaN {
	double d;
	float f;
	long i[2];
	struct {
		unsigned dflow:32;
		unsigned dfhi:20;
		unsigned de:11;
		unsigned :1;
	} ds;
	struct {
		unsigned ff:23;
		unsigned fe:8;
		unsigned :1;
	} fs;
};
#define NaN(X)	((((union __NaN *)&X)->ds.de == 0x7ff))
#define NaD(X)	((((union __NaN *)&X)->ds.de == 0x7ff) || \
		((((union __NaN *)&X)->ds.de == 0) && \
		((((union __NaN *)&X)->i[0] != 0) || \
		((((union __NaN *)&X)->i[1]&0x7fffffff) != 0))))
#define NaF(X)	((((union __NaN *)&X)->fs.fe == 0xff) || \
		((((union __NaN *)&X)->fs.fe == 0) && \
		((((union __NaN *)&X)->i[0]&0x7fffffff) != 0)))
#define KILLNaN(X)	if (NaN(X)) KILLFPE()
#else
#define Nan(X)	0
#define KILLNaN(X)
#endif
#endif
