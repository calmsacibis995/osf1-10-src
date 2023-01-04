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
 * @(#)$RCSfile: mathlib.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/14 17:45:52 $
 */

/*****************************************************************************/
/*FILE: MATHLIB.H - calls all the math emulation routines		     */
/*****************************************************************************/
 
/*****************************************************************************
! REVISION HISTORY:
! Who   When            What
!---------------------------------------------------------------
! HA	16-Dec-1991	eliminate pre SRM4.0 stuff
! SJM	23-Oct-1989	First pass
 *****************************************************************************/

#ifndef _MATHLIB_H_
#define _MATHLIB_H_

#define	DOADD128(a,b,d)		ADD128((int *) a,(int *) b,(int *) d)
#define	DOADD64(a,b,d)		ADD64((int *) a,(int *) b,(int *) d)
#define	DOADD32(a,b,d)		ADD32((int *) a,(int *) b,(int *) d)

#define	DOSUB128(a,b,d)		SUB128((int *) a,(int *) b,(int *) d)
#define	DOSUB64(a,b,d)		SUB64((int *) a,(int *) b,(int *) d)
#define	DOSUB32(a,b,d)		SUB32((int *) a,(int *) b,(int *) d)

#define	DOMUL32(sgn,a,b,d1,d2)	MUL32((int) sgn,(int *) a,(int *) b,(int *) d1,(int *) d2)
#define	DOMUL64(sgn,a,b,d1,d2)	MUL64((int) sgn,(int *) a,(int *) b,(int *) d1,(int *) d2)
#define	DODIV128(sgn,a,b,d1)	DIV128((int) sgn,(int *) a,(int *) b,(int *) d1)
#define	DODIV64(sgn,a,b,d1,d2)	DIV64((int) sgn,(int *) a,(int *) b,(int *) d1,(int *) d2)

#define	DOCVTFG(opt,a,d)	CVTFG((int) opt, (unsigned int *) a, (unsigned int *) d)
#define	DOCVTGF(opt,a,d)	CVTGF((int) opt, (unsigned int *) a, (unsigned int *) d)
#define	DOCVTGD(opt,a,d)	CVTGD((int) opt, (unsigned int *) a, (unsigned int *) d)
#define	DOCVTDG(opt,a,d)	CVTDG((int) opt, (unsigned int *) a, (unsigned int *) d)
#define	DOCVTST(opt,a,d)	CVTST((int) opt, (unsigned int *) a, (unsigned int *) d)
#define	DOCVTTS(opt,a,d)	CVTTS((int) opt, (unsigned int *) a, (unsigned int *) d)

#define	DOCVTQF(opt,a,d)	CVTQF((int) opt, (unsigned int *) a, (unsigned int *) d)
#define	DOCVTQG(opt,a,d)	CVTQG((int) opt, (unsigned int *) a, (unsigned int *) d)
#define	DOCVTQS(opt,a,d)	CVTQS((int) opt, (unsigned int *) a, (unsigned int *) d)
#define	DOCVTQT(opt,a,d)	CVTQT((int) opt, (unsigned int *) a, (unsigned int *) d)

#define	DOCVTFQ(opt,a,d)	CVTFQ((int) opt, (unsigned int *) a, (unsigned int *) d)
#define	DOCVTGQ(opt,a,d)	CVTGQ((int) opt, (unsigned int *) a, (unsigned int *) d)
#define	DOCVTSQ(opt,a,d)	CVTSQ((int) opt, (unsigned int *) a, (unsigned int *) d)
#define	DOCVTTQ(opt,a,d)	CVTTQ((int) opt, (unsigned int *) a, (unsigned int *) d)

#define	DOADDF(opt,a,b,d)	ADDF((int) opt, (unsigned int *) a, (unsigned int *) b, (unsigned int *) d)
#define	DOADDG(opt,a,b,d)	ADDG((int) opt, (unsigned int *) a, (unsigned int *) b, (unsigned int *) d)
#define	DOADDS(opt,a,b,d)	ADDS((int) opt, (unsigned int *) a, (unsigned int *) b,(unsigned int *) d)
#define	DOADDT(opt,a,b,d)	ADDT((int) opt, (unsigned int *) a, (unsigned int *) b,(unsigned int *) d)

#define	DOSUBF(opt,a,b,d)	SUBF((int) opt, (unsigned int *) a, (unsigned int *) b, (unsigned int *) d)
#define	DOSUBG(opt,a,b,d)	SUBG((int) opt, (unsigned int *) a, (unsigned int *) b, (unsigned int *) d)
#define	DOSUBS(opt,a,b,d)	SUBS((int) opt, (unsigned int *) a, (unsigned int *) b,(unsigned int *) d)
#define	DOSUBT(opt,a,b,d)	SUBT((int) opt, (unsigned int *) a, (unsigned int *) b,(unsigned int *) d)

#define	DOMULF(opt,a,b,d)	MULF((int) opt, (unsigned int *) a, (unsigned int *) b, (unsigned int *) d)
#define	DOMULG(opt,a,b,d)	MULG((int) opt, (unsigned int *) a, (unsigned int *) b, (unsigned int *) d)
#define	DOMULS(opt,a,b,d)	MULS((int) opt, (unsigned int *) a, (unsigned int *) b,(unsigned int *) d)
#define	DOMULT(opt,a,b,d)	MULT((int) opt, (unsigned int *) a, (unsigned int *) b,(unsigned int *) d)

#define	DODIVF(opt,a,b,d)	DIVF((int) opt, (unsigned int *) a, (unsigned int *) b, (unsigned int *) d)
#define	DODIVG(opt,a,b,d)	DIVG((int) opt, (unsigned int *) a, (unsigned int *) b, (unsigned int *) d)
#define	DODIVS(opt,a,b,d)	DIVS((int) opt, (unsigned int *) a, (unsigned int *) b, (unsigned int *) d)
#define	DODIVT(opt,a,b,d)	DIVT((int) opt, (unsigned int *) a, (unsigned int *) b, (unsigned int *) d)

#define	DOCMPGEQ(opt,a,b,d)	CMPGEQ((int) opt, (unsigned int *) a, (unsigned int *) b,(unsigned int *) d)
#define	DOCMPGLT(opt,a,b,d)	CMPGLT((int) opt, (unsigned int *) a, (unsigned int *) b,(unsigned int *) d)
#define	DOCMPGLE(opt,a,b,d)	CMPGLE((int) opt, (unsigned int *) a, (unsigned int *) b,(unsigned int *) d)
#define	DOCMPTEQ(opt,a,b,d)	CMPTEQ((int) opt, (unsigned int *) a, (unsigned int *) b,(unsigned int *) d)
#define	DOCMPTLT(opt,a,b,d)	CMPTLT((int) opt, (unsigned int *) a, (unsigned int *) b,(unsigned int *) d)
#define	DOCMPTLE(opt,a,b,d)	CMPTLE((int) opt, (unsigned int *) a, (unsigned int *) b,(unsigned int *) d)
#define	DOCMPTUN(opt,a,b,d)	CMPTUN((int) opt, (unsigned int *) a, (unsigned int *) b,(unsigned int *) d)

#endif
