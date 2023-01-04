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
 *	@(#)cpuconf.h	9.6	(ULTRIX/OSF)	11/18/91
 */ 
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
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Derived from cpuconf.h	4.6	(ULTRIX)	9/3/90
 */

/*
 * Modification History: cpuconf.h
 *
 * 10-May-91	Paul Grist
 * 	Added support for 3max+/bigmax (DS_5000_300).
 *
 * 20-Aug-90	Matt Thomas
 *	Added maxdriftrecip (The reciprocal of the maximum clock drift
 *	rate) for both VAX and MIPS.
 *
 * 13-Aug-90	sekhar
 *	added print_consinfo interface for both vax and mips.
 *	added log_errinfo interface for both vax and mips.
 *
 * 03-Aug-90	Randall Brown
 *	Added system specific entries for all spl's and the intr() routine.
 *	Added entries for specific clock variables ( todrzero, rdclk_divider).
 *	Added #define for DS_5000_100 (3MIN).
 *
 * 03-Aug-90	rafiey (Ali Rafieymehr)
 *	Added support for VAX9000.
 *
 * 01-May-90    szczypek
 *      Added Mariah ID and Mariah variant.
 *
 * 01-May-90    Paul Grist
 *      Added support for mipsmate - DECsystem 5100
 *
 * 10-July-89	burns
 *	Moved Vax io related items to common section of table for DS5800.
 *	Added memsize and cpuinit for afd.
 *
 * 30-June-89	afd
 *	Add memory sizing routine to cpu switch table.
 *
 * 09-June-89	afd
 *	Added field HZ to cpusw (used to be in param.c).
 *	hz, tick, tickadj are set in processor specific init routines.
 *
 * 02-May-89	afd
 *	Added define VAX_3900 for Mayfair III system.
 *
 * 07-Apr-89	afd
 *	Created this file as a merged version of the old VAX cpuconf.h
 *	with new entries for MIPS based systems.  This file now supports
 *	both VAX and MIPS based systems.
 *
 *	Moved cpu type and system type defines here from cpu.h
 *	Moved macros for getting items from "systype" word here from hwconf.h
 *	Moved defines for R2000a cpu type and PMAX systype here from hwconf.h
 *	Added additional defines for CPU types, systypes and cpusw indexes.
 */


#ifndef _CPUCONF_H_
#define _CPUCONF_H_

/*
 * DEC processor types for Alpha systems.  Found in HWRPB.
 * These values are architected.
 */
#define EV3_CPU		1
#define EV4_CPU		2

/*
 * DEC system types for Alpha systems.  Found in HWRPB.
 * These values are architected.
 */
#define ST_ADU			1	/* Alpha ADU systype */
#define ST_DEC_4000		2	/* Cobra systype */
#define ST_DEC_7000		3	/* Ruby systype */
#define ST_DEC_3000_500		4	/* Flamingo systype */
#define ST_DEC_2000_300		6	/* Jensen systype */
#define ST_DEC_3000_300         7       /* Pelican systype */

/*
 * Bits in the Alpha processor variation field
 */
#define VAR_VAXFP	0x0000000000000001
#define VAR_IEEEFP	0x0000000000000002

/*
 * Bits in the Alpha system variation field
 */
#define SYS_MP		0x0000000000000001

/*
 * Value to be stored in the global variable "cpu".
 * The contents of "cpu" is no longer used to index into the "cpusw".
 * These values are arbitrarily assigned by ULTRIX (now OSF), but must each 
 * be unique and must be in the range 0 through (256 * 1024 -1). 	
 * See note on 3rd party CPU support below. 			 	
 * The variable "cpu" really contains a unique "system" identifier.
 *
 * note: the vector cpu_types[] within ../../io/scs/scsvar.c must be
 * updated when new cpu values are defined.
 *
 * These values must also track with MIPs (VAX) stuff, since the common
 * vector cpu_types[] uses them.
 *
 * note: Macros for registration of 3rd party CPUs are defined in
 * mach/machine.h  Digital is Vendor ID 0. 
 * Full macro definitions for each Digital CPU type are not provided here,  
 * but would be as shown in the example below for Flamingo:
 *
 *      #define DEC_VENDOR_ID   0       
 *      #define VENDOR_ID DEC_VENDOR_ID 
 *      #define DEC_3000_500  MAKESID(30)
 *  
 */


#define UNKN_SYSTEM	0
#define VAX_780         1
#define VAX_750         2
#define VAX_730         3
#define VAX_8600        4
#define VAX_8200        5
#define VAX_8800        6
#define MVAX_I          7
#define MVAX_II         8
#define V_VAX           9	/* Virtual VAX			*/
#define VAX_3600        10	/* Mayfair I			*/
#define VAX_6200        11	/* CVAX/Calypso			*/
#define VAX_3400        12	/* Mayfair II			*/
#define C_VAXSTAR       13	/* VAX3100 (PVAX)		*/
#define VAX_60          14	/* Firefox			*/
#define VAX_3900        15	/* Mayfair III			*/
#define	DS_3100		16	/* DECstation 3100 (PMAX)	*/
#define VAX_8820        17	/* This is the SID for Polarstar*/
#define	DS_5400 	18	/* MIPSfair			*/
#define	DS_5800		19	/* ISIS				*/
#define	DS_5000		20	/* 3MAX				*/
#define	DS_CMAX		21	/* CMAX				*/
#define VAX_6400	22	/* RIGEL/Calypso		*/
#define VAXSTAR		23	/* VAXSTAR			*/
#define DS_5500		24	/* MIPSFAIR-2			*/
#define DS_5100		25	/* MIPSMATE			*/
#define	VAX_9000	26	/* VAX9000			*/
#define DS_5000_100	27	/* 3MIN 			*/

#define ALPHA_ADU	28	/* Alpha ADU			*/
#define DEC_4000	29	/* Cobra			*/
#define DEC_3000_500	30	/* Flamingo workstation		*/
#define DEC_7000	31	/* EV4 on Lazer platform	*/
#define DS_5000_300	32	/* 3MAX+/BIGMAX			*/
#define DEC_3000_300	33	/* Pelican workstation		*/
#define DEC_2000_300	34	/* Jensen Alpha PC		*/
#define CPU_MAX		34	/* Same # as last real entry	*/

#define S_CORR_ERR    0x620   /* System correctable error */
#define P_CORR_ERR   0x630   /* Processor correctable error */
#define S_MCHECK      0x660   /* System machine check */
#define P_MCHECK     0x670   /* Processor machine check */

/*
 * Defines for possible values of the flags field for mips and alpha
 */
#define SCS_START_SYSAPS	0x00000001
#define MSCP_POLL_WAIT		0x00000002

#ifndef ASSEMBLER
/*
 * The system switch is defined here.  Each entry is the only
 * line between the main unix code and the cpu dependent
 * code.  The initialization of the system switch is in the
 * file cpuconf.c.  The index values used in the switch are
 * defined in cpu.h.
 */

struct cpusw
{
	/* Common Routines */
	int system_type;	/* 1 Value for "cpu" (Alpha_nnnn)	*/
	int (*machcheck)();	/* 2 Hrdwre trap (Vax: mcheck; Mips: trap err) */
	int (*harderr_intr)();	/* 3 Hard err (Vax:SCB 60)		*/
	int (*softerr_intr)();	/* 4 Soft err (Vax:SCB 54 (CRD)		*/
	int (*timer_action)();	/* 5 VAX: Enable CRD intr; MIPS: check CPEs */
	int (*cons_putc)();	/* 6 Write char to console		*/
	int (*cons_getc)();	/* 7 Read char from console		*/
	int (*config)();	/* 8 System configuration		*/
	int (*cachenbl)();	/* 9 Turn on cache (I or D cache)	*/
	int (*cachdisbl)();	/* 10 Turn off cache (I or D cache)	*/
	int (*flush_cache)();	/* 11 Flush caches (I or D)		*/
	int (*badaddr)();	/* 12 Probe addresses			*/
	int (*readtodr)();	/* 13 Read time of day			*/
	int (*writetodr)();	/* 14 Write time of day			*/
	int (*microdelay)();	/* 15 Microdelay			*/
	int (*clear_err)();	/* 16 Clear hardware error (from I/O probe)*/
	int (*mapspace)();	/* 17 Make IO, CSR, ..etc accessible	*/
	int (*reboot)();	/* 18 Reboot Ultrix			*/
	int (*halt)();		/* 19 Do a cpu halt (simulate on mips)	*/
	int (*startcpu)();	/* 20 Start a non-boot cpu		*/
	int (*stopcpu)();	/* 21 Stop a cpu			*/
	vm_offset_t (*nexaddr)(); /* 22 Get address of next I/O adapter	*/
	int (*umaddr)();	/* 23					*/
	int (*udevaddr)();	/* 24					*/
	int (*print_consinfo)(); /* 25 print information to console	*/
	int (*log_errinfo)();	 /* 26 log information to error log buf	*/
	vm_offset_t (*get_io_handle)(); /* 27 Place holder for bus support */
	int (*read_io_port)(); /* 28 Place holder for bus support 	*/
	int (*write_io_port)(); /* 29 Place holder for bus support 	*/
	int (*ring_bell)();	/* 30 ring bell				*/
	int (*io_bcopy)();	/* 31 Place holder for bus support      */
	int (*reserved3)();	/* 32 Place holder for future expansion */
	int (*reserved4)();	/* 33 Place holder for future expansion */
	int (*unique_sysid)();	/* 34 Get a unique system id */
	int (*trans_dumpdev)();	/* 35 Get system specific device string for firmware */
	int (*get_info)();	/* 36 Get system specific info */

	int (*init)();		 /* Initialization 		 	*/

	/* Common Data Values */
	int pc_umsize;		 /* unibus memory size			*/
	short pc_haveubasr;	 /* have uba status register		*/
	unsigned long maxdriftrecip; /* maximum drift rate for this processor */
	char *system_string;	 /* system type string			*/

	/* Alpha Specific Data Values */
	int HZ;			/* how many times a sec the clock interrupts */
	int flags;		/* system specific flags		*/

};

extern struct cpusw cpusw[];

#endif /* ASSEMBLER */
#endif /* _CPUCONF_H_ */

