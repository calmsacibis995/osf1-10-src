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
 *	@(#)$RCSfile: localenv.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:40:19 $
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
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

#ifndef _H_LOCALENV
#define _H_LOCALENV

/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: localenv.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <limits.h>
#include <dirent.h>

#define BUFFSIZE 512
#define NLPATH  "/usr/lib/nls/"
#define PASS 0
#define FAIL -1


struct locarray {
	char category[MAXNAMLEN];		
}; 

struct localeinfo {
	char value[BUFFSIZE];
	char parsed;	
 }; 


#define NLTIME 		0
#define NLDATE 		1
#define NLLDATE		2
#define NLDATIM		3
#define NLSDAY		4
#define NLLDAY		5
#define NLSMONTH	6
#define NLLMONTH	7
#define NLTMISC		8
#define DEC_PNT		9
#define THOUS_SEP	10
#define GROUPING	11
#define INT_CUR_SYM	12
#define CUR_SYM		13
#define MON_DEC_PNT	14
#define MON_THOUS	15
#define MON_GRP		16
#define POS_SGN		17
#define NEG_SGN		18
#define INT_FRAC_DIG	19
#define FRAC_DIG	20
#define P_CS_PRE	21
#define P_SEP_SP	22
#define N_CS_PRE	23
#define N_SEP_SP	24
#define P_SGN_POS	25
#define N_SGN_POS	26
#define MESSAGES	27
#define YES_STR		28
#define NO_STR		29
#define AMSTR		30
#define PMSTR		31

#endif /* _H_LOCALENV */
