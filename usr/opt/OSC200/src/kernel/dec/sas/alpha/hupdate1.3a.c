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
static char *rcsid = "@(#)$RCSfile: hupdate1.3a.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/07/30 18:30:32 $";
#endif

/*
 *	This file is used to determine if a hardware upgrade install should
 *	be done on this piece of hardware.  The 'hupdate' executable is
 *	loaded by osf_boot into the bootstrap address space.  osf_boot then
 *	executes 'hupdate' and expects a return value of non-zero if 
 *	the hardware requires a hardware upgrade install.
 *
 *	This executable will normally be loaded into the address space
 *	occupied by the primary bootstrap loader.  The same interfaces
 *	are available that are available to osf_boot.
 *
 *	NOTE:  The size of the entire executable can not be larger than 64K,
 *		because it would collide with the osf_boot address space.
 */

#include <machine/rpb.h>
#include <machine/cpuconf.h>

main()
{
    struct rpb *rpb = (struct rpb *)HWRPB_ADDR;

    if (rpb->rpb_systype == ST_DEC_2000_300) {
	printf("Found a DEC 2000 Model 300, using hvmunix\n\n");
	return(1);
    }

    if (rpb->rpb_systype == ST_DEC_4000) {
	if (cobra_nvram_card_present()) {
	    printf("Found a NVRAM memory card for a DEC 4000, using hvmunix\n\n");
	    return(1);
	}
    }
	
    return (0);
}

/*
 * cobra_nvram_card_present() -
 *
 *      returns true if the presto card is present, false otherwise.
 *      needs rpb.h
 */
cobra_nvram_card_present()
{
    struct rpb_mdt *memdsc ;
    struct rpb_cluster *clusterdsc ;
    int i ;
    struct rpb *rpb = (struct rpb *)HWRPB_ADDR;
    
    /*
     * Look through memory descriptors to see if NVRAM is present
     */
    memdsc = (struct rpb_mdt *)((char *)rpb + rpb->rpb_mdt_off);
    for (i = 0; i < (int) memdsc->rpb_numcl; i++) {
	clusterdsc = (struct rpb_cluster *)((char *)(memdsc->rpb_cluster) +
					    (i * sizeof(struct rpb_cluster))) ;
	if (clusterdsc->rpb_usage == CLUSTER_USAGE_NVRAM) {
	    /* found the presto nvram cluster descriptor */
	    return(1);
	}
    }
    /* didn't find NVRAM card */
    return (0);
}
