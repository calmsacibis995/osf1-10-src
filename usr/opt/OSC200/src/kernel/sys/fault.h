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
 * @(#)$RCSfile: fault.h,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/25 09:44:54 $
 */

/*
 * This header file defines the /proc fault tracing nmemonics in terms of
 * EXC_ defines (from <machine/cpu.h> for the mips acrhitecture).
 */

#ifndef	_SYS_FAULT_H_
#define	_SYS_FAULT_H_

#ifdef mips

#include	<machine/cpu.h>

#define	FLTILL		(EXC_II)		/* illegal instruction */
#define	FLTBPT		(EXC_BREAK)		/* breakpoint */
#define	FLTIOVF		(EXC_OV)		/* overflow */
#define	FLTIZDIV	(EXC_BREAK)		/* n/0 handled via breakpoint*/
#define	FLTPAGE		(EXC_RMISS | EXC_WMISS)	/* read or write TLB miss */

/* #define	FLTPRIV		not supported by MIPS architecture */
/* #define	FLTTRACE	not supported by MIPS architecture */
/* #define	FLTACCESS	not supported by MIPS architecture */
/* #define	FLTBOUNDS	not supported by MIPS architecture */
/* #define	FLTFPE		not supported by MIPS architecture */
/* #define	FLTSTACK	not supported by MIPS architecture */

#endif /* mips */

#ifdef __alpha

#include	<machine/trap.h>

#define	FLTIOVF		T_ARITH
#define	FLTIZDIV	T_ARITH
#define	FLTILL		T_IFAULT
#define	FLTBPT		T_IFAULT
#define	FLTPAGE		T_MMANG

#endif /* __alpha */

#endif	/* _SYS_FAULT_H_ */
