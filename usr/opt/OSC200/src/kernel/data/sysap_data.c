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
 *
 *   Facility:	Systems Communication Architecture
 *
 *   Abstract:	This module contains the global table of routines that
 *		initialize and or start System Applications (SYSAPS).
 *
 *
 *   Creator:	Larry Cohen	Creation Date:  March 13, 1987
 *
 *   History:
 *
 *   14-Mar-88	Larry Cohen
 *	Initialize scs before other sysaps start up.
 *
 *   18-Apr-88  Ricky Palmer
 *	Added support for MSI (Mass Storage Interconnect) on MF2.
 *
 *   16-Jun-88		larry
 *	mscp drivers initialized when uq,bvpssp,ci, or msi are configured in.
 *
 */
/**/


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/time.h>
#include	<dec/binlog/errlog.h>
#include 	<io/dec/sysap/sysap_start.h>

#include "uq.h"
#include "ci.h"
#include "msi.h"
#include "bvpssp.h"
#include "inet.h"
#include "scsnet.h"


#if NCI > 0 || NUQ > 0 || NBVPSSP > 0 || NMSI > 0
	extern void scsdir_init();
	extern void scs_initialize();
#	define SYSAP_SCS$DIRECTORY scsdir_init
#	define SCS_INIT scs_initialize
#else
#	define SYSAP_SCS$DIRECTORY 0
#	define SCS_INIT 0
#endif

#if NCI > 0 && NINET > 0 && NSCSNET > 0
	extern void scsnet_attach();
#	define SYSAP_SCSNET scsnet_attach
#else
#	define SYSAP_SCSNET 0
#endif

#if NCI > 0 || NUQ > 0 || NBVPSSP > 0 || NMSI > 0
	extern void mscp_init_driver();
#	define SYSAP_MSCP mscp_init_driver
#else
#	define SYSAP_MSCP 0
#endif


#if NCI > 0 || NUQ > 0 || NBVPSSP > 0 || NMSI > 0
	extern void      tmscp_init_driver();
#	define SYSAP_TMSCP tmscp_init_driver
#else
#	define SYSAP_TMSCP 0
#endif

#ifndef BINARY

	Sysap_start sysaps[] = {
		SCS_INIT,
		SYSAP_SCS$DIRECTORY,
		SYSAP_SCSNET,
		SYSAP_TMSCP,
		SYSAP_MSCP,
		SYSAP_LAST
	};

#endif
