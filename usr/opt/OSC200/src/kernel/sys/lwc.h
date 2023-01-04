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
 * @(#)$RCSfile: lwc.h,v $ $Revision: 1.1.8.2 $ (DEC) $Date: 1993/04/05 01:26:28 $
 */
#ifndef	__LWC__
#define	__LWC__

#ifndef	ASSEMBLER

#ifndef	spllwc
#define	spllwc splextreme
#endif	/* !spllwc */

typedef vm_offset_t lwc_id_t;
typedef long lwc_pri_t;

struct lwc {
	struct lwc	*lwc_fl, *lwc_bl;	/* Forward and back links */
	unsigned short	lwc_ref;		/* References */
	unsigned short	lwc_flags;		/* None current */
	lwc_id_t	lwc_id;			/* Light weight context id */
	lwc_pri_t	lwc_pri;		/* Priority */
	void 		(*lwc_ctxt)();		/* Context */
						/* Must be at end of struct */
	long		lwc_cpu[1];		/* cpus active on */
};

typedef	struct lwc * lwc_t;

#define	LWC_NULL	(struct lwc *) 0
#define	LWC_ID_NULL	(lwc_id_t) 0

#endif /* !ASSEMBLER */

#ifdef	KERNEL
#ifndef	ASSMEBLER

extern lwc_id_t lwc_create(/* lwc_pri_t pri, void (*)() */);
extern int lwc_destroy(/* lwc_id_t id */);

#define LWC_PRI_VFS_UBC 1
#define LWC_PRI_VM_SWAP 2
#define LWC_PRI_SYSLOG 16
#define LWC_PRI_BINLOG 18
#define LWC_PRI_CPU    20

#endif	/* !ASSEMBLER */
#endif	/* KERNEL */

#endif /* !__LWC__ */
