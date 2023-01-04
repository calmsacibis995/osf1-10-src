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
/* DEC/CMS REPLACEMENT HISTORY, Element BMI_QUERY.C*/
/* *4     9-JUN-1992 10:00:43 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *3     3-MAR-1992 17:07:50 KARDON "UCXed"*/
/* *2     1-NOV-1991 12:48:20 BALLENGER "Reintegrate  memex support"*/
/* *1    16-SEP-1991 12:42:40 PARMENTER "Querying the Linkworks repository"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BMI_QUERY.C*/
#ifndef VMS
 /*
#else
#module BMI_QUERY "V03-0002"
#endif
#ifndef VMS
  */
#endif
/*
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1990  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use,	     **
**  duplication or dissemination of the software and	     **
**  media is authorized only pursuant to a valid written     **
**  license from Digital Equipment Corporation.	    	     **
**  	    	    	    	    	    	    	     **
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	     **
**  disclosure by the U.S. Government is subject to 	     **
**  restrictions as set forth in Subparagraph (c)(1)(ii)     **
**  of DFARS 252.227-7013, or in FAR 52.227-19, as  	     **
**  applicable.	    	    	    	    	    	     **
***************************************************************
*/

/*
**++
**  FACILITY:
**
**      Bookreader Memex Interface (bmi_*)
**
**  ABSTRACT:
**
**	Routines for querying the Hyperinformation repository.
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     02-Jul-1990
**
**  MODIFICATION HISTORY:
**
**  V03-0002 DLB0002		David L Ballenger	30-Apr-1991
**           Fix problems with handling of surrogate objects in the
**           library window.
**
**
**  V03-0001 DLB0001     David L Ballenger           01-Mar-1991
**           Fix problems with surrogate highlighting after the composite
**           network changes for QAR 807.  LinkWorks naming convention changes.
**
**--
**/



#include "bmi_private_defs.h"
#include "bmi_surrogate_defs.h"
#include "bmi_query.h"
#include "bmi_user_interface.h"


/* Query nodes
 */
static lwk_query_node Any = {lwk_c_any,0,0};

/* Nodes for matching surrogate subtype
 */
static lwk_query_node SubTypeName = {lwk_c_string_literal, 
                                     (lwk_any_pointer)BMI_SURROGATE_SUBTYPE,
                                     NULL};
static lwk_query_node SubTypeProp = {lwk_c_property_value,
                                     (lwk_any_pointer)lwk_c_p_surrogate_sub_type,
                                     NULL};
static lwk_query_node SubTypeIs   = {lwk_c_string_is_eql,
                                     (lwk_any_pointer)&SubTypeProp,
                                     (lwk_any_pointer)&SubTypeName};
static lwk_query_node HasSubtype = {lwk_c_has_properties,
                                    (lwk_any_pointer)&SubTypeIs,

                                    NULL};

/* Nodes for matching object type
 */
static lwk_query_node ObjectTypeName = {lwk_c_string_literal,NULL,NULL}; 
static lwk_query_node ObjectTypeProp = {lwk_c_property_value,
                                        (lwk_any_pointer)BMI_PROP_OBJECT_TYPE,
                                        NULL};
static lwk_query_node ObjectTypeIs   = {lwk_c_string_is_eql,
                                        (lwk_any_pointer)&ObjectTypeProp,
                                        (lwk_any_pointer)&ObjectTypeName};


static lwk_query_node ContainerName = {lwk_c_string_literal,NULL,NULL};
static lwk_query_node ContainerProp = {lwk_c_property_value,
                                       (lwk_any_pointer)BMI_PROP_CONTAINER,
                                       NULL};
static lwk_query_node ContainerIs   = {lwk_c_string_is_eql,
                                       (lwk_any_pointer)&ContainerProp,
                                       (lwk_any_pointer)&ContainerName};

static lwk_query_node ObjectInContainer = {lwk_c_and,
                                           (lwk_any_pointer)&ObjectTypeIs,
                                           (lwk_any_pointer)&ContainerIs};

static lwk_query_node  HasOtherProps = 
                       {lwk_c_has_properties,
                        (lwk_any_pointer)&ObjectInContainer,
                        NULL};

static lwk_query_node Query = {lwk_c_and,
                                   (lwk_any_pointer)&HasSubtype,
                                   (lwk_any_pointer)&HasOtherProps};


lwk_status
bmi_query_book PARAM_NAMES((book,object_type,callback))
    BKR_BOOK_CTX_PTR book PARAM_SEP
    char *object_type PARAM_SEP
    lwk_callback *callback PARAM_END

/*
 *
 * Function description:
 *
 *      Routine to query the current composit network for bookreader
 *      directory or topic surrogate objects.
 *
 * Arguments:
 *
 *      book        - pointer to the bmi context for the book
 *      object_type - name of the Bookreader object type for directories
 *                    or topics
 *      callback    - a callback routine for saving the surrogates.
 *
 * Return value:
 *
 *      An lwk_status value indicating success or failure.
 *
 * Side effects:
 *
 *      The current composite network will be updated if it is not valid.
 *
 */

{
    lwk_status status;
    lwk_termination termination; 
    lwk_composite_linknet cnet;

    ContainerName.lwk_operand_1 = (lwk_any_pointer)book->filename;
    ObjectTypeName.lwk_operand_1 = (lwk_any_pointer) object_type;

#ifdef MEMEX_DEBUG
    fprintf(stderr,
            "Querying book %s for object type %s.\n",
            book->container,
            object_type
            );
#endif 

    if (bmi_context.composite_net_valid == FALSE) {
        RETURN_ON_ERROR(bmi_update_composite_network());
    }
    if (bmi_context.current_composite_net == lwk_c_null_object) {
#ifdef MEMEX_DEBUG
        fprintf(stderr,"Composite network is null.\n");
#endif 
        return lwk_s_success;
    }

    status = lwk_query(bmi_context.current_composite_net,
                       lwk_c_domain_surrogate,
                       &Query,
                       (lwk_closure)book,
                       callback,
                       &termination
                       );
    PRINT_STATUS("lwk_query()",status)
    return status;
} /* end bmi_query_book () */

lwk_status
bmi_query_shelf PARAM_NAMES((shelf,callback))
    BKR_NODE_PTR shelf PARAM_SEP
    lwk_callback callback PARAM_END

/*
 *
 * Function description:
 *
 *      Routine to query the current composit network for bookreader
 *      shelf entry surrogate objects.
 *
 * Arguments:
 *
 *      shelf    - a pointer to the BKR_NODE for the shelf that we want
 *                 to find surrogates for
 *      callback - a callback routine for saving the surrogates.
 *
 * Return value:
 *
 *      An lwk_status value indicating success or failure.
 *
 * Side effects:
 *
 *      The current composite network will be updated if it is not valid.
 *
 */

{
    lwk_status status;
    lwk_termination termination; 
    lwk_composite_linknet cnet;

    ContainerName.lwk_operand_1 = (lwk_any_pointer)shelf->u.shelf.path;
    ObjectTypeName.lwk_operand_1 = (lwk_any_pointer)BMI_OBJ_NAME_SHELF;

#ifdef MEMEX_DEBUG
    fprintf(stderr,"Querying library for shelf %s\n",shelf->u.shelf.path);
#endif 

    if (bmi_context.composite_net_valid == FALSE) {
        RETURN_ON_ERROR(bmi_update_composite_network());
    }

    if (bmi_context.current_composite_net == lwk_c_null_object) {
#ifdef MEMEX_DEBUG
        fprintf(stderr,"Composite network is null.\n");
#endif 
        return lwk_s_success;
    }
    status = lwk_query(bmi_context.current_composite_net,
                       lwk_c_domain_surrogate,
                       &Query,
                       (lwk_closure)shelf,
                       callback,
                       &termination
                       );
    PRINT_STATUS("lwk_query()",status)
    return status;
} /* end bmi_query_shelf() */

