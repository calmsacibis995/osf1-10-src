/* dwc_db_virtual_buffers.c */
#ifndef lint
static char rcsid[] = "$Id$";
#endif /* lint */
/*
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**++
**  FACILITY:
**
**	DECwindows Calendar; database access routines
**
**  AUTHOR:
**
**	Per Hamnqvist, November 1987
**
**  ABSTRACT:
**
**	This module implements the virtual buffer management used by
**	the database access routines. This module implements the first
**	layer above the physical file operations (in DWC$DB_PHYSICAL_RECORDS).
**
**--
*/

/*
**  Include Files
*/
#include "dwc_compat.h"

#include <stdio.h>
#include <string.h>

#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"


void
DWC$$DB_Delete_all_buffers
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab)
#else	/* no prototypes */
	(Cab)
	struct DWC$db_access_block *Cab;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine deletes all buffers on the free list. The virtual
**	memory is returned to the VAX C RTL. If any deallocation would fail the
**	routine will signal a fatal error.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    struct DWC$db_vm_buffer *Wrkbuf;	    /* Next buffer to be deleted    */
    
    /*
    **  Remove entries from the free list until the queue is empty. Did we
    **	get one?
    */
    while
    (
	DWC$$DB_Remque
	(
	    (struct DWC$db_queue_head *) &(Cab->DWC$a_dcab_free_flink),
	    (struct DWC$db_queue_head **) &Wrkbuf
	)
    )
	{
	/*
	**  Yes, we got a buffer. Deallocate the memory. Signal an error if this
	**  failed.
	*/
	XtFree (Wrkbuf);
	}

}

void
DWC$$DB_Cache_buffer
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	struct DWC$db_vm_record *Buffer)
#else	/* no prototypes */
	(Cab, Buffer)
	struct DWC$db_access_block *Cab;
	struct DWC$db_vm_record *Buffer;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine places a buffer on the LRU cache. The buffer is inserted at
**	the head of the queue. A buffer should not be put on this list unless it
**	has been completely initialized (e.g., make the call to this routine one
**	of the last steps).
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	Buffer : Pointer to VM buffer to add to cache
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    /*
    **  Insert the buffer into the head of the least recently used cache
    */
    DWC$$DB_Insque
    (
	(struct DWC$db_queue_head *) &(Buffer->DWC$a_dbvm_lru_flink),
	(struct DWC$db_queue_head *) &(Cab->DWC$a_dcab_lru_flink)
    );

    /*
    **  Increment count of buffers in cache
    */
    Cab->DWC$l_dcab_cache_count++;
    
}

void
DWC$$DB_Touch_buffer
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	struct DWC$db_vm_record *Buffer)
#else	/* no prototypes */
	(Cab, Buffer)
	struct DWC$db_access_block *Cab;
	struct DWC$db_vm_record *Buffer;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine marks that a cached buffer has been touched. The routine
**	will move this buffer to the head of the cache queue. The buffer must
**	have been previously cache before you can call this routine.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	Buffer : Pointer to VM buffer to move
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    /*
    **  Remove the buffer from the queue, where it is currently located
    */
    DWC$$DB_Remque
    (
	(struct DWC$db_queue_head *) Buffer->DWC$a_dbvm_lru_blink,
	(struct DWC$db_queue_head **) 0
    );

    /*
    **  Reinsert the entry at the head of the queue
    */
    DWC$$DB_Insque
    (
	(struct DWC$db_queue_head *) &(Buffer->DWC$a_dbvm_lru_flink),
	(struct DWC$db_queue_head *) &(Cab->DWC$a_dcab_lru_flink)
    );
    
}

void
DWC$$DB_Freelist_buffer
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	struct DWC$db_vm_record *Buffer)
#else	/* no prototypes */
	(Cab, Buffer)
	struct DWC$db_access_block *Cab;
	struct DWC$db_vm_record *Buffer;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine frees a buffer. If the buffer is in the cache, it will be
**	removed from the cache. If the buffer is of the fixed length used
**	for virtual records, the buffer will simply put on what is called the
**	free list. If, however, the buffer is not of that size then the virtual
**	memory held by the buffer is deallocated. If the deallocation of the
**	virtual memory fails, this routine will signal a fatal error.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	Buffer : Pointer to VM buffer to free
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    /*
    **  Is this buffer a fixed length record buffer?
    */
    if (Buffer->DWC$l_dbvm_rec_len != DWC_vm_record_len)

	/*
	**  No. This is a variable length work buffer of some sort. Such a
	**  buffer cannot be put on the free list, since the free list consists
	**  of only fixed length record buffers. Release the virtual memory held
	**  by this buffer. Signal an error if it failed (fatal).
	*/
	{
	XtFree (Buffer);
	}

    else
	{
	/*
	**  Yes, this buffer is a fixed length record buffer. Remove it from
	**  cache, if in cache
	*/
	if (Buffer->DWC$a_dbvm_lru_flink != 0)
	    {
	    DWC$$DB_Remque
	    (
		(struct DWC$db_queue_head *) Buffer->DWC$a_dbvm_lru_blink,
		(struct DWC$db_queue_head **) 0
	    );
	    Cab->DWC$l_dcab_cache_count--;
	    }

	/*
	**  Put it on the free list
	*/
	DWC$$DB_Insque
	(
	    (struct DWC$db_queue_head *) Buffer,
	    (struct DWC$db_queue_head *) &(Cab->DWC$a_dcab_free_flink)
	);
	}
}	

void
DWC$$DB_Insque
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_queue_head *Entry,
	struct DWC$db_queue_head *Pred)
#else	/* no prototypes */
	(Entry,Pred)
	struct DWC$db_queue_head *Entry;
	struct DWC$db_queue_head *Pred;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Insert an entry into a doubly linked list. The routine is very
**	similar to the VAX hardware instruction INSQUE.
**
**  FORMAL PARAMETERS:
**
**	Entry : Pointer to entry to be inserted into queue. The first two
**		longwords of the entry is assumed to be reserved for this
**		insert queue operation.
**	Pred :	Pointer to the predecessor after which the entry is to be
**		inserted. If you want to insert entry at head of queue then
**		this should point at the head of the queue
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    /*
    **  Insert entry into queue
    */
    
    Entry->flink = Pred->flink;	    /* Pickup Flink from Predecessor	    */
    Entry->blink = Pred;	    /* Make blink the Predecessor itself    */
    Pred->flink = Entry;	    /* Let Flink to Pred point at new entry */
    Entry->flink->blink = Entry;    /* Patch backlink of next		    */

}    

int
DWC$$DB_Remque
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_queue_head *Head,
	struct DWC$db_queue_head **Entry)
#else	/* no prototypes */
	(Head,Entry)
	struct DWC$db_queue_head *Head;
	struct DWC$db_queue_head **Entry;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine removes an entry from a doubly linked list. The routine
**	is very similar to the VAX hardware REMQUE instruction, with the
**	exception that this routine zero:es the flink of the entry removed.
**
**  FORMAL PARAMETERS:
**
**	Head : Pointer to Queue head from which the entry is to be removed
**	Entry : Pointer to callers field which is to receive the pointer
**		to the entry that was removed. If this parameter is specified
**		as zero, then nothing will be returned.
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION CODES:
**
**	TRUE  -- Entry removed
**      FALSE -- Nothing removed
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    struct DWC$db_queue_head *Next; /* Temp pointer to entry following Head */
    struct DWC$db_queue_head *E;    /* Temp entry pointer		    */

    /*
    **  Is there anything in the queue?
    */
    if (Head->flink != Head)

	/*
	**  Yes, remove first entry from Queue head
	*/
	{
	E = Head->flink;	/* Let the entry be the first entry	    */
	Next = E->flink;	/* Point at the second entry		    */
	Head->flink = Next;	/* Let the Head point at the second entry   */
	Next->blink = Head;	/* Let the Second entry backlink to head    */
	E->flink = 0;		/* Fry the flink to indicate out of queue   */
	if (Entry != 0) *Entry = E; /* Return Entry, if requested	    */
	return (TRUE)	;	/* Indicate that something was removed	    */
	}

    else
	/*
	**  Nothing there.
	*/
	return (FALSE);

}

int
DWC$$DB_Get_free_buffer
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	int Reqlen,
	struct DWC$db_vm_record **Retbuf)
#else	/* no prototypes */
	(Cab, Reqlen, Retbuf)
	struct DWC$db_access_block *Cab;
	int Reqlen;
	struct DWC$db_vm_record **Retbuf;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine allocates a new virtual buffer. If the requested buffer
**	is for a record (indicated by the size) then an attempt is made to
**	get the buffer from the free list. If not or if the request fails the
**	buffer is allocated from memory.
**
**	A virtual buffer header is prepended in front of the requested buffer.
**	If obtaining the buffer was successful, the header of the buffer is
**	initialized. Please note that the user part of the buffer is not
**	initialized.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	Reqlen : Length of requested buffer, exluding the size of the VM header
**	Retbuf : Pointer to field that will receive a pointer to the allocated
**		 buffer. The pointer will point at the VM header. The actual
**		 user buffer is located at offset DWC$K_VM_HEADER.
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION CODES:
**
**	0 -- Buffer allocated
**     -1 -- Failed to allocate buffer
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    struct DWC$db_vm_record *VM_buffer = 0; /* Temp work buffer		*/
    char *Work_buffer;			    /* address in middle of buf	*/

    /*
    **  Try to reuse a buffer. This is only possible if the buffer being
    **	requested is of the fixed size kept on the free list. If this is true,
    **	try to remove a buffer from the free list.
    **
    **  Got anything?
    */
    if (Reqlen == DWC$k_db_record_length)
	{
	if
	(
	    DWC$$DB_Remque
	    (
		(struct DWC$db_queue_head *) &(Cab->DWC$a_dcab_free_flink),
		(struct DWC$db_queue_head **) &Work_buffer
	    )
	)
	    {
	    VM_buffer = (VM_record *)Work_buffer;
	    }
	else
	    {
	    /*
	    **  No. Is the cache full?
	    */
	    if (Cab->DWC$l_dcab_cache_count > DWC$k_dbvm_max_cache)

		/*
		**  Yes. Release some buffers. If this failed, we're in trouble.
		**	If it worked, which it should, just remove the first buffer on
		**	the free list. No need to test for completion status since there
		**	MUST be something on the queue.
		*/
		{
		if (!DWC$$DB_Purge_cache(Cab))
		    {
		    _Signal(DWC$_PURGEFAIL);
		    return(_Failure);
		    }
		DWC$$DB_Remque
		(
		    (struct DWC$db_queue_head *) &(Cab->DWC$a_dcab_free_flink),
		    (struct DWC$db_queue_head **) &Work_buffer
		);
		VM_buffer = (VM_record *)Work_buffer;
		}

	    else

		/*
		**  The cache is not full. Try and get a new buffer from
		**	the RTL. If this fails we shall try and remove something
		**	from the cache and use that. If, however, we're both unable to
		**	allocate virtual memory and there is nothing that can be removed
		**	from the cache we shall return failure.
		*/
		{
		if ((VM_buffer = (VM_record *)XtMalloc (Reqlen + DWC$K_VM_HEADER)) == 0)
		    {
		    _Record_error;
		    if (!DWC$$DB_Purge_cache(Cab))
			{
			_Signal(DWC$_PURGEFAIL);
			return(_Failure);
			}
		    DWC$$DB_Remque
		    (
			(struct DWC$db_queue_head *)
			    &(Cab->DWC$a_dcab_free_flink),
			(struct DWC$db_queue_head **) &Work_buffer
		    );
		    VM_buffer = (VM_record *)Work_buffer;
		    }
		}
		
	    }
	}
    /*
    **  Now we may or not have a buffer. If not, try and get one from the RTL.
    **	Do we have a buffer?
    */
    if (VM_buffer == 0)
	{
	/*
	**  No buffer yet. We must allocate some fresh memory. Save the
	**  pointer. If this fails, return to caller indicating that the
	**  allocation failed
        */
	if ((VM_buffer = (VM_record *)XtMalloc (Reqlen + DWC$K_VM_HEADER)) == 0)
	    {
	    _Record_error;
	    _Signal(DWC$_INSVIRMEM);
	    }
	}
    /*
    **  We have a buffer. Initialize the VM header part of the buffer. This
    **	routine does not initialize the rest of the buffer
    */
    memset(VM_buffer, 0, DWC$K_VM_HEADER);
    VM_buffer->DWC$l_dbvm_rec_len = Reqlen + DWC$K_VM_HEADER;

    /*
    **  Pass the buffer pointer back to caller and indicate that we got it
    */
    *Retbuf = VM_buffer;
    return (_Success);

}

int
DWC$$DB_Get_free_buffer_z
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	int Reqlen,
	struct DWC$db_vm_record **Retbuf)
#else	/* no prototypes */
	(Cab, Reqlen, Retbuf)
	struct DWC$db_access_block *Cab;
	int Reqlen;
	struct DWC$db_vm_record **Retbuf;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine is almost identical to DWC$$DB_Get_buffer. The only
**	difference is that this routine also initializes the user part of
**	the allocated buffer
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	Reqlen : Length of requested buffer, exluding the size of the VM header
**	Retbuf : Pointer to field that will receive a pointer to the allocated
**		 buffer. The pointer will point at the VM header. The actual
**		 user buffer is located at offset DWC$K_VM_HEADER.
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION CODES:
**
**	0 -- Buffer allocated
**     -1 -- Failed to allocate buffer
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    int getsts;			/* Status from buffer allocation	    */
    
    /*
    **  Allocate a buffer and save the status.
    */
    getsts = DWC$$DB_Get_free_buffer(Cab, Reqlen, Retbuf);

    /*
    **	Initialize the user specific part of the buffer. The header was already
    **	initialized by DWC$$DB_Get_free_buffer
    */
    {
    VM_record *Zbuf = *Retbuf;
    memset(&Zbuf->DWC$t_dbvm_data[0], 0, Reqlen);
    }

    /*
    **  Return back to caller with whatever status we got back from the
    **	allocation routine.
    */
    return (getsts);
	
}

int
DWC$$DB_Purge_cache
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block    *Cab)
#else	/* no prototypes */
	(Cab)
	struct DWC$db_access_block    *Cab;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine is invoked when the number of cached buffers has reached
**	a max limit or if XtMalloc failed to obtain a fixed size buffer (in
**	which case we can attempt to reuse another buffer).
**
**	The purging is done by removing the last entry off the cache and
**	then releasing that buffer (e.g., removing it from the virtual copy of
**	the database). Some more complex record types have specific release
**	routines that fork down and release lower structures.
**
**	The purge may release nothing or a lot. A lot means that the purge
**	could release a major chunk when attempting to rundown a rangemap, for
**	example.
**
**	Please note that this routine pays special attention to repeat
**	expression blocks. If the repeat expression block has outstanding
**	repeats in any day already in the cache, it will place that repeat
**	expression block in the front of the cache and try with the next block.
**	
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION CODES:
**
**	1 -- Nothing more to purge
**	0 -- Something purged
**     -1 -- Failed to purge
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    VM_record	*Work_buffer;
    if (Cab->DWC$a_dcab_lru_flink == (VM_record *)&Cab->DWC$a_dcab_lru_flink)
	{
	return (FALSE);
	}
    else
	{
	while (TRUE)
	    {
	    Work_buffer = (VM_record *)(((char *)(Cab->DWC$a_dcab_lru_blink))
				- DWC$K_VM_LRU_OFFSET);
	    if ((Work_buffer->DWC$t_dbvm_data[0] != DWC$k_db_repeat_expr_block) ||
		(Work_buffer->DWC$a_dbvm_special == 0))
		{
		break;
		}
	    DWC$$DB_Touch_buffer(Cab, Work_buffer);
	    }
	return (DWC$$DB_Release_buffer(Cab, Work_buffer));
	}
}

int
DWC$$DB_Release_buffer
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	VM_record *Input_buffer_VM)
#else	/* no prototypes */
	(Cab, Input_buffer_VM)
	struct DWC$db_access_block	*Cab;
	VM_record *Input_buffer_VM;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine frees the input buffer with DWC$$DB_Freelist_buffer. But,
**	before the buffer is placed on the freelist, any parent buffers are
**	updated so that _Follow will force the specified buffer to be brought
**	back into memory again.
**
**	The routine first does a short generic setup, then it executes type
**	specific cleanup. For example, before a rangemap can be releaseed, all
**	its subrangemaps must be released. Thus, the block being removed must be
**	properly removed from hierarchy. Finally, the routine returns the buffer
**	back to the VM management routines.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	Input_buffer_VM : buffer to release
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION CODES:
**
**	TRUE	- Buffer released
**	FALSE	- Failed to release buffer
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    VM_record	*Buffer_VM;
    char    *Buffer_data;
    int i;

    Buffer_VM   = Make_VM_address((unsigned long)Input_buffer_VM);
    Buffer_data = (char *)&(Buffer_VM->DWC$t_dbvm_data[0]);
    /*
    **  First release any buffer this buffer may point to.
    */
    switch (Buffer_data[0])
	{
	case DWC$k_db_header :
		{
	        struct DWCDB_header *Head;	/* Pointer to header block */
	        Head = (struct DWCDB_header *)Buffer_data;
		if (Is_VM_pointer(Head->DWC$l_dbhd_first_range))
		    DWC$$DB_Release_buffer
		    (
			Cab,
			(VM_record *) Head->DWC$l_dbhd_first_range
		    );
		if (Is_VM_pointer(Head->DWC$l_dbhd_profile))
		    DWC$$DB_Release_buffer
		    (
			Cab,
			(VM_record *) Head->DWC$l_dbhd_profile
		    );
		break;
		}

	case DWC$k_db_rangemap :
		{
	        struct DWCDB_rangemap *Rangemap; /* Pointer to range map */
		struct DWCDB_header *Head;
		VM_record *Head_vm;
				
		Rangemap = (struct DWCDB_rangemap *)Buffer_data;
		Head_vm = Cab->DWC$a_dcab_header;
		Head = _Bind(Head_vm, DWCDB_header);
		if ((Is_VM_pointer(Rangemap->DWC$l_dbra_flink)) &&
		    (Rangemap->DWC$l_dbra_flink !=
			Head->DWC$l_dbhd_current_range))
		    {
		    DWC$$DB_Release_buffer
		    (
			Cab,
			(VM_record *) Rangemap->DWC$l_dbra_flink
		    );
		    }
		for (i=0; i<DWC$k_db_rangemap_entries; i++)
		    if (Is_VM_pointer(Rangemap->DWC$l_dbra_subvec[i]))
			DWC$$DB_Release_buffer
			(
			    Cab,
			    (VM_record *) Rangemap->DWC$l_dbra_subvec[i]
			);
		if (Buffer_VM->DWC$a_dbvm_special != (char *)NULL)
		    {
		    *(CARD32 *)(Buffer_VM->DWC$a_dbvm_special) =
			Buffer_VM->DWC$l_dbvm_rec_addr;
		    }
		break;
		}	    

	case DWC$k_db_subrangemap :
		{
	        struct DWCDB_subrangemap *submap;	  /* Pointer to subrange map */
		submap = (struct DWCDB_subrangemap *)Buffer_data;
		for (i=0; i<DWC$k_db_submap_entries; i++)
		    if (Is_VM_pointer(submap->DWC$l_dbsu_dayvec[i]))
			DWC$$DB_Release_buffer
			(
			    Cab,
			    (VM_record *) submap->DWC$l_dbsu_dayvec[i]
			);
		break;
		}	    

	case DWC$k_db_day_data :
		{
	        struct DWCDB_day   *Daymap;	  /* Pointer to day map */
		VM_record	    *Item_VM;
		Daymap = (struct DWCDB_day *)Buffer_data;
		while
		(
		    DWC$$DB_Remque
		    (
			(struct DWC$db_queue_head *)
			    &Buffer_VM->DWC$a_dbvm_vm_flink,
			(struct DWC$db_queue_head **) &Item_VM
		    )
		)
		    {
		    if (Item_VM == Cab->DWC$a_dcab_current_item_vm)
			{
			Cab->DWC$a_dcab_current_item_vm = 0;
			}
		    if (Item_VM->DWC$t_dbvm_data[0] == DWC$k_dbit_entry)
			{
			struct DWCDB_entry *Wit;
			Wit = _Bind(Item_VM, DWCDB_entry);
			if (Wit->DWC$b_dben_flags & DWC$m_dben_repeat)
			    {
			    VM_record *Par_vm;
			    Par_vm = (VM_record *)Item_VM->DWC$a_dbvm_parent_vm;
			    Par_vm->DWC$a_dbvm_special--;
			    if (Par_vm->DWC$a_dbvm_special != 0)
				{
				DWC$$DB_Remque
				(
				    (struct DWC$db_queue_head *)
					Item_VM->DWC$a_dbvm_lru_blink,
				    (struct DWC$db_queue_head **) 0
				);
				Item_VM->DWC$a_dbvm_lru_flink = 0;
				}
			    else
				{
				Item_VM = 0;
				}
			    }
			}
		    if (Item_VM != 0)
			{
			DWC$$DB_Freelist_buffer(Cab, Item_VM);
			}
		    }
		break;
		}	    

	case DWC$k_db_day_extension :
		{
		break;
		}	    

	case DWC$k_db_repeat_expr_block :
		{
		VM_record *Item_VM;
		char *Nit;
		
		DWC$$DB_Back_out_repeats(Cab, Buffer_VM);
		DWC$$DB_Remque
		(
		    (struct DWC$db_queue_head *)
			&Buffer_VM->DWC$a_dbvm_vm_flink,
		    (struct DWC$db_queue_head **) &Nit
		);
		Item_VM = (VM_record *)
		    (Nit - _Field_offset(Item_VM, DWC$a_dbvm_lru_flink));
		Item_VM->DWC$a_dbvm_lru_flink = 0;
		DWC$$DB_Freelist_buffer(Cab, Item_VM);
		break;
		}		
	
	default :
		break;
	}		    	
    /*
    **  Now current buffer now no longer points to any other buffer,
    **	and more to the point, no other buffer points BACK to this
    **	one for address fixup. Proceed by removing pointers to this
    **	buffer, declare the memory free, which will in fact remove
    **	it from the cache.
    */

    if (Buffer_VM->DWC$a_dbvm_parent_vm != (char **)NULL)
	{
#if 0
	*(Buffer_VM->DWC$a_dbvm_parent_vm) = (char *)Buffer_VM->DWC$l_dbvm_rec_addr;
#endif
	*((CARD32 *)Buffer_VM->DWC$a_dbvm_parent_vm) = (CARD32)Buffer_VM->DWC$l_dbvm_rec_addr;
	}
    else
	{
	printf ("\nERROR. No parent VM for record %10lX, Type %d\n",
	    Buffer_VM->DWC$l_dbvm_rec_addr, Buffer_data[0]);
	}		    

    DWC$$DB_Freelist_buffer(Cab, Buffer_VM);
    return(TRUE);
}

int
DWC$$DB_Write_virtual_record
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	VM_record *Input_record_vm)
#else	/* no prototypes */
	(Cab, Input_record_vm)
	struct DWC$db_access_block	*Cab;
	VM_record *Input_record_vm;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine writes out a disk record from virtual memory. The
**	source record is part of the virtual disk structure. A copy of the
**	record is made in a write buffer. All virtual pointers of the record
**	are then fixed, according to each specific record type. The result
**	is then written out to disk.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	Input_record_vm : Pointer to DWC$db_vm_record
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION CODES:
**
**	0 -- Buffer written
**     -1 -- Failed to write buffer
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    unsigned long sts;		    /* temp work status			*/
    char *write_buf;		    /* buffer to write from 		*/
    int i;			    /* Temp loop counter		*/
    
    /*
    **  Get started. Make sure that writes are permitted in this session. If
    **	not, signal an error and return back to caller with failure
    */
    _Set_cause(DWC$_PUTLBUF);
    if (!(Cab->DWC$l_dcab_flags & DWC$m_dcab_write))
	{
	_Signal(DWC$_READONLY);
	return(_Failure);
	}

    /*
    **  Validate the target pointer. Make sure that the record is allocated,
    **	etc.
    */
    DWC$$DB_Validate_record_ptr(Cab, Input_record_vm->DWC$l_dbvm_rec_addr, 1);

    /*
    **  Copy the source record to the write buffer, so that we can do fixup.
    */
    write_buf = Cab->DWC$a_dcab_write_buff;
    memcpy(write_buf, &(Input_record_vm->DWC$t_dbvm_data[0]), DWC$k_db_record_length);

    /*
    **  Perform record specific fixup of VM pointers
    */
    switch (write_buf[0])
	{
	case DWC$k_db_header :
		{
	        struct DWCDB_header *Head;	/* Pointer to header block */
	        Head = (struct DWCDB_header *)write_buf;
		_Fixup(Head->DWC$l_dbhd_current_range);
		_Fixup(Head->DWC$l_dbhd_first_range);
		_Fixup(Head->DWC$l_dbhd_profile);
		_Fixup(Head->DWC$l_dbhd_repeat_head);
		break;
		}
	case DWC$k_db_bitmap :	    break;

	case DWC$k_db_rangemap :
		{
	        struct DWCDB_rangemap *Rangemap; /* Pointer to range map */
		Rangemap = (struct DWCDB_rangemap *)write_buf;
		_Fixup(Rangemap->DWC$l_dbra_flink);
		for (i=0; i<DWC$k_db_rangemap_entries; i++)
		    _Fixup(Rangemap->DWC$l_dbra_subvec[i]);
		break;
		}	    

	case DWC$k_db_subrangemap :
		{
	        struct DWCDB_subrangemap *submap;	  /* Pointer to subrange map */
		submap = (struct DWCDB_subrangemap *)write_buf;
		for (i=0; i<DWC$k_db_submap_entries; i++)
		    _Fixup(submap->DWC$l_dbsu_dayvec[i]);
		break;
		}	    

	case DWC$k_db_day_data :
		{
	        struct DWCDB_day *Daymap;	  /* Pointer to day map */
		Daymap = (struct DWCDB_day *)write_buf;
		_Fixup(Daymap->DWC$l_dbda_flink);
		break;
		}	    

	case DWC$k_db_day_extension :
		{
		break;
		}	    

	case DWC$k_db_repeat_expr_block :
		{
		break;
		}

	case DWC$k_db_repeat_exception_vec :
		{
		break;
		}
		
	case DWC$k_db_repeat_expr_vec :
		{
		struct DWCDB_repeat_vector *Rblk;
		struct DWCDB_repeat_expr *Next_exp;
		struct DWC$db_exception_head *Ex_head;
		Rblk = (struct DWCDB_repeat_vector *)write_buf;
		Next_exp = (struct DWCDB_repeat_expr *)Rblk->DWC$t_dbrv_data;
		for (i=1; i<=DWC$k_db_repeats_per_vector; i++)
		    {
		    if (Next_exp->DWC$b_dbre_flags & DWC$m_dbre_used)
			{
			if (Is_VM_pointer(Next_exp->DWC$l_dbre_exceptions))
			    {
			    Ex_head = (struct DWC$db_exception_head *)
				Make_VM_address(Next_exp->DWC$l_dbre_exceptions);
			    Next_exp->DWC$l_dbre_exceptions =
				Ex_head->DWC$l_dbeh_first_rec;
			    }	
			if (Next_exp->DWC$l_dbre_repeat_interval2 != 0)
			    {
			    _Fixup(Next_exp->DWC$l_dbre_repeat_interval2);
			    }
			}
		    Next_exp = (struct DWCDB_repeat_expr *)
			((char *)Next_exp + DWC$k_db_repeat_expr_len);
		    }
		break;
		}
		    
	case DWC$k_db_repeat_extension :
		{
		break;
		}
	    
	case DWC$k_db_profile :
		{
		break;
		}
	default :
		{
		_Signal(DWC$_NOFIXUP);
		break;
		}
	}		    	

    /*
    **  Write out the record and return back to caller
    */
    sts = DWC$$DB_Write_physical_record(Cab, 
	   Input_record_vm->DWC$l_dbvm_rec_addr, 1, Cab->DWC$a_dcab_write_buff);
    _Pop_cause;
    return (sts);
}        

VM_record *
DWC$$DB_Follow_pointer
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	void *Pointer,
	int Expected_type)
#else	/* no prototypes */
	(Cab, Pointer, Expected_type)
	struct DWC$db_access_block *Cab;
	void *Pointer;
	int Expected_type;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine follows a pointer located in a record in memory. The
**	pointer is either a virtual pointer to the target record or it is
**	the address of the record within the database. The two types of
**	pointers are distingiushed by a bitmask (set if Virtual Address).
**
**	If the pointer is a record pointer, the specified record is brought
**	into memory, validated and placed in the cache. The source pointer
**	is then modified to contain the VA instead of the record pointer.
**
**	The target buffer's VA is then extracted and
**	returned to caller.
**
**  FORMAL PARAMETERS:
**
**	Cab	: Pointer to DWC$db_access_block
**	Pointer	: Address of Record pointer input and output.
**		  On input the target field either contains
**		  a virtual pointer or a record pointer in the file.
**		  On output it will contain a VM pointer.
**	Expected_type : Record type expected
**
**  IMPLICIT OUTPUTS:
**
**      Target record may have been read into (freshly allocated) memory
**
**  COMPLETION VALUE:
**
**	Virtual address of target buffer (or failure indicator)
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    VM_record *VM_address;	    /* Pointer to target    */
    
    /*
    **  Treat the special case where the pointer is NULL
    */
    if (*((CARD32*)Pointer) == (CARD32) 0)
	return ((VM_record *)(NULL));
    
    /*
    **  Is this a record pointer (e.g., is this record not in VM)?
    */
    if (Is_VM_pointer(*((CARD32 *)Pointer)))
	{
	/*
	**  Yes, already in VM. Move it to front of cache.
	*/
	VM_address = Make_VM_address(*((CARD32 *)Pointer));
	Cab->DWC$l_hit ++;
	DWC$$DB_Touch_buffer(Cab, VM_address);
	}
    else
	{
    	/*
	**  No. we need to read in the record from the disk.
	**  Put the parent pointers address in the new VM header.
	**  Zap the parent pointer contents to contain the new virtual address.
	**  Finally, place this buffer in front of the cache
	*/
	VM_address = DWC$$DB_Read_physical_record
	    (Cab, *((CARD32 *)Pointer), 1, Expected_type);

	VM_address->DWC$a_dbvm_parent_vm = (char **)Pointer;
	*((CARD32 *)Pointer) = Make_VM_pointer(VM_address);
	Cab->DWC$l_miss ++;
	DWC$$DB_Cache_buffer(Cab, VM_address);
	}

    /*
    **  Return to caller with the virtual address of the VM buffer
    */
    return (VM_address);
}	

VM_record *
DWC$$DB_Follow_pointer_NC
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	void *Ptr,
	int Typ)
#else	/* no prototypes */
	(Cab, Ptr, Typ)
	struct DWC$db_access_block	*Cab;
	void *Ptr;
	int Typ;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine is identical to DWC$$DB_Follow_pointer except that
**	the target for the follow is not in the cache (will not be placed there
**	and no attempt is made to move it to the front of the cache).
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	Ptr : Address of Record pointer. The target field either contains
**	      a virtual pointer or a record pointer.
**	Typ : Record type expected
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION VALUE:
**
**	Virtual address of target buffer (or -1 if failure)
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    struct DWC$db_vm_record *Ptr2;	    /* Pointer to target    */
    
    /*
    **  Treat the special case where the pointer is NULL
    */
    if (*((CARD32 *)Ptr) == (CARD32) 0)
	return (0);
    
    /*
    **  Is this a record pointer (e.g., is this record not in VM)?
    */
    if (DWC$$DB_Is_va_addr(*((CARD32 *)Ptr)) == (CARD32) 0)

	/*
	**  Yes. we need to bring in the record.  Save pointer to the pointer
	**  so that we can perform a fixup later on.
	*/
	{
	Ptr2 = DWC$$DB_Read_physical_record
	    (Cab, *((CARD32 *)Ptr), 1, Typ);
	Ptr2->DWC$a_dbvm_parent_vm = (char **)Ptr;

	/*
	**  Zap the original pointer to contain the new virtual address
	*/
	*((CARD32 *)Ptr) = DWC$$DB_Make_va_addr((CARD32)Ptr2);
	}

    /*
    **  Return to caller with the virtual address of the VM buffer
    */
    return (VM_record *)(DWC$$DB_Make_real_addr(*((CARD32 *)Ptr)));
}	

void
DWC$$DB_Fixup_pointer
#ifdef	_DWC_PROTO_
	(void *Pointer)
#else	/* no prototypes */
	(Pointer)
	void *Pointer;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Change memory address to disk address.
**	This routine is called by each physical record specific fixup routine
**	before the physical record is written back to the file again. Before
**	this routine is called, the record about to be written has been copied
**	into the write buffer. Then, for the target record, fixup specific
**	routine makes sure that all the virtual pointers in the record are
**	replace with record pointers. Each such replacement is done by calling
**	this routine.
**
**  FORMAL PARAMETERS:
**
**	Pointer : Address of the pointer, located in the record in the write
**		  buffer, that is to be fixed.
**
**--
**/
{
    struct DWC$db_vm_record *VM_address;		/* Target VM record */
    
    /*
    **  If the address pointed at has the flag bit set (negatif value)
    **	then it must be a flagged virtual buffer address. Otherwise it
    **	is already the address of a record in the file.
    */
    if (Is_VM_pointer(*((CARD32 *)Pointer)))
	{
	/*
	**  Yes, this pointer need to be converted. This is done by clearing
	**  the flag bit and casting the result as a real VM pointer.
	**  Follow up the real pointer to the target and use the record
	**  address found in the header of the target record.
	*/
	VM_address = Make_VM_address(*((CARD32 *)Pointer));
	*((CARD32 *)Pointer) = VM_address->DWC$l_dbvm_rec_addr;
	}
}

/*
**  End of Module
*/
