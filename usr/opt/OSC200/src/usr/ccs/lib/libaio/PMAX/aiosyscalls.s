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
 * @(#)$RCSfile: aiosyscalls.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/06/03 17:23:54 $
 */

#include <machine/regdef.h>
#include <sys/habitat.h>
#include <machine/asm.h>
#include <sys/rt_syscall.h>

SYSCALL(aio_init)
        RET
.end	aio_int

SYSCALL(aio_transfer)
        RET
.end	aio_transfer

SYSCALL(aio_transfer_done)
        RET
.end	aio_transfer_done

SYSCALL(aio_wait)
        RET
.end	aio_wait

SYSCALL(aio_done)
        RET
.end	aio_done

SYSCALL(aio_info)
        RET
.end	aio_done
