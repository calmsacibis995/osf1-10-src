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
static char *rcsid = "@(#)$RCSfile: mold_mo.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:05:35 $";
#endif
#ifndef lint
static char *sccsid = "%W%	DECwest	%G%" ;
#endif

/****************************************************************************
 *
 * Copyright (c) Digital Equipment Corporation, 1989, 1990, 1991, 1992.
 * All Rights Reserved.  Unpublished rights reserved
 * under the copyright laws of the United States.
 *
 * The software contained on this media is proprietary
 * to and embodies the confidential technology of
 * Digital Equipment Corporation.  Possession, use,
 * duplication or dissemination of the software and
 * media is authorized only pursuant to a valid written
 * license from Digital Equipment Corporation.
 *
 * RESTRICTED RIGHTS LEGEND   Use, duplication, or
 * disclosure by the U.S. Government is subject to
 * restrictions as set forth in Subparagraph (c)(1)(ii)
 * of DFARS 252.227-7013, or in FAR 52.227-19, as
 * applicable.
 *
 ****************************************************************************
 *
 *
 * Facility:
 *
 *    Management - POLYCENTER (tm) Common Agent
 *
 * Abstract:
 *
 *    The following routines define the interface for the Manageable Object.
 *
 * Routines:
 *
 *    mold_register_mom()
 *    mold_deregister_mom()
 *    register_containment()
 *    register_lexi()
 *
 *
 * Author:
 *
 *    Wim Colgate
 *
 * Date:
 *
 *    October 25, 1989
 *
 * Revision History :
 *
 *    Miriam Amos Nihart, May 14th, 1990.
 *
 *    Change the file names to reflect the 14 character restriction.
 *
 *    Miriam Amos Nihart, October 11th, 1990.
 *
 *    Release the input parameters to prevent unneccessay memory consumption.
 *
 *    Miriam Amos Nihart, October 29th, 1990.
 *
 *    Change bcopy to ansi equivalent memcpy.
 *
 *    Wim Colgate
 *
 *    Retired routine mold_register_object(). Replaced it with mold_register_mom(),
 *    and added routines register_containment() and register_lexi().
 *    Only mold_register_mom is externally accessable.
 */

/*
 *  Function Prototypes to satisfy c89 -std warnings.
 *
 *   - copy_oid() is in mold.c
 */

extern void copy_oid() ;

/*
 *  Support header files
 */

#include <string.h>  

/*
 *  Header files
 */

#include "man_data.h"
#include "man.h"

/*
 *  MOLD header files
 */

#include "mold_private.h"
#include "moss.h"

/*
 *  External data
 */

extern family_relation_t *containment_head ; 
extern queue_t lexi_head ;


man_status
mold_register_mom(
		  mold_handle,
                  man_handle,
                  parent_id,
                  mom_id,
                  registration_type,
                  supported_interface,
                  pid
                 )
man_binding_handle mold_handle ;
management_handle *man_handle ;
object_id *parent_id ;
object_id *mom_id ;
unsigned int registration_type ;
unsigned int supported_interface ;
unsigned int pid ;

/*
 *
 * Function description:
 *
 *     Register the object in the necessary locations as dictated by the insertion
 *     type argument
 *
 * Arguments:
 *
 *    mold_handle        MOLD binding handle
 *    man_handle         A pointer to the management handle.
 *    parent_id          A pointer to the parent class object id (may be NULL for global entities, or objects
 *                       being inserted into the lexigraphic ordering only). 
 *    mom_id             A pointer to the object id of the object being registered.
 *    registration_type  A bit-mask that represents the type(s) of insertion required.
 *    supported_interface A bit-mask that represents the type(s) of interfaces supported.
 *    pid                The process identification of the managed object module.
 *
 * Return value:
 *
 *    MAN_C_NO_SUCH_PARENT_CLASS    Parent class does not exist (containment only)
 *    MAN_C_OBJECT_ALREADY_EXISTS   Class already registered
 *    MAN_C_SUCCESS                 Class registered successfully
 *    MAN_C_PROCESSING_FAILURE      An error occured attempting to register the object
 *    MAN_C_INVALID_ARGUMENT_VALUE  An illegal combination of insertion types present
 *    MAN_C_BAD_PARAMETER           The mom_id is NULL or has a count of 0 or
 *                                  a value of NULL
 *
 * Side effects:
 *
 */

{

    man_status mstatus ;
    object_t *object ;

    /*
     * Check if mom_id is valid. It has to be non-NULL. The count must be at
     * least 1 and value pointer must not be NULL.
     */

    if ( mom_id == NULL )
	return( MAN_C_BAD_PARAMETER ) ;

    if ( mom_id->count < 1 )
	return( MAN_C_BAD_PARAMETER ) ;

    if ( mom_id->value == NULL )
	return( MAN_C_BAD_PARAMETER ) ;

    /*
     * If you are going to register a mom, you better have at least
     * ONE registration type and interface! 
     */

    if ( registration_type == 0 ||
         supported_interface == 0 )
        return( MAN_C_INVALID_ARGUMENT_VALUE ) 

    WRITE_LOCK() ;

    /*
     * Check to see if the object already exists
     */

    object = find_object( mom_id ) ;
    if ( object != NULL )
    {
	if ( object->state_flag == MAN_C_REGISTERED )
	{
	    WRITE_UNLOCK() ;
	    return( MAN_C_OBJECT_ALREADY_EXISTS ) ;
	}
    }

    /*
     * When we come here, either the object is not in MOLD at all or
     * it is in MOLD but the state is MAN_C_PREREGISTERED or
     * MAN_C_DEREGISTERED.
     */

    /*
     * Now, based off of the registration type bit mask we will do our work. 
     * NOTE that the registration types are NOT mutually exclusive.
     */

    if ( registration_type & MAN_M_CONTAINMENT )
    {
        mstatus = register_containment( man_handle, parent_id, mom_id, &object ) ;

        if ( ( mstatus != MAN_C_SUCCESS ) &&
	     ( mstatus != MAN_C_OBJECT_ALREADY_EXISTS ) )
        {
             WRITE_UNLOCK() ;
             return( mstatus ) ;
        }
    }

    if ( registration_type & MAN_M_LEXIGRAPHIC )
    {
        mstatus = register_lexi( man_handle, mom_id, &object ) ;
        if ( ( mstatus != MAN_C_SUCCESS ) &&
	     ( mstatus != MAN_C_OBJECT_ALREADY_EXISTS ) )
        {
             WRITE_UNLOCK() ;
             return( mstatus ) ;
        }
    }

    /*
     * We only update the state_flag to MAN_C_REGISTERED only if
     * the man_handle is not NULL. Otherwise just marked it as
     * MAN_C_PREREGISTERED.
     */
     
    if ( man_handle != NULL )
    {
	object->state_flag = MAN_C_REGISTERED ;
    }
    else
    {
	object->state_flag = MAN_C_PREREGISTERED ;
    }

    object->supported_interface = supported_interface ;

    WRITE_UNLOCK() ;

    return( MAN_C_SUCCESS ) ;

}  /* end of mold_register_mom() */


man_status
register_containment( 
                     man_handle,
                     parent_class,
                     obj_class,
                     return_object 
                    )

management_handle *man_handle ;
object_id *parent_class ;
object_id *obj_class ;
object_t **return_object ;

/*
 *
 * Function description:
 *
 *     Register the object in the containment hierarchy.
 *
 * Arguments:
 *
 *    man_handle        A pointer to the management handle.
 *    parent_class      A pointer to the parent object class.
 *    obj_class         A pointer to the object class.
 *    return_object     An address of the pointer to an object structure.
 *
 * Return value:
 *
 *    MAN_C_NO_SUCH_PARENT_CLASS    Parent class does not exist
 *    MAN_C_SUCCESS                 Class registered successfully
 *    MAN_C_BAD_PARAMETER           obj_class is NULL or has a count of 0 or
 *                                  a value of NULL or parent_class has a
 *                                  count of non-zero but a value of NULL
 *    MAN_C_PROCESSING_FAILURE      Failed to create object
 *
 * Side effects:
 *
 */

{

    object_t *parent_object ;
    object_t *object ;
    man_status status ;
    object_id nil_oid ;

    /*
     * Check if obj_class is valid. It has to be non-NULL. The count must be at
     * least 1 and value pointer must not be NULL.
     */

    if ( obj_class == NULL )
	return( MAN_C_BAD_PARAMETER ) ;

    if ( obj_class->count < 1 )
	return( MAN_C_BAD_PARAMETER ) ;

    if ( obj_class->value == NULL )
	return( MAN_C_BAD_PARAMETER ) ;

    /*
     * Make sure there is a parent class. If parent_class is NULL, point it
     * to nil_object_id.
     */

    if ( parent_class == NULL )
    {
	nil_oid.count = 0 ;
	nil_oid.value = NULL ;
	parent_class = &nil_oid ;
    }
    else
    {
	if ( ( parent_class->count > 0 ) &&
	     ( parent_class->value == NULL ) )
	{
	    return( MAN_C_BAD_PARAMETER ) ;
	}
    }

    /*
     * The count and value fields must not be 0 and NULL.
     */

    if ( ( parent_class->count > 0 ) &&
	 ( parent_class->value != NULL ) )
    {
        parent_object = find_object( parent_class ) ;

        if ( parent_object == NULL )
            return( MAN_C_NO_SUCH_PARENT_CLASS ) ;
    }

    /*
     * Find the object first. It might be there but with state of
     * of MAN_C_PREREGISTERED or MAN_C_DEREGISTERED or MAN_C_REGISTERED.
     */

     object = find_object( obj_class ) ;
    
    /*
     * If the object did not already exists, then populate the data
     * structure.
     */

    if ( object == NULL )
    {
	/*
	 * Create the object. If there was some problem, return a
	 * processing. failure. This is not quite correct, but the
	 * best we can do.
	 */

	object = create_object( obj_class ) ;

	if ( object == NULL )
	    return( MAN_C_PROCESSING_FAILURE ) ;

	copy_oid( &( object->oid ), obj_class ) ;
    }

    /*
     * If the object is already in the containment tree, do not
     * re-insert it. The assumption here is that the sibling pointers
     * within the family relation structure are always non-zero if the
     * object is inserted into the containment hierarchy.
     */

    if ( object->relation.fsibling != NULL )
    {
	status = MAN_C_OBJECT_ALREADY_EXISTS ;
    }
    else
    {
	/*
	 * If there is a parent class, mark it in the structure
	 */

	if ( ( parent_class->count > 0 ) &&
	     ( parent_class->value != NULL ) )
	{
	    copy_oid( &( object->parent_oid ), parent_class ) ;
	}
	else
	{
	    object->parent_oid.count = 0 ;
	    object->parent_oid.value = NULL ;
	}

	/*
	 * Now insert it into the containment tree used for scoping.
	 */

	if ( ( parent_class->count > 0 ) &&
	     ( parent_class->value != NULL ) )
	{
	    insert_containment( &containment_head, 
				&( parent_object->relation ), 
				&( object->relation ), 
				&( object->oid ) ) ;
	}
	else
	{
	    insert_containment( &containment_head, 
				NULL, 
				&( object->relation ), 
				&( object->oid ) ) ;
	}

	status = MAN_C_SUCCESS ;
    }

    /* 
     * Copy the management handle, if the caller specified one.
     * (When we are registering an object.
     */

    if ( man_handle != NULL )
    {
	object->man_handle.length = man_handle->length ;
	memcpy( object->man_handle.socket_address ,
		man_handle->socket_address ,
		object->man_handle.length ) ;
    }

    *return_object = object ;

    return( MAN_C_SUCCESS ) ;

}  /* end of register_containment() */


static
man_status
register_lexi( 
              man_handle,
              obj_class,
              return_object
             )

management_handle *man_handle ;
object_id *obj_class ;
object_t **return_object ;

/*
 *
 * Function description:
 *
 *     Register the object in the lexigraphic hierarchy.
 *
 * Arguments:
 *
 *    man_handle        A pointer to the management handle.
 *    obj_class         A pointer to the object class.
 *    return_object     An adress of a pointer to an object structure.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS                 Class registered successfully
 *
 * Side effects:
 *
 */

{

    object_t *object ;

    /*
     * The object may have just been created by the containment code 
     * path (remember, the containment and lexigraphic 'lists' are not
     * mutually exclusive
     */

    object = find_object( obj_class ) ;

    if ( object == NULL )
        object = create_object( obj_class ) ;
   
    /*
     * Populate the data structure with the needed info. 
     */

    copy_oid( &( object->oid ), obj_class ) ;

    /* 
     * Copy the management handle. 
     */

    object->man_handle.length = man_handle->length ;
    memcpy( object->man_handle.socket_address ,
            man_handle->socket_address ,
            object->man_handle.length ) ;

    /*
     * Now insert it into the lexigraphic list 
     */

    insert_lexi( &lexi_head, &( object->lexi_q ), &( object->oid ) ) ;

    *return_object = object ;

    return( MAN_C_SUCCESS ) ;

}  /* end of register_lexi() */


man_status
mold_deregister_mom( 
		    mold_handle,
		    man_handle,
		    obj_class 
		    )

man_binding_handle mold_handle ;
management_handle *man_handle ;
object_id *obj_class ;

/*
 *
 * Function description:
 *
 *     Deregister the object.
 *
 * Arguments:
 *
 *    mold_handle      MOLD binding handle
 *    man_handle       A pointer to the management handle.
 *    obj_class        A pointer to the object class.
 *
 * Return value:
 *
 *    MAN_C_NO_SUCH_CLASS         Class not previously registered
 *    MAN_C_HAS_ACTIVE_CHILDREN   Class cannot be deregistered because of children
 *    MAN_C_SUCCESS               Class deregistered successfully
 *    MAN_C_BAD_PARAMETER         obj_class is NULL or has a count of 0 or
 *                                a value of NULL
 * Side effects:
 *
 *    None.
 */
{

    int is_it_new ;
    object_t *object ;

    WRITE_LOCK() ;

    /*
     * Check if obj_class is valid. It has to be non-NULL. The count must be at
     * least 1 and value pointer must not be NULL.
     */

    if ( obj_class == NULL )
	return( MAN_C_BAD_PARAMETER ) ;

    if ( obj_class->count < 1 )
	return( MAN_C_BAD_PARAMETER ) ;

    if ( obj_class->value == NULL )
	return( MAN_C_BAD_PARAMETER ) ;

    /*
     * Find the object.
     */

    object = find_object( obj_class ) ;

    if ( object == NULL )
    {
        WRITE_UNLOCK() ;
        return ( MAN_C_NO_SUCH_CLASS ) ; 
    }

    /*
     * If the object is in MOLD but marked MAN_C_DEREGISTERED or
     * MAN_C_PREREGISTERED, return MAN_C_NO_SUCH_CLASS.
     */

    if ( object->state_flag != MAN_C_REGISTERED )
    {
	WRITE_UNLOCK() ;
	return( MAN_C_NO_SUCH_CLASS ) ;
    }

    /*
     * If we come here, the object is in MOLD and marked MAN_C_REGISTERED.
     * Mark the object as deregistered.
     */

    object->state_flag = MAN_C_DEREGISTERED ;

    WRITE_UNLOCK() ;

    return( MAN_C_SUCCESS ) ;

}  /* end of mold_deregister_mom() */

/* end of mold_mo.c */
