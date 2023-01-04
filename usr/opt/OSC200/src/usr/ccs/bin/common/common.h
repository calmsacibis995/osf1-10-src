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
 *	@(#)$RCSfile: common.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/12/03 15:37:19 $
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
 * COMPONENT_NAME: (CMDPROG) common.h
 *
 * FUNCTIONS: outdouble                                                      
 *
 * ORIGINS: 27 03 09 32 00 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Changes for ANSI C were developed by HCR Corporation for IBM
 * Corporation under terms of a work made for hire contract.
 */

# include "caloff.h"
# include "error.h"
# include "opdesc.h"
# include "treemgr.h"
# include "treewalk.h"

/* -------------------- outdouble -------------------- */

outdouble(dfval, type)
#ifdef HOSTIEEE
  double dfval;
#else
  FP_DOUBLE dfval;
#endif
  TWORD type;
{
#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
	union {
#ifdef HOSTIEEE
		double one;
#else
		FP_DOUBLE one;
#endif
		struct {unsigned long dfracth, dfractl;} two;
	} split;
	unsigned long longone;

	split.two.dfracth = split.two.dfractl = 0;
	split.one = dfval;
	if ( type == FLOAT) {
#ifdef HOSTIEEE
		float afloat;

		afloat = split.one;
		longone = *(long *)(&afloat);
#else
		longone = _FPd2fi(1, split.one);
#endif
		printf("\t.long\t0x%lX\n", longone);
	}
	else if (type == DOUBLE || type == LDOUBLE) {
		printf("\t.long\t0x%lX\n\t.long\t0x%lX\n",
			split.two.dfracth, split.two.dfractl);
	}
	else cerror(TOOLSTR(M_MSG_269, "outdouble: bad arguments\n"));
#endif
}
