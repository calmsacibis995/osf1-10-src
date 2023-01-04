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
/* DEC/CMS REPLACEMENT HISTORY, Element VRI-MEM.C*/
/* *2     9-JUL-1990 15:50:44 FITZELL ""*/
/* *1    24-APR-1990 15:44:01 FITZELL "creating initial elements that shipped with V2 VWI"*/
/* DEC/CMS REPLACEMENT HISTORY, Element VRI-MEM.C*/
#ifndef lint
static char *sccsid = "%W%   OZIX    %G%";
#endif
/****************************************************************************
 *                                                                          *
 *  (C) DIGITAL EQUIPMENT CORPORATION 1988, 1989                            *
 *                                                                          *
 *      This is an unpublished work which was created in the indicated      *
 *      year, which contains confidential and secret information,  and      *
 *      which is protected under the copyright laws.  The existence of      *
 *      the copyright notice is not to be construed as an admission or      *
 *      presumption that publication has occurred. Reverse engineering      *
 *      and unauthorized copying is strictly prohibited.   All  rights      *
 *      reserved.                                                           *
 *                                                                          *
 ****************************************************************************
 *
 *
 * Facility:
 *
 *      VRI -- VOILA Reader Interface
 *      
VWI -- VOILA Writer Interface
 *
 * Abstract:
 *
 *      This module implements routines shared by the VOILA Reader Interface
 *      (VRI) and the VOILA Writer Interface (VWI).
 *
 *      Buffer managment routines are implemented here. VRI and VWI modules
 *      should use only these routines.  These are C versions of the original
 *      routines written in BLISS.  They use memory management routines
 *      (malloc(), realloc(), and free()) from the C run-time library
 *      and are portable across all platforms providing these routines.
 *
 * Functions:
 *
 *      VriMalloc
 *      VriRealloc
 *      VriStringAlloc
 *      VriDataAlign      - mips only
 *      VriFree
 *
 * Author:
 *
 *      David L. Ballenger
 *
 * Date:
 *
 *      Wed May 17 12:02:19 1989
 *
 * Revision History:
 *
 *      DLB001 13-Oct-1989 Remove hardcoded error messages.
 */


/* INCLUDES
 */

#include "bxi_ods.h"


/* DEFINES
 */

/* Pool allocation constants
 */
#define INITIAL_SLOT_ALLOC 100       /* Number of buffer slots to allocate
                                     * initially.
                                     */
#define INCREMENTAL_SLOT_ALLOC 100   /* Number of slots for incremental
                                     * allocations.
                                     */ 


/* Local Variables
 */

/* This provides a backup context to use for reporting errors if
 * VriContextInherit is unable to allocate memory for the context for
 * a book or shelf.
 */
static VriContext backup_context;



static LookupBuffer(context,buffer)
    VriContext *context;
    VriGenericPtr buffer;
/*
 *
 * Function description:
 *
 *      Find the buffer with the specified address in the specified pool
 *      and return the slot number of the buffer.
 *
 * Arguments:
 *
 *      pool        Address of the specified buffer pool.
 *
 *      buffer      Address of the buffer we are trying to match.  Can be
 *                  specifed as NULL in which case it returns the first free
 *                  slot, if any.
 *
 * Return value:
 *
 *      Slot number or -1 if buffer is not found.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    register int index ;
    VriGenericPtr *ptr;

    for ( index = 0 ; index < context->pool.n_slots ; index++) {

/*	ptr = context->pool.buffer_slots;
	ptr += index;
	if(*ptr == buffer)
	    return index; */

        if (context->pool.buffer_slots[index] == buffer) {
            return index; 
        }
    }

    return -1 ;

} /* end LookupBuffer */


static
CreateBuffer(context,size)
    VriContext *context;
    unsigned int    size;
/*
 *
 * Function description:
 *
 *     Allocate a buffer in the first available buffer slot in the 
 *     specifed pool, and return the index of the buffer slot.
 *
 * Arguments:
 *
 *      pool  Address of the pool descriptor.
 * 
 *      size  Size of the buffer to be allocated.
 *
 * Return value:
 *
 *      The index of the buffer slot for the allocated buffer or -1 if
 *      no buffer or buffer slot could be allocated.
 *
 * Side effects:
 *
 */

{
    int index ;
    VriGenericPtr new_buffer;

    index = LookupBuffer(context,NULL);

    if ( index < 0 ) {
        
        VriGenericPtr *new_slots;
        int new_slot_count;

        if (context->pool.buffer_slots == NULL) {
            new_slot_count = INITIAL_SLOT_ALLOC ;
            new_slots = (VriGenericPtr *)
                        malloc(new_slot_count * sizeof(VriGenericPtr )) ;
                
        } else {
            new_slot_count = context->pool.n_slots + INCREMENTAL_SLOT_ALLOC ;
            new_slots = (VriGenericPtr *)
                        realloc(context->pool.buffer_slots,
                                new_slot_count * sizeof(VriGenericPtr )
                                ) ;
        }
        if (new_slots == NULL) 
            longjmp(context->jump_buffer,BwiErrNoMemoryNum); 

        /* Initalize the pointers in all the new slots to NULL
         */ 
        for (index = context->pool.n_slots ; index < new_slot_count ; index ++) {
            new_slots[index] = NULL;
        }

        /* Set the index for the new buffer to the old number of
         * slots, i.e. the first new slot.  Then update the pool
         * information to reflect the new slots.
         */
        index = context->pool.n_slots ;
        context->pool.n_slots = new_slot_count ;
        context->pool.buffer_slots = new_slots ;
    }

    /* Now actually try to allocate the new buffer.
     */
    new_buffer = (VriGenericPtr)malloc(size);
    if (new_buffer == NULL) 
        longjmp(context->jump_buffer,BwiErrNoMemoryNum);  

    /* The buffer has been allocated, set the appropriate buffer slot
     * to point to it and return the index of that buffer slot.
     */
    context->pool.buffer_slots[index] = new_buffer;
    return index;
    
} /* end CreateBuffer */


void *
VriMalloc(size,context)
    unsigned long int size;
    register VriContext *context;
/*
 *
 * Function description:
 *
 *      Allocates a new buffer in the specified pool.
 *
 * Arguments:
 *
 *      size     Size of the buffer to be allocated.
 * 
 *      pool_id  Id of pool in which to allocate the buffer.
 *
 * Return value:
 *
 *      The address of the buffer as a generic pointer.
 *
 * Side effects:
 *
 *      None
 *
 */

{

    /* As simple as 1, 2, 3 ...
     *
     *   1. Get the pool descriptor for the pool.
     *
     *   2. Allocate the buffer in the pool.
     *
     *   3. Return the address of the buffer.
     */
    return (void *)context->pool.buffer_slots[CreateBuffer(context,size)];

} /* end VriMalloc */


void *
VriRealloc(buffer,new_size,old_size,context)
    VriGenericPtr buffer;
    unsigned int new_size;
    unsigned int old_size;
    register VriContext *context;
/*
 *
 * Function description:
 *
 *      Reallocates a buffer, increasing the size and copying data if
 *      necessary.
 *
 * Arguments:
 *
 *      buffer    Address of the buffer to be reallocated.
  * 
 *      new_size  New size of the buffer.
 * 
 *      pool_id   Pool ID for the pool containing the buffer.
 *
 * Return value:
 *
 *      The address of the "realloc"d buffer or Null if an error occured.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    int buffer_index;
    VriGenericPtr new_buffer;

    /* Get the pool dexcriptor and then find the buffer index in the pool.
     */
    buffer_index = LookupBuffer(context,buffer);

    /* Check to see if it is a valid buffer address for the specified
     * buffer pool.
     */
    if ( buffer_index < 0 ) 
	longjmp(context->jump_buffer,BwiErrInvalidMemNum);

    /* Calculate the new length, then call realloc() to change the
     * size and copy the data if necessary.
     */

    new_buffer = (VriGenericPtr)realloc(buffer,new_size); 

    /* Check to see if the reallocation was successful.
     */
    if (new_buffer == NULL) 
        longjmp(context->jump_buffer,BwiErrNoMemoryNum);

    /* Make the buffer slot in the pool point to the new buffer, and
     * return the new buffer address and length.
     */
    context->pool.buffer_slots[buffer_index] = new_buffer ;
    return ((void *)new_buffer) ;

} /* end VriRealloc */


void
VriStringAlloc(dst,src,context)
    register char **dst;
    register char *src;
    register VriContext *context;
/*
 *
 * Function description:
 *
 *      Alocates buffer space for the designated string and copies the
 *      string into it.
 *
 * Arguments:
 *
 *      dst - Address of the pointer to receive the new string
 *
 *      src - String for which to allocate space or null if this is
 *                    new string and nothing needs to be copied.
 *
 *      context -  Book or bookshelf context.
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      None
 *
 */

{
    char *new ;
    
    new = context->pool.buffer_slots[CreateBuffer(context,strlen(src)+1)];
    strcpy(new,src);
    *dst = new;

} /* end VriStringAlloc */

#ifdef mips

void *
VriDataAlign(addr,size,context)
    register VriGenericPtr addr;
    register unsigned int size;
    register VriContext *context;
/*
 *
 * Function description:
 *
 *      A routine for providing properly aligned data on the mips
 *      platforms.  The routine:
 *
 *          1. provides a buffer of the requested size
 *          2. copies the data specifed by addr argument
 *          3. returns a pointer to the new buffer
 *
 *      The old buffer at addr is not freed since more than likely the
 *      data being aligned is part of a larger data structure.
 *
 *      The routine is only defined for the mips and should be called
 *      via the VriAlign macro which calls this routine on mips and                     
 *      simply resolves to the specifed address on the vax.
 *
 * Arguments:
 *
 *      addr - Address of data to be aligned
 *
 *      size - Size of data to be aligned
 *
 *      context - Book or shelf enironment for the data
 *
 * Return value:
 *
 *      Pointer to the new buffer
 *
 * Side effects:
 *
 *      New buffer allocated in the context's pool.  If the
 *      allocation fails the allocation routine will do a VriLongJmp
 *      to signal the error and not return to this routine.
 *
 */
{
    register VriGenericPtr new_buffer;

    new_buffer = context->pool.buffer_slots[CreateBuffer(context,size)];
    memcpy(new_buffer,addr,size);
    return (void *)new_buffer;

} /* end VriDataAlign */

#endif 


void
VriFree(buffer,context)
    VriGenericPtr buffer;
    VriContext *context;
/*
 *
 * Function description:
 *
 *      Frees the specified buffer in the specified pool.
 *
 * Arguments:
 *
 *      buffer   Address of the buffer
 *      pool_id  Pool ID for the buffer
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      None
 *
 */

{

    register int index ;

    /* Get the index for the buffer in the pool.
     */
    index = LookupBuffer(context,buffer);

    /* Check to make sure we have a valid index
     */
    if ( index < 0 ) {
        longjmp(context,BwiErrInvalidMemNum);
    }

    /* Now free the buffer and indicate in the pool that it is not allocated.
     */
    free(context->pool.buffer_slots[index]);
    context->pool.buffer_slots[index] = NULL ;

} /* end VriFree */


void *
VwiMalloc(size,context,status)
    unsigned long int size;
    VriContext *context;
    unsigned long int *status;
/*
 *
 * Function description:
 *
 *      Jacket routine to catch longjumps.
 *
 * Arguments:
 *
 *      size     Size of the buffer to be allocated.
 * 
 *      status   addres of wher to put the error code if needed
 *
 * Return value:
 *
 *      The address of the buffer as a generic pointer.
 *
 * Side effects:
 *
 *      None
 *
 */
{

    /* set up jump buffer in case of errors */
    if((*status = setjmp(context->jump_buffer)) != 0) 	
        return(NULL);

    return(VriMalloc(size,context));
}

void *
VwiRealloc(buffer,new_size,old_size,context,status)
    VriGenericPtr buffer;
    unsigned int new_size;
    unsigned int old_size;
    register VriContext *context;
    unsigned long int *status;
{
    /* set up jump buffer in case of errors */
    if((*status = setjmp(context->jump_buffer)) != 0)
        return(NULL);

    return(VriRealloc(buffer,new_size,old_size,context));
}
