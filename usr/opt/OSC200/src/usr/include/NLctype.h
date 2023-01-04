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
 *	@(#)$RCSfile: NLctype.h,v $ $Revision: 4.2.6.4 $ (DEC) $Date: 1993/11/09 23:55:50 $
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
 * ORIGINS: 3 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 */
#ifndef _NLCTYPE_H_
#define _NLCTYPE_H_

#include <NLchar.h>
#include <ctype.h>

#ifdef _KJI
#include <jctype.h>    
#endif  /* _KJI */

/*
 *  NOTE!!!  All of these character classifications are duplicated in
 *	     usr/bin/ctab.c and they must always match EXACTLY.  Change
 *	     them here and the identical change(s) must be made to
 *	     ctab.c!
 */
#define	_U	   01
#define	_L	   02
#define	_N	   04
#define	_S	  010
#define	_P	  020
#define	_C	  040
#define	_B	 0100
#define	_X	 0200
#define _A       0400
#define _G      01000

/*  Ctype definitions for use with setlocale (loc_t->lc_ctype) table.
 *  (Note:  NCisshift definition is in NLchar.h.)
 */

#ifndef lint
#ifdef _KJI
#define	_NCtoupper(c)	(isascii(c) ? _toupper(c) : _tojupper(c))
#define	_NCtolower(c)	(isascii(c) ? _tolower(c) : _tojlower(c))
#define _atojis(c)	(_atojistab[(c) - 0x20])
#define _jistoa(c)	(_jistoatab[((c)>>8) - 0x81][((c)&0xff) - 0x40])
#endif   /* _KJI */
#endif   /* lint */

/* The following macros implement character "flattening". This feature
 * may not be portable to future releases of OSF/1.
 */

/*  Macros with no old equivalents.
 */
#ifndef _KJI
extern char _NLflattab[];
extern unsigned _NLflattsize;
#endif   /* _KJI */

#ifndef lint
#ifndef _KJI
#ifdef _NAME_SPACE_WEAK_STRONG
#ifdef NCflatchr
#undef NCflatchr
#endif
#endif
#define	NCflatchr(c)	(((unsigned)(c) < 0x80) ? \
				(c) : ((unsigned)(c) - 0x80 < _NLflattsize) ? \
				_NLflattab[c - 0x80] : '?')
#endif /* _KJI */
#endif   /* lint */

#ifndef lint
/*  Translate single NLchar at nlc to char escape string at c.
 */
#ifdef _KJI
#define hextoa(c)       (((c) < 10) ? ('0' + (c)) : ('a' + ((c) - 10)))
#define atohex(c)       (((c) <= '9') ? ((c) - '0') : (((c) - 'a') + 10))
/*  Translate single NLchar at nlc to char escape string at c.
 */
#define NCeschex(nlc, c)    ((c)[0] = '\\', \
			    (c)[1] = '<', \
                            (c)[2] = hextoa ((*(nlc) >> 12) & 0xf), \
                            (c)[3] = hextoa ((*(nlc) >> 8) & 0xf), \
                            (c)[4] = hextoa ((*(nlc) >> 4) & 0xf), \
                            (c)[5] = hextoa (*(nlc) & 0xf), \
			    (c)[6] = '>')

/*  Translate hex escape string at c to single NLchar at nlc.
 */
#define NCuneschex(c, nlc)  ((nlc)[0] = (((atohex((c)[2])) & 0xf) << 12) | \
                                        (((atohex((c)[3])) & 0xf) << 8) | \
                                        (((atohex((c)[4])) & 0xf) << 4) | \
                                        ((atohex((c)[5])) & 0xf))
                                         
#define ishexesc(c)	    (((((c)[0] >= 'a' && (c)[0] <= 'f') || \
					((c)[0] >= '0' && (c)[0] <= '9')) && \
				(((c)[1] >= 'a' && (c)[1] <= 'f') || \
					((c)[1] >= '0' && (c)[1] <= '9')) && \
				(((c)[2] >= 'a' && (c)[2] <= 'f') || \
					((c)[2] >= '0' && (c)[2] <= '9')) && \
				(((c)[3] >= 'a' && (c)[3] <= 'f') || \
					((c)[3] >= '0' && (c)[3] <= '9'))) ? 1 : -1)
#endif   /* _KJI */
#endif   /* lint */

#endif	/* _NLCTYPE_H_ */
