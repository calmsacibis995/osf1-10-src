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
 * @(#)$RCSfile: envram_data.c,v $ $Revision: 1.1.6.5 $ (DEC) $Date: 1993/12/09 20:25:07 $
 */


/*
 * Digital EISA non-voltile RAM driver  (DEC2500)
 */

#include "envram.h"
#include <vm/vm_kern.h>
#include <sys/presto.h>
#include <io/common/devdriver.h>
#include <io/dec/eisa/eisa.h>
#include <machine/rpb.h>
#include <io/dec/eisa/envram_reg.h>

					
/*
 * Define the softc for the EISA NVRAM driver
 */
struct	envram_softc {
    io_handle_t	regbase;	     /* base address for registers */
    io_handle_t cache_phys_start;    /* Physical start address of NVRAM cache*/
    io_handle_t cache_base;	     /* base address of NVRAM in EISA address space */
    vm_offset_t	cache_kseg_start;    /* KSEG start addr of the presto cache*/
    u_long      saved_mem_sysmap;    /* sysmap portion of mem io_handle_t */
    u_int       cache_size;	     /* size of NVRAM cache */
    u_int       cache_offset;        /* Offset to the first nvram location 
					from start of EISA slot address*/
    io_handle_t diag_status;	     /* If the board passed diags or not */
    dma_handle_t sglp;               /* pointer to byte pair list */
    struct controller *ctlr;         /* pointer to nvram controller */
    
};

struct 	envram_softc *envram_softc;
struct	controller *envram_info[NENVRAM];








































