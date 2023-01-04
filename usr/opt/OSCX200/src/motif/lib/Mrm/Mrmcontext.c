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
/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: Mrmcontext.c,v $ $Revision: 1.1.6.3 $ $Date: 1993/07/15 18:06:34 $"
#endif
#endif

/*
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */

/*
 *++
 *  FACILITY:
 *
 *      UIL Resource Manager (URM):
 *	
 *	URM context routines
 *
 *  ABSTRACT:
 *
 *	These routines create, modify, and delete a URM context structure.
 *	Note that accessors for a URM context are macros.
 *
 *--
 */


/*
 *
 *  INCLUDE FILES
 *
 */


#include <Mrm/MrmAppl.h>
#include <Mrm/Mrm.h>


/*
 *
 *  TABLE OF CONTENTS
 *
 *	UrmGetResourceContext		- Allocate a new resource context
 *
 *	UrmResizeResourceContext	- Resize a resource context
 *
 *	UrmFreeResourceContext	- Free a resource context
 *
 */


/*
 *
 *  DEFINE and MACRO DEFINITIONS
 *
 */

/*
 *
 *  EXTERNAL VARIABLE DECLARATIONS
 *
 */


/*
 *
 *  GLOBAL VARIABLE DECLARATIONS
 *
 */


/*
 *
 *  OWN VARIABLE DECLARATIONS
 *
 */




Cardinal UrmGetResourceContext
#ifndef _NO_PROTO
    (char			*((*alloc_func) ()),
    void			(*free_func) (),
    MrmSize			size,
    URMResourceContextPtr	*context_id_return)
#else
        (alloc_func, free_func, size, context_id_return)
    char			*((*alloc_func) ()) ;
    void			(*free_func) () ;
    MrmSize			size ;
    URMResourceContextPtr	*context_id_return ;
#endif

/*
 *++
 *
 *  PROCEDURE DESCRIPTION:
 *
 *	UrmGetResourceContext allocates a new resource context, then
 *	allocates a memory buffer of the requested size and associates it
 *	with the context.
 *
 *  FORMAL PARAMETERS:
 *
 *	alloc_func		Function to use in allocating memory for this
 *				context. A null pointer means use default
 *				(XtMalloc).
 *	free_func		Function to use in freeing memory for this
 * 				context. A null pointer means use default
 *				(XtFree).
 *	size			Size of memory buffer to allocate.
 *	context_id_return	To return new context.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  FUNCTION VALUE:
 *
 *	MrmSUCCESS	operation succeeded
 *	MrmFAILURE	allocation failure
 *
 *  SIDE EFFECTS:
 *
 *--
 */

{

/*
 *  External Functions
 */

/*
 *  Local variables
 */


/*
 * Set function defaults if NULL
 */
if ( alloc_func == NULL ) alloc_func = XtMalloc ;
if ( free_func == NULL ) free_func = XtFree ;

/*
 * Allocate the context buffer and memory buffer, and set the
 * context up.
 */
*context_id_return =
    (URMResourceContextPtr) (*alloc_func) (sizeof(URMResourceContext)) ;
if ( *context_id_return == NULL )
    return Urm__UT_Error
        ("UrmGetResourceContext", "Context allocation failed",
        NULL, *context_id_return, MrmFAILURE) ;

(*context_id_return)->validation = URMResourceContextValid ;
(*context_id_return)->data_buffer = NULL ;

if ( size > 0 )
    {
    (*context_id_return)->data_buffer = (char *) (*alloc_func) (size) ;
    if ( (*context_id_return)->data_buffer == NULL )
        {
        (*free_func) (*context_id_return) ;
        return Urm__UT_Error
            ("UrmGetResourceContext", "Buffer allocation failed",
            NULL, *context_id_return, MrmFAILURE) ;
        }
    }

(*context_id_return)->buffer_size = size ;
(*context_id_return)->resource_size = 0 ;
(*context_id_return)->group = URMgNul ;
(*context_id_return)->type = URMtNul ;
(*context_id_return)->access = 0 ;
(*context_id_return)->lock = 0 ;
(*context_id_return)->alloc_func = alloc_func ;
(*context_id_return)->free_func = free_func ;

/*
 * Context successfully created
 */
return MrmSUCCESS ;


}



Cardinal UrmResizeResourceContext (context_id, size)
    URMResourceContextPtr	context_id ;
    int				size ;

/*
 *++
 *
 *  PROCEDURE DESCRIPTION:
 *
 *	UrmResizeResourceContext reallocates the memory buffer in a
 *	resource context to increase its size. The contents of the current
 *	memory buffer are copied into the new memory buffer. If the size
 *	parameter is smaller than the current memory buffer size, the request
 *	is ignored. If the size parameter is greater than MrmMaxResourceSize, a
 *	MrmTooMany error is returned
 *
 *  FORMAL PARAMETERS:
 *
 *	context_id	Resource context to receive resized memory buffer
 *	size		New size for memory buffer in resource context
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  FUNCTION VALUE:
 *
 *	MrmSUCCESS	operation succeeded
 *	MrmBAD_CONTEXT	Illegal resource context
 *	MrmFAILURE	memory allocation failure
 *	MrmTOO_MANY	size was larger than MrmMaxResourceSize
 *
 *  SIDE EFFECTS:
 *
 *--
 */

{

/*
 *  External Functions
 */

/*
 *  Local variables
 */
char			*newbuf ;	/* new buffer */


if ( ! UrmRCValid(context_id) )

    return Urm__UT_Error
        ("UrmResizeResourceContext", "Validation failed",
        NULL, context_id, MrmBAD_CONTEXT) ;

if ( size > MrmMaxResourceSize)
    return Urm__UT_Error
        ("MrmResizeResourceContext", "Resource size too large",
        NULL, context_id, MrmTOO_MANY) ;

/*
 * Resize unless buffer is bigger than requested size.
 */
if ( context_id->buffer_size > size ) return MrmSUCCESS ;

/*
 * Allocate the new buffer, copy the old buffer contents, and
 * update the context.
 */
if ( context_id->alloc_func == XtMalloc )
    {
    context_id->data_buffer = XtRealloc (context_id->data_buffer, size) ;
    context_id->buffer_size = size ;
    }
else
    {
    newbuf = (char *) (*(context_id->alloc_func)) (size) ;
    if ( newbuf == NULL )
        return Urm__UT_Error
            ("UrmResizeResourceContext", "Buffer allocation failed",
            NULL, context_id, MrmFAILURE) ;
    if ( context_id->data_buffer != NULL )
        {
	UrmBCopy (context_id->data_buffer, newbuf, context_id->buffer_size) ;
        (*(context_id->free_func)) (context_id->data_buffer) ;
        }
    context_id->data_buffer = newbuf ;
    context_id->buffer_size = size ;
    }

/*
 * Resize succeeded
 */
return MrmSUCCESS ;

}



Cardinal UrmFreeResourceContext (context_id)
    URMResourceContextPtr	context_id ;

/*
 *++
 *
 *  PROCEDURE DESCRIPTION:
 *
 *	UrmFreeResourceContext frees the memory buffer and the
 *	resource context.
 *
 *  FORMAL PARAMETERS:
 *
 *	context_id	Resource context to be freed
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  FUNCTION VALUE:
 *
 *	MrmSUCCESS	operation succeeded
 *	MrmBAD_CONTEXT	Illegal resource context
 *
 *  SIDE EFFECTS:
 *
 *--
 */

{

/*
 *  External Functions
 */

/*
 *  Local variables
 */


if ( ! UrmRCValid(context_id) )
    return Urm__UT_Error
        ("UrmFreeResourceContext", "Validation failed",
        NULL, context_id, MrmBAD_CONTEXT) ;

context_id->validation = NULL ;
if ( context_id->data_buffer != NULL )
    (*(context_id->free_func)) (context_id->data_buffer) ;
(*(context_id->free_func)) (context_id) ;
return MrmSUCCESS ;

}
