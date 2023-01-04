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
static char *rcsid = "@(#)$RCSfile: moss_filter.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:32:17 $";
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
 *    The following contains filter support for manipulating CMISE filters,
 *    and for comparing instance names.  Theses routines are:
 *        moss_init_cmise_filter()
 *        moss_finish_cmise_filter()
 *        moss_free_cmise_filter()
 *        moss_add_cmise_filter()
 *        moss_start_cmise_or_set()
 *        moss_start_cmise_and_set()
 *        moss_add_cmise_not()
 *        moss_end_cmise_andornot_set()
 *        moss_read_cmise_filter()
 *        exact_match()
 *        level_one_match()
 *        moss_match_instance_name()
 *        moss_start_cmise_filter_conitem()
 *        moss_add_cmise_filter_conitem()
 *        moss_start_cmise_nested_conitem()
 *        moss_end_cmise_nested_conitem()
 *        moss_end_cmise_filter_conitem()
 *        moss_apply_smi_filter()
 *        moss_validate_filter()
 *
 * Author:
 *
 *    Wim Colgate
 *
 * Date:
 *
 *    December 21st, 1989
 *
 * Revision History :
 *
 *    Wim Colgate, February 22nd, 1990:
 *
 *    Changed filter construction to map to the new AVL constructed types,
 *    Fixed a bug in moss_apply_cmise_filter() - moss_compare() was passed
 *    The wrong variables!
 *
 *    Miriam Amos Nihart, February 27th, 1990:
 *
 *    Change the routines calling moss_avl_point to use a modifier and tag
 *    of unsigned long int *.
 *
 *    Wim Colgate, April 3rd, 1990.
 *
 *    fixed up 2 calls to bzero in moss_cmise_apply_filter() (size was incorrect).
 *
 *    Wim Colgate, May 10th, 1990.
 *
 *    Rewrote how avl elements are extracted from the filter, also, added a 
 *    MOM specific parameter to the apply filter routine.
 *
 *    Miriam Amos Nihart, May 14th, 1990.
 *
 *    Change the file names to reflect the 14 character restriction.
 *
 *    Wim Colgate, May 16th, 1990.
 *
 *    Added yet another construction routine to take care of nested constructions
 *    within a constructed filter item.
 *
 *    Wim Colgate, May, 30th, 1990.
 *
 *    fixed filter application to properly deal with complex data types.
 *
 *    Wim Colgate, May 31st, 1990.
 *
 *    Changed a parameter in moss_avl_start_cmise_nested_conitem().
 *
 *    Wim Colgate, June 1st, 1990.
 *
 *    Changed how strip_filter works. In addition to extracting the avl elements
 *    to compare againsts, extract JUST the object id's of the attributes to get.
 *    this may lead to an unbalanced avl (no EOC's), but that's OK; It is only
 *    an interim avl and never sent over the wire. The attribute-only avl is 
 *    passed to the mo's get routine.
 *
 *    Wim Colgate, June 14th, 1990.
 *
 *    Changed the interpretation of the modifier field in the apply_cmise_filter 
 *    routine. The MO is supposed to place a value in the modifier field of the
 *    return avl. Previously this was interpreted as a man_status code. Now
 *    it is a man_reply_codes. Also modified this section to check only the modifier
 *    of an avl element that has a corresponding object id.
 *
 *    Kathy Faust, June 18th, 1990.
 *
 *    Changed moss_apply_cmise_filter to always return MAN_C_TRUE if the the
 *    filter avl doesn't have any elements.    
 *
 *    Kathy Faust, June 20th, 1990.
 *
 *    Changed moss_apply_cmise_filter() to return MAN_C_FALSE if the modifier
 *    field of an output avl from the mo_get_routine is not understood.  Also
 *    added a check for a valid oid before checking oid->count in
 *    moss_apply_cmise_filter().
 *
 *    Kathy Faust, July 3rd, 1990.
 *
 *    Added status checks to calls to moss_avl_add, and updated return status
 *    information for routines that call moss_avl_add to include
 *    MAN_C_PROCESSING_FAILURE.
 *
 *    Miriam Amos Nihart, October 29th, 1990.
 *
 *    Change bzero to ansi equivalent memset.
 *
 *    Wim Colgate, November 8th, 1990.
 *
 *    Added two new routines: moss_validate_filter() and moss_apply_smi_filter().
 *    The first validates the filter - makes sure that all attributes are defined 
 *    by the same SMI, that all attributes have a comparison routine, and that the
 *    filter can, in fact, be evaluated. The second functions exactly like
 *    moss_apply_cmise_filter(), but a new parameter is added to explicitly define
 *    which comparison tables are to be used. This allows for an MOM to support
 *    multi-SMI filtering (like the event dispatcher). Removed moss_apply_cmise_filter().
 *
 *    This also required the routine (exact_match, and level_one_match and moss_compare,
 *    moss_match_instance_name) to take the new table_ptr parameter as well.
 *
 *    Wim Colgate, December 20th, 1990.
 *
 *    Changed return values in moss_apply_smi_filter() to be explicit 
 *    (MAN_C_PROCESSING_FAILURE and MAN_C_INVALID_OPERATOR) instead of the return values
 *    from moss_avl_xxxx routines.
 *
 *    Wim Colgate, January 8th, 1991.
 *
 *    Mapped failure returns from avl calls in moss_apply_smi_filter to 
 *    MAN_C_PROCESSING_FAILURE. (seems I missed one in the last modification).
 *
 *    Miriam Amos Nihart, March 13th, 1991.
 *
 *    Change modifier comparison in moss_apply_smi_filter to check for
 *    MAN_C__SUCCESS rather than MAN_C__NORMAL_REPLY.
 *
 *    Miriam Amos Nihart, June 13th, 1991.
 *
 *    Changes for merging of man status and reply codes.
 *
 *    Miriam Amos Nihart, October 30th, 1991.
 *
 *    Changes for prototyping.
 *
 *    Wim Colgate, November 18th, 1991.
 *
 *    Yet another attempt at simple name comparison (moss_match_instance_name)
 *
 */

/*
 *  Support header files
 */  

#include <string.h>
#include "man_data.h"
#include "man.h"

/*
 *  MOSS Specific header files
 */

#include "moss_private.h" 
#include "extern_nil.h"
#include "moss_cmp.h"

/*
 *  External
 */

/* 
 * Define some very local represenations of the operators.
 */

#define NONE_C -1
#define AND_C  128
#define OR_C   129
#define NOT_C  130

/*
 * This is an external routine.
 */

extern man_status moss_compare() ;


man_status
moss_init_cmise_filter(
                       filter
                      )
avl **filter ;

/*
 *
 * Function Description:
 *
 *    Initialize a cmise filter. The filter is represented as an AVL.
 *    Since a filter is really a constructed type, create a beginning record.
 *
 * Parameters:
 *
 *    filter         a pointer to a filter handle.
 *
 * Return value:
 *
 *      MAN_C_SUCCESS              Handle is successfully initialized
 *      MAN_C_ALREADY_INITIALIZED  Handle is already initialized
 *      MAN_C_BAD_PARAMETER        Handle is NULL
 *      MAN_C_PROCESSING_FAILURE   Error with memory allocation
 *
 * Side effects:
 *
 *    None
 *
 * Note:
 *    The return values are anything moss_avl_init() can return.
 *
 */

{
    man_status status ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    status = moss_avl_init( filter ) ;
    if ( status != MAN_C_SUCCESS )
        return( status ) ;

    status = moss_avl_start_construct( *filter,  &nil_object_id, 0, CMIS_C_FILTER_TYPE, &nil_octet_string ) ; 

    return( status ) ;

}  /* end of moss_init_cmise_filter() */


man_status
moss_finish_cmise_filter(
                         filter
                        )
avl *filter ;

/*
 *
 * Function Description:
 *
 *    This terminates a cmise filter.  The filter is represented as an AVL.
 *
 * Parameters:
 *
 *    filter         a pointer to a filter handle.
 *
 * Return value:
 *
 *     MAN_C_SUCCESS          Success
 *     MAN_C_NOT_INITIALIZED  Handle is not initialized
 *     MAN_C_BAD_PARAMETER    Handle is NULL
 *     MAN_C_NOT_CONSTRUCTED  The construction level is 0
 *     MAN_C_PROCESSING_FAILURE Error on memory allocation
 *
 * Side effects:
 *
 *     None
 *
 * Note:
 *     The return values are anything moss_avl_end_construct() can return.
 *
 */

{
#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    return( moss_avl_end_construct( filter ) ) ;
}  /* end of moss_finish_cmise_filter() */


man_status
moss_free_cmise_filter(
                       filter
                      )
avl **filter ;

/*
 *
 * Function Description:
 *
 *    Free a cmise filter. The filter is represented as an AVL.
 *
 * Parameters:
 *
 *    filter         a pointer to a filter handle.
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
 * Note:
 *    The return values are anything moss_avl_free() can return.
 *
 */

{
#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    return( moss_avl_free( filter, TRUE ) ) ;

}  /*  end of moss_free_cmise_filter() */


man_status
moss_add_cmise_filter_item( 
                           filter ,
                           attribute_id ,
                           tag ,
                           octet ,
                           relational_operation
                          )
avl *filter ;
object_id *attribute_id ;
unsigned int tag ;
octet_string *octet ;
cmise_filter_relation relational_operation ;

/*
 *
 * Function Description:
 *
 *    Add a filter element to the filter.
 *
 * Parameters:
 *
 *    filter          A pointer to a filter handle 
 *    attribute_id    A pointer to an object id
 *    tag             The asn1 data type of the attribute
 *    octet           A pointer to an octet string
 *    relational_operation A CMIS filter relation
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_NOT_CONSTRUCTED  The construction level is 0
 *    MAN_C_PROCESSING_FAILURE  Error in memory allocation
 *
 * Side effects:
 *
 *    None
 * 
 * Note:
 *    The return values are anything moss_avl_add() can return.
 *
 */

{
    man_status status ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    /*
     * Add the start constructed type marker.
     */

    status = moss_avl_start_construct( filter,  &nil_object_id, 0, CMIS_C_FILTER_ITEM, &nil_octet_string ) ; 
    if ( status != MAN_C_SUCCESS )
        return( status ) ;

    /* 
     * Add the attribute value pair. The operation is stored in the modifier. (relational_operation) 
     */

    status = moss_avl_add( filter, attribute_id, relational_operation, tag, octet ) ;
    if ( status != MAN_C_SUCCESS )
        return( status ) ;

    /*
     * Demarcate the end of this filter element.
     */

    status = moss_avl_end_construct( filter ) ;

    return( status ) ;
}  /* end of moss_add_cmise_filter_item() */


man_status
moss_start_cmise_or_set(
                        filter
                       )
avl *filter ;

/*
 *
 * Function Description:
 *
 *    Start the or set.
 *
 * Parameters:
 *
 *    filter         a pointer to a filter handle.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_NOT_CONSTRUCTED  The ASN1 tag is not of the constructed type
 *    MAN_C_PROCESSING_FAILURE Error on memory allocation
 *
 * Side effects:
 *
 *    None
 *
 * Note:
 *    The return values are anything moss_avl_start_construct() can return.
 *
 */

{
    man_status status ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    status = moss_avl_start_construct( filter,  &nil_object_id, 0, CMIS_C_OR_SET, &nil_octet_string ) ; 
    return( status ) ;

}  /* end of moss_start_cmise_or_set() */


man_status
moss_start_cmise_and_set(
                         filter
                        )
avl *filter ;

/*
 *
 * Function Description:
 *
 *    Start the and set.
 *
 * Parameters:
 *
 *    filter         a pointer to a filter handle.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_NOT_CONSTRUCTED  The ASN1 tag is not of the constructed type
 *    MAN_C_PROCESSING_FAILURE Error on memory allocation
 *
 * Side effects:
 *
 *    None
 *
 * Note:
 *    The return values are anything moss_avl_start_construct() can return.
 *
 */

{
    man_status status ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    status = moss_avl_start_construct( filter,  &nil_object_id, 0, CMIS_C_AND_SET, &nil_octet_string ) ; 
    return( status ) ;

}  /* end of moss_start_cmise_and_set() */


man_status
moss_add_cmise_not(
                   filter
                  )
avl *filter ;

/*
 *
 * Function Description:
 *
 *    add a NOT.
 *
 * Parameters:
 *
 *    filter         a pointer to a filter handle.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_NOT_CONSTRUCTED  The ASN1 tag is not of the constructed type
 *    MAN_C_PROCESSING_FAILURE Error on memory allocation
 *
 * Side effects:
 *
 *    None
 *
 * Note:
 *    The return values are anything moss_avl_start_construct() can return.
 *
 */

{
    man_status status ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    status = moss_avl_start_construct( filter,  &nil_object_id, 0, CMIS_C_NOT, &nil_octet_string ) ; 
    return( status ) ;

}  /* end of moss_add_cmise_not() */


man_status
moss_end_cmise_andornot_set(
                            filter
                           )
avl *filter ;

/*
 *
 * Function Description:
 *
 *    end the and or or set.
 *
 * Parameters:
 *
 *    filter         a pointer to a filter handle.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_NOT_CONSTRUCTED  The construction level is 0
 *    MAN_C_PROCESSING_FAILURE Error on memory allocation
 *
 * Side effects:
 *
 *    None
 *
 * Note:
 *    The return values are anything moss_avl_end_construct() can return.
 *
 */

{
#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    return( moss_avl_end_construct( filter ) ) ;

}  /* end of moss_end_cmise_andornot_set() */


man_status
moss_point_filter( 
               filter,
               oid, 
               modifier, 
               tag, 
               octet, 
               last_one 
              )

avl *filter ;
object_id **oid ;
unsigned int *modifier ;
unsigned int *tag ;
octet_string **octet ;
int *last_one ;

/*
 *
 * Function Description:
 *
 *    Ask the avl pointing routine to write into the passed in parameters.
 *
 * Parameters:
 *
 *    filter      The pointer to a pre-allocated intialized filter
 *    oid         An address of a pointer of an object identifier.
 *    tag         A pointer to an unsigned long int.
 *    modifier    A pointer to an unsigned long int.
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
#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    return( moss_avl_point( filter, oid, tag, modifier, octet, last_one ) ) ;

} /* end of moss_point_filter() */


man_status
moss_start_cmise_filter_conitem( 
                                filter ,
                                attribute_id ,
                                tag ,
                                octet ,
                                relational_operation
                               )
avl *filter ;
object_id *attribute_id ;
unsigned int tag ;
octet_string *octet ;
cmise_filter_relation relational_operation ;

/*
 *
 * Function Description:
 *
 *    Start a filter item that is a constructed data type to the filter.
 *
 * Parameters:
 *
 *    filter          A pointer to a filter handle 
 *    attribute_id    A pointer to an object id
 *    tag             The asn1 data type of the attribute
 *    octet           A pointer to an octet string
 *    relational_operation  A CMIS filter relational operation 
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_NOT_CONSTRUCTED  The construction level is 0
 *    MAN_C_PROCESSING_FAILURE Error on memory allocation
 *
 * Side effects:
 *
 *    None
 * 
 * Note:
 *    The return values are anything moss_avl_add() can return.
 *
 */

{
    man_status status ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    /*
     * Add the start constructed type marker.
     */

    status = moss_avl_start_construct( filter,  &nil_object_id, 0, CMIS_C_FILTER_ITEM, &nil_octet_string ) ; 
    if ( status != MAN_C_SUCCESS )
        return( status ) ;

    status = moss_avl_start_construct( filter, attribute_id, relational_operation, tag, octet ) ;

    return( status ) ;

}  /* end of moss_start_cmise_filter_item_constructed */


man_status
moss_add_cmise_conitem_field ( 
                              filter ,
                              explicit ,
                              tag ,
                              octet
                             )
avl *filter ;
unsigned int explicit ;
unsigned int tag ;
octet_string *octet ;

/*
 *
 * Function Description:
 *
 *    Add a piece of the constructed item data type to the filter.
 *
 * Parameters:
 *
 *    filter          A pointer to a filter handle 
 *    explicit        A context specific piece of information
 *    tag             The asn1 data type of the attribute
 *    octet           A pointer to an octet string
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_NOT_CONSTRUCTED  The construction level is 0
 *    MAN_C_PROCESSING_FAILURE Error on memory allocation
 *
 * Side effects:
 *
 *    None
 * 
 * Note:
 *    The return values are anything moss_avl_add() can return.
 *
 */

{
    man_status status ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    /*
     * Add the start constructed type marker.
     */

    status = moss_avl_add( filter, &nil_object_id, explicit , tag, octet ) ; 

    return( status ) ;

}  /* end of moss_add_cmise_filter_conitem */


man_status
moss_start_cmise_nested_conitem( 
                                filter ,
                                explicit ,
                                tag ,
                                octet
                               )
avl *filter ;
unsigned int explicit ;
unsigned int tag ;
octet_string *octet ;

/*
 *
 * Function Description:
 *
 *    Start a nested item to the already constructed filter
 *    item. 
 *
 * Parameters:
 *
 *    filter          A pointer to a filter handle 
 *    explicit        The supplemental information for this value
 *    tag             The asn1 data type of the attribute
 *    octet           A pointer to an octet string
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_NOT_CONSTRUCTED  The construction level is 0
 *    MAN_C_PROCESSING_FAILURE Error on memory allocation
 *
 * Side effects:
 *
 *    None
 * 
 * Note:
 *    The return values are anything moss_avl_add() can return.
 *
 */

{
    man_status status ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    status = moss_avl_start_construct( filter, &nil_object_id, explicit, tag, octet ) ;

    return( status ) ;

}  /* end of moss_start_cmise_nested_conitem */


man_status
moss_end_cmise_nested_conitem( 
                              filter 
                             )
avl *filter ;

/*
 *
 * Function Description:
 *
 *    Add a end-of-construction to the filter stream to complete the
 *    nested constructed filter item.
 *
 * Parameters:
 *
 *    filter          A pointer to a filter handle 
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_NOT_CONSTRUCTED  The construction level is 0
 *    MAN_C_PROCESSING_FAILURE Error with memory allocation
 *
 * Side effects:
 *
 *    None
 * 
 * Note:
 *    The return values are anything moss_avl_add() can return.
 *
 */

{
    man_status status ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    /*
     * Demarcate the end of the constructed type.
     */

    status = moss_avl_end_construct( filter ) ;

    return( status ) ;
} /* end of moss_end_nested_conitem */


man_status
moss_end_cmise_filter_conitem( 
                              filter 
                             )
avl *filter ;

/*
 *
 * Function Description:
 *
 *    Add a end-of-construction to the filter stream under the guise of
 *    ending a filter item.
 *
 * Parameters:
 *
 *    filter          A pointer to a filter handle 
 *
 * Return value:
 *
 *    MAN_C_SUCCESS          Success
 *    MAN_C_NOT_INITIALIZED  Handle is not initialized
 *    MAN_C_BAD_PARAMETER    Handle is NULL
 *    MAN_C_NOT_CONSTRUCTED  The construction level is 0
 *    MAN_C_PROCESSING_FAILURE Error on memory allocation
 *
 * Side effects:
 *
 *    None
 * 
 * Note:
 *    The return values are anything moss_avl_add() can return.
 *
 */

{
    man_status status ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    /*
     * Demarcate the end of the constructed type.
     */

    status = moss_avl_end_construct( filter ) ;
    if ( status != MAN_C_SUCCESS )
        return( status ) ;

    /*
     * Demarcate the end of this filter element.
     */

    status = moss_avl_end_construct( filter ) ;

    return( status ) ;
} /* end of moss_end_cmise_filter_item_constructed */


static
man_status
strip_filter( 
	     filter ,
	     return_avl ,
	     return_attributes
	     )
avl *filter ;
avl *return_avl ;
avl *return_attributes ;

/*
 *
 * Function Description:
 *
 *    strip out the filter information. We are going to leave ONLY
 *    data elements (and, of course, framework for constructed types).
 *
 *
 * Parameters:
 *
 *    filter           a pointer to a filter handle.
 *    return_avl       the remaining data elements (we will append to this avl).
 *
 * Return value:
 *
 *    MAN_C_SUCCESS             successful extraction.
 *    MAN_C_FAILURE             an error was encountered trying to extract the items.
 *
 * Side effects:
 *
 *    Plus anything that moss_avl_add can return.
 */

{
    int construction_level = 0 ;
    int filter_level = 0 ;
    object_id *oid;
    octet_string *octet ;
    int last_one ;
    unsigned int tag ;
    unsigned int modifier ;
    man_status status ;

    do 
    {
        status = moss_avl_point( filter, &oid, &modifier, &tag, &octet, &last_one ) ;
        if (status != MAN_C_SUCCESS)
            return( status ) ;

        /*
         * ANYTHING with an object id is included in the attributes avl.
         */

	if ( oid )
	{
            if ( oid->count != 0 )
            {
                status = moss_avl_add( return_attributes, oid, modifier, tag, octet ) ;
                if (status != MAN_C_SUCCESS)
                    return( status ) ;
            }
        }

        /* 
         * Anything that is CONTEXT_SPECIFIC and >= CMIS_C_BASE (32768) is assumed 
         * to be part of the filter directives. CONTEXT_SPECIFIC is represented by the last 2 bits
         * being 10 (binary). So shift the tag 30 bits and compare to '2' (decimal) to determine if 
         * it's CONTEXT_SPECIFIC.  (to see if the value is less than CMIS_C_BASE, logicaly and off the top 
         * 3 bits. The top 3 bits represent the table and the constructed/simple flag )
         */

        if ( ( (tag>>30) == 2 ) &&
             ( tag & 0x1fffffff ) >= CMIS_C_BASE )                  /* logically and off the top 3 bits */
        {
            if ( IS_CONSTRUCTED( tag ) )
                filter_level++ ;
        }
        else
        {
            if ( IS_CONSTRUCTED( tag ) )
            {
                construction_level++ ;
                status = moss_avl_add( return_avl, oid, modifier, tag, octet ) ;
                if ( status != MAN_C_SUCCESS )
                    return( status ) ;
            }
            else
            if ( tag == ASN1_C_EOC ) 
            {
                if ( construction_level > 0 )
                {
                    construction_level-- ;
                    status = moss_avl_add( return_avl, oid, modifier, tag, octet ) ;
                    if ( status != MAN_C_SUCCESS )
                        return( status ) ;
                }
                else
                if ( filter_level > 0 )
                {
                    filter_level-- ;
                }
                else
                    return ( MAN_C_FAILURE ) ;
            }
            else
            {
                status = moss_avl_add( return_avl, oid, modifier, tag, octet ) ;
                if ( status != MAN_C_SUCCESS )
                    return( status ) ;
            }
        }
    } while ( last_one == FALSE ) ;

    return( MAN_C_SUCCESS ) ;
}


man_status
moss_apply_smi_filter(
                        filter ,
                        mo_instance ,
                        mo_get_routine ,
                        mo_specific ,
                        table_ptr
                       )
avl *filter ;
avl *mo_instance ;
PF *mo_get_routine ;
char *mo_specific ;
comparison *table_ptr[] ;

/*
 *
 * Function Description:
 *
 *    apply the cmise filter
 *
 *    We will make two passes. One to gather all the attribute values (so that the idempotency
 *    of the filter application is preserved). We do this by extracting the attribute assertions,
 *    and pass the atribute list to the MO's get routine. We then go through and compare the values
 *    returned by the MO with the assertions handed to us inside the filter. Since the filter is
 *    expressed parenthetically, we will apply the filter using a stack based implementation.
 * 
 *    NOTE: If this routine gets modified, make sure that moss_validate_filter() also gets
 *    the necessary logic.
 *
 * Parameters:
 *
 *    filter           a pointer to a filter handle.
 *    mo_instance      an AVL that represents the identity of the instance
 *                     that we will apply the filter to.
 *    mo_get_routine   The MO's routine we can call to get the attributes
 *                     current values.
 *    mo_specific      A non-interpreted parameter sent back to the MOM's
 *                     mo_get_routine.
 *    table_ptr        A pointer to the set of comparison entry tables.
 *
 * Return value:
 *
 *    MAN_C_TRUE                Passed the filter
 *    MAN_C_FALSE               Did not pass the filter
 *    MAN_C_FILTER_TOO_COMPLEX  Filter not in the realm of comprehension
 *    MAN_C_INVALID_OPERATOR    Invalid operator within the filter
 *
 * Side effects:
 *
 *    None
 *
 */

{
    int opstack[256] ;
    int opcounter = 2 ;
    int opcode = NONE_C ;
    int value = 0 ;
    int operand1, operand2 ;
    int last_one ;
    object_id *oid ;
    octet_string *octet ;
    unsigned int modifier1 ;
    unsigned int tag ;
    int construction_level ;
    man_status status ;
    man_status filter_status ;
    avl *avl_attributes = NULL ;
    avl *avl_compares = NULL ;
    avl *avl_value = NULL ;
    avl_element *avl_el ;

    opstack[0] = 0 ;  /* null operand */
    opstack[1] = -1 ; /* no op */

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    /* 
     * If there is no filter, then we pass the filter.
     */

    if ( filter == NULL )
        return( MAN_C_TRUE ) ;

    /*
     * If we couldn't reset the filter, return error.
     */

    status = moss_avl_reset( filter ) ;
    if ( status != MAN_C_SUCCESS ) 
        return( MAN_C_INVALID_OPERATOR ) ;

    moss_avl_init( &avl_attributes ) ;
    moss_avl_init( &avl_compares ) ;
    moss_avl_init( &avl_value ) ;

    /*
     * Pass one: get all the data elements out of the filter, and all the attribute id's, 
     * build an AVL out of them, and pass it to the the mo's get routine. The mo should 
     * return the attributes and values into a new AVL.
     */

    status = strip_filter( filter, avl_compares, avl_attributes ) ;
    if ( status != MAN_C_SUCCESS ) 
    {
        moss_avl_free( &avl_compares, TRUE ) ;
        moss_avl_free( &avl_value, TRUE ) ;
        moss_avl_free( &avl_attributes, TRUE ) ;
	if ( status == MAN_C_NO_ELEMENT )
	    return( MAN_C_TRUE ) ;
	else
            return( MAN_C_INVALID_OPERATOR ) ;
    }

    moss_avl_reset( avl_attributes ) ;

    /*
     * If there were no data items (in a weakly constructed filter)
     * return a valid status anyway
     */

    avl_el = moss_avl_element( (iavl*)avl_attributes ) ;
    if ( avl_el == NULL )
        return( MAN_C_TRUE ) ;

    mo_get_routine( mo_instance, avl_attributes, avl_value, mo_specific ) ;

    moss_avl_reset( avl_value ) ;

    /*
     * Scan through the returned AVL to see if any of the attribute 'gets' were
     * in error. Note that some errors are OK. 
     */

    do
    {
        status = moss_avl_point( avl_value, &oid, &modifier1, &tag, &octet, &last_one ) ;
        if ( status != MAN_C_SUCCESS )
        {
            moss_avl_free( &avl_compares, TRUE ) ;
            moss_avl_free( &avl_value, TRUE ) ;
            moss_avl_free( &avl_attributes, TRUE ) ;
            return( MAN_C_INVALID_OPERATOR ) ;
        }

        /*
         * If there is a valid object id, then check the modifier 
         */

	if ( oid )
	{
            if ( oid->count > 0 )
            {
                if ( ( modifier1 != (int)MAN_C_SUCCESS ) && 
                     ( modifier1 != (int)MAN_C_MISSING_ATTRIBUTE_VALUE ) &&
                     ( modifier1 != (int)MAN_C_WRITE_ONLY_ATTRIBUTE ) )
                {
                    moss_avl_free( &avl_compares, TRUE ) ;
                    moss_avl_free( &avl_value, TRUE ) ;
                    moss_avl_free( &avl_attributes, TRUE ) ;
                    return( MAN_C_FALSE ) ;
                }
            }
        }
    } while (last_one != TRUE) ;

    /*
     * Pass two: Reset all AVL's, take apart the filter, and apply the operations on the returned values.
     */

    moss_avl_reset( filter ) ;
    moss_avl_reset( avl_compares ) ;
    moss_avl_reset( avl_value ) ;

    /*
     * the atributes avl is no longer needed
     */

    moss_avl_free( &avl_attributes, TRUE ) ;
     
    /*
     * Here's where the fun begins.
     */

    do
    {
        /*
         * Get AVL element from the filter.
         */

        status = moss_avl_point( filter, &oid, &modifier1, &tag, &octet, &last_one ) ;

        if ( IS_CONSTRUCTED( tag ) )
             switch( tag ) 
             {
                 case CMIS_C_FILTER_TYPE:
                      break ;

                 /*
                  * AND, OR and NOT will cause the current opcode and
                  * value (current evaluation of the expression) onto the
                  * the stack. It's as if we hit a parantheses in evaluation
                  * the expression. value is set to 1 (for AND) and 0 (for OR).
                  * This is so, because when we reach the close-partheses, we will
                  * perform the logical operation with value, and the result of the moss_compare
                  * routine. ANDing with 1, or ORing with 0 will yield proper results.
                  * NOT simply negates the result of the moss_compare.
                  */

                 case CMIS_C_AND_SET:
                      opstack[opcounter++] = value ;
                      opstack[opcounter++] = opcode ;
                      opcode = AND_C ;
                      value = 1 ;
                      break ;

                 case CMIS_C_OR_SET:    
                      opstack[opcounter++] = value ;
                      opstack[opcounter++] = opcode ;
                      opcode = OR_C ;
                      value = 0 ;
                      break ;

                 case CMIS_C_NOT:
                      opstack[opcounter++] = value ;
                      opstack[opcounter++] = opcode ;
                      opcode = NOT_C ;
                      break ;

                 /*
                  * We've reached a filter item, take the attribute assertion out and perform
                  * the current opcode on the result of moss_compare
                  */

                 case CMIS_C_FILTER_ITEM: 

                      /*
                       * Check what the filter is pointing to. Make sure that it is a valid
                       * filter element.
                       */

                      avl_el = moss_avl_element( (iavl*)filter ) ;
                      if (avl_el == NULL)
                          return( MAN_C_PROCESSING_FAILURE ) ;

                      /*
                       * Perform the comparison. pass in the two avl handles, and
                       * the operation to perform.
                       */

                      filter_status = moss_compare( avl_compares, avl_value, avl_el->modifier, table_ptr ) ;

                      /*
                       * Once we have returned from the comparison, both avl_compares and avl_value 
                       * should be pointing the the NEXT data item for the filter.
                       * BUT THE FILTER AVL IS NOT. We have to scan foward through the filter
                       * until we reach the the-end-of-construction that matches this filter
                       * item entry.
                       */

                      construction_level = 1 ;

                      while( construction_level > 0 )
                      {
                          status = moss_avl_next( filter ) ;
                          if ( status != MAN_C_SUCCESS ) 
                              return( MAN_C_PROCESSING_FAILURE ) ;

                          avl_el = moss_avl_element( (iavl*)filter ) ;
                          if (avl_el == NULL)
                              return( MAN_C_PROCESSING_FAILURE ) ;

                          if ( IS_CONSTRUCTED( avl_el->tag ) )
                              construction_level ++ ;
                          else 
                          if ( avl_el->tag == ASN1_C_EOC )
                              construction_level -- ;
                      }

                      /*
                       * Skip the on we ended on. It is an EOC for the FILTER_ITEM 
                       */

                      status = moss_avl_next( filter ) ;
                      if ( status != MAN_C_SUCCESS ) 
                          return( MAN_C_PROCESSING_FAILURE ) ;

                      if ( (filter_status != MAN_C_TRUE) && (filter_status != MAN_C_FALSE) )
                      {
                          moss_avl_free( &avl_compares, TRUE ) ;
                          moss_avl_free( &avl_value, TRUE ) ;
                          if ( filter_status == MAN_C_NO_ELEMENT)
                              return( MAN_C_PROCESSING_FAILURE ) ;
                          else
                              return( filter_status ) ;
                      }
                      else
                      {
                          if (filter_status == MAN_C_TRUE)
                              operand1 = TRUE ;
                          else
                              operand1 = FALSE ;
                      }

                      switch( opcode )
                      {
                          case AND_C:    
                               value = value & operand1 ;
                               break ;

                          case OR_C:
                               value = value | operand1 ;
                               break ;

                          case NONE_C:   
                               value = operand1 ;
                               break ;

                          case NOT_C:
                               if ( operand1 == TRUE )
                                   value = FALSE ;
                               else
                                   value = TRUE ;
                               break ;

                      }
                      break ;

                 default:     
                      return( MAN_C_INVALID_OPERATOR ) ;

            }

            /*
             * We have reached a close-parentheses. Push the current value on the stack, then
             * pop the operand, opcode, and operand from the stack. Then perform the operation.
             */

            else
            {
                 opstack[opcounter++] = value ;
                 operand1 = opstack[--opcounter] ;
                 opcode = opstack[--opcounter] ;
                 operand2 = opstack[--opcounter] ;

                 switch( opcode )
                 {
                     case AND_C:
                          value = operand1 & operand2 ;
                          break ;

                     case OR_C:     
                          value = operand1 | operand2 ;
                          break ;

                     case NONE_C:
                          value = operand1 ;
                          break ;

                     /*
                      * Note: the NOT operand does not utilize operand2. 
                      * e.g. the expression is NOT( operand1 ).
                      */

                     case NOT_C:    
                          if ( operand1 == TRUE )
                              value = FALSE ;
                          else
                              value = TRUE ;
                          break ;
                 }

             }

    } while ( last_one == FALSE ) ;

    if (value == TRUE)
        return( MAN_C_TRUE ) ;
    else
        return( MAN_C_FALSE ) ;
}  /* end of moss_apply_smi_filter () */


man_status
moss_validate_filter(
                        filter ,
                        table_ptr
                       )
avl *filter ;
comparison *table_ptr[ 4 ] ;

/*
 *
 * Function Description:
 *
 *    validate that the filter is evaluable. For a filter to be evaluable, 
 *    it must:
 *
 *    1) All attribute assertions belong to the same SMI.
 *    2) All attributes have a comparison entry in the SMI comparison table
 *    3) Be well constructed (i.e. balanced paranthetical constructs)
 *    4) Have recognizable operations.
 *
 *    Note that this routine is basically a cut/paste of apply_filter.
 *
 * Parameters:
 *
 *    filter           a pointer to a filter handle.
 *    table_ptr        A pointer to the set of comparison entry tables that are returned.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS             The filter is valid
 *    MAN_C_FAILURE             The filter is not valid
 *
 * Side effects:
 *
 *    None
 *
 */

{
    /* temporary hack */
    extern comparison universal_table [] ;
    extern comparison dna_application_table [] ;
    extern comparison dna_context_specific_table [] ;
    extern comparison dna_private_table [] ;
    /* temporary hack */

    int value = 0 ;
    int last_one ;
    object_id *oid ;
    octet_string *octet ;
    unsigned int modifier1 ;
    unsigned int tag ;
    int construction_level ;
    man_status status ;
    avl *avl_attributes = NULL ;
    avl *avl_compares = NULL ;
    avl_element *avl_el ;
    comparison *table_entry ;
    comparison *lookup() ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    /* 
     * If there is no filter, then it is still 'evaluable'
     */

    if ( filter == NULL )
        return( MAN_C_SUCCESS ) ;

    /*
     * If we couldn't reset the filter, return error.
     */

    status = moss_avl_reset( filter ) ;
    if ( status != MAN_C_SUCCESS ) 
        return( MAN_C_FAILURE ) ;

    /*
     *  Check for avl with no elements.
     */

    status = moss_avl_point( filter ,
                             &oid ,
                             &modifier1 ,
                             &tag ,
                             &octet ,
                             &last_one ) ;
    if ( status == MAN_C_NO_ELEMENT )
        return( MAN_C_SUCCESS ) ;

    status = moss_avl_reset( filter ) ;

    /*
     * First step: get all the data elements out of the filter, and all the attribute id's, 
     * build an AVL out of them. They will be used to lookup in the MIR and the SMI 
     * comparison tables.
     */

    moss_avl_init( &avl_attributes ) ;
    moss_avl_init( &avl_compares ) ;

    status = strip_filter( filter, avl_compares, avl_attributes ) ;
    if ( status != MAN_C_SUCCESS ) 
    {
        moss_avl_free( &avl_compares, TRUE ) ;
        moss_avl_free( &avl_attributes, TRUE ) ;
	return( MAN_C_FAILURE ) ;
    }

    /* 
     * we are not interested in the values of the filter...
     */

    moss_avl_free( &avl_compares, TRUE ) ;

    /*
     * If there were no data items (in a weakly constructed filter)
     * it is still an evaluable filter 
     */

    moss_avl_reset( avl_attributes ) ;

    avl_el = moss_avl_element( (iavl*)avl_attributes ) ;
    if ( avl_el == NULL )
    {
        moss_avl_free( &avl_attributes, TRUE ) ;
        return( MAN_C_SUCCESS ) ;
    }

    do
    {
        status = moss_avl_point( avl_attributes, &oid, &modifier1, &tag, &octet, &last_one ) ;
        if (status != MAN_C_SUCCESS ) 
        {
            moss_avl_free( &avl_attributes, TRUE ) ;
            return( MAN_C_FAILURE ) ;
        }

        /* do MIR call to get SMI on object oid. - make sure they are all the same ... (wkc)*/

    } while ( last_one != TRUE ) ;

    /* build the table_ptr from the SMI we figured out.... temporary hack... (wkc) */
    table_ptr[ 0 ] = universal_table ;
    table_ptr[ 1 ] = dna_application_table ;
    table_ptr[ 2 ] = dna_context_specific_table ;
    table_ptr[ 3 ] = dna_private_table ;

    /*
     * the atributes avl is no longer needed
     */

    moss_avl_free( &avl_attributes, TRUE ) ;
     
    /*
     * Pass two: Reset the filter, and give it a run
     */

    moss_avl_reset( filter ) ;

    /*
     * Here's where the fun begins.
     */

    do
    {
        /*
         * Get AVL element from the filter.
         */

        status = moss_avl_point( filter, &oid, &modifier1, &tag, &octet, &last_one ) ;

        if ( IS_CONSTRUCTED( tag ) )
             switch( tag ) 
             {
                 /*
                  * For the perpose of validating, these are not interesting
                  */

                 case CMIS_C_FILTER_TYPE:
                 case CMIS_C_AND_SET:
                 case CMIS_C_OR_SET:    
                 case CMIS_C_NOT:
                      break ;

                 /*
                  * We've reached a filter item, check to make sure it has got a comparison routine
                  */

                 case CMIS_C_FILTER_ITEM: 

                      /*
                       * Check what the filter is pointing to. Make sure that it is a valid
                       * filter element.
                       */

                      avl_el = moss_avl_element( (iavl*)filter ) ;
                      if (avl_el == NULL)
                          return( MAN_C_FAILURE ) ;

                      /*
                       * Lookup the tag in the smi defined comparison routines.
                       */

                      table_entry = lookup ( avl_el->tag, table_ptr ) ;
                      if ( table_entry == NULL )
                          return( MAN_C_FAILURE ) ;

                      /*
                       * Once we have returned from the comparison, both avl_compares and avl_value 
                       * should be pointing the the NEXT data item for the filter.
                       * BUT THE FILTER AVL IS NOT. We have to scan foward through the filter
                       * until we reach the the-end-of-construction that matches this filter
                       * item entry.
                       */

                      construction_level = 1 ;

                      while( construction_level > 0 )
                      {
                          status = moss_avl_next( filter ) ;
                          if ( status != MAN_C_SUCCESS ) 
                              return( MAN_C_FAILURE ) ;

                          avl_el = moss_avl_element( (iavl*)filter ) ;
                          if (avl_el == NULL)
                              return( MAN_C_FAILURE ) ;

                          if ( IS_CONSTRUCTED( avl_el->tag ) )
                              construction_level ++ ;
                          else 
                          if ( avl_el->tag == ASN1_C_EOC )
                              construction_level -- ;
                      }

                      /*
                       * Skip the on we ended on. It is an EOC for the FILTER_ITEM 
                       */

                      status = moss_avl_next( filter ) ;
                      if ( status != MAN_C_SUCCESS ) 
                          return( MAN_C_FAILURE ) ;
                      break ;

                 default:     
                      return( MAN_C_FAILURE ) ;

            }

            /*
             * The else clause is not interesting
             */

    } while ( last_one == FALSE ) ;

    return( MAN_C_SUCCESS ) ;

}  /* end of moss_validate_filter () */


man_status
exact_match(
            source_instance ,
            candidate_instance ,
            table_ptr 
           ) 
avl *source_instance ;
avl *candidate_instance ;      
comparison *table_ptr[] ;

/*
 *
 * Function Description:
 *
 *    Compare the two instance names and look for an EXACT match.
 *    The purpose of this routine is to compare two instance names.
 *
 * Parameters:
 *
 *    source_instance    A pointer to an AVL of the first instance name 
 *    candidate_instance A pointer to an AVL of the instance name to compare the first instance name to.
 *    table_ptr          A pointer to the set of comparison entry tables.
 *
 * Return value:
 *
 *    MAN_C_TRUE   Instance names match
 *    MAN_C_FALSE  Instance names are not a match
 *
 * Side effects:
 *
 *    None
 *
 */
{
    avl_element *avl_el1 ;
    avl_element *avl_el2 ;
    man_status status ;

    do
    {
        avl_el1 = moss_avl_element( (iavl *)source_instance ) ;
        avl_el2 = moss_avl_element( (iavl *)candidate_instance ) ;

        /*
         * If we are at the end of both, they are equal.
         */

        if ((avl_el1 == NULL) && (avl_el2 == NULL))
            return( MAN_C_TRUE ) ;

        /*
         * If one and not the other is at the end, they are not equal.
         */

        if ((avl_el1 == NULL) || (avl_el2 == NULL))
            return( MAN_C_FALSE ) ;

        /*
         * If both AVL elements are the same field markers, then we can skip comaring them.
         */

        if ( ( IS_CONSTRUCTED( avl_el1->tag ) && IS_CONSTRUCTED( avl_el2->tag ) ) ||
             ( ( avl_el1->tag == ( unsigned int )ASN1_C_EOC ) &&
               ( avl_el2->tag == ( unsigned int )ASN1_C_EOC ) )
           )
        {
            status = moss_avl_next( source_instance ) ;
            status = moss_avl_next( candidate_instance ) ;
        }
        else
        {

            /*
             * Note that the comparison routine is responsible for adjusting the avl
             * pointers to the place *after* the current comparison
             */

            status = moss_compare( source_instance, candidate_instance, CMIS_C_EQUALITY, table_ptr ) ;
            if (status != MAN_C_TRUE)
                return( status ) ;
        }

        /*
         * If we have reached the end of both of them (at the same time)
         * then they are equal, otherwise they are not equal.
         */

        avl_el1 = moss_avl_element( (iavl *)source_instance ) ;
        avl_el2 = moss_avl_element( (iavl *)candidate_instance ) ;
        if ( avl_el1 == NULL ) 
        {
            if ( avl_el2 != NULL )
                return( MAN_C_FALSE ) ;
        }
        else
        if (avl_el2 == NULL )
            return( MAN_C_FALSE ) ;

    } while ( 1 ) ;

}  /* end of exact_match() */


man_status
level_one_match(
                source_instance ,
                candidate_instance ,
                table_ptr
               ) 
avl *source_instance ;
avl *candidate_instance ;
comparison *table_ptr[] ;

/*
 *
 * Function Description:
 *
 *    Compare the two instance names - source_instance must match the prefix of candidate_instance,
 *    AND candidate_instance must be longer than source_instance (at least one more name...)
 *
 *    The purpose of this routine is to implement scoping as it pertains to instance names.
 *    Since one can only scope on the last element of an instance name, we want to check that
 *    all pieces of the instance name between the source and candidate are equal, EXCEPT that
 *    the candidate must have at least one more element in its name. For example, if the source
 *    name is 'name.this.foo' then 'name.this.foo.bar' would match, but 'name.this.foo' would not.
 *
 * Parameters:
 *
 *    source_instance    A pointer to an AVL of the first instance name 
 *    candidate_instance A pointer to an AVL of the instance name to compare the first instance name to.
 *    table_ptr          A pointer to the set of comparison entry tables.
 *
 * Return value:
 *
 *    MAN_C_TRUE   Prefixes match
 *    MAN_C_FALSE  Prefixes do not match
 *
 * Side effects:
 *
 *    None
 *
 */
{
    avl_element *avl_el1 ;
    avl_element *avl_el2 ;
    man_status status ;

    do
    {
        avl_el1 = moss_avl_element( (iavl *)source_instance ) ;
        avl_el2 = moss_avl_element( (iavl *)candidate_instance ) ;

        /*
         * If we are at the end of source_instance, source_instance must match the prefix of candidate_instance, 
         * AND candidate_instance must have at least one more name element.
         */

        if (avl_el1 == NULL)
        {

            /*
             * Search candidate_instance looking for a real piece of an instance name...
             */

            while (avl_el2 != NULL)
            {
                if ( IS_CONSTRUCTED( avl_el2->tag ) &&
                   ( avl_el2->tag != ( unsigned int)ASN1_C_EOC ) )
                    return( MAN_C_TRUE ) ;

                /*
                 * Try the next AVL element...
                 */

                moss_avl_next( candidate_instance ) ;
                avl_el2 = moss_avl_element( (iavl *)candidate_instance ) ;
            }
            return( MAN_C_FALSE) ;
        }

        /*
         * If we have reached the end of candidate_instance, then we have failed to match.
         */

        if (avl_el2 == NULL)
            return( MAN_C_FALSE ) ;

        /*
         * If both AVL elements are the same field markers, then we can skip comaring them.
         */

        if ( ( IS_CONSTRUCTED( avl_el1->tag ) && IS_CONSTRUCTED( avl_el2->tag ) ) ||
             ( ( avl_el1->tag == ( unsigned int )ASN1_C_EOC ) &&
               ( avl_el2->tag == ( unsigned int )ASN1_C_EOC ) )
           )
        {
            status = moss_avl_next( source_instance ) ;
            status = moss_avl_next( candidate_instance ) ;
        }
        else
        {

            /*
             * Note that the comparison routine is responsible for adjusting the avl
             * pointers to the place *after* the current comparison
             */

            status = moss_compare( source_instance, candidate_instance, CMIS_C_EQUALITY, table_ptr ) ;
            if (status != MAN_C_TRUE)
                return( status ) ;
        }

        /*
         * If we have reached the end of both of them (at the same time)
         * then they are equal, otherwise they are not equal.
         */

        avl_el1 = moss_avl_element( (iavl *)source_instance ) ;
        avl_el2 = moss_avl_element( (iavl *)candidate_instance ) ;
        if ( avl_el1 == NULL ) 
        {
            if ( avl_el2 != NULL )
                return( MAN_C_FALSE ) ;
        }
        else
        if (avl_el2 == NULL )
            return( MAN_C_FALSE ) ;

    } while ( 1 ) ;

}  /* end of level_one_match() */


man_status
moss_match_instance_name(
                         source_instance ,
                         candidate_instance ,
                         iscope ,
                         table_ptr
                        )
avl *source_instance ;
avl *candidate_instance ;
unsigned int iscope ;
comparison *table_ptr[] ;

/*
 *
 * Function Description:
 *
 *    Compare the two instance names in context of a scope.
 *    An instance name is a sequence of relative distinguished names.
 *
 * Parameters:
 *
 *    source_instance    A pointer to an AVL of the first instance name 
 *    candidate_instance A pointer to an AVL of the instance name to compare the first instance name to.
 *    iscope             An integer scope which describes the depth of our comparison.
 *    table_ptr          A pointer to the set of comparison entry tables.
 *
 * Return value:
 *
 *    MAN_C_TRUE              Match found
 *    MAN_C_FALSE             Match failed
 *    MAN_C_INVALID_OPERATOR  Invalid operator
 *    MAN_C_BAD_PARAMETER     AVL contains a bad parameter
 *    MAN_C_NOT_INITIALIZED   AVL not initialized
 *
 * Side effects:
 *
 *    None
 *
 */

{
    man_status status ;
    unsigned char *split_scope = (unsigned char *)&iscope ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    status = moss_avl_reset( source_instance ) ;
    if (status != MAN_C_SUCCESS)
        return( status ) ;

    status = moss_avl_reset( candidate_instance ) ;
    if (status != MAN_C_SUCCESS)
        return( status ) ;

    /*
     * Break the instance name matching into 4 cases:
     *
     *       scope level    
     *                0+0              This means base object only, and thus we must
     *                                 perform an exact match only.
     *                0+1              This means all subordinates only, and thus we must
     *                                 perform a level one match only.
     *                0+2, 1+1, 2+1    This means base object and at least one level of
     *                                 subordinates, thus we perform an exact match, AND
     *                                 a level one match. (note that if the exact match returns
     *                                 true, there is no need to try a level one match...)
     *
     *                anything else is invalid
     */

    switch( split_scope[0] )
    { 
        case 0:
            switch( split_scope[1] )
            {
                case 0:
                    status = exact_match( source_instance, candidate_instance, table_ptr ) ;
                    break ;

                case 1:
                    status = level_one_match( source_instance, candidate_instance, table_ptr ) ;
                    break ;

                case 2:
                    status = exact_match( source_instance, candidate_instance, table_ptr ) ;

                    /*
                     * The exact match failed.  Try for a level one match.
                     */

                    if (status == MAN_C_FALSE)
                        status = level_one_match( source_instance, candidate_instance, table_ptr ) ;
                    break ;
                          
            }

            break ;

        case 1:
        case 2:
            switch( split_scope[1] )
            {
                case 1:
                    status = exact_match( source_instance, candidate_instance, table_ptr ) ;

                    /*
                     * The exact match failed, try for a level one match.
                     */  

                    if (status == MAN_C_FALSE)
                        status = level_one_match( source_instance, candidate_instance, table_ptr ) ;
                    break ;

                default:
                    status = MAN_C_INVALID_OPERATOR ;
                    break ;
            }
            break ;

        default:
            status = MAN_C_INVALID_OPERATOR ;
            break ;
    }

    return( status ) ;

}  /* end of moss_match_instance_name() */

/* end of moss_filter.c */
