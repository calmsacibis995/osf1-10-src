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
 * @(#)$RCSfile: gen_bus_adapt.h,v $ $Revision: 1.1.8.3 $ (DEC) $Date: 1993/08/18 19:18:59 $
 */

#ifndef _GEN_BUS_ADAPT_H
#define _GEN_BUS_ADAPT_H

#include <sys/buf.h>
#include <io/dec/vme/vbareg.h>
#include <io/common/devdriver.h>
#include <io/dec/tc/tc.h>

/*
 * Generic bus_adapter structure.
 * 
 */

struct	gen_bus_adapt {
	struct	gen_bus_adapt *next;	/* pointer to next in list */
	int	xbanum;		/* ba number passed in to adapter code */
	int	adptnum;	/* internal number of adapter per bus type */
	caddr_t virt_addr;		/* virt addr of bus adapter */
	caddr_t phys_addr;		/* phys addr of bus adapter */
#ifdef __alpha
	struct scbentry *intr_vec;	/* interrupt vectors */
#else /* __alpha */
	int	(**intr_vec)();		/* interrupt vectors */
#endif /* __alpha */
	caddr_t	adapt_regs;		/* ptr to adapter registers	*/
	caddr_t	adapt_vars;		/* ptr to adapter specific variables */
	caddr_t	bus_vars;		/* ptr to bus specific variables    */
	short	bus_want;		/* someone is waiting for bus space */
	caddr_t reserved[5];
	/* adapter specific rtns */
	int	(*display_addr_type)(); /* displays atype during config */
	int	(*is_ivec_valid)();	/* is interrupt vector valid? */
	caddr_t	(*map_csr)();		/* map csr (for pio) */
	void	(*unmap_csr)();		/* unmap csr (for pio) */
	u_int	(*setup)();	        /* set up dma mapping */
	void	(*release)();		/* release dma mapping */
	void 	(*log_ctlr_err)();	/* log controller error */
	void 	(*log_dev_err)();	/* log device error */
	int	(*rmw)();		/* do read/modify/write */
	u_long	(*get_bus_addr)();      /* get address on target bus */
	int	(*errors)();		/* Error routine for adapter */
	char    (*read_byte)();         /* rtn to read a byte register */
	short   (*read_word)();         /* rtn to read a short register */
	int     (*read_long)();         /* rtn to read an int register */
	void    (*write_byte)();        /* rtn to write a byte register */
	void    (*write_word)();        /* rtn to write a short register */
	void    (*write_long)();        /* rtn to write an int register */
};

#define xbahd private[4]

extern int gen_ba_ins();
extern struct gen_bus_adapt *gen_ba_get();
extern caddr_t gen_ba_map_csr();
extern void gen_ba_unmap_csr();
extern u_int gen_ba_setup();
extern u_int gen_ba_alloc();
extern void gen_ba_release();
extern void gen_ba_log_ctlr_err();
extern void gen_ba_log_dev_err();
extern int gen_ba_rmw();
extern u_long gen_ba_get_bus_addr();
extern int gen_ba_config_bus();
extern u_long gen_ba_vatophys();

#endif /* _GEN_BUS_ADAPT_H */
