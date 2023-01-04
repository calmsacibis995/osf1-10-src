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
static char *rcsid = "@(#)$RCSfile: build_init_perform_instance.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:22:05 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**	[[class_prefix]]PERFORM_INIT.C
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

EXPORT [[class_name]]_DEF    *new_[[class_name]]_header;


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
**          ***************************************************************
**          Note:  This routine does NOT do instantiation!  malloc() is
**                 used only to do class initialization.  Leave that first
**                 element in the doubly-linked-list blank!  To load 
**                 instance data, allocate memory using malloc() (even 
**		   for a single instance), add the instance to the 
**		   doubly-linked-list using [[class_name]]_add_new_instance(), 
**		   and then populate it!  It is recommended that a separate
**		   module be created to maintain the instance information
**		   which can then be called from the MOM routines that
**                 need this information.
**          ***************************************************************
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
    
    /*
     |  Perform class_initialization by creating the first element in
     |  the doubly-linked-list.  This element is to be left blank!  For
     |  instantiation, do another malloc(), add that element to the
     |  end of the linked-list using [[class_name]]_add_new_instance, and
     |  then populate it.
     */
    new_[[class_name]]_header = ([[class_name]]_DEF *) malloc(sizeof( [[class_name]]_DEF ));
    if (new_[[class_name]]_header  == NULL)
        return MAN_C_INSUFFICIENT_RESOURCES;

    /** This section of code may not be needed if the MOM stores its data
     ** outside of this process.
     **/
    new_[[class_name]]_header->next = new_[[class_name]]_header; 	 
    new_[[class_name]]_header->prev = new_[[class_name]]_header; 

    /** Perform any [[class_prefix]]specific required initialization. **/

    if (FALSE) /** error encountered during initialization **/
       {
       free( new_[[class_name]]_header );
       return MAN_C_PROCESSING_FAILURE;
       }

    return MAN_C_SUCCESS;
} /* End of [[class_prefix]]perform_init() */

/*-insert-code-init-instance-*/

/* End of [[class_prefix]]perform_init.c */
