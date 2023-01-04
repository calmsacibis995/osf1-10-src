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
static char *rcsid = "@(#)$RCSfile: man_rpc.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 15:59:24 $";
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
 *    The following routines are the Managed Object's call interfaces' RPC
 *    marshalling routines.
 *        object_id_from_xmit
 *        object_id_to_xmit
 *        object_id_free_xmit
 *        avl_from_xmit
 *        avl_to_xmit
 *        avl_free_xmit
 *        free_avl_from_xmit
 *        free_avl_to_xmit
 *        free_avl_free_xmit
 *        managemnt_handle_from_xmit
 *        managemnt_handle_to_xmit
 *        managemnt_handle_free
 *        managemnt_handle_free_xmit
 *        man_binding_handle_bind
 *        man_binding_handle_unbind
 *
 * Author:
 *
 *    Miriam Nihart
 *
 * Date:
 *
 *    November 21, 1989
 *
 * Revision History :
 *
 *
 *    Wim Colgate, March 28th, 1990.
 *
 *    Added a separate avl_to_xmit routine to free avl. Now one does and one 
 *    doesn't free the avl upon transmission. This was needed because some
 *    routines (like mold_find_object) does not have a chance to free the 
 *    generated data before returning to RPC listen.
 *
 *    Miriam Amos Nihart, May 14, 1990.
 *
 *    Correct include file names to be restricted to less than 14 characters.
 *    Also rename file from man_rpc_marshall.c to man_rpc.c
 *
 *    Miriam Amos Nihart, October 29, 1990.
 *
 *    Change the bzero and bcopy to the ansi equivalents memset and memcpy.
 *
 *    Miriam Amos Nihart, November 8th, 1990.
 *
 *    Changes for the moss_avl_init() and moss_avl_free() calls.
 *
 *    Jim Teague, March 4th, 1991.
 *
 *    Port to DCE RPC.  Most of the the change here was adjusting the
 *    user marshalling routine names to match the required format.
 *    NOTE: because of a bug in the NIDL-generated stubs, the routines that
 *    free the transmitted representation of the data are commented out.
 *    This will have to be changed when the bug is fixed.
 *    
 *    Miriam Amos Nihart, October 16th, 1991.
 *
 *    Fixes for prototyping.
 *
 *    Kathy Faust, October 25th, 1991.
 *
 *    Added man_binding_handle_bind and man_binding_handle_unbind for
 *    explicit binding with customized handle.
 *
 *    Kathy Faust, November 14th, 1991.
 *
 *    Fixed bugs in _free_inst marshalling routines so that free of
 *    allocated data occurs, not free of stack data.  Uncommented the
 *    _free_xmit routines because the bug in the NIDL-generated stubs
 *    has been fixed.
 *
 *    Wim Colgate, November 18th, 1991.
 *
 *    removed one level of indirection from free_avl_free_xmit
 *
 *    Miriam Amos Nihart, December 3rd, 1991.
 *
 *    Put in macros for malloc and free.
 *    
 */

/*
 *  Function Prototypes to satisfy c89 -std warnings.
 *
 *   - cma_lib_malloc() has no prototype, it is defined in
 *     /usr/include/dce/cmalib_crtlx.h to replace malloc()
 *
 *   - cma_lib_free() has no prototype, it is defined in
 *     /usr/include/dce/cmalib_crtlx.h to replace free()
 */

#include "man_data.h"


#include <stdio.h>

#ifndef NOIPC
extern void *cma_lib_malloc() ;
extern void *cma_lib_free() ;
#else
#include <stdlib.h>
#endif /* NOIPC */

/*
 *  Header files
 */

#ifndef NOIPC
#include "rpc.h"
#endif /* NOIPC */

#include <string.h>
#include "man.h"
#include "moss.h"
#include "moss_private.h"
#include "extern_nil.h"

/*
 *  External
 */

/*
 *  Data
 */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef NULL
#define NULL 0
#endif


void
object_id_from_xmit(
                        xmit_object ,
                        object
                       )
/*
 *
 * Function description:
 *
 *    This routine unmarshalls the transmitted object into an object id.
 *
 * Arguments:
 *
 *    xmit_object    address of the data recieved
 *    object         address of the local data type
 *
 * Return value:
 *
 *    The address of the unmarshalled data is returned in object.
 *
 * Side effects:
 *
 */

obj_id_trans_t *xmit_object ;
object_id *object ;
{
    unsigned int *new_int_array_p ;
    int values ;
    unsigned int *read_int_p ;

    object->count = xmit_object->count ;

    if ( xmit_object->count != 0 )
    {
        MOSS_MALLOC( new_int_array_p, unsigned int, xmit_object->count * sizeof( int ) )
        object->value = new_int_array_p ;

        read_int_p = xmit_object->value ;

        for ( values = 0 ; values < xmit_object->count ; values++ )
            *new_int_array_p++ = *read_int_p++ ;
    }
    else
        object->value = ( unsigned int * )NULL ;

} /* end of object_id_from_xmit() */


void
object_id_to_xmit(
                        object ,
                        xmit_object
                     )
/*
 *
 * Function description:
 *
 *    This routine marshalls an object id into a transmittable format.
 *
 * Arguments:
 *
 *    object         local data type to be marshalled
 *    xmit_object    address to place the transmit data
 *
 * Return value:
 *
 *
 * Side effects:
 *
 */

object_id *object ;
obj_id_trans_t **xmit_object ;
{
    int size ;
    int values ;
    obj_id_trans_t *transmit_buffer_p ;
    unsigned int *read_int_p ;
    unsigned int *write_int_p ;

    size = ( object->count * sizeof( unsigned int ) ) + ( sizeof( int ) ) ;

    MOSS_MALLOC( transmit_buffer_p, obj_id_trans_t, size )

    transmit_buffer_p->count = object->count ;
    if ( object->count != 0 )
    {
        read_int_p = object->value ;
        write_int_p =  transmit_buffer_p->value ;
        for ( values = 0 ; values < object->count ; values++ )
            *write_int_p++ = *read_int_p++ ;
    }

    *xmit_object = transmit_buffer_p ;

} /* end of object_id_to_xmit() */

void
object_id_free_xmit(
		    xmit_object
		    )
/*
 *
 * Function description:
 *
 *    This routine frees the transmittable format of an object id.
 *
 * Arguments:
 *
 *    xmit_object    address of the transmited data to be freed
 *
 * Return value:
 *
 *
 * Side effects:
 *
 */

obj_id_trans_t *xmit_object ;
{
    MOSS_FREE( xmit_object ) ;
} /* end of object_id_free_xmit() */

void
object_id_free_inst (
		     native_object_id
		     )
/*
 * Free a "native" object id.
 */
object_id *native_object_id;
{
    /*
     *  Can only what was allocated
     */

    if ( native_object_id->value != ( unsigned int *)NULL ) 
    {
        MOSS_FREE( native_object_id->value ) ;
        native_object_id->value = ( unsigned int * )NULL ;
    }
}


void
avl_from_xmit(
                  xmit_object ,
                  object
                 )
/*
 *
 * Function description:
 *
 *    This routine unmarshalls the transmitted object into an AVL.
 *
 * Arguments:
 *
 *    xmit_object    address of the data recieved
 *    object         address of the local data type
 *
 * Return value:
 *
 *    The address of the unmarshalled data is returned in object.
 *
 * Side effects:
 *
 *    The transmitted buffer is still in use.
 *
 */

avl_trans_t *xmit_object ;
avl *object ;
{
    man_status status ;

    /*
     *  AVL is given to us by RPC, but may have garb in it.
     *  This guarantees a clean one.
     */

    if ( object != ( avl * )NULL )
        memset( ( void * )object, '\0', sizeof( avl ) ) ;

    status = moss_avl_init( &object ) ; 

    if ( xmit_object->buf_len != 0 )
    {
        status = moss_avl_from_buf( object, ( char * )xmit_object ) ;
        if ( status != MAN_C_SUCCESS )
            fprintf( stderr, "Error with moss_avl_from_buf call.\n" ) ;
    }
    
} /* end of avl_from_xmit() */


void
avl_to_xmit(
                object ,
                xmit_object
               )
/*
 *
 * Function description:
 *
 *    This routine marshalls an AVL into a transmittable format.
 *    Note that the xmit_object (AVL) is not affected.
 *
 * Arguments:
 *
 *    object         local data type to be marshalled
 *    xmit_object    address to place the transmit data
 *
 * Return value:
 *
 *
 * Side effects:
 *
 */

avl *object ;
avl_trans_t **xmit_object ;
{
    man_status status ;
    avl_trans_t *transmit_buffer_p = NULL;
    int dummy = 0;

    /*
     * Convert the AVL into a buffer.
     *
     * The buffer length is actually the first int of the buffer.
     */

    status = moss_avl_to_buf( object ,
                             ( char **)&( transmit_buffer_p ) ,
                             &( dummy ) ) ;

    /*
     *  We must pass a null AVL_trans_t structure for RPC.
     */

    if ( status == MAN_C_NO_ELEMENT )
    {
        MOSS_MALLOC( transmit_buffer_p, avl_trans_t, sizeof ( avl_trans_t ) )
        transmit_buffer_p->buf_len = 0 ;
        *( transmit_buffer_p->avl ) = 0 ;
    }

    if ( ( status != MAN_C_SUCCESS ) && ( status != MAN_C_NO_ELEMENT ) )
        fprintf( stderr, "Error with moss_avl_to_buf call.\n" ) ;

    *xmit_object = transmit_buffer_p ;
} /* end of avl_to_xmit() */



void
avl_free_xmit(
	      xmit_object
	      )
/*
 *
 * Function description:
 *
 *    This routine frees the transmittable format of an AVL.
 *
 * Arguments:
 *
 *    xmit_object    address of the transmited data to be freed
 *
 * Return value:
 *
 *
 * Side effects:
 *
 */

avl_trans_t *xmit_object ;
{
    MOSS_FREE( xmit_object );
} /* end of avl_free_xmit() */



void
free_avl_from_xmit(
                       xmit_object ,
                       object
                      )
/*
 *
 * Function description:
 *
 *    This routine unmarshalls the transmitted object into an AVL.
 *
 * Arguments:
 *
 *    xmit_object    address of the data recieved
 *    object         address of the local data type
 *
 * Return value:
 *
 *    The address of the unmarshalled data is returned in object.
 *
 * Side effects:
 *
 *    The transmitted buffer is still in use.
 *
 */

free_avl_trans_t *xmit_object ;
free_avl *object ;
{
    man_status status ;

    /*
     *  AVL is given to us by RPC, but may have garb in it.
     *  This guarantees a clean one.
     */

    if ( object != ( free_avl * )NULL )
        memset( ( void * )object, '\0', sizeof( free_avl ) ) ;

    status = moss_avl_init( ( avl ** )&object ) ; 
    if ( status != MAN_C_SUCCESS )
        fprintf( stderr, "Error with call mos_avl_init from free_avl_from_xmit - %d\n",
	       status ) ;

    if ( xmit_object->buf_len != 0 )
    {
        status = moss_avl_from_buf( ( avl * )object, ( char * )xmit_object ) ;
        if ( status != MAN_C_SUCCESS )
            fprintf( stderr, "Error with moss_avl_from_buf call.\n" ) ;
    }
    
} /* end of free_avl_from_xmit() */


void
free_avl_to_xmit(
                     object ,
                     xmit_object
                    )
/*
 *
 * Function description:
 *
 *    This routine marshalls an AVL into a transmittable format.
 *    And frees the AVL. This is used by routines returning data
 *    as a result of an RPC.
 *
 * Arguments:
 *
 *    object         local data type to be marshalled
 *    xmit_object    address to place the transmit data
 *
 * Return value:
 *
 *
 * Side effects:
 *
 */

free_avl *object ;
free_avl_trans_t **xmit_object ;
{
    man_status status ;
    free_avl_trans_t *transmit_buffer_p ;
    int dummy ;
    avl *dummy_avl ;
    iavl *iavl_handle ;
    avl_element *element ;

    /*
     * Convert the AVL into a buffer.
     *
     * The buffer length is actually the first int of the buffer.
     */

    status = moss_avl_to_buf( (avl *)object ,
                             ( char ** )&( transmit_buffer_p ) ,
                             &( dummy ) ) ;

    /*
     *  We must pass a null AVL_trans_t structure for RPC.
     */

    if ( status == MAN_C_NO_ELEMENT )
    {
        MOSS_MALLOC( transmit_buffer_p, free_avl_trans_t, sizeof ( free_avl_trans_t ) )
        transmit_buffer_p->buf_len = 0 ;
        *( transmit_buffer_p->avl ) = 0 ;
    }

    if ( ( status != MAN_C_SUCCESS ) && ( status != MAN_C_NO_ELEMENT ) )
        fprintf( stderr, "Error with moss_avl_to_buf call.\n" ) ;

    /*
     * Because we created this AVL on the fly, and we will never return by this
     * way again, we should free the AVL lest all the memory accumulate and
     * we die a horrible death due to insufficient resources.
     */

    dummy_avl = ( avl * )object ;
    moss_avl_free( ( avl ** )&object, FALSE ) ;

    /*
     * The moss_avl_free() above only frees the AVL elements, the AVL is still
     * initialized and can be reused ( there is still an AVL header and an
     * empty AVL element ). We only want to get rid of the empty AVL element
     * because in the stub file mold_sstub.c generated by DCE idl compiler,
     * the AVL header is actually stack memory, not allocated on the heap.
     * Therefore we cannot use TRUE in the moss_avl_free() call. Also in the
     * stub file mold_cstub.c generated by DCE idl compiler, the AVL header
     * is always clear, thus without the following fix, we will leak 44 bytes
     * ( which is the size of an AVL element ) in the PE.
     */

    if ( object != ( free_avl * )NULL )
    {
        iavl_handle = ( iavl * )object ;
        element = iavl_handle->first_avl ;

        if ( element != ( avl_element * )NULL )
        {
            MOSS_FREE( element ) ;
        }

        memset( ( void * )object, '\0', sizeof( free_avl ) ) ;
    }

    *xmit_object = transmit_buffer_p ;
} /* end of free_avl_to_xmit() */
                                                                    


void
free_avl_free_xmit(
		   xmit_object 
		   )

/*
 *
 * Function description:
 *
 *    This function calls a separate marshall routine. Just as 
 *    the normal avl marshalling routine.
 *
 * Arguments:
 *
 *    object         local data type to be marshalled
 *    xmit_object    address to place the transmit data
 *
 * Return value:
 *
 *
 * Side effects:
 *
 */

free_avl_trans_t *xmit_object ;
{
    MOSS_FREE( xmit_object ) ;
} /* end of free_avl_free_xmit */

void free_avl_free_inst (
			 native_free_avl
			 )
/*
 * Free a "native" free_avl.
 */
free_avl *native_free_avl;
{
    /*
     *  Can only free what has been allocated by free_avl_from_xmit
     */

    moss_avl_free( ( avl ** )&native_free_avl, FALSE ) ; 

} /* end of free_avl_free_inst */


void
management_handle_from_xmit(
                                xmit_object ,
                                object
                               )
/*
 *
 * Function description:
 *
 *    This routine unmarshalls the transmitted object into a management handle.
 *
 * Arguments:
 *
 *    xmit_object    address of the data recieved
 *    object         address of the local data type
 *
 * Return value:
 *
 *    The address of the unmarshalled data is returned in object.
 *
 * Side effects:
 *
 */

man_handle_trans_t *xmit_object ;
management_handle *object ;
{
    memcpy( object, xmit_object->management_handle, xmit_object->buf_len ) ;

} /* end of management_handle_from_xmit() */


void
management_handle_to_xmit(
                              object ,
                              xmit_object
                             )
/*
 *
 * Function description:
 *
 *    This routine marshalls a management handle into a transmittable format.
 *
 * Arguments:
 *
 *    object         local data type to be marshalled
 *    xmit_object    address to place the transmit data
 *
 * Return value:
 *
 *
 * Side effects:
 *
 */

management_handle *object ;
man_handle_trans_t **xmit_object ;
{
    int size ;
    man_handle_trans_t *transmit_buffer_p ;

    size = sizeof( management_handle ) + sizeof( int ) ;

    MOSS_MALLOC( transmit_buffer_p, man_handle_trans_t, size )

    transmit_buffer_p->buf_len = sizeof( management_handle ) ;
    memcpy( transmit_buffer_p->management_handle, object, 
		sizeof( management_handle ) ) ;

    *xmit_object = transmit_buffer_p ;

} /* end of management_handle_to_xmit() */


void
avl_free_inst(
	      object
	      )
/*
 *
 */

avl *object;

{
    iavl *iavl_handle ;
    avl_element *element ;

    /*
     *  Can only free what has been allocated by avl_from_xmit
     */
    man_status status ;

    status = moss_avl_free( ( avl ** )&object, FALSE ) ; 
    if ( status != MAN_C_SUCCESS )
        fprintf(stderr, "Error on moss_avl_free() in avl_free_inst %d\n", status ) ;

    /*
     * The moss_avl_free() above only frees the AVL elements, the AVL is still
     * initialized and can be reused ( there is still an AVL header and an
     * empty AVL element ). We only want to get rid of the empty AVL element
     * because in the stub file mo_sstub.c and pe_sstub.c generated by DCE
     * idl compiler, the AVL header is actually stack memory, not allocated
     * on the heap. Therefore we cannot use TRUE in the moss_avl_free() call.
     * Without the following fix, we will leak 44 bytes ( which is the size
     * of an AVL element ) per AVL passed both in the MOM and PE.
     */

    if ( status == MAN_C_SUCCESS )
    {
        if ( object != ( avl * )NULL )
        {
            iavl_handle = ( iavl * )object ;
            element = iavl_handle->first_avl ;

            if ( element != ( avl_element * )NULL )
            {
                MOSS_FREE( element ) ;
            }

            memset( ( void * )object, '\0', sizeof( avl ) ) ;
        }
    }

    /* free (*object); */
} /* end of avl_free_inst */



void
management_handle_free_inst(
			    object
			    )
/*
 *
 * Function description:
 *
 *    This routine frees a management handle.
 *
 * Arguments:
 *
 *    object    local object id to be freed
 *
 * Return value:
 *
 *
 * Side effects:
 *
 */

management_handle *object ;
{
    /*
     *  management_handle is allocated on the stack by 
     *  management_handle_from_xmit, so this routine is a noop.
     *
     *  free( object ) ;
     */

} /* end of management_handle_free() */


void
management_handle_free_xmit(
			    xmit_object
			    )
/*
 *
 * Function description:
 *
 *    This routine frees the transmittable format of a management handle.
 *
 * Arguments:
 *
 *    xmit_object    address of the transmited data to be freed
 *
 * Return value:
 *
 *
 * Side effects:
 *
 */

man_handle_trans_t *xmit_object ;
{
    MOSS_FREE( xmit_object ) ;

} /* end of management_handle_free_xmit() */

handle_t
man_binding_handle_bind(
			man_handle
			)

/*
 *
 * Function description:
 *
 *    This routine frees the transmittable format of a management handle.
 *
 * Arguments:
 *
 *    xmit_object    address of the transmited data to be freed
 *
 * Return value:
 *
 *
 * Side effects:
 *
 */

man_binding_handle man_handle ;
{
    return( (handle_t)man_handle ) ;
} /* end of man_binding_handle_bind() */

void
man_binding_handle_unbind(
			  man_handle ,
			  rpc_handle
			  )

/*
 *
 * Function description:
 *
 *    This routine frees the transmittable format of a management handle.
 *
 * Arguments:
 *
 *    xmit_object    address of the transmited data to be freed
 *
 * Return value:
 *
 *
 * Side effects:
 *
 */

man_binding_handle man_handle ;
handle_t rpc_handle ;
{
} /* end of man_binding_handle_unbind() */

/* end of man_rpc.c */
