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
static char	*sccsid = "@(#)$RCSfile: slot_name.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:11:22 $";
#endif 
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
 *	File:	slot_name.c
 *	Author:	Avadis Tevanian, Jr.
 *
 *	Convert machine slot values to human readable strings.
 */

#include <mach.h>

/*
 *	Convert the specified cpu_type/cpu_subtype pair to their
 *	human readable form.
 */
slot_name(cpu_type, cpu_subtype, cpu_name, cpu_subname)
	cpu_type_t	cpu_type;
	cpu_subtype_t	cpu_subtype;
	char		**cpu_name, **cpu_subname;
{
	register char	*name, *subname;

	switch (cpu_type) {
	case CPU_TYPE_VAX:
		name = "VAX";
		switch (cpu_subtype) {
		case CPU_SUBTYPE_VAX780:	subname = "780"; break;
		case CPU_SUBTYPE_VAX785:	subname = "785"; break;
		case CPU_SUBTYPE_VAX750:	subname = "750"; break;
		case CPU_SUBTYPE_VAX730:	subname = "730"; break;
		case CPU_SUBTYPE_UVAXI:		subname = "MicroVax I"; break;
		case CPU_SUBTYPE_UVAXII:	subname = "MicroVax II"; break;
		case CPU_SUBTYPE_VAX8200:	subname = "8200"; break;
		case CPU_SUBTYPE_VAX8500:	subname = "8500"; break;
		case CPU_SUBTYPE_VAX8600:	subname = "8600"; break;
		case CPU_SUBTYPE_VAX8650:	subname = "8650"; break;
		case CPU_SUBTYPE_VAX8800:	subname = "8800"; break;
		case CPU_SUBTYPE_UVAXIII:	subname = "MicroVax III"; break;
		}
		break;
	case CPU_TYPE_ROMP:
		name = "ROMP";
		switch (cpu_subtype) {
		case CPU_SUBTYPE_RT_PC:		subname = "RT/PC"; break;
		case CPU_SUBTYPE_RT_APC:	subname = "RT/APC"; break;
		case CPU_SUBTYPE_RT_135:	subname = "RT/135"; break;
		}
		break;
	case CPU_TYPE_MC68020:
		name = "MC68020";
		switch (cpu_subtype) {
		case CPU_SUBTYPE_SUN3_50:	subname = "Sun-3/50"; break;
		case CPU_SUBTYPE_SUN3_160:	subname = "Sun-3/160"; break;
		case CPU_SUBTYPE_SUN3_260:	subname = "Sun-3/260"; break;
		case CPU_SUBTYPE_SUN3_110:	subname = "Sun-3/110"; break;
		case CPU_SUBTYPE_SUN3_60:	subname = "Sun-3/60"; break;
		}
		break;
	case CPU_TYPE_NS32032:
		name = "NS32032";
		switch (cpu_subtype) {
		case CPU_SUBTYPE_MMAX_DPC: subname = "Multimax [DPC]"; break;
		case CPU_SUBTYPE_SQT:	subname = "Balance 21000"; break;
		}
		break;
	case CPU_TYPE_NS32332:
		name = "NS32332";
		switch (cpu_subtype) {
		case CPU_SUBTYPE_MMAX_APC_FPU:
			subname = "Multimax [APC]"; break;
		case CPU_SUBTYPE_MMAX_APC_FPA:
			subname = "Multimax [APC] with FPA"; break;
		}
		break;
	case CPU_TYPE_NS32532:
		name = "NS32532";
		switch (cpu_subtype) {
		case CPU_SUBTYPE_MMAX_XPC_FPU:
			subname = "Multimax [XPC]"; break;
		case CPU_SUBTYPE_MMAX_XPC_FPA:
			subname = "Multimax [XPC] with FPA"; break;
		}
		break;
	case CPU_TYPE_I386:
		name = "I386";
		switch (cpu_subtype) {
		case CPU_SUBTYPE_AT386: subname = "i386 [AT]"; break;
		case CPU_SUBTYPE_EXL: subname = "i386 [EXL]"; break;
		}
		break;
	case CPU_TYPE_MIPS:
		name = "MIPS";
		switch (cpu_subtype) {
		case CPU_SUBTYPE_MIPS_R2300: subname = "R2300"; break;
		case CPU_SUBTYPE_MIPS_R2600: subname = "R2600"; break;
		case CPU_SUBTYPE_MIPS_R2800: subname = "R2800"; break;
		case CPU_SUBTYPE_MIPS_R2000a: subname = "R2000a"; break;
		}
		break;
	default:
		name = "Unknown CPU";
		subname = "";
		break;
	}
	*cpu_name = name;
	*cpu_subname = subname;
}

