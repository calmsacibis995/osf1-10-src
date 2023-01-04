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
 * Digital TC non-voltile RAM
 */

#include "nvtc.h"

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/presto.h>
#include <sys/syslog.h>
#include <vm/vm_kern.h>
#include <hal/cpuconf.h>
#include <io/common/devdriver.h>
#include <io/dec/tc/nvtcreg.h>
#include <io/dec/uba/ubavar.h>
#include <io/dec/tc/tc.h>
#ifdef __alpha
#include <hal/kn15aa.h>
#endif
					

/*
 *  Macros to read the TCNVRAM registers.
 *  Users of macro should explicitly test for wanted bits.
 */

#define NVTCREG_WR(csr,v) (csr) = (v)
#define NVTCREG_RD(csr) (csr)
/*
 * Define the softc for the NVRAM driver
 */
struct	nvtc_softc {
	NVRAM_REG 	_nvtc_reg;   		/* nvram registers */
	caddr_t		compare_buffer;		/* Used for debugging; comparing shadow to cache copy */
	vm_offset_t	regbase;		/* base address for registers */
	u_long		cache_physical_start;	/* Physical start address of NVRAM cache */
	vm_offset_t	cache_start;		/* start of KSEG address of the cache for presto */
	caddr_t		shadow;			/* where main memory shadow copy goes */
	u_int		burst_size;		/* Size of bursts on TC during DMA transfers */
	u_int		slot;			/* Slot this presto board is located in */
	u_int		cache_size;		/* size of NVRAM cache */
	u_int		cache_offset;		/* Offset to the first nvram location from the start of the TC slot address space */
	u_int		cache_4MB;		/* boolean - true = 4MB false = 1MB */
	u_int		block_mode;		/* Boolean for block mode I/O writes is enabled or not */
	u_int		diag_status;		/* If the board passed diags or not */
	struct 		nvtcinit *initblk;	/* Init command info */
	struct 		nvtcstatus *statusblk;	/* Per-unit status info */
};

#define csraddr 	_nvtc_reg.nvtc_csr
#define daraddr		_nvtc_reg.nvtc_dar 
#define	bataddr 	_nvtc_reg.nvtc_bat 
#define maraddr		_nvtc_reg.nvtc_mar 
#define bcraddr		_nvtc_reg.nvtc_bcr  

#define reg_csr 	_nvtc_reg.nvtc_csr->reg
#define reg_dar		_nvtc_reg.nvtc_dar->reg
#define	reg_bat 	_nvtc_reg.nvtc_bat->reg 
#define reg_mar		_nvtc_reg.nvtc_mar->reg 
#define reg_bcr		_nvtc_reg.nvtc_bcr->reg  


#ifdef BINARY

extern	struct 	nvtc_softc *nvtc_softc[];
extern	struct	controller *nvtcinfo[];

#else /* BINARY */

#if NNVTC > 0
struct  nvtc_softc *nvtc_softc[NNVTC];
struct	controller *nvtcinfo[NNVTC];
#else
struct 	nvtc_softc *nvtc_softc;
struct	controller *nvtcinfo;
#endif	/* NNVTC > 0 */
#endif	/* BINARY */
