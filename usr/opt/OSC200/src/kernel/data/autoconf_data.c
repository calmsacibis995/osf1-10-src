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
 *	@(#)$RCSfile: autoconf_data.c,v $ $Revision: 1.2.10.2 $ (DEC) $Date: 1993/04/01 20:03:35 $
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
 * derived from autoconf_data.c	2.13	(ULTRIX)	12/28/89
 */
/************************************************************************
 *
 *			Modification History
 *
 * 28-Dec-89 Robin
 *	Added nNKDM nNKLESIB nNMBA so unifind can know if the bus is there
 *	when its called.  If unifind is called now and no devices are
 *	configed on the bus (no bus) then the system crashes.
 *
 * 14-Oct_89 Robin
 *	The cpu.h include needs the types.h include before its used.
 *	This include of types.h should be in cpu.h but I seem to
 *	remember a "rule (?)" about no nested includes to help
 *	make depend run, so I'll put it here.  XXX
 *
 * 13-Oct-1989 gmm
 *	Moved the include location of cpu.h before cpudata.h. Needed for
 *	MIPS smp support
 *
 * 08-June-1989	Robin
 *	added a stub routine for uqdriver so that it would be
 *	defined if nothing causes the uqdriver to be in the system.  This
 *	is needed in machdep gendump routine to allow dumps on Q-bus
 *	devices.  Also added #if on uba_hd structure declaration to make
 *	it a size of one if all the uba devices evaluste to zero.  It
 *	was causing a complie warning on array uba_hd[0,0,0]; no wonder!
 *
 * 20-Jul-89	rafiey (Ali Rafieymehr)
 *	included two stub routines (xmisst(), and get_xmi()).
 *
 * 20-Jul-1989  map (Mark A. Parenti)
 *	Include number of KDM70's when sizing uba structures.
 *
 * 24-May-1989	darrell
 *	changed the #include to find cpuconf.h in it's new location -- 
 *	sys/machine/common/cpuconf.h
 *
 * 24-Mar-1989  Pete Keilty
 * 	Added msi interrupt routine for mips.
 *
 * 21-Mar-1989  Pete Keilty
 *	Added ci interrupt routines for mips. Also added ifdef vax
 * 	around mba.h
 *
 * 30-jan-1989	jaw
 * 	cleanup of SMP per-cpu data.
 *
 * 09-Dec-1988	Todd M. Katz			TMK0002
 *	1. Changed MSI defines completely:
 *		1) The variable nummsi is always defined and declared.
 *		2) Dummy routines are never defined( their definitions have
 *	   	   been moved to conf.c for consistentcy ).
 *		3) Define and declare nNMSI, msi_adapt[], and msiintv[] only
 *		   when local MSI ports have been configured.
 *	2. Currently only one CI port is supported.  Change ciintv[] to reflect
 *	   this and rename the appropriate locore interrupt service routine
 *	   from Xcia0int() -> Xci0int().
 *
 * 20-Apr-1988  Ricky Palmer
 *	Added an omitted dummy "msiintr" routine to MSI defines.
 *
 * 24-Mar-1988	Robin
 *	Added code to protect locore from causing undefines in processors that
 *	do not config in the NI or MSI drivers.  Locore calls the interupt
 *	routines and if they are not there the stub in here is used.  Also
 *	the interupt for a NI on a ka640 needs to go to STRAY if the ka640
 *	does not config in the device and that is also done here.
 *
 * 15-Feb-1988	Fred Canter
 *	Added VAX420 (CVAXstar/PVAX) to CPUs needing emulation code.
 *	Also added VAX3600 & VAX6200, which were missing.
 *
 * 08-Jan-1988	Todd M. Katz			TMK0001
 *	Added the data variable scs_disable.  This variable is initialized
 *	by SCS to contain the address of the SCS shutdown routine.  Otherwise,
 *	it is referenced only during panics when the specified shutdown
 *	routine is invoked to permanently disable all local system ports.
 *
 * 12-11-87	Robin L. and Larry C.
 *	Added portclass support to the system.
 *
 * 12-Aug-86  -- prs	Removed isGENERIC option.
 *
 * 3-Aug-86   -- jaw 	allocate a uba struct for klesib
 *
 * 5-Jun-86   -- jaw 	changes to config.
 *
 * 14-Apr-86 -- jaw
 *	remove MAXNUBA referances.....use NUBA only!
 *
 * 18-Mar-86 -- jaw  add routines to cpu switch for nexus/unibus addreses
 *		     also got rid of some globals like nexnum.
 *		     ka8800 cleanup.
 *
 *	Stephen Reilly, 22-Mar-85
 *	Added new structures for the floating emulation stuff
 *
 ***********************************************************************/

#include "ci.h"
#include "ln.h"
#ifndef __alpha
#include "msi.h"
#include "vaxbi.h"
#endif
#ifdef mips
#include "ne.h"
#endif

#ifdef vax
#include "mba.h"
#else
#define NMBA 0
#endif vax

#include "uq.h"
#include "uba.h"
#include "kdb.h"
#include "klesib.h"
#include "kdm.h"

#include <sys/types.h>			/* cpu.h needs this */
/* #include <machine/pte.h> */
#include <machine/cpu.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/map.h>
#include <sys/buf.h>
#include <sys/dk.h>
#include <sys/vm.h>
#include <sys/conf.h>
/*#include <sys/dmap.h>	*/
#include <sys/reboot.h>
/* #include <sys/cpudata.h> */


#ifdef vax
#include <machine/mem.h>
#include <machine/mtpr.h>
#include <machine/ioa.h>
#include <machine/nexus.h>
#endif vax

#ifdef __alpha
#include <machine/scb.h>
#else
#include <hal/scb.h>
#endif
#include <io/dec/scs/sca.h>

#ifdef vax
#include <dec/io/mba/vax/mbareg.h>
#include <dec/io/mba/vax/mbavar.h>
#endif vax

#include <io/dec/uba/ubareg.h>
#include <io/dec/uba/ubavar.h>
#include <hal/cpuconf.h>

#ifdef mips
struct qbm_regs *qb_ptr;	/* Points to unibus adaptor regs */
#endif mips

#ifdef	BINARY
#if NCI > 0
extern	int	nNCI;
#endif NCI

#if NMSI > 0
extern	int	nNMSI;
#endif NMSI

extern	int	numbvp;
extern	int	numci;
extern	int	nummsi;
#ifdef vax
extern	int	(*mbaintv[])();
#endif vax
extern	int	(*ubaintv[])();
extern	int	nNMBA;
extern	(*Mba_find)();
extern	int	nNUBA;
extern  int	nNKDM;
extern  int	nNKDB;
extern  int	nNMBA;
extern  int	nNKLESIB;

#else
int cpu_avail = 1;

/*
 * Addresses of the (locore) routines which bootstrap us from
 * hardware traps to C code.  Filled into the system control block
 * as necessary.
 */
#if defined mips || defined __alpha

#ifndef DEC4000
cobra_config_cbus() {};
#endif /* !DEC4000 */

#if NVAXBI == 0
bisst() {}	/* stub for VAXBI start self test */
#endif NVAXBI
#endif mips

#if NUQ == 0
uqdriver() {}	/* stub for machdep gendump() */
#else
#ifdef __alpha
#if NKDM == 0
#undef NKDM
#define NKDM NUQ
#endif
#endif /* __alpha */
#endif /* NUQ */

#if NMBA > 0 
int	(*mbaintv[4])() =	{ Xmba0int, Xmba1int, Xmba2int, Xmba3int };
#endif NMBA 

#ifdef VAX8600
int	(*ubaintv[7])() =	{ Xua0int, Xua1int, Xua2int, Xua3int, Xua4int, Xua5int, Xua6int }; 
#else
#ifdef VAX780
int	(*ubaintv[4])() =	{ Xua0int, Xua1int, Xua2int, Xua3int };
#else VAX780
int	(*ubaintv[4])() = 	{ (int (*)()) -1, ( int(*)()) -1, ( int(*)()) -1, ( int(*)()) -1};
#endif VAX780
#endif VAX8600

#if defined mips || defined __alpha /* Stubs for the SGEC */
#if NNE == 0
neintr(){stray(0,0xd4);}
neprobe(){return(-1);     /* stub returns -1 if its not configured */ }
#endif NNE
#endif mips
		/* Make locore and ka650 happy if no NI device is configed in */
#if NLN == 0

#ifdef vax
lnintr(){logstray(1,0x14,0xd4);} /* ELSI_SCB */
#endif vax
#if defined mips || defined __alpha /* Stubs for the NI */
lnintr() {stray(0,0xd4);} 
#endif mips

lnprobe(){return(-1);     /* stub returns -1 if its not configured */ }
#endif NLN
		/* Protect ka650.c from undefines if no MSI devices are configed in */

/*
 * Allocate the MSI adapter data structures.
 */
int		nummsi = 0;		/* Number of local MSI ports	     */
#if NMSI > 0
int		nNMSI = NMSI;		/* Number of configured MSI ports    */
struct _pccb	*msi_adapt[ NMSI ];	/* MSI adapter structures	     */
#endif NMSI

int		numbvp = 0;		/* Number of local BVP ports	     */

/*
 * Allocate the CI adapter structures
 */
int		numci = 0;		/* Number of local CI ports	     */
#if NCI > 0
int		nNCI = NCI;		/* Number of configured CI ports     */
CIISR		ci_isr[ NCI ];

#ifdef mips
ciintr(int i) { (ci_isr[i].isr)(ci_isr[i].pccb); }
#endif mips

#else
CIISR		ci_isr[ 1 ];
#endif NCI

void		( *scs_disable )()	/* Address of SCS shutdown routine   */
		    = NULL;		/* Initialized by SCS		     */

/*
 * This allocates the space for the per-uba information,
 * such as buffered data path usage.
 */
#if NUBA != 0  || NKDB != 0 || NKLESIB != 0 || NKDM != 0
struct	uba_hd uba_hd[NUBA+NKDB+NKLESIB+NKDM];
#else
struct uba_hd uba_hd[1];
#endif NUBA

#if NUBA > 0
int	tty_ubinfo[NUBA];
#else
int	tty_ubinfo[1];
#endif

#if NMBA > 0
extern	mbafind();
int	(*Mba_find)() = mbafind;
#else NMBA
int	(*Mba_find)() = (int (*)()) -1;
mbintr()	{/* Keep locore happy */ }
#endif NMBA

#ifdef __alpha

#include "xmi.h"
int numxmi;
#if NXMI == 0
get_xmi() {/* stub */}
xmisst() {/* stub */}
#endif /* NXMI */

#endif /* __alpha */

int	nNKDM = NKDM;
int	nNUBA = NUBA;
int	nNKDB = NKDB;
int	nNMBA = NMBA;
int	nNKLESIB = NKLESIB;

#ifdef vax
#ifdef	EMULFLT

asm(".globl	_vaxopcdec");
asm("_vaxopcdec:	.long	vax$opcdec");
asm(".globl	_vaxspecialhalt");
asm("_vaxspecialhalt: .long	vax$special_halt");
asm(".globl	_vaxspecialcont");
asm("_vaxspecialcont: .long	vax$special_cont");
asm(".globl	_vaxemulbegin");
asm("_vaxemulbegin:	.long	vax$emul_begin");
asm(".globl	_vaxemulend");
asm("_vaxemulend:	.long	vax$emul_end");
asm(".globl	_exeacviolat");
asm("_exeacviolat:	.long	exe$acviolat");

#else	EMULFLT

int (*vaxopcdec)() = 0;

#if defined (MVAX) || defined (VAX3600) || defined (VAX6200) || defined (VAX420)
asm(".globl	_vaxspecialhalt");
asm("_vaxspecialhalt: .long	vax$special_halt");
asm(".globl	_vaxspecialcont");
asm("_vaxspecialcont: .long	vax$special_cont");
asm(".globl	_vaxemulbegin");
asm("_vaxemulbegin:	.long	vax$emul_begin");
asm(".globl	_vaxemulend");
asm("_vaxemulend:	.long	vax$emul_end");
asm(".globl	_exeacviolat");
asm("_exeacviolat:	.long	exe$acviolat");

#else MVAX || VAX420

int (*vaxspecialhalt)() = 0;
int (*vaxspecialcont)() = 0;
int (*vaxemulbegin)() = 0;
int (*vaxemulend)() = 0;
int (*exeacviolat)() = 0;
#endif MVAX || VAX420

#endif EMULFLT
#endif vax

#endif BINARY

