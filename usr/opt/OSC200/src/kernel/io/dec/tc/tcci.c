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
static char *rcsid = "@(#)$RCSfile: tcci.c,v $ $Revision: 1.1.9.2 $ (DEC) $Date: 1993/08/18 21:48:23 $";
#endif
/*
 * derived from tcci.c	5.2	(ULTRIX)	10/16/91";
 */
/*
 *   Facility:	Systems Communication Architecture
 *		Computer Interconnect Port Driver
 *
 *   Abstract:	This module contains Computer Interconnect Port Driver( CI )
 *		TC based port autoconfiguration routines.
 *
 *   Creator:	Peter Keilty	Creation Date:	Apr. 01, 1991
 *
 *   Functions/Routines:
 *
 *   tcciconf			TC to CI Autoconfiguration Glue Routine
 *
 *   Modification History:
 *
 *   16-Oct-1991	Brian Nadeau
 *	Use new TC_CITCA define.
 *
 */

/* Libraries and Include Files.
 */
#include		<sys/types.h>
#include		<sys/time.h>
#include		<dec/binlog/errlog.h>
#include		<sys/param.h>
#include		<kern/kalloc.h>
#include		<machine/scb.h>
#include		<io/dec/scs/scs.h>
#include		<io/dec/sysap/sysap.h>
#include		<io/dec/ci/cippd.h>
#include		<io/dec/np/npport.h>
#include		<io/dec/np/npadapter.h>
#include		<io/dec/tc/tc.h>

/* External Variables and Routines.
 */
extern	NPISR		ci_isr[];
extern	void		np_unmapped_isr(), citca_isr();
extern	PCCB 		*np_probe();
extern  u_int		np_nci_supported;
extern  int		numci, nNCI, (*ciint0[])();
extern  ihandler_id_t	*handler_add();
extern  int		handler_enable();

/*
 *   Name:	tcciconf	- TC to CI Autoconfiguration Glue Routine
 *
 *   Abstract:	This routine is invoked whenever the TC configuration routine
 *		discovers a CI port during autoconfiguration.
 *
 *   Inputs:
 *
 *   0				- Interrupt processor level
 *   ci_isr			- Vector of CI Interrupt Service Blocks
 *   cpu			- CPU type code
 *   cpusw			- CPU switch structure
 *   numci			- Number of CI ports AND number of this CI port
 *   nxv			- Adapter I/O space virtual address
 *   nxp			- Adapter I/O space physical address
 *   slot			- slot number
 *   intr			- interrupt service routine pointer
 *
 *   Outputs:
 *
 *   0				- Interrupt processor level
 *   npadap			- Target CI Adapter Interface block pointer
 *				   ( INITIALIZED as required )
 *   numci			- Number of CI ports 
 *
 *   SMP:	No locks are required.  This function is only called during
 *		system initialization and at that time only the processor
 *		executing this code is operational.  This guarantees
 *		uncompromised access to all data structures without the added
 *		complications of locking.
 */
int
tcciconf( binfo, bus )
    caddr_t	binfo;
    struct bus  *bus;
{
    struct tc_info 	*tinfo = (struct tc_info *)binfo;
    NPADAP		*npadap;
    PCCB		*pccb;
    int	 		hpt;
    int	 		chnlnum;

    /* For the "current" CI adapter to be autoconfigured the following criteria
     * must be met:
     *
     * 1. The currently supported number of CI adapters/system( NCI_SUPPORTED )
     *	  must not have already been exceeded.
     * 2. The configured number of CI adapters/system( nNCI ) must not have
     *    already been exceeded.
     *
     * Failure to meet any one of these criteria results in both an appropriate
     * message and failure to autoconfigure the "current" CI adapter.  Else,
     * the "current" CI adapter undergoes autoconfiguration as follows:
     *
     * 1. The next available CI adapter interface structure is initialized.
     * 2. The CI adapter's SCB interrupt vector is initialized.
     * 3. The CI port driver probe routine is invoked to initiate CI adapter
     *    initialization.
     * 4. Mark the CI adapter alive within the configuration database.
     */
    ( void )printf( "ci%d at tc%d slot %d ", numci, tinfo->unit, tinfo->slot );
    if(((( struct _tca_reg * )tinfo->addr )->tca_dtype & TC_DTYPE ) == 
	   TC_CITCA ) {
	( void )printf( "(CITCA)\n" );
	hpt = HPT_CITCA;
    } else {
	( void )printf( "(0x%04x)\n", 
		(( struct _tca_reg * )tinfo->addr )->tca_dtype );
	hpt = 0;
    }

    if( hpt == 0 ) {
	( void )printf( "ci%d has unsupported hardware port type\n", numci );
    } else if( numci >= np_nci_supported ) {
	( void )printf( "ci%d unsupported\n", numci );
    } else if( numci >= nNCI ) {
	( void )printf( "ci%d not configured\n", numci );
    } else {
        SCA_KM_ALLOC( npadap, NPADAP *, 8192, KM_SCA, KM_NOW_CL_CA )
        if( npadap == 0 ) {
            ( void )np_log_initerr( numci, ICT_TC, hpt, FE_INIT_NOMEM );
            ci_isr[ numci ].isr = np_unmapped_isr;
        } else {
	    npadap->viraddr = (u_char *)tinfo->addr;
	    npadap->phyaddr = (u_char *)tinfo->physaddr;
	    npadap->Tcnum = tinfo->unit;
	    npadap->Tcnode = tinfo->slot;
	    npadap->npages = NP_ADAPSIZE;

	    chnlnum = 0;
	    pccb = np_probe( numci,tinfo->addr,ICT_TC,hpt,chnlnum,npadap,bus );
	    if( pccb ) {
		ihandler_t handler;
		ihandler_id_t *id;
		struct tc_intr_info info;
		
                info.configuration_st = (caddr_t)bus;
                info.config_type = TC_ADPT;
                info.intr = ( int (*)() )citca_isr;
                info.param = (caddr_t)pccb;
                handler.ih_bus_info = (char *)&info;
		handler.ih_bus = tinfo->bus_hd;

                if(( id = handler_add(&handler)) == NULL ) {
                        printf("tcciconf: failed handler_add\n");
                        return(0);
                }
                if( handler_enable( id ) != 0 ) {
                        printf("tcciconf: failed handler_enable\n");
                        return(0);
                }

                ci_isr[ numci ].isr = citca_isr;
	        npadap->isr[chnlnum] = citca_isr;

	    }
        }
    }
    numci++;
    return(1);
}
