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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BRI_MEM.C*/
/* *8     3-MAR-1993 13:58:45 BALLENGER "Fix double freeing of memory."*/
/* *7    13-AUG-1992 15:10:50 GOSSELIN "updating with necessary A/OSF changes"*/
/* *6    19-JUN-1992 20:16:35 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *5     9-JUN-1992 10:02:19 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *4     8-APR-1992 14:49:34 GOSSELIN "added ALPHA fixes"*/
/* *3     3-MAR-1992 17:11:03 KARDON "UCXed"*/
/* *2    13-NOV-1991 14:51:10 GOSSELIN "alpha checkins"*/
/* *1    16-SEP-1991 12:44:20 PARMENTER "BRI Memory management"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BRI_MEM.C*/
/*  DEC/CMS REPLACEMENT HISTORY, Element BRI_MEM.C */
/*  *6     8-APR-1992 14:23:52 GOSSELIN "fixed hyperhelp on ALPHA" */
/*  *5    30-MAR-1992 16:48:38 GOSSELIN "updated with BookreaderPlus EFT code" */
/*  *4    15-NOV-1991 10:26:53 GOSSELIN "checked in common sources for VDM 1.1 BL2" */
/*  *3    30-SEP-1991 14:07:00 FITZELL "Alpha checkins" */
/*  *2    17-SEP-1990 10:02:35 FERGUSON "put RMS back into BRI and perf. fix for topic invocations" */
/*  *1    21-AUG-1990 17:35:37 FERGUSON "pre-ift (BL6) checkins - MEMEX support" */
/*  DEC/CMS REPLACEMENT HISTORY, Element BRI_MEM.C */
/* DEC/CMS REPLACEMENT HISTORY, Element BRI_MEM.C*/
/* *3    25-JAN-1991 16:50:01 FITZELL "V3_EFT_24_JAN"*/
/* *2    12-DEC-1990 11:37:52 FITZELL "V3 ift update snapshot"*/
/* *1     8-NOV-1990 11:26:38 FITZELL "V3 IFT"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BRI_MEM.C*/
#ifndef VMS
 /*
#else
# module BRI_MEM "V03-0002"
#endif
#ifndef VMS
  */
#endif

#ifndef lint
static char *sccsid = "%W%   OZIX    %G%";
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
**
** Facility:
**
**      BRI -- Book Reader Interface
**      BWI -- Book Writer Interface
**
** Abstract:
**
**      This module implements routines shared by the Book Reader Interface
**      (BRI) and the Book Writer Interface (BWI).
**
**      Buffer managment routines are implemented here. BRI and BWI modules
**      should use only these routines.  These are C versions of the original
**      routines written in BLISS.  They use memory management routines
**      (malloc(), realloc(), and BXI_FREE()) from the C run-time library
**      and are portable across all platforms providing these routines.
**
** Functions:
**
**  	LookupBufer
**  	CreateBuffer
**      BriMalloc
**      BriRealloc
**      BriStringAlloc
**      BriDataAlign      - mips, ALPHA, MS Windows only
**      BriFree
**      BriContextInherit
**  	BriContextNew
**      BriContextDelete
**
** Author:
**
**      David L. Ballenger
**
** Date:
**
**      Wed May 17 12:02:19 1989
**
** Revision History:
**
**  V03-0002	JAF0002	    James A. Ferguson	    	7-Sep-1990
**  	      	Change ">" to "<" in BriContextDelete when freeing buffer pool.
**
**  V03-0001    DLB         David L Ballenger           29-Aug-1990
**              Use RMS for file access on VMS.
**
**  V03-0000	JAF0001	    James A. Ferguson	    	16-Aug-1990 
**  	      	Create new module, modify revision history format and
**  	    	update copyright.
**
**          	DLB001 	    David L Ballenger	    	13-Oct-1989 
**  	    	Remove hardcoded error messages.
**
**  	    	DLB 	    David L Ballenger	    	30-May-1990 
**             	Move memory management macros to bri_private_def.h.
**
**  	    	DLB 	    David L Ballenger	    	05-Jul-1990 
**             	Allocate space for file name when creating a new context.
**
*/


/* INCLUDES
 */

#include "bri_private_def.h"


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
 * BriContextInherit is unable to allocate memory for the context for
 * a book or shelf.
 */
static BRI_CONTEXT backup_context;



static
LookupBuffer(context,buffer)
    BRI_CONTEXT *context;
    BMD_GENERIC_PTR buffer;
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

    for ( index = 0 ; index < context->pool.n_slots ; index++) {

        if (context->pool.buffer_slots[index] == buffer) {
            return index;
        }
    }

    return -1 ;

} /* end LookupBuffer */


static
CreateBuffer(context,size)
    BRI_CONTEXT *context;
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
    BMD_GENERIC_PTR new_buffer;

    index = LookupBuffer(context,NULL);

    if ( index < 0 ) {
        
        BMD_GENERIC_PTR *new_slots;
        int new_slot_count;

        if (context->pool.buffer_slots == NULL) {
            new_slot_count = INITIAL_SLOT_ALLOC ;
            new_slots = (BMD_GENERIC_PTR *)
                        BXI_MALLOC(new_slot_count * sizeof(BMD_GENERIC_PTR)) ;
                
        } else {
            new_slot_count = context->pool.n_slots + INCREMENTAL_SLOT_ALLOC ;
            new_slots = (BMD_GENERIC_PTR *)
                        BXI_REALLOC(context->pool.buffer_slots,
                                new_slot_count * sizeof(BMD_GENERIC_PTR)
                                ) ;
        }
        if (new_slots == NULL) {
            BriLongJmp(context,BriErrNoMemoryNum);
        }

        /* Initalize the pointers in all the new slots to NULL
         */ 
        for (index = context->pool.n_slots ; index < new_slot_count ; index++) {
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
    new_buffer = (BMD_GENERIC_PTR)BXI_MALLOC(size);
    if (new_buffer == NULL) {
        BriLongJmp(context,BriErrNoMemoryNum);
    }

    /* The buffer has been allocated, set the appropriate buffer slot
     * to point to it and return the index of that buffer slot.
     */
    memset(new_buffer,0,size);
    context->pool.buffer_slots[index] = new_buffer;
    return index ;
    
} /* end CreateBuffer */


void *
BriMalloc(size,context)
    unsigned int size;
    register BRI_CONTEXT *context;
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

    int index;

    index = CreateBuffer(context,size);

    return (void *)context->pool.buffer_slots[index];

} /* end BriMalloc */


void *
BriRealloc(buffer,new_size,context)
    BMD_GENERIC_PTR buffer;
    unsigned int new_size;
    register BRI_CONTEXT *context;
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
    BMD_GENERIC_PTR new_buffer;

    /* Get the pool dexcriptor and then find the buffer index in the pool.
     */
    buffer_index = LookupBuffer(context,buffer);

    /* Check to see if it is a valid buffer address for the specified
     * buffer pool.
     */
    if ( buffer_index < 0 ) {
        BriLongJmp(context,BriErrInvalidMemNum);
    }

    /* Calculate the new length, then call realloc() to change the
     * size and copy the data if necessary.
     */
    new_buffer = (BMD_GENERIC_PTR)BXI_REALLOC(buffer,new_size);

    /* Check to see if the reallocation was successful.
     */
    if (new_buffer == NULL) {
        BriLongJmp(context,BriErrNoMemoryNum);
    }

    /* Make the buffer slot in the pool point to the new buffer, and
     * return the new buffer address and length.
     */
    context->pool.buffer_slots[buffer_index] = new_buffer ;
    return ((void *)new_buffer) ;

} /* end BriRealloc */


void
BriStringAlloc(dst,src,context)
    register char **dst;
    register char *src;
    register BRI_CONTEXT *context;
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
    unsigned int howbig;
    int     index;

    /* get the size of the string */
    howbig = strlen(src)+1;

    /* allocate the space and get back the index into the memory pool */
    index = CreateBuffer(context,howbig);

    /* using said index get the memory addrees */
    new = (char *)context->pool.buffer_slots[index];

    /* now copy the string into the newly allocated string */
    strcpy(new,src);

    *dst = new;

} /* end BriStringAlloc */

#if defined(mips)  || defined(ALPHA) || defined(MSWINDOWS)

void *
BriDataAlign(addr,size,context)
    register BMD_GENERIC_PTR addr;
    register unsigned int size;
    register BRI_CONTEXT *context;
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
 *      via the BriAlign macro which calls this routine on mips and                     
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
 *      allocation fails the allocation routine will do a BriLongJmp
 *      to signal the error and not return to this routine.
 *
 */
{
    register BMD_GENERIC_PTR new_buffer;
    int index;

    if (size == 0)
	return( addr );

    index = CreateBuffer(context,size);

    new_buffer = context->pool.buffer_slots[index];

    memcpy(new_buffer,addr,size);
    return (void *)new_buffer;

} /* end BriDataAlign */

#endif 


void
BriFree(buffer,context)
    BMD_GENERIC_PTR buffer;
    BRI_CONTEXT *context;
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
        BriLongJmp(context,BriErrInvalidMemNum);
    }

    /* Now free the buffer and indicate in the pool that it is not allocated.
     */
    BXI_FREE(context->pool.buffer_slots[index]);
    context->pool.buffer_slots[index] = NULL ;

} /* end BriFree */


void
BriContextInherit(child_return,parent,entry_id,entry_type,error_return)
    BRI_CONTEXT **child_return;
    BRI_CONTEXT *parent;
    unsigned int entry_id;
    BMD_ENTRY_TYPE entry_type;
    jmp_buf error_return;
/*
 *
 * Function description:
 *
 *      Creates a context for a shelf or book by inheriting the context
 *      from its shelf entry in its parent shelf, and then initializing
 *      the rest of it's own context.  This routine is called by
 *      bri_book_open and BRI_SHELF_OPEN to create the context for the
 *      book or shelf being opened.
 *
 * Arguments:
 *
 *      child_return - Address of a BRI_CONTEXT pointer by which the address
 *                     of the context for the child book / shelf.
 * 
 *      parent       - Address of parent shelf context.
 * 
 *      entry_id     - Id of entry in parent shelf for book / shelf being
 *                     opened
 * 
 *      entry_type   - Type (book / shelf) of entry being opened. This
 *                     is primarily for error checking to make sure that
 *                     the entry specified by entry_id is of the right
 *                     type.
 * 
 *      error_return - A jump buffer initialized by the calling routine
 *                     to be used if an error occurs in lower level BRI
 *                     routines.  If an error occurs these routines will
 *                     "signal" an error by calling BriLongJmp to unwind
 *                     back to the public BRI routine which did the setjmp.
 *
 * Return value:
 *
 *      void
 *
 * Side effects:
 *
 *      Will do a BriLongJmp if an error is detected.
 *
 */

{

    BRI_CONTEXT *child;
    int error = 0;
    
    /* Allocate the child (of the parent shelf) context block for the book or 
     * shelf about to be opened
     */
    child = (BRI_CONTEXT *)BXI_MALLOC(sizeof(BRI_CONTEXT));
    if (child == NULL) {

        /* Couldn't allocate the context block so indicate an error has 
         * occurred, initialize the backup context enough to report the
         * error via BriLongJmp().
         */
        error = BriErrNoMemoryNum ;
        child = &backup_context;
    }
    
    /* Go ahead and return the pointer to child context ot the calling routine
     * (bri_book_open or BRI_SHELF_OPEN).  This is done by assigning through
     * the child_return variable so the context will be available to the
     * calling routine even if it is "unwound" to by BriLongJmp().
     */
    *child_return = child ;

    /* Do common context initialization for the book or shelf.
     *
     *	o Give it the entry information form its shelf entry in the parent
     *    shelf.
     *
     *  o Save the jump buffer set up by the calling routine.
     *
     *  o Initialize file, memory pool, and error reasons to NULL or 0.
     */
    memset(child,0,sizeof(BRI_CONTEXT));
    child->entry = parent->data.shelf->shelf_entries[entry_id - 1];
    (void)memcpy(child->jump_buffer,error_return,sizeof(jmp_buf));
    child->file_open = FALSE ;
    child->pool.n_slots = 0;
    child->pool.buffer_slots = NULL;
    child->reason[0] = '\000' ;
    child->found_file_spec[0] = '\000' ;

    /* Do book or shelf specific initiaization.
     */
    switch (child->entry.entry_type) {
        case BMD_C_BOOK: {

            /* Make sure this is initially NULL
             */
            child->data.book = NULL ;

            /* Nothing else do to if an error has already occurred.
             */
            if (error != 0) {
                break ;
            }

            /* Make sure the calling routine really wanted a book.
             */
            if (entry_type != BMD_C_BOOK) {
                error = BriErrBadBookIdNum ;
                break ;
            }

            /* Allocate the book block.
             */
            child->data.book = (BRI_BOOK_BLOCK *)BXI_MALLOC(sizeof(BRI_BOOK_BLOCK));
            if (child->data.book == NULL) {
                error = BriErrNoMemoryNum ;
            } else {
    	    	memset(child->data.book,0,sizeof(BRI_BOOK_BLOCK));
    	    }
            break ;
        }
        case BMD_C_SHELF: {

            /* Make sure this is initially NULL
             */
            child->data.shelf = NULL ;

            /* Nothing else do to if an error has already occurred.
             */
            if (error != 0) {
                break ;
            }

            /* Make sure the calling routine really wanted a book.
             */
            if (entry_type != BMD_C_SHELF) {
                error = BriErrBadShelfIdNum ;
                break ;
            }

            /* Allocate the shelf block.
             */
            child->data.shelf = (BRI_SHELF_BLOCK *)BXI_MALLOC(sizeof(BRI_SHELF_BLOCK));
            if (child->data.shelf == NULL) {
                error = BriErrNoMemoryNum ;
            } else {
            	memset(child->data.shelf,0,sizeof(BRI_SHELF_BLOCK));
    	    }
            break ;
        }

        case BMD_C_UNKNOWN: {

            /* Make sure this is initially NULL
             */
            child->data.shelf = NULL ;

            /* Nothing else do to if an error has already occurred.
             */
            if (error != 0) {
                break ;
            }

            /* Make sure the calling routine really wanted a book.
             */
            if (entry_type != BMD_C_UNKNOWN) {
                error = BriErrBadShelfIdNum ;
                break ;
            }
            break ;
        }

        default: {
            if (error == 0) {
                error = BriErrBadShelfIdNum ;
            }
            break ;
        }
    }

    /* If an error has occurred then signal the error via BriLongJmp() which
     * will determine the reason, format that part of the message and then
     * do a longjmp() to the caller of this routine.
     */
    if (error != 0) {
        BriLongJmp(child,error);
    }
} /* end BriContextInherit */


void
BriContextNew(new_context,file_name,entry_type,error_return)
    BRI_CONTEXT **new_context;
    char *file_name;
    BMD_ENTRY_TYPE entry_type;
    jmp_buf error_return;
/*
 *
 * Function description:
 *
 *      Sets up a new context for opening a book or shelf by file name
 *      rather than by shelf and entry ids.
 *
 * Arguments:
 *
 *      new_context - Return parameter for the new context.
 *      file_name   - Name of file we're creating a context for.
 *      entry_type  - The type of entry to be opened.
 *      error_return - Caller's jump buffer.
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
    BRI_CONTEXT dummy_context;
    BRI_SHELF_ENTRY_ITEM dummy_vse;
    BRI_SHELF_BLOCK dummy_sfb;

    /* Create a dummy parent context.
     */
    dummy_vse.entry_type = entry_type;
    dummy_vse.target_file = file_name;
    dummy_vse.home_directory = "";
    dummy_vse.title = NULL;
    dummy_sfb.n_entries = 1;
    dummy_sfb.shelf_entries = &dummy_vse;
    dummy_context.data.shelf = &dummy_sfb;

    /* Call BriContextInherit to do the rest of the work of setting
     * up the context.
     */
    BriContextInherit(new_context,&dummy_context,1,entry_type,error_return);
    BriStringAlloc(&(*new_context)->entry.target_file,file_name,*new_context);
} /* end BriContextNew */


void
BriContextDelete(context)
    BRI_CONTEXT *context;

/*
 *
 * Function description:
 *
 *      Deletes the context for a book or shelf that is about to be closed,
 *      or which couldn't be opened.
 *
 * Arguments:
 *
 *      context - Pointer to the context for the book or shelf.
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
    int index ;

    /* Free the book or shelf block.
     */
    switch (context->entry.entry_type) {
        case BMD_C_BOOK: {
            if (context->data.book != NULL) {
                BXI_FREE(context->data.book);
            }
            break ;
        }
        case BMD_C_SHELF: {
            if (context->data.shelf != NULL) {
                BXI_FREE(context->data.shelf);
            }
            break ;
        }
    }
        
    if (context->pool.buffer_slots) 
    {
        /* Free the pool of buffers associated with the context.
         */
        for (index = 0 ; index < context->pool.n_slots; index++ ) 
        {
            if (context->pool.buffer_slots[index]) 
            {
                BXI_FREE(context->pool.buffer_slots[index]);
            }
        }
        /* Free the pool itself.
         */
        BXI_FREE(context->pool.buffer_slots);
        context->pool.n_slots = 0;
    }

    /* Close the file if it is open.
     */
    if (context->file_open) {
#ifdef vms
        sys$disconnect(&context->rab);
        sys$close(&context->fab);
#else
        fclose(context->file);
#endif 
    }
    /* Finally, free the context block itself.
     */
    if (context != &backup_context) {
        BXI_FREE(context);
    }

} /* end BriContextDelete */


