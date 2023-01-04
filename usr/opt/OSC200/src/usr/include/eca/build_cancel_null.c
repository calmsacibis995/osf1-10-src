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
static char *rcsid = "@(#)$RCSfile: build_cancel_null.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:55:03 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**	CANCEL.C
**
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].
**	It provides the entry point for the CANCEL_GET function.
**
**	Note: This MOM does not support a CANCEL_GET function!
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
**  TEMPLATE HISTORY:
**
**--
*/

#include "man_data.h"
#include "man.h"
#include "moss.h"
#include "common.h"


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      MOI_CANCEL_GET
**
**  FORMAL PARAMETERS:
**
**	CANCEL_INVOKE_ID
**	    Invoke ID for the get operation to be canceled.
**
**	INVOKE_ID
**	    Invocation ID.
**
**	HANDLE
**	    Handle for internal use.
**
**  RETURN VALUE:
**
**	MAN_C_DIRECTIVE_NOT_SUPPORTED
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status moi_cancel_get(  cancel_invoke_id,
			    invoke_id,
			    handle)

int                 cancel_invoke_id;
int                 invoke_id;
management_handle   *handle;

{
    man_binding_handle pe_handle;
    int bind_flag = FALSE;
    man_status		status;

#ifdef MOMGENDEBUG
    printf("In the CANCEL_GET routine\n");
#endif

    if (((man_status) _reply_required( handle )) == MAN_C_TRUE)
        {
        status = moss_alloc_pe_handle( handle, &pe_handle );
        if ERROR_CONDITION(status)
          return(MAN_C_INSUFFICIENT_RESOURCES);

        status = moss_send_cancel_get_reply( invoke_id,
			    cancel_invoke_id,
			    MAN_C_DIRECTIVE_NOT_SUPPORTED);

	status = moss_free_pe_handle( pe_handle );
	}
    
    return MAN_C_SUCCESS;
}
