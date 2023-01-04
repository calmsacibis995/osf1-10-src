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
 * This include is required to support DDI/DKI interfaces
 */
#ifndef _SYS_OPEN_H
#define _SYS_OPEN_H

#if	!defined(NO_SVR4_COMPAT)
#include <sys/mode.h>		/*S_IFBLK and S_IFCHR defined here*/

#define OTYP_MNT	0x10000000
#define OTYP_BLK	S_IFBLK
#define OTYP_CHAR	S_IFCHR
#define OTYP_LYR	0x00000001
#endif	/* !NO_SVR4_COMPAT */
	
#endif	/* _SYS_OPEN_H */
