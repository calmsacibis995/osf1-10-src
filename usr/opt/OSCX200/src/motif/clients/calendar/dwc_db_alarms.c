/* dwc_db_alarms.c */
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
**	Per Hamnqvist, March 1988
**
**  ABSTRACT:
**
**	This module implements the management of in-memory alarms, used to
**	determine (by the running calendar) when alarms are to be triggered.
**
**--
*/

/*
**  Include Files
*/
#include "dwc_compat.h"
#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"

void
DWC$$DB_Compute_alarm_time
#ifdef	_DWC_PROTO_
	(
	struct DWCDB_entry *Entry,
	unsigned int Org_day,
	unsigned short int Alarm_offset,
	unsigned int (*Trigg_day),
	unsigned short int (*Trigg_min))
#else	/* no prototypes */
	(Entry, Org_day, Alarm_offset, Trigg_day, Trigg_min)
	struct DWCDB_entry *Entry;
	unsigned int Org_day;
	unsigned short int Alarm_offset;
	unsigned int (*Trigg_day);
	unsigned short int (*Trigg_min);
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine computes the absolute triggering time for a specific
**	alarm associated with a specific day entry.
**
**  FORMAL PARAMETERS:
**
**	Entry : Entry for which time is to be computed
**	Org_day : Day in which this entry belongs
**	Alarm_offset : Alarm offset (positive delta minutes, from the time
**		       of the entry itself).
**	Trigg_day : Day in which alarm will trigger (by ref)
**	Trigg_min : Minute (within day) in which alarm will trigger (by ref)
**	
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
**--
**/
{	

    unsigned short int temp_time;	/* Temp alarm work time	*/
    
    /*
    **  Is the triggering time within the same day?
    */
    if (Alarm_offset > Entry->DWC$w_dben_start_minute)
	{

	/*
	**  No. Compute new day
	*/

	/*
	*Trigg_day =
	    Org_day - ((Alarm_offset + (DWC$k_db_calendar_precision - 1))
		    / DWC$k_db_calendar_precision);
	*/

	*Trigg_day = ( Org_day*DWC$k_db_calendar_precision +
	    Entry->DWC$w_dben_start_minute - Alarm_offset )
	    / DWC$k_db_calendar_precision;

	/*
	**  Compute new minute. This requires a bit of work since the
	**  Alarm_offset can be more than 24 hours.
	*/
	temp_time = (Alarm_offset % DWC$k_db_calendar_precision);
	if (temp_time > Entry->DWC$w_dben_start_minute)
	    {
	    *Trigg_min = 
		DWC$k_db_calendar_precision + Entry->DWC$w_dben_start_minute -
		    temp_time;
	    }
	else
	    {
	    *Trigg_min = Entry->DWC$w_dben_start_minute - temp_time;
	    }
	}
    else
	{

	/*
	**  Same day. Use originating day and compute triggering minute
	*/
	*Trigg_day = Org_day;
	*Trigg_min = Entry->DWC$w_dben_start_minute - Alarm_offset;
	}
}

int
DWC$$DB_Compare_times
#ifdef	_DWC_PROTO_
	(
	unsigned int Day_1,
	unsigned short int Min_1,
	unsigned int Day_2,
	unsigned short int Min_2)
#else	/* no prototypes */
	(Day_1, Min_1, Day_2, Min_2)
	unsigned int Day_1;
	unsigned short int Min_1;
	unsigned int Day_2;
	unsigned short int Min_2;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine compares DWC time #1 with DWC time #2 and returns the
**	result of the comparision to caller.
**
**  FORMAL PARAMETERS:
**
**	Day_1 : Day of time #1
**	Min_1 : Minute of time #1
**	Day_2 : Day of time #2
**	Min_2 : Minute of time #2
**	
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
**      1   - Time #1 is greater than time #2
**	0   - Time #1 is equal to time #2
**     -1   - Time #1 is less than time #2
**
**  SIDE EFFECTS:
**
**      none
**--
**/
{

    /*
    **  Very straightforward compare
    */
    if (Day_1 < Day_2) return (-1);
    if (Day_1 > Day_2) return (1);
    if (Min_1 < Min_2) return (-1);
    if (Min_1 > Min_2) return (1);
    return (0);
}

int
DWC$$DB_Insert_alarm_item
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	VM_record   *Item_VM,
	int	    Org_day)
#else	/* no prototypes */
	(Cab, Item_VM, Org_day)
	struct DWC$db_access_block	*Cab;
	VM_record   *Item_VM;
	int	    Org_day;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine computes the alarm time(s) associated with the input Day
**	entry and creates the appropriate entries in the in-memory alarm queue.
**	The routine will not create in-memory entries for alarms under
**	the following circumstances:
**
**	    1. The alarm is triggering before the most recent time
**	       specified in call to DWC$DB_Get_next_alarm_time
**	    2. The owning day is past the "lookahead" depth (which is
**	       determined by the max time in advance an alarm can be set for
**	       and the most recent time specified in call to
**	       DWC$DB_Get_next_alarm_time).
**	    3. The in-memory alarm database has not yet been activated.
**	    4. The alarm is already there. This can only happen for alarms
**	       associated with repeating entries that trigger on days with
**	       no hard entries on. In those cases, the alarm is inserted
**	       into the queue when the queue is first built, but the
**	       associated repeat instance is deleted along with its stubday
**	       when current day changes. When the caller later tries to
**	       re-reference that day, the repeat instance is re-inserted
**	       and the code also tries to re-insert the alarms for it!!
**	    5. The input entry isn't a dayentry.
**	    6. The input entry doesn't have any alarms associated with it.
**
**	The queue of pending alarms is sorted primarily by ascending triggering
**	time and secondarily by descending originating time. This means that
**	the first entry in the queue is to be triggered before the last. And,
**	when the triggering time is the same for two entries, the entry bound
**	to time furthest out will come before the one close in time (this
**	allows the more urgent ones to come "on top" of the other windows
**	rather than being occluded).
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by reference
**	Item_VM : Day entry, by reference, which is to be inserted into queue
**	Org_day : Originating day of this entry
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
**	TRUE	    - Alarm added
**	FALSE	    - Failed to place item in queue
**
**  SIDE EFFECTS:
**
**	A) Re-initializes the current Get_item context.
**	B) Re-initializes the current Get_next_alarm_id context.
**--
**/
{

    unsigned short int *Alarm_offsets;		/* Vector of alarms	    */
    unsigned int Num_alarms_for_tlwk_entry;	/* Count of alarms	    */
    struct DWCDB_entry *Entry;			/* Work entry		    */
    struct DWC$db_alarm *Next_alarm;		/* New alarm block	    */
    unsigned int Trigg_day;			/* Day in which alarm triggs*/
    unsigned short int Trigg_min;		/* Minute		    */
    unsigned short int i;			/* General loop variable    */
    struct DWC$db_alarm *Alarm;			/* Work pointer to alarm    */
    int Cmp_sts;				/* Status from time compare */
    int insert_alarm;				/* Boolean: Insert or not   */

    /*
    **  If the alarm database is not active then we don't even try to record
    **	the alarms. This can be the case if items are added but no calls
    **	have been made to DWC$DB_Get_next_alarm_time.
    */
    if (!(Cab->DWC$l_dcab_flags & DWC$m_dcab_alarm_ld))
    {
	return (TRUE);
    }

    /*
    **  Alarms cannot be assigned to anything by Dayentries.
    */
    if (Item_VM->DWC$t_dbvm_data[0] != DWC$k_dbit_entry)
    {
	return (TRUE);
    }
    /*
    **  The job is very easy if the input entry does not have any alarms
    **	associated with it.
    */
    Entry = _Bind(Item_VM, DWCDB_entry);
    if (!(Entry->DWC$b_dben_flags & DWC$m_dben_alarm_present))
    {
	return (TRUE);
    }

    /*
    **  Setup work parameters and start realizing the alarms associated with
    **	this entry.
    */
    Num_alarms_for_tlwk_entry = Entry->DWC$t_dben_data[0];

    /*  Get the list of alarm_offsets (positive delta minutes, from the	    */
    /*	time of the entry itself */
    Alarm_offsets = (unsigned short int *)&(Entry->DWC$t_dben_data[1]);

    for (i=0; i<Num_alarms_for_tlwk_entry; i++)
    {

	/*
	**  Compute when this specific alarm is to be triggered (absolute time).
	*/
	DWC$$DB_Compute_alarm_time(Entry, (unsigned int)Org_day,
				    Alarm_offsets[i],
				    &Trigg_day, &Trigg_min);

	/*
	**  We will only record this alarm in memory if it triggers after the
	**  most recent "interest" time and before the end of last day part of
	**  the lookahead
	*/
	if ((Org_day <= Cab->DWC$l_dcab_hi_org_day) &&
	    (DWC$$DB_Compare_times( Trigg_day, Trigg_min,
				    Cab->DWC$l_dcab_alarm_day,
				    Cab->DWC$w_dcab_alarm_min) >= 0))
	{

	    /*
	    **  Ok. This alarm shall be recorded. First, try and recycle an
	    **	alarm buffer. If none, get a new one.
	    */
	    if (!DWC$$DB_Remque
		(
		(struct DWC$db_queue_head *) (&(Cab->DWC$a_dcab_free_alarm_f)),
		(struct DWC$db_queue_head **) (&Next_alarm)
		)
	       )
	    {
		Next_alarm = (struct DWC$db_alarm *)XtMalloc
		    (sizeof(*Next_alarm));
		if (Next_alarm == NULL)
		{
		    _Record_error;
		    _Signal(DWC$_INSVIRMEM);
		    return (FALSE);
		}
	    }

	    /*
	    **  Fill in the alarm block
	    */
	    Next_alarm->DWC$l_dbal_org_day = Org_day;
	    Next_alarm->DWC$w_dbal_org_min = Entry->DWC$w_dben_start_minute;
	    Next_alarm->DWC$w_dbal_id = Entry->DWC$w_dben_entry_id;
	    Next_alarm->DWC$l_dbal_trigg_day = Trigg_day;
	    Next_alarm->DWC$w_dbal_trigg_min = Trigg_min;
	    Next_alarm->DWC$w_dbal_alarm_idx = i;

	    /*
	    **  Find a good place to link this new alarm in. The criteries
	    **	are listed in the routine header.
	    */
	    insert_alarm = TRUE;

	    /* get the first alarm in the alarm queue */
	    Alarm = Cab->DWC$a_dcab_alarm_flink;
	    while (TRUE)
	    {
		/* if the forward link points to the first alarm in the	    */
		/* queue then we're at the end of the alarm queue.  */
		if (Alarm == (struct DWC$db_alarm *)&Cab->DWC$a_dcab_alarm_flink)
		    /* break out to insert our alarm */
		    break;

		/* compare our entry to the one in the alarm queue */
		Cmp_sts = DWC$$DB_Compare_times( Alarm->DWC$l_dbal_trigg_day,
					   Alarm->DWC$w_dbal_trigg_min,
					   Trigg_day, Trigg_min);
	        if (Cmp_sts > 0) break; /* Alarm triggers after Next_alarm */
		if (Cmp_sts == 0) /* They trigger at the same time */
		{
		    while (TRUE)
		    {
                        /*	  
                        **  if this is a repeat entry and the alarm id and the
			**  index are the same as the alarm already in the queue
			**  then don't add it.  How do we get in this situation
			**  in the first place?  Well, when we're initially
			**  called to look through the lookahead, we insert
			**  alarms for the repeats.  If the day only has
			**  repeats, it will disapper as we look ahead but its
			**  alarms will still be in the queue.  Next time
			**  through we'll be trying to insert a duplicate.  Why
			**  does this result in a missing alarm as in QARs 1872
			**  and 98, beats me, but presumably it is because we
			**  don't flush the alarm queue of repeat entry alarms
			**  when we look at another day.
                        */	  
			if ((Next_alarm->DWC$w_dbal_id < 0) &&
			   (Next_alarm->DWC$w_dbal_id == Alarm->DWC$w_dbal_id) &&
			   (Next_alarm->DWC$w_dbal_alarm_idx ==
				Alarm->DWC$w_dbal_alarm_idx))
			{
			    /* Throw away the alarm we had planned on   */
			    /* inserting since it's a duplicate */
			    DWC$$DB_Insque
			    (
				(struct DWC$db_queue_head *) Next_alarm,
				(struct DWC$db_queue_head *)
				    (&Cab->DWC$a_dcab_free_alarm_f)
			    );
					
			    insert_alarm = FALSE;
			    /* Let's fall through to the end of the routine */
			    break;
			}

                        /*	  
			**  Well we know that our new entry and the one on the
			**  queue trigger at the same time.  Let's check to see
			**  which alarm's entry is sooner
                        */	  
			if (DWC$$DB_Compare_times( Alarm->DWC$l_dbal_org_day,
						   Alarm->DWC$w_dbal_org_min,
						   Next_alarm->DWC$l_dbal_org_day,
						   Next_alarm->DWC$w_dbal_org_min
						   ) < 0)
                            /*	  
			    ** The alarm-in-the-queue's entry comes before the
			    ** alarm-we-want-to-insert so let's put the
			    ** alarm-we-want-to-insert in front of it so that
			    ** the alarm-in-the-queue's alarm displays on
			    ** (after) and on top of it.
                            */	  
			    break;

                        /*	  
                        **  The alarm-in-the-queue's entry comes after or at the
			**  same time as the alarm-we-want-to-insert. Get the
			**  next alarm in the queue to look ahead
                        */	  
			Alarm = Alarm->DWC$a_dbal_flink;

			/* if this is the last alarm in the queue then	    */
			/* jump out */
			if (Alarm == (struct DWC$db_alarm *)&Cab->DWC$a_dcab_alarm_flink)
			    break;

			/* if the alarm-in-the-queue isn't at the same time */
			/* as the alarm we want to insert then jump out */
			if (DWC$$DB_Compare_times( Alarm->DWC$l_dbal_trigg_day,
						   Alarm->DWC$w_dbal_trigg_min,
						   Trigg_day, Trigg_min) != 0)
			    break;

                        /*	  
                        **  if the alarm-in-the-queue has a trigger time at the
			**  same time as the alarm-we-want-to-insert then we
			**  need to check it's entry time to see if it is the
			**  same or not.
                        */	  
		    } /* end of Cmp_sts == 0 while */

		    break;
		} /* end of Cmp_sts == 0 if */

		/* We don't know where to fit our entry in yet so keep	    */
		/* walking through the alarm queue */
		Alarm = Alarm->DWC$a_dbal_flink;
	    }

	    /*
	    **  Insert alarm and increment count of active alarms in memory.
	    **  Do also invalidate the alarm context, since we've clearly
	    **  changed the contents of the queue.
	    */
	    if (insert_alarm)
	    {
		DWC$$DB_Insque
		(
		    (struct DWC$db_queue_head *)Next_alarm,
		    (struct DWC$db_queue_head *)Alarm->DWC$a_dbal_blink
		);
		Cab->DWC$l_dcab_alarm_cnt++;
		Cab->DWC$a_dcab_alarm_ptr = 0;
	    }
	}
    }

    /*
    **  All done.
    */
    return (TRUE);
}

int
DWC$$DB_Remove_alarm_item
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	int Day,
	int Id)
#else	/* no prototypes */
	(Cab, Day, Id)
	struct DWC$db_access_block	*Cab;
	int Day;
	int Id;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine removes all in-memory alarm entries associated with
**	a particular day entry.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by reference
**	Day : Day in which item exists
**	Id : Id (within day) of item for which in-memory alarms are to
**	     be removed.
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
**	TRUE	    - Alarm(s) removed
**	FALSE	    - Failed to remove alarm(s) from queue
**
**  SIDE EFFECTS:
**
**	Re-initializes the current Get_next_alarm_id context.
**--
**/
{
    struct DWC$db_alarm *Alarm;			/* Current work alarm	    */
    struct DWC$db_alarm *Next_alarm;		/* Next alarm		    */
	    
    /*
    **  Enter removal loop
    */
    Next_alarm = Cab->DWC$a_dcab_alarm_flink;
    while (TRUE)
	{

	/*
	**  We're done when we're at the end of the queue
	*/
	if (Next_alarm == (struct DWC$db_alarm *)&Cab->DWC$a_dcab_alarm_flink) break;

	/*
	**  Point at current and advance to next. We need to do this before
	**  the alarm is removed, since the Remque/Insque will alter the
	**  Flink field.
	*/
	Alarm = Next_alarm;
	Next_alarm = Alarm->DWC$a_dbal_flink;

	/*
	**  Does this particular alarm belong to the entry we're looking for?
	*/
	if ((Alarm->DWC$l_dbal_org_day == Day) &&
	    (Alarm->DWC$w_dbal_id == Id))
	    {

	    /*
	    **  Yes, remove alarm from alarm queue. Decrement count of
	    **	pending alarms and place this alarm block on the free list (for
	    **	later recycle by DWC$$DB_Insert_alarm_item).
	    */
	    DWC$$DB_Remque
	    (
		(struct DWC$db_queue_head *) Alarm->DWC$a_dbal_blink,
		(struct DWC$db_queue_head **) 0
	    );
	    Cab->DWC$l_dcab_alarm_cnt--;
	    DWC$$DB_Insque
	    (
		(struct DWC$db_queue_head *) Alarm,
		(struct DWC$db_queue_head *) (&Cab->DWC$a_dcab_free_alarm_f)
	    );
	    }

	}

    /*
    **  Clear the alarm context, since it is possible that we removed the one
    **	we we're pointing at.
    */
    Cab->DWC$a_dcab_alarm_ptr = 0;
    return (TRUE);
}

int
DWC$$DB_Release_alarms
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
**	This routine releases all in-memory alarm entries and places
**	them on the free list (for later use).
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by reference
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
**	TRUE	    - Alarm(s) removed
**	FALSE	    - Failed to remove alarm(s) from queue
**
**  SIDE EFFECTS:
**
**	Re-initializes the current Get_next_alarm_id context.
**--
**/
{
    
    struct DWC$db_alarm *Alarm;			/* Work pointer to alarm    */
    
    /*
    **  Save some time if there is no work to be done.
    */

    if (Cab->DWC$l_dcab_flags & DWC$m_dcab_alarm_ld)
	{

	/*
	**  Clear the work context, since we're going to release the alarm it is
	**	pointing at (if any).
	*/
	Cab->DWC$a_dcab_alarm_ptr = 0;

	/*
	**  Move entries from the active queue to the recycle list until the
	**	active list is empty. For each entry removed, decrement count of
	**	alarms pending.
	*/
	while (DWC$$DB_Remque
		(
		(struct DWC$db_queue_head *) (&Cab->DWC$a_dcab_alarm_flink),
		(struct DWC$db_queue_head **) &Alarm
		)
	       )
	    {
	    DWC$$DB_Insque
	    (
		(struct DWC$db_queue_head *) Alarm,
		(struct DWC$db_queue_head *) (&Cab->DWC$a_dcab_free_alarm_f)
	    );
	    Cab->DWC$l_dcab_alarm_cnt--;
	    }

	/*
	**  Indicate that the in-memory database is no longer active (loaded)
	*/
	Cab->DWC$l_dcab_flags = (Cab->DWC$l_dcab_flags &
				    ~DWC$m_dcab_alarm_ld);

	/*
	**  Make sure that the count went down to zero (sanity)
	*/
	if (Cab->DWC$l_dcab_alarm_cnt != 0)
	    {
	    _Signal(DWC$_ALARMINCON);
	    return (FALSE);
	    }
	}
	
    /*
    **  All fine and dandy.
    */
    return (TRUE);
}    

int
DWC$$DB_Unload_alarms
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
**	This routine cleans up the in-memory alarm-database (e.g.,
**	releases all in-memory alarm entries).
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by reference
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
**	TRUE	    - Alarm(s) removed
**	FALSE	    - Failed to remove alarm(s) from queue
**
**  SIDE EFFECTS:
**
**	Re-initializes the current Get_next_alarm_id context.
**--
**/
{
    
    struct DWC$db_alarm *Alarm;		    /* Work pointer to alarm	*/

    /*
    **  Empty queue of active alarms and move them to the free list
    */
    DWC$$DB_Release_alarms(Cab);

    /*
    **  Take off all entries from the free list and release them back to the
    **	CRTL.
    */
    while (
	    DWC$$DB_Remque
	    (
	    (struct DWC$db_queue_head *)(&Cab->DWC$a_dcab_free_alarm_f),
	    (struct DWC$db_queue_head **) &Alarm
	    )
	  )
	{
	XtFree(Alarm);
	}

    /*
    **  Done.
    */
    return (TRUE);
}    

void
DWC$$DB_Adjust_vm_alarms
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	unsigned int Request_day,
	unsigned short int Request_min)
#else	/* no prototypes */
	(Cab, Request_day, Request_min)
	struct DWC$db_access_block	*Cab;
	unsigned int Request_day;
	unsigned short int Request_min;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine adjusts the in-memory alarm database for a new base
**	time. The in-memory database is deep enough that it can handle the
**	longest possible alarm (in advance). The management is optimized
**	for sequential (chronological) access. The routine does the following
**	basic operations:
**
**	    1. If the new base-time is before the most recently specified
**	       base-time, the entire in-memory database is flushed. The only
**	       case where this will happen is when a system manager backs
**	       the system clock on the system on which this calendar session
**	       is running.
**	    2. Entries triggering before the new base-time are purged out,
**	       to release space.
**	    3. New entries are appended at the end of the queue, if needed
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Request_day : New day requested to be base-time
**	Request_min : Minute of day, requested to be base-time
**	
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
**--
**/
{
    struct DWC$db_alarm *Alarm;		/* Current alarm		*/
    struct DWC$db_alarm *Next_alarm;	/* Next alarm			*/
    unsigned int Start_day;		/* Start day for readjustment	*/
    unsigned int End_day;		/* End day for readjustment	*/
    int i;				/* General loop variable	*/
    VM_record *Item_VM;			/* Pointer to Day item		*/
    VM_record *Day_VM;			/* Daymap			*/
    struct DWCDB_item *Item;		/* Item buffer (without VM head)*/


    /*
    **  Flush the alarm queue if repeat context has changed
    */
    if (Cab->DWC$l_dcab_alarm_rctx != Cab->DWC$l_dcab_repeat_ctx)
	{
	DWC$$DB_Release_alarms(Cab);
	Cab->DWC$l_dcab_alarm_rctx = Cab->DWC$l_dcab_repeat_ctx;
	}
    else
	{	

	/*
	**  If alarms are loaded and that the request time is going backwards
	**  then we can treat that as if the system time was set back. Since
	**  our code is optimized for sequential read, empty the database and
	**  get ready for reload.
	*/

	if ((Cab->DWC$l_dcab_flags & DWC$m_dcab_alarm_ld) &&
	    (DWC$$DB_Compare_times(	Request_day, Request_min,
				    Cab->DWC$l_dcab_alarm_day,
				    Cab->DWC$w_dcab_alarm_min) == -1))
	    DWC$$DB_Release_alarms(Cab);
	}

    /*
    **  Record the start "interest" time. This time determines the borders
    **	of the VM copy of the alarms
    */
    Cab->DWC$l_dcab_alarm_day = Request_day;
    Cab->DWC$w_dcab_alarm_min = Request_min;
    
    /*
    **  Determine first day and last day of reload.
    */
    Start_day = Request_day;
    End_day = Request_day + ((Request_min + DWC$k_db_max_alarm_time) /
				DWC$k_db_calendar_precision);

    /*
    **  If the alarm database is currently active them we may to do some
    **	purge behind (as a result of the interest time moving forwards).
    */
    if (Cab->DWC$l_dcab_flags & DWC$m_dcab_alarm_ld)
	{

	/*
	**  Yes. Remove all pending alarms that trigger before the time
	**  specified as the starting "interest" time. Alarm blocks are
	**  placed on the free list for later reuse by DWC$$DB_Insert_alarm_item.
	*/
	Next_alarm = Cab->DWC$a_dcab_alarm_flink;
	while (TRUE)
	    {
	    if (Next_alarm == (struct DWC$db_alarm *)&Cab->DWC$a_dcab_alarm_flink) break;
	    Alarm = Next_alarm;
	    if (DWC$$DB_Compare_times(  Alarm->DWC$l_dbal_trigg_day,
					Alarm->DWC$w_dbal_trigg_min,
					Request_day, Request_min) >= 0) break;
	    Next_alarm = Alarm->DWC$a_dbal_flink;
	    DWC$$DB_Remque
	    (
		(struct DWC$db_queue_head *) (Alarm->DWC$a_dbal_blink),
		(struct DWC$db_queue_head **) 0
	    );
	    Cab->DWC$l_dcab_alarm_cnt--;
	    DWC$$DB_Insque
	    (
		(struct DWC$db_queue_head *) Alarm,
		(struct DWC$db_queue_head *) (&Cab->DWC$a_dcab_free_alarm_f)
	    );
	    }
	Start_day = Cab->DWC$l_dcab_hi_org_day + 1;
	}

    /*
    **  Indicate that the alarm database indeed is active now.
    */
    Cab->DWC$l_dcab_flags = (Cab->DWC$l_dcab_flags | DWC$m_dcab_alarm_ld);

    /*
    **  Do we need to bring in any more days in the future for lookahead?
    */
    if (Start_day <= End_day)
	{

	/*
	**  Yes, record new hi day (this determines the high originating day
	**  limit accepted by DWC$$DB_Insert_alarm_item).
	*/
	Cab->DWC$l_dcab_hi_org_day = End_day;

	/*
	**  Enter load loop (one lap per day).
	*/
	for (i=Start_day; i<=End_day; i++)
	    {

	    /*
	    **	Is there any day data associated with this day? If so, insert
	    **	all alarms (that trigger within our frame of interest)
	    **	associated with day into alarm queue. Please note that alarms
	    **	associated with repeat expressions were inserted indirectly
	    **	by the Locate_day routine.
	    */
	    if (DWC$$DB_Locate_day(Cab, &Day_VM, i, FALSE))
		{
		Item_VM = Day_VM->DWC$a_dbvm_vm_flink;
		while (TRUE)
		    {
		    if (Item_VM == (VM_record *)&Day_VM->DWC$a_dbvm_vm_flink) break;
		    Item = _Bind(Item_VM, DWCDB_item);
		    if (Item->DWC$w_dbit_id > 0)
			{
			DWC$$DB_Insert_alarm_item(Cab, Item_VM, i);
			}
		    Item_VM = Item_VM->DWC$a_dbvm_vm_flink;
		    }
		}
	    }
	}

    /*
    **  We're done. The "next" alarm is now the first one in the queue
    */
    Cab->DWC$a_dcab_alarm_ptr = Cab->DWC$a_dcab_alarm_flink;
    return;
}    

int
DWC$DB_Get_next_alarm_time
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	unsigned int Start_day,
	unsigned short int Start_min,
	unsigned int *Alarm_day,
	unsigned short int (*Alarm_min))
#else	/* no prototypes */
	(Cab, Start_day, Start_min, Alarm_day, Alarm_min)
	struct DWC$db_access_block	*Cab;
	unsigned int Start_day;
	unsigned short int Start_min;
	unsigned int *Alarm_day;
	unsigned short int (*Alarm_min);
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine determines the next time when any alarm is due, starting
**	with a specific time. The time when that first alarm is due is returned.
**	The context for subsequent Get_next_alarm_id is established.
**
**	Please note that the working context of the in-memory alarms is
**	altered whenever any entries are added or removed. Thus, this routine
**	must be called after each batch of calls to Put_item, Modify_item
**	and Delete_item.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Start_day : Day at which to start looking for triggering alarms
**	Start_min : Minute within day at which to start looking for triggering
**		    alarms
**	Alarm_day : Day at which the next alarm is due.
**	Alarm_min : Minute at which the next alarm is due
**	
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
**	DWC$k_db_normal	    -- Next alarm time retreived
**	DWC$k_db_noala	    -- There are no alarms to be triggered between
**			       the requested start time and the longest
**			       time an alarm can be set for (in advance).
**	DWC$k_db_failure    -- General failure; please check errno (and
**			       possibly vaxc$errno).
**
**  SIDE EFFECTS:
**
**      none
**--
**/
{

    struct DWC$db_alarm *Alarm;			/* First alarm in queue	*/

    /*
    **  Establish error context
    */
    _Set_base(DWC$_FGNALT);

    /*
    **  Readjust alarm queue
    */
    DWC$$DB_Adjust_vm_alarms(Cab, Start_day, Start_min);

    /*
    **  Are there any alarms?
    */
    if (Cab->DWC$l_dcab_alarm_cnt == 0)
	{
	
	/*
	**  Nope. Tell caller.
	*/
	*Alarm_day = 0;
	*Alarm_min = 0;
	Cab->DWC$a_dcab_alarm_ptr = 0;
	_Pop_cause;
	return (DWC$k_db_noala);
	}

    /*
    **  Yes, pass back day and time of the first alarm to be triggered.
    */
    Alarm = Cab->DWC$a_dcab_alarm_ptr;	
    *Alarm_day = Alarm->DWC$l_dbal_trigg_day;
    *Alarm_min = Alarm->DWC$w_dbal_trigg_min;
    _Pop_cause;
    return (DWC$k_db_normal);
}

int
DWC$DB_Get_next_alarm_id
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	unsigned int *Day,
	int *Id)
#else	/* no prototypes */
	(Cab, Day, Id)
	struct DWC$db_access_block	*Cab;
	unsigned int *Day;
	int *Id;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine retreives the next alarm alarm to be triggered at the
**	time returned in the most recent call to DWC$DB_Get_next_alarm_time.
**
**	As for DWC$DB_Get_next_alarm_time, the working context of the in-memory
**	alarms is altered whenever any entries are added or removed. Thus,
**	DWC$DB_Get_next_alarm time must be called prior to this after each
**	batch of calls to Put_item, Modify_item and Delete_item.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Day : Day of entry
**	Id : Id of entry
**	
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
**	DWC$k_db_normal	    -- Next id retreived
**	DWC$k_db_nomore	    -- No more IDs at this alarm time
**	DWC$k_db_failure    -- General failure; please check errno (and
**			       possibly vaxc$errno).
**
**  SIDE EFFECTS:
**
**      none
**--
**/
{

    struct DWC$db_alarm *Alarm;		    /* Alarm to be returned	*/
    struct DWC$db_alarm *Next_alarm;	    /* Next alarm in the queue	*/

    /*
    **  If nothing more of interest, tell caller.
    */
    if ((Cab->DWC$a_dcab_alarm_ptr == 0) ||
	(Cab->DWC$a_dcab_alarm_ptr == (struct DWC$db_alarm *)&Cab->DWC$a_dcab_alarm_flink))
	{
	return (DWC$k_db_nomore);
	}

    /*
    **  The next alarm in the queue should be good. Pass parameters back to
    **	caller.
    */
    Alarm = Cab->DWC$a_dcab_alarm_ptr;
    *Day = Alarm->DWC$l_dbal_org_day;
    *Id = Alarm->DWC$w_dbal_id;

    /*
    **  Now, since the queue can change in between calls, we need to
    **	do some lookahead now. Determine if the next alarm in the queue
    **	matches the query criterias too? If not, simulate end of query
    */
    Next_alarm = Alarm->DWC$a_dbal_flink;
    if (Next_alarm != (struct DWC$db_alarm *)&Cab->DWC$a_dcab_alarm_flink)
	{
	if ((Alarm->DWC$l_dbal_trigg_day != Next_alarm->DWC$l_dbal_trigg_day) ||
	    (Alarm->DWC$w_dbal_trigg_min != Next_alarm->DWC$w_dbal_trigg_min))
	    {
	    Next_alarm = (struct DWC$db_alarm *)&Cab->DWC$a_dcab_alarm_flink;
	    }
	}

    /*
    **  Setup for next invokcation and back to caller
    */
    Cab->DWC$a_dcab_alarm_ptr = Next_alarm;
    return (DWC$k_db_normal);
}

/*
**  End of Module
*/
