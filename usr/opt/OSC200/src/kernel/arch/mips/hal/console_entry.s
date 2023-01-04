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
 * @(#)$RCSfile: console_entry.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 15:05:56 $
 */

#include  <machine/regdef.h>
#include  <machine/asm.h>
#include  <machine/cpu.h>
#include  <hal/entrypt.h>

/*
 * define prom entrypoints of interest to kernel
 *
 * reset reboots as indicated by $bootmode and looks for warm start block
 */
EXPORT(prom_reset)
	li	v0,PROM_RESET
	j	v0

/*
 * halt
 */
EXPORT(prom_halt)
	li	v0,PROM_HALT
	j	v0

/*
 * autoboot always reboots regardless of $bootmode
 */
EXPORT(prom_autoboot)
	li	v0,PROM_AUTOBOOT
	j	v0

/*
 * reboot does action indicated by $bootmode
 */
EXPORT(prom_reboot)
	li	v0,PROM_REBOOT
	j	v0

/*
 * restart always enters prom command mode
 */
EXPORT(prom_restart)
	li	v0,PROM_RESTART
	j	v0

/*
 * prom flow-controlled console io
 */
EXPORT(prom_getchar)
	li	v0,PROM_GETCHAR
	j	v0

EXPORT(prom_putchar)
	li	v0,PROM_PUTCHAR
	j	v0

#ifdef notdef
/*
 * The next 6 are no longer in the PROM
 */
/*
 * prom read-modify-write routines
 */
EXPORT(andb_rmw)
	li	v0,PROM_ANDB_RMW
	j	v0

EXPORT(andh_rmw)
	li	v0,PROM_ANDH_RMW
	j	v0

EXPORT(andw_rmw)
	li	v0,PROM_ANDW_RMW
	j	v0

EXPORT(orb_rmw)
	li	v0,PROM_ORB_RMW
	j	v0

EXPORT(orh_rmw)
	li	v0,PROM_ORH_RMW
	j	v0

EXPORT(orw_rmw)
	li	v0,PROM_ORW_RMW
	j	v0
#endif /* notdef */

EXPORT(prom_exec)
	li	v0,PROM_EXEC
	j	v0

EXPORT(prom_getenv)
	lw	v0,rex_base
	bne	v0,zero,1f
	li	v0,PROM_GETENV
	j	v0
1:	j	rex_getenv

EXPORT(prom_setenv)
	li	v0,PROM_SETENV
	j	v0

EXPORT(prom_open)
	li	v0,PROM_OPEN
	j	v0

EXPORT(prom_close)
	li	v0,PROM_CLOSE
	j	v0

EXPORT(prom_lseek)
	li	v0,PROM_LSEEK
	j	v0

EXPORT(prom_read)
	li	v0,PROM_READ
	j	v0

EXPORT(prom_write)
	li	v0,PROM_WRITE
	j	v0


/*
 * ROM Executive Program (REX) callbacks (entry points)
 *
 * These are presently used only with 3MAX.  They are currently TURBOchannel
 * specific.
 */

/*
 * REX global variable
 */
	BSS(rex_base,4)			# REX base address.
	BSS(rex_magicid,4)		# REX base address.

EXPORT(rex_memcpy)
	lw	v0,rex_base
	lw	v0,REX_MEMCPY(v0)
	j	v0
/*
EXPORT(rex_memset)
	lw	v0,rex_base
	lw	v0,REX_MEMSET(v0)
	j	v0

EXPORT(rex_strcat)
	lw	v0,rex_base
	lw	v0,REX_STRCAT(v0)
	j	v0

EXPORT(rex_strcmp)
	lw	v0,rex_base
	lw	v0,REX_STRCMP(v0)
	j	v0

EXPORT(rex_strlen)
	lw	v0,rex_base
	lw	v0,REX_STRLEN(v0)
	j	v0

EXPORT(rex_strncat)
	lw	v0,rex_base
	lw	v0,REX_STRNCAT(v0)
	j	v0

EXPORT(rex_strncpy)
	lw	v0,rex_base
	lw	v0,REX_STRNCPY(v0)
	j	v0

EXPORT(rex_strncmy)
	lw	v0,rex_base
	lw	v0,REX_STRNCMY(v0)
	j	v0

EXPORT(rex_getchar)
	lw	v0,rex_base
	lw	v0,REX_GETCHAR(v0)
	j	v0
*/
EXPORT(rex_gets)
	lw	v0,rex_base
	lw	v0,REX_GETS(v0)
	j	v0
/*
EXPORT(rex_puts)
	lw	v0,rex_base
	lw	v0,REX_PUTS(v0)
	j	v0
*/
EXPORT(rex_printf)
	lw	v0,rex_base
	lw	v0,REX_PRINTF(v0)
	j	v0
/*
EXPORT(rex_sprintf)
	lw	v0,rex_base
	lw	v0,REX_SPRINTF(v0)
	j	v0

EXPORT(rex_io_poll)
	lw	v0,rex_base
	lw	v0,REX_IO_POLL(v0)
	j	v0

EXPORT(rex_strtol)
	lw	v0,rex_base
	lw	v0,REX_STRTOL(v0)
	j	v0

EXPORT(rex_signal)
	lw	v0,rex_base
	lw	v0,REX_SIGNAL(v0)
	j	v0

EXPORT(rex_raise)
	lw	v0,rex_base
	lw	v0,REX_RAISE(v0)
	j	v0

EXPORT(rex_time)
	lw	v0,rex_base
	lw	v0,REX_TIME(v0)
	j	v0

EXPORT(rex_setjump)
	lw	v0,rex_base
	lw	v0,REX_SETJUMP(v0)
	j	v0

EXPORT(rex_longjump)
	lw	v0,rex_base
	lw	v0,REX_LONGJUMP(v0)
	j	v0
*/
EXPORT(rex_bootinit)
	lw	v0,rex_base
	lw	v0,REX_BOOTINIT(v0)
	j	v0

EXPORT(rex_bootread)
	lw	v0,rex_base
	lw	v0,REX_BOOTREAD(v0)
	j	v0

EXPORT(rex_bootwrite)
	lw	v0,rex_base
	lw	v0,REX_BOOTWRITE(v0)
	j	v0
/*
EXPORT(rex_setenv)
	lw	v0,rex_base
	lw	v0,REX_SETENV(v0)
	j	v0
*/
EXPORT(rex_getenv)
	lw	v0,rex_base
	lw	v0,REX_GETENV(v0)
	j	v0
/*
EXPORT(rex_unsetenv)
	lw	v0,rex_base
	lw	v0,REX_UNSETENV(v0)
	j	v0

EXPORT(rex_slot_address)
	lw	v0,rex_base
	lw	v0,REX_SLOT_ADDRESS(v0)
	j	v0

EXPORT(rex_wbflush)
	lw	v0,rex_base
	lw	v0,REX_WBFLUSH(v0)
	j	v0

EXPORT(rex_msdelay)
	lw	v0,rex_base
	lw	v0,REX_MSDELAY(v0)
	j	v0

EXPORT(rex_leds)
	lw	v0,rex_base
	lw	v0,REX_LEDS(v0)
	j	v0

EXPORT(rex_clear_cache)
	lw	v0,rex_base
	lw	v0,REX_CLEAR_CACHE(v0)
	j	v0
*/
EXPORT(rex_getsystype)
	lw	v0,rex_base
	lw	v0,REX_GETSYSTYPE(v0)
	j	v0

EXPORT(rex_getbitmap)
	lw	v0,rex_base
	lw	v0,REX_GETBITMAP(v0)
	j	v0
/*
EXPORT(rex_disableintr)
	lw	v0,rex_base
	lw	v0,REX_DISABLEINTR(v0)
	j	v0

EXPORT(rex_enableintr)
	lw	v0,rex_base
	lw	v0,REX_ENABLEINTR(v0)
	j	v0

EXPORT(rex_testintr)
	lw	v0,rex_base
	lw	v0,REX_TESTINTR(v0)
	j	v0
*/
EXPORT(rex_console_init)
	lw	v0,rex_base
	lw	v0,REX_CONSOLE_INIT(v0)
	j	v0

EXPORT(rex_halt)
	lw	v0,rex_base
	lw	v0,REX_HALT(v0)
	j	v0
/*
EXPORT(rex_showfault)
	lw	v0,rex_base
	lw	v0,REX_SHOWFAULT(v0)
	j	v0

EXPORT(rex_gettcinfo)
	lw	v0,rex_base
	lw	v0,REX_GETTCINFO(v0)
	j	v0
*/
EXPORT(rex_execute_cmd)
	lw	v0,rex_base
	lw	v0,REX_EXECUTE_CMD(v0)
	j	v0

EXPORT(rex_rex)
	lw	v0,rex_base
	lw	v0,REX_REX(v0)
	j	v0

