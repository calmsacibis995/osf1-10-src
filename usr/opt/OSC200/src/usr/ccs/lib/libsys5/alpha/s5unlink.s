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
 *	@(#)$RCSfile: s5unlink.s,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1992/07/14 17:05:51 $
 */ 
/*
 * ALL RIGHTS RESERVED
 */

/*
 * s5unlink.s
 *
 *      Revision History:
 *
 * 12-Mar-91
 *      Initial Implementation.
 *
 */


#include <sys/habitat.h>
#include <alpha/regdef.h>
#include <alpha/asm.h>
#include <sys/sysv_syscall.h>


SYSCALL(unlink)
	RET
.end unlink
