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
static char *rcsid = "@(#)$RCSfile: build_trap.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:23:57 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**	TRAP.C
**
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].
**	It provides the entry points for the init_trap and
**	send_<trap_name>_trap functions if any.
**
**  AUTHORS:
**
**      [[author]]
**
**      This code was initially created with the 
**	[[system]] MOM Generator - version [[version]]
**
**  CREATION DATE:  [[creation_date]]       
**
**  MODIFICATION HISTORY:
**
**
**--
*/

#include "moss.h"
#include "extern_nil.h"
#include "common.h"
#include "evd_defs.h"
#include "moss_inet.h"

# define enterprise_specific_trap 6

extern man_status evd_create_queue_handle();
extern man_status evd_post_event();

static unsigned int enterprise_event_type = enterprise_specific_trap;
static object_id event_type_oid = { 1, (unsigned int *)
                                    &enterprise_event_type};

/*-insert-code-trap-export-defs-*/

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      init_trap
**
**	This routine is called from the routine mom_init() in init.c.
**	If there are any traps defined, for each trap, this routine
**	initializes this global trap variable list structure called
**	<trap_name>_traplist defined in this module.
**
**	NOTE: This routine may be modified to include any initialization
**	      necessary for sending traps.
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/

man_status init_trap()

{
    /*
     * Modify the following if necessary for initialization.
     * Add other necessary initialization for sending traps.
     */

/*-insert-code-trap-arg-inits-*/

    return MAN_C_SUCCESS;

}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      send_trap
**
**      This routine is called from each of the send_<trap_name>_trap
**      routines in this module for sending a particular trap.
**
**
**  FORMAL PARAMETERS:
**
**      object_class            - OID of the managed object class
**                                generating the trap
**      instance_name           - AVL containing the instance name of the
**                                managed object generating the trap
**      event_type              - OID of the type of trap
**      event_parameters        - AVL containing the event parameters to
**                                evd_post_event() call.
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS
**      failure status from evd_create_queue_handle() or
**      evd_post_event() call.
**
**  SIDE EFFECTS:
**
**      A trap is sent to the evd.
**
**--
*/

static man_status send_trap( object_class,
                             instance_name,
                             event_type,
                             event_parameters )
object_id       *object_class;
avl             *instance_name;
object_id       *event_type;
avl             *event_parameters;

{
    man_status status;
    evd_queue_handle *handle;

    /*
     * Create a queue handle to the evd.
     */

    status = evd_create_queue_handle( &handle, nil_avl, EVD_POST );
    if ERROR_CONDITION(status)
        return status;

    /*
     * Post the trap to the evd.
     */

    status = evd_post_event( handle,
                             object_class,
                             instance_name,
                             &nil_time,
                             event_type,
                             event_parameters,
                             &nil_uid,
                             &nil_uid );
    if ERROR_CONDITION(status)
	return status;

    /*
     * Delete the queue handle to the evd.
     */

    status = evd_delete_queue_handle( &handle );

    return status;

}

/*-insert-code-trap_arg_code-*/
