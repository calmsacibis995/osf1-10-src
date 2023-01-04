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
 *	@(#)$RCSfile: setjmp.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:05:57 $
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
/*	setjmp.h	4.1	83/05/03	*/

/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/*
 * COMPONENT_NAME: (INCSTD) setjmp.h
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

#ifndef _SETJMP_H_
#define _SETJMP_H_
#include <standards.h>

/*
 *
 *      The ANSI and POSIX standards require that certain values be in setjmp.h.
 *      They also require that if _ANSI_C_SOURCE or _POSIX_SOURCE is defined 
 *      then ONLY those standard specific values are present. This header 
 *      includes all the ANSI and POSIX required entries.
 *
 */

#ifdef _ANSI_C_SOURCE
#define _JBLEN  11      /* regs, fp regs, cr, sigmask, context, etc. */

typedef int jmp_buf[_JBLEN];

#ifdef   _NO_PROTO
extern void longjmp();
extern int setjmp(); 
#else  /*_NO_PROTO */
extern void longjmp(jmp_buf , int );
extern int setjmp(jmp_buf ); 
#endif /*_NO_PROTO */

#endif /* _ANSI_C_SOURCE */

#ifdef _POSIX_SOURCE

typedef int sigjmp_buf[_JBLEN];
#define sigsetjmp(env, save)	((save) ? setjmp(env) : _setjmp(env))
#ifdef _NO_PROTO
extern void siglongjmp();
#else
extern void siglongjmp(sigjmp_buf, int);
#endif /* _NO_PROTO */
#endif /* _POSIX_SOURCE */

#ifdef _OSF_SOURCE

#ifdef _NO_PROTO
extern int _setjmp ();
extern void _longjmp ();
#else
extern int _setjmp (jmp_buf);
extern void _longjmp (jmp_buf, int);
#endif

#ifdef _REENTRANT
#ifdef setjmp
#undef setjmp
#endif
#define setjmp(buf)	_setjmp(buf)
#endif

#endif  /* _OSF_SOURCE */

#endif /* _SETJMP_H_ */
