/* dwc_db_day_mapping.c */
#ifndef lint
static char rcsid[] = "$Header$";
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
**	This module contains the routines for handling the intermediate
**	mapping of daymaps, through rangemaps and subrangemaps.
**
**--
*/

/*
**  Include Files
*/
#include "dwc_compat.h"

#include <string.h>
#include <stdio.h>

#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"


int DWC$$DB_Locate_rangemap
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	VM_record **Returned_vm,
	int Day,
	int Alloc)
#else	/* no prototypes */
	(Cab, Returned_vm, Day, Alloc)
	struct DWC$db_access_block *Cab;
	VM_record **Returned_vm;
	int Day;
	int Alloc;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine locates the rangemap associated with a particular day.
**	Optionally, this routine can create the rangemap too. If the rangemap
**	is created, it will not be written to disk nor will any disk records
**	be allocated to hold it (see DWC$$DB_Flush_maps).
**
**	Rangemaps are linked together in a singly linked list, originating
**	in the Header record. The field DWC$l_dbhd_first_range points at the
**	first rangemap and the field DWC$l_dbhd_current_range points at the
**	current map.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Returned_vm :	Pointer to rangemap (if found), by reference
**	Day :		Day for which rangemap is to be located
**	Alloc :		TRUE if rangemap is to be allocated if not found
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	DWC$a_dbvm_parent_vm	- Pointer to the pointer that points at this
**				  rangemap. Could be the previous rangemap or
**				  DWC$l_dbhd_first_range.
**	DWC$a_dbvm_special	- If nonzero, backlinks to the
**				  DWC$l_dbhd_current_range field in the header.
**				  Nonzero contents also means that this is the
**				  current rangemap.
**
**  COMPLETION CODES:
**
**      TRUE	- Rangemap (al)located
**	FALSE	- Failed to (al)locate rangemap
**
**  SIDE EFFECTS:
**
**
**--
**/
{
    struct DWC$db_vm_record *Header_vm;	    /* VM header		*/
    struct DWCDB_header *Head;		    /* Header itself		*/
    VM_record	*Rangemap_vm;		    /* rnagemap VM header addr  */
    struct DWCDB_rangemap *Rangemap;	    /* Work rangemap		*/
    Rec_ptr *Next_range_pointer;	    /* Pointer, by reference	*/
    Rec_ptr Zero = 0;			    /* Zero, for use by ref	*/
    Rec_ptr Current_rmap_record;	    /* Record no for "current"	*/
        

    Header_vm = Cab->DWC$a_dcab_header;
    Head = _Bind(Header_vm, DWCDB_header);

    /*
    **  Pick up starting place for search. The default is to start with the
    **	first rangemap. If, however, the day we're looking for is within or
    **	beyond the "current" rangemap AND that we've already joined the flink/
    **	parent_vm for the "current" and the one just before (indicated by that
    **	the "current" field contains a virtual address) we shall start the scan
    **	with the current rangemap.
    */
    Next_range_pointer = (Rec_ptr *) &Head->DWC$l_dbhd_first_range;
    if ((Head->DWC$l_dbhd_current_range != 0) &&
	(Day >= Head->DWC$l_dbhd_current_base) &&
	(Is_VM_pointer(Head->DWC$l_dbhd_current_range)))
	Next_range_pointer = (Rec_ptr *) &Head->DWC$l_dbhd_current_range;

    /*
    **  Determine the record address of the current rangemap. This will be
    **	used later when we want to join records.
    */
    if (Is_VM_pointer(Head->DWC$l_dbhd_current_range))
	{
	Rangemap_vm = Make_VM_address(Head->DWC$l_dbhd_current_range);
	Current_rmap_record = Rangemap_vm->DWC$l_dbvm_rec_addr;
	}
    else
	{
	Current_rmap_record = Head->DWC$l_dbhd_current_range;
	}

    /*
    **  Unless otherwise, stay in this loop until we've come to the end of the
    **	list. This is the main scanning loop.
    */
    while (*Next_range_pointer != (CARD32) 0)
	{

	/*
	**  Point at the "Next" rangemap.
	*/
        Rangemap_vm = _Follow(Next_range_pointer, DWC$k_db_rangemap);
	Rangemap = _Bind(Rangemap_vm, DWCDB_rangemap);

	/*
	**  If we now have come to the "current" rangemap we need to make
	**  sure that the header indicates that the "current" is in memory.
	**  This update can happen if we encounter it, after having started the
	**  scan from "first". If, however, we started the scan from "current"
	**  then this update will not _change_ the contents of the field.
	*/
	if (Rangemap->DWC$l_dbra_baseday == Head->DWC$l_dbhd_current_base)
	    {
	    Head->DWC$l_dbhd_current_range = Make_VM_pointer(Rangemap_vm);
	    Rangemap_vm->DWC$a_dbvm_special = (char *)&(Head->DWC$l_dbhd_current_range);
	    }

	/*
	**  Have we gone far enough? If not, we need to make a special check
	**  here. If the rangemap following this one (the one that was too
	**  far) is in fact the "current" rangemap we need to join the links so
	**  that the VM database is kept up-to-date.
	*/
	if (Day >= (Rangemap->DWC$l_dbra_baseday + DWC$k_db_days_per_rangemap))
	    {
	    Next_range_pointer = &(Rangemap->DWC$l_dbra_flink);
	    if ((Rangemap->DWC$l_dbra_flink == Current_rmap_record) &&
		(Is_VM_pointer(Head->DWC$l_dbhd_current_range)))
		{
		Rangemap->DWC$l_dbra_flink = Head->DWC$l_dbhd_current_range;
		}
	    Next_range_pointer = (Rec_ptr *) &(Rangemap->DWC$l_dbra_flink);
	    }
	else
	    {

	    /*
	    ** We've either found it or it does not exist (if we're before then
	    ** its not here). In eny event, this will terminate the loop.
	    */
	    if (Day < Rangemap->DWC$l_dbra_baseday)
		Next_range_pointer = &Zero;	/* Gone beyond target day   */
	    else
		break;					    /* Found it!    */
	    }
	}

    /*
    **  The scan is now complete. Did we find the rangemap?
    */
    if (*Next_range_pointer == (CARD32) 0)
	{

	/*
	**  No, we did not find it. If caller requested to allocate if not
	**  found, do it, else return FALSE
	*/
	if (Alloc)
	    return (DWC$$DB_Add_rangemap(Cab, Returned_vm, Day));
	else
	    return (FALSE);
	}
    else
	{

	/*
	**  Yes, we found it. Pass pointer back to caller and return in success
	*/
	*Returned_vm = Rangemap_vm;
	return (TRUE);
	}
}

int DWC$$DB_Add_rangemap
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	VM_record **Returned_vm,
	int Day)
#else	/* no prototypes */
	(Cab, Returned_vm, Day)
	struct DWC$db_access_block *Cab;
	VM_record **Returned_vm;
	int Day;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine allocates a new rangemap. The rangemap will not be
**	linked in with the rest of the VM structures, in case we need to
**	back out this operation or the operation in which this call is part of.
**	However, all fields in the allocated rangemap are setup such as if it
**	was linked in.
**
**	No disk record is allocated. That and the joining of links is done by
**	DWC$$DB_Flush_maps.
**
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Returned_vm :	Pointer to rangemap (if found), by reference
**	Day :		Day for which rangemap is to be located
**	Alloc :		TRUE if rangemap is to be allocated if not found
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	DWC$a_dbvm_parent_vm	- Pointer to the pointer that points at this new
**				  rangemap. Could be the previous rangemap or
**				  DWC$l_dbhd_first_range.
**	DWC$a_dbvm_special	- If nonzero, backlinks to the
**				  DWC$l_dbhd_current_range field in the header.
**				  Nonzero contents also means that this is the
**				  current rangemap.
**
**  COMPLETION CODES:
**
**      FALSE	- Failed to allocate rangemap
**	TRUE	- Rangemap allocated
**
**  SIDE EFFECTS:
**
**
**--
**/
{
    VM_record	    *Header_vm;		    /* Header VM address	    */
    struct DWCDB_header *Head;		    /* Header itself		    */
    struct DWCDB_rangemap *New_rangemap;   /* Allocated rangemap	    */
    Rec_ptr *Rangemap_parent_pointer;	    /* Addr of pointer we're    */
					    /* following		    */
    VM_record *Next_rangemap_vm;	    /* Pointer to current rangemap  */
    struct DWCDB_rangemap *Next_rangemap;  /* Rangemap (no VM header)	    */
    Rec_ptr *Rangemap_current_pointer;	    /* Backlink to current field    */
    VM_record	    *New_range_vm;	    /* New rangemap Virtual address */
    int Follow_cnt;			    /* Count of follows we've done  */


    Header_vm = Cab->DWC$a_dcab_header;
    Head = _Bind(Header_vm, DWCDB_header);

    /*
    **  Get a free buffer and start filling it in.
    */
    DWC$$DB_Get_free_buffer_z(  Cab,
				    DWC$k_db_record_length,
				    &New_range_vm);
    New_rangemap = _Bind(New_range_vm, DWCDB_rangemap);
    New_rangemap->DWC$b_dbra_blocktype = DWC$k_db_rangemap;

    /*
    **  Set the baseday of the rangemap. All rangemaps are aligned with
    **	the max number of days a rangemap can span.
    */
    New_rangemap->DWC$l_dbra_baseday =	DWC$k_db_days_per_rangemap *
				    ( Day / DWC$k_db_days_per_rangemap );

    /*
    **  Pick up starting place for search. The default is to start with the
    **	first rangemap. If, however, the day we're looking for is within or
    **	beyond the "current" rangemap AND that we've already joined the flink/
    **	parent_vm for the "current" and the one just before (indicated by that
    **	the "current" field contains a virtual address) we shall start the scan
    **	with the current rangemap.
    */
    Rangemap_parent_pointer = (Rec_ptr *) &(Head->DWC$l_dbhd_first_range);
    if ((Head->DWC$l_dbhd_current_range != 0) &&
	(Day > Head->DWC$l_dbhd_current_base))
	Rangemap_parent_pointer = (Rec_ptr *) &(Head->DWC$l_dbhd_current_range);

    /*
    **  Scan for place to store this rangemap. Follow_cnt is used for making
    **	sure that we do not get into the sitation where we've followed so many
    **	links that the next follow will cause all the previous rangemaps to be
    **	purged out. This can happen if the number of rangemaps is equal to the
    **	number of entries there are in the cache.
    **
    **	At successful completion of this loop we should know:
    **	    a) The Rangemap in front of the one we're about to add, if
    **	       it exists
    **	    b) The Rangemap behind the one we're about to add
    **	    c) Any backlink to the "current" field in the header, in case we're
    **	       adding the first rangemap to this database.
    */
    Follow_cnt = 0;
    if  (*Rangemap_parent_pointer != 0)
	{
	while (1)
	    {
	    if (Follow_cnt == DWC$k_dbvm_max_cache)
		{
		DWC$$DB_Freelist_buffer(Cab, New_range_vm);
		return (FALSE);
		}
	    Follow_cnt++;
	    Next_rangemap_vm = _Follow(Rangemap_parent_pointer,
				       DWC$k_db_rangemap);
	    if (Next_rangemap_vm == NULL)
		{
		DWC$$DB_Freelist_buffer(Cab, New_range_vm);
		return (FALSE);
		}
	    Next_rangemap = _Bind(Next_rangemap_vm, DWCDB_rangemap);
	    if (Day < Next_rangemap->DWC$l_dbra_baseday) break;
	    Rangemap_parent_pointer = (Rec_ptr *) &(Next_rangemap->DWC$l_dbra_flink);
	    if (*Rangemap_parent_pointer == 0) break;
	    }
	Rangemap_current_pointer = 0;
	}
    else
	{
	Rangemap_current_pointer = (Rec_ptr *) &(Head->DWC$l_dbhd_current_range);
	}

    /*
    **  If we're adding at the end, make sure that the flink is Zero for the new
    **	one. If, on the other hand, we're not at the front, make our Flink point
    **	at what is in front of us.
    */
    if (*Rangemap_parent_pointer == 0)
	{
	New_rangemap->DWC$l_dbra_flink = 0;
	}
    else
	{
	New_rangemap->DWC$l_dbra_flink = Make_VM_pointer(Next_rangemap_vm);
	}

    /*
    **  Save backlinks for DWC$$DB_Flush_maps and others. Pass pointer back to
    **	caller and return in success
    */
    New_range_vm->DWC$a_dbvm_parent_vm = (char **)Rangemap_parent_pointer;
    New_range_vm->DWC$a_dbvm_special = (char *)Rangemap_current_pointer;
    *Returned_vm = New_range_vm;
    return (TRUE);
}

int
DWC$$DB_Locate_submap
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	int Day,
	VM_record **Returned_vm,
	int Alloc)
#else	/* no prototypes */
	(Cab, Day, Returned_vm, Alloc)
	struct DWC$db_access_block *Cab;
	int Day;
	VM_record **Returned_vm;
	int Alloc;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine locates the subrangemap associated with a particular day.
**	Optionally the routine can also create the subrangemap. First, this
**	routine will locate the rangemap associated with the subrangemap.
**	
**	The subrangemap will not be linked in with the rest of the VM
**	structures, in case we need to back out this operation or the operation
**	in which this call is part of.  However, all fields in the allocated
**	subrangemap are setup such as if it was linked in.
**
**	No disk record is allocated. That and the joining of links is done by
**	DWC$$DB_Flush_maps.
**
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Returned_vm :	Pointer to subrangemap (if found), by reference
**	Day :		Day for which subrangemap is to be located
**	Alloc :		TRUE if subrangemap is to be allocated if not found
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	DWC$a_dbvm_parent_vm	- Pointer to the pointer, within the rangemap
**				  associated with this subrangemap, that points
**				  at this subrangemap.
**	DWC$a_dbvm_special	- Points back to the start of the associated
**				  rangemap (VM buffer).
**
**  COMPLETION CODES:
**
**      FALSE	- Failed to (al)locate subrangemap
**	TRUE	- Subrangemap (al)located
**
**  SIDE EFFECTS:
**
**
**--
**/
{

    VM_record *Range_vm;			/* VM buffer for the	    */
						/* associated rangemap	    */
    struct DWCDB_rangemap *Range;		/* Associated rangemap	    */
    int Relative_submap;			/* Relative index of this   */
						/* subrangemap within the   */
						/* rangemap vector	    */
    VM_record *Parent_vm;			/* Backlink to field that   */
						/* points at this subrange  */
    VM_record *Submap_vm;			/* VM buffer for this	    */
						/* subrange map		    */
    struct DWCDB_subrangemap *Submap;		/* Subrangemap without	    */
						/* header		    */
    

    /*
    **  First, (al)locate the associated rangemap. Pass the Allocation flag down
    **	from the caller of this routine. Did we get it?
    */
    if (DWC$$DB_Locate_rangemap(Cab, &Range_vm, Day, Alloc))
	{

	/*
	**  Allocated or not, we have the rangemap.
	*/
	Range = _Bind(Range_vm, DWCDB_rangemap);
	Relative_submap = (Day - Range->DWC$l_dbra_baseday) /
			    DWC$k_db_submap_entries;
	Parent_vm = (VM_record *)&(Range->DWC$l_dbra_subvec[Relative_submap]);

	/*
	**  Bring the associated subrangemap into memory. This can only work if
	**  the rangemap we got existed before (as well as this subrangemap)
	*/
	Submap_vm = _Follow(Parent_vm, DWC$k_db_subrangemap);
	if (Submap_vm != NULL)
	    {
	    Submap_vm->DWC$a_dbvm_special = (char *)Range_vm;
	    *Returned_vm = Submap_vm;
	    return (TRUE);
	    }

	/*
	**  The fact that we got this far means that we have a rangemap,
	**  allocated or not, but no associated subrangemap. If allocation
	**  wasn't requested, then that implies that the rangemap we have
	**  existed before. If so, we can safely return with a failure. The
	**  rangemap will just be left where it is, loaded in the cache.
	*/
	if (!Alloc)
	    return (FALSE);

	/*
	**  User wants a new subrangemap. Ask for a free buffer. If this fails,
	**  back out the Rangemap, in case it was just allocated.
	*/
	DWC$$DB_Get_free_buffer_z(	Cab,
					DWC$k_db_record_length,
					&Submap_vm);

	/*
	**  Everything worked fine. Fill in this subrangemap and return a
	**  pointer back to caller.
	*/
	Submap = _Bind(Submap_vm, DWCDB_subrangemap);
	Submap->DWC$b_dbsu_blocktype = DWC$k_db_subrangemap;
	Submap->DWC$w_dbsu_baseday = Day - (Day % DWC$k_db_submap_entries);
	Submap_vm->DWC$a_dbvm_parent_vm = (char **)Parent_vm;
	Submap_vm->DWC$a_dbvm_special = (char *)Range_vm;
	*Returned_vm = Submap_vm;
	return (TRUE);
	}

    /*
    **  We could not find the rangemap and that implies there is no subrangemap
    **	for this day.
    */
    return (FALSE);
}

int
DWC$$DB_Locate_day
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	VM_record **Returned_vm,
	int Day,
	int Alloc)
#else	/* no prototypes */
	(Cab, Returned_vm, Day, Alloc)
	struct DWC$db_access_block *Cab;
	VM_record **Returned_vm;
	int Day;
	int Alloc;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine locates the daymap associated with a particular day.
**	Optionally the routine can also create the daymap. First, this
**	routine will locate the subrangemap associated with the daymap.
**	
**	The daymap will not be linked in with the rest of the VM structures, in
**	case we need to back out this operation or the operation in which this
**	call is part of.  However, all fields in the allocated daymap are setup
**	such as if it was linked in.
**
**	No disk record is allocated. That and the joining of links is done by
**	DWC$$DB_Flush_maps.
**
**	Please note that if Allocation is not requested and the target day is
**	empty but that there are repeat expressions triggering on the specified
**	day, a stub will be allocated in VM. This stub will be deallocated when
**	the day context is cleared.
**
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Returned_vm :	Pointer to daymap (if found), by reference
**	Day :		Day for which daymap is to be located
**	Alloc :		TRUE if daymap is to be allocated if not found
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	DWC$a_dbvm_parent_vm	- Pointer to the pointer, within the subrangemap
**				  associated with this daymap, that points
**				  at this daymap.
**	DWC$a_dbvm_special	- Points back to the start of the associated
**				  subrangemap (VM buffer).
**
**  COMPLETION CODES:
**
**      FALSE	- Failed to (al)locate daymap
**	TRUE	- Daymap (al)located
**
**  SIDE EFFECTS:
**
**
**--
**/
{
    int Subday;				/* Relative day within subrangemap  */
    VM_record *Submap_vm;		/* VM buffer for subrangemap	    */
    VM_record *Daymap_vm;		/* VM buffer for daymap		    */
    struct DWCDB_subrangemap *Submap;	/* Subrangemap (without VM head)    */
    struct DWCDB_day *Daymap;		/* Daymap (without VM head)	    */
    int Repeats;			/* Boolean: There are repeats	    */
    

    /*
    **  Zap day context, in case this fails. In eny event, we're starting from
    **	scratch looking at this day.
    */
    DWC$$DB_Clear_day_context(Cab);

    /*
    **  Determine if there are any repeats on target day.
    */
    Repeats = DWC$$DB_Check_day(Cab, Day, DWC$k_use_empty);
    
    /*
    **  (Al)locate subrangemap and rangemap associated with this day. Did this
    **	work?
    */
    if (DWC$$DB_Locate_submap(Cab, Day, &Submap_vm, (Alloc | Repeats)))
	{

	/*
	**  Yes, we have the subrangemap
	*/
	Submap = _Bind(Submap_vm, DWCDB_subrangemap);
	Subday = Day % DWC$k_db_submap_entries;

	/*
	**  Get the daymap. Did it work?
	*/
	Daymap_vm = _Follow(&Submap->DWC$l_dbsu_dayvec[Subday], DWC$k_db_day_data);
	if (Daymap_vm == 0)
	    {

	    /*
	    **  No. Shall we allocate one? If not, return failure. Leave the
	    **	rangemap and subrangemap in cache. The fact that Alloc is false
	    **	at this point implies that both the rangemap and the subrangemap
	    **	must have existed.
	    */
	    if (!(Alloc | Repeats)) return (FALSE);

	    /*
	    **  The caller wants a new daymap. Allocate a VM buffer. Back this
	    **	out if we could not get the buffer.
	    */
	    DWC$$DB_Get_free_buffer_z(  Cab,
					    DWC$k_db_record_length,
					    &Daymap_vm);

	    /*
	    **  All fine and dandy. Fill in the Daymap.
	    */
	    Daymap = _Bind(Daymap_vm, DWCDB_day);
	    Daymap->DWC$b_dbda_blocktype = DWC$k_db_day_data;
	    Daymap->DWC$w_dbda_baseday = Day;
	    Daymap_vm->DWC$a_dbvm_parent_vm = (char **)&(Submap->DWC$l_dbsu_dayvec[Subday]);
	    Daymap_vm->DWC$a_dbvm_vm_flink = (VM_record *)&(Daymap_vm->DWC$a_dbvm_vm_flink);
	    Daymap_vm->DWC$a_dbvm_vm_blink = (VM_record *)&(Daymap_vm->DWC$a_dbvm_vm_flink);
	    }
	else
	    {

	    /*
	    **  The daymap already exists and we got it. Attempt to load the
	    **	items associated with this day. If this failed, back out this
	    **	operation.
	    */
	    if ((Daymap_vm->DWC$a_dbvm_vm_flink == NULL) &&
		!DWC$$DB_Load_day(Cab, Daymap_vm))
		{
		DWC$$DB_Release_day(Cab, Daymap_vm);
		return (FALSE);
		}
	    }

	/*
	**  Make sure backlink points to Subrangemap
	*/
	Daymap_vm->DWC$a_dbvm_special = (char *)Submap_vm;

	/*
	**  We have the daymap, subrangemap and rangemap. Initialize
	**  Day (Get_item) context.
	*/
	Cab->DWC$a_dcab_current_item_vm = (VM_record *)&(Daymap_vm->DWC$a_dbvm_vm_flink);
	Cab->DWC$l_dcab_current_day = Day;
	Cab->DWC$a_dcab_current_day_vm = Daymap_vm;
	*Returned_vm = Daymap_vm;

	/*
	**  Merge in the repeat expressions and then return back to caller
	**  (This may have the unfortunate side effect of reinserting alarms
	**  that are already in the alarm queue. pglf 9/17/90)
	*/
	return (DWC$$DB_Insert_repeats(Cab, Daymap_vm, Day));
	}

    /*
    **  We did not get the subrangemap. Indicate failure
    */
    return (FALSE);
}

int
DWC$$DB_Load_day
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	VM_record *Day_VM)
#else	/* no prototypes */
	(Cab, Day_VM)
	struct DWC$db_access_block *Cab;
	VM_record *Day_VM;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine will unpack the data from the day record and from the
**	extension record into individual item blocks that will all be placed in
**	a linked list in the daymap vm header. Items are continuing as long as
**	the item blocktype is not zero. An item header does not span the day
**	data boundary.
**
**	A number of sanity checks are performed on the data. They are:
**		- consistent length,
**		- recognized item code,
**		- modification level bytes.
**
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Day_VM :	VM record for day (by ref)
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	DWC$a_dbvm_vm_flink	- Head for queue of items associated with day.
**	DWC$a_dbvm_vm_blink	
**
**  COMPLETION CODES:
**
**      FALSE	- Failed to load
**	TRUE	- Items loaded
**
**  SIDE EFFECTS:
**
**
**--
**/
{
    struct DWCDB_day	*Day_data;	    /* Day data structure	*/
    VM_record		*Item_VM;	    /* Item header		*/
    VM_record		*Ext_VM;	    /* Extension header		*/
    struct DWCDB_item	*Item;		    /* input item pointer (file)*/
    char		*Input_data;	    /* byte in input item buf	*/
    char		*Output_data;	    /* byte in output item buf	*/
    unsigned long	*Ext_pointer;	    /* Pointer to extension	*/
    int	Item_size;			    /* Size of current item	*/
    int Max_data;			    /* Max data in this part	*/
    int i;				    /* Temp loop variable	*/

    /*
    **	Initialize the link header and some work variables.
    */
    Day_VM->DWC$a_dbvm_vm_flink = (VM_record *)&(Day_VM->DWC$a_dbvm_vm_flink);
    Day_VM->DWC$a_dbvm_vm_blink = (VM_record *)&(Day_VM->DWC$a_dbvm_vm_flink);
    Day_data = _Bind(Day_VM, DWCDB_day);
    Item = (struct DWCDB_item *)&(Day_data->DWC$t_dbda_data[0]);
    Ext_pointer = (unsigned long *) &Day_data->DWC$l_dbda_flink;
    Ext_VM = NULL;
    Max_data = DWC$k_db_max_day_data;		/* Copy loop tests for <0   */

    /*
    **  Start looping through the items in the day data.
    **	Check item block type. Allocate memory and hook up.
    */
    while (Item->DWC$b_dbit_blocktype != 0)
	{
	switch (Item->DWC$b_dbit_blocktype)
	    {
	    case DWC$k_dbit_entry:
		Item_size = Item->DWC$w_dbit_size;
		DWC$$DB_Get_free_buffer_z(Cab,
			    Item_size, &Item_VM);
		Item_VM->DWC$a_dbvm_special = (char *)Day_VM;
		DWC$$DB_Insque
		(
		    (struct DWC$db_queue_head *) Item_VM,
		    (struct DWC$db_queue_head *) Day_VM->DWC$a_dbvm_vm_blink
		);
		Output_data = (char *)&(Item_VM->DWC$t_dbvm_data[0]);
		Input_data = (char *)Item;

		/*
		**  The output item is allocated and hooked up.
		**  the input is ready to be copied, but is all the data there?
		*/
		if (Item_size >= Max_data)
		    {

		    /*
		    **  The data does not fit entirely. Copy first bit, read
		    **	extension and then copy the rest.
		    */
		    memcpy(Output_data, Input_data, Max_data);
		    Output_data += Max_data;
		    Item_size -= Max_data;
		    if (!DWC$$DB_Read_day_extension (Cab, &Ext_VM,
					Day_data, &Input_data, &Max_data) ||
			(Item_size > Max_data))
			{
			_Signal(DWC$_DLENMM);
			DWC$$DB_Release_day(Cab, Day_VM);
			return(FALSE);
			}
		    }    /* End of two-staged copy			    */
		/*
		**  Copy the (rest of) input data into the item record.
		*/
		memcpy(Output_data, Input_data, Item_size);
		Item_size += (Item->DWC$w_dbit_size & 1); /* Alignment */
		Input_data += Item_size;
		Max_data -= Item_size;
		Item = (struct DWCDB_item *)Input_data;/* Point the next item	    */
		break;
		
	    case DWC$k_dbit_extension:

		/*
		**  The item header for the next item did not fit
		**	entirely into the day data. Continue in extension.
		**  The previous Item is entirely copied. The next item
		**  will start in the extension... if there is any.
		*/
		if (DWC$$DB_Read_day_extension
		    (Cab, &Ext_VM, Day_data, (char **)&Item, &Max_data))
		{
		    break;
		}
		else
		    return (TRUE);

	    default:
		{
		_Signal(DWC$_UNKDAYIT);
		if (Ext_VM != 0) DWC$$DB_Freelist_buffer(Cab, Ext_VM);
		DWC$$DB_Release_day(Cab, Day_VM);
		return (FALSE);
		}
	    }       /* End of switch on item type.			    */
	if (Max_data == 0)
	    {
	    break;
	    }
	}          /* End of while loop					    */

    /*
    **  If there was a VM buffer allocated for the day extension, then
    **	it is no longer needed at this point. Release it.
    */
    if (Ext_VM != NULL)
	{
	DWC$$DB_Freelist_buffer (Cab, Ext_VM);
	}
    return (TRUE);
}    

int DWC$$DB_Flush_day
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	VM_record *Day_VM,
	int Day_num)
#else	/* no prototypes */
	(Cab, Day_VM, Day_num)
	struct DWC$db_access_block *Cab;
	VM_record *Day_VM;
	int Day_num;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine will pack the item data hanging of the day record into the
**	day data space as long as it fits and into extension records as needed.
**	Items are tightly packed, terminating with a 0 byte.  An item header
**	does not span the day data boundary. The sanity information provide for
**	are: consistent length field, and two updated sanity bytes.  If the
**	extension record length or address changes, then the Day is updated.
**
**	Once the day extension has been written out, the day and all the parent
**	maps are flushed out too.
**	
**	Repeat expressions are not flushed out.
**
**	Items have to start on even word boundary because the size field (word)
**	will be accessed while the item is still in the record buffer. This
**	is of importance on machines that require aligment. Because of the
**	nature of how items are stored in the daymap and in the extension
**	record, the decision to padd or not can be made by looking at the
**	size of the item. If the size is odd, then it must be padded.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Day_VM :	VM record for day (by ref)
**	Day_num :	DWC day of day
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	DWC$a_dbvm_vm_flink	- Head for queue of items associated with day.
**	DWC$a_dbvm_vm_blink	
**
**  COMPLETION CODES:
**
**	DWC$k_db_normal	    -- Day flushed
**	DWC$k_db_insdisk    -- Could not extend file to accomodate item
**	DWC$k_db_dayfull    -- Target day full; item could not be added
**	DWC$k_db_failure    -- General failure; please check errno (and
**			       possibly vaxc$errno).
**
**  SIDE EFFECTS:
**
**
**--
**/
{
    struct DWCDB_day	*Day_data;	    /* Day data structure	*/
    VM_record		*Item_VM;	    /* Item header		*/
    VM_record		*Ext_VM;	    /* Extension header		*/
    struct DWCDB_extension *Ext_data;	    /* Day extension		*/
    struct DWCDB_item	*Item;		    /* item data pointer(VM)	*/
    struct DWCDB_entry	*Entry;		    /* Entry pointer		*/
    char		*Input_data;	    /* byte in input item buf	*/
    char		*Output_data;	    /* byte in output item buf	*/
    int Max_data;			    /* Max data for this part	*/
    int Ext_count_old;			    /* Currently allocated extension*/
    int Ext_count_new;			    /* needed extension recs count  */
    Rec_ptr Ext_address_old;		    /* Old extension pointer	*/
    Rec_ptr Ext_address_new;		    /* New extension pointer	*/
    int	Item_size;			    /* Size of current item	*/
    int i;				    /* Temp loop variable	*/
    int Size_stat;			    /* Status from size compute	*/
    int temp_status;			    /* Temp work status		*/
    int Significant;			    /* Significant entries in d	*/
    int Significant_old;		    /* Old sig flag		*/
    VM_record *Submap_vm;		    /* Subrangemap (VM)		*/
    struct DWCDB_subrangemap *Submap;	    /* Real submap		*/
    int Day_idx;			    /* Index of day in submap	*/
    int Force_submap;			    /* Force write of submap	*/

    /*
    **  Get started
    */
    Day_data = _Bind(Day_VM, DWCDB_day);
    Max_data = DWC$k_db_max_day_data;		    /* copy loop tests for < */
    Output_data = &Day_data->DWC$t_dbda_data[0];
    Ext_count_old = Day_data->DWC$w_dbda_ext_count;
    Day_data->DWC$w_dbda_ext_count = 0;

    /*
    **  Start looping through the items hooked off the day header.
    **	Copy contents into Day_data or Ext_data. Total size has just
    **	been established an validated by caller.
    */
    Significant = DWC$m_dbsu_insignif;
    Item_VM = Day_VM->DWC$a_dbvm_vm_flink;
    while (Item_VM != (VM_record *)&Day_VM->DWC$a_dbvm_vm_flink)
	{
	Entry = _Bind(Item_VM, DWCDB_entry);
	Item = _Bind(Item_VM, DWCDB_item);
	if ((Item->DWC$b_dbit_blocktype != DWC$k_dbit_entry) ||
	    (!(Entry->DWC$b_dben_flags & DWC$m_dben_repeat)))
	    {	
	    if (!(Entry->DWC$b_dben_flags & DWC$m_dben_insignif))
		{
		Significant = 0;
		}
	    Item_size = Item->DWC$w_dbit_size;
	    Input_data = (char *)Item;
	    /*
	    **  Will all the data fit here?
	    */
	    if (Item_size >= Max_data)
		{
		/*
		**  All the data for this item will not fit in the current
		**	output buffer.  It will actually start in the extension
		**	IF the item header does not fit here.
		*/
		if (Max_data < _Field_offset(Item,DWC$t_dbit_data[0]))
		    {
		    *Output_data = DWC$k_dbit_extension;
		    }
		else
		    {
		    /*
		    **  Copy first bit, then get the extension
		    */
		    memcpy(Output_data, Input_data, Max_data);
		    Input_data += Max_data;
		    Item_size -= Max_data;
		    }
		    
		if (((Size_stat = DWC$$DB_Size_day(Cab, Day_VM,
			&Ext_VM, &Output_data, &Max_data)) != DWC$k_db_normal) ||
			(Item_size > Max_data))
			{
			DWC$$DB_Release_day(Cab, Day_VM);
			return (Size_stat);
			}
		}
	    /*
	    **  Copy whatever is left. (Might be all)
	    */
	    memcpy(Output_data, Input_data, Item_size);

	    /*
	    **  Account for padding at the end (making it even length)
	    */
	    Item_size += (Item->DWC$w_dbit_size & 1);
	    Output_data += Item_size;
	    Max_data -= Item_size;
	    }
	Item_VM = Item_VM->DWC$a_dbvm_vm_flink;
	}		 /* End Item While Loop			    */
    if (Max_data)
	{
	*Output_data = 0;	/* End Item Data marker			    */
	}
    /*
    **  Time has come to go to the disk. First see if we need to
    **  needed an extend record and whether we can re-use the old space.
    */
    Ext_count_new = Day_data->DWC$w_dbda_ext_count;
    Ext_address_old = Day_data->DWC$l_dbda_flink;
    Ext_address_new = Ext_address_old;
    if (Ext_count_new != 0)
	{
	Ext_address_new = DWC$$DB_Alloc_records (Cab, Ext_count_new );
	if ((Ext_address_new == 0) ||
	    (DWC$$DB_Write_physical_record (Cab, Ext_address_new,
		    Ext_count_new, &Ext_VM->DWC$t_dbvm_data[0]) == _Failure))
	    {
	    DWC$$DB_Freelist_buffer (Cab, Ext_VM);
	    DWC$$DB_Release_day(Cab, Day_VM);
	    return (DWC$k_db_insdisk);
	    }
	else
	    {
	    /*
	    **  all done with this buffer now. Free it up.
	    */
	    DWC$$DB_Freelist_buffer (Cab, Ext_VM);
	    }	    
	_Write_back(Cab);
	}

    /*
    **  Determine if we need to update the subrangemap with the new
    **	state of a significance flag
    */
    Submap_vm = (VM_record *)Day_VM->DWC$a_dbvm_special;
    Submap = _Bind(Submap_vm, DWCDB_subrangemap);
    Day_idx = (Day_num % DWC$k_db_submap_entries);
    Significant_old = (Submap->DWC$b_dbsu_dayflags[Day_idx] & DWC$m_dbsu_insignif);
    Force_submap = FALSE;
    if (Significant != Significant_old)
	{
	Submap->DWC$b_dbsu_dayflags[Day_idx] =
		(Submap->DWC$b_dbsu_dayflags[Day_idx] & DWC$m_dbsu_userflags) |
		    Significant;
	Force_submap = TRUE;
	}
    
    /*
    **	The (new) extend, if any, is on the disk.  Write out the day record and
    **	parent maps
    */
    Day_data->DWC$l_dbda_flink = Ext_address_new;
    temp_status = DWC$$DB_Flush_maps(Cab, Day_VM, Force_submap);

    /*
    **	The (new) extend and the day are on the disk. Now deallocate the old
    **	extend records (if any).
    */
    if ((Ext_count_old != 0) && (temp_status == DWC$k_db_normal))
	{
	(void) DWC$$DB_Deallocate_records(Cab, Ext_count_old, Ext_address_old);
	}
    return (temp_status);
}    

int DWC$$DB_Size_day
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	VM_record *Day_VM,
	VM_record **Returned_vm,
	char **Returned_data,
	int *Returned_max)
#else	/* no prototypes */
	(Cab,  Day_VM, Returned_vm, Returned_data,
				Returned_max)
	struct DWC$db_access_block	*Cab;
	VM_record *Day_VM;
	VM_record **Returned_vm;
	char **Returned_data;
	int *Returned_max;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine computes the amount of extension space needed to hold the
**	items bound to this day. Once this has been computed, the associated VM
**	buffer is allocated. The generic part of the buffer is filled in.
**
**	Please note that repeat expressions are not included in the needed
**	byte count.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Day_VM :	VM record for day (by ref)
**	Returned_vm :	VM record associated with extension buffer (by ref)
**	Returned_data :	Start of data area in extension, by ref
**	Returned_max :	Max number of data bytes available
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	DWC$a_dbda_ext_count	- Records required for extension
**	DWC$a_dbda_ext_sanity	- Sanity field (incremented)
**
**  COMPLETION CODES:
**
**	DWC$k_db_normal	    -- Sizing Ok
**	DWC$k_db_dayfull    -- Target day full; item could not be added
**	DWC$k_db_failure    -- General failure; please check errno (and
**			       possibly vaxc$errno).
**  SIDE EFFECTS:
**
**
**--
**/
{
    struct DWCDB_day	*Day_data;	    /* Day data structure	*/
    VM_record		*Ext_VM;	    /* Extension header		*/
    VM_record		*Item_VM;	    /* Item VM header		*/
    struct DWCDB_extension *Ext_data;	    /* Day extension		*/
    struct DWCDB_entry	*Item;	    	    /* Item data		*/
    char *Last_char;			    /* Pointer to last byte in	*/
					    /* buffer			*/
    int Max_data;			    /* Max amount of data we can*/
					    /* store			*/
    char Ext_sanity[1];			    /* Sanity check field	*/
    int Total_bytes;			    /* Total bytes needed	*/
    int Needed_recs;			    /* Number of records needed	*/
    int i;



    /*
    **	Start of by establishing how many bytes required for the storage on
    **	disk.
    */
    Total_bytes = 0;
    Day_data = _Bind(Day_VM, DWCDB_day);
    Item_VM = Day_VM->DWC$a_dbvm_vm_flink;
    while (Item_VM != (VM_record *)&Day_VM->DWC$a_dbvm_vm_flink)
	{
	Item = _Bind(Item_VM, DWCDB_entry);
	if ((Item->DWC$b_dben_blocktype != DWC$k_dbit_entry) ||
	    (!(Item->DWC$b_dben_flags & DWC$m_dben_repeat)))
	    {
	    Total_bytes += Item->DWC$w_dben_size;
	    Total_bytes += (Item->DWC$w_dben_size & 1); /* Alignment */
	    }
	Item_VM = Item_VM->DWC$a_dbvm_vm_flink;
	}

    /*
    **  Will this fit into the Daymap itself or do we need an extension?
    */
    if (Total_bytes < ( DWC$k_db_max_day_data - 1 ) )
	{
	Needed_recs = 0;
	}
    else
	{

	/*
	**  Determine how many records the extension needs to be.
	*/
	Total_bytes -= DWC$k_db_max_day_data;		    /* in the day.  */
	Total_bytes += 3;				    /* control bytes*/
	Total_bytes += _Field_offset(Ext_data,DWC$t_dbex_data[0]);
	Needed_recs = (Total_bytes + DWC$k_db_record_length - 1);
	Needed_recs = (Needed_recs / DWC$k_db_record_length);

	/*
	**  Determine if we can allocate this many records or if the day
	**  will be overflowed.
	*/
	if (Needed_recs > DWC$k_db_max_allocation_chunk)
	    {
	    return (DWC$k_db_dayfull);
	    }
	}


    /*
    **  Get the extension buffer.
    */
    DWC$$DB_Get_free_buffer(Cab, Needed_recs * DWC$k_db_record_length,
	    &Ext_VM);

    /*
    **  From here on, we should have no more problems. Update the new extension
    **	block and the daymap
    */
    Ext_sanity[0] = Day_data->DWC$b_dbda_ext_sanity + 1;
    Ext_data = _Bind(Ext_VM, DWCDB_extension);
    Ext_data->DWC$b_dbex_blocktype = DWC$k_db_day_extension;
    Ext_data->DWC$w_dbex_ext_count = Needed_recs;
    Ext_data->DWC$b_dbex_sanity	   = Ext_sanity[0];
    Day_data->DWC$w_dbda_ext_count = Needed_recs;
    Day_data->DWC$b_dbda_ext_sanity= Ext_sanity[0];
    Last_char = &Ext_VM->DWC$t_dbvm_data[
		    (Needed_recs * DWC$k_db_record_length) - 1 ];
    Last_char[0] = Ext_sanity[0];

    /*
    **  Pass return parameters back to caller and return with success
    */
    *Returned_max = (int)(Last_char - (char *)&Ext_data->DWC$t_dbex_data[0] - 1); /* Sanity &  */
								/* Null byte */
    *Returned_data = (char *)&(Ext_data->DWC$t_dbex_data[0]);
    *Returned_vm = Ext_VM;
    return (DWC$k_db_normal);
}

int DWC$$DB_Read_day_extension
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	VM_record **Returned_vm,
	struct DWCDB_day *Day_data,
	char **Returned_data,
	int *Max_data)
#else	/* no prototypes */
	(Cab, Returned_vm, Day_data, Returned_data, Max_data)
	struct DWC$db_access_block	*Cab;
	VM_record **Returned_vm;
	struct DWCDB_day *Day_data;
	char **Returned_data;
	int *Max_data;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Read in the day extension block and sanity check it.  The read routine
**	called will actuall mark the contents of the pointer to indicate that
**	is is in memory.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Returned_vm :	VM record associated with extension buffer (by ref)
**	Day_data :	Daymap (by ref)
**	Returned_data :	Start of data area in extension, by ref
**	Max_data :	Number of bytes used in the extension
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**      FALSE	- Failed to read extension or allocate buffer for it. This
**		  status is also returned if there is no extension.
**	TRUE	- Extension loaded
**
**  SIDE EFFECTS:
**
**
**--
**/
{		
    VM_record		*Ext_VM;	    /* Extension record header	*/
    struct DWCDB_extension *Ext_data;	    /* Day extension		*/
    char		*Last_char;	    /* Pointer to last byte	*/
    Rec_ptr		Ext_pointer;	    /* Pointer to extension	*/
    int			Ext_count;	    /* Count of extension recs	*/
    char Ext_sanity[1];			    /* Expected sanity value	*/


    /*
    **  Get started. Return FALSE if there is no extension
    */
    Ext_pointer = Day_data->DWC$l_dbda_flink;
    if (Ext_pointer == 0)
	return (FALSE);
    Ext_count = Day_data->DWC$w_dbda_ext_count;
    Ext_sanity[0] = Day_data->DWC$b_dbda_ext_sanity;

    /*
    **  Load the extension and compute the number of databytes.
    */
    Ext_VM = DWC$$DB_Read_physical_record(Cab, Ext_pointer,
	     Ext_count, DWC$k_db_day_extension);
    Ext_data = _Bind(Ext_VM, DWCDB_extension);
    Last_char = &Ext_VM->DWC$t_dbvm_data[
		    (Ext_count*DWC$k_db_record_length) - 1 ];
    *Max_data = (int)(Last_char - (char *)&Ext_data->DWC$t_dbex_data[0] - 1);	/* sanity & */
								/* Null byte */
    /*
    **  Sanity check and return to caller
    */
    if ((Ext_sanity[0] == Ext_data->DWC$b_dbex_sanity) &&
	(Ext_sanity[0] == Last_char[0]) &&
	(Ext_count  == Ext_data->DWC$w_dbex_ext_count))
	{
	*Returned_data = (char *)&(Ext_data->DWC$t_dbex_data[0]);
	*Returned_vm = Ext_VM;
	return (TRUE);
	}
    else
	{
	_Signal(DWC$_BADDCHK);
	return (FALSE);
	}
}

void DWC$$DB_Release_maps
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	VM_record *Start_map_vm,
	int Remove_count)
#else	/* no prototypes */
	(Cab, Start_map_vm, Remove_count)
	struct DWC$db_access_block *Cab;
	VM_record *Start_map_vm;
	int Remove_count;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine deletes a number of VM records that were allocated during
**	some add/modify operation. It is intended to remove VM buffers that have
**	not yet been linked in with the rest of the VM structure.
**
**	This routine operates in two modes:
**	    1) Remove all buffers until we hit the Header or the first buffer
**	       with a disk record associated with it.
**	    2) Remove a certain number of records, backwards.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Start_map_vm :	Starting point for remove
**	Remove_count :	Number of records to remove. If zero, use mode (1).
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**
**--
**/
{
    VM_record *Next_map_vm;		/* Next record to remove	*/
    VM_record *Parent_map_vm;		/* Backlink for remove		*/
    int i;				/* Records removed so far	*/
    int Range_exit;			/* TRUE if to exit loop		*/

    /*
    **  Setup and enter loop
    */
    Next_map_vm = Start_map_vm;
    i = 0;
    Range_exit = FALSE;
    while (TRUE)
	{

	/*
	**  Exit, if at the end of the list
	*/
	if (Remove_count == 0)
	    {
	    if (Next_map_vm->DWC$l_dbvm_rec_addr != 0) break;
	    }
	else
	    {
	    if (i == Remove_count) break;
	    i++;
	    }

	/*
	**  Save backlink. Indicate that MUST exit if this record
	**  is a rangemap (because special will backlink into header)
	*/
	Parent_map_vm = (VM_record *)Next_map_vm->DWC$a_dbvm_special;
	if (Next_map_vm->DWC$t_dbvm_data[0] == DWC$k_db_rangemap)
	    {
	    Range_exit = TRUE;
	    }

	/*
	**  Freelist buffer
	*/
	DWC$$DB_Freelist_buffer(Cab, Next_map_vm);
	if (Range_exit) break;
	Next_map_vm = Parent_map_vm;
	}
}

int
DWC$$DB_Allocate_maps
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	VM_record *Start_map_vm,
	Rec_ptr *Starting_record,
	int *Records_allocated)
#else	/* no prototypes */
	(Cab, Start_map_vm, Starting_record, Records_allocated)
	struct DWC$db_access_block *Cab;
	VM_record *Start_map_vm;
	Rec_ptr *Starting_record;
	int *Records_allocated;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Allocate records for each one of the maps (found through backlinks) that
**	do not yet have a record associated with them.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Start_map_vm :	Starting point for allocate
**	Starting_record : Start of allocation, by ref (record addr)
**	Records_allocated : Number of records allocated (by ref)
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	TRUE	- Records allocated
**	FALSE	- Could not allocate requested number of records
**
**  SIDE EFFECTS:
**
**
**--
**/
{
    int Needed_records;		/* Number of records needed		*/
    VM_record *Next_map_vm;	/* Next VM buffer (when counting)	*/
    Rec_ptr Base_record;	/* Starting record for allocation	*/
    int Return_status;		/* Return status from this routine	*/
    
    
    /*
    **  Compute the number of records we need to allocate. Stop if we find
    **	a buffer with a record address or we hit a rangemap (in which case
    **	the backlink goes to the header)
    */
    Needed_records = 0;
    Next_map_vm = Start_map_vm;
    while (TRUE)
	{
	if (Next_map_vm->DWC$l_dbvm_rec_addr != 0) break;
	Needed_records++;
	if (Next_map_vm->DWC$t_dbvm_data[0] == DWC$k_db_rangemap) break;
	Next_map_vm = (VM_record *)Next_map_vm->DWC$a_dbvm_special;
	}
    /*
    **  Assume no base record and that this is going to work. Then, attempt
    **	to get the records. Return failure if we could not get the records.
    */
    Base_record = 0;
    Return_status = TRUE;
    if (Needed_records != 0)
	{
	Base_record = DWC$$DB_Alloc_records(Cab, Needed_records);
	if (Base_record == 0)
	    {
	    Return_status = FALSE;
	    Needed_records = 0;
	    }
	}

    /*
    **  We got the records (or none, as computed). Pass results back to caller
    */
    *Records_allocated = Needed_records;
    *Starting_record = Base_record;
    return (Return_status);
}

int
DWC$$DB_Flush_maps
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	VM_record *Start_map_vm,
	int Force_sub)
#else	/* no prototypes */
	(Cab, Start_map_vm, Force_sub)
	struct DWC$db_access_block *Cab;
	VM_record *Start_map_vm;
	int Force_sub;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine joins VM buffers and disk records when an update
**	transaction is about to complete. It does this in a fashion which allows
**	the operation to back-out if it fails midway.
**
**	It is expected that this routine will be called when, for example,
**	the day extension has been written. Or when a day mark has been updated.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Start_map_vm :	Starting point for flush
**	Force_sub :	Boolean; TRUE if to force write of subrangemap
**			even though it did not need to.
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	TRUE	- Success in flush
**	FALSE	- Flush failed, operation was backed out. In reality, the
**		  only case where control is also returned to caller, this
**		  means that there was not enough disk space.
**
**  SIDE EFFECTS:
**
**
**--
**/
{
    Rec_ptr Start_record;		/* Start record for disk write	    */
    int Record_count;			/* Number of records allocated	    */
    int i;				/* Loop variable for update	    */
    VM_record *Next_map_vm;		/* Temp pointer to current record   */
    VM_record *Last_rangemap_vm;	/* Used when adding a rangemap	    */
    Rec_ptr *Patch_field_ptr_1;		/* Pointer to first backlink field  */
					/* to patch			    */
    Rec_ptr *Patch_field_ptr_2;		/* Pointer to second backlink field */
					/* to patch. This field is	    */
					/* currently only used for	    */
					/* "current" rangemap		    */
    Rec_ptr Patch_field_org_1;		/* Original contents of field #1    */
    Rec_ptr Patch_field_org_2;		/* Original contents of field #2    */
    VM_record *Head_VM;			/* Header VM record		    */
    struct DWCDB_header *Head;		/* "real" header		    */
    struct DWCDB_rangemap *Rangemap;	/* Used when dealing with rangemaps */
    VM_record *Rangemap_front_vm;	/* VM pointer to rangemap in front  */
    int stat;				/* Temp work status		    */
    
    /*
    **  Attempt to allocate the records we need to make this updatable. If it
    **	failed, back out this operation.
    */
    if (!DWC$$DB_Allocate_maps(Cab, Start_map_vm, &Start_record, &Record_count))
	{
	DWC$$DB_Release_maps(Cab, Start_map_vm, Record_count);
	return (FALSE);
	}

    /*
    **  Enter update loop. In this loop, records that previously did not exist
    **	on disk are placed on disk. They are written backwards, so that there
    **	should be nothing pointing to them, in case we fail.
    */
    Patch_field_ptr_1 = Patch_field_ptr_2 = 0;
    Next_map_vm = Start_map_vm;
    for (i=0; i<Record_count; i++)
	{

	/*
	**  The record address for each new record is handed out backwards,
	**  since forward disk head movement is more likely than backwards.
	*/
	Next_map_vm->DWC$l_dbvm_rec_addr =
	    (Start_record + ((Record_count - 1 - i) * DWC$k_db_record_length));

	/*
	**  Write out this record. If it failed, back out this operation.
	*/
	if (DWC$$DB_Write_virtual_record(Cab, Next_map_vm) == _Failure)
	    {
	    if (Patch_field_ptr_1 != 0)
		*Patch_field_ptr_1 = Patch_field_org_1;
	    DWC$$DB_Release_maps(Cab, Start_map_vm, Record_count);
	    return (FALSE);
	    }

	/*
	**  So far so good. Cache this buffer. Patch parent records flink to
	**  contain the VM of the record we just wrote out.
	*/
	DWC$$DB_Cache_buffer(Cab, Next_map_vm);
	Patch_field_ptr_1 = (Rec_ptr *)Next_map_vm->DWC$a_dbvm_parent_vm;
	Patch_field_org_1 = *Patch_field_ptr_1;
	*Patch_field_ptr_1 = Make_VM_pointer(Next_map_vm);

	/*
	**  Rangemaps need special attention. In particular, records need to be
	**  joined, etc. Please note that if we're adding the "current" record
	**  it means that this is the first rangemap. And, in that case, we do
	**  not need to worry about updating both the header and the previous
	**  record.
	*/
	if (Next_map_vm->DWC$t_dbvm_data[0] == DWC$k_db_rangemap)
	    {
	    Rangemap = _Bind(Next_map_vm, DWCDB_rangemap);
	    if (Is_VM_pointer(Rangemap->DWC$l_dbra_flink))
		{
		Rangemap_front_vm = Make_VM_address(Rangemap->DWC$l_dbra_flink);
		Rangemap_front_vm->DWC$a_dbvm_parent_vm =
			(char **)&(Rangemap->DWC$l_dbra_flink);
		}
	    if (Next_map_vm->DWC$a_dbvm_special != 0)		
		{
		Patch_field_ptr_2 = (Rec_ptr *)Next_map_vm->DWC$a_dbvm_special;
		Patch_field_org_2 = *Patch_field_ptr_2;
		Last_rangemap_vm = Next_map_vm;
		Next_map_vm = Cab->DWC$a_dcab_header;
		}
	    else
		{
		Head_VM = Cab->DWC$a_dcab_header;
		Head = _Bind(Head_VM, DWCDB_header);
		if (Patch_field_ptr_1 != (Rec_ptr *)&(Head->DWC$l_dbhd_first_range))
		    {
		    Next_map_vm = (VM_record *)(
				    (char *)(Patch_field_ptr_1) -
				    ((char *)(&Rangemap->DWC$l_dbra_flink) -
				    (char *)(Next_map_vm)));
		    }
		}		
	    }
	else
	    {

	    /*
	    **  Not a rangemap. Back up one level.
	    */
	    Next_map_vm = (VM_record *)Next_map_vm->DWC$a_dbvm_special;
	    }
	}

    /*
    **  We've now written all records we can, without joining the data with the
    **	existing structure. Before we do that, make sure that pending I/Os are
    **	completed.
    */
#if 1
    _Write_back(Cab);
#else
{
    int	flush_val;
    flush_val = fflush(Cab->DWC$l_dcab_fd);
    if (flush_val != 0)
    {
	_Record_error;
	_Signal(DWC$_FILEIOERR);
    }
}
#endif    
    /*
    **  Patch "current" field, if needed
    */
    if (Patch_field_ptr_2 != 0)
	*Patch_field_ptr_2 = Make_VM_pointer(Last_rangemap_vm);

    /*
    **  Join this with the rest of the database. If it fails, try and back out
    **	the entire operation.
    */
    if (DWC$$DB_Write_virtual_record(Cab, Next_map_vm) == _Failure)
	{
	if (Patch_field_ptr_1 != 0)
	    {
	    *Patch_field_ptr_1 = Patch_field_org_1;
	    if (Patch_field_ptr_2 != 0)
		*Patch_field_ptr_2 = Patch_field_org_2;
	    }
	DWC$$DB_Release_maps(Cab, Start_map_vm, Record_count);
	return (FALSE);
	}

    /*
    **  Shall we force the writing of the subrangemap?
    */
    stat = TRUE;
    if ((Force_sub) &&
	(Next_map_vm->DWC$t_dbvm_data[0] == DWC$k_db_day_data))
	{
	Next_map_vm = (VM_record *)Next_map_vm->DWC$a_dbvm_special;
	if (DWC$$DB_Write_virtual_record(Cab, Next_map_vm) == _Failure)
	    {
	    stat = FALSE;
	    }
	}
    
    /*
    **  Once again, flush this out. When that is done, the operation is
    **	complete.
    */
    _Write_back(Cab);
    return (stat);
}

void DWC$$DB_Clear_day_context
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
**	This routine clears the current day item context for Get_item. If
**	the daymap being pointed at by the day context points at a day with no
**	database records associated with it, it will be released. This indicates
**	that it is some sort of daymap stub created to simulate a populated day
**	when there were only repeat expressions on it.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**
**--
**/
{

    VM_record *Day_vm;		    /* Daymap for target day		*/
    VM_record *Submap_vm;	    /* Submap for target day		*/
    
    /*
    **  Shall I release any daymap stub?
    */
    DWC$$DB_Fry_stub_day(Cab, Cab->DWC$a_dcab_current_day_vm);
        
    /*
    **  Get rid of temp arrays used by Get_specific_r_item
    */
    DWC$$DB_Fry_temp_arrays(Cab);
    
    /*
    **  Indicate that we no longer have an item nor a Day pointer.
    */
    Cab->DWC$a_dcab_current_item_vm = 0;
    Cab->DWC$a_dcab_current_day_vm = 0;
}

int
DWC$$DB_Fry_stub_day
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	VM_record *Daymap_vm)
#else	/* no prototypes */
	(Cab, Daymap_vm)
	struct DWC$db_access_block	*Cab;
	VM_record *Daymap_vm;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine releases a daymap that was allocated to be a stub
**	for repeat expressions.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Daymap_vm :	Daymap (Submap and Rmap) to fry
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	TRUE	- Maps zapped
**	FALSE	- Nothing zapped
**
**  SIDE EFFECTS:
**
**
**--
**/
{

    VM_record *Submap_vm;	    /* Submap for target day		*/
    
    /*
    **  Shall I release any daymap stub?
    */
    if ((Daymap_vm != 0) &&
	(Daymap_vm->DWC$l_dbvm_rec_addr == 0))
	{
	Submap_vm = (VM_record *)Daymap_vm->DWC$a_dbvm_special;
	DWC$$DB_Release_buffer(Cab, Daymap_vm);
	DWC$$DB_Release_maps(Cab, Submap_vm, 0);
	return (TRUE);
	}

    /*
    **  Nothing to delete, tell caller
    */
    return (FALSE);
}

int DWC$$DB_Fry_temp_arrays
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab)
#else	/* no prototypes */
	(Cab)
	struct DWC$db_access_block	*Cab;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine releases the temporary arrays used by Get_specific_r_item
**	for keeping text and alarm array after stub day has been fried.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	TRUE	- Arrays fried
**	FALSE	- Failed to fry arrays
**
**  SIDE EFFECTS:
**
**
**--
**/
{

    /*
    **  Get rid of Alarm array (if present)
    */
    if (Cab->DWC$a_dcab_temp_alarm != 0)
	{
	XtFree(Cab->DWC$a_dcab_temp_alarm);
	Cab->DWC$a_dcab_temp_alarm = 0;
	}

    /*
    **  Get rid of Text array (if present)
    */
    if (Cab->DWC$a_dcab_temp_text != 0)
	{
	XtFree(Cab->DWC$a_dcab_temp_text);
	Cab->DWC$a_dcab_temp_text = 0;
	}

    /*
    **  Done, back to caller
    */
    return (TRUE);
}

int DWCDB__FryTempArrays
#ifdef	_DWC_PROTO_
    (struct DWC$db_access_block *Cab)
#else	/* no prototypes */
    (Cab)
    struct DWC$db_access_block *Cab;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine releases the temporary arrays used by Get_specific_r_item
**	for keeping text and alarm array after stub day has been fried.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	TRUE	- Arrays fried
**	FALSE	- Failed to fry arrays
**
**  SIDE EFFECTS:
**
**
**--
**/
{
    /*
    **  Get rid of Alarm array (if present)
    */
    if (Cab->DWC$a_dcab_temp_alarm != NULL)
    {
	XtFree (Cab->DWC$a_dcab_temp_alarm);
	Cab->DWC$a_dcab_temp_alarm = NULL;
    }

    /*
    **  Get rid of Text array (if present)
    */
    if (Cab->DWC$a_dcab_temp_text != NULL)
    {
	XmStringFree (Cab->DWC$a_dcab_temp_text);
	Cab->DWC$a_dcab_temp_text = NULL;
    }

    /*
    **  Get rid of Icons array (if present)
    */
    if (Cab->DWC$a_dcab_temp_icons != NULL)
    {
	XtFree (Cab->DWC$a_dcab_temp_icons);
	Cab->DWC$a_dcab_temp_icons = NULL;
    }

    /*
    **  Done, back to caller
    */
    return (TRUE);
}

void
DWC$$DB_Release_day
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	VM_record *Daymap_vm)
#else	/* no prototypes */
	(Cab, Daymap_vm)
	struct DWC$db_access_block *Cab;
	VM_record *Daymap_vm;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine releases the current day data. It is expected that this
**	routine will be called when things like Modify_item, etc, fails.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Daymap_vm :	Starting point for release
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**
**--
**/
{


    /*
    **  First, make sure we don't leave any obsolete Day context behind.
    */
    Cab->DWC$a_dcab_current_item_vm = 0;
    Cab->DWC$a_dcab_current_day_vm = 0;

    /*
    **  If there is a record allocated to the Daymap then this simply means that
    **	the VM structures associated with this day may be incorrect. In that
    **	case we only need to get rid of the Daymap and its children. Else, we've
    **	not yet placed this day on the disk. If that is the case we need to back
    **	out all VM buffers (possibly up to and including the rangemap).
    */
    if (Daymap_vm->DWC$l_dbvm_rec_addr != 0)
	{
	DWC$$DB_Release_buffer(Cab, Daymap_vm);
	}
    else
	{
	DWC$$DB_Release_maps(Cab, Daymap_vm, 0);
	}
}

int
DWC$$DB_Is_subrangemap_empty
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	VM_record *Submap_vm)
#else	/* no prototypes */
	(Cab, Submap_vm)
	struct DWC$db_access_block	*Cab;
	VM_record *Submap_vm;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine will determine if specified Subrangemap is empty or
**	not. If empty, this subrangemap could be a candidate for delete.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Submap_vm :	Pointer to Virtual subrangemap to check
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	TRUE	    - Subrangemap is empty (and can be deleted)
**	FALSE	    - Subrangemap is not empty and should not be deleted
**
**  SIDE EFFECTS:
**
**	none
**--
**/
{
    struct DWCDB_subrangemap *Submap;		    /* Real subrangemap rec */
    int Empty;					    /* True if empty	    */
    int i;					    /* Local loop variable  */
    
    /*
    **  Assume subrangemap is empty
    */
    Empty = TRUE;
    Submap = _Bind(Submap_vm, DWCDB_subrangemap);

    /*
    **  Check contents of Subrangemap for each day. As soon as any nonzero
    **	value is detected, indicate not empty and leave loop.
    */
    for (i=0; i<DWC$k_db_submap_entries; i++)
	{
	if ((Submap->DWC$l_dbsu_dayvec[i] != 0) ||
	    (Submap->DWC$b_dbsu_dayflags[i] != 0) ||
	    (Submap->DWC$b_dbsu_daymarks[i] != 0))
	    {
	    Empty = FALSE;
	    break;
	    }
	}

    /*
    **  If Empty is still TRUE then we did not find anything. Back to caller
    **	with an indication of what we found.
    */
    return (Empty);
}

int DWC$$DB_Is_daymap_empty
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	VM_record *Daymap_vm)
#else	/* no prototypes */
	(Cab, Daymap_vm)
	struct DWC$db_access_block	*Cab;
	VM_record *Daymap_vm;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine will determine if specified Daymap is empty or
**	not. If empty, this daymap could be a candidate for delete.
**
**	Please note that the day will be considered empty, even though there are
**	repeat expressions associated with the day (since they are stored
**	separately). 
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Daymap_vm :	Pointer to Virtual daymap to check
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	TRUE	    - Daymap is empty (and can be deleted)
**	FALSE	    - Daymap is not empty and should not be deleted
**
**  SIDE EFFECTS:
**
**	none
**--
**/
{
    int Empty;					    /* True if empty	    */
    VM_record *Item_vm;				    /* Next item record	    */
    struct DWCDB_entry *Entry;			    /* Entry in day	    */
    
    /*
    **  Assume daymap is empty
    */
    Empty = TRUE;

    /*
    **  Start looping through entries in the day. As soon as a non-repeating
    **	entry is found then we know that the day is not emoty.
    */
    Item_vm = Daymap_vm->DWC$a_dbvm_vm_flink;
    while (Item_vm != (VM_record *)&Daymap_vm->DWC$a_dbvm_vm_flink)
	{
	Entry = _Bind(Item_vm, DWCDB_entry);
	if ((Entry->DWC$b_dben_blocktype != DWC$k_dbit_entry) ||
	    (!(Entry->DWC$b_dben_flags & DWC$m_dben_repeat)))
	    {
	    Empty = FALSE;
	    break;
	    }
	Item_vm = Item_vm->DWC$a_dbvm_vm_flink;
	}

    /*
    **  Back to caller with whatever we found out.
    */
    return (Empty);
}

int DWC$$DB_Dump_rmaps
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab)
#else	/* no prototypes */
	(Cab)
	struct DWC$db_access_block	*Cab;
#endif	/* prototypes */

/*
**  Dump in memory rangemaps
*/

{
    struct DWC$db_vm_record *Header_vm;	    /* VM header		*/
    struct DWCDB_header *Head;		    /* Header itself		*/
    VM_record	*Rangemap_vm;		    /* rnagemap VM header addr  */
    struct DWCDB_rangemap *Rangemap;	    /* Work rangemap		*/
    Rec_ptr Next_range_pointer;
        
    Header_vm = Cab->DWC$a_dcab_header;
    Head = _Bind(Header_vm, DWCDB_header);

    printf("\n**** Rangemap database dump. ****\n");
    printf("Current baseday: %d, ", Head->DWC$l_dbhd_current_base);
    printf("Current pointer: %8X\n", Head->DWC$l_dbhd_current_range);
    printf("First range pointer %8X\n", Head->DWC$l_dbhd_first_range);
    printf("&Head->DWC$l_dbhd_current_range = %8X\n",
	&Head->DWC$l_dbhd_current_range);
    printf("&Head->DWC$l_dbhd_first_range = %8X\n",
	&Head->DWC$l_dbhd_first_range);
    printf("Following FIRST thread\n");
    Next_range_pointer = Head->DWC$l_dbhd_first_range;
    while (1)
	{
	if (!Is_VM_pointer(Next_range_pointer)) break;
	printf("V: %8X", Next_range_pointer);
	if (Next_range_pointer == 0)
	    {
	    printf("\nEnd of list.\n");
	    break;
	    }
	Rangemap_vm = Make_VM_address((unsigned long)Next_range_pointer);
	printf(", D: %8X, P: %8X, S: %8X", Rangemap_vm->DWC$l_dbvm_rec_addr,
		Rangemap_vm->DWC$a_dbvm_parent_vm,
		Rangemap_vm->DWC$a_dbvm_special);
	Rangemap = _Bind(Rangemap_vm, DWCDB_rangemap);
	printf(", B: %8d", Rangemap->DWC$l_dbra_baseday);
	printf(", F: %8X", Rangemap->DWC$l_dbra_flink);
	if (Rangemap->DWC$l_dbra_baseday == Head->DWC$l_dbhd_current_base)
	    {
	    printf("\n");
	    break;
	    }
	printf("\n");
	Next_range_pointer = Rangemap->DWC$l_dbra_flink;
	}
    printf("Following CURRENT thread\n");
    Next_range_pointer = Head->DWC$l_dbhd_current_range;
    while (1)
	{
	if (!Is_VM_pointer(Next_range_pointer)) break;
	printf("V: %8X", Next_range_pointer);
	if (Next_range_pointer == 0)
	    {
	    printf("\nEnd of list.\n");
	    break;
	    }
	Rangemap_vm = Make_VM_address((unsigned long)Next_range_pointer);
	printf(", D: %8X, P: %8X, S: %8X", Rangemap_vm->DWC$l_dbvm_rec_addr,
		Rangemap_vm->DWC$a_dbvm_parent_vm,
		Rangemap_vm->DWC$a_dbvm_special);
	Rangemap = _Bind(Rangemap_vm, DWCDB_rangemap);
	printf(", B: %8d", Rangemap->DWC$l_dbra_baseday);
	printf(", F: %8X", Rangemap->DWC$l_dbra_flink);
	printf("\n");
	Next_range_pointer = Rangemap->DWC$l_dbra_flink;
	}
    printf("**** Dump complete.****\n\n");
    return (TRUE);
}

int DWC$$DB_First_day
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
**	This routine locates the first day containing any non-repeating data.
**
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**      -1	- Unable to locate first day
**	n >= 0	- First day in Calendar with scheduled event
**
**  SIDE EFFECTS:
**
**	none
**--
**/
{
    struct DWC$db_vm_record *Header_vm;	    /* VM header		*/
    struct DWCDB_header *Head;		    /* Header itself		*/
    VM_record	*Rangemap_vm;		    /* rangemap VM header addr  */
    struct DWCDB_rangemap *Rangemap;	    /* Work rangemap		*/
    VM_record	*Subrangemap_vm;	    /* subrangemap VM header addr  */
    struct DWCDB_subrangemap *Subrangemap; /* Work subrangemap		*/
    Rec_ptr Next_range_pointer;	    	    /* Pointer	*/
    Rec_ptr Zero = 0;			    /* Zero, for use by ref	*/
    Rec_ptr Current_rmap_record;	    /* Record no for "current"	*/
    int first_day = -1;			    /* First day with data in calendar. Default to none. */
    int smap_index;			    /* Loop Index Variable */
    int day_index;			    /* Loop Index Variable */

    Header_vm = Cab->DWC$a_dcab_header;
    Head = _Bind(Header_vm, DWCDB_header);

    /*
    **  Starting place for the search is the first rangemap.
    */
    Next_range_pointer = Head->DWC$l_dbhd_first_range;
    
    /* 
    ** Search until we find a day with an entry.
    */
    while (Next_range_pointer != 0)
	{

	/*
	**  Point at the "Next" rangemap.
	*/
        Rangemap_vm = _Follow( &Next_range_pointer, DWC$k_db_rangemap);
	Rangemap = _Bind(Rangemap_vm, DWCDB_rangemap);

	/*
	**  If we now have come to the "current" rangemap we need to make
	**  sure that the header indicates that the "current" is in memory.
	**  This update can happen if we encounter it, after having started the
	**  scan from "first". If, however, we started the scan from "current"
	**  then this update will not _change_ the contents of the field.
	*/
	if (Rangemap->DWC$l_dbra_baseday == Head->DWC$l_dbhd_current_base)
	    {
	    Head->DWC$l_dbhd_current_range = Make_VM_pointer(Rangemap_vm);
	    Rangemap_vm->DWC$a_dbvm_special = (char *)&(Head->DWC$l_dbhd_current_range);
	    }

	/* 
	 * Check all of the submaps for this range for non-empty days.
	 */
	for ( smap_index = 0;
	      smap_index < DWC$k_db_rangemap_entries;
	      smap_index++)
	    {
	    Subrangemap_vm = _Follow( &Rangemap->DWC$l_dbra_subvec[ smap_index], DWC$k_db_subrangemap);
	    if ( Subrangemap_vm  == NULL)
		/*
		 * No submap here for this range map. 
		 */
		continue;
	    else
	    	/* Set up pointer to access submap. */
		Subrangemap = _Bind( Subrangemap_vm, DWCDB_subrangemap);

	    /* 
	     * Check each day in this subrange map to see if it has any events.
	     */
	    for (day_index = 0;
	         day_index < DWC$k_db_submap_entries;
	         day_index++)
	    	{
		/* 
		 * Day has information if dayvec entry is non-null.
		 */
		if (Subrangemap->DWC$l_dbsu_dayvec[day_index] != 0)
		    {
		    /* 
		     * This day has an event, so set first day.
		     * first day = range map baseday 
		     *	     + (days per subrange * (submap_index + 1))
		     *	     + (day_index + 1)
		     * Note: subrange baseday is low 16 bits of day and could be used instead.
		     */
		    first_day = Rangemap->DWC$l_dbra_baseday + (DWC$k_db_submap_entries * smap_index) + day_index;
		    return first_day;
		    }
		}

	    }    
	    
	    /*
	     * Get next range to continue search.
	     */
	    Next_range_pointer = Rangemap->DWC$l_dbra_flink;

	}    /* *Next_range_ptr != NULL */

    /* 
     * Return with the value found.
     */
     
    return first_day;
}

int
DWC$$DB_Last_day
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
**	This routine locates the last day containing any non-repeating data.
**
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**      -1	- Unable to locate last day
**	n >= 0	- Last day in Calendar with scheduled event
**
**  SIDE EFFECTS:
**
**	none
**
**  NOTES:
**
**	Due to the "virtual" memory/file record number scheme used in the DWC database,
**	it is unsafe to maintain pointers to records in virtual memory since they may be
**	flushed from the cache without notice. Therefore, if we do not find any days with
**	non-repeating events in the last range map, we need to start again from the front
**	of the list to locate the next to last map. Since each range map covers ~6.9 years,
**	we shouldn't find many empty range maps.
**--
**/
{
    struct DWC$db_vm_record *Header_vm;	    /* VM header		*/
    struct DWCDB_header *Head;		    /* Header itself		*/
    VM_record	*Rangemap_vm;		    /* rangemap VM header addr  */
    struct DWCDB_rangemap *Rangemap;	    /* Work rangemap		*/
    VM_record	*Subrangemap_vm;	    /* subrangemap VM header addr  */
    struct DWCDB_subrangemap *Subrangemap; /* Work subrangemap		*/
    Rec_ptr Next_range_pointer;	    	    /* Pointer	*/
    Rec_ptr Zero = 0;			    /* Zero, for use by ref	*/
    Rec_ptr Current_rmap_record;	    /* Record no for "current"	*/
    int last_day = -1;			    /* Last day with data in calendar. Default to none. */
    int smap_index;			    /* Loop Index Variable */
    int day_index;			    /* Loop Index Variable */
    int range_index;			    /* Loop Index Variable */
    int last_range_depth;    		    /* Depth to stop searching for last range of interest. */
#ifndef INT_MAX				    /* Should defined for ANSI C. Used for max search depth. */
#define INT_MAX 32767
#endif


    Header_vm = Cab->DWC$a_dcab_header;
    Head = _Bind(Header_vm, DWCDB_header);

    /*
    **  Starting place for the search is the first rangemap.
    */
    Next_range_pointer = Head->DWC$l_dbhd_first_range;

    /* 
     * Quit if no ranges.
     */
    if (Next_range_pointer == 0)
    	return last_day;

    /*
    **  Initial search depth limit is "infinity".
    */
    last_range_depth = INT_MAX;

    /* 
     * Search backwards through Calendar DB until we find a day with a
     * non-repeating event or note, or until we've finished checking the
     * complete DB.
     */
    while (last_range_depth > 0)
        {
    	/* 
    	** Find the last rangemap we haven't tested yet.
    	*/
    	for (range_index = 0;
	     range_index < last_range_depth;
	     range_index++)
	    {
	    /*
	    **  Point at the "Next" rangemap.
	    */
            Rangemap_vm = _Follow( &Next_range_pointer, DWC$k_db_rangemap);
	    Rangemap = _Bind(Rangemap_vm, DWCDB_rangemap);
    
	    /*
	    **  If we now have come to the "current" rangemap we need to make
	    **  sure that the header indicates that the "current" is in memory.
	    **  This update can happen if we encounter it, after having started the
	    **  scan from "last". If, however, we started the scan from "current"
	    **  then this update will not _change_ the contents of the field.
	    */
	    if (Rangemap->DWC$l_dbra_baseday == Head->DWC$l_dbhd_current_base)
	    	{
	    	Head->DWC$l_dbhd_current_range = Make_VM_pointer(Rangemap_vm);
	    	Rangemap_vm->DWC$a_dbvm_special = (char *)&(Head->DWC$l_dbhd_current_range);
	    	}
    
	    /*
	     * Get next range to continue search.
	     */
	    Next_range_pointer = Rangemap->DWC$l_dbra_flink;
	    if (Next_range_pointer == 0)
	    	break;
    
	    }	/* for range_index */
	
	/* 
	 * Update search depth in case we need to do it again.
	 */
	last_range_depth = range_index;

	/* 
	 * Check all of the submaps for this range for non-empty days.
	 */
	for ( smap_index = DWC$k_db_rangemap_entries-1;
	      smap_index >= 0 ;
	      smap_index--)
	    {
	    Subrangemap_vm = _Follow( &Rangemap->DWC$l_dbra_subvec[ smap_index], DWC$k_db_subrangemap);
	    if ( Subrangemap_vm  == NULL)
		/*
		 * No submap here for this range map. 
		 */
		continue;
	    else
		/* Set up pointer to access submap. */
		Subrangemap = _Bind( Subrangemap_vm, DWCDB_subrangemap);

	    /* 
	     * Check each day in this subrange map to see if it has any events.
	     */
	    for (day_index = DWC$k_db_submap_entries - 1;
		 day_index >= 0;
		 day_index--)
		{
		/* 
		 * Day has information if dayvec entry is non-null.
		 */
		if (Subrangemap->DWC$l_dbsu_dayvec[day_index] != 0)
		    {
		    /* 
		     * This day has an event, so set last day.
		     * last day = range map baseday 
		     *	     + (days per subrange * (submap_index + 1))
		     *	     + (day_index + 1)
		     * Note: subrange baseday is low 16 bits of day and could be used instead.
		     */
		    last_day = Rangemap->DWC$l_dbra_baseday + (DWC$k_db_submap_entries * smap_index) + day_index;
		    return last_day;
		    }
		}   /* for day_index */
	    }   /* for smap_index */
        }    /* while last_range_depth */
    
    /* 
     * Return with the value found.
     */
     
    return last_day;
}

/*
**  End of Module
*/
