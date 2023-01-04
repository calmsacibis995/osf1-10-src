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
static char	*sccsid = "@(#)$RCSfile: cpuconf.c,v $ $Revision: 1.2.3.5 $ (DEC) $Date: 1992/06/03 10:27:12 $";
#endif 
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
 * derived from cpuconf.c	4.14      (ULTRIX)  12/6/90";
 */


/***********************************************************************
 *
 * Modification History: cpuconf.c
 *
 * 10-May-91	Paul Grist
 *	Added support for 3max+/bigmax (DS_5000_300).
 *
 * 15-Oct-90	Randall Brown
 *	Added errlogging routines to cpusw for 3min.
 *
 * 09-Oct-90    jaw
 * 	merge in MM changes for rigel.
 *
 * 09-Oct-90    Paul Grist
 *      added startclock routine to cpuswitch for mipsmate, this
 *      fixes hangs after power-fails when the system needs to
 *      acess non-root disks, which are not spun up, the cases
 *      that were seen were swap on non-root and presto buffers.
 *
 * 01-Sep-90	sekhar
 *	added functions and stubs for print_consinfo interface.
 *      kn02_print_consinfo, kn02_log_errinfo 	- DS5000 (3MAX)
 *	kn220_print_consinfo, kn220_log_errinfo	- DS5500 (MIPSFAIR2)
 *	nullcpu stubs for other machines (both mips and vax).
 *
 * 21-Jun-90	Fred L. Templin
 *	Added dummies for TURBOchannel data structures for case of
 *	DS5000 not defines. (Solution from afd).
 *
 * 20-Mar-90    Paul Grist
 *      Added MIPSMATE support (DS_5100).
 *
 * 30-Apr-90	Randall Brown
 *	Added new cpu entry for DS_5000_100.  Filled in the new values
 *	of the switch table for the MIPS systems. ( spls, intr, clock stuff)
 *
 * 06-Mar-90	afd
 *	Put mc146818startclocks() into cpu switch for pmax/3max.
 *
 * 18-Jan-90	robin
 *	Added kn220badaddr function to get badaddr to work with the
 *	way the KN220 memory intrrupts are cleared.
 *
 * 29-Dec-89	afd
 *	Added definitions for kn02erradr & kn02chksyn for when
 *	DS5000 not defined.
 *
 * 26-Dec-89	robin
 *	changed the kn220 write buffer routine used by 5500.
 *
 * 14-Nov-89	gmm
 *	Remove kn5800_iscpu() and add kn5800_init_secondary().
 *
 * 30-Oct-89	afd
 *	Use kn01 cache flush routines for kn02 (DS5000 - 3max).
 *
 * 11-Aug-89	afd
 *	Set up 3MAX cpu struct in cpu switch table.
 *	
 * 10-Jul-89	burns
 *	For DS5800 moved several "vax" only fields into mips. Added
 *	the msize and cpuinit routines for afd. Added cache routines
 *	to the cpu switch for DS5800.
 *
 * 14-June-89	afd
 *	Fill in new HZ field in cpuswitch (used to be in param.c).
 *	hz, tick, tickadj are set in processor specific init routines.
 *
 **********************************************************************/


#include <sys/types.h>
#include <hal/cpuconf.h>
#include <machine/cpu.h>
#include <hal/kn5800.h>

int	nocpu();
int	nullcpu();
int 	bbadaddr();
int	readtodr();
int 	writetodr();
int 	uSSCdelay();
int	ssc_readtodr();
int	ssc_writetodr();

char    UNKNOWN_SYS_STRING[] = "Unknown system type";

#ifdef DS3100
/* We are linking with kn01.o */
extern int	kn01conf();
extern int	kn01trap_error();
extern int	kn01memintr();
extern int	kn_flush_cache();
extern int	kn_clean_icache();
extern int	kn_clean_dcache();
extern int	kn_page_iflush();
extern int	kn_page_dflush();
#ifdef HAL_LIBRARY
extern int	kn01wbflush();
extern int	kn_getspl();
extern int	kn_whatspl();
extern int	kn_delay();
#endif /* HAL_LIBRARY */
extern int	chk_cpe();
extern int	mc146818read_todclk();
extern int	mc146818write_todclk();
extern int	mc146818startclocks();
extern int	mc146818stopclocks();
extern int	kn01init();
extern int	msize_baddr();
extern int	wbadmemaddr();
extern int	kn01_intr();
char    DS_3100_STRING[] = "DECstation/DECsystem 2100/3100";
#else
/* Resolve dangling references */
int	parityerr;	
pmopen() {}
pmclose() {}
pmstop() {}
pmioctl() {}
kn01config_devices() {}
#endif /* DS3100 */


#ifdef DS5100
/* We are linking with kn230.o */
extern int	kn230_conf();
extern int	kn230_trap_error();
extern int	kn230_memintr();
extern int	kn_flush_cache();
extern int	kn_clean_icache();
extern int	kn_clean_dcache();
extern int	kn_page_iflush();
extern int	kn_page_dflush();
#ifdef HAL_LIBRARY
extern int	kn210wbflush();
extern int	kn_getspl();
extern int	kn_whatspl();
extern int	kn_delay();
#endif /* HAL_LIBRARY */
extern int	chk_cpe();
extern int	mc146818read_todclk();
extern int	mc146818write_todclk();
extern int	mc146818startclocks();
extern int	mc146818stopclocks();
extern int	kn230_init();
extern int	msize_bitmap();
extern int	wbadmemaddr();
extern int	kn01_intr();
char	DS_5100_STRING[] = "DECstation 5100";
#else
/* Resolve dangling references */
kn230_config_devices() {}
#endif	/* DS5100 */

#ifdef DS5400
/* We are linking with kn210.o */
extern int	chk_cpe();
extern int	kn210conf();
#ifdef HAL_LIBRARY
extern int	kn210wbflush();
extern int	kn_getspl();
extern int	kn_whatspl();
extern int	uSSCdelay();
#endif /* HAL_LIBRARY */
extern int	kn210trap_error();
extern int	kn210harderrintr();
extern int	kn_flush_cache();
extern int	kn_clean_icache();
extern int	kn_clean_dcache();
extern int	kn_page_iflush();
extern int	kn_page_dflush();
extern int	ssc_readtodr();
extern int	ssc_writetodr();
extern int	kn210startrtclock();
extern int	kn210stopclocks();
extern int	bbadaddr();
extern int	msize_baddr();
extern int	kn210init();
extern int	kn01_intr();
char    DS_5400_STRING[] = "DECsystem 5400";
#else
/* Add the stubs for dangling references */
int kn210hardintr0() {}
int kn210hardintr1()	{}
int kn210hardintr2() {}
int kn210hardintr3()	{}
int kn210harderrintr() {}
int kn210haltintr() {}
#endif /* DS5400 */

#ifdef	DS5800
/* We are linking with kn5800.o */
extern int	chk_cpe();
extern int	kn5800_conf();
extern int	kn5800_flush_cache();
extern int	kn5800_clean_icache();
extern int	kn5800_clean_dcache();
extern int	kn5800_page_iflush();
extern int	kn5800_page_dflush();
extern int	kn5800_enable_cache();
extern int	kn5800badaddr();
extern int	kn5800_intr3();
extern int	kn5800_trap_error();
#ifdef HAL_LIBRARY
extern int	kn5800_wbflush();
extern int	kn_getspl();
extern int	kn_whatspl();
extern int	uSSCdelay();
#endif /* HAL_LIBRARY */
extern int	kn5800flush_cache();
extern int	kn5800_start_clock();
extern int	kn5800_stop_clock();
extern int	reprime_ssc_clock();
extern int	kn5800nexaddr();
extern int	kn5800udevaddr();
extern int	kn5800umaddr();
extern int	msize_bitmap();
extern int	kn5800_init();
extern int	cca_startcpu();
extern int	kn01_intr();
extern int	kn5800_init_secondary();
char    DS_5800_STRING[] = "DECsystem 5800 Series";
#else
struct	v5800csr *v5800csr;
int	*kn5800_wbflush_addr;
int	wbflush_dummy;
nxaccess() {}
kn5800_cpuid() {};
bbcci() {};
bbssi() {};
kn5800_init_secondary() {};
#endif	/* DS5800 */

#ifdef DS5000
/* We are linking with kn02.o */
extern int	kn02conf();
extern int	kn02trap_error();
extern int	kn02errintr();
extern int	kn_flush_cache();
extern int	kn_clean_icache();
extern int	kn_clean_dcache();
extern int	kn_page_iflush();
extern int	kn_page_dflush();
#ifdef HAL_LIBRARY
extern int	kn02wbflush();
extern int	kn_getspl();
extern int	kn_whatspl();
extern int	kn_delay();
#endif /* HAL_LIBRARY */
extern int	mc146818read_todclk();
extern int	mc146818write_todclk();
extern int	mc146818startclocks();
extern int	mc146818stopclocks();
extern int	msize_bitmap();
extern int	kn02init();
extern int	bbadaddr();
extern int	chk_cpe();
extern int	kn01_intr();
extern int	kn02_print_consinfo();
extern int	kn02_log_errinfo();
char    DS_5000_STRING[] = "DECstation/DECsystem 5000 Model 200";
#else
int kn02erradr;
int kn02chksyn;
#endif /* DS5000 */

#ifdef DS5500
/* We are linking with kn220.o */
extern int	chk_cpe();
extern int	kn220conf();
#ifdef HAL_LIBRARY
extern int	kn220wbflush();
extern int	kn_getspl();
extern int	kn_whatspl();
extern int	uSSCdelay();
#endif /* HAL_LIBRARY */
extern int	kn220trap_error();
extern int	kn220memintr();
extern int	kn_flush_cache();
extern int	kn_clean_icache();
extern int	kn_clean_dcache();
extern int	kn_page_iflush();
extern int	kn_page_dflush();
extern int	ssc_readtodr();
extern int	ssc_writetodr();
extern int	kn220startrtclock();
extern int	kn220stopclocks();
extern int	kn220badaddr();
extern int	msize_bitmap();
extern int	kn220init();
extern int	kn01_intr();
extern int	kn220_print_consinfo();
extern int	kn220_log_errinfo();
char    DS_5500_STRING[] = "DECsystem 5500";
#else
/* Add the stubs for dangling references */
int kn220hardintr0() {}
int kn220hardintr1()	{}
int kn220hardintr2() {}
int kn220hardintr3()	{}
int kn220memintr() {}
int kn220haltintr() {}
int kn220badaddr() {}
int kn220config_devices() {}
#endif /* DS5500 */

#ifdef DS5000_100
/* We are linking with kn02ba.o */
extern int	kn02ba_conf();
extern int	kn02ba_trap_error();
extern int	kn02ba_errintr();
extern int	kn_flush_cache();
extern int	kn_clean_icache();
extern int	kn_clean_dcache();
extern int	kn_page_iflush();
extern int	kn_page_dflush();
extern int	kn02ba_print_consinfo();
extern int	kn02ba_log_errinfo();
#ifdef HAL_LIBRARY
extern int	kn02ba_wbflush();
extern int	kn02ba_getspl();
extern int	kn02ba_whatspl();
extern int	kn_delay();
#endif /* HAL_LIBRARY */
extern int	mc146818read_todclk();
extern int	mc146818write_todclk();
extern int	mc146818startclocks();
extern int	mc146818stopclocks();
extern int	msize_bitmap();
extern int	kn02ba_init();
extern int	bbadaddr();
extern int	chk_cpe();
extern int	kn02ba_intr();
char    DS_5000_100_STRING[] = "DECstation/DECsystem 5000 Model 1xx";
#else
int kn02ba_exception_exit()	{}
int	ipllevel;
int	kn02ba_sim[SPLMSIZE];
int	kn02ba_intr()		{}
#endif /* DS5000_100 */

#ifdef DSPERSONAL_DECSTATION
/* We are linking with kn02ca.o */
extern int	kn02ca_conf();
extern int	kn02ca_trap_error();
extern int	kn02ca_errintr();
extern int	kn_flush_cache();
extern int	kn_clean_icache();
extern int	kn_clean_dcache();
extern int	kn_page_iflush();
extern int	kn_page_dflush();
extern int	kn02ca_print_consinfo();
extern int	kn02ca_log_errinfo();
#ifdef HAL_LIBRARY
extern int	kn02ca_wbflush();
extern int	kn_getspl();
extern int	kn_whatspl();
extern int	kn_delay();
#endif /* HAL_LIBRARY */
extern int	mc146818read_todclk();
extern int	mc146818write_todclk();
extern int	mc146818startclocks();
extern int	mc146818stopclocks();
extern int	msize_bitmap();
extern int	kn02ca_init();
extern int	bbadaddr();
extern int	chk_cpe();
extern int	kn02ca_iointr();
char	DS_MAXINE_STRING[] = "Personal DECstation Model xx";
#else
int kn02ca_exception_exit()	{}
#endif /* DSPERSONAL_DECSTATION */

#ifdef DS5000_300
/* We are linking with kn03.o */
extern int	kn03_conf();
extern int	kn03_trap_error();
extern int	kn03_errintr();
extern int	kn_flush_cache();
extern int	kn_clean_icache();
extern int	kn_clean_dcache();
extern int	kn_page_iflush();
extern int	kn_page_dflush();
#ifdef HAL_LIBRARY
extern int	kn03_wbflush();
extern int	kn_getspl();
extern int	kn_whatspl();
extern int      kn_delay();
#endif /* HAL_LIBRARY */
extern int	mc146818read_todclk();
extern int	mc146818write_todclk();
extern int	mc146818startclocks();
extern int	mc146818stopclocks();
extern int	msize_bitmap();
extern int	kn03_init();
extern int	bbadaddr();
extern int	chk_cpe();
extern int      kn03_print_consinfo();
extern int      kn03_log_errinfo();
char    DS_5000_300_STRING[] = "DECstation/DECsystem 5000 Model 2xx";
#else
int kn03erradr;
int kn03chksyn;
int get_kn03_subtype() {}
#endif /* DS5000_300 */

/* if - else - endif - Add machine support to proper area */


struct cpusw	cpusw[] =
{
#ifdef DS3100
    {	DS_3100,		kn01trap_error,		kn01memintr,
	nullcpu,		chk_cpe,		nullcpu,
	nullcpu,		kn01conf,		nullcpu,
	nullcpu,		kn_flush_cache,		wbadmemaddr,
	mc146818read_todclk,	mc146818write_todclk,	
#ifdef HAL_LIBRARY
	                                                kn_delay,
#else
	                                                nocpu,
#endif /* HAL_LIBRARY */
	nullcpu,		nullcpu,		nullcpu,
	nullcpu,		nullcpu,		nullcpu,
	nullcpu,		nullcpu,		nullcpu,
	nullcpu,		nullcpu,		nullcpu,
#ifdef HAL_LIBRARY
kn01wbflush,
#else
nocpu,
#endif /* HAL_LIBRARY */
				mc146818startclocks,	mc146818stopclocks,
	kn01init,		msize_baddr,		kn_clean_icache,
	kn_clean_dcache,	kn_page_iflush,	kn_page_dflush,
	kn01_intr,		
#ifdef HAL_LIBRARY
	                        kn_getspl,		kn_whatspl,
#else
	                        nocpu,			nocpu,
#endif
	0,			0,			20000,
	DS_3100_STRING,		256,			(1 << 26),
	1,			0 },
#endif /* DS3100 */



#ifdef DS5100
    {	DS_5100,		kn230_trap_error,	kn230_memintr,
	nullcpu,		chk_cpe,		nullcpu,
	nullcpu,		kn230_conf,		nullcpu,
	nullcpu,		kn_flush_cache,		wbadmemaddr,
	mc146818read_todclk,	mc146818write_todclk,	
#ifdef HAL_LIBRARY
	                                                kn_delay,
#else
	                                                nocpu,
#endif /* HAL_LIBRARY */
	nullcpu,		nullcpu,		nullcpu,
	nullcpu,		nullcpu,		nullcpu,
	nullcpu,		nullcpu,		nullcpu,
	nullcpu,		nullcpu,		nullcpu,
#ifdef HAL_LIBRARY
kn210wbflush,
#else
nocpu,
#endif /* HAL_LIBRARY */
				mc146818startclocks,	mc146818stopclocks,
	kn230_init,		msize_bitmap,		kn_clean_icache,
	kn_clean_dcache,	kn_page_iflush,	kn_page_dflush,
	kn01_intr,		
#ifdef HAL_LIBRARY
	                        kn_getspl,		kn_whatspl,
#else
	                        nocpu,			nocpu,
#endif
	0,			0,			20000,
	DS_5100_STRING,		256,			(1 << 26),
	1,			0 },
#endif /* DS5100 */



#ifdef DS5400
#define DS5400_FLAGS	(SCS_START_SYSAPS | MSCP_POLL_WAIT)
    {	DS_5400,		kn210trap_error,	kn210harderrintr,
	nullcpu,		chk_cpe,		nullcpu,
	nullcpu,		kn210conf,		nullcpu,
	nullcpu,		kn_flush_cache,		bbadaddr,
	ssc_readtodr,		ssc_writetodr,		uSSCdelay,
	nullcpu,		nullcpu,		nullcpu,
	nullcpu,		nullcpu,		nullcpu,
	nullcpu,		nullcpu,		nullcpu,
	nullcpu,		nullcpu,		nullcpu,
#ifdef HAL_LIBRARY
kn210wbflush,
#else
nocpu,
#endif /* HAL_LIBRARY */
				kn210startrtclock,	kn210stopclocks,
	kn210init,		msize_baddr,		kn_clean_icache,
	kn_clean_dcache,	kn_page_iflush,	kn_page_dflush,
	kn01_intr,		
#ifdef HAL_LIBRARY
	                        kn_getspl,		kn_whatspl,
#else
	                        nocpu,			nocpu,
#endif
	0,			0,			50000,
	DS_5400_STRING,		100,			(1 << 28),
	100,			DS5400_FLAGS },
#endif /* DS5400 */



#ifdef DS5800
#define DS5800_FLAGS	(SCS_START_SYSAPS | MSCP_POLL_WAIT)
    {	DS_5800,		kn5800_trap_error,	kn5800_intr3,
	nullcpu,		chk_cpe,		nullcpu,
	nullcpu,		kn5800_conf,		kn5800_enable_cache,
	nullcpu,		kn5800_flush_cache,	kn5800badaddr,
	ssc_readtodr,		ssc_writetodr,		uSSCdelay,
	nullcpu,		nullcpu,		nullcpu,
	nullcpu,		cca_startcpu,		nullcpu,
	kn5800nexaddr,		kn5800umaddr,		kn5800udevaddr,
	nullcpu,		nullcpu,		nullcpu,
	kn5800_wbflush,		kn5800_start_clock,	kn5800_stop_clock,
/*	reprime_ssc_clock,*/	kn5800_init,		msize_bitmap,
	kn5800_clean_icache,	kn5800_clean_dcache,	kn5800_page_iflush,
	kn5800_page_dflush,	kn01_intr,		kn_getspl,
	kn_whatspl,		0,			0,
	10000,			DS_5800_STRING,		100,
	(1 << 28),		100,			DS5800_FLAGS },
#endif /* DS5800 */



#ifdef DS5000
#define DS5000_FLAGS	(SCS_START_SYSAPS | MSCP_POLL_WAIT | CONSINIT_OK)
    {	DS_5000,		kn02trap_error,		kn02errintr,
	nullcpu,		chk_cpe,		nullcpu,
	nullcpu,		kn02conf,		nullcpu,
	nullcpu,		kn_flush_cache,		bbadaddr,
	mc146818read_todclk,	mc146818write_todclk,	
#ifdef HAL_LIBRARY
	                                                kn_delay,
#else
	                                                nocpu,
#endif /* HAL_LIBRARY */
	nullcpu,		nullcpu,		nullcpu,
	nullcpu,		nullcpu,		nullcpu,
	nullcpu,		nullcpu,		nullcpu,
	kn02_print_consinfo,	kn02_log_errinfo,	nullcpu,
#ifdef HAL_LIBRARY
kn02wbflush,
#else
nocpu,
#endif /* HAL_LIBRARY */
				mc146818startclocks,	mc146818stopclocks,
	kn02init,		msize_bitmap,		kn_clean_icache,
	kn_clean_dcache,	kn_page_iflush,	kn_page_dflush,
	kn01_intr,		
#ifdef HAL_LIBRARY
	                        kn_getspl,		kn_whatspl,
#else
	                        nocpu,			nocpu,
#endif
	0,			0,			20000,
	DS_5000_STRING,		256,			(1 << 26),
	1,			DS5000_FLAGS },
#endif /* DS5000 */

 

#ifdef DS5500
#define DS5500_FLAGS	(SCS_START_SYSAPS | MSCP_POLL_WAIT)
    {	DS_5500,		kn220trap_error,	kn220memintr,
	nullcpu,		chk_cpe,		nullcpu,
	nullcpu,		kn220conf,		nullcpu,
	nullcpu,		kn_flush_cache,	kn220badaddr,
	ssc_readtodr,		ssc_writetodr,
#ifdef HAL_LIBRARY
							uSSCdelay,
#else
	                                                nocpu,
#endif /* HAL_LIBRARY */
	nullcpu,		nullcpu,		nullcpu,
	nullcpu,		nullcpu,		nullcpu,
	nullcpu,		nullcpu,		nullcpu,
	kn220_print_consinfo,	kn220_log_errinfo,	nullcpu,
#ifdef HAL_LIBRARY
kn220wbflush,
#else
nocpu,
#endif /* HAL_LIBRARY */
				kn220startrtclock,	kn220stopclocks,
	kn220init,		msize_bitmap,		kn_clean_icache,
	kn_clean_dcache,	kn_page_iflush,	kn_page_dflush,
	kn01_intr,		
#ifdef HAL_LIBRARY
	                        kn_getspl,		kn_whatspl,
#else
	                        nocpu,			nocpu,
#endif
	0,			0,			50000,
	DS_5500_STRING,		100,			(1 << 28),
	100,			DS5500_FLAGS },
#endif /* DS5500 */



#ifdef DS5000_100
#define DS5000_100_FLAGS	(SCS_START_SYSAPS | MSCP_POLL_WAIT)
    {	DS_5000_100,		kn02ba_trap_error,	kn02ba_errintr,
	nullcpu,		chk_cpe,		nullcpu,
	nullcpu,		kn02ba_conf,		nullcpu,
	nullcpu,		kn_flush_cache,		bbadaddr,
	mc146818read_todclk,	mc146818write_todclk,	
#ifdef HAL_LIBRARY
	                                                kn_delay,
#else
	                                                nocpu,
#endif /* HAL_LIBRARY */
	nullcpu,		nullcpu,		nullcpu,
	nullcpu,		nullcpu,		nullcpu,
	nullcpu,		nullcpu,		nullcpu,
	kn02ba_print_consinfo,	kn02ba_log_errinfo,	nullcpu,
#ifdef HAL_LIBRARY
kn02ba_wbflush,
#else
nocpu,
#endif /* HAL_LIBRARY */
				mc146818startclocks,	mc146818stopclocks,
	kn02ba_init,		msize_bitmap,		kn_clean_icache,
	kn_clean_dcache,	kn_page_iflush,	kn_page_dflush,
	kn02ba_intr,		
#ifdef HAL_LIBRARY
	                        kn02ba_getspl,		kn02ba_whatspl,
#else
	                        nocpu,			nocpu,
#endif
	0,			0,			20000,
	DS_5000_100_STRING,	256,			(1 << 26),
	1,			DS5000_100_FLAGS },
#endif /* DS5000_100 */



#ifdef DS5000_300
#define DS5000_300_FLAGS	(SCS_START_SYSAPS | MSCP_POLL_WAIT)
    {	DS_5000_300,		kn03_trap_error,	kn03_errintr,
	nullcpu,		chk_cpe,		nullcpu,
	nullcpu,		kn03_conf,		nullcpu,
	nullcpu,		kn_flush_cache,		bbadaddr,
	mc146818read_todclk,	mc146818write_todclk,	
#ifdef HAL_LIBRARY
	                                                kn_delay,
#else
	                                                nocpu,
#endif /* HAL_LIBRARY */
	nullcpu,		nullcpu,		nullcpu,
	nullcpu,		nullcpu,		nullcpu,
	nullcpu,		nullcpu,		nullcpu,
	kn03_print_consinfo,	kn03_log_errinfo,	nullcpu,
#ifdef HAL_LIBRARY
kn03_wbflush,
#else
nocpu,
#endif /* HAL_LIBRARY */
				mc146818startclocks,	mc146818stopclocks,
	kn03_init,		msize_bitmap,		kn_clean_icache,
	kn_clean_dcache,	kn_page_iflush,	kn_page_dflush,
	nullcpu,		
#ifdef HAL_LIBRARY
	                        kn_getspl,		kn_whatspl,
#else
	                        nocpu,			nocpu,
#endif
	0,			0,			20000,
	DS_5000_300_STRING,	256,			(1 << 26),
	1,			DS5000_300_FLAGS },
#endif /* DS5000_300 */



#ifdef DSPERSONAL_DECSTATION
#define DSPERSONAL_DECSTATION_FLAGS   (SCS_START_SYSAPS | MSCP_POLL_WAIT)
    {	DS_MAXINE,		kn02ca_trap_error,	kn02ca_errintr,
	nullcpu,		chk_cpe,		nullcpu,
	nullcpu,		kn02ca_conf,		nullcpu,
	nullcpu,		kn_flush_cache,		bbadaddr,
	mc146818read_todclk,	mc146818write_todclk,	
#ifdef HAL_LIBRARY
	                                                kn_delay,
#else
	                                                nocpu,
#endif /* HAL_LIBRARY */
	nullcpu,		nullcpu,		nullcpu,
	nullcpu,		nullcpu,		nullcpu,
	nullcpu,		nullcpu,		nullcpu,
	kn02ca_print_consinfo,	kn02ca_log_errinfo,	nullcpu,
#ifdef HAL_LIBRARY
kn02ca_wbflush,	
#else
nocpu,
#endif /* HAL_LIBRARY */
				mc146818startclocks,	mc146818stopclocks,
	kn02ca_init,		msize_bitmap,		kn_clean_icache,
	kn_clean_dcache,	kn_page_iflush,	kn_page_dflush,
	nullcpu,		
#ifdef HAL_LIBRARY
	                        kn_getspl,		kn_whatspl,
#else
	                        nocpu,			nocpu,
#endif
	0,			0,			20000,
	DS_MAXINE_STRING,	256,			(1 << 26),
	1,			DSPERSONAL_DECSTATION_FLAGS },
#endif /* DSPERSONAL_DECSTATION */

	/*
	 * We have to be able to find the end of the table
	 */
    {	0,			0,			0,
	0,			0,			0,
	0,			0,			0,
	0,			0,			0,
	0,			0,			0,
	0,			0,			0,
	0,			0,			0,
	0,			0,			0,
	0,			0,			0,
	0 }
};


extern struct cpusw *cpup;

/*
 * The following was added for network management software.
 * Purpose is to provide a friendly system id string.
 */
char *
get_system_type_string ()
{
    return (cpup->system_string);
}

/*
 * When this routine is called, we are doing something wrong.
 */
nocpu()
{
	panic("nocpu");
}

/*
 * null routine to pass back a success since this cpu type
 * doesn't need one of these routines.
 */
nullcpu()
{
	return(0);
}


/******************************************
 * 
 * Moved from machdep.c; used to set up
 * the spl's for 3min.  Need it in a 
 * common/generic (C) module, so this 
 * is a good place for it, esp. since it
 * is unique to DEC MIPS machines.
 *
 *****************************************/
 
int mips_spl_arch_type =0;

/********************************************
 * 
 * Moved from machdep.c; good home for now;
 * DSA support must decide its long term need.
 *
 ********************************************/
u_long ci_ucode = 0;
