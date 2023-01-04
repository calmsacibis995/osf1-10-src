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
 *      @(#)$RCSfile: habitat.h,v $ $Revision: 4.3.6.3 $ (DEC) $Date: 1993/10/18 22:27:03 $
 */ 

/*
 * Modification History
 *     27-Dec-91  Philip Cameron
 *		  habitats now start at 0
 *     31-Jun-91  Vipul Patel
 *		  Added habitat marking support
 *		  for S5 and RT habitats.
 *     06-Jun-91  Paula Long
 *                Added Habitat for Realtime.
 *     20-May-91  Vipul Patel
 *		  Initial implementation.
 */

#ifndef _HABITAT_H_
#define _HABITAT_H_

/* 
 * Macros for shifting and extracting the habitat and system 
 * call numbers
*/
#define HABITAT_SHIFT	24
#define HABITAT_MASK	0xff000000	/* mask for 8 bit habitat number */
#define HABITAT_LOW	0xff		/* mask for 8 bit habitat index  */

/*
 * OSF1 "Habitat"
 */
#define OSF1_HAB	"OSF1"
#define OSF1_HAB_NO	0x0
#define OSF1_BASE_PATH	"/usr/opt/osf1"

/* System V habitat */
/* used in constructing the load module */
#define SYSV_HAB	"SystemV"
#define SYSVV11		0x0101	/* 1.1 */
#define SYSVV11A	"1.1"
#define SVID2_HAB_NO	0x01000000
#define SYSV_HAB_NO	SVID2_HAB_NO	/* XXX AZ: this should go */
/* used in constructing the library */
#ifdef  H_S5
#define	HABITAT_INDEX	SVID2_HAB_NO
#define HABITAT_STD_CALL(x)	SYS_sysv_ ## x
#define HABITAT_NSTD_CALL(x)	SYS_sysv_/**/x
#define HABITAT_BASE	sysv_FIRST

#define	HABITAT_ID	"@(#) habitat_id = svid2_12:1991"
#define HABITAT_LABEL	_S5_habitat_id

#ifdef __LANGUAGE_C__
static char HABITAT_LABEL[]= HABITAT_ID;
#elif __LANGUAGE_ASSEMBLY__
#include <machine/m_habitat.h>
#endif

#endif	/* H_S5 */


/* RT Habitat */
/* used in constructing the load module */
#define RT_HAB		"RT"
#define RTV11		0x0101	/* 1.1 */
#define RTV11A		"1.1"
#define RT_HAB_NO	0x02000000
/* used in constructing the library */
#ifdef H_RT
#define HABITAT_INDEX	RT_HAB_NO
#define HABITAT_STD_CALL(x)     SYS_rt_ ## x
#define HABITAT_NSTD_CALL(x)    SYS_rt_/**/x
#define HABITAT_BASE    rt_FIRST

#define HABITAT_ID      "@(#) habitat_id = realtime_12:1991"
#define HABITAT_LABEL   _RT_habitat_id

#ifdef __LANGUAGE_C__
static char HABITAT_LABEL[]= HABITAT_ID;
#elif __LANGUAGE_ASSEMBLY__
#include <machine/m_habitat.h>
#endif

#endif /* H_RT */


/* SVID 3 habitat */
/* used in constructing the load module */
#define SVID3_HAB	"Svid3"
#define SVID3V11	0x0101	/* 1.1 */
#define SVID3V11A	"1.1"
#define SVID3_HAB_NO	0x03000000
/* used in constructing the library */
#ifdef  H_SVID3
#define	HABITAT_INDEX	SVID3_HAB_NO
#define HABITAT_STD_CALL(x)	SYS_svid_three_ ## x
#define HABITAT_NSTD_CALL(x)	SYS_svid_three_/**/x
#define HABITAT_BASE	svid_three_FIRST

#define	HABITAT_ID	"@(#) habitat_id = svid3_03:1992"
#define HABITAT_LABEL	_SVID3_habitat_id

#ifdef __LANGUAGE_C__
static char HABITAT_LABEL[]= HABITAT_ID;
#elif __LANGUAGE_ASSEMBLY__
#include <machine/m_habitat.h>
#endif

#endif	/* H_SVID3 */


/* SVR 4 habitat */
/* used in constructing the load module */
#define SVR4_HAB	"Svr4"
#define SVR4V11		0x0101	/* 1.1 */
#define SVR4V11A	"1.1"
#define SVR4_HAB_NO	0x04000000
/* used in constructing the library */
#ifdef  H_SVR4
#define	HABITAT_INDEX	SVR4_HAB_NO
#define HABITAT_STD_CALL(x)	SYS_svr_four_ ## x
#define HABITAT_NSTD_CALL(x)	SYS_svr_four_/**/x
#define HABITAT_BASE	svr_four_FIRST

#define	HABITAT_ID	"@(#) habitat_id = svr4_03:1992"
#define HABITAT_LABEL	_SVR4_habitat_id

#ifdef __LANGUAGE_C__
static char HABITAT_LABEL[]= HABITAT_ID;
#elif __LANGUAGE_ASSEMBLY__
#include <machine/m_habitat.h>
#endif

#endif	/* H_SVR4 */


/* SOE 2 habitat */
/* used in constructing the load module */
#define SOE2_HAB	"Soe2"
#define SOE2V11		0x0101	/* 1.1 */
#define SOE2V11A	"1.1"
#define SOE2_HAB_NO	0x05000000
/* used in constructing the library */
#ifdef  H_SOE2
#define	HABITAT_INDEX	SOE2_HAB_NO
#define HABITAT_STD_CALL(x)	SYS_soe_two_ ## x
#define HABITAT_NSTD_CALL(x)	SYS_soe_two_/**/x
#define HABITAT_BASE	soe_two_FIRST

#define	HABITAT_ID	"@(#) habitat_id = soe2_03:1992"
#define HABITAT_LABEL	_SOE2_habitat_id

#ifdef __LANGUAGE_C__
static char HABITAT_LABEL[]= HABITAT_ID;
#elif __LANGUAGE_ASSEMBLY__
#include <machine/m_habitat.h>
#endif

#endif	/* H_SOE2 */

/* MPSG habitat */
/* used in constructing the load module */
#define	MPSG_HAB      "Mpsg"
#define	MPSGV11       0x0101  /* 1.1 */
#define	MPSGV11A      "1.1"
#define	MPSG_HAB_NO   0x06000000
/* used in constructing the library */
#ifdef  H_MPSG
#define	HABITAT_INDEX   MPSG_HAB_NO
#define	HABITAT_STD_CALL(x)	SYS_mpsg_ ## x
#define	HABITAT_NSTD_CALL(x)	SYS_mpsg_/**/x
#define	HABITAT_BASE  mpsg_FIRST

#define	HABITAT_ID      "@(#) habitat_id = mpsg_03:1992"
#define	HABITAT_LABEL _MPSG_habitat_id

#ifdef __LANGUAGE_C__
static char HABITAT_LABEL[]= HABITAT_ID;
#elif __LANGUAGE_ASSEMBLY__
#include <machine/m_habitat.h>
#endif

#endif        /* H_MPSG */

/*
 * Abstractions for uswitch.h:
 * MAX_HAB_NO is used in conjunction with MIN_HAB_NO for range 
 * checking, so when new habitats are added MAX_HAB_NO must be updated.
*/
#define DEFAULT_HAB_NO	0
#define MIN_HAB_NO	0
#define MAX_HAB_NO	MPSG_HAB_NO
#define hbval(x)	(((x) >> HABITAT_SHIFT) & HABITAT_LOW)
#define syscval(x)	((x) & ~HABITAT_MASK)
#define mksysc(h, s)	((((h) & HABITAT_LOW) << HABITAT_SHIFT) | (s))

#endif /* _HABITAT_H_ */
