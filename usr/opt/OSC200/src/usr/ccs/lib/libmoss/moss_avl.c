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
static char *rcsid = "@(#)$RCSfile: moss_avl.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:31:32 $";
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
 *    The following contains Attribute Value List (AVL) support for 
 *    manipulating Attribute Value Lists.
 *
 * Routines:
 *
 *    moss_avl_init()
 *    moss_avl_start_construct()
 *    moss_avl_end_construct()
 *    moss_avl_reset()
 *    moss_avl_free()
 *    moss_avl_add()
 *    moss_avl_add_cons_field()
 *    moss_avl_point()
 *    moss_avl_backwards_point()
 *    moss_avl_backup()
 *    moss_avl_next()
 *    moss_avl_element()
 *    moss_avl_remove()
 *    moss_avl_append()
 *    moss_avl_to_buf()
 *    moss_avl_from_buf()
 *    moss_avl_copy()
 *    moss_avl_exit_construction()
 *    moss_avl_find_item()
 *    moss_avl_index_buf()
 *    qremove()
 *    avl_insert()
 *    moss_avl_length()
 *    moss_avl_to_mybuf()
 *    moss_avl_init_mutex()
 *    moss_avl_free_mutex()
 *    moss_avl_lock_mutex()
 *    moss_avl_unlock_mutex()
 *    moss_avl_get_user_area()
 *
 * Author:
 *
 *    Wim Colgate
 *
 * Date:
 *
 *    November 21st, 1989
 *
 * Revision History :
 *
 *      Miriam Amos Nihart, December 6th, 1991.
 *
 *      Change moss_avl_exit_construction to move one level of
 *      construction or all the way to the end or beginnging of
 *      the constructed element.
 *
 *      Miriam Amos Nihart, December 3rd, 1991.
 *
 *      Put in macros for malloc and free.
 *
 *      Miriam Amos Nihart, November 27th, 1991.
 *
 *      Change moss_avl_free to leave the avl initialized.  Change
 *      moss_avl_init to only return MAN_C_ALREADY_INITIALIZED if
 *      the avl has elements.
 *
 *      Miriam Amos Nihart, November 27th, 1991.
 *
 *      Fix moss_avl_append to leave the destination avl as an empty
 *      and initialized avl.
 *
 *      Richard J. Bouchard, Jr.  November 19th, 1991 (RJB1095)
 *
 *      Updated moss_avl_init() to clear the "construction_level" field.
 *
 *      Wim Colgate, November 18th, 1991.
 *
 *      Reordered qremove, avl_insert and avl_index_add to beginning of module, and
 *      removed forward declarations.
 *
 *      Miriam Amos Nihart, November 8th, 1991.
 *
 *      Add the mutex routines.
 *
 *      Miriam Amos Nihart June 3rd, 1991.
 *
 *      Add oid argument to moss_avl_copy.
 *
 *      Miriam Amos Nihart May 28th, 1991.
 *
 *      Remove index check on moss_avl_length.
 *
 *      Miriam Amos Nihart January 17th, 1991.
 *
 *      Put additional parametering checking on moss_avl_length.
 *
 *      Wim Colgate, January 9th, 1991.
 *
 *      Fixed the length-bound checking in moss_avl_to_mybuf.
 *
 *      Miriam Amos Nihart November 26th, 1990.
 *
 *      Change moss_avl_index_buff to moss_avl_index_buf.
 *
 *      Miriam Amos Nihart November 18th, 1990.
 *
 *      Remove implicit casts of avl to iavl with explicit ones.
 *
 *      Wim Colgate November 13th, 1990
 *
 *      Added two new routines: moss_avl_length() and moss_avl_to_mybuf().
 *      Broke up moss_avl_to_buf into three pieces that call the above two
 *      routines.
 *
 *      Miriam Amos Nihart November 7th, 1990.
 *
 *      Change moss_avl_init() and moss_avl_free() to truely hide the
 *      data structure.
 *
 *      Miriam Amos Nihart October 29th, 1990.
 *
 *      Change bzero and bcopy to the ansi equivalents memset and memcpy.
 *
 *      Miriam Amos Nihart October 15th, 1990.
 *
 *      Remove moss_avl_point_cons_field routine.
 *
 *      Oscar Newkerk October 4, 1990
 *
 *      Add the moss_avl_exit_construction, moss_avl_find_item, and
 *      moss_avl_index_buff routines.
 *
 *      Oscar Newkerk, October 3, 1990
 *
 *      Add a new field to the avl element to indicate if the element is an attribute.
 *      This requires changes to the moss_avl_add and moss_avl_start_construct.  Also
 *      added a field to the AVL handle to indicate if the AVL is read only. That is, 
 *      was produced by a call to moss_avl_index_buff.  This requires changes to all off
 *      the moss_avl routines that might change values to chech for this type of AVL and return 
 *      an error.
 *
 *      Oscar Newkerk September 26, 1990.
 *
 *      Fix a bug in the moss_avl_append call that coused the contruction_level
 *      in the destination avl header not to be updated.
 *
 *      Kathy Faust, July 20th 1990.
 *
 *      Fixed bug in moss_avl_copy to only use new modifier value if
 *      supplied in first element copied.  Otherwise explicit ASN1
 *      encoding information in constructed types is lost.
 *
 *      Kathy Faust, July 18th 1990.
 *
 *      Modified avl_insert to be static.
 *
 *      Kathy Faust, July 17th, 1990.
 *
 *      Modified moss_avl_append and moss_avl_copy to take a new argument
 *      that tells where avl_handle_dest should be after the call.  Also
 *      modified moss_avl_copy to take a new argument for the copy modifier
 *      value.
 * 
 *      Kathy Faust, July 12th, 1990.
 *
 *      Modified moss_avl_add to explicitly check for NULL oid or octet
 *      before looking at fields w/in these to prevent segmentation fault.
 *
 *      Kathy Faust, July 11th, 1990.
 *
 *      Added moss_avl_copy.
 *
 *      Kathy Faust, July 10th, 1990.
 *
 *      Added check in moss_avl_from_buf to check for NULL buffer argument
 *      to prevent segmentation fault on bad parameter.
 *
 *      Kathy Faust, July 3rd, 1990.
 *
 *      Modified moss_avl_init, moss_avl_add, moss_avl_add_cons_field, 
 *      moss_avl_start_construct, moss_avl_end_construct, moss_avl_to_buf,
 *      and moss_avl_from_buf to return MAN_C_PROCESSING_FAILURE if 
 *      memory allocation fails.
 *
 *      Miriam Amos Nihart, July 1st, 1990.
 *
 *      Shorten moss_avl_*_constructed_field() calls to moss_avl_*_cons_field() for
 *      VMS compiler.
 *
 *      Kathy Faust, June 20th, 1990.
 *
 *      Modified moss_avl_to_buf to fix buf increment when oid is NULL.
 *
 *      Wim Colgate, June 19th, 1990.
 *
 *      Modified how NULL octet and oids are handled in adding and pointing.
 *
 *      Wim Colgate, June 11th, 1990.
 *
 *      Fixed how ALIGN is used. 
 *
 *      Wim Colgate, May 31st, 1990
 *
 *      Changed the behavior of moss_avl_next. Now we allow moving
 *      the current avl element 'onto' the terminator element.
 *
 *      Wim Colgate, May 24th, 1990
 *
 *      Added a new routine, moss_avl_backwards_point() to
 *      move back one avl element and point to the data. In that order.
 *
 *      Miriam Amos Nihart, May 14th, 1990.
 *
 *      Change the file names to reflect the 14 character restriction.
 *
 *	Miriam Amos Nihart April 17th, 1990:
 *
 *	Put in void declarations for calls to qremove().
 *
 *	Wim Colgate March 27th, 1990
 *
 *	Forced avl_handle->curr_avl to 0 in moss_avl_free().
 *  
 *	Miriam Amos Nihart February 27th 1990:
 *
 *	Changed the moss_avl_point() routine's parameters modifier and
 *	tag to be unsigned long int *.  This is consistent with the
 *	data types.  See moss_avl_add() and moss_avl_start_construct().
 *
 *	Wim Colgate February 22nd, 1990
 *
 *	Changed avl element field names from tag to modifier, and type to tag.
 *	Also changed the way inwhich constructed types are handled. Now the
 *	tag (ASN1 tag) contains a bit indicates constructed or not.
 *
 *	Uttam S. February 13th 1990:
 *
 *	Changed the moss_avl_to_buf() routine to use the bcopy() system
 *	call. This was done to avoid unaligned data access on a MIPS based
 *	machine.
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

#ifndef NOIPC
extern void *cma_lib_malloc() ;
extern void *cma_lib_free() ;
#else
#include <stdlib.h>
#endif /* NOIPC */

/*
 *  Support header files
 */  

#include <string.h>

#ifdef RPCV2
#if defined(__osf__)
/* KLUDGE. 
 * On Ultrix pthread.h includes cma.h which includes dce/cma_errno.h
 * which includes errno.h.  On OSF dce/cma_errno.h is not present so we 
 * include errno.h here.
 */
#include <errno.h>
#endif

#include "pthread.h"
#endif /* RPCV2 */

/*
 *  MOSS Specific header files
 */

#include "nil.h"
#include "moss.h" 
#include "moss_private.h"
#include "man_data.h"
#include "man.h"

/*
 *  External
 */


static
void
qremove( 
        handle , 
        element 
       )

iavl *handle ;
avl_element *element ;

/*
 *
 * Function Description:
 *
 *    Remove the avl element from the avl handle.
 *
 * Parameters:
 *
 *    handle      The pointer to a REAL AVL.
 *    element     A pointer to the avl-element to be removed.
 *
 * Return value:
 *
 *    None.
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 *    Note that we DO NOT free any memory
 */

{
    avl_element *next ;
    avl_element *prev ;

    next = element->next_avl ;
    prev = element->prev_avl ;

    if ( prev == NULL )
        handle->first_avl = next ;
    else
        prev->next_avl = next ;

    if ( next == NULL )
        ;
    else
       next->prev_avl = prev ;

    if ( handle->curr_avl == element )
       handle->curr_avl = next ;

    element->next_avl = NULL ;
    element->prev_avl = NULL ;

} /* end of qremove() */


static
void
avl_insert ( 
            handle , 
            element 
           )

iavl *handle ;
avl_element *element ;

/*
 *
 * Function Description:
 *
 *    Insert the avl-element into the avl-handle at the current location.
 *
 * Parameters:
 *
 *    handle      The pointer to a REAL AVL.
 *    element     A pointer to the avl-element to be inserted.
 *
 * Return value:
 *
 *    None.
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 */

{
    avl_element *next ;
    avl_element *prev ;

    next = handle->curr_avl ;
    prev = next->prev_avl ;

    if ( prev == NULL )
        handle->first_avl = element ;
    else
        prev->next_avl = element ;

    next->prev_avl = element ;

    element->prev_avl = prev ;
    element->next_avl = next ;

} /* end of avl_insert() */


static
man_status
avl_index_add( 
             avl_handle , 
             oid , 
             modifier , 
             tag , 
             octet 
            )

iavl *avl_handle ;
object_id *oid ;
unsigned int modifier ;
unsigned int tag ;
octet_string *octet ;

/*
 *
 * Function Description:
 *
 *    Add a new AVL element with the information provided. Place the AVL before the current AVL
 *    pointer (usually, the current AVL is pointing to the terminator entry).  This version simply
 *    points at data that is already in the flat buffer.  Since we are called after the checks of the
 *    AVL handle in moss_avl_index_buf, we won't check again here.
 *
 * Parameters:
 *
 *    avl_handle     The pointer to a REAL AVL.
 *    oid            A pointer to an an object id.
 *    modifier       The modifier for this avl.
 *    tag            The ASN.1 data type of this avl.
 *    octet          A pointer to the octet string representation of the value.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_PROCESSING_FAILURE  Error in memory allocation
 *
 * Side effects:
 *
 *    None
 *
 */

{
    avl_element *element ;

    /* 
     * First, allocate necessary memory -- memory is allocated for avl element only.
     */

    MOSS_MALLOC( element, avl_element, sizeof( avl_element ) )
    if ( element == NULL )
        return( MAN_C_PROCESSING_FAILURE ) ;

    memset( ( void * )element, '\0', sizeof( avl_element ) ) ;


    /*
     * Populate the AVL element portion.
     */

    element->terminator = FALSE ;

    /*
     * Replicate the oid.
     */

    if ( oid != NULL )
    {
        element->oid.count = oid->count ;
        if ( oid->count != 0 )
        {
            element->oid.value = oid->value ;                   /*  Just copy the pointer we were given.  */
            if ( avl_handle->construction_level == 0 )
                element->item_flag = TRUE ;
            else
                element->item_flag = FALSE ;
	}
    }
    else
    {
        element->oid.count = 0xffffffff ;
        element->oid.value = NULL ;
    }

    /*
     * Copy in octet string.
     */

    if ( octet != NULL )
    {
        element->oct.length = octet->length ;
        element->oct.data_type = octet->data_type ;
        if ( octet->length != 0 )
            element->oct.string = octet->string ;               /*  Just copy the pointer we were given.  */
    }
    else
    {
        element->oct.length = 0xffffffff ;
        element->oct.data_type = 0 ;
        element->oct.string = NULL ;
    }

    /*
     * If the tag is NULL (which is also ASN1_C_EOC) then lets check to
     * see if it was really a end of construction or just a programmers 
     * oversite. the end_of_construction call passes a 0 length for oid
     * and octet, as well as 0's for both tag and octet. If there is some
     * data here, we munge the tag to be ASN1_C_NULL
     */
    if ( tag == 0 )
    {
	if ( ( element->oid.count == 0 ) && ( element->oct.length == 0 ) )
            element->tag = tag ;
        else
            element->tag = ( unsigned int )ASN1_C_NULL ;
    }
    else
        element->tag = tag ;

    /*
     * Now set the construction level in the handle to the correct value.  This is needed to keep
     * the item_flag code working correctly.
     */

    if ( IS_CONSTRUCTED( tag ) )
        avl_handle->construction_level += 1 ;
    else
        if ( tag == ( unsigned int )ASN1_C_EOC )
            avl_handle->construction_level -= 1 ;


    element->modifier = modifier ;

    /* 
     * Place the completed avl element in the AVL.
     */

    avl_insert( avl_handle, element ) ;
     
    return( MAN_C_SUCCESS ) ;

} /* end of avl_index_add() */


man_status
moss_avl_init(
              avl_handle
             )

avl **avl_handle ;

/*
 *
 * Function Description:
 *
 *    Initialize an AVL.
 *
 * Parameters:
 *
 *    avl_handle        The address to a pointer to an opaque AVL
 *
 * Return value:
 *
 *    MAN_C_SUCCESS              Success
 *    MAN_C_ALREADY_INITIALIZED  Handle is already initialized
 *    MAN_C_BAD_PARAMETER        Handle is NULL
 *    MAN_C_PROCESSING_FAILURE   Error on memory allocation
 *
 * Side effects:
 *
 *    None
 *
 * NOTE:
 *    Because the avl_handle pointer is usually allocate on the stack,
 *    the avl_handle pointer may contain garbage.  As a result SOMETIMES
 *    the 'already initialized' AVL return code is returned erroneously.
 *    Good programming practice dictates that the programmer initialize
 *    the avl_handle pointer to all NULL.
 *
 */

{
    avl_element *terminator ;
    avl_element *check_element ;
    iavl *iavl_handle ;        /* ptr to READ AVL - use for accessing fields */

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    /*
     *  If the address of the avl_handle pointer is NULL or if
     *  the avl_handle pointer is not NULL and its already been
     *  initialized return appropriate error condition.
     */

    if ( avl_handle == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    iavl_handle = ( iavl * )*avl_handle ;
    if ( iavl_handle != NULL )
    {
        if ( iavl_handle->initialized == TRUE )
        {
            check_element = iavl_handle->first_avl ;
            if ( check_element->terminator != TRUE )
                return( MAN_C_ALREADY_INITIALIZED ) ;
            iavl_handle->construction_level = 0 ;
            iavl_handle->index_flag = FALSE ;
            iavl_handle->curr_avl = iavl_handle->first_avl ;
            return( MAN_C_SUCCESS ) ;
        }
    }
    else
    {
        MOSS_MALLOC( iavl_handle, iavl, sizeof( iavl ) )
        if ( iavl_handle == NULL )
            return( MAN_C_PROCESSING_FAILURE ) ;
        memset( ( void * )iavl_handle, '\0', sizeof( iavl ) ) ;
    }

    /*
     *  Ok now we have a valid avl - so initialize it.
     */

    MOSS_MALLOC( terminator, avl_element, sizeof( avl_element ) )
    if ( terminator == NULL )
    {
       if ( *avl_handle == NULL )
           MOSS_FREE( iavl_handle ) ;
       return( MAN_C_PROCESSING_FAILURE ) ;
    }

    iavl_handle->initialized = TRUE ;
    iavl_handle->index_flag = FALSE ;

    /* 
     * Make a terminator record.
     */

    memset( ( void * )terminator, '\0', sizeof( avl_element ) ) ;
    terminator->terminator = TRUE ;

    iavl_handle->construction_level = 0 ;
    iavl_handle->first_avl = terminator ;
    iavl_handle->curr_avl = terminator ;

    *avl_handle = ( avl * )iavl_handle ;
    return( MAN_C_SUCCESS ) ;

} /* end of moss_avl_init() */


man_status
moss_avl_start_construct( 
                         avl_handle , 
                         oid , 
                         modifier , 
                         tag , 
                         octet 
                        )

avl *avl_handle ;
object_id *oid ;
unsigned int modifier ;
unsigned int tag ;
octet_string *octet ;

/*
 *
 * Function Description:
 *
 *    Start (another level) of construction.
 *
 * Parameters:
 *
 *    avl_handle     The pointer to an opaque AVL.
 *    oid            A pointer to an an object id.
 *    modifier       The modifier for this avl.
 *    tag            The ASN.1 data type of this avl.
 *    octet          A pointer to the octet string representation of the value.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_NOT_CONSTRUCTED  The ASN1 tag is not of the constructed type
 *    MAN_C_PROCESSING_FAILURE Error on memory allocation
 *    MAN_C_READ_ONLY       The AVL is an indexed AVL and can not be modified
 *    
 *
 * Side effects:
 *
 *    None
 *
 */

{
    iavl *iavl_handle = ( iavl * )avl_handle ;  /* ptr to REAL AVL - used for accessing fields */
    man_status status ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( !IS_CONSTRUCTED( tag ) )
        return( MAN_C_NOT_CONSTRUCTED ) ;

    if ( avl_handle == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    if ( iavl_handle->initialized != TRUE )
       return( MAN_C_NOT_INITIALIZED ) ;

    /* 
     * Place it in the AVL.
     */

    status = moss_avl_add( avl_handle, oid, modifier, tag, octet ) ;
     
    /*
     * This increment of the construction level MUST be done AFTER the call to 
     * moss_avl_add.  This is to insure that the item_flag in the avl element 
     * is set correctly in moss_avl_add.
     */

    if ( status == MAN_C_SUCCESS )
        iavl_handle->construction_level += 1 ;

    return( status ) ;

} /* end of moss_avl_start_construct() */


man_status
moss_avl_end_construct(
                       avl_handle
                      )

avl *avl_handle ;

/*
 *
 * Function Description:
 *
 *    end (another level) of construction.
 *
 * Parameters:
 *
 *    avl_handle        The pointer to an opaque AVL.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS            Success
 *    MAN_C_NOT_INITIALIZED    Handle is not initialized
 *    MAN_C_BAD_PARAMETER      Handle is NULL
 *    MAN_C_NOT_CONSTRUCTED    The construction level is 0
 *    MAN_C_PROCESSING_FAILURE Error on memory allocation
 *    MAN_C_READ_ONLY          The AVL is an indexed AVL and can not be modified
 *
 * Side effects:
 *
 *    None
 *
 */

{
    iavl *iavl_handle = ( iavl * )avl_handle  ;  /* ptr to REAL AVL - used for accessing fields */
    avl_element *construct ;
    man_status status ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( avl_handle == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    if ( iavl_handle->initialized != TRUE )
       return( MAN_C_NOT_INITIALIZED ) ;

    if ( iavl_handle->construction_level == 0 )
       return( MAN_C_NOT_CONSTRUCTED ) ;

    /* 
     * Place it in the AVL.
     */

    status = moss_avl_add( avl_handle, &nil_object_id, 0, ASN1_C_EOC, &nil_octet_string ) ;

    /*
     * This decrement of the construction level MUST be done AFTER the call to 
     * moss_avl_add.  This is to insure that the item_flag in the avl element 
     * is set correctly in moss_avl_add.
     */

    if ( status == MAN_C_SUCCESS )
        iavl_handle->construction_level -= 1 ;
     
    return( status ) ;

} /* end of moss_avl_end_construct() */


man_status
moss_avl_reset(
               avl_handle
              )

avl *avl_handle ;

/*
 *
 * Function Description:
 *
 *    Reset an AVL - This means to reset the current element pointer to the
 *    beginning (first AVL element).
 *
 * Parameters:
 *
 *    avl_handle        The pointer to an opaque AVL.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *
 * Side effects:
 *
 *    None
 *
 */

{
    iavl *iavl_handle = ( iavl * )avl_handle ; /* ptr to REAL AVL - used for accessing fields */

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( avl_handle == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    if ( iavl_handle->initialized != TRUE )
       return( MAN_C_NOT_INITIALIZED ) ;

    /*
     * Set the current pointer to the first avl element.
     */

    iavl_handle->curr_avl = iavl_handle->first_avl ;
    iavl_handle->construction_level = IS_CONSTRUCTED( iavl_handle->curr_avl->tag ) ;

    return( MAN_C_SUCCESS ) ;

} /* end of moss_avl_reset() */


man_status
moss_avl_free(
              avl_handle ,
              flag
             )

avl **avl_handle ;
int flag ;

/*
 *
 * Function Description:
 *
 *    Free the associated memory of an AVL. If flag is set to TRUE,
 *    the associated memory and the AVL are freed and the pointer
 *    is set to NULL.  If flag is set to FALSE, the associated memory
 *    is freed and the AVL handle is set to be uninitialized.
 *                                              
 * Parameters:
 *
 *    avl_handle        The address of a pointer to an opaque AVL
 *    flag              A value indicating whether to release all AVL memory
 *                      including the AVL.
 *                          TRUE    Frees all memory and sets the pointer to NULL
 *                          FALSE   Frees memory associated with the AVL, but
 *                                  the avl is still useable.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_MUTEX_EXISTS     A mutex is still associated with the avl
 *
 * Side effects:
 *
 *    None
 *
 */

{
    iavl *iavl_handle ;      /* ptr to REAL AVL */
    avl_element *element ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( ( avl_handle == NULL ) || ( *avl_handle == NULL ) )
        return( MAN_C_BAD_PARAMETER ) ;

    iavl_handle = ( iavl * )*avl_handle ;

    if ( iavl_handle->initialized != TRUE )
       return( MAN_C_NOT_INITIALIZED ) ;

#ifdef RPCV2
    if ( iavl_handle->mutex_handle != ( pthread_mutex_t * )NULL )
       return( MAN_C_MUTEX_EXISTS ) ;
#endif /* RPCV2 */

    /*
     * Loop through all the AVL elements, freeing all the data (if there is
     * any), then remove the element from the avl element chain and free it.
     */

    while( iavl_handle->first_avl != NULL)
    {
        element = iavl_handle->first_avl ;

        /*
         *  Exit loop if the element is the terminator.
         *  Don't release this unless the flag is TRUE.
         */

        if ( element->terminator == TRUE )
            break ;

        if ( iavl_handle->index_flag != TRUE )
        {
            if ( element->oid.value != NULL )
                MOSS_FREE( element->oid.value) ;
            if ( element->oct.string != NULL )
                MOSS_FREE( element->oct.string ) ;
        }

        qremove( iavl_handle, element ) ;
        MOSS_FREE( element ) ;

    } 

    if ( flag == FALSE )
    {
        /*
         * Reset the flags and leave the terminator element.
         */

        iavl_handle->index_flag = FALSE ;
        iavl_handle->initialized = TRUE ;
        iavl_handle->construction_level = 0 ;
        iavl_handle->curr_avl = iavl_handle->first_avl ;
    }
    else
    {
        MOSS_FREE( element ) ;
        MOSS_FREE( iavl_handle ) ;
        *avl_handle = ( avl * )NULL ;
    }

    return( MAN_C_SUCCESS ) ;

} /* end of moss_avl_free() */
                                       

man_status
moss_avl_add( 
             avl_handle , 
             oid , 
             modifier , 
             tag , 
             octet 
            )

avl *avl_handle ;
object_id *oid ;
unsigned int modifier ;
unsigned int tag ;
octet_string *octet ;

/*
 *
 * Function Description:
 *
 *    Add a new AVL with the information provided. Place the AVL before the current AVL
 *    pointer (usually, the current AVL is pointing to the terminator entry).
 *
 * Parameters:
 *
 *    avl_handle     The pointer to an opaque AVL.
 *    oid            A pointer to an an object id.
 *    modifier       The modifier for this avl.
 *    tag            The ASN.1 data type of this avl.
 *    octet          A pointer to the octet string representation of the value.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS             Success
 *    MAN_C_NOT_INITIALIZED     Handle is not initialized
 *    MAN_C_BAD_PARAMETER       Handle is NULL
 *    MAN_C_PROCESSING_FAILURE  Error in memory allocation
 *    MAN_C_READ_ONLY           The AVL is an indexed AVL and is read only
 *
 * Side effects:
 *
 *    None
 *
 */

{
    iavl *iavl_handle = ( iavl * )avl_handle ;  /* ptr to REAL AVL - used for accessing fields */
    avl_element *element ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( avl_handle == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    if ( iavl_handle->initialized != TRUE )
       return( MAN_C_NOT_INITIALIZED ) ;

    if ( iavl_handle->index_flag == TRUE )
        return( MAN_C_READ_ONLY ) ;

    /* 
     * First, allocate necessary memory -- memory is allocated for avl, object_id if non-null,
     * and octet if non-null.
     */

    MOSS_MALLOC( element, avl_element, sizeof( avl_element ) )
    if ( element == NULL )
        return( MAN_C_PROCESSING_FAILURE ) ;

    memset( ( void * )element, '\0', sizeof( avl_element ) ) ;

    if ( oid != NULL )
    {
        if ( oid->count != 0 )
	{
            MOSS_MALLOC( element->oid.value, unsigned int, oid->count * sizeof( int ) )
	    if ( element->oid.value == NULL ) 
	    {
		MOSS_FREE( element ) ;
		return( MAN_C_PROCESSING_FAILURE ) ;
	    }
	}
    }

    if ( octet != NULL )
    {
	if ( octet->length != 0 )
	{
            MOSS_MALLOC( element->oct.string, char, octet->length ) 
	    if ( element->oct.string == NULL )
	    {
		if ( oid != NULL )
		    if ( oid->count != 0 )
		        MOSS_FREE( element->oid.value ) ;
		MOSS_FREE( element ) ;
		return( MAN_C_PROCESSING_FAILURE ) ;
	    }
	}
    }

    /*
     * Populate the AVL element portion.
     */

    element->terminator = FALSE ;

    /*
     * Replicate the oid.
     */

    if ( oid != NULL )
    {
        element->oid.count = oid->count ;
        if ( oid->count != 0 )
        {
            memcpy( element->oid.value, oid->value, oid->count *  sizeof( int ) ) ;
            if ( iavl_handle->construction_level == 0 )
                element->item_flag = TRUE ;
            else
                element->item_flag = FALSE ;
	}
    }
    else
    {
        element->oid.count = 0xffffffff ;
        element->oid.value = NULL ;
    }

    /*
     * Copy in octet string.
     */

    if ( octet != NULL )
    {
        element->oct.length = octet->length ;
        element->oct.data_type = octet->data_type ;
        if ( octet->length != 0 )
            memcpy( element->oct.string, octet->string, octet->length ) ;
    }
    else
    {
        element->oct.length = 0xffffffff ;
        element->oct.data_type = 0 ;
        element->oct.string = NULL ;
    }

    /*
     * If the tag is NULL (which is also ASN1_C_EOC) then lets check to
     * see if it was really a end of construction or just a programmers 
     * oversite. the end_of_construction call passes a 0 length for oid
     * and octet, as well as 0's for both tag and octet. If there is some
     * data here, we munge the tag to be ASN1_C_NULL
     */
    if ( tag == 0 )
    {
	if ( ( element->oid.count == 0 ) && ( element->oct.length == 0 ) )
            element->tag = tag ;
        else
            element->tag = ( unsigned int )ASN1_C_NULL ;
    }
    else
        element->tag = tag ;

    element->modifier = modifier ;

    /* 
     * Place the completed avl element in the AVL.
     */

    avl_insert( iavl_handle, element ) ;
     
    return( MAN_C_SUCCESS ) ;

} /* end of moss_avl_add() */


man_status
moss_avl_add_cons_field( 
                        avl_handle , 
                        explicit , 
                        tag , 
                        octet 
                       )

avl *avl_handle ;
unsigned int tag ;
unsigned int explicit ;
octet_string *octet ;

/*
 *
 * Function Description:
 *
 *    Add a new AVL inside a constructed type with the information provided. 
 *    This is just like moss_avl_add, but we use this routine for 'ease of use'.
 *    There is no oid. and we stuff the modifier field with the explicit field.
 *
 * Parameters:
 *
 *    avl_handle     The pointer to an opaque AVL.
 *    explicit       An extra piece of info stored in the modifier.
 *    tag            The ASN.1 data type of this avl.
 *    octet          A pointer to the octet string representation of the value.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_PROCESSING_FAILURE Error on memory allocation
 *    MAN_C_READ_ONLY       The AVL is an indexed AVL and can not be modified
 *
 * Side effects:
 *
 *    None
 *
 */

{
    man_status status ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    status = moss_avl_add( avl_handle, &nil_object_id, explicit, tag, octet ) ;

    return( status ) ;

} /* end of moss_avl_add_cons_field() */


man_status
moss_avl_point( 
               avl_handle , 
               oid , 
               modifier , 
               tag , 
               octet , 
               last_one 
              )

avl *avl_handle ;
object_id **oid ;
unsigned int *modifier ;
unsigned int *tag ;
octet_string **octet ;
int *last_one ;

/*
 *
 * Function Description:
 *
 *    Set the pointers to the current info in the AVL. No copying is performed.
 *    The function advances the current AVL pointer to the next element.
 *
 * Parameters:
 *
 *    avl_handle  The pointer to an opaque AVL.
 *    oid         An address of a pointer of an object identifier.
 *    tag         A pointer to an int.
 *    modifier    A pointer to an int.
 *    octet       An address of a pointer to an octet string.
 *    last_one    A address of an int that indicates if this is the last element.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_NO_ELEMENT       The current element is the terminator element
 *
 * Side effects:
 *
 *    None
 *
 */

{
    iavl *iavl_handle = ( iavl * )avl_handle ; /* ptr to REAL AVL - used for accessing fields */
    avl_element *element ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( avl_handle == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    if ( iavl_handle->initialized != TRUE )
       return( MAN_C_NOT_INITIALIZED ) ;

    element = iavl_handle->curr_avl ;
    if ( element->terminator == TRUE )
       return( MAN_C_NO_ELEMENT ) ;

    if ( oid != NULL )
        if ( element->oid.count == 0xffffffff )
            *oid = NULL ;
        else
            *oid = &( element->oid ) ;
    if ( octet != NULL )
        if ( element->oct.length == 0xffffffff )
            *octet = NULL ;
        else
            *octet = &( element->oct ) ;
    if ( modifier != NULL )
        *modifier = element->modifier ;
    if ( tag != NULL )
        *tag = element->tag ;
    if ( last_one != NULL )
        *last_one = element->next_avl->terminator ;

    iavl_handle->curr_avl = element->next_avl ;

    element = iavl_handle->curr_avl ;
    if ( IS_CONSTRUCTED( element->tag ) )
        iavl_handle->construction_level += 1 ;
    else
        if ( element->tag == ( unsigned int )ASN1_C_EOC )
            iavl_handle->construction_level -= 1 ;
   

    return( MAN_C_SUCCESS ) ;

} /* end of moss_avl_point() */


man_status
moss_avl_backwards_point( 
                         avl_handle , 
                         oid , 
                         modifier , 
                         tag , 
                         octet , 
                         last_one 
                        )

avl *avl_handle ;
object_id **oid ;
unsigned int *modifier ;
unsigned int *tag ;
octet_string **octet ;
int *last_one ;

/*
 *
 * Function Description:
 *
 *    Move the pointer (if possible) from the current AVL to the preceding one.
 *    The write into the pointers passed in.
 *
 * Parameters:
 *
 *    avl_handle  The pointer to an opaque AVL.
 *    oid         An address of a pointer of an object identifier.
 *    tag         A pointer to an int.
 *    modifier    A pointer to an int.
 *    octet       An address of a pointer to an octet string.
 *    last_one    A address of an int that indicates if this is the last element.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_NO_ELEMENT       The current element is the first element
 *
 * Side effects:
 *
 *    None
 *
 */

{
    iavl *iavl_handle = ( iavl * )avl_handle ;  /* ptr to REAL AVL - used for accessing fields */
    avl_element *element ;
    man_status status ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    status = moss_avl_backup( avl_handle ) ;
    if ( status != MAN_C_SUCCESS ) 
        return( status ) ;
    
    element = iavl_handle->curr_avl ;

    if ( oid != NULL )
    {
        if ( element->oid.count == 0xffffffff ) 
            *oid = NULL ;
        else
            *oid = &( element->oid ) ;
    }
    if ( octet != NULL )
    {
        if ( element->oct.length == 0xffffffff )
            *octet = NULL ;
        else
            *octet = &( element->oct ) ;
    }
    if ( modifier != NULL )
        *modifier = element->modifier ;
    if ( tag != NULL )
        *tag = element->tag ;
    if ( last_one != NULL )
        *last_one = (element->prev_avl == NULL) ;

    return( MAN_C_SUCCESS ) ;

} /* end of moss_avl_backwards_point() */


man_status
moss_avl_backup(
                avl_handle
               )

avl *avl_handle ;

/*
 *
 * Function Description:
 *
 *     Move the current AVL element back one.
 *
 * Parameters:
 *
 *    avl_handle        The pointer to an opaque AVL.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_NO_ELEMENT       The current element is the terminator element
 *
 * Side effects:
 *
 *    None
 *
 */

{
    iavl *iavl_handle = ( iavl * )avl_handle ;  /* ptr to REAL AVL - used for accessing fields */
    avl_element *element ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( avl_handle == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    if ( iavl_handle->initialized != TRUE )
       return( MAN_C_NOT_INITIALIZED ) ;

    element = iavl_handle->curr_avl ;

    if ( element->prev_avl == NULL )
        return( MAN_C_NO_ELEMENT ) ;

    /*
     *  Update the construction level according to
     *  the element we are backing up from.
     */

    if ( IS_CONSTRUCTED( element->tag ) )
        iavl_handle->construction_level -= 1 ;
    else
        if ( element->tag == ( unsigned int )ASN1_C_EOC )
            iavl_handle->construction_level +=1 ;

    /*
     * Set the current pointer back one AVL element.
     */

    iavl_handle->curr_avl = element->prev_avl ;

    return( MAN_C_SUCCESS ) ;

} /* end of moss_avl_backup() */


man_status
moss_avl_next(
              avl_handle
             )

avl *avl_handle ;

/*
 *
 * Function Description:
 *
 *     Move the current AVL element forward one.
 *
 * Parameters:
 *
 *    avl_handle        The pointer to an opaque AVL.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_NO_ELEMENT       The current element is the terminator element
 *
 * Side effects:
 *
 *    None
 *
 */

{
    iavl *iavl_handle = ( iavl * )avl_handle ;  /* ptr to REAL AVL - used for accessing fields */
    avl_element *element ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( avl_handle == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    if ( iavl_handle->initialized != TRUE )
       return( MAN_C_NOT_INITIALIZED ) ;

    element = iavl_handle->curr_avl ;

    /*
     *  If the element after this one is the terminator record,
     *  then we will not move it.
     */

    if ( element->terminator == TRUE )
        return( MAN_C_NO_ELEMENT ) ;

    /*
     *  Set the current pointer to the next AVL.
     */

    iavl_handle->curr_avl = element->next_avl ;

    /*
     *  Update the construction level according to the
     *  new element -  only if it is not the terminator.
     */

    element = iavl_handle->curr_avl ;
    if ( element->terminator != TRUE )
    {
        if (element->tag == ( unsigned int )ASN1_C_EOC )
            iavl_handle->construction_level -=1 ;
        else
            if ( IS_CONSTRUCTED( element->tag ) )
                iavl_handle->construction_level += 1 ;
    }

    return( MAN_C_SUCCESS ) ;

} /* end of moss_avl_next() */


avl_element *
moss_avl_element(
                 avl_handle
                )

iavl *avl_handle ;

/*
 *
 * Function Description:
 *
 *     Return a pointer to the current AVL element.
 *
 * Parameters:
 *
 *    avl_handle        The pointer to an internal AVL.
 *
 * Return value:
 *
 *    Address of AVL element
 *    or NULL if the current element is the terminator.
 *
 * Side effects:
 *
 *    None
 *
 */

{
    avl_element *element ;

    if ( avl_handle == NULL )
        return( NULL ) ;

    if ( avl_handle->initialized != TRUE )
       return( NULL ) ;

    element = avl_handle->curr_avl ;

    if ( element->terminator == TRUE )
        return( NULL ) ;

    return( element ) ;

} /* end of moss_avl_element() */


man_status
moss_avl_remove(
                avl_handle
               )

avl *avl_handle ;

/*
 *
 * Function Description:
 *
 *     remove the current AVL element.
 *
 * Parameters:
 *
 *    avl_handle        The pointer to an opaque AVL.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_NO_ELEMENT       The current element is the terminator element
 *    MAN_C_READ_ONLY        The VAL is an indexed AVL and can not be modified
 *
 * Side effects:
 *
 *    None
 *
 */

{
    iavl *iavl_handle = ( iavl * )avl_handle ; /* ptr to REAL AVL - used for accessing fields */
    avl_element *element ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( avl_handle == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    if ( iavl_handle->initialized != TRUE )
       return( MAN_C_NOT_INITIALIZED ) ;

    if ( iavl_handle->index_flag == TRUE )
        return( MAN_C_READ_ONLY ) ;

    element = iavl_handle->curr_avl ;

    if ( element->terminator == TRUE )
        return( MAN_C_NO_ELEMENT ) ;

    if ( element->oid.value != NULL )
        MOSS_FREE( element->oid.value ) ;
    if ( element->oct.string != NULL )
        MOSS_FREE( element->oct.string ) ;
    qremove( iavl_handle, element ) ;
    MOSS_FREE( element ) ;

    return( MAN_C_SUCCESS ) ;

} /* end of moss_avl_remove() */


man_status
moss_avl_append(
                avl_handle_dest ,
                avl_handle_source ,
		reset_dest 
               )

avl *avl_handle_dest ;
avl *avl_handle_source ;
int reset_dest ;

/*
 *
 * Function Description:
 *
 *     Append the source avl to the destination avl. The source avl
 *     becomes INVALID.
 *
 * Parameters:
 *
 *    avl_handle_dest		The desination of the append (opaque avl).
 *    avl_handle_source		The (opaque) avl that is appended to avl_handle_dest.
 *    reset_dest                If TRUE, reset the avl_handle_dest.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_READ_ONLY        The AVL is an indexed AVL and can not be modified
 *
 * Side effects:
 *
 *    None
 *
 */

{
    iavl *iavl_handle_dest = ( iavl * )avl_handle_dest ; /* ptr to REAL AVL - used for accessing fields */
    iavl *iavl_handle_source = ( iavl * )avl_handle_source ; /* ptr to REAL AVL - used for accessing fields */
    avl_element *element ;
    avl_element *element_last ;
    avl_element *element_first ;
    avl_element *terminator ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( ( avl_handle_source == NULL ) || ( avl_handle_dest == NULL ) )
        return( MAN_C_BAD_PARAMETER ) ;

    if ( ( iavl_handle_source->initialized != TRUE ) ||
         ( iavl_handle_dest->initialized != TRUE ) )
       return( MAN_C_NOT_INITIALIZED ) ;

    if ( ( iavl_handle_dest->index_flag == TRUE ) ||
         ( iavl_handle_source->index_flag == TRUE ) )
        return( MAN_C_READ_ONLY ) ;

    /*
     * Reset the destination avl_handle
     */

    moss_avl_reset( avl_handle_dest ) ;

    /*
     * Walk through the destination avl to find the
     * termniator avl_element.
     */

    element = iavl_handle_dest->first_avl ;
    while (element->terminator != TRUE)
    {
        if ( IS_CONSTRUCTED( element->tag ) )
            iavl_handle_dest->construction_level += 1 ;
        if (element->tag == ( unsigned int )ASN1_C_EOC )
            iavl_handle_dest->construction_level -=1 ;
        element = element->next_avl ;
    }

    /*
     * Set the last useful avl_element to the element
     * preceeding the terminator record - we will be appending
     * the source avl to it.
     */

    element_last = element->prev_avl ;

    /*
     * Remove the terminator element, but save it for use
     * in the empty source avl.
     */

    terminator = element ;
    qremove( iavl_handle_dest, element ) ;

    /*
     * Get the first element of the source
     */

    element_first = iavl_handle_source->first_avl ;

    /* 
     * Attach it to the destination. If there were no elements
     * in the destination, then we must append the source avl 
     * directly to the handle. If there were elements, we append
     * after the last destination element.
     */

    if (element_last == NULL)
        iavl_handle_dest->first_avl = element_first ;
    else
    {
        element_last->next_avl = element_first ;
        element_first->prev_avl = element_last ;
    }

    /*
     * Reset the destination handle if requested.  Otherwise,
     * set destination handle to end.
     */

    if ( reset_dest == TRUE )
        moss_avl_reset( avl_handle_dest ) ;
    else
    {
	element = iavl_handle_dest->first_avl ;
	while (element->terminator != TRUE)
            element = element->next_avl ;
	iavl_handle_dest->curr_avl = element ;
    }

    /*
     * Through away any data in the source - it is no
     * longer valid.  Leave it as an empty avl.  Use the
     * terminator dequeue above to terminate the empty avl.
     */

    memset( ( void * )avl_handle_source, '\0', sizeof( iavl ) ) ;
    iavl_handle_source->first_avl = terminator ;
    iavl_handle_source->curr_avl = terminator ;
    iavl_handle_source->initialized = TRUE ;
    iavl_handle_source->index_flag = FALSE ;

    return( MAN_C_SUCCESS ) ;
    
} /* end of moss_avl_append() */


man_status
moss_avl_to_buf( 
                avl_handle , 
                buffer , 
                length
               )

avl *avl_handle ;
char **buffer ;
int *length ;

/*
 *
 * Function Description:
 *
 *     Translate an AVL into a flat buffer. This facilitates transfer over address
 *     spaces.
 *
 * Parameters:
 *
 *    avl_handle   The pointer to an opaque AVL.
 *    buffer       The address of a pointer to a buffer that gets created here.
 *    len          The return length of the buffer.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS            Success
 *    MAN_C_NOT_INITIALIZED    Handle is not initialized
 *    MAN_C_BAD_PARAMETER      Handle is NULL
 *    MAN_C_NOT_CONSTRUCTED    The construction is invalid (mismatched begin/end)
 *    MAN_C_NO_ELEMENT         The current element is the terminator element
 *    MAN_C_PROCESSING_FAILURE An error occured in memory allocation
 *    MAN_C_READ_ONLY          The AVL is an indexed AVL and can not be modified
 *
 * Side effects:
 *
 *    None
 *
 */

{
    man_status status ;      

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    /*
     * calculate the length of the flattened AVL
     */

    status = moss_avl_length( avl_handle, length ) ;
    if ( status != MAN_C_SUCCESS )
        return( status ) ;
    
    /*
     * as long as its non-zero, allocate some memory
     */

    if ( *length != 0 ) 
    {
       MOSS_MALLOC( *buffer, char, *length + sizeof( int ) )
       if ( *buffer == NULL )
           return( MAN_C_PROCESSING_FAILURE ) ;
    }
    else
       return( MAN_C_NO_ELEMENT ) ;

    /*
     * copy the data from the avl to the new buffer
     */

    status = moss_avl_to_mybuf( avl_handle, *buffer, *length + sizeof( int ) ) ;
    if ( status != MAN_C_SUCCESS )
    {
        MOSS_FREE( *buffer ) ;
        *buffer = 0 ;
        *length = 0 ;
        return( status ) ;
    }

    moss_avl_reset( avl_handle ) ;

    return( MAN_C_SUCCESS ) ;
    
} /* end of moss_avl_to_buf() */


man_status
moss_avl_length( 
                avl_handle , 
                len 
               )

avl *avl_handle ;
int *len ;

/*
 *
 * Function Description:
 *
 *    Count up all the avl elements and overhead needed to flatten
 *    the avl into a single buffer.
 *
 * Parameters:
 *
 *    avl_handle   The pointer to an opaque AVL.
 *    len          The return length of the buffer.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_NOT_CONSTRUCTED  The construction is invalid (mismatched begin/end)
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 *    Note that this routine and moss_avl_from_buf expect the structure of an AVL to be:
 *        oid:    {int,[int][...]},
 *        modifier:    {int},
 *        tag:    {int},
 *        item_flag:   {int},
 *        octet:  {int,int,[char][...]}
 */

{
    iavl *real_handle = ( iavl * )avl_handle ;  /* ptr to REAL AVL - used for accessing fields */
    avl_element *element ;
    int length = 0 ;
    int count = 0 ;
    int balanced = 0 ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( ( real_handle == NULL ) || ( len == NULL ) )
        return( MAN_C_BAD_PARAMETER ) ;

    if ( real_handle->initialized != TRUE )
       return( MAN_C_NOT_INITIALIZED ) ;

    moss_avl_reset( avl_handle ) ;
    element = real_handle->curr_avl ;

    /*
     * Save room for the length (it will be returned AS WELL as being placed)
     * in the buffer.
     *
     * It is placed in the buffer to make the RPC call perform one less buffer
     * copy.
     */

    length = 0 ; 

    while( element->terminator == FALSE )
    {
        count++ ;

        /*
         * We are specifically checking for 0xffffffff in either the oid or octet length field.
         * A length of 0xffffffff indicates an non-entry.
         */

        if ( element->oid.count != 0xffffffff )
            length += element->oid.count * sizeof( unsigned int ) ; /* object id elements */

        if ( element->oct.length != 0xffffffff )
            length += element->oct.length * sizeof( char ) ;             /* octet string elements */

        length = ALIGN( length ) ;                                       /* make alignment (longword aligned) */

        /*
         * Make sure to check if the parantheses (begin/end field markers)
         * are balanced
         */

        if ( IS_CONSTRUCTED( element->tag ) )
            balanced++ ;
        else
        if (element->tag == ( unsigned int )ASN1_C_EOC )
            balanced-- ;

        element = element->next_avl ;
    }

    if (balanced != 0)
        return( MAN_C_NOT_CONSTRUCTED ) ;

    /* 
     * Add the modifier, tag, and octet string and oid counts to the
     * overall length. This makes the assumption that ints are infact longwords.
     */

    length += count * sizeof( unsigned int ) * 2 ;        /* modifier and tag, per element */
    length += count * sizeof( unsigned int ) * 2 ;             /* octet and oid count per element */
    length += count * sizeof( unsigned int) ;             /* octet string data type */

    *len = length ;

    return( MAN_C_SUCCESS ) ;

} /* end of moss_avl_length() */


man_status
moss_avl_to_mybuf( 
                  avl_handle , 
                  buffer ,
                  len 
                 )

avl *avl_handle ;
char *buffer ;
int len ;

/*
 *
 * Function Description:
 *
 *    Copy the avl to the buffer. Note that the buffer is len bytes long. 
 *    Being good citizens, we check to make sure wwe don't overwrite our bounds.
 *
 * Parameters:
 *
 *    avl_handle   The pointer to an opaque AVL.
 *    buffer       The pointer to pre-allocated memory.
 *    len          The length of the buffer.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_BAD_PARAMETER    The avl_handle or buffer argument was NULL
 *    MAN_C_PROCESS_FAILURE  Buffer size is too small
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 *    Note that this routine and moss_avl_from_buf expect the structure of an AVL to be:
 *        oid:    {int,[int][...]},
 *        modifier:    {int},
 *        tag:    {int},
 *        item_flag:   {int},
 *        octet:  {int,int,[char][...]}
 */

{
    iavl *real_handle = ( iavl * )avl_handle ;  /* ptr to REAL AVL - used for accessing fields */
    avl_element *element ;
    int total = sizeof( int ) ;   /* account for overall counter at front of buffer */

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( ( avl_handle == NULL ) || ( buffer == NULL ) || ( len == 0 ) )
        return( MAN_C_BAD_PARAMETER ) ;

    element = real_handle->first_avl ;

    /*
     * Store the length.
     */

    if ( sizeof( int ) > len ) 
        return( MAN_C_PROCESSING_FAILURE ) ;

    memcpy( (void *)buffer, (void *)&len, sizeof( int ) ) ;
    buffer = buffer + sizeof(int);
 
    while( element->terminator == FALSE )
    {

        /*
         * check for the bounds
         */

        total += 3*sizeof( unsigned int ) ;                         /* modifier, tag, octet type */
        total += 2*sizeof( unsigned int ) ;                              /* octet and oid count */
        if ( element->oid.count != 0xffffffff )
            total += element->oid.count * sizeof( unsigned int ) ;  /* oid */
        if ( element->oct.length != 0xffffffff )
            total += element->oct.length * sizeof( char ) ;              /* octet */

        total = ALIGN( total ) ;
        
        if ( total > len ) 
            return( MAN_C_PROCESSING_FAILURE ) ;

        /* 
         * Move in the oid count.
         */

    	memcpy( (void *)buffer, (void *)&element->oid.count, sizeof( unsigned int ) ) ;
    	buffer = buffer + sizeof( unsigned int );

        /* 
         * Then the integer sequence.
         */

        if ( element->oid.count != 0xffffffff ) 
        {
	    memcpy( (void *)buffer, (void *)element->oid.value, sizeof( int ) * element->oid.count );
	    buffer = buffer + sizeof( int ) * element->oid.count;
        }

	/*
   	 * Then the modifier.
	 */

	memcpy( (void *)buffer, (void *)&element->modifier, sizeof( int ) ) ;
	buffer = buffer + sizeof(int);

	/*
   	 * Then the tag.
	 */

	memcpy( (void *)buffer, (void *)&element->tag, sizeof( int ) ) ;
	buffer = buffer + sizeof(int);	

        /* 
         * Move in the octet length. 
         */

    	memcpy( (void *)buffer, (void *)&element->oct.length, sizeof( unsigned int ) ) ;
    	buffer = buffer + sizeof( unsigned int );

        /*
         * Then the octet data type.             
         */

        memcpy( (void *)buffer, (void *)&element->oct.data_type, sizeof( unsigned int ) ) ;
        buffer = buffer + sizeof( unsigned int ) ;

        /*                          
         * Then the octet string.
         *
         * (Note at this point we are still longword aligned... the
         * character move may upset the longword alignment)
         */

        if ( element->oct.length != 0xffffffff )
          {
            memcpy( (void *)buffer, (void *)element->oct.string, element->oct.length ) ;
	    buffer = buffer + sizeof( char ) * (element->oct.length);
          }

        /* 
         * Now make sure we align on a longword boundry.
         */

        buffer = (char *)ALIGN( buffer ) ;

        element = element->next_avl ;
    }

    return( MAN_C_SUCCESS ) ;
    
} /* end of moss_avl_to_mybuf() */


man_status
moss_avl_from_buf( 
                  avl_handle , 
                  buffer 
                 )

avl *avl_handle ;
char *buffer ;

/*
 *
 * Function Description:
 *
 *     Translate flat buffer into an AVL. This facilitates transfer over address
 *    spaces.
 *
 * Parameters:
 *
 *    avl_handle        The pointer to an opaque AVL.
 *    buffer            A pointer to a buffer that gets translated into an AVL.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS            Success
 *    MAN_C_NOT_INITIALIZED    Handle is not initialized
 *    MAN_C_BAD_PARAMETER      Handle is NULL
 *    MAN_C_PROCESSING_FAILURE Error in memory allocation
 *    MAN_C_READ_ONLY          The AVL is an indexed AVL and can not be modified
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 *    Note that we DO NOT delete the from buffer.
 */

{
    iavl *iavl_handle = ( iavl * )avl_handle ;  /* ptr to REAL AVL - used for accessing fields */
    object_id oid ;
    octet_string octet ;
    unsigned int *int_ptr ;
    unsigned int modifier ;
    unsigned int tag ;
    int len ;
    man_status status ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( ( avl_handle == NULL ) || ( buffer == NULL ) )
        return( MAN_C_BAD_PARAMETER ) ;

    if ( iavl_handle->initialized != TRUE )
       return( MAN_C_NOT_INITIALIZED ) ;

    if ( iavl_handle->index_flag == TRUE )
        return( MAN_C_READ_ONLY ) ;

    int_ptr = ( unsigned int * )buffer ;

    /*
     * Extract the length of the buffer (the first int).
     */

    len = *int_ptr++ ;

    /*
     * This routine actually points into the flat AVL structure, then calls the 
     * AVL add routine which copies the data fromt the flat structure into 
     * the real representation of the AVL.
     */

    while( ( long )int_ptr < ( long )buffer + ( int )len )
    {

        /*
         * Get the object id.
         */

        oid.count = *int_ptr++ ;
        oid.value= int_ptr ; 
        if ( oid.count != 0xffffffff ) 
            int_ptr = ( unsigned int * )( ( long )int_ptr + ( int )oid.count * sizeof( int ) ) ;

        /*
         * Get the modifier and tag.
         */

        modifier = *int_ptr++ ;
        tag = *int_ptr++ ;

        /*
         * Get the octet.
         */

        octet.length = *int_ptr++ ;
        octet.data_type = *int_ptr++ ;
        octet.string = ( char * )int_ptr ;
        if ( octet.length != 0xffffffff ) 
            int_ptr = ( unsigned int * )( ( long )int_ptr + ( int )octet.length) ;

        int_ptr = (unsigned int *)ALIGN( int_ptr ) ;

        /*
         * OK. Crock time. if either the oid or octet was not specified (passed
         * in as a 0xffffffff in the (length) field, then make sure to add it as a NULL
         * (not specified).
         */

        if ( oid.count == 0xffffffff &&
             octet.length == 0xffffffff )
            status = moss_avl_add( avl_handle, NULL, modifier, tag, NULL ) ;
        else
        if ( oid.count == 0xffffffff )
            status = moss_avl_add( avl_handle, NULL, modifier, tag, &octet ) ;
        else
        if ( octet.length == 0xffffffff ) 
            status = moss_avl_add( avl_handle, &oid, modifier, tag, NULL ) ;
        else
            status = moss_avl_add( avl_handle, &oid, modifier, tag, &octet ) ;
	if ( status != MAN_C_SUCCESS )
	    return( status ) ;
    }

    iavl_handle->curr_avl = iavl_handle->first_avl ;

    return( MAN_C_SUCCESS ) ;

} /* end of moss_avl_from_buf() */


man_status
moss_avl_copy( 
	      avl_handle_dest ,
	      avl_handle_source ,
	      reset_dest ,
              oid_dest ,
	      modifier_dest ,
	      source_last_one 
             )

avl *avl_handle_dest ;
avl *avl_handle_source ;
int reset_dest ;
object_id *oid_dest ;
unsigned int *modifier_dest ;
int *source_last_one ;

/*
 *
 * Function Description:
 *
 *    Copy an element or entire constructed element from avl_handle_source
 *    to avl_handle_dest.
 *
 * Parameters:
 *
 *    avl_handle_dest		The desination of the append (opaque AVL).
 *    avl_handle_source		The opaque avl that is appended to avl_handle_dest.
 *    reset_dest                If true, reset the avl_handle_dest
 *    oid_dest                  Pointer to new object identifier value for copied
 *                              AVL.  If NULL, use object identifier from source.
 *    modifier_dest             Pointer to new modifier value for copied
 *                              AVL.  If NULL, use modifier from source.
 *    source_last_one           Address of an int that indicates if this is 
 *                              the last element in the source avl.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          The call was successful
 *    MAN_C_BAD_PARAMETER    One of the AVL handles was NULL
 *    MAN_C_NOT_INITIALIZED  One of the AVL handles was not initialized
 *    MAN_C_NO_ELEMENT       The source AVL is empty
 *    MAN_C_NOT_CONSTRUCTED  The construction is invalid (mismatched begin/end)
 *    MAN_C_PROCESSING_FAILURE An error occured in memory allocation
 *    MAN_C_READ_ONLY       The destination AVL is marked as read only
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 */

{
    iavl *iavl_handle_dest = ( iavl * )avl_handle_dest ;  /* ptr to REAL AVL - used for accessing fields */
    iavl *iavl_handle_source = ( iavl * )avl_handle_source ; /* ptr to REAL AVL - used for accessing fields */
    avl_element *element ;
    int count = 0 ;
    int balanced = 0 ;
    man_status status ;
    unsigned int modifier ;
    object_id *oid_p ;
    int first_one ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( ( avl_handle_source == NULL ) || ( avl_handle_dest == NULL ) )
        return( MAN_C_BAD_PARAMETER ) ;

    if ( ( iavl_handle_source->initialized != TRUE ) ||
	 ( iavl_handle_dest->initialized != TRUE ) )
       return( MAN_C_NOT_INITIALIZED ) ;

    element = iavl_handle_source->curr_avl ;
    if ( element->terminator == TRUE )
        return( MAN_C_NO_ELEMENT ) ;

    if ( iavl_handle_dest->index_flag == TRUE ) 
        return( MAN_C_READ_ONLY ) ;
    /*
     * Make sure to check if the parantheses (begin/end field markers)
     * are balanced
     */

    do
    {
        count++ ;

        if ( IS_CONSTRUCTED( element->tag ) )
            balanced++ ;
        else
        if (element->tag == ( unsigned int )ASN1_C_EOC )
            balanced-- ;

        element = element->next_avl ;
    } while ( ( element->terminator == FALSE ) && ( balanced != 0 ) ) ;

    if ( balanced != 0 )
        return( MAN_C_NOT_CONSTRUCTED ) ;

    /*
     *  Copy count avl elements -- only modify the modifier value
     *  on the first element, since on subsequent elements it may
     *  contain explicit ASN1 encoding information.
     */

    element = iavl_handle_source->curr_avl ;
    first_one = count ;
    for ( ; count > 0 ; count-- ) 
    {
	if ( count == first_one )
	{
            if ( oid_dest == NULL )
                oid_p = &element->oid ;
            else
                oid_p = oid_dest ;

	    if ( modifier_dest == NULL )
	        modifier = element->modifier ;
	    else
	        modifier = *modifier_dest ;
	}
	else
        {
            oid_p = &element->oid ;
	    modifier = element->modifier ;
        }

        if ( ( oid_p->count == 0xffffffff ) 
	    && ( element->oct.length == 0xffffffff ) )
	    status = moss_avl_add( avl_handle_dest, NULL, modifier ,
				   element->tag,  NULL ) ;
	else
        if ( oid_p->count == 0xffffffff )
	    status = moss_avl_add( avl_handle_dest, NULL, modifier ,
				   element->tag,  &element->oct ) ;
	else
        if ( element->oct.length == 0xffffffff )
	    status = moss_avl_add( avl_handle_dest , 
				   oid_p, modifier ,
				   element->tag,  NULL ) ;
	else
	    status = moss_avl_add( avl_handle_dest , 
				   oid_p, modifier ,
				   element->tag,  &element->oct ) ;

	if ( status != MAN_C_SUCCESS )
	    return( status ) ;
	element = element->next_avl ;
    }

    if ( source_last_one != NULL )
        *source_last_one = element->terminator ;

    iavl_handle_source->curr_avl = element ;

    if ( reset_dest == TRUE )
        moss_avl_reset( avl_handle_dest ) ;

    return( MAN_C_SUCCESS ) ;

} /* end of moss_avl_copy() */


man_status
moss_avl_exit_construction(
                           avl_handle ,
                           direction ,
                           number_of_levels ,
                           last_one
                          )

avl *avl_handle ;
int direction ;
int number_of_levels ;
int *last_one ;

/*
 *
 * Function Description:
 *
 *     Move the current AVL element out of the constucted item.
 *     If the flag is TRUE, then we will move forward in the AVL
 *     through the number of construction levels specified.  If
 *     the flag is FALSE, then we will move backwards in the same
 *     fashion.
 *
 * Parameters:
 *
 *    avl_handle        The pointer to an opaque AVL.
 *    direction         Flag indicating direction of movement
 *    number_of_levels  Number of construction levels to move
 *                        0 means move until construction level is 0
 *    last_one          The address to return the indication of the
 *                      pointer at the end or not, that is, the next
 *                      element is the terminator
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Either the avl handle or the last_one argument is NULL
 *    MAN_C_NO_ELEMENT       The AVL contains no elements
 *    MAN_C_NOT_CONSTRUCTED  The AVL was not pointing into a constructed item
 *
 * Side effects:
 *
 *    None
 *
 */

{
    iavl *iavl_handle = ( iavl * )avl_handle ;  /* ptr to REAL AVL - used for accessing fields */
    avl_element *element ;
    int original_level ;
    int stop_level ;
    man_status return_status ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( ( avl_handle == NULL ) || ( last_one == NULL ) )
        return( MAN_C_BAD_PARAMETER ) ;

    if ( iavl_handle->initialized != TRUE )
       return( MAN_C_NOT_INITIALIZED ) ;

    element = iavl_handle->first_avl ;
    if ( element->terminator == TRUE )
        return( MAN_C_NO_ELEMENT ) ;

    if ( iavl_handle->construction_level == 0 )
        return( MAN_C_NOT_CONSTRUCTED ) ;

    /*
     *  Save the construction level.
     */

    original_level = iavl_handle->construction_level ;

    /*
     *  Calculate construction level to stop at.
     */

    if ( number_of_levels == 0 )
        stop_level = 0 ;
    else
    {
        stop_level = original_level - number_of_levels ;
        if ( stop_level < 0 )
            stop_level = 0 ;
    }

    if ( direction == TRUE )
    {

        /*
         *  Move forward.
         */

        while ( moss_avl_next( avl_handle ) == MAN_C_SUCCESS )
            if ( iavl_handle->construction_level == stop_level )
                break ;

        /*
         *  This leaves the curr_avl pointing to the EOC with the
         *  appropriate level.  Must step past the EOC.
         */

        return_status = moss_avl_next( avl_handle ) ;
        element = iavl_handle->curr_avl ;
        *last_one = element->terminator ;
    }
    else
    {
        /*
         *  Move backward.
         */

        while( moss_avl_backup( avl_handle ) == MAN_C_SUCCESS )
            if ( iavl_handle->construction_level == stop_level )
                break ;

        /*
         *  If the construction level is 0, then we need to step
         *  forward to the start-of-construction element.  If not
         *  and the previous element is NULL, then we are at the
         *  beginning.
         */

        element = iavl_handle->curr_avl ;
        if ( ( iavl_handle->construction_level != 0 ) &&
             ( element->prev_avl == NULL ) )
            *last_one = TRUE ;
        else
        {
            return_status = moss_avl_next( avl_handle ) ;
            *last_one = FALSE ;
        }
    }

    return( MAN_C_SUCCESS ) ;

}   /* End of moss_avl_exit_construction()    */


man_status
moss_avl_find_item(
                   avl_handle ,
                   item_oid ,
                   modifier ,
                   tag ,
                   octet
                  )

avl *avl_handle ;
object_id *item_oid ;
octet_string **octet ;
unsigned int *tag ;
unsigned int *modifier ;

/*
 *
 * Function Description:
 *
 *    Find the item in the AVL.
 *
 * Parameters:
 *
 *    avl_handle        The pointer to an opaque AVL.
 *    item_oid          Address of search item's oid
 *    octet             Address to return octet information
 *    tag               Address to return tag information
 *    modifier          Address to return modifier information
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_NO_ELEMENT       The requested item was not found
 *
 * Side effects:
 *
 *    None
 *
 */

{
    iavl *iavl_handle = ( iavl * )avl_handle ; /* ptr to REAL AVL - used for accessing fields */
    avl_element *element ;
    avl_element *saved_element ;
    int saved_construction_level ;
    man_status return_status ;
    int last_element ;
    object_id *element_oid ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( avl_handle == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    if ( iavl_handle->initialized != TRUE )
       return( MAN_C_NOT_INITIALIZED ) ;

    saved_element = iavl_handle->curr_avl ;				    /* Save the current element and construction level.  */
    saved_construction_level = iavl_handle->construction_level ;

    return_status = moss_avl_reset ( avl_handle ) ;

    while ( moss_avl_point( avl_handle, &element_oid, modifier,
                            tag, octet, &last_element )  == MAN_C_SUCCESS )
    {

        /*
         * Since avl_point advances the pointer, look back.
         */

	element = iavl_handle->curr_avl ;
	element = element->prev_avl ;

        if ( ( element->item_flag == TRUE ) &&
             ( moss_compare_oid( item_oid, element_oid ) == MAN_C_EQUAL ) )
            return( MAN_C_SUCCESS ) ;

        if ( last_element == TRUE )
        {                                                                   /*  Hit the last element. */
            iavl_handle->curr_avl = saved_element ;                         /*  Restore the position and */
            iavl_handle->construction_level = saved_construction_level ;    /*  construction level.      */
            return( MAN_C_NO_ELEMENT ) ;
        }

    }

    iavl_handle->curr_avl = saved_element ;
    iavl_handle->construction_level = saved_construction_level ;
    return( MAN_C_NO_ELEMENT ) ;    
                    
}   /* End of moss_avl_find_item()  */


man_status
moss_avl_index_buf( 
                   avl_handle , 
                   buffer 
                  )

avl *avl_handle ;
char *buffer ;

/*
 *
 * Function Description:
 *
 *     Creates an index (read only) avl from a flat buffer.  The buffer is produced by the moss_avl_to_buf
 *     routine.
 *
 * Parameters:
 *
 *    avl_handle        The pointer to an opaque AVL.
 *    buffer            A pointer to a buffer that gets translated into an AVL.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS            Success
 *    MAN_C_NOT_INITIALIZED    Handle is not initialized
 *    MAN_C_BAD_PARAMETER      Handle is NULL
 *    MAN_C_PROCESSING_FAILURE Error in memory allocation
 *    MAN_C_READ_ONLY          The AVL is an indexed AVL and can not be modified
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 *    Note that we DO NOT delete the from buffer.
 */

{
    iavl *iavl_handle = ( iavl * )avl_handle ;  /* ptr to REAL AVL - used for accessing fields */
    object_id oid ;
    octet_string octet ;
    unsigned int *int_ptr ;
    unsigned int modifier ;
    unsigned int tag ;
    int len ;
    man_status status ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( ( avl_handle == NULL ) || ( buffer == NULL ) )
        return( MAN_C_BAD_PARAMETER ) ;

    if ( iavl_handle->initialized != TRUE )
       return( MAN_C_NOT_INITIALIZED ) ;

    if ( iavl_handle->index_flag == TRUE )  /*  If the AVL is already marked as read only exit. */
        return( MAN_C_READ_ONLY ) ;

    iavl_handle->index_flag = TRUE ;        /*  Now mark this as a read only index AVL.  */

    int_ptr = ( unsigned int * )buffer ;

    /*
     * Extract the length of the buffer (the first int).
     */

    len = *int_ptr++ ;

    /*
     * This routine actually points into the flat AVL structure, then calls the 
     * avl_index_add routine which copies the pointers to the data in the flat structure into 
     * the index AVL.
     */

    while( ( long )int_ptr < ( long )buffer + ( int )len )
    {

        /*
         * Get the object id.
         */

        oid.count = *int_ptr++ ;
        oid.value= int_ptr ; 
        if ( oid.count != 0xffffffff ) 
            int_ptr = ( unsigned int * )( ( long )int_ptr + ( int )oid.count * sizeof( int ) ) ;

        /*
         * Get the modifier and tag.
         */

        modifier = *int_ptr++ ;
        tag = *int_ptr++ ;

        /*
         * Get the octet.
         */

        octet.length = *int_ptr++ ;
        octet.data_type = *int_ptr++ ;
        octet.string = ( char * )int_ptr ;
        if ( octet.length != 0xffffffff ) 
            int_ptr = ( unsigned int * )( ( long )int_ptr + ( int )octet.length) ;

        int_ptr = (unsigned int *)ALIGN( int_ptr ) ;

        /*
         * OK. Crock time. if either the oid or octet was not specified (passed
         * in as a 0xffffffff in the (length) field, then make sure to add it as a NULL
         * (not specified).
         */

        if ( oid.count == 0xffffffff &&
             octet.length == 0xffffffff )
            status = avl_index_add( iavl_handle, NULL, modifier, tag, NULL ) ;
        else
        if ( oid.count == 0xffffffff )
            status = avl_index_add( iavl_handle, NULL, modifier, tag, &octet ) ;
        else
        if ( octet.length == 0xffffffff ) 
            status = avl_index_add( iavl_handle, &oid, modifier, tag, NULL ) ;
        else
            status = avl_index_add( iavl_handle, &oid, modifier, tag, &octet ) ;
	if ( status != MAN_C_SUCCESS )
	    return( status ) ;
    }

    iavl_handle->curr_avl = iavl_handle->first_avl ;

    return( MAN_C_SUCCESS ) ;

} /* end of moss_index_buff() */


#ifdef RPCV2
man_status
moss_avl_init_mutex( 
                    avl_handle , 
                    user_context_size ,
                    user_context_area
                   )

avl *avl_handle ;
int user_context_size ;
char **user_context_area ;

/*
 *
 * Function Description:
 *
 *     This routine allocates and initializes a mutex that is
 *     then associated with the avl.  It also allocates a user
 *     context buffer.
 *
 * Parameters:
 *
 *    avl_handle        The pointer to an opaque AVL.
 *    user_context_size The size of the user's context area
 *    user_context_area The address of the pointer to place the
 *                      address of the user's context area.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS                  Success
 *    MAN_C_BAD_PARAMETER            Handle is NULL
 *    MAN_C_INSUFFICIENT_RESOURCES   Error in memory allocation
 *    MAN_C_MUTEX_ALREADY_EXISTS     The avl already has a mutex
 *    MAN_C_NOT_INITIALIZED          Handle is not initialized
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 */

{
    iavl *iavl_handle ;  /*  Ptr to READ AVL - use for accessing fields */
    int status = 0 ;
    int *area_size = ( int * )NULL ;
    char *tmp_area = ( char * )NULL ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( avl_handle == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    iavl_handle = ( iavl * )avl_handle ;

    if ( iavl_handle->initialized != TRUE )
        return( MAN_C_NOT_INITIALIZED ) ;

    if ( iavl_handle->mutex_handle != ( pthread_mutex_t * )0 )
        return( MAN_C_MUTEX_ALREADY_EXISTS ) ;

    MOSS_MALLOC( tmp_area, char, sizeof( pthread_mutex_t ) + user_context_size + sizeof( int ) )
    if ( tmp_area == NULL )
        return( MAN_C_INSUFFICIENT_RESOURCES ) ;


    status = pthread_mutex_init( ( pthread_mutex_t * )tmp_area ,
                                 pthread_mutexattr_default ) ;
    if ( status == -1 )
    {
        MOSS_FREE( tmp_area ) ;
        return( MAN_C_INSUFFICIENT_RESOURCES ) ;
    }

    /*
     *  Put the size of the user's context area in the integer
     *  immediately following the pthread mutex.  We need this
     *  to ensure the user does get a bogus context area if they
     *  use the moss_avl_get_user_context() when no user context
     *  area was allocated.
     */

    iavl_handle->mutex_handle = ( pthread_mutex_t * )tmp_area ;
    area_size = ( int * )( tmp_area + sizeof( pthread_mutex_t ) ) ;
    *area_size = user_context_size ;
    if ( ( user_context_area != ( char ** )NULL  ) && ( user_context_size != 0 ) )
    {
        area_size++ ;
        *user_context_area = ( char * )( area_size ) ;
    }
    return( MAN_C_SUCCESS ) ;

} /* end of moss_avl_init_mutex() */


man_status
moss_avl_free_mutex( 
                    avl_handle 
                   )

avl *avl_handle ;

/*
 *
 * Function Description:
 *
 *     This routine frees the mutex and the associated user's
 *     context area.
 *
 * Parameters:
 *
 *    avl_handle        The pointer to an opaque AVL.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS            Success
 *    MAN_C_BAD_PARAMETER      Handle is NULL
 *    MAN_C_MUTEX_IS_LOCKED    The mutex is locked and cannot be freed
 *    MAN_C_NO_MUTEX           There is no mutex associated with the avl
 *    MAN_C_NOT_INITIALIZED    Handle is not initialized
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 */

{
    iavl *iavl_handle ;  /*  Ptr to READ AVL - use for accessing fields */
    int status = 0 ;
    int clear_size = 0 ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( avl_handle == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    iavl_handle = ( iavl * )avl_handle ;

    if ( iavl_handle->initialized != TRUE )
        return( MAN_C_NOT_INITIALIZED ) ;

    if ( iavl_handle->mutex_handle == ( pthread_mutex_t * )0 )
        return( MAN_C_NO_MUTEX ) ;

    status = pthread_mutex_destroy( iavl_handle->mutex_handle ) ;
    if ( status == -1 )
    {
        if ( errno == EBUSY )
            return( MAN_C_MUTEX_IS_LOCKED ) ;
        else
            return( MAN_C_BAD_PARAMETER ) ;
    }

    /*
     *  Zero out the pthread mutex and the integer containing the
     *  the length of the user's context area.
     */

    clear_size = sizeof( pthread_mutex_t ) + sizeof( int ) ;
    memset( ( void * )iavl_handle->mutex_handle, '\0', clear_size ) ;
    MOSS_FREE( iavl_handle->mutex_handle ) ;
    iavl_handle->mutex_handle = ( pthread_mutex_t * )NULL ;
    return( MAN_C_SUCCESS ) ;
        
} /* end of moss_avl_free_mutex() */


man_status
moss_avl_lock_mutex( 
                    avl_handle 
                   )

avl *avl_handle ;

/*
 *
 * Function Description:
 *
 *     This routine locks the mutex associated with the avl.
 *
 * Parameters:
 *
 *    avl_handle        The pointer to an opaque AVL.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS           Success
 *    MAN_C_BAD_PARAMETER     Handle is NULL
 *    MAN_C_NO_MUTEX          There is no mutex associated with the avl
 *    MAN_C_NOT_INITIALIZED   Handle is not initialized
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 */

{
    iavl *iavl_handle ;  /*  Ptr to READ AVL - use for accessing fields */
    int status = 0 ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( avl_handle == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    iavl_handle = ( iavl * )avl_handle ;

    if ( iavl_handle->initialized != TRUE )
        return( MAN_C_NOT_INITIALIZED ) ;

    if ( iavl_handle->mutex_handle == ( pthread_mutex_t * )0 )
        return( MAN_C_NO_MUTEX ) ;

    /*
     *  The only failure is EINVAL, which indicates
     *  an invalid mutex.
     */

    status = pthread_mutex_lock( iavl_handle->mutex_handle ) ;
    if ( status == -1 )
            return( MAN_C_BAD_PARAMETER ) ;

    return( MAN_C_SUCCESS ) ;

} /* end of moss_avl_lock_mutex() */


man_status
moss_avl_unlock_mutex( 
                      avl_handle 
                     )

avl *avl_handle ;

/*
 *
 * Function Description:
 *
 *     This routine unlocks the mutex associated with the avl.
 *
 * Parameters:
 *
 *    avl_handle        The pointer to an opaque AVL.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS           Success
 *    MAN_C_BAD_PARAMETER     Handle is NULL
 *    MAN_C_NO_MUTEX          The avl already has a mutex
 *    MAN_C_NOT_INITIALIZED   Handle is not initialized
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 */

{
    iavl *iavl_handle ;  /*  Ptr to READ AVL - use for accessing fields */
    int status = 0 ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( avl_handle == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    iavl_handle = ( iavl * )avl_handle ;

    if ( iavl_handle->initialized != TRUE )
        return( MAN_C_NOT_INITIALIZED ) ;

    if ( iavl_handle->mutex_handle == ( pthread_mutex_t * )0 )
        return( MAN_C_NO_MUTEX ) ;

    status = pthread_mutex_unlock( iavl_handle->mutex_handle ) ;

    /*
     *  The only possible error return is - EINVAL, which
     *  means the mutex is not locked.  So ignore the return value.
     */

    return( MAN_C_SUCCESS ) ;

} /* end of moss_avl_unlock_mutex() */


man_status
moss_avl_get_user_area( 
                       avl_handle , 
                       user_context_area
                      )

avl *avl_handle ;
char **user_context_area ;

/*
 *
 * Function Description:
 *
 *     This routine returns the address of a user's context
 *     area.  The context area is allocated when a mutex is
 *     associated with an avl and freed when the mutex is
 *     freed.
 *
 * Parameters:
 *
 *    avl_handle        The pointer to an opaque AVL.
 *    user_context_area The address of the pointer to place the
 *                      address of the user's context area.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS           Success
 *    MAN_C_BAD_PARAMETER     Handle is NULL
 *    MAN_C_NO_MUTEX          The avl already has a mutex
 *    MAN_C_NOT_INITIALIZED   Handle is not initialized
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 */

{
    iavl *iavl_handle ;  /*  Ptr to READ AVL - use for accessing fields */
    char *tmp_area ;
    int  *area_size ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( ( avl_handle == NULL ) || ( user_context_area == ( char ** )NULL ) )
        return( MAN_C_BAD_PARAMETER ) ;

    iavl_handle = ( iavl * )avl_handle ;

    if ( iavl_handle->initialized != TRUE )
        return( MAN_C_NOT_INITIALIZED ) ;

    if ( iavl_handle->mutex_handle == ( pthread_mutex_t * )0 )
        return( MAN_C_NO_MUTEX ) ;

    tmp_area = ( char * )iavl_handle->mutex_handle ;
    area_size = ( int * )( tmp_area + sizeof( pthread_mutex_t ) ) ;
    if ( *area_size == 0 )
        *user_context_area = ( char * )NULL ;
    else
        *user_context_area = ( char * )++area_size ;
    return( MAN_C_SUCCESS ) ;

} /* end of moss_avl_get_user_area() */
#endif /* RPCV2 */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Routine: moss_avl_copy_all()
**
**	Utility routine to copy all of the elements of one AVL to another AVL.
**
**
**  FORMAL PARAMETERS:
**
**    avl_handle_dest		The desination of the append (opaque AVL).
**    avl_handle_src		The opaque avl that is appended to avl_handle_dest.
**    reset_dest                If true, reset the avl_handle_dest
**
**
**  RETURN VALUE:
**
**    MAN_C_SUCCESS          The call was successful
**    MAN_C_BAD_PARAMETER    One of the AVL handles was NULL
**    MAN_C_NOT_INITIALIZED  One of the AVL handles was not initialized
**    MAN_C_NOT_CONSTRUCTED  The construction is invalid (mismatched begin/end)
**    MAN_C_PROCESSING_FAILURE An error occured in memory allocation
**    MAN_C_READ_ONLY       The destination AVL is marked as read only
**
**
**  SIDE EFFECTS:
**
**      The source and destination AVLs are left pointing at the AVL terminator.
**
**
**  CALLING SEQUENCE:
**
**  	mstatus = moss_avl_copy_all (a_dest_avl, a_src_avl, reset_dest);
**--
*/

man_status
moss_avl_copy_all (avl_handle_dest,
		   avl_handle_src,
		   reset_dest)

avl *avl_handle_dest;
avl *avl_handle_src;
int reset_dest;

{

    man_status	mstatus		= MAN_C_SUCCESS;	/* return status from moss_xxx() routines */
    int		last_one	= FALSE;		/* flag to/from moss_avl_copy() */


    /* reset the source AVL to its first element */
    mstatus = moss_avl_reset (avl_handle_src);
    if (mstatus != MAN_C_SUCCESS)
	{
	return mstatus;
	}


    /* reset the destination AVL to its first element if requested */
    if (reset_dest)
	{
	mstatus = moss_avl_reset (avl_handle_dest);
	if (mstatus != MAN_C_SUCCESS)
	    {
	    return mstatus;
	    }
	}


    /* copy each element in the source AVL to the destination AVL */
    do
	{
	mstatus = moss_avl_copy (avl_handle_dest, avl_handle_src,
				FALSE,
				(object_id *) NULL, (unsigned int *) NULL,
				&last_one);
	} while ((mstatus == MAN_C_SUCCESS) && (! last_one));
    /* as long as the element copy succeeded and we have not copied the last element */


    /* make a copy of an empty source AVL succeed */
    if (mstatus == MAN_C_NO_ELEMENT)
	{
	mstatus = MAN_C_SUCCESS;
	}


    /* pass the status of the copy operation back to the caller */
    return mstatus;
} /* moss_avl_copy_all */

/* end of moss_avl.c */
