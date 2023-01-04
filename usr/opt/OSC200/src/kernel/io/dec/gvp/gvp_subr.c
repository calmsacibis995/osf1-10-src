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
static char *rcsid = "@(#)$RCSfile: gvp_subr.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/07/13 17:08:16 $";
#endif
/*
 * derived from gvp_subr.c	4.1	(ULTRIX)	7/2/90";
 */
/************************************************************************
 *                                                                      *
 *                      Copyright (c) 1988 - 1989 by                    *
 *              Digital Equipment Corporation, Maynard, MA              *
 *                      All rights reserved.                            *
 *                                                                      *
 *   This software is furnished under a license and may be used and     *
 *   copied  only  in accordance with the terms of such license and     *
 *   with the  inclusion  of  the  above  copyright  notice.   This     *
 *   software  or  any  other copies thereof may not be provided or     *
 *   otherwise made available to any other person.  No title to and     *
 *   ownership of the software is hereby transferred.                   *
 *                                                                      *
 *   The information in this software is subject to change  without     *
 *   notice  and should not be construed as a commitment by Digital     *
 *   Equipment Corporation.                                             *
 *                                                                      *
 *   Digital assumes no responsibility for the use  or  reliability     *
 *   of its software on equipment which is not supplied by Digital.     *
 *                                                                      *
 ************************************************************************
 *
 *
 *   Facility:	Systems Communication Architecture
 *		Generic Vaxport Port Driver
 *
 *   Abstract:	This module contains Generic Vaxport Port Driver( GVP )
 *		miscellaneous functions and routines.  Some of these
 *		functions are invokable only by port drivers while others
 *		are for use only by SCS.
 *
 *   Creator:	Todd M. Katz	Creation Date:	September 24, 1987
 *
 *   Function/Routines:
 *
 *   gvp_info_gvp		Return Generic Vaxport Driver Information
 *   gvp_initialize		Initialize Generic Vaxport Driver
 *
 *   Modification History:
 *
 *   31-Oct-1991	Pete Keilty
 *	Changed memeory allocation to OSF/1 memory zones. 
 *	Routines are in scs_subr.c.
 *
 *   27-Feb-1991	Tom Tierney
 *	ULTRIX to OSF/1 port:
 *
 *   20-May-1989	Pete Keilty
 *	Added support for mips risc cpu's double mapped buffer & pte's
 *	into a Vaxmap of the system.
 *	CI/BVP ports need to have VAX pte's and system addresses.	
 *
 *   06-Apr-1989	Pete Keilty
 *	Add include file smp_lock.h
 *
 *   07-Mar-1989	Todd M. Katz		TMK0002
 *	Include header file ../vaxmsi/msisysap.h.
 *
 *   02-Jun-1988     Ricky S. Palmer
 *	Removed inclusion of header file ../vaxmsi/msisysap.h
 *
 *   08-Jan-1988	Todd M. Katz		TMK0001
 *	Formated module, revised comments, increased robustness, made GVP
 *	completely independent from underlying port drivers, restructured code
 *	paths, and added SMP support.
 */

/* Libraries and Include Files.
 */
#include		<sys/types.h>
#include		<sys/param.h>
#include		<sys/systm.h>
#include		<sys/vmmac.h>
#include 		<mach/vm_param.h>
#include		<dec/binlog/errlog.h>
#include		<kern/lock.h>
#include		<machine/cpu.h>
#include		<hal/cpuconf.h>
#include		<io/dec/scs/scs.h>
#include		<io/dec/sysap/sysap.h>
#include		<io/dec/gvp/gvp.h>

/* External Variables and Routines.
 */
extern	int		cpu;
extern	SCSIB		lscs;
extern	GVPBDDB		*gvp_bddb;
extern	u_long		gvp_max_bds, gvp_queue_retry;
extern	void		( *gvp_info )(), gvp_info_gvp();
extern  caddr_t		sca_zalloc(), sca_zget();
extern  int		sca_zfree(), sca_zones_init(), sca_zones_initialized;
extern  struct zone 	*sca_zone[];

/*   Name:	gvp_info_gvp	- Return Generic Vaxport Driver Information
 *
 *   Abstract:	This function returns generic Vaxport driver information.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   gvp_bddb			- Generic Vaxport Buffer Descriptor Database
 *   gvp_queue_retry		- Queuing failure retry account
 *   scsib			- SCS Information Block pointer
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   scsib			- SCS Information Block pointer
 *	gvp_qretry		-  GVP queuing failure retry count
 *	gvp_free_bds		-  Number of free GVPBDs
 *
 *   SMP:	The GVP buffer descriptor database is locked to postpone all
 *		modifications to the free GVPBD list while it is being
 *		traversed.
 */
void
gvp_info_gvp( scsib )
    SCSIB		*scsib;
{
    GVPBD		*bd;

    /* The following current items of Generic Vaxport Driver information are
     * returned:
     *
     * 1. GVP queuing failure retry count.
     * 2. Number of free GVPBDs.
     *
     * The GVP buffer descriptor database is locked while the free GVPBD
     * list is traversed.
     */
    scsib->gvp_qretry = gvp_queue_retry;
    Lock_gvpbd()
    for( bd = gvp_bddb->free_bd, scsib->gvp_free_bds = 0;
	 bd;
	 bd = bd->next_free_bd, ++scsib->gvp_free_bds ) {}
    Unlock_gvpbd()
}

/*   Name:	gvp_initialize	- Initialize Generic Vaxport Driver
 *
 *   Abstract:	This function initializes the generic Vaxport driver and all of
 *		its data structures.  Each port driver may call this function
 *		once during its own initialization.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   gvp_bddb			- Generic Vaxport Buffer Descriptor Database
 *   gvp_max_bds		- Maximum number of Buffer Descriptors
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   gvp_bddb			- Generic Vaxport Buffer Descriptor Database
 *				  ( INITIALIZED )
 *   lscs			- Local system permanent information
 *	max_gvpbds		-  Maximum number of GVPBDs
 *
 *   Return Values:
 *
 *   RET_SUCCESS		- GVP successfully initialized
 *   RET_ALLOCFAIL		- Storage allocation failure occurred
 *
 *   SMP:	No locks are required even though shared data structures are
 *		manipulated.  This function is only called during system
 *		initialization and at that time only the processor executing
 *		this code is operational.  This guarantees uncompromised access
 *		to any shared data structure without locking.
 *
 *		The GVP buffer descriptor database lock structure is
 *		initialized.
 */
u_long
gvp_initialize()
{
    GVPBD		*bdp;
    long		index;
    u_long		size, status = RET_SUCCESS;

    /* Immediately return success if the GVP has already been initialized.
     * Otherwise, initialize the generic Vaxport driver as follows:
     *
     * 1. Allocate and zero all the dynamic memory required for the generic
     *	  Vaxport buffer descriptor database.  Immediately return if
     *	  insufficient dynamic memory is available.
     * 2. Initialize relevant local system information.
     * 3. Initialize the variable containing the address of GVP routine
     *	  returning GVP information.
     * 4. Initialize the GVP buffer descriptor database.
     * 5. Initialize the key and lock structure portions of each GVPBD and
     *	  place all the GVPBDs onto the GVPBD free list.
     * 6. Return an appropriate status.
     */
    if( gvp_bddb == 0 ) {
	size = sizeof( GVPBDDB ) + ( sizeof( GVPBD ) * gvp_max_bds );
	while( size > PAGE_SIZE ) {/* OSF/MIPS max. contigous space, for now */
	    size -= sizeof( GVPBD );/* Generic kernels have large numbers of */
	    gvp_max_bds--;	    /* DSSC and HSC controllers.             */
	}
	SCA_KM_ALLOC( gvp_bddb, GVPBDDB *, size, KM_SCA, KM_NOW_CL_CA )
	if( gvp_bddb ) {
	    gvp_info = gvp_info_gvp;
	    lscs.max_gvpbds = gvp_max_bds;
	    gvp_bddb->bdt = ( GVPBD * )( gvp_bddb + 1 );
	    U_long( gvp_bddb->size ) = size;
	    gvp_bddb->type = DYN_GVPBDDB;
	    Init_gvpbd_lock()
	    for( index = ( lscs.max_gvpbds - 1 ), bdp = gvp_bddb->bdt + index;
		 index >= 0;
		 --index, --bdp ) {
		bdp->key = 1;
		bdp->next_free_bd = gvp_bddb->free_bd;
		gvp_bddb->free_bd = bdp;
	    }
#if 	OSF
#else	/* OSF */
	    Dm_bddbifISIS
#endif	/* OSF */
	} else {
	    status = RET_ALLOCFAIL;
	}
    }
    return( status );
}
