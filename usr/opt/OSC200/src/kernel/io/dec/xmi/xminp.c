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
static char *rcsid = "@(#)$RCSfile: xminp.c,v $ $Revision: 1.1.7.3 $ (DEC) $Date: 1993/08/02 12:35:06 $";
#endif
/*
 * derived from	xminp.c		5.2	(ULTRIX)	10/16/91
 */
/************************************************************************
 *
 *   Facility:	Systems Communication Architecture
 *		Computer Interconnect Port Driver
 *
 *   Abstract:	This module contains Computer Interconnect Port Driver( CI )
 *		XMI based port autoconfiguration routines.
 *
 *   Creator:	Pete Keilty	Creation Date:	June 08, 1991
 *
 *   Functions/Routines:
 *
 *   xminpinit			XMI to CI Autoconfiguration Glue Routine
 *
 *   Modification History:
 *
 *   04-Feb-1992	Carol Sheridan
 *	Ported to Alpha/OSF
 *
 *   16-Oct-1991	Brian Nadeau
 *	NPORT updates/bug fixes.
 *
 *   06-Jun-1990	Pete Keilty
 */

/* Libraries and Include Files.
 */
#include		<sys/types.h>
#include		<sys/time.h>
#include		<dec/binlog/errlog.h>
#include		<sys/param.h>
#include		<sys/vmmac.h>
#include		<machine/scb.h>

#include		<io/dec/xmi/xmireg.h>
#include		<io/dec/scs/scs.h>
#include		<io/dec/sysap/sysap.h>
#include		<io/dec/np/npport.h>
#include		<io/dec/np/npadapter.h>

#ifdef __alpha
#include		<io/dec/mbox/mbox.h>
#include		<mach/error.h>
#include		<vm/pmap.h>
#endif

/* External Variables and Routines.
 */
extern	NPISR		ci_isr[];
extern	PCCB		*np_probe();
extern	void		cimna_isr(), np_unmapped_isr(), cirsp_isr();
extern  u_int		np_nci_supported;
extern  int		numci, nNCI, (*ciintv[])(); 
#ifdef __alpha
extern	void		intrsetvec();
extern	u_short		vecoffset();
#endif

/*
 *   Name:     xminpinit	- XMI to N_PORT CI Autoconfiguration Glue 
 *				  Routine
 *
 *   Abstract: This routine is invoked whenever the XMI configuration routine
 *	       discovers a CI port during autoconfiguration.
 *
 *   Inputs:
 *
 *   0				- Interrupt processor level
 *   ci_isr			- Vector of CI Interrupt Service Blocks
 *   cpu			- CPU type code
 *   cpusw			- CPU switch structure
 *   numci			- Number of CI ports AND no. of this CI port
 *   nxv			- Adapter I/O space virtual address
 *   nxp			- Adapter I/O space physical address
 *   xmidata			- XMI device/adapter information
 *   xminode			- XMI node number
 *   xminumber			- XMI number
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
#ifdef __alpha
xminpinit( nxv, nxp, xminumber, xminode, xmidata, xmibus )
    volatile struct xmi_reg *nxv;	/* virtual pointer to XMI node */
    volatile struct xmi_reg *nxp;	/* virtual pointer to XMI node */
    register int xminumber;		/* bus_num from bus structure  */
    register int xminode;
    register struct xmidata *xmidata;
    struct bus *xmibus;
{
    NPADAP	*npadap;
    u_int	hpt;
    u_int	chnlnum;
    u_int 	dtype;
    vm_offset_t	*vecaddr0;
    vm_offset_t	*vecaddr1;
    PCCB	*pccb;
    struct bus	*ci_bus;

    /* For the "current" CI adapter to be autoconfigured the following 
     * criteria must be met:
     *
     * 1. The currently supported number of CI adapters/system
     *    ( NCI_SUPPORTED ) must not have already been exceeded.
     * 2. The configured number of CI adapters/system( nNCI ) must not have
     *    already been exceeded.
     *
     * Failure to meet any one of these criteria results in both an 
     * appropriate message and failure to autoconfigure the "current" 
     * CI adapter.  Else, the "current" CI adapter undergoes 
     * autoconfiguration as follows:
     *
     * 1. The next available CI adapter interface structure is initialized.
     * 2. The CI adapter's SCB interrupt vector is initialized.
     * 3. The CI port driver probe routine is invoked to initiate CI adapter
     *    initialization.
     * 4. Mark the CI adapter alive within the configuration database.
     */

#ifdef CI_DEBUG
    printf(">>> xminpinit: entered\n");
#endif /* CI_DEBUG */
    ( void )printf( "ci%d at xmi%d node %d ", numci, xminumber, xminode );

/* Call get_bus to find the ci bus structure in the static bus list */
    if ( !( (ci_bus = get_bus("ci", xminode, xmibus->bus_name, xminumber)) ||
	(ci_bus = get_bus("ci", xminode, xmibus->bus_name, -1 )) ||
	(ci_bus = get_bus("ci", -1, xmibus->bus_name, xminumber)) ||
	(ci_bus = get_bus("ci", -1, xmibus->bus_name, -1 )) ||
	(ci_bus = get_bus("ci", -1, "*", -99 )) ) ) {
		/* If the bus structure is not found, allocate a new bus
		   structure for the CI since there was not in the 
		   config file */
		ci_bus = (struct bus *)kalloc( sizeof(struct bus) );
		if (ci_bus == 0) {
			(void)np_log_initerr(numci,ICT_XMI,HPT_CIMNA,FE_INIT_NOMEM);
			ci_isr[numci].isr = np_unmapped_isr;
		}
		bzero(ci_bus, sizeof( struct bus ));
		ci_bus->bus_name = "ci";
    }

/* Initialize the bus structure */
    ci_bus->bus_type = BUS_CI;
    ci_bus->bus_num = numci;
    ci_bus->slot = xminode; 
    ci_bus->connect_bus = xmibus->bus_name; 
    ci_bus->connect_num = xminumber; 
    ci_bus->pname = "np";
    ci_bus->alive |= ALV_ALIVE; 

/* Initialize the bus' mailbox */
    MBOX_GET(xmibus,ci_bus);

/* Connect the ci bus to the xmi bus */
    conn_bus(xmibus,ci_bus);

/* Read XMI device register */
    dtype = RDCSR(LONG_32,ci_bus,&nxv->xmi_dtype);
    switch( dtype & XMIDTYPE_TYPE ) {

	case XMI_CIMNA:
	    ( void )printf( "(CIMNA)\n" );
	    hpt = HPT_CIMNA;
	    break;
	default:
	    ( void )printf( "(0x%04x)\n", dtype);
	    hpt = 0;
	    break;
    }
    if( hpt == 0 ) {
	( void )printf("ci%d has unsupported hardware port type\n",numci);
    } else if( numci >= np_nci_supported ) {
	( void )printf( "ci%d unsupported\n", numci );
    } else if( numci >= nNCI ) {
	( void )printf( "ci%d not configured\n", numci );
    } else {
	npadap = (NPADAP *) kalloc(sizeof( NPADAP )); 
        if( npadap == 0 ) {
           ( void )np_log_initerr( numci, ICT_XMI, hpt, FE_INIT_NOMEM);
           ci_isr[ numci ].isr = np_unmapped_isr;
        } else {
	   npadap->viraddr = nxv;
	   npadap->phyaddr = nxp;
	   npadap->npages = NP_ADAPSIZE;
	   npadap->Xminum = xminumber;
	   npadap->Xminode = xminode;
           ci_isr[ numci ].isr = cimna_isr;
	   chnlnum = 0;
	   npadap->isr[chnlnum] = cimna_isr;
	   npadap->Mna_aid = xmidata->xmiintr_dst;
#ifdef CI_DEBUG
	   printf("xminpinit: npadap->Mna_aid = %lx\n", npadap->Mna_aid);
#endif /* CI_DEBUG */
           pccb = (PCCB *)np_probe(numci,nxv,ICT_XMI,hpt,chnlnum,npadap,ci_bus);
	   if( (allocvec(1,&vecaddr0) != KERN_SUCCESS) ||
	       (allocvec(1,&vecaddr1) != KERN_SUCCESS) ) {
		  ( void )np_log_initerr( numci, ICT_XMI, hpt, FE_INIT_NOMEM);
		  ci_isr[ numci ].isr = np_unmapped_isr;
	   }
#ifdef CI_DEBUG
	    printf("xminpinit: vecaddr1 = %lx\n",vecaddr1);
	    printf("xminpinit: vecaddr0 = %lx\n",vecaddr0);
#endif /* CI_DEBUG */
            npadap->Mna_amiv = ( (int)vecoffset(vecaddr1) | XMI_LEVEL15 );
            npadap->Mna_aciv = ( (int)vecoffset(vecaddr0) | XMI_LEVEL15 );
#ifdef CI_DEBUG
	    printf("xminpinit: npadap->Mna_amiv = %x, npadap->Mna_aciv = %x\n", npadap->Mna_amiv,npadap->Mna_aciv); 
#endif /* CI_DEBUG */
/* vecoffset() returns the 16 bit offset into the scb; intrsetvec() puts the
   address of intr handler & ptr to bus structure in SCB */
	    intrsetvec(vecoffset(vecaddr1),cimna_isr,pccb);
	    intrsetvec(vecoffset(vecaddr0),cirsp_isr,pccb);
	}
    } 
    numci++;
}

#else 	/* vax, mips code to be ported to Silver */
xminpinit( nxv, nxp, xminumber, xminode, xmidata )
    register u_char	*nxv;
    u_char		*nxp;
    register u_long	xminumber;
    register u_long	xminode;
    struct xmidata	*xmidata;
{
    register NPADAP	*npadap;
    register int	hpt;
    register int	chnlnum;

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
    ( void )printf( "ci%d at xmi%d node %d ", numci, xminumber, xminode );
    switch((( struct xmi_reg * )nxv )->xmi_dtype & XMIDTYPE_TYPE ) {

	case XMI_CIMNA:
	    ( void )printf( "(CIMNA)\n" );
	    hpt = HPT_CIMNA;
	    break;
	default:
	    ( void )printf( "(0x%04x)\n",
			    (( struct xmi_reg * )nxv )->xmi_dtype );
	    hpt = 0;
	    break;
    }
    if( hpt == 0 ) {
	( void )printf("ci%d has unsupported hardware port type\n",numci);
    } else if( numci >= np_nci_supported ) {
	( void )printf( "ci%d unsupported\n", numci );
    } else if( numci >= nNCI ) {
	( void )printf( "ci%d not configured\n", numci );
    } else {
        KM_ALLOC( npadap, NPADAP *, sizeof( NPADAP ),KM_SCA,KM_NOW_CL_CA)
        if( npadap == 0 ) {
            ( void )np_log_initerr( numci, ICT_XMI, hpt, FE_INIT_NOMEM);
            ci_isr[ numci ].isr = np_unmapped_isr;
        } else {
	    npadap->viraddr = nxv;
	    npadap->iopte = &Sysmap[ btop((( u_long )nxv ) & 0x7fffffff )];
	    npadap->phyaddr = nxp;
	    npadap->npages = NP_ADAPSIZE;
	    npadap->Xminum = xminumber;
	    npadap->Xminode = xminode;

            ci_isr[ numci ].isr = cimna_isr;
	    chnlnum = 0;
	    npadap->isr[chnlnum] = cimna_isr;
	    npadap->Mna_aid = xmidata->xmiintr_dst;
	    npadap->Mna_amiv = SCB_XMI_LWOFFSET( xminode, LEVEL14 )|XMI_LEVEL15;
	    npadap->Mna_aciv = SCB_XMI_LWOFFSET( xminode, LEVEL14 )|XMI_LEVEL15;
	    *SCB_XMI_VEC_ADDR( xmidata, xminumber, xminode, LEVEL14 ) =
	                 scbentry( ciintv[ numci ], SCB_ISTACK );
	    ( void )np_probe( numci, nxv, ICT_XMI, hpt, chnlnum, npadap );
	    ( void )config_set_alive( "ci", numci, xminumber, xminode );
	}
    } 
    numci++;
}
#endif
