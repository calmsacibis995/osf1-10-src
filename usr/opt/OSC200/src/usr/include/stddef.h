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
 *	@(#)$RCSfile: stddef.h,v $ $Revision: 4.2.11.3 $ (DEC) $Date: 1993/10/11 18:32:36 $
 */ 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

/*
 * COMPONENT_NAME: stddef.h
 *                                                                    
 *
 * Copyright International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * stddef.h 1.9  com/inc,3.1,9013 1/14/90 16:46:17
 */                                                                   

#ifndef _STDDEF_H_ 
#define _STDDEF_H_ 
#include <standards.h>

/*
 *
 *      The ANSI standard requires that certain values be in stddef.h.
 *      It also requires that if _ANSI_C_SOURCE is defined then ONLY these
 *      values are present. This header includes all the ANSI required entries.
 *
 */

#ifdef _ANSI_C_SOURCE

/*
 *      The following definitions are included in <sys/types.h>.  They
 *      are included in <stddef.h> to comply with ANSI.
 */

#ifndef _PTRDIFF_T
#define _PTRDIFF_T
typedef long             ptrdiff_t;
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long   size_t;
#endif

#ifndef _WCHAR_T
#define _WCHAR_T

#ifndef __WCHAR_T_LEN
#    define __WCHAR_T_LEN 4
#endif

#if __WCHAR_T_LEN == 1
    typedef unsigned char wchar_t;
#elif __WCHAR_T_LEN == 2
    typedef unsigned short wchar_t;
#else
    typedef unsigned int  wchar_t;
#endif /* __WCHAR_T_LEN == ?? */

#endif /* _WCHAR_T */

#ifndef NULL
#define NULL    0L
#endif

#define offsetof(s_name, s_member) \
        ((size_t)((char *)&((s_name *)0L)->s_member - (char *)0L))

#endif /* _ANSI_C_SOURCE */
#endif /* _STDDEF_H_ */
