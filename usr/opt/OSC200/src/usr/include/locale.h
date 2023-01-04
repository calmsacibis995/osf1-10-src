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
 *	@(#)$RCSfile: locale.h,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/06/08 01:02:11 $
 */ 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCLOC) Locale Related Data Structures and API
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.24  com/inc/locale.h, libcnls, bos320, 9132320h 8/7/91
 */
#ifndef _LOCALE_H_
#define _LOCALE_H_

#include <standards.h>

#ifndef NULL
#define NULL	0L
#endif

struct lconv {
   char *decimal_point;		/* decimal point character		*/
   char *thousands_sep;		/* thousands separator		 	*/
   char *grouping;		/* digit grouping		 	*/
   char *int_curr_symbol;	/* international currency symbol	*/
   char *currency_symbol;	/* national currency symbol		*/
   char *mon_decimal_point;	/* currency decimal point		*/
   char *mon_thousands_sep;	/* currency thousands separator		*/
   char *mon_grouping;		/* currency digits grouping		*/
   char *positive_sign;		/* currency plus sign			*/
   char *negative_sign;		/* currency minus sign		 	*/
   char int_frac_digits;	/* internat currency fractional digits	*/
   char frac_digits;		/* currency fractional digits		*/
   char p_cs_precedes;		/* currency plus location		*/
   char p_sep_by_space;		/* currency plus space ind.		*/
   char n_cs_precedes;		/* currency minus location		*/
   char n_sep_by_space;		/* currency minus space ind.		*/
   char p_sign_posn;		/* currency plus position		*/
   char n_sign_posn;		/* currency minus position		*/
   char *__left_parenthesis;	/* negative currency left paren         */
   char *__right_parenthesis;	/* negative currency right paren        */
#ifdef _OSF_SOURCE
#define left_parenthesis	__left_parenthesis
#define right_parenthesis	__right_parenthesis
#endif /* _OSF_SOURCE */

};

#define LC_ALL		0xFFFF	/* name of locale's category name 	*/

#define LC_COLLATE	0	/* locale's collation data		*/
#define LC_CTYPE	1	/* locale's ctype handline		*/
#define LC_MONETARY	2	/* locale's monetary handling		*/
#define LC_NUMERIC	3	/* locale's decimal handling		*/
#define LC_TIME		4	/* locale's time handling		*/
#define LC_MESSAGES	5	/* locale's messages handling		*/
#define _LastCategory	LC_MESSAGES	/* This must be last category	*/

#define _ValidCategory(c) \
    ((int)(c) == LC_ALL || ((int)(c) >= 0 && ((int)(c) <= _LastCategory)))

#if defined(__cplusplus)
extern "C" {
#endif
struct lconv *localeconv __((void));
char   *setlocale __((int, const char *));
#if defined(__cplusplus)
}
#endif

#endif /* _LOCALE_H_ */
