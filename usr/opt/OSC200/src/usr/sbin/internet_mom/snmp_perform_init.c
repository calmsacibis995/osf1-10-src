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
static char *rcsid = "@(#)$RCSfile: snmp_perform_init.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/09/07 21:25:51 $";
#endif
/*
**++
**  FACILITY:  LKG2-1
**
**  Copyright (c) 1993  Digital Equipment Corporation
**
**  MODULE DESCRIPTION:
**
**	snmp_PERFORM_INIT.C
**
**      This module is part of the Managed Object Module (MOM)
**	for rfc1213.
**      It performs class-specific initialization for the
**	snmp_ class.
**
**  AUTHORS:
**
**      Muhammad I. Ashraf
**
**      This code was initially created with the 
**	Ultrix MOM Generator - version X1.1.0
**
**  CREATION DATE:  22-Feb-1993
**
**  MODIFICATION HISTORY:
**
**
**  TEMPLATE HISTORY:
**
**--
*/

#ifdef VMS
#include "ssdef.h"
#include "syidef.h"
#include "moss_dna.h"
#endif /* VMS */
#include "man_data.h"
#include "man.h"
#include "moss.h"
#include "common.h"
#include "iso_defs.h"
#include "snmp_mib.h"
#include <sys/ipc.h>

static int shared_mem_id;
struct snmp_stats *s_stat;

EXPORT snmp_DEF    *new_snmp_header;


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      perform_init
**
**	This routine performs any class-specific initialization
**	which is required before management directives can be
**	received for this class.  This routine must be called before
**	the class is registered.
**
**	This routine also allocates and initializes the instance header 
**      queue.
**
**  FORMAL PARAMETERS:
**
**	NONE
**
**  RETURN VALUE:
**
**	MAN_C_SUCCESS			Normal success
**	MAN_C_PROCESSING_FAILURE	Unexpected (fatal) error
**--
*/
man_status  snmp_perform_init()
{
    
    new_snmp_header = (snmp_DEF *) malloc(sizeof( snmp_DEF ));
    if (new_snmp_header  == NULL)
        return MAN_C_INSUFFICIENT_RESOURCES;

    /** This section of code may not be needed if the MOM stores its data
     ** outside of this process.
     **/
    new_snmp_header->next = new_snmp_header; 	 
    new_snmp_header->prev = new_snmp_header; 

    /** Perform any snmp_specific required initialization. **/

    if (FALSE) /** error encountered during initialization **/
       {
       free( new_snmp_header );
       return MAN_C_PROCESSING_FAILURE;
       }

    /**
     **  Set up the snmp statics structure.  This structure exists in
     **  shared memory so that the SNMP engine may update the stats and
     **  the internet MOM may manage it.
     **/

     if ((shared_mem_id = shmget(SHARED_MEM_KEY,
                                sizeof(struct snmp_stats),
                                IPC_CREAT | 0600)) < 0) {
        if ((shared_mem_id = shmget(SHARED_MEM_KEY,
                                    sizeof(struct snmp_stats),
                                    0)) < 0)
        {
	     return(MAN_C_FAILURE);
        }
     }

     if ((s_stat = (struct snmp_stats *) shmat(shared_mem_id, 0, 0)) <
	  (struct snmp_stats *) 0) {

	  return(MAN_C_FAILURE);
     }

    return MAN_C_SUCCESS;
}
