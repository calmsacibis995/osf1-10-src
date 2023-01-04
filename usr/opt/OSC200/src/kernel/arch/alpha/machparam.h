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
 *	"@(#)machparam.h	9.1	(ULTRIX/OSF)	10/21/91"
 */ 
/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/*
 * Modification History: machine/alpha/machparam.h
 *
 * 30-Apr-91 -- afd
 *	derived from Alpha/Ultrix-64 machine/alpha/param.h
 *
 */

#ifndef _MACHPARAM_H_
#define _MACHPARAM_H_

#ifndef BYTE_ORDER
#ifndef ASSEMBLER
#include <machine/endian.h>
#endif
#endif
#include <machine/machlimits.h>
/*
 * Machine dependent constants for alpha.
 */
#define	MACHINE		"alpha"

#define CLSIZELOG2	0
#define CLSIZE		(1<<CLSIZELOG2)

#define SELF_PFN	(-2L)
#define OTHER_PFN	(-3L)

#ifdef KERNEL
#include <machine/pmap.h>
#else /* KERNEL */
typedef long pt_entry_t;
#endif /* KERNEL */

#ifdef __LANGUAGE_C__

/*
 * KERNEL_PARAMETERS is intended to be a general repository for
 * frequently accessed data.  The structure is at most 32 KB mapped at
 * 0xffff ffff ffff 8000.  Accesses to quadwords in this range require
 * a single instruction, "ldq/stq rx, offset(r31)".
 */

struct kernel_parameters {
	struct vm_parameters {
		long nbpg;
		long pgofset;
		long pgshift;
		long address_mask;
		pt_entry_t * selfmap;
		pt_entry_t * othermap;
		long unity_base;
		long seg1_base;
	} vm;
};

#ifdef ALPHA_VARIABLE_PAGE_SIZE
/*
 * Presently, KERNEL_PARAMETERS is used only by the memory management
 * subsystem, and all the values are derived from the page size.
 * Until there is more than one Alpha page size, these values can be
 * computed at compiletime, and KERNEL_PARAMETERS doesn't exist.
 */

#define KERNEL_PARAMETERS ((struct kernel_parameters *)(-1L << 15))

#define NBPG		(KERNEL_PARAMETERS->vm.nbpg)
#define PGOFSET		(KERNEL_PARAMETERS->vm.pgofset)
#define PGSHIFT		(KERNEL_PARAMETERS->vm.pgshift)
#define ADDRESS_MASK	(KERNEL_PARAMETERS->vm.address_mask)
#define Selfmap		(KERNEL_PARAMETERS->vm.selfmap)
#define Othermap	(KERNEL_PARAMETERS->vm.othermap)
#define UNITY_BASE	(KERNEL_PARAMETERS->vm.unity_base)
#define SEG1_BASE	(KERNEL_PARAMETERS->vm.seg1_base)

/*
 * init_vm_parameters is invoked by pmap_bootstrap.  If
 * KERNEL_PARAMETERS is expanded to include data not dependent on page
 * size, and this happens before the appearance of variable size
 * pages, then this macro must move outside the scope of "#ifdef
 * ALPHA_VARIABLE_PAGE_SIZE..."
 */
#define init_vm_parameters(page_shift, page_table) if (1) {		       \
	long _PGSHIFT = (page_shift);					       \
	long _NBPG = 1L << _PGSHIFT;					       \
	long _PGOFSET = _NBPG - 1;					       \
	long _INTRA_PAGE_OFFSET = (long)KERNEL_PARAMETERS & _PGOFSET;	       \
	struct kernel_parameters *_PARAM;				       \
									       \
	_PARAM = (struct kernel_parameters *)				       \
		    (map_bootspace(NEXT_VA(), next_pfn(), PROT_KW)	       \
		     + _INTRA_PAGE_OFFSET /* non-zero for 64KB pages */ );     \
	bzero((char*)_PARAM - _INTRA_PAGE_OFFSET, _NBPG);		       \
	(page_table)[level3_vpn(KERNEL_PARAMETERS)] = *BOOT_vtopte(_PARAM);    \
	_PARAM->vm.nbpg = _NBPG;					       \
	_PARAM->vm.pgofset = _PGOFSET;					       \
	_PARAM->vm.pgshift = _PGSHIFT;					       \
	_PARAM->vm.address_mask = ~(unsigned long)0 >> (73 - 4* _PGSHIFT);     \
	_PARAM->vm.selfmap = (pt_entry_t *)(SELF_PFN << (3 * _PGSHIFT - 6));   \
	_PARAM->vm.othermap = (pt_entry_t *)(OTHER_PFN << (3 * _PGSHIFT - 6)); \
	_PARAM->vm.unity_base = (~0L) << (4 * _PGSHIFT - 10);		       \
	_PARAM->vm.seg1_base = (~0L) << (4 * _PGSHIFT - 11);		       \
} else

#else	/* !ALPHA_VARIABLE_PAGE_SIZE, i.e. 8KB PAGES */

#define KERNEL_PARAMETERS ((vm_offset_t)0) /* used to catch HWRPB overrun */

#define PGSHIFT		13
#define NBPG		((long)(1<<PGSHIFT))
#define PGOFSET		(NBPG - 1)
#define ADDRESS_MASK	(~(unsigned long)0 >> (73 - 4*PGSHIFT))
#define Selfmap		((pt_entry_t *)(SELF_PFN << (3 * PGSHIFT - 6)))
#define Othermap	((pt_entry_t *)(OTHER_PFN << (3 * PGSHIFT - 6)))
#define UNITY_BASE	((~0L) << (4 * PGSHIFT - 10))
#define SEG1_BASE	((~0L) << (4 * PGSHIFT - 11))

#define init_vm_parameters(page_shift, page_table)

#endif	/* ALPHA_VARIABLE_PAGE_SIZE */

#define IS_SEG0_VA(x)	(((unsigned long)(x) & UNITY_BASE) == 0)
#define IS_SYS_VA(x)	(((unsigned long)(x) & UNITY_BASE) == UNITY_BASE)
#define IS_KSEG_VA(x)	(((unsigned long)(x) & SEG1_BASE) == UNITY_BASE)
#define IS_SEG1_VA(x)	(((unsigned long)(x) & SEG1_BASE) == SEG1_BASE)

#endif /* __LANGUAGE_C__ */

#define	SSIZE		CLSIZE		/* initial stack size/NBPG */
#define	SINCR		CLSIZE		/* increment of stack/NBPG */

#define UPAGECNT	CLSIZE		/* u-area size in pages */
#define KSTKPGS		CLSIZE		/* kernel stack size in pages */
#define	UPAGES		(UPAGECNT+KSTKPGS)

#define	KERNELSTACK	0xffffffffffff8000UL /* top of kernel boot stack */

/*
 * Some macros for units conversion
 */
/* Core clicks to segments and vice versa */
#define	ctos(x)	(x)
#define	stoc(x)	(x)

/* Core clicks to disk blocks */
#define	DBSHIFT		9		/* LOG2(Disk block size) */
#define	ctod(x)	((x)<<(PGSHIFT-DBSHIFT))
#define	dtoc(x)	((unsigned)(x)>>(PGSHIFT-DBSHIFT))
#define	dtob(x)	((x)<<DBSHIFT)

/* clicks to bytes */
#define	ctob(x)	((x)<<PGSHIFT)

/* bytes to clicks */
#define	btoc(x)	(((unsigned long)(x)+PGOFSET)>>PGSHIFT)

/*
 * Macros to decode processor status word.
 */
#define	USERMODE(ps)	(((ps) & PSL_CURMOD) == PSL_CURMOD)
#define	BASEPRI(ps)	(((ps) & PSL_IPL) <= PSL_IPL_LOW)

#define DELAY(n)	{ microdelay(n); }

#endif /* _MACHPARAM_H_ */
