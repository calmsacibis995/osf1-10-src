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
 *	@(#)$RCSfile: dedevs.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:56:13 $
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
 * Copyright (c) 1990 SecureWare, Inc.
 *   All rights reserved
 *

 *
 * Common screen definitions for the device main screens
 */

/* screen description common to all screens */

#define TITLE_DESC		0
#define NAME_DESC		1
#define DEVLIST_TITLE_DESC	2
#define DEVLIST_DESC		3

#define NAME_STRUCT		0
#define DEVLIST_STRUCT		1

/* Descs and structs common to removable screens */

#if SEC_MAC
#define RMV_DEVLABEL_TITLE_DESC	4
#define RMV_SL_TITLE_DESC	5
#define RMV_DEVLABEL_UNDER_DESC	6
#define RMV_SL_UNDER_DESC	7
#define RMV_SINGLE_TITLE_DESC	8
#define RMV_SINGLE_SL_DESC	9
#define RMV_MULTI_TITLE_DESC	10
#define RMV_MULTI_SL_DESC	11
#define RMV_NEXT_DESC		12
#else
#define RMV_NEXT_DESC		4
#endif /* SEC_MAC */

#if SEC_ARCH
#define RMV_CTL_TITLE_DESC	RMV_NEXT_DESC
#define RMV_SET_DESC		RMV_CTL_TITLE_DESC + 1
#define RMV_CTL_UNDER_DESC	RMV_SET_DESC + 1
#define RMV_CTL_IMP_TITLE_DESC	RMV_CTL_UNDER_DESC + 1
#define RMV_CTL_IMP_TOG_DESC	RMV_CTL_IMP_TITLE_DESC + 1
#define RMV_CTL_EXP_TITLE_DESC	RMV_CTL_IMP_TOG_DESC + 1
#define RMV_CTL_EXP_TOG_DESC	RMV_CTL_IMP_TITLE_DESC + 1
#endif /* SEC_ARCH */

#if SEC_MAC
#define RMV_SINGLE_SL_STRUCT	2
#define RMV_MULTI_SL_STRUCT	3
#define RMV_NEXT_STRUCT		4
#else
#define RMV_NEXT_STRUCT		2
#endif /* SEC_MAC */

#if SEC_ARCH
#define RMV_CTL_IMP_TOG_STRUCT	RMV_NEXT_STRUCT
#define RMV_CTL_EXP_TOG_STRUCT	RMV_CTL_IMP_TOG_STRUCT + 1
#define RMV_NSCRNSTRUCT		RMV_CTL_EXP_TOG_STRUCT + 1
#else
#define RMV_NSCRNSTRUCT		RMV_NEXT_STRUCT
#endif /* SEC_ARCH */

/* Descs and structs common to terminal screens */

#define TRM_ULOG_TITLE_DESC	4
#define TRM_ULOG_VALUE_DESC	5
#define TRM_ULOG_DEFTOG_DESC	6
#define TRM_ULOG_DEFVAL_DESC	7
#define TRM_LOGTIME_TITLE_DESC	8
#define TRM_LOGTIME_VALUE_DESC	9
#define TRM_LOGTIME_DEFTOG_DESC	10
#define TRM_LOGTIME_DEFVAL_DESC 11
#define TRM_LOGDEL_TITLE_DESC	12
#define TRM_LOGDEL_VALUE_DESC	13
#define TRM_LOGDEL_DEFTOG_DESC	14
#define TRM_LOGDEL_DEFVAL_DESC	15

#if SEC_MAC
#define TRM_DEVLAB_TITLE_DESC	16
#define TRM_DEVLAB_UNDER_DESC	17
#define TRM_SL_TITLE_DESC	18
#define TRM_SL_UNDER_DESC	19
#define TRM_SINGLE_TITLE_DESC	20
#define TRM_SINGLE_SL_DESC	21
#define TRM_MULTI_TITLE_DESC	22
#define TRM_MULTI_SL_DESC	23
#endif /* SEC_MAC */

#define TRM_ULOG_VALUE_STRUCT		2
#define TRM_ULOG_DEFTOG_STRUCT		3
#define TRM_ULOG_DEFVAL_STRUCT		4
#define TRM_LOGTIME_VALUE_STRUCT	5
#define TRM_LOGTIME_DEFTOG_STRUCT	6
#define TRM_LOGTIME_DEFVAL_STRUCT	7
#define TRM_LOGDEL_VALUE_STRUCT		8
#define TRM_LOGDEL_DEFTOG_STRUCT	9
#define TRM_LOGDEL_DEFVAL_STRUCT	10

#if SEC_MAC
#define TRM_SINGLE_SL_STRUCT		11
#define TRM_MULTI_SL_STRUCT		12
#define TRM_NSCRNSTRUCT			13
#else
#define TRM_NSCRNSTRUCT			11
#endif /* SEC_MAC */

/* Descs and structs common to printer screens */

#if SEC_MAC
#define PTR_DEVLABEL_TITLE_DESC	4
#define PTR_SL_TITLE_DESC	5
#define PTR_DEVLABEL_UNDER_DESC	6
#define PTR_SL_UNDER_DESC	7
#define PTR_SINGLE_TITLE_DESC	8
#define PTR_SINGLE_SL_DESC	9
#define PTR_MULTI_TITLE_DESC	10
#define PTR_MULTI_SL_DESC	11
#endif /* SEC_MAC */

#if SEC_MAC
#define PTR_SINGLE_SL_STRUCT	2
#define PTR_MULTI_SL_STRUCT	3
#define PTR_NSCRNSTRUCT		4
#else /* SEC_MAC */
#define PTR_NSCRNSTRUCT		2
#endif /* SEC_MAC */

