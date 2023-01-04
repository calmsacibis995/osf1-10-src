/* DWC_DB_DAY_MANAGEMENT "V3.0-029" */
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
**	This module implements the routines that operate on information
**	bound to days, such as Time maps, Daymarks and Dayflags.
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**  V3.0-	Paul Ferwerda					    13-Sep-1990
**		Change to ANSI declarations
**
**	V2-028	Per Hamnqvist					    04-May-1989
**		Insignificant entries "quick lookup flag" in submap was not
**		preserved when user updated daymarks.
**		
**	V2-027	Per Hamnqvist					    10-Apr-1989
**		Special-only days not picked up correctly from DB on get.
**		
**	V2-026	Per Hamnqvist					    30-Mar-1989
**		Did not propagate DWC$m_day_defaulted when subrangemap not
**		found.
**		
**	V2-025	Per Hamnqvist					    29-Mar-1989
**		Add support for flag DWC$m_day_defaulted in Daymark.
**		
**	V2-024	Per Hamnqvist					    27-Mar-1989
**		Correct a few loop condition checks that were one off.
**		
**	V2-023	Per Hamnqvist					    26-Mar-1989
**		a) Invalidate day type cache when modifying day attributes.
**		b) Load correct subrangemap on daytype cache reload
**		
**	V2-022	Per Hamnqvist					    24-Mar-1989
**		Increment repeat change index when we touch day attributes.
**		
**	V2-021	Per Hamnqvist					    24-Mar-1989
**		One more level of parenthesis needed for insignificance test to
**		work properly.
**		
**	V2-020	Per Hamnqvist					    24-Mar-1989
**		Flag insignificant entries instead of significant
**		
**	V2-019	Per Hamnqvist					    23-Mar-1989
**		a) Start using new day marks.
**		b) Change daytype cache to mask out special flag, since it
**		   cannot be used for conditional repeats.
**		
**	V2-018	Per Hamnqvist					    23-Mar-1989
**		Add full support for significant entries
**		
**	V2-017	Per Hamnqvist					    17-Mar-1989
**		a) Mark daytype cache valid after it has been reloaded.
**		b) Reload_daytype did not load the correct subrangemap.
**		c) Reload_daytype did not propagate information from
**		   Set_workweek.
**		
**	V2-016	Per Hamnqvist					    14-Mar-1989
**		Mask out 8th bit in input to put day attr. Do not allow
**		user to retreive it either (with get_day ..)
**		
**	V2-015	Per Hamnqvist					    14-Mar-1989
**		Add more meaning to Day_used in Get_day_attr.
**
**	V2-014	Per Hamnqvist					    13-Mar-1989
**		Add routines DWC$$DB_Set_centerpoint and DWC$$DB_Get_daytype
**
**	V2-013	Per Hamnqvist					    02-Mar-1989
**		Add routine DWC$DB_Set_workweek
**		
**	V2-012	Per Hamnqvist					    26-Feb-1989
**		Add support for new status codes
**		
**	V2-011	Per Hamnqvist					    25-Feb-1989
**		Deal with not enough disk space better.
**		
**	V1-010	Per Hamnqvist					    05-Sep-1988
**		First shot at porting to pcc.
**		
**	V1-009	Per Hamnqvist					    27-Jul-1988
**		Correct bad loop test conditional.
**		
**	V1-008	Per Hamnqvist					    09-Apr-1988
**		Add support for repeat expressions in routines for retreiving
**		day attributes.
**
**	V1-007	Per Hamnqvist					    07-Apr-1988
**		a) Clear day context if day was truncated.
**		b) Zero backlink after day has been released since it
**		   otherwise will be restored.
**		
**	V1-006	Per Hamnqvist					    06-Apr-1988
**		Release the buffer instead of freelisting it when truncating
**		the day.
**		
**	V1-005	Per Hamnqvist					    29-Feb-1988
**		Add routine to delete empty day.
**		
**	V1-004	Per Hamnqvist					    25-Feb-1988
**		Add support for Day attributes
**		
**	V1-003	Per Hamnqvist					    20-Jan-1988
**		A few changes to make it compile under Ultrix. Most of the
**		code was commented out so that it would compile.
**
**	V1-002	Per Hamnqvist					    24-Dec-1987
**		Add DWC$$DB_Remove_day
**		
**	V1-001	Per Hamnqvist					    30-Nov-1987
**		Use VMS style headers
**
**--
**/

/*
**  Include Files
*/
#include "dwc_compat.h"
#include <string.h>
#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"

int DWC$$DB_Reload_daytype PROTOTYPE ((
	struct DWC$db_access_block  *Cab,
	int start_day,
	int count));
/*
**  Conversion table from old style daymarks to new style
*/
static int DWC$l_mark_cvt_tab[4] =
{
    DWC$k_day_default,
    DWC$k_day_workday,
    DWC$k_day_nonwork,
    DWC$k_day_default | DWC$m_day_special | DWC$m_mark_new_style
};

int
DWC$DB_Put_day_attr
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block  *Cab,
	unsigned long Day,
	unsigned char Daymark,
	unsigned char Dayflags)
#else	/* no prototypes */
	(Cab, Day, Daymark, Dayflags)
	struct DWC$db_access_block  *Cab;
	unsigned long Day;
	unsigned char Daymark;
	unsigned char Dayflags;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Update Daymark and Dayflags associated with a given day. This routine
**	overwrites these two fields so it is recommended to retreive the
**	attributes if they are to be modified.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Day :		Day for which parameters are to updated
**	Daymark	:	New Daymarks. The daymark is the way to indicate
**			the day type:
**
**			    DWC$k_day_default	    -- Follow default work
**						       week.
**			    DWC$k_day_workday	    -- Work day
**			    DWC$k_day_nonwork	    -- Non workday
**
**			In addition to the above types, the following flags
**			can be OR:ed with the day type:
**
**			    DWC$m_day_special	    -- Day is special
**
**	Dayflags :	New dayflags; bit 0..6 (7 will be masked out)
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
**	DWC$k_db_normal	    -- Item added/updated
**	DWC$k_db_insdisk    -- Could not extend file to accomodate item
**	DWC$k_db_failure    -- General failure; please check errno (and
**			       possibly vaxc$errno).
**
**  SIDE EFFECTS:
**
**
**--
**/
{
    VM_record *Submap_vm;			/* Submap (VM) with attrs   */
    struct DWCDB_subrangemap *Submap;		/* Real submap		    */
    int Dayi;					/* Day index within Submap  */
    int retsts;					/* Return status from add   */
    int Cre_it;					/* Create if not found	    */
    
    /*
    **  Remember where we were, in case of error
    */
    _Set_cause(DWC$_DAYATAF);

    /*
    **  Mask out only lower 7 bits. The 8th bit is special and is used to
    **	indicate if there are any significant hard item on day. 8th bit
    **	is managed by internal code.
    */
    Dayflags = (Dayflags & DWC$m_dbsu_userflags);
    
    /*
    **  Mask out lower 7-bits of Daymarks too. We do this so that the user
    **	will not write back the "DWC$m_day_defaulted" flag by accident.
    */
    Daymark = (Daymark & DWC$m_dbsu_userflags);
    
    /*
    **  Assume we need not create a subrangemap and check if we possibly should
    **	do this (e.g., we don't need to attempt to create if caller is just
    **	requesting to zero the fields, in which case the subrangemap could
    **	potentially be removed anyway).
    */
    Cre_it = FALSE;
    if ((Daymark != 0) || (Dayflags != 0)) Cre_it = TRUE;

    /*
    **  Attempt to locate the Subrangemap in which this information is
    **	supposed to be stored. Create Subrangemap and Rangemap if they do
    **	not exist, unless caller is requesting to zero the fields.
    */
    if (!(DWC$$DB_Locate_submap(Cab, Day, &Submap_vm, Cre_it)))
	{
	_Pop_cause;
	if (Cre_it)
	    {
	    return (DWC$k_db_failure);
	    }
	else
	    {
	    return (DWC$k_db_normal);
	    }
	}

    /*
    **  Update the subrangemap
    */
    Submap = _Bind(Submap_vm, DWCDB_subrangemap);
    Dayi = (Day % DWC$k_db_submap_entries);
    Submap->DWC$b_dbsu_daymarks[Dayi] = Daymark;
    Submap->DWC$b_dbsu_dayflags[Dayi] = Dayflags |
	(Submap->DWC$b_dbsu_dayflags[Dayi] & ~DWC$m_dbsu_userflags);

    /*
    **  If we zeroed a field (and we've come this far) it means that this
    **	subrangemap could be empty. If so, we want to delete it. Check
    **	this and delete if needed. Else, flush out maps.
    */
    if ((!(Cre_it)) && (DWC$$DB_Is_subrangemap_empty(Cab, Submap_vm)))
	{
	*Submap_vm->DWC$a_dbvm_parent_vm = 0;
	DWC$$DB_Write_virtual_record
	    (Cab, (VM_record *) Submap_vm->DWC$a_dbvm_special);
	_Write_back(Cab);
	DWC$$DB_Deallocate_records(Cab, 1, Submap_vm->DWC$l_dbvm_rec_addr);
	DWC$$DB_Freelist_buffer(Cab, Submap_vm);
	retsts = DWC$k_db_normal;
	}
    else
	{	    
	retsts = DWC$$DB_Flush_maps(Cab, Submap_vm, FALSE);
	}

    /*
    **  Increment repeat change index since conditional repeats may move as a
    **	result of this change in day attributes.
    */
    Cab->DWC$l_dcab_repeat_ctx++;
        
    /*
    **  Invalidate day type cache if currently valid and the day that we
    **	modified is in the cached range.
    */
    if ((Cab->DWC$l_dcab_flags & DWC$m_dcab_dtcvalid) &&
	(Day >= Cab->DWC$l_dcab_dtbase) &&
	(Day < (Cab->DWC$l_dcab_dtbase + (DWC$k_db_submap_entries * 3))))
	{
	Cab->DWC$l_dcab_flags = Cab->DWC$l_dcab_flags & ~DWC$m_dcab_dtcvalid;
	}

    /*
    **  Done, exit
    */
    _Pop_cause;
    return (retsts);
}

int
DWC$DB_Get_day_attr
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block  *Cab,
	unsigned long Day,
	unsigned char *Daymark,
	unsigned char *Dayflags,
	unsigned char *Dayused)
#else	/* no prototypes */
	(Cab, Day, Daymark, Dayflags, Dayused)
	struct DWC$db_access_block  *Cab;
	unsigned long Day;
	unsigned char *Daymark;
	unsigned char *Dayflags;
	unsigned char *Dayused;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine retreives the Day attributes and the Day flags associated
**	with a day. Days with Daymark = DWC$k_day_default will be evaluated
**	into DWC$k_day_workday or DWC$k_day_nonwork using the input passed
**	in previous call to DWC$DB_Set_workweek.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Day :		Day for which info is to be returned
**	Daymark	:	Buffer to receive Daymark. The daymark is the way
**			to indicate the day type:
**
**			    DWC$k_day_workday	    -- Work day
**			    DWC$k_day_nonwork	    -- Non workday
**
**			In addition to the above types, the following flags
**			can be OR:ed with the day type:
**
**			    DWC$m_day_special	    -- Day is special
**			    DWC$m_day_defaulted	    -- Indicates that the
**						       actual daytype setting
**						       was DWC$k_day_default
**						       and that the routine
**						       converted that, using
**						       input to Set_workweek,
**						       into DWC$k_day_workday
**						       or DWC$k_day_nonwork.
**						       If this flag is clear
**						       it means that the daytype
**						       was set explicitly by
**						       the user.
**
**	Dayflags :	Buffer to receive Dayflags
**	Dayused :	Value =
**
**			    DWC$k_use_empty	    -- Day is empty
**			    DWC$k_use_normal	    -- There is something
**						       on day
**			    DWC$k_use_significant   -- Day contains
**						       something significant
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
**	DWC$k_db_normal	    -- Information retreived
**	DWC$k_db_failure    -- General failure; please check errno (and
**			       possibly vaxc$errno).
**
**  SIDE EFFECTS:
**
**	none
**--
**/
{
    VM_record *Submap_vm;			/* Submap (VM) with attrs   */
    struct DWCDB_subrangemap *Submap;		/* Real submap		    */
    int Dayi;					/* Day index within Submap  */
    int week_day;				/* Day of week		    */
    int temp_daymark;				/* Temp daymark (work)	    */
    int saved_bits;				/* Saved bits before qualif */

    
    /*
    **  Remember where we were, in case of error
    */
    _Set_cause(DWC$_DAYATGF);

    /*
    **  Assume this day is not used
    */
    *Dayused = DWC$k_use_empty;
    
    /*
    **  Attempt to locate associated Subrangemap. If this fails, there
    **	are no attributes. Tell caller.
    */
    week_day = ((Day + 5) % 7);
    if (!(DWC$$DB_Locate_submap(Cab, Day, &Submap_vm, FALSE)))
	{
	if ((1 << week_day) & Cab->DWC$b_dcab_workweek)
	    {
	    *Daymark = (DWC$k_day_workday | DWC$m_day_defaulted);
	    }
	else
	    {
	    *Daymark = (DWC$k_day_nonwork | DWC$m_day_defaulted);
	    }
	*Dayflags = 0;
	*Dayused = DWC$$DB_Check_day(Cab, Day, DWC$k_use_normal);
	_Pop_cause;
	return (DWC$k_db_normal);
	}

    /*
    **  Pick up daymark. First convert from old style to new and then qualify
    **	default day into work or non-work.
    */
    Submap = _Bind(Submap_vm, DWCDB_subrangemap);
    Dayi = (Day % DWC$k_db_submap_entries);
    temp_daymark = Submap->DWC$b_dbsu_daymarks[Dayi];
    if (((temp_daymark & DWC$m_mark_type_mask) != DWC$k_mark_default) &&
	(!(temp_daymark & DWC$m_mark_new_style)))
	{
	temp_daymark = DWC$l_mark_cvt_tab[temp_daymark];
	}
    if ((temp_daymark & DWC$m_mark_type_mask) == DWC$k_day_default)
	{
	saved_bits = temp_daymark & ~DWC$m_mark_type_mask;
	if ((1 << week_day) & Cab->DWC$b_dcab_workweek)
	    {
	    temp_daymark = DWC$k_day_workday | saved_bits | DWC$m_day_defaulted;
	    }
	else
	    {
	    temp_daymark = DWC$k_day_nonwork | saved_bits | DWC$m_day_defaulted;
	    }
	}
    *Daymark = temp_daymark;
	    
    /*
    **  Extract Day attributes and return back to caller
    */
    *Dayflags = (Submap->DWC$b_dbsu_dayflags[Dayi] & DWC$m_dbsu_userflags);
    if (Submap->DWC$l_dbsu_dayvec[Dayi] != 0)
	{
	*Dayused = DWC$k_use_normal;
	if (!(Submap->DWC$b_dbsu_dayflags[Dayi] & DWC$m_dbsu_insignif))
	    {
	    *Dayused = DWC$k_use_significant;
	    }
	else
	    {
	    if (DWC$$DB_Check_day(Cab, Day, DWC$k_use_significant)
		    == DWC$k_use_significant)
		{
		*Dayused = DWC$k_use_significant;
		}
	    }
	}
    else
	{
	*Dayused = DWC$$DB_Check_day(Cab, Day, DWC$k_use_normal);
	}
    _Pop_cause;
    return (DWC$k_db_normal);
}

int
DWC$DB_Get_day_attr_rng
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block  *Cab,
	unsigned long Start_day,
	unsigned long End_day,
	unsigned char *Daymark_vec,
	unsigned char *Dayflag_vec,
	unsigned char *Dayused_vec)
#else	/* no prototypes */
	(Cab, Start_day, End_day, Daymark_vec, Dayflag_vec, Dayused_vec)
	struct DWC$db_access_block  *Cab;
	unsigned long Start_day;
	unsigned long End_day;
	unsigned char *Daymark_vec;
	unsigned char *Dayflag_vec;
	unsigned char *Dayused_vec;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine retreives Day attributes for a range of days.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Start_day :	Day to start retreival
**	End_day :	Last day of retreival
**	Daymark_vec :	Caller allocated byte vector to receive Daymarks.
**			Index 0 corresponds to Start_day. The daymark is
**			the way to indicate the day type:
**
**			    DWC$k_day_workday	    -- Work day
**			    DWC$k_day_nonwork	    -- Non workday
**
**			In addition to the above types, the following flags
**			can be OR:ed with the day type:
**
**			    DWC$m_day_special	    -- Day is special
**			    DWC$m_day_defaulted	    -- Indicates that the
**						       actual daytype setting
**						       was DWC$k_day_default
**						       and that the routine
**						       converted that, using
**						       input to Set_workweek,
**						       into DWC$k_day_workday
**						       or DWC$k_day_nonwork.
**						       If this flag is clear
**						       it means that the daytype
**						       was set explicitly by
**						       the user.
**
**	Dayflag_vec :	Caller allocated byte vector to receive Dayflags.
**			Index 0 corresponds to Start_day.
**	Dayused_vec :	Caller allocated byte vector to receive Value =
**
**			    DWC$k_use_empty	    -- Day is empty
**			    DWC$k_use_normal	    -- There is something
**						       on day
**			    DWC$k_use_significant   -- Day contains
**						       something significant
**
**			Index 0 in vector corresponds to Start_day.
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
**	DWC$k_db_normal	    -- Information retreived
**	DWC$k_db_failure    -- General failure; please check errno (and
**			       possibly vaxc$errno).
**
**  SIDE EFFECTS:
**
**	none
**--
**/
{

    VM_record *Submap_vm;			/* Submap (VM) with attrs   */
    struct DWCDB_subrangemap *Submap;		/* Real submap		    */
    int Dayi;					/* Day index within Submap  */
    int Day_count;				/* Number of info days	    */
    int Output_index;				/* Index in output vectors  */
    int Base_day;				/* Starting day in 1:st Smap*/
    int Days_left;				/* Number of days to copy   */
    int i;					/* Loop variable	    */
    int week_day;				/* Day of week		    */
    int temp_daymark;				/* Temp daymark (work)	    */
    int saved_bits;				/* Saved bits before qualif */
    
    /*
    **  Remember where we were, in case of error
    */
    _Set_cause(DWC$_DAYATGF);

    /*
    **  Calculate the number of days for which information is to be retreived
    **	and initialize the output vectors to default values.
    */
    Day_count = (End_day - Start_day) + 1;
    memset(Daymark_vec, DWC$k_day_default, Day_count);
    memset(Dayflag_vec, 0, Day_count);
    memset(Dayused_vec, DWC$k_use_empty, Day_count);

    /*
    **  First, retreive information from the first subrangemap in which the
    **	requested range resides. This is a bit of a special case, since the
    **	entire requested range may reside in this first map or only a part of
    **	it. In any event, we may have to start looking at data at a nonzero
    **	index in this subrangemap.
    */
    Output_index = 0;
    Base_day = (Start_day % DWC$k_db_submap_entries);
    Days_left = DWC$k_db_submap_entries - Base_day;
    if (Days_left > Day_count) Days_left = Day_count;
    if (DWC$$DB_Locate_submap(Cab, Start_day, &Submap_vm, FALSE))
	{
	Submap = _Bind(Submap_vm, DWCDB_subrangemap);
	for (i=Base_day; i<(Base_day+Days_left); i++)
	    {
	    Daymark_vec[Output_index] = Submap->DWC$b_dbsu_daymarks[i];
	    Dayflag_vec[Output_index] =
		    (Submap->DWC$b_dbsu_dayflags[i] & DWC$m_dbsu_userflags);
	    if (Submap->DWC$l_dbsu_dayvec[i] != 0)
		{
		Dayused_vec[Output_index] = DWC$k_use_normal;
		if (!(Submap->DWC$b_dbsu_dayflags[i] & DWC$m_dbsu_insignif))
		    {
		    Dayused_vec[Output_index] = DWC$k_use_significant;
		    }
		}
	    Output_index++;
	    }
	}

    /*
    **  Is there more stuff to check?
    */
    if (Days_left != Day_count)
	{

	/*
	**  There is more. Set the output index properly, since we do not
	**	know if it got updated by the above loop or not.
	*/
	Output_index = Days_left;

	/*
	**  Enter the main retreival loop. We will not exit this loop until we've
	**	come to the end of the requested range.
	*/
	while (TRUE)
	    {

	    /*
	    **  Compute the number of days we shall retreive from this subrange
	    **  map. We either get everything or up to the end of the request
	    */
	    Days_left = DWC$k_db_submap_entries;
	    if (Day_count < (Days_left + Output_index))
		{
		Days_left = Day_count - Output_index;
		}

	    /*
	    **  Get the subrangemap and pass data back to caller, if any.
	    */
	    if (DWC$$DB_Locate_submap(  Cab, Start_day + Output_index,
					&Submap_vm, FALSE))
		{
		Submap = _Bind(Submap_vm, DWCDB_subrangemap);
		for (i=0; i<Days_left; i++)
		    {
		    Daymark_vec[Output_index] = Submap->DWC$b_dbsu_daymarks[i];
		    Dayflag_vec[Output_index] =
			(Submap->DWC$b_dbsu_dayflags[i] & DWC$m_dbsu_userflags);
		    if (Submap->DWC$l_dbsu_dayvec[i] != 0)
			{
			Dayused_vec[Output_index] = DWC$k_use_normal;
			if (!(Submap->DWC$b_dbsu_dayflags[i] & DWC$m_dbsu_insignif))
			    {
			    Dayused_vec[Output_index] = DWC$k_use_significant;
			    }
			}
		    Output_index++;
		    }
		}
	    else
		Output_index = Output_index + Days_left;

	    /*
	    **  Have we returned all? If so, exit this loop.
	    */
	    if (Output_index == Day_count) break;
	    }
	}

    /*
    **  Tidy up vectors
    */
    for (i=0; i<Day_count; i++)
	{

	/*
	**  Check empty days for repeat expressions.
	*/
	switch (Dayused_vec[i])
	    {
	    case DWC$k_use_empty :
		{
		Dayused_vec[i] = DWC$$DB_Check_day(Cab, Start_day + i,
						    DWC$k_use_empty);
		break;
		}
	    case DWC$k_use_normal :
		{
		if (DWC$$DB_Check_day(Cab, Start_day + i,
			DWC$k_use_significant) == DWC$k_use_significant)
		    {
		    Dayused_vec[i] = DWC$k_use_significant;
		    }
		break;
		}
	    case DWC$k_use_significant :
		{
		break;
		}
	    }

	/*
	**  Convert old style daymark to new
	*/
	week_day = ((Start_day + i + 5) % 7);
	temp_daymark = Daymark_vec[i];
	if (((temp_daymark & DWC$m_mark_type_mask) != DWC$k_mark_default) &&
	    (!(temp_daymark & DWC$m_mark_new_style)))
	    {
	    temp_daymark = DWC$l_mark_cvt_tab[temp_daymark];
	    }

	/*
	**  Qualify default into work or non-working day
	*/
	if ((temp_daymark & DWC$m_mark_type_mask) == DWC$k_day_default)
	    {
	    saved_bits = temp_daymark & ~DWC$m_mark_type_mask;
	    if ((1 << week_day) & Cab->DWC$b_dcab_workweek)
		{
		temp_daymark = DWC$k_day_workday | saved_bits |
				    DWC$m_day_defaulted;
		}
	    else
		{
		temp_daymark = DWC$k_day_nonwork | saved_bits |
				    DWC$m_day_defaulted;
		}
	    }
	Daymark_vec[i] = temp_daymark;
	
	}
    
    /*
    **  We're done.
    */
    _Pop_cause;
    return (DWC$k_db_normal);
}    

int
DWC$$DB_Truncate_day
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	VM_record *Daymap_vm,
	int Day_num)
#else	/* no prototypes */
	(Cab, Daymap_vm, Day_num)
	struct DWC$db_access_block	*Cab;
	VM_record *Daymap_vm;
	int Day_num;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine truncates a day if it is empty. This means that if the
**	input day contains no items it is removed. Furthermore, if the owning
**	subrangemap is empty, too, it will also be removed. Removed in this
**	case means that the record will be deleted.
**	
**	This routine does not attempt to delete the Day extension, since,
**	by definition, if the day is empty is shall contain no extension.
**
**	If the day is truncated the routines does clear the current day
**	context.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Daymap_vm :	Daymap to perform empty check on.
**	Day_num :	DWC day to truncate
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
**	TRUE	    - Day truncated.
**	FALSE	    - Nothing was truncated.
**
**  SIDE EFFECTS:
**
**	none
**--
**/
{

    VM_record *Submap_vm;			/* Work pointer to submap   */
    char **Day_pat;				/* Patch for daymap VM	    */
    char **Sub_pat;				/* Patch for subrangemap VM */
    struct DWCDB_subrangemap *Submap;		/* Real subrangemap	    */
    int Day_idx;				/* Index of day to truncate */

    
    /*
    **  Check if day is empty. If so, proceed with further tests
    */
    if (DWC$$DB_Is_daymap_empty(Cab, Daymap_vm))
	{

	/*
	**  Point at subrangemap. Clear and save backlink to Submap's
	**  so that we can perform an ''empty'' test on the submap too.
	*/
	Submap_vm = (VM_record *)Daymap_vm->DWC$a_dbvm_special;
	Day_pat = Daymap_vm->DWC$a_dbvm_parent_vm;
	*Day_pat = 0;
	
	/*
	**  Clear the insignificance bit.
	*/
	Submap = _Bind(Submap_vm, DWCDB_subrangemap);
	Day_idx = (Day_num % DWC$k_db_submap_entries);
	Submap->DWC$b_dbsu_dayflags[Day_idx] =
	    (Submap->DWC$b_dbsu_dayflags[Day_idx] & DWC$m_dbsu_userflags);
	
	/*
	**  Is associated subrangemap empty?
	**
	*/	
	if (DWC$$DB_Is_subrangemap_empty(Cab, Submap_vm))
	    {

	    /*
	    **  Yes. Write out the associated Rangemap with zero pointer to
	    **	subrangemap and flush pending buffers.
	    */
	    Sub_pat = Submap_vm->DWC$a_dbvm_parent_vm;
	    *Sub_pat = 0;
	    DWC$$DB_Write_virtual_record
		(Cab, (VM_record *) Submap_vm->DWC$a_dbvm_special);
	    _Write_back(Cab);

	    /*
	    **  Delete both Daymap and Subrangemap
	    */
	    DWC$$DB_Deallocate_records(Cab, 1, Daymap_vm->DWC$l_dbvm_rec_addr);
	    DWC$$DB_Release_buffer(Cab, Daymap_vm);
	    DWC$$DB_Deallocate_records(Cab, 1, Submap_vm->DWC$l_dbvm_rec_addr);
	    DWC$$DB_Freelist_buffer(Cab, Submap_vm);

	    /*
	    **  Clear backlink from in Rangemap since it was restored by
	    **	the free-buffer code
	    */
	    *Sub_pat = 0;
	    
	    }
	else
	    {

	    /*
	    **  Nope. This Subrangemap still contains information. Write
	    **	it out, with the new zero Daymap link.
	    */
	    DWC$$DB_Write_virtual_record
		(Cab, (VM_record *) Daymap_vm->DWC$a_dbvm_special);
	    _Write_back(Cab);

	    /*
	    **  Deallocate daymap
	    */
	    DWC$$DB_Deallocate_records(Cab, 1, Daymap_vm->DWC$l_dbvm_rec_addr);
	    DWC$$DB_Release_buffer(Cab, Daymap_vm);

	    /*
	    **  Clear backlink to subrangemap
	    */
	    *Day_pat = 0;
	    	    
	    }

	/*
	**  Clear day context, since it is probably no longer valid
	*/
	DWC$$DB_Clear_day_context(Cab);
	
	/*
	**  Indicate that something was truncated
	*/
	return (TRUE);
	}

    /*
    ** Indicate that nothing was truncated.
    */
    return (FALSE);
}

int
DWC$DB_Set_workweek
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block  *Cab,
	int Work_mask)
#else	/* no prototypes */
	(Cab, Work_mask)
	struct DWC$db_access_block  *Cab;
	int Work_mask;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine declares which days are working days in the week.
**	The declaration is used when determining whether a default day
**	is a working day during evaluation of repeat expressions. The
**	data passed to this routine is not passed on to the profile. You
**	will have to call this routine each time you open a database
**	and each time the working week changes.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Work_mask :	Bit mask. One bit for each working day; bit 0 is
**			Sunday. TRUE is working day. FALSE means
**			non-working day.
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
**	DWC$k_db_normal	    -- Work week declared
**
**  SIDE EFFECTS:
**
**	none
**--
**/
{

    /*
    **  Save default working week in Cab
    */
    Cab->DWC$b_dcab_workweek = (Work_mask & 127);

    /*
    **  Back to caller
    */
    return (DWC$k_db_normal);
}

int
DWC$$DB_Get_daytype
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block  *Cab,
	int Day)
#else	/* no prototypes */
	(Cab, Day)
	struct DWC$db_access_block  *Cab;
	int Day;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine retreives the day type for a given day. If the
**	day number is outside the range of what has been established
**	with DWC$$DB_Set_centerpoint, the routine will signal an error.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Day :		Day for which day type is to be returned
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION VALUD:
**
**	Day type of day (for example, DWC$k_day_workday)
**
**  SIDE EFFECTS:
**
**	none
**--
**/
{

    int offset;			/* Offset within cache vector	*/
    

    /*
    **  Make sure that cache is valid
    */
    if (Cab->DWC$l_dcab_flags & DWC$m_dcab_dtcvalid)
	{
	
	/*
	**  Compute offset within vector for daytype
	*/
	offset = Day - Cab->DWC$l_dcab_dtbase;
	if (offset < ((DWC$k_db_submap_entries * 3)))
	    {
	    
	    /*
	    **  Extract day type and return it to caller
	    */
	    return (Cab->DWC$b_dcab_dtcache[offset]);
	    }
	}	    
	
    /*
    **  We should not get here. The only reason why we would get here is
    **  because of something being wrong with the cache (invalid or
    **  requested day out of range). Signal fatal error.
    */
    _Signal(DWC$_DTCBAD);
    return (DWC$k_db_failure);
}

int
DWC$$DB_Set_centerpoint
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block  *Cab,
	int Day)
#else	/* no prototypes */
	(Cab, Day)
	struct DWC$db_access_block  *Cab;
	int Day;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine defines a new centerpoint for cached day attributes.
**	The cache stores the day type for 3 consecutive subrangemaps. Calling
**	this routine will ensure that the input day is located in the
**	middle one. This ensures that there is cached information for
**	the "Day" +/- the size of a subrangemap. The current subrangemap
**	holds more days than can be found in any month. Thus routine will
**	therefore ensure that there is day information for Day +/- 1 Month.
**
**	The routine will load subrangemaps, if needed. Default days will
**	be refined into (non)working days by using the data declared in
**	call to DWC$$DB_Set_workweek.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	Day :		Day that should be present in center map
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
**	DWC$k_db_normal	    -- Cache rebuilt
**
**  SIDE EFFECTS:
**
**	none
**--
**/
{

    int new_base;		/* Desired base day		*/
    int offset;			/* submaps shift offset		*/
    int i;			/* Local loop variable		*/
    
    
    /*
    **  Determine desired starting point for first cached map
    */
    new_base = (Day - DWC$k_db_submap_entries);
    new_base = new_base - (new_base % DWC$k_db_submap_entries);

    /*
    **  Is the cache valid?
    */
    if (Cab->DWC$l_dcab_flags & DWC$m_dcab_dtcvalid)
	{

	/*
	**  Are we Ok with the base?
	*/
	if (new_base == Cab->DWC$l_dcab_dtbase)
	    {

	    /*
	    **  Yes. Back to caller in success
	    */
	    return (DWC$k_db_normal);
	    }

	/*
	**  Where is the new center relative to the old one? We need to know
	**  so that we can salvage any data (and thereby avoid disk I/Os).
	*/
	if (new_base < Cab->DWC$l_dcab_dtbase)
	    {

	    /*
	    **  Determine number of records to shift RIGHT and then reload
	    */
	    offset = ((Cab->DWC$l_dcab_dtbase - new_base) /
			DWC$k_db_submap_entries);
	    Cab->DWC$l_dcab_dtbase = new_base;
	    switch (offset)
		{
		case 1 :
		    {
		    for (   i=(DWC$k_db_submap_entries * 3) - 1;
			    i>=(DWC$k_db_submap_entries); i--)
			{
			Cab->DWC$b_dcab_dtcache[i] =
			  Cab->DWC$b_dcab_dtcache[i - DWC$k_db_submap_entries];
		        }
		    DWC$$DB_Reload_daytype(Cab, new_base, 1);
		    break;
		    }
		case 2 :
		    {
		    for (   i=(DWC$k_db_submap_entries * 3) - 1;
			    i>=(DWC$k_db_submap_entries * 2); i--)
			{
			Cab->DWC$b_dcab_dtcache[i] =
			  Cab->DWC$b_dcab_dtcache[i - (
				    DWC$k_db_submap_entries * 2)];
		        }
		    DWC$$DB_Reload_daytype(Cab, new_base, 2);
		    break;
		    }
		default :
		    {
		    DWC$$DB_Reload_daytype(Cab, new_base, 3);
		    break;
		    }
		}
	    }
	else

	    {
	    /*
	    **  Determine number of records to shift LEFT and then reload
	    */
	    offset = ((new_base - Cab->DWC$l_dcab_dtbase) /
			DWC$k_db_submap_entries);
	    Cab->DWC$l_dcab_dtbase = new_base;
	    switch (offset)
		{
		case 1 :
		    {
		    for (   i=0;
			    i<(DWC$k_db_submap_entries * 2);
			    i++)
			{
			Cab->DWC$b_dcab_dtcache[i] =
			  Cab->DWC$b_dcab_dtcache[i + DWC$k_db_submap_entries];
		        }
		    DWC$$DB_Reload_daytype( Cab,
					    new_base +
						(DWC$k_db_submap_entries * 2),
					    1);
		    break;
		    }
		case 2 :
		    {
		    for (   i=0;
			    i<DWC$k_db_submap_entries;
			    i++)
			{
			Cab->DWC$b_dcab_dtcache[i] =
			  Cab->DWC$b_dcab_dtcache[i +
					(DWC$k_db_submap_entries * 2)];
		        }
		    DWC$$DB_Reload_daytype( Cab,
					    new_base + DWC$k_db_submap_entries,
					    2);
		    break;
		    }
		default :
		    {
		    DWC$$DB_Reload_daytype(Cab, new_base, 3);
		    break;
		    }
		}
	    }
	}
    else
	{

	/*
	**  Cache not valid. Reload all and mark cache as valid.
	*/
	Cab->DWC$l_dcab_dtbase = new_base;
	DWC$$DB_Reload_daytype(Cab, new_base, 3);
	Cab->DWC$l_dcab_flags = (Cab->DWC$l_dcab_flags | DWC$m_dcab_dtcvalid);
	}

    /*
    **  Done, back to caller
    */
    return (DWC$k_db_normal);
}

int
DWC$$DB_Reload_daytype
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block  *Cab,
	int start_day,
	int count)
#else	/* no prototypes */
	(Cab, start_day, count)
	struct DWC$db_access_block  *Cab;
	int start_day;
	int count;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine rebuilds the cache of daytype for a specified number
**	of subrangemaps. The data is brought in from disk (if available).
**	Default days are converted into their real value by using caller's
**	input to DWC$$DB_Set_workweek.
**	
**  FORMAL PARAMETERS:
**
**	Cab :		Pointer to DWC$db_access_block
**	start_day :	First day (aligned with submap) where to start
**			the load
**	count :		number of submaps to load (1..3)
**
**  IMPLICIT INPUTS:
**
**      Cab->DWC$l_dcab_dtbase	-- set to desired base
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	DWC$k_db_normal	    -- Cache rebuilt
**
**  SIDE EFFECTS:
**
**	none
**--
**/
{

    int i;					/* loop variable	    */
    int j;					/* Second loop variable	    */
    int week_day;				/* Day of week		    */
    int s_i;					/* Start index for copy	    */
    VM_record *Submap_vm;			/* Submap (VM) with attrs   */
    struct DWCDB_subrangemap *Submap;		/* Real submap		    */
    int Dayi;					/* Day index within Submap  */
    int offset;					/* Dayoffset from startday  */
    
    /*
    **  Enter load loop
    */
    for (i=0; i<count; i++)
	{

	/*
	**  Determine start index
	*/
	offset = (i * DWC$k_db_submap_entries);
	s_i = (start_day - Cab->DWC$l_dcab_dtbase) + offset;

	/*
	**  Attempt to load subrangemap for specified day
	*/
	if (DWC$$DB_Locate_submap(  Cab, start_day + offset, &Submap_vm, FALSE))
	    {

	    /*
	    **  Map is there, copy daytype into work vector
	    */
	    Submap = _Bind(Submap_vm, DWCDB_subrangemap);
	    memcpy( &Cab->DWC$b_dcab_dtcache[s_i],
		    &Submap->DWC$b_dbsu_daymarks[0],
		    DWC$k_db_submap_entries);
	    }
	else
	    {

	    /*
	    **  Submap is not there, fill vector with default values
	    */
	    memset( &Cab->DWC$b_dcab_dtcache[s_i],
		    DWC$k_day_default,
		    DWC$k_db_submap_entries);
	    }

	/*
	**  Convert from old to new style and qualify default values
	*/
	for (j=0; j<DWC$k_db_submap_entries; j++)
	    {

	    /*
	    **  Convert from old style to new, if needed
	    */
	    Cab->DWC$b_dcab_dtcache[j + s_i] =
		(Cab->DWC$b_dcab_dtcache[j + s_i] & DWC$m_mark_type_mask);
	    if (Cab->DWC$b_dcab_dtcache[j + s_i] == DWC$k_mark_special)
		{
		Cab->DWC$b_dcab_dtcache[j + s_i] = DWC$k_day_default;
		}
		
	    /*
	    **  Turn default into real value
	    */
	    if (Cab->DWC$b_dcab_dtcache[j + s_i] == DWC$k_day_default)
		{
		
		/*
		**  This day has default value. Determine weekday, where
		**  base 0 is Sunday. The "5" is used, since day 0 is
		**  a Friday.
		*/
		week_day = ((start_day + j + 5) % 7);
		if ((1 << week_day) & Cab->DWC$b_dcab_workweek)
		    {
		    Cab->DWC$b_dcab_dtcache[j + s_i] = DWC$k_day_workday &
					    DWC$m_mark_type_mask;
		    }
		else
		    {
		    Cab->DWC$b_dcab_dtcache[j + s_i] = DWC$k_day_nonwork &
				    DWC$m_mark_type_mask;
		    }
		}
	    }
	}
	
    /*
    **  All done, back to caller
    */
    return (DWC$k_db_normal);
}

/*
**  End of Module
*/
