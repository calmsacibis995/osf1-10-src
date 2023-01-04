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
 *	@(#)$RCSfile: prom_entry.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:38:45 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * Copyright 1985 by MIPS Computer Systems, Inc.
 *
 * prom_entry.s -- interface to prom entry points
 */

#include "../../regdef.h"
#include "../../asm.h"
#include "../../cpu.h"
#include "../../entrypt.h"

/*
 * Prom entry points
 */

/*
 * Return control to prom entry points
 *
 * RESET	transferred to on hardware reset, configures MIPS boards,
 *		runs diags, check for appropriate auto boot action in
 *		"bootmode" environment variable and performs that action.
 *
 * EXEC		called to utilize prom to boot new image.  After the booted
 *		program returns control can either be returned to the
 *		original caller of the exec routine or to the prom monitor.
 *		(to return to the original caller, the new program must
 *		not destroy any text, data, or stack of the parent.  the
 *		new programs stack continues on the parents stack.
 *
 * RESTART	re-enter the prom command parser, do not reset prom state
 *
 * REINIT	reinitialize prom state and re-enter the prom command parser
 *
 * REBOOT	check for appropriate bootmode and perform, no configuration
 *		or diags run
 *
*/
#if 0
LEAF(_prom_autoboot)
XLEAF(prom_autoboot)
	li	v0,PROM_AUTOBOOT
	j	v0
	END(_prom_autoboot)

LEAF(_prom_reset)
XLEAF(prom_reset)
	li	v0,PROM_RESET
	j	v0
	END(_prom_reset)

LEAF(_prom_exec)
XLEAF(prom_exec)
	li	v0,PROM_EXEC
	j	v0
	END(_prom_exec)
#endif

LEAF(_prom_restart)
XLEAF(prom_restart)
	li	v0,PROM_RESTART
	j	v0
	END(_prom_restart)

#if 0
LEAF(_prom_reinit)
XLEAF(prom_reinit)
	li	v0,PROM_REINIT
	j	v0
	END(_prom_reinit)

LEAF(_prom_reboot)
XLEAF(prom_reboot)
	li	v0,PROM_REBOOT
	j	v0
	END(_prom_reboot)
#endif
/*
 * these routines access prom "stdio" routines, and may be used
 * by standalone programs that would like to use prom io redirection
 */
LEAF(_prom_lseek)
	li	v0,PROM_LSEEK
	j	v0
	END(_prom_lseek)

#if 0
LEAF(_prom_getchar)
	li	v0,PROM_GETCHAR
	j	v0
	END(_prom_getchar)

LEAF(_prom_putchar)
	li	v0,PROM_PUTCHAR
	j	v0
	END(_prom_putchar)
#endif

LEAF(_prom_gets)
	li	v0,PROM_GETS
	j	v0
	END(_prom_gets)

#if 0
LEAF(_prom_puts)
	li	v0,PROM_PUTS
	j	v0
	END(_prom_puts)
#endif

LEAF(_prom_printf)
	li	v0,PROM_PRINTF
	j	v0
	END(_prom_printf)

LEAF(prom_getenv)
	li	v0,PROM_GETENV
	j	v0
	END(prom_getenv)
#if 0
LEAF(prom_setenv)
	li	v0,PROM_SETENV
	j	v0
	END(prom_setenv)

LEAF(prom_atob)
	li	v0,PROM_ATOB
	j	v0
	END(prom_atob)
#endif
LEAF(prom_strcmp)
	li	v0,PROM_STRCMP
	j	v0
	END(prom_strcmp)

LEAF(prom_strlen)
	li	v0,PROM_STRLEN
	j	v0
	END(prom_strlen)
#if 0
LEAF(prom_strcpy)
	li	v0,PROM_STRCPY
	j	v0
	END(prom_strcpy)

LEAF(prom_strcat)
	li	v0,PROM_STRCAT
	j	v0
	END(prom_strcat)
#endif
#if MSERIES
/*
 * read-modify-write routine use special cpu board circuitry to accomplish
 * vme bus r-m-w cycles.  all routines are similar to:
 *	unsigned char
 *	orb_rmw(addr, mask)
 *	unsigned char *addr;
 *	unsigned mask;
 *	{
 *		register unsigned rval;
 *
 *		lockbus();
 *		rval = *addr;
 *		*addr = rval & mask;
 *		unlockbus();
 *		return(rval);
 *	}
 */
LEAF(orw_rmw)
	li	v0,PROM_ORW_RMW
	j	v0
	END(orw_rmw)

LEAF(orh_rmw)
	li	v0,PROM_ORH_RMW
	j	v0
	END(orh_rmw)

LEAF(orb_rmw)
	li	v0,PROM_ORB_RMW
	j	v0
	END(orb_rmw)

LEAF(andw_rmw)
	li	v0,PROM_ANDW_RMW
	j	v0
	END(andw_rmw)

LEAF(andh_rmw)
	li	v0,PROM_ANDH_RMW
	j	v0
	END(andh_rmw)

LEAF(andb_rmw)
	li	v0,PROM_ANDB_RMW
	j	v0
	END(andb_rmw)
#endif

/*
 * prom saio entry points
 * (mainly for implementing stdin/stdout for standalones)
 */
LEAF(_prom_open)
	li	v0,PROM_OPEN
	j	v0
	END(_prom_open)

LEAF(_prom_read)
	li	v0,PROM_READ
	j	v0
	END(_prom_read)

LEAF(_prom_write)
	li	v0,PROM_WRITE
	j	v0
	END(_prom_write)
#if 0
LEAF(_prom_ioctl)
	li	v0,PROM_IOCTL
	j	v0
	END(_prom_ioctl)
#endif
LEAF(_prom_close)
	li	v0,PROM_CLOSE
	j	v0
	END(_prom_close)
