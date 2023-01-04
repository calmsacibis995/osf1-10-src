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
 * 30-oct-91	jac	Fixup Alpha switches from sync to BL6
 *
 * 28-oct-91	jac	Added nulladdrcpu routine. Used as null routine for
 *			functions which must return an address (and therefore
 *			may have a 64 bit return value)
 *
 * 03-Oct-91    ald     added flamingo support
 *
 * 12-sep-91    jac     added nexaddr routine reference
 *
 * 21-jun-91    jac     added laser/ruby  support
 *
 * 03-May-91    afd
 *      Alpha support.
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
 * 31-Aug-90	Jim Paradis
 *	Added additional stubs for VAX9000 routines
 *
 * 03-Aug-90	rafiey (Ali Rafieymehr)
 *	Added support for VAX9000.
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
 * 08-Dec-89	jaw
 *	fix 6200 entry from merge damage.
 *
 * 30-Nov-89    Paul Grist
 *      Added 8800 error logging routines as stubs for non-8800 
 *      VAXBI systems that will use biinit.c Did the same for
 *      ka6200 and ka6400.
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
 * 23-May-89	darrell
 *	Merged VVAX support.
 *
 * 03-May-89	darrell
 *	Merged in VAX6400 support.
 *
 * 07-Apr-89	afd
 *	Created this file as a merged version of the old VAX cpuconf.c
 *	with new entries for MIPS based systems.  This file now supports
 *	both VAX and MIPS based systems.
 *
 **********************************************************************/


#include <sys/param.h>
#include <hal/cpuconf.h>
#include <machine/cpu.h>
#include <io/dec/uba/ubareg.h>

int	nocpu();
int	nullcpu();
vm_offset_t nulladdrcpu();

int	readtodr();
int 	writetodr();
int 	uICRdelay();
int 	uInoICRdelay();
int 	uIInoICRdelay();
int 	cVSnoICRdelay();
int 	uSSCdelay();
int	uRSSCdelay();
int	ssc_readtodr();
int	ssc_writetodr();

char    UNKNOWN_SYS_STRING[] = "Unknown system type";

#ifdef ALPHAADU
extern int      adu_machcheck();
extern int      adu_memerr();
extern int      adu_crderr();
extern int      adu_memenable();
extern int      adu_conf();
extern int      adu_init();
extern int      adu_cachenble();
extern int      adu_cachdisble();
extern int      adu_flush_cache();
extern int      alpha_readtodr();
extern int      alpha_writetodr();
extern int      adu_delay();
extern int      adu_map_io();
char    ALPHA_ADU_STRING[] = "ALPHA ADU";
#endif /* ALPHAADU */

#ifdef DEC7000
extern int      ruby_machcheck();
extern int      kn7aa_proccorr();
extern int      kn7aa_syscorr();
extern int      lsb_memerr();
extern int      lsb_crderr();
extern int      lsb_memenable();
extern int      ruby_conf();
extern int      ruby_init();
extern int      ruby_cachenble();
extern int      ruby_cachdisble();
extern int      ruby_flush_cache();
extern int      ruby_badaddr();
extern int      ka_ruby_readtodr();
extern int      ka_ruby_writetodr();
extern int      alpha_delay();
extern int      ruby_map_io();
extern int	kn7aa_dump_dev();
extern int	kn7aa_getinfo();
extern vm_offset_t	ruby_nexaddr();
char    DEC7000_STRING[] = "DEC7000";
#endif /* DEC7000 */

#ifdef DEC4000
extern int      kn430_machcheck();	/* 0x660, 0x670 */
extern int      kn430_proccorr();	/* 0x630 */
extern int      cobra_conf();
extern int      cobra_init();
extern int      cobra_cachenble();
extern int      cobra_cachdisble();
extern int      cobra_flush_cache();
extern int      cobra_badaddr();
extern int      cobra_readtodr();
extern int      cobra_writetodr();
extern int      alpha_delay();
extern int      cobra_consprint();
extern int	kn430_dump_dev();
extern int	kn430_getinfo();
char    DEC4000_STRING[] = "DEC4000";
#endif /* DEC4000 */

#ifdef DEC3000_500
extern int      kn15aa_machcheck();
extern int      kn15aa_memerr();
extern int      kn15aa_crderr();
extern int      kn15aa_memenable();
extern int      kn15aa_conf();
extern int      kn15aa_init();
extern int      kn15aa_cachenble();
extern int      kn15aa_cachdisble();
extern int      kn15aa_flush_cache();
extern int      kn15aa_badaddr();
extern int      kn15aa_readtodr();
extern int      kn15aa_writetodr();
extern int      alpha_delay();
extern int      kn15aa_map_io();
extern int	kn15aa_system_id();
extern int	kn15aa_dump_dev();
extern int	kn15aa_getinfo();
char    DEC_3000_500_STRING[] = "DEC3000 - M500";
#else
kn15aa_is_sandpiper() { return (0); }
kn15aa_set_ioslot() {}
kn15aa_read_ioslot() { return (0); }
kn15aa_read_ir() { return (0); }
#endif /* DEC3000_500 */

#ifdef DEC3000_300
extern int      kn16aa_machcheck();
extern int      kn16aa_memerr();
extern int      kn16aa_crderr();
extern int      kn16aa_memenable();
extern int      kn16aa_conf();
extern int      kn16aa_init();
extern int      kn16aa_cachenble();
extern int      kn16aa_cachdisble();
extern int      kn16aa_flush_cache();
extern int      kn16aa_badaddr();
extern int      kn16aa_readtodr();
extern int      kn16aa_writetodr();
extern int      alpha_delay();
extern int      kn16aa_map_io();
extern int 	kn16aa_system_id();
extern int	kn16aa_dump_dev();
extern int	kn16aa_getinfo();
char    DEC_3000_300_STRING[] = "DEC3000 - M300";
#endif /* DEC3000_300 */


#ifdef DEC2000_300
extern int      kn121_machcheck();
extern int      kn121_conf();
extern int      kn121_init();
extern int      kn121_cachenble();
extern int      kn121_cachdisble();
extern int      kn121_flush_cache();
extern int      mc146818_readtodclk();
extern int      mc146818_writetodclk();
extern int      alpha_delay();
extern vm_offset_t	kn121_get_io_handle();
extern int	kn121_read_io_port();
extern int	kn121_write_io_port();
extern int	kn121_dump_dev();
extern int	kn121_ring_bell();
extern int	kn121_io_bcopy();
char    DEC_2000_300_STRING[] = "DEC2000 - Model 300";
#else
extern int	kn121_configure_io()  { return (0); }
#endif /*DEC2000_300*/


struct cpusw	cpusw[] =
{
#ifdef DEC2000_300
        { DEC_2000_300,		kn121_machcheck,	nullcpu,
          nullcpu,		nullcpu,		nullcpu,
          nullcpu,              kn121_conf,             nullcpu,
          nullcpu,              nullcpu,                nocpu,
          mc146818_readtodclk,	mc146818_writetodclk,	alpha_delay,
          nullcpu,              nullcpu,                nullcpu,
          nullcpu,              nullcpu,                nullcpu,
          nulladdrcpu,          nullcpu,		nullcpu,
          nullcpu,              nullcpu,		kn121_get_io_handle,
	  kn121_read_io_port,	kn121_write_io_port,	kn121_ring_bell,
	  kn121_io_bcopy,	nullcpu,		nullcpu,
	  nullcpu,		kn121_dump_dev,		nullcpu,
/* end of cpusw common area */
/* Specific Routines */
          kn121_init,
          0 /*pc_umsize*/,	 0 /*pc_haveubasr*/,	 10000 /* drift */,
          DEC_2000_300_STRING,	100 /*HZ*/,		0 /*flags*/,
          },
#endif /* DEC2000_300 */

#ifdef DEC7000
#define DEC7000_FLAGS (SCS_START_SYSAPS | MSCP_POLL_WAIT)
        { DEC_7000,             ruby_machcheck,         kn7aa_syscorr,
          kn7aa_proccorr,       lsb_memenable,          nullcpu,
          nullcpu,              ruby_conf,              nullcpu,
          nullcpu,              nullcpu,                ruby_badaddr,
          ka_ruby_readtodr,     ka_ruby_writetodr,      alpha_delay,
          nullcpu,              ruby_map_io,            nullcpu,
          nullcpu,              nullcpu,                nullcpu,
          ruby_nexaddr,         nullcpu,		nullcpu,
          nullcpu,              nullcpu,		nulladdrcpu,
	  nullcpu,		nullcpu,		nullcpu,
	  nullcpu,		nullcpu,		nullcpu,
	  nullcpu,		kn7aa_dump_dev,		kn7aa_getinfo,

/* end of cpusw common area */
/* Specific Routines */
          ruby_init, /* could be nullcpu */
          0,                    0,                      10000 /* drift */,
	  DEC7000_STRING,	100 /*HZ*/,
	  DEC7000_FLAGS /*flags*/,
          },

#endif /* DEC7000 */

#ifdef DEC4000
        { DEC_4000,             kn430_machcheck,        nullcpu,
          kn430_proccorr,       nullcpu,		nullcpu,
          nullcpu,              cobra_conf,             nullcpu,
          nullcpu,              nullcpu,                cobra_badaddr,
          cobra_readtodr,       cobra_writetodr,        alpha_delay,
          nullcpu,              nullcpu,                nullcpu,
          nullcpu,              nullcpu,                nullcpu,
          nulladdrcpu,	        nullcpu,		nullcpu,
          nullcpu,              nullcpu,		nulladdrcpu,
	  nullcpu,		nullcpu,		nullcpu,
	  nullcpu,		nullcpu,		nullcpu,
	  nullcpu,		kn430_dump_dev,		kn430_getinfo,

/* end of cpusw common area */
/* Specific Routines */
          cobra_init, /* could be nullcpu */
          0,                    0,                      10000 /* drift */,
	  DEC4000_STRING,	100 /*HZ*/,		0 /*flags*/,
          },
#endif /* DEC4000 */

#ifdef DEC3000_500
        { DEC_3000_500,       	kn15aa_machcheck,     	kn15aa_memerr,
          kn15aa_crderr,      	kn15aa_memenable,     	nullcpu,
          nullcpu,              kn15aa_conf,          	nullcpu,
          nullcpu,              nullcpu,	      	kn15aa_badaddr,
          kn15aa_readtodr,    	kn15aa_writetodr,	alpha_delay,
          nullcpu,              kn15aa_map_io,	        nullcpu,
          nullcpu,              nullcpu,                nullcpu,
          nulladdrcpu,          nullcpu,		nullcpu,
          nullcpu,              nullcpu,		nulladdrcpu,
	  nullcpu,		nullcpu,		nullcpu,
	  nullcpu,		nullcpu,		nullcpu,
	  kn15aa_system_id,	kn15aa_dump_dev,	kn15aa_getinfo,

/* end of cpusw common area */
/* Specific Routines */
          kn15aa_init, /* can not be nullcpu */
          0,                    0,                      10000 /* drift */,
	  DEC_3000_500_STRING, 	100 /*HZ*/,		0 /*flags*/,
          },
#endif /* DEC3000_500 */

#ifdef DEC3000_300
        { DEC_3000_300,         kn16aa_machcheck,       kn16aa_memerr,
          kn16aa_crderr,        kn16aa_memenable,       nullcpu,
          nullcpu,              kn16aa_conf,            nullcpu,
          nullcpu,              nullcpu,                kn16aa_badaddr,
          kn16aa_readtodr,      kn16aa_writetodr,       alpha_delay,
          nullcpu,              kn16aa_map_io,          nullcpu,
          nullcpu,              nullcpu,                nullcpu,
          nulladdrcpu,          nullcpu,              	nullcpu,
          nullcpu,              nullcpu,		nulladdrcpu,
	  nullcpu,		nullcpu,		nullcpu,
	  nullcpu,		nullcpu,		nullcpu,
	  kn16aa_system_id,	kn16aa_dump_dev,	kn16aa_getinfo,
/* end of cpusw common area */
/* Specific Routines */
          kn16aa_init, /* can not be nullcpu */
          0,                    0,                      10000 /* drift */,
	  DEC_3000_300_STRING, 100 /*HZ*/,              0 /*flags*/,
          },
#endif /* DEC3000_300 */

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
	0,			0,			0,
	0,			0,			0,
	0,			0,			0,
	0,			0,			0,
	0,			0,			0,
	 }
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
	panic("nocpu: unimplemented cpu");
}

/*
 * null routine to pass back a success since this cpu type
 * doesn't need one of these routines.
 */
nullcpu()
{
	return(0);
}

/*
 * nulladdrcpu() is used to as a stub for those CPUs that don't need
 * to use a routine that otherwise must return an addresss
*/
vm_offset_t nulladdrcpu()
{
    static int once = 0;
    if (once == 0) {
	printf("nulladdrcpu called\n");
	once++;
    }
	return(0);
}
