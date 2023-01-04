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
static char *rcsid = "@(#)$RCSfile: build_perform_init.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:22:54 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**      [[class_prefix]]PERFORM_INIT.C
**
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].
**      It performs class-specific initialization for the
**	[[class]] class.
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
#include "common.h"


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      perform_init
**
**	    This routine performs any class-specific initialization
**	    which is required before management directives can be
**	    received for this class.
**
**          *************************************************************
**          Note:  This routine does NOT do instantiation!  malloc() is
**                 used only to do class initialization.  Leave that first
**                 element in the doubly-linked-list blank!  To load
**                 instance data, allocate memory using malloc() (even
**                 for a single instance), add the instance to the
**                 doubly-linked-list using [[class_name]]_add_new_instance(),
**                 and then populate it!  It is recommended that a separate
**                 module be created to maintain the instance information
**                 which can then be called from the MOM routines that
**                 need this information.
**          *************************************************************
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
man_status  [[class_prefix]]perform_init()
{
    /** Perform any required initialization. **/

    if (FALSE) /** error encountered during initialization **/
       return MAN_C_PROCESSING_FAILURE;

    return MAN_C_SUCCESS;
} /* End of [[class_prefix]]perform_init() */
