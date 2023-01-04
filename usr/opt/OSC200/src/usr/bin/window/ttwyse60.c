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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: ttwyse60.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:12:20 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif

/*
 * 	 ttwyse60.c	3.1 10/6/87
 */

#include "ww.h"
#include "tt.h"

/*
 * Wyse-60
 *
 * Contributed by David Elliott of MIPS (mips!dce).
 */

/*
 * Copyright 1987 by David C. Elliott, MIPS Computer Systems.
 *
 * Unlimited redistribution allowed as long as this notice
 * is kept intact.
 */

#define G (WWM_GRP << WWC_MSHIFT)
short wyse60_frame[16] = {
	' ',	'6'|G,	':'|G,	'1'|G,
	'6'|G,	'6'|G,	'2'|G,	'4'|G,
	':'|G,	'5'|G,	':'|G,	'='|G,
	'3'|G,	'9'|G,	'0'|G,	'0'|G
};

extern struct tt_str *gen_AS;
extern struct tt_str *gen_AE;

tt_wyse60()
{
	static struct tt_str ae = { "\033H\003", 3 };
	static struct tt_str as = { "\033H\002", 3 };

	if (tt_generic() < 0)
		return -1;
	tt.tt_availmodes |= WWM_GRP;
	tt.tt_frame = wyse60_frame;
	if (gen_AS == 0)
		gen_AS = &as;
	if (gen_AE == 0)
		gen_AE = &ae;
	return 0;
}
