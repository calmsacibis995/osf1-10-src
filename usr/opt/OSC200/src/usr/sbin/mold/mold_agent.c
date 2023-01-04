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
static char *rcsid = "@(#)$RCSfile: mold_agent.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:05:29 $";
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
 *    The following routines define the Agent interface to mold.
 *
 * Routines:
 *    
 *    mold_find_mom()
 *    mold_find_next_mom()
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
 *    Change file names to reflect 14 character restriction.
 *
 *    Miriam Amos Nihart, October 11, 1990.
 *
 *    Release input parameters on mold_find_object().
 *
 *    Wim Colgate, August 20th, 1991.
 *
 *    Added the routine mold_find_next_mom(). This routine supports
 *    the SNMP lexigraphic ordering. retired mold_find_object and
 *    replaced it with mold_find_mom
 *
 *    Kathy Faust, October 7, 1991.
 *
 *    Changed RPC interface to use explicit binding
 */

/*
 *  Support header files
 */  

#include "man_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

/*
 *  Header files
 */

#include "man.h"

/*
 *  MOLD header files
 */

#include "mold_private.h"
#include "moss.h"

#ifndef NOIPC
#include <cma.h>
#endif

/*
 *  Function Prototypes to satisfy c89 -std warnings.
 *
 *   - cma_lib_malloc() has no prototype, it is defined in
 *     /usr/include/dce/cmalib_crtlx.h to replace malloc()
 *
 *   - cma_lib_free() has no prototype, it is defined in
 *     /usr/include/dce/cmalib_crtlx.h to replace free()
 *
 *   - mold_signal_handler() is in mold.c
 *
 *   - mold_dump_init_mold() is in mold.c
 *
 *   - mold_dump_single_object() is in mold.c
 */

extern void mold_signal_handler() ;
extern man_status mold_dump_init_mold() ;
extern man_status mold_dump_single_object() ;

#ifndef NOIPC
extern void *cma_lib_malloc() ;
extern void *cma_lib_free() ;
#endif /* NOIPC */

/*
 * Include files for Internationalization on unix only.
 */

#ifndef VMS
#include "mold_msg_text.h"
#endif /* VMS */

/*
 *  External
 */

extern queue_t lexi_head ;



man_status
mold_find_mom(
	      mold_handle ,
	      base_class,
	      containment_level,
	      return_info 
	      )
man_binding_handle mold_handle ;
object_id *base_class ;
unsigned int containment_level ;
avl *return_info ;

/*
 *
 * Function description:
 *
 *    Find an object. Do so by searching the CONTAINMENT tree.
 *
 * Arguments:
 *
 *    mold_handle         Binding handle of MOLD caller
 *    base_class          Pointer to the base class.
 *    containment_level   Integer specifying the level of the tree
 *                        to search (relative to base class).]
 *    return_info         Pointer to a return AVL handle.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS         Class(es) found
 *    MAN_C_INVALID_SCOPE   Invalid scop value passed
 *    MAN_C_NO_SUCH_CLASS   Class not found
 *    MAN_C_BAD_PARAMETER   base_class or return_info is invalid 
 *
 *    If no objects are found, the return info AVL is empty. 
 *
 * Side effects:
 *
 *    None
 *
 */

{

    object_t *object ;
    man_status status ;
    unsigned char *iscope = ( unsigned char * )&containment_level ;
    octet_string *octet ;
    octet_string return_octet ;
    object_id *user_oid = NULL ;
    char *dump_file = NULL ;
    int i ;
    uid_t owner ;
    gid_t group ;
    char *oid_text = NULL ;
    char *return_text = NULL ;
    extern man_status mold_dump_single_object() ;

    /*
     * Check if base_class is valid. It has to be non-NULL. The count must
     * be at least 1 and value pointer must not be NULL.
     */

     if ( base_class == NULL )
	return( MAN_C_BAD_PARAMETER ) ;

     if ( base_class->count < 1 )
	return( MAN_C_BAD_PARAMETER ) ;

     if ( base_class->value == NULL )
	return( MAN_C_BAD_PARAMETER ) ;

#ifndef NOIPC

    /*
     * This is a hack for the mold_dump program to have the MOLD dump
     * itself to a file.
     */

    /*
     * If the count of the oid is 180, this is a call from mold_dump
     * to dump the MOLD.
     */

    if ( base_class->count == 0x000000b4 )
    {
	/*
	 * Get the dump file name which is passed in the avl.
	 */

	status = moss_avl_point( return_info, NULL, NULL, NULL,
				 &octet, NULL ) ;

	/*
	 * Allocate some memory.
	 */

	if ( status == MAN_C_SUCCESS )
	{
	    MALLOC( dump_file, char *, octet->length + 1 ) ;

	    if ( dump_file == NULL )
		status = MAN_C_PROCESSING_FAILURE ;
	}

	/*
	 * Copy the dump file name for local use.
	 */

	if ( status == MAN_C_SUCCESS )
	{
	    memcpy( dump_file, octet->string, octet->length ) ;
	    memcpy( &dump_file[octet->length], "\0", 1 ) ;
	}

	/*
	 * Call the mold_dump_init_mold() routine to dump the MOLD.
	 */

	if ( status == MAN_C_SUCCESS )
	{
	    status = mold_dump_init_mold( dump_file ) ;
	}

	/*
	 * Get the uid of the caller passed in avl.
	 */

	if ( status == MAN_C_SUCCESS )
	{
	    status = moss_avl_point( return_info, NULL, NULL, NULL,
				     &octet, NULL ) ;
	}

	if ( status == MAN_C_SUCCESS )
	{
	    memcpy( &i, octet->string, 4 ) ;
	    owner = i ;
	}

	/*
	 * Get the gid of the caller passed in avl.
	 */

	if ( status == MAN_C_SUCCESS )
	{
	    status = moss_avl_point( return_info, NULL, NULL, NULL,
				     &octet, NULL ) ;
	}

	if ( status == MAN_C_SUCCESS )
	{
	    memcpy( &i, octet->string, 4 ) ;
	    group = i ;
	}

	/*
	 * Change the owner id and group id of the dump file to
	 * that of the caller.
	 */

	if ( status == MAN_C_SUCCESS )
	{
	    chown( dump_file, owner, group ) ;
	}

	/*
	 * Free resources.
	 */

	if ( dump_file != NULL )
	{
	    FREE( dump_file ) ;
	}

	return( status ) ;
    }

    /*
     * If the count of the oid is 170, this is a call from mold_dump
     * to return the information for a specific oid passed return_info.
     */

    if ( base_class->count == 0x000000aa )
    {
	/*
	 * Get the octet string which contains the oid.
	 */

	status = moss_avl_point( return_info, NULL, NULL, NULL,
				 &octet, NULL ) ;

	/*
	 * Convert the octet string to an object id.
	 */

	if ( status == MAN_C_SUCCESS )
	{
	    status = moss_octet_to_oid( octet, &user_oid ) ;
	}

	if ( status == MAN_C_SUCCESS )
	{
	    /*
	     * Check if the object oid is in MOLD.
	     */

	    object = find_object( user_oid ) ;

	    if ( object == NULL )
	    {
		/*
		 * If not, return text saying so.
		 */

		status = moss_oid_to_text( user_oid, ".", NULL, NULL,
					   &oid_text ) ;

		if ( status == MAN_C_SUCCESS )
		{
		    MALLOC( return_text, char *, ( strlen( oid_text ) +
			strlen( "Object oid is not found in mold." ) + 10 ) ) ;

		    if ( return_text == NULL )
			status = MAN_C_PROCESSING_FAILURE ;
		}

		if ( status == MAN_C_SUCCESS )
		{
		    strcpy( return_text, "\nObject oid " ) ;
		    strcat( return_text, oid_text ) ;
		    strcat( return_text, " is not found in mold.\n" ) ;
		}
	    }
	    else
	    {
		/*
		 * If object oid is in MOLD, call to get the information.
		 */

		status = mold_dump_single_object( user_oid, &return_text ) ;
	    }
	}

	if ( status == MAN_C_SUCCESS )
	{
	    /*
	     * Put the text as the second avl element.
	     */

	    return_octet.length = strlen( return_text ) + 1 ;
	    return_octet.string = return_text ;

	    status = moss_avl_add( return_info, base_class, 0, 0,
				   &return_octet ) ;
	}

	if ( oid_text != NULL )
	    free( oid_text ) ;

	if ( return_text != NULL )
	    free( return_text ) ;

	if ( user_oid != NULL )
	    moss_free_oid( user_oid ) ;

	return( status ) ;
    }

#endif

    /*
     * Only scopes of 0+0, 0+1 and 0+2 or
     * 1+n, 2+n are allowed.
     */

    if ( ( ( iscope[ 1 ] == 0 ) && ( iscope[ 0 ] > 2 ) ) ||
          ( iscope[ 1 ] > 2 ) )
        return( MAN_C_INVALID_SCOPE ) ;

    /*
     * If the pointer to avl is NULL, return error.
     */

    if ( return_info == NULL )
	return( MAN_C_BAD_PARAMETER ) ;

    READ_LOCK() ;

    /*
     * Assume that no objects are found.
     */

    status = MAN_C_NO_SUCH_CLASS ;

    object = find_object( base_class ) ;

    /*
     * Return the scoped objects if the object exists and is registered.
     */

    if ( object != NULL )
    {
	if ( object->state_flag == MAN_C_REGISTERED )
	{
	    status = find_scoped_objects( object, containment_level,
					  return_info ) ;
	}
    }

    READ_UNLOCK() ;

    /*
     *  Release base_class as it is not automatically release by RPC via the stack.
     */

#ifndef SYSV    /* Do this for all systems except System V */
#ifndef NOIPC   /* Don't do this if single image mode */
#ifndef RPCV2   /* Don't do this with DCE RPC */
    free( base_class->value ) ;
#endif /* RPCV2 */
#endif /* NOIPC */
#endif /* SYSV */

    return( status ) ;

}  /* end of mold_find_mom() */


man_status
mold_find_next_mom(
		   mold_handle ,
		   current_class,
		   return_info 
                  )
man_binding_handle mold_handle ;
object_id *current_class ;
avl *return_info ;

/*
 *
 * Function description:
 *
 *    Find the next object. Do so by searching the lexigraphic queue.
 *
 * Arguments:
 *
 *    mold_handle         Binding handle of MOLD caller
 *    current_class          Pointer to the current class.
 *    return_info         Pointer to a return AVL handle.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS         Class found
 *    MAN_C_NO_SUCH_CLASS   Current class was not found.
 *    MAN_C_END_OF_MIB      The current class is the last object in the lexigraphic queue.
 *                          and the return avl is empty
 *    MAN_C_BAD_PARAMETER   current_class or return_info is invalid
 *
 * Side effects:
 *
 *    None
 *
 */

{

    object_t *object = NULL ;
    man_status status = MAN_C_NO_SUCH_CLASS ;
    octet_string man_handle_octet ;
    queue_t *next_q = NULL ;

    /*
     * If the pointer to avl is NULL, return error.
     */

    if ( return_info == NULL )
	return( MAN_C_BAD_PARAMETER ) ;

    READ_LOCK() ;

    /*
     * If there is a current class, then use it, otherwise, 
     * start at the beginning of the MIB (lexigraphic list).
     */

    if ( current_class == NULL )
    {
	next_q = lexi_head.flink ;
    }
    else
    {
	if ( current_class->count > 0 )
	{
	    if ( current_class->value != NULL )
	    {
		object = find_object( current_class ) ;
		if ( object != NULL ) 
		    next_q = object->lexi_q.flink ;
	    }
	    else
	    {
		return( MAN_C_BAD_PARAMETER ) ;
	    }
	}
	else
	{
	    next_q = lexi_head.flink ;
	}
    }

    /*
     * If the next object is NULL, then the current object is only
     * a member of the containment hierarchy, and thusly there is
     * no next object.
     */

    if ( next_q == NULL )
    {
        return( MAN_C_NO_SUCH_CLASS ) ;
    }

    if ( next_q != &lexi_head )
    {
        object = CONTAINING_RECORD_BY_NAME( next_q, object_t, lexi_q ) ;
        man_handle_octet.length = object->man_handle.length ;
        man_handle_octet.string = (char *)( object->man_handle.socket_address ) ;
        status = moss_avl_add( return_info, 
                               &( object->oid ), 
                               object->supported_interface, 
                               ASN1_C_NULL, 
                               &( man_handle_octet ) ) ;
    }
    else
        status = MAN_C_END_OF_MIB ;

    READ_UNLOCK() ;

    /*
     *  Release base_class as it is not automatically release by RPC via the stack.
     */

#ifndef SYSV    /* Do this for all systems except System V */
#ifndef NOIPC   /* Don't do this if single image mode */
#ifndef RPCV2   /* Don't do this with DCE RPC */
    free( current_class->value ) ;
#endif /* RPCV2 */
#endif /* NOIPC */
#endif /* SYSV */

    return( status ) ;

}  /* end of mold_find_next_mom() */

/* end of mold_agent.c */
