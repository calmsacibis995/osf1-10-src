/* dwc_db_day_items.c */
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
**	Hein van den Heuvel, Jan 1988
**
**  ABSTRACT:
**
**	This module implements the routines for the top level routines
**	in the database (those that will be called from outside) such
**	as get/put Day items.
**
**--
*/

/*
**  Include Files
*/
#include "dwc_compat.h"
#include <string.h>
#include "dwc_db_public_structures.h"	/* make sure DWC$db_time_ctx exists */
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"

int DWC$DB_Put_r_item
#ifdef _DWC_PROTO_
(
    DWC$CAB	(Cab),
    int				*Item_id,
    int				Start_day,
    int				Start_min,
    int				Duration_days,
    int				Duration_min,
    int				Alarm_vec_len,
    unsigned short int		Alarm_vec[],
    int				Flags,
    int				Input_text_len,
    char			Input_text_ptr[],
    int				Input_text_class,
    int				P1,
    int				P2,
    int				P3,
    int				End_day,
    int				End_min
)
#else
(
    Cab, Item_id, Start_day, Start_min, Duration_days, Duration_min,
    Alarm_vec_len, Alarm_vec, Flags,
    Input_text_len, Input_text_ptr, Input_text_class, P1, P2, P3,
    End_day, End_min
)
struct DWC$db_access_block	*Cab;
int				*Item_id;
int				Start_day;
int				Start_min;
int				Duration_days;
int				Duration_min;
int				Alarm_vec_len;
unsigned short int		Alarm_vec[];
int				Flags;
int				Input_text_len;
char				Input_text_ptr[];
int				Input_text_class;
int				P1;
int				P2;
int				P3;
int				End_day;
int				End_min;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine adds a new item to a day. It can also be used to define
**	a repeat expression. The alarm database is updated.
**
**	It is not recommended to create repeat expressions that start before
**	current time.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Item_id : Caller field to receive id of item just created
**	Start_day : Starting day for this entry or repeat expression
**	Start_min : Starting minute for this entry or repeat expression
**	Duration_days : Length in days of one instansiations
**	Duration_min : Length in minutes (within days) of one instansiation
**	Alarm_vec_len : Number of alarms associated with each instansiation
**	Alarm_vec : Word vector of delta minutes before instansiation to alarm
**	Flags : The following flags are known:
**
**			DWC$m_item_insignif	-- Item is not significant;
**						   please refer to routine
**						   DWC$DB_Get_day_attr for
**						   more details.
**
**	Input_text_length : Number of bytes of text data associated with
**			    item
**	Input_text_ptr : Text string to be associated with item
**	Input_text_class : Type of text: The following types are
**			   recognized:
**
**			DWC$k_item_text		-- 8bit text (MCS); without
**						   trailing NULL
**			DWC$k_item_texti	-- Text and icon special.
**						   The first byte indicates
**						   the number of icons
**						   associated with item.
**						   There is then one byte
**						   for each icon (index). The
**						   actual MCS string
**						   starts after this ASCIC
**					           icon data. Please note
**						   that Input_text_length
**						   is the COMBINED length
**						   of these.
**			DWC$k_item_cstr		-- Compound string special.
**						   same as DWC$k_item_texti
**						   but text is in the form
**						   of a compond string.
**
**	P1 : Repeat parameter #1; see separate description
**	P2 : Repeat parameter #2; see separate description
**	P3 : Repeat parameter #3; see separate description
**	End_day : Ending day for repeat expression (ignored if not). If this
**		  parameter and End_min are both zero then the repeat expression
**		  will be infinite. Please note that this is interpreted as
**		  the starting time of the last instansiation.
**	End_min : Ending minute within Ending day of last instansiation. Please
**		  refer to End_day parameter.
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
**	DWC$k_db_normal	    -- Item added
**	DWC$k_db_insdisk    -- Could not extend file to accomodate item
**	DWC$k_db_dayfull    -- Target day full; item could not be added
**	DWC$k_db_toobig	    -- Too big item
**	DWC$k_db_failure    -- General failure; please check errno (and
**			       possibly vaxc$errno).
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    VM_record *Day_VM;			/* Virtual day for entry	    */
    VM_record *Item_VM;			/* Virtual item			    */
    VM_record *Next_item_VM;		/* Next VM item in scan		    */
    struct DWCDB_day *Day_data;	/* The daymap itself		    */
    struct DWCDB_entry *Entry;		/* Entry buffer, if adding entry    */
    int Item_size;			/* Number of databytes for new item */
    int Item_header_size;		/* Fixed length header (item specif)*/
    char *Item_text;			/* Where to store text in item	    */
    int Alarm_len;			/* Number of bytes of Alarm data    */
    int Status;				/* Return status		    */
    int flush_stat;			/* Status from DB flush		    */


    /*
    **  Setup error context
    */
    _Set_base(DWC$_PUTIFAIL);

    /*
    **  Is this a repeat expression?
    */
    if (P1 != DWC$k_db_none)
    {

	/*
	**  Yes, add repeat and back to caller
	*/
	Status = DWC$$DB_Add_repeat(	Cab,
					Item_id,
					Start_day,
					Start_min,
					End_day,
					End_min,
					Duration_min,
					Alarm_vec_len,
					Alarm_vec,
					Flags,
					Input_text_ptr,
					Input_text_len,
					Input_text_class,
					P1,
					P2,
					P3);
	_Pop_cause;
	return (Status);
	}

    /*
    **  Determine length of work buffer and compute length of alarm data array
    */
    Alarm_len = 0;
    Item_header_size = _Field_offset(Entry,DWC$t_dben_data[0]);
    if (Alarm_vec_len != 0)
	{
	Alarm_len = (Alarm_vec_len * 2) + 1;
	}

    /*
    **  Compute the length of the buffer. Make sure that we do not exceed
    **	the 64Kb limit
    */
    Item_size = Item_header_size + Input_text_len + Alarm_len;
    if (Item_size > DWC$k_db_max_16bit_unsigned)
	{
	_Pop_cause;
	return (DWC$k_db_toobig);
	}
    
    /*
    **  This is not a repeat expression. Locate the target day
    */
    if (!DWC$$DB_Locate_day(Cab, &Day_VM, Start_day, TRUE))
	{
	_Pop_cause;
	return (DWC$k_db_failure);
	}

    /*
    **  Allocate work buffer
    */
    DWC$$DB_Get_free_buffer_z(Cab, Item_size, &Item_VM);

    /*
    **  Fill in the item, both the generic part and then the type specific part
    */
    Day_data = _Bind(Day_VM, DWCDB_day);
    Day_data->DWC$w_dbda_item_idx ++;
    Item_VM->DWC$a_dbvm_special = (char *)Day_VM;
    Entry = _Bind(Item_VM, DWCDB_entry);
    Item_text = &(Item_VM->DWC$t_dbvm_data[Item_header_size]);
    Entry->DWC$b_dben_blocktype = DWC$k_dbit_entry;
    Entry->DWC$w_dben_size = Item_size;
    Entry->DWC$w_dben_entry_id	= Day_data->DWC$w_dbda_item_idx ;
    if (Alarm_len != 0)
	{
	Entry->DWC$b_dben_flags = (Entry->DWC$b_dben_flags
				    | DWC$m_dben_alarm_present);
	Item_text[0] = Alarm_vec_len;
	memcpy(&Item_text[1], Alarm_vec, (Alarm_len - 1));
	}		
    memcpy(&Item_text[Alarm_len], Input_text_ptr, Input_text_len);
    Entry->DWC$w_dben_start_minute = Start_min;
    Entry->DWC$l_dben_delta_days = Duration_days;
    Entry->DWC$w_dben_delta_minutes	= Duration_min;
    Entry->DWC$w_dben_text_class = Input_text_class;	
    if (Flags & DWC$m_item_insignif)
	{
	Entry->DWC$b_dben_flags = (Entry->DWC$b_dben_flags
				    | DWC$m_dben_insignif);
	}	    

    /*
    **  Find the right place to hook up: after the last note,
    **	in the right start time order. Having found the insertion
    **	point, insert and call UNLOAD DAY to write out to disk.
    */
    Next_item_VM = Day_VM->DWC$a_dbvm_vm_flink;
    while (Next_item_VM != (VM_record *)&Day_VM->DWC$a_dbvm_vm_flink)
	{
	Entry = _Bind(Next_item_VM, DWCDB_entry);
	if (Entry->DWC$w_dben_start_minute > Start_min) break;
	Next_item_VM = Next_item_VM->DWC$a_dbvm_vm_flink;
	}
    DWC$$DB_Insque
    (
	(struct DWC$db_queue_head *) Item_VM,
	(struct DWC$db_queue_head *) Next_item_VM->DWC$a_dbvm_vm_blink
    );
    if ((flush_stat = DWC$$DB_Flush_day(Cab, Day_VM, Start_day ))
		    != DWC$k_db_normal)
	{
	_Pop_cause;
	return (flush_stat);
	}
    DWC$$DB_Insert_alarm_item(Cab, Item_VM, Start_day);
    *Item_id = Day_data->DWC$w_dbda_item_idx;
	
    /*
    **  Done, back to caller in success
    */
    _Pop_cause;
    return (DWC$k_db_normal);
}

int DWC$DB_Get_r_item
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block  *Cab,
	int			From_day,
	int			*Next_item_id,
	int			*Rep_start_day,
	int			*Rep_start_min,
	int			*Start_min,
	int			*Duration_days,
	int			*Duration_min,
	int			*Alarm_vec_len,
	unsigned short int	**Alarm_vec,
	int			*Flags,
	int			*Output_text_len,
	char			**Output_text_ptr,
	int			*Output_text_class,
	int			*P1,
	int			*P2,
	int			*P3,
	int			*End_day,
	int			*End_min)
#else	/* no prototypes */
	(Cab, From_day, Next_item_id, Rep_start_day,
	    Rep_start_min, Start_min, Duration_days,
	    Duration_min, Alarm_vec_len, Alarm_vec, Flags,
	    Output_text_len, Output_text_ptr, Output_text_class,
	    P1, P2, P3, End_day, End_min)
	struct DWC$db_access_block  *Cab;
	int			From_day;
	int			*Next_item_id;
	int			*Rep_start_day;
	int			*Rep_start_min;
	int			*Start_min;
	int			*Duration_days;
	int			*Duration_min;
	int			*Alarm_vec_len;
	unsigned short int	**Alarm_vec;
	int			*Flags;
	int			*Output_text_len;
	char			    **Output_text_ptr;
	int			*Output_text_class;
	int			*P1;
	int			*P2;
	int			*P3;
	int			*End_day;
	int			*End_min;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine reads the next item from the specified day. The routine
**	does have its own context and it is thefore possible to read the items
**	assigned to a particular day, once by one.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	From_day : Day in which item appears
**	Next_item_id : Id of next item
**	Rep_start_day : Starting day of repeat expression, if repeated, else
**			the value of From_day.
**	Rep_start_min : Starting minute of repeat expression, if repeated, else
**			the value of Start_min.
**	Start_min : Starting minute for this instansiation
**	Duration_days : Length in days of one instansiations
**	Duration_min : Length in minutes (within days) of one instansiation
**	Alarm_vec_len : Number of alarms associated with each instansiation
**	Alarm_vec : Word vector of delta minutes before instansiation to alarm
**	Flags : The following flags are known:
**
**			DWC$m_item_insignif	-- Item is not significant;
**						   please refer to routine
**						   DWC$DB_Get_day_attr for
**						   more details.
**
**			DWC$m_item_rpos		-- Repeat position value;
**						   can contain one of the
**						   following values:
**			  DWC$k_item_last	-- Last or only entry in
**						   repeat sequence.
**			  DWC$k_item_first	-- First entry in repeat
**						   sequence.
**			  DWC$k_item_middle	-- Neither first nor last
**						   entry.
**
**	Output_text_length : Number of bytes of data associated with item
**	Output_text_ptr : Pointer to text data associated with item. 
**	Output_text_class : Type of text: The following types are
**			   recognized:
**
**			DWC$k_item_text		-- 8bit text (MCS); without
**						   trailing NULL
**			DWC$k_item_texti	-- Text and icon special.
**						   The first byte indicates
**						   the number of icons
**						   associated with item.
**						   There is then one byte
**						   for each icon (index). The
**						   actual MCS string
**						   starts after this ASCIC
**					           icon data. Please note
**						   that Output_text_length
**						   is the COMBINED length
**						   of these.
**			DWC$k_item_cstr		-- Compound string special.
**						   same as DWC$k_item_texti
**						   but text is in the form
**						   of a compond string.
**
**	P1 : Repeat parameter #1; see separate description
**	P2 : Repeat parameter #2; see separate description
**	P3 : Repeat parameter #3; see separate description
**	End_day : Ending day for repeat expression (ignored if not). If this
**		  parameter and End_min are both zero then the repeat expression
**		  will be infinite. Please note that this is interpreted as
**		  the starting time of the last instansiation.
**	End_min : Ending minute within Ending day of last instansiation. Please
**		  refer to End_day parameter.
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
**	DWC$k_db_normal	    -- Item retreived
**	DWC$k_db_nomore	    -- No more items on day
**	DWC$k_db_failure    -- General failure; please check errno (and
**			       possibly vaxc$errno).
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    VM_record *Day_VM;			/* Virtual day for entry	    */
    VM_record *Item_VM;			/* Virtual item			    */
    struct DWCDB_entry *Entry;		/* Entry buffer, if adding entry    */
    VM_record *Rep_blo_vm;		/* Repeat block for rep expr	    */
    struct DWC$db_repeat_control *Repc;	/* Repeat expression control block  */
    struct DWCDB_repeat_expr *Rep_exp;	/* Repeat expression vector entry   */

    /*
    **  Seup error context
    */
    _Set_base(DWC$_GETIFAIL);

    /*
    **	If the Current Day or Current Item vM in the CAB is zero then we start
    **	by finding the day. Otherwise we assume those pointers are correct, and
    **	follow the backpointer to the day structure.
    */
    if (( Cab->DWC$l_dcab_current_day != From_day ) ||
	( Cab->DWC$a_dcab_current_item_vm == 0))
	{
	if (!DWC$$DB_Locate_day(Cab, &Day_VM, From_day, FALSE))
	    {
	    _Pop_cause;
	    return (DWC$k_db_nomore);
	    }
	}

    /*
    **  As this point we have a valid Day header pointer, and a valid
    **  current item pointer. Advance to the next item, check for end and
    **  update the Cab.
    */
    Day_VM = Cab->DWC$a_dcab_current_day_vm;
    Item_VM = Cab->DWC$a_dcab_current_item_vm;
    Item_VM = Item_VM->DWC$a_dbvm_vm_flink;
    if (Item_VM == (VM_record *)&Day_VM->DWC$a_dbvm_vm_flink)
	{
	Cab->DWC$a_dcab_current_item_vm = 0;
	_Pop_cause;
	return (DWC$k_db_nomore);
	}
    Cab->DWC$a_dcab_current_item_vm = Item_VM;
    Entry = _Bind(Item_VM, DWCDB_entry);

    /*
    **  Unpack an entry
    */
    *Next_item_id = Entry->DWC$w_dben_entry_id;
    *Start_min = Entry->DWC$w_dben_start_minute;
    *Duration_min = Entry->DWC$w_dben_delta_minutes;
    *Duration_days = Entry->DWC$l_dben_delta_days;
    *Output_text_class = Entry->DWC$w_dben_text_class;

    if (Entry->DWC$b_dben_flags & DWC$m_dben_insignif)
	{
	*Flags = (DWC$m_item_insignif | Item_VM->DWC$b_dbvm_special3);
	}
    else
	{
	*Flags = Item_VM->DWC$b_dbvm_special3;
	}
	
    if (Entry->DWC$b_dben_flags & DWC$m_dben_alarm_present)
	{
	*Alarm_vec_len = Entry->DWC$t_dben_data[0];
	*(char **)Alarm_vec = &Entry->DWC$t_dben_data[1];
	*Output_text_ptr = &Entry->DWC$t_dben_data[1 + (*Alarm_vec_len * 2)];
	*Output_text_len = Entry->DWC$w_dben_size -
			   _Field_offset(Entry,DWC$t_dben_data[0]) -
			   ((Entry->DWC$t_dben_data[0] * 2) + 1);
	}
    else
	{
	*Alarm_vec_len = 0;
	*Output_text_ptr = (char *)&(Entry->DWC$t_dben_data[0]);
	*Output_text_len = Entry->DWC$w_dben_size -
			    _Field_offset(Entry,DWC$t_dben_data[0]);
	}

    if (Entry->DWC$b_dben_flags & DWC$m_dben_repeat)
	{
	Rep_blo_vm = (VM_record *)Item_VM->DWC$a_dbvm_parent_vm;
	Repc = (struct DWC$db_repeat_control *)Rep_blo_vm->DWC$l_dbvm_special2;
	Rep_exp = Repc->DWC$a_dbrc_vector_vm;
	*Rep_start_day = Rep_exp->DWC$l_dbre_baseday;
	*Rep_start_min = Rep_exp->DWC$w_dbre_basemin;
	*End_day = Rep_exp->DWC$l_dbre_endday;
	*End_min = Rep_exp->DWC$w_dbre_endmin;
	*P1 = Repc->DWC$b_dbrc_type;
	switch (Repc->DWC$b_dbrc_type)
	    {
	    case DWC$k_db_absolute :
		{
		*P2 = Rep_exp->DWC$l_dbre_repeat_interval;
		*P3 = 0;
		break;
		}
	    case DWC$k_db_abscond :
		{
		*P2 = Repc->DWC$l_dbrc_n;
		*P3 = Repc->DWC$w_dbrc_daycond;
		break;
		}
	    case DWC$k_db_nth_day_cwd :
		{
		*P2 = Repc->DWC$b_dbrc_month_int;
		*P3 = Repc->DWC$w_dbrc_daycond |
			(Repc->DWC$l_dbrc_n << DWC$v_cond_day);
		break;
		}
	    default :
		{
		*P2 = Repc->DWC$b_dbrc_month_int;
		*P3 = Repc->DWC$w_dbrc_daycond;
		}
	    }
	}
    else
	{
	*Rep_start_day = From_day;
	*Rep_start_min = Entry->DWC$w_dben_start_minute;
	*P1 = DWC$k_db_none;
	}

    /*
    **  Done, back to caller
    */
    _Pop_cause;
    return (DWC$k_db_normal);

}

int DWC$DB_Modify_r_item
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block  *Cab,
	int			Item_id,
	int			Start_day,
	int			Start_min,
	int			Not_used_1,
	int			Not_used_2,
	int			Duration_days,
	int			Duration_min,
	int			Alarm_vec_len,
	unsigned short int	Alarm_vec[],
	int			Flags,
	int			Input_text_len,
	char			Input_text_ptr[],
	int			Input_text_class,
	int			P1,
	int			P2,
	int			P3,
	int			End_day,
	int			End_min)
#else	/* no prototypes */
	(Cab, Item_id, Start_day, Start_min, Not_used_1,
				 Not_used_2, Duration_days, Duration_min,
				 Alarm_vec_len, Alarm_vec, Flags, Input_text_len,
				 Input_text_ptr, Input_text_class, P1,
				 P2, P3, End_day, End_min)
	struct DWC$db_access_block  *Cab;
	int			Item_id;
	int			Start_day;
	int			Start_min;
	int			Not_used_1;
	int			Not_used_2;
	int			Duration_days;
	int			Duration_min;
	int			Alarm_vec_len;
	unsigned short int	Alarm_vec[];
	int			Flags;
	int			Input_text_len;
	char			    Input_text_ptr[];
	int			Input_text_class;
	int			P1;
	int			P2;
	int			P3;
	int			End_day;
	int			End_min;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine modifies one existing item/expression. Please note that
**	this routine cannot be used to:
**
**	    1. Convert a repeat expression to a non-repeating expression. For
**	       that you should Modify the repeat expression to go up to
**	       min-1 of the last repeat. Then add a non-repeating clone at
**	       this point.
**
**	    2. Convert a non-repeating expression to a repeat expression. The
**	       preferred method for doing this is to "clone" the parameters for
**	       the non-repeating and create a new repeating expression starting
**	       at the same time as the source for the clone. Then you can delete
**	       the non-repeating source.
**
**	It is not recommended to create repeat expressions that start before
**	current time.
**	
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Item_id : Caller field to receive id of item just created
**	Start_day : Starting day for this entry or repeat expression,
**		    input only; cannot be modified.
**	Start_min : Starting minute for this entry or repeat expression
**	Not_used_1 : Currently not used
**	Not_used_2 : Currently not used
**	Duration_days : Length in days of one instansiations
**	Duration_min : Length in minutes (within days) of one instansiation
**	Alarm_vec_len : Number of alarms associated with each instansiation
**	Alarm_vec : Word vector of delta minutes before instansiation to alarm
**	Flags : The following flags are known:
**
**			DWC$m_item_insignif	-- Item is not significant;
**						   please refer to routine
**						   DWC$DB_Get_day_attr for
**						   more details.
**
**	Input_text_length : Number of bytes of text data associated with
**			    item
**	Input_text_ptr : Text string to be associated with item
**	Input_text_class : Type of text: The following types are
**			   recognized:
**
**			DWC$k_item_text		-- 8bit text (MCS); without
**						   trailing NULL
**			DWC$k_item_texti	-- Text and icon special.
**						   The first byte indicates
**						   the number of icons
**						   associated with item.
**						   There is then one byte
**						   for each icon (index). The
**						   actual MCS string
**						   starts after this ASCIC
**					           icon data. Please note
**						   that Input_text_length
**						   is the COMBINED length
**						   of these.
**			DWC$k_item_cstr		-- Compound string special.
**						   same as DWC$k_item_texti
**						   but text is in the form
**						   of a compond string.
**
**	P1 : Repeat parameter #1; see separate description
**	P2 : Repeat parameter #2; see separate description
**	P3 : Repeat parameter #3; see separate description
**	End_day : Ending day for repeat expression (ignored if not). If this
**		  parameter and End_min are both zero then the repeat expression
**		  will be infinite. Please note that this is interpreted as
**		  the starting time of the last instansiation.
**	End_min : Ending minute within Ending day of last instansiation. Please
**		  refer to End_day parameter.
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
**	DWC$k_db_normal	    -- Item modified
**	DWC$k_db_insdisk    -- Could not extend file to accomodate modification
**	DWC$k_db_dayfull    -- Target day full; modification could not be
**			       completed
**	DWC$k_db_nsitem	    -- No such item
**	DWC$k_db_toobig	    -- Item is too big
**	DWC$k_db_failure    -- General failure; please check errno (and
**			       possibly vaxc$errno).
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    VM_record *Day_VM;			/* Virtual day for entry	    */
    VM_record *Item_VM;			/* Virtual item			    */
    VM_record *Next_item_VM;		/* Next VM item in scan		    */
    struct DWCDB_day *Day_data;	/* The daymap itself		    */
    struct DWCDB_entry *Entry;		/* Entry buffer, if adding entry    */
    int Item_size;			/* Number of databytes for new item */
    int Item_header_size;		/* Fixed length header (item specif)*/
    char *Item_text;			/* Where to store text in item	    */
    int Alarm_len;			/* Number of bytes of Alarm data    */
    int Status;				/* Return status		    */
    int flush_stat;			/* Status from Db flush		    */


    /*
    **  Setup error context, in case we fail
    */
    _Set_cause(DWC$_MODIFAIL);

    /*
    **  Is this a repeat expression?
    */
    if (Item_id < 0)
	{

	/*
	**  Yes, use special routine for this and back to caller
	*/
	Status = DWC$$DB_Modify_repeat(	Cab,
					Item_id,
					Start_day,
					Start_min,
					End_day,
					End_min,
					Duration_min,
					Alarm_vec_len,
					Alarm_vec,
					Flags,
					Input_text_ptr,
					Input_text_len,
					Input_text_class,
					P1,
					P2,
					P3);
	_Pop_cause;
	return (Status);
	}

    /*
    **  Determine length of work buffer and compute length of alarm data array
    */
    Alarm_len = 0;
    Item_header_size = _Field_offset(Entry,DWC$t_dben_data[0]);
    if (Alarm_vec_len != 0)
	{
	Alarm_len = (Alarm_vec_len * 2) + 1;
	}

    /*
    **  Compute the length of the buffer that we need and then make sure that
    **	it is not longer than 64Kb
    */
    Item_size = Item_header_size + Input_text_len + Alarm_len;
    if (Item_size > DWC$k_db_max_16bit_unsigned)
	{
	_Pop_cause;
	return (DWC$k_db_toobig);
	}

    /*
    **  Call find_day with the allocation flag set to FALSE. Thus the day
    **  will note be created if it is not there.
    */
    if (!DWC$$DB_Locate_day(Cab, &Day_VM, Start_day, FALSE))
	{
	_Pop_cause;
	return (DWC$k_db_nsitem);
	}

    /*
    **	Find the right item to unhook. Remove it from queue and release it's
    **	buffer.
    */
    Status = DWC$k_db_nsitem;
    Item_VM = Day_VM->DWC$a_dbvm_vm_flink;
    while (Item_VM != (VM_record *)&Day_VM->DWC$a_dbvm_vm_flink)
	{
	Entry = _Bind(Item_VM, DWCDB_entry);
	if (Entry->DWC$w_dben_entry_id == Item_id)
	    {
	    DWC$$DB_Remque
	    (
		(struct DWC$db_queue_head *) Item_VM->DWC$a_dbvm_vm_blink,
		0
	    );
	    DWC$$DB_Freelist_buffer(Cab, Item_VM );
	    Status = DWC$k_db_normal;
	    break;
	    }
	Item_VM = Item_VM->DWC$a_dbvm_vm_flink;
	}

    /*
    **  Remove alarm item and flush the modified day.
    */
    if (Status == DWC$k_db_normal)
	{
	DWC$$DB_Remove_alarm_item(Cab, Start_day, Item_id);
	Status = DWC$$DB_Flush_day(Cab, Day_VM, Start_day);
	}
        
    /*
    **  Back to caller if anyting is wrong
    */
    if (Status != DWC$k_db_normal)
	{
	DWC$$DB_Release_day(Cab, Day_VM);
	_Pop_cause;
	return (Status);
	}
	
    /*
    **  Allocate work buffer
    */
    DWC$$DB_Get_free_buffer_z(Cab, Item_size, &Item_VM);

    /*
    **  Fill in the item, both the generic part and then the type specific part
    */
    Day_data = _Bind(Day_VM, DWCDB_day);
    Entry = _Bind(Item_VM, DWCDB_entry);
    Item_text = &(Item_VM->DWC$t_dbvm_data[Item_header_size]);
    Entry->DWC$b_dben_blocktype = DWC$k_dbit_entry;
    Entry->DWC$w_dben_size = Item_size;
    Entry->DWC$w_dben_entry_id	= Item_id;

    if (Alarm_len != 0)
	{
	Entry->DWC$b_dben_flags = (Entry->DWC$b_dben_flags |
					DWC$m_dben_alarm_present);
	Item_text[0] = Alarm_vec_len;
	memcpy(&Item_text[1], Alarm_vec, (Alarm_len - 1));
	}		
    memcpy(&Item_text[Alarm_len], Input_text_ptr, Input_text_len);
    Entry->DWC$w_dben_start_minute = Start_min;
    Entry->DWC$l_dben_delta_days = Duration_days;
    Entry->DWC$w_dben_delta_minutes	= Duration_min;
    Entry->DWC$w_dben_text_class = Input_text_class;	
    if (Flags & DWC$m_item_insignif)
	{
	Entry->DWC$b_dben_flags = (Entry->DWC$b_dben_flags
				    | DWC$m_dben_insignif);
	}	    

    /*
    **  Find the right place to hook up: after the last note,
    **	in the right start time order. Having found the insertion
    **	point, insert and call UNLOAD DAY to write out to disk.
    */
    Next_item_VM = Day_VM->DWC$a_dbvm_vm_flink;
    while (Next_item_VM != (VM_record *)&Day_VM->DWC$a_dbvm_vm_flink)
	{
	Entry = _Bind(Next_item_VM, DWCDB_entry);
	if (Entry->DWC$w_dben_start_minute > Start_min) break;
	Next_item_VM = Next_item_VM->DWC$a_dbvm_vm_flink;
	}
    DWC$$DB_Insque
    (
	(struct DWC$db_queue_head *) Item_VM,
	(struct DWC$db_queue_head *) Next_item_VM->DWC$a_dbvm_vm_blink
    );
    if ((flush_stat = DWC$$DB_Flush_day(Cab, Day_VM, Start_day ))
	    != DWC$k_db_normal)
	{
	_Pop_cause;
	return (flush_stat);
	}
    DWC$$DB_Insert_alarm_item(Cab, Item_VM, Start_day);
	
    /*
    **  Done, back to caller in success
    */
    _Pop_cause;
    return (DWC$k_db_normal);
}

int
DWC$DB_Delete_r_item
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block  *Cab,
	int			Item_id,
	int			Start_day,
	int			Delete_all)
#else	/* no prototypes */
	(Cab, Item_id, Start_day, Delete_all)
	struct DWC$db_access_block  *Cab;
	int			Item_id;
	int			Start_day;
	int			Delete_all;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine deletes one repeat expression or one item. To delete
**	one instansiation of a repeat expression, specify the Id of the repeat
**	but indicate Curr_day = 0. This will except Start_day.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Item_id : Id of item to delete
**	Start_day : Day in which item starts or in which one instansiation of
**		    repeat expression is located
**	Delete_all : Boolean; TRUE if to delete all instances of repeat
**		     else instance at Start_day
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
**	DWC$k_db_normal	    -- Item deleted
**	DWC$k_db_insdisk    -- Could not extend file for "safe" space
**			       when deleting
**	DWC$k_db_nsitem	    -- No such item
**	DWC$k_db_failure    -- General failure; please check errno (and
**			       possibly vaxc$errno).
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    VM_record *Day_VM;			/* Virtual day for entry	    */
    VM_record *Item_VM;			/* Virtual item			    */
    struct DWCDB_day *Day_data;	/* The daymap itself		    */
    struct DWCDB_item *Item;		/* Item wihout VM header	    */
    int Status;				/* Return status		    */


    /*
    **  Setup error context
    */
    _Set_base(DWC$_DELIFAIL);

    /*
    **  Shall we delete a repeat expression?
    */
    Status = DWC$k_db_normal;
    if (Item_id < 0)
	{

	/*
	**  Yes. remove repeat expression and back to caller
	*/
	if (!Delete_all)
	    {
	    if (DWC$$DB_Locate_day(Cab, &Day_VM, Start_day, FALSE))
		{
		Status = DWC$$DB_Add_exception_day(Cab, Item_id, Start_day);
		if (Status == DWC$k_db_normal)
		    {
		    if (DWC$$DB_Remove_specific_expr(Cab, Item_id, Day_VM))
			{
			DWC$$DB_Clear_day_context(Cab);
			}
		    }
		}
	    }
	else
	    {
	    Status = DWC$$DB_Delete_repeat(Cab, Item_id);
	    }
	_Pop_cause;
	return (Status);
	}

    /*
    **  Call find_day with the allocation flag set to FALSE. Thus the day
    **  will note be created if it is not there.
    */
    if (!DWC$$DB_Locate_day(Cab, &Day_VM, Start_day, FALSE))
	{
	_Pop_cause;
	return (DWC$k_db_nsitem);
	}
    /*
    **	Find the right item to unhook. Also count the number of bytes required
    **	for the storage on disk of result.  Having found the deletion point,
    **	remove and continue looping through all items to establish the total
    **	size.  Make sure NOT to count the item to be deleted.
    */
    Status = DWC$k_db_nsitem;
    Item_VM = Day_VM->DWC$a_dbvm_vm_flink;
    while (Item_VM != (VM_record *)&Day_VM->DWC$a_dbvm_vm_flink)
	{
	Item = _Bind(Item_VM, DWCDB_item);
	if (Item->DWC$w_dbit_id == Item_id)
	    {
	    DWC$$DB_Remque
	    (
		(struct DWC$db_queue_head *) Item_VM->DWC$a_dbvm_vm_blink,
		0
	    );
	    DWC$$DB_Freelist_buffer(Cab, Item_VM );
	    Status = DWC$k_db_normal;
	    break;
	    }
	Item_VM = Item_VM->DWC$a_dbvm_vm_flink;
	}

    /*
    **	If everything is ok this far and caller did request that information
    **	shall be flushed back to disk attempt truncating the day. If this
    **	failed (meaing there was nothing to truncate -- there is still data in
    **	this day) attempt to flush back the maps.
    */
    if (Status == DWC$k_db_normal)
	{
	DWC$$DB_Remove_alarm_item(Cab, Start_day, Item_id);
	if (!DWC$$DB_Truncate_day(Cab, Day_VM, Start_day))
	    {
	    if (Status = (DWC$$DB_Flush_day(Cab, Day_VM, Start_day)
			    == DWC$k_db_normal))
		{
		/*
		**  Remerge repeats to uncover hidden ones
		*/
		Day_VM->DWC$l_dbvm_special2 = 0;
		if (!DWC$$DB_Insert_repeats(Cab, Day_VM, Start_day))
		    {
		    Status = DWC$k_db_failure;
		    }
		}
	    }
	}
    _Pop_cause;
    return (Status);

}

int DWC$DB_Get_specific_r_item
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block  *Cab,
	int			From_day,
	int			Item_id,
	int			*Rep_start_day,
	int			*Rep_start_min,
	int			*Start_min,
	int			*Duration_days,
	int			*Duration_min,
	int			*Alarm_vec_len,
	unsigned short int	**Alarm_vec,
	int			*Flags,
	int			*Output_text_len,
	char			    **Output_text_ptr,
	int			*Output_text_class,
	int			*P1,
	int			*P2,
	int			*P3,
	int			*End_day,
	int			*End_min)
#else	/* no prototypes */
	(Cab, From_day, Item_id, Rep_start_day,
	   Rep_start_min, Start_min, Duration_days,
	   Duration_min, Alarm_vec_len, Alarm_vec,
	   Flags, Output_text_len, Output_text_ptr,
	   Output_text_class, P1, P2, P3,
	   End_day, End_min)
	struct DWC$db_access_block  *Cab;
	int			From_day;
	int			Item_id;
	int			*Rep_start_day;
	int			*Rep_start_min;
	int			*Start_min;
	int			*Duration_days;
	int			*Duration_min;
	int			*Alarm_vec_len;
	unsigned short int	**Alarm_vec;
	int			*Flags;
	int			*Output_text_len;
	char			    **Output_text_ptr;
	int			*Output_text_class;
	int			*P1;
	int			*P2;
	int			*P3;
	int			*End_day;
	int			*End_min;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine retreives a specific item on a specific day, without
**	changing the current Get_item context.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	From_day : Day in which item appears
**	Item_id : Id of item to retreive
**	Rep_start_day : Starting day of repeat expression, if repeated, else
**			the value of From_day.
**	Rep_start_min : Starting minute of repeat expression, if repeated, else
**			the value of Start_min.
**	Start_min : Starting minute for this instansiation
**	Duration_days : Length in days of one instansiations
**	Duration_min : Length in minutes (within days) of one instansiation
**	Alarm_vec_len : Number of alarms associated with each instansiation
**	Alarm_vec : Word vector of delta minutes before instansiation to alarm
**	Flags : The following flags are known:
**
**			DWC$m_item_insignif	-- Item is not significant;
**						   please refer to routine
**						   DWC$DB_Get_day_attr for
**						   more details.
**
**			DWC$m_item_rpos		-- Repeat position value;
**						   can contain one of the
**						   following values:
**			  DWC$k_item_last	-- Last or only entry in
**						   repeat sequence.
**			  DWC$k_item_first	-- First entry in repeat
**						   sequence.
**			  DWC$k_item_middle	-- Neither first nor last
**						   entry.
**
**	Output_text_length : Number of bytes of data associated with item
**	Output_text_ptr : Pointer to text data associated with item. 
**	Output_text_class : Type of text: The following types are
**			   recognized:
**
**			DWC$k_item_text		-- 8bit text (MCS); without
**						   trailing NULL
**			DWC$k_item_texti	-- Text and icon special.
**						   The first byte indicates
**						   the number of icons
**						   associated with item.
**						   There is then one byte
**						   for each icon (index). The
**						   actual MCS string
**						   starts after this ASCIC
**					           icon data. Please note
**						   that Output_text_length
**						   is the COMBINED length
**						   of these.
**			DWC$k_item_cstr		-- Compound string special.
**						   same as DWC$k_item_texti
**						   but text is in the form
**						   of a compond string.
**
**	P1 : Repeat parameter #1; see separate description
**	P2 : Repeat parameter #2; see separate description
**	P3 : Repeat parameter #3; see separate description
**	End_day : Ending day for repeat expression (ignored if not). If this
**		  parameter and End_min are both zero then the repeat expression
**		  will be infinite. Please note that this is interpreted as
**		  the starting time of the last instansiation.
**	End_min : Ending minute within Ending day of last instansiation. Please
**		  refer to End_day parameter.
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
**	DWC$k_db_normal	    -- Item retreived
**	DWC$k_db_nsitem	    -- No such item on day
**	DWC$k_db_failure    -- General failure; please check errno (and
**			       possibly vaxc$errno).
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    
    VM_record *Day_VM;			/* Virtual day for entry	    */
    VM_record *Item_VM;			/* Virtual item			    */
    struct DWCDB_entry *Entry;		/* Entry buffer, if adding entry    */
    VM_record *Rep_blo_vm;		/* Repeat block for rep expr	    */
    struct DWC$db_repeat_control *Repc;	/* Repeat expression control block  */
    struct DWCDB_repeat_expr *Rep_exp;	/* Repeat expression vector entry   */
    VM_record *Old_day_vm;		/* Saved day context		    */
    int Old_day;			/* Saved day number		    */
    VM_record *Old_item_vm;		/* Save item context		    */
    VM_record *Sub_VM;			/* Virtual subrangemap		    */
    int Alarm_len;			/* Number of bytes of alarm data    */
    

    /*
    **  Seup error context
    */
    _Set_base(DWC$_GETIFAIL);

    /*
    **  Save old context, if any
    */
    Old_day_vm = Cab->DWC$a_dcab_current_day_vm;
    Old_day = Cab->DWC$l_dcab_current_day;
    Old_item_vm = Cab->DWC$a_dcab_current_item_vm;

    /*
    **	If there is a day item context, different from the one we are about to
    **	retreive, force it to the front of the VMS cache so that it will not be
    **	purged out in case we need to look for the requested day. This action
    **	should not be taken if the current daymap is a stub.
    */
    if (((Day_VM = Cab->DWC$a_dcab_current_day_vm) != 0) &&
	(Cab->DWC$l_dcab_current_day != From_day) &&
	(Day_VM->DWC$l_dbvm_rec_addr != 0))
	{
	DWC$$DB_Touch_buffer(Cab, Day_VM);
	Sub_VM = (VM_record *)Day_VM->DWC$a_dbvm_special;
	DWC$$DB_Touch_buffer (Cab, Sub_VM);
	DWC$$DB_Touch_buffer (Cab, (VM_record *)Sub_VM->DWC$a_dbvm_special);
	}

    /*
    **  Get target day and context loaded
    */
    if (( Cab->DWC$l_dcab_current_day != From_day ) ||
	( Cab->DWC$a_dcab_current_item_vm == 0))
	{
	Cab->DWC$a_dcab_current_day_vm = 0;
	if (!DWC$$DB_Locate_day(Cab, &Day_VM, From_day, FALSE))
	    {
	    Cab->DWC$a_dcab_current_day_vm = Old_day_vm;
	    Cab->DWC$l_dcab_current_day = Old_day;
	    Cab->DWC$a_dcab_current_item_vm = Old_item_vm;
	    _Pop_cause;
	    return (DWC$k_db_nsitem);
	    }
	}

    /*
    **  Scan for specificed item
    */
    Day_VM = Cab->DWC$a_dcab_current_day_vm;
    Item_VM = Day_VM->DWC$a_dbvm_vm_flink;
    while (TRUE)
	{
	if (Item_VM == (VM_record *)&Day_VM->DWC$a_dbvm_vm_flink)
	    {
	    if (Day_VM != Old_day_vm)
		{
		DWC$$DB_Fry_stub_day(Cab, Day_VM);
		}
	    Cab->DWC$a_dcab_current_day_vm = Old_day_vm;
	    Cab->DWC$l_dcab_current_day = Old_day;
	    Cab->DWC$a_dcab_current_item_vm = Old_item_vm;
	    _Pop_cause;
	    return (DWC$k_db_nsitem);
	    }
	Entry = _Bind(Item_VM, DWCDB_entry);
	if (Item_id == Entry->DWC$w_dben_entry_id) break;
	Item_VM = Item_VM->DWC$a_dbvm_vm_flink;
	}

    /*
    **  Restore old day context
    */
    Cab->DWC$a_dcab_current_day_vm = Old_day_vm;
    Cab->DWC$l_dcab_current_day = Old_day;
    Cab->DWC$a_dcab_current_item_vm = Old_item_vm;

    /*
    **  Unpack an entry
    */
    *Start_min = Entry->DWC$w_dben_start_minute;
    *Duration_min = Entry->DWC$w_dben_delta_minutes;
    *Duration_days = Entry->DWC$l_dben_delta_days;
    *Output_text_class = Entry->DWC$w_dben_text_class;

    if (Entry->DWC$b_dben_flags & DWC$m_dben_insignif)
	{
	*Flags = (DWC$m_item_insignif | Item_VM->DWC$b_dbvm_special3);
	}
    else
	{
	*Flags = Item_VM->DWC$b_dbvm_special3;
	}
	
    if (Entry->DWC$b_dben_flags & DWC$m_dben_alarm_present)
	{

	Alarm_len = (Entry->DWC$t_dben_data[0] * 2) + 1;
	*Alarm_vec_len = Entry->DWC$t_dben_data[0];
	*Output_text_len = Entry->DWC$w_dben_size -
			   _Field_offset(Entry,DWC$t_dben_data[0]) -
			   ((Entry->DWC$t_dben_data[0] * 2) + 1);
	}
    else
	{
	Alarm_len = 0;
	*Alarm_vec_len = 0;
	*Output_text_len = Entry->DWC$w_dben_size -
			    _Field_offset(Entry,DWC$t_dben_data[0]);
	}

    if (Entry->DWC$b_dben_flags & DWC$m_dben_repeat)
	{
	Rep_blo_vm = (VM_record *)Item_VM->DWC$a_dbvm_parent_vm;
	Repc = (struct DWC$db_repeat_control *)Rep_blo_vm->DWC$l_dbvm_special2;
	Rep_exp = Repc->DWC$a_dbrc_vector_vm;
	*Rep_start_day = Rep_exp->DWC$l_dbre_baseday;
	*Rep_start_min = Rep_exp->DWC$w_dbre_basemin;
	*End_day = Rep_exp->DWC$l_dbre_endday;
	*End_min = Rep_exp->DWC$w_dbre_endmin;
	*P1 = Repc->DWC$b_dbrc_type;
	switch (Repc->DWC$b_dbrc_type)
	    {
	    case DWC$k_db_absolute :
		{
		*P2 = Rep_exp->DWC$l_dbre_repeat_interval;
		*P3 = 0;
		break;
		}
	    case DWC$k_db_abscond :
		{
		*P2 = Repc->DWC$l_dbrc_n;
		*P3 = Repc->DWC$w_dbrc_daycond;
		break;
		}
	    case DWC$k_db_nth_day_cwd :
		{
		*P2 = Repc->DWC$b_dbrc_month_int;
		*P3 = Repc->DWC$w_dbrc_daycond |
			(Repc->DWC$l_dbrc_n << DWC$v_cond_day);
		break;
		}
	    default :
		{
		*P2 = Repc->DWC$b_dbrc_month_int;
		*P3 = Repc->DWC$w_dbrc_daycond;
		}
	    }
	}
    else
	{
	*Rep_start_day = From_day;
	*Rep_start_min = Entry->DWC$w_dben_start_minute;
	*P1 = DWC$k_db_none;
	}

    /*
    **  Save string and alarm vector in safe place before we fry stub
    **  day
    */
    DWC$$DB_Fry_temp_arrays(Cab);
    if (*Alarm_vec_len != 0)
	{
	if ((Cab->DWC$a_dcab_temp_alarm =
	       (char *)XtMalloc(Alarm_len - 1)) == 0)
	    {
	    _Record_error;
	    _Signal(DWC$_INSVIRMEM);
	    }
	memcpy(Cab->DWC$a_dcab_temp_alarm, &Entry->DWC$t_dben_data[1],
		Alarm_len - 1);
	*(char **)Alarm_vec = Cab->DWC$a_dcab_temp_alarm;
	}
    if (*Output_text_len != 0)
	{
	if ((Cab->DWC$a_dcab_temp_text =
		(char *)XtMalloc(*Output_text_len)) == 0)
	    {
	    _Record_error;
	    _Signal(DWC$_INSVIRMEM);
	    }
	memcpy(Cab->DWC$a_dcab_temp_text,
	    &Entry->DWC$t_dben_data[Alarm_len],
	    *Output_text_len);
	*Output_text_ptr = Cab->DWC$a_dcab_temp_text;
	}

    /*
    **  Get rid of temp daymap
    */
    if (Day_VM != Old_day_vm)
	{
	DWC$$DB_Fry_stub_day(Cab, Day_VM);
	}

    /*
    **  Done, back to caller
    */
    _Pop_cause;
    return (DWC$k_db_normal);
}

int
DWC$DB_Check_overlap
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block  *Cab,
	unsigned long Day,
	int Start_time,
	int Duration_minutes,
	int *Over_id)
#else	/* no prototypes */
	(Cab, Day, Start_time, Duration_minutes, Over_id)
	struct DWC$db_access_block  *Cab;
	unsigned long Day;
	int Start_time;
	int Duration_minutes;
	int *Over_id;
#endif	/* prototypes */

/*
**++
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine checks if the specified timeslot, in a given day, does or
**	does not have any entries assigned to it (e.g., is specified timeslot
**	still unused?).
**
**	The test assumes that the slot to be checked does not cross midnight.
**
**	The Get_item context is preserved.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Start_time : Starting minute for slot to check
**	Duration_minutes : Length of slot to be checked
**	Over_id : Id of the first item that is assigned to the specified
**	          timeslot, if any. By ref.
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
**	DWC$k_db_normal	    -- No overlap
**	DWC$k_db_overlap    -- Something is assigned to specified timeslot.
**			       Over_id contains the Id (with the day just
**			       checked) of the first item within the
**			       specified timeslot.
**	DWC$k_db_failure    -- General failure; please check errno (and
**			       possibly vaxc$errno).
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    VM_record *Day_VM;			/* Virtual day for entry	    */
    VM_record *Item_VM;			/* Virtual item			    */
    struct DWCDB_entry *Item;		/* Item wihout VM header	    */
    VM_record *Old_day_vm;		/* Saved day context		    */
    int Old_day;			/* Saved day number		    */
    VM_record *Old_item_vm;		/* Save item context		    */
    VM_record *Sub_VM;			/* Submap VM pointer		    */
    int Stat;				/* Work status			    */
    int End_minute;			/* Last minute (absolute)	    */


    /*
    **  Setup error context
    */
    _Set_base(DWC$_OCFAIL);

    /*
    **  Save old context, if any
    */
    Old_day_vm = Cab->DWC$a_dcab_current_day_vm;
    Old_day = Cab->DWC$l_dcab_current_day;
    Old_item_vm = Cab->DWC$a_dcab_current_item_vm;

    /*
    **	If there is a day item context, different from the one we are about to
    **	retreive, force it to the front of the VMS cache so that it will not be
    **	purged out in case we need to look for the requested day. This action
    **	should not be taken if the current daymap is a stub.
    */
    if (((Day_VM = Cab->DWC$a_dcab_current_day_vm) != 0) &&
	(Cab->DWC$l_dcab_current_day != Day) &&
	(Day_VM->DWC$l_dbvm_rec_addr != 0))
	{
	DWC$$DB_Touch_buffer(Cab, Day_VM);
	Sub_VM = (VM_record *)Day_VM->DWC$a_dbvm_special;
	DWC$$DB_Touch_buffer(Cab, Sub_VM);
	DWC$$DB_Touch_buffer(Cab, (VM_record *) Sub_VM->DWC$a_dbvm_special);
	}


    /*
    **  Get target day and context loaded
    */
    if (( Cab->DWC$l_dcab_current_day != Day ) ||
	( Cab->DWC$a_dcab_current_item_vm == 0))
	{
	Cab->DWC$a_dcab_current_day_vm = 0;
	if (!DWC$$DB_Locate_day(Cab, &Day_VM, Day, FALSE))
	    {
	    Cab->DWC$a_dcab_current_day_vm = Old_day_vm;
	    Cab->DWC$l_dcab_current_day = Old_day;
	    Cab->DWC$a_dcab_current_item_vm = Old_item_vm;
	    _Pop_cause;
	    return (DWC$k_db_normal);
	    }
	}

    /*
    **  Scan for overlapping item(s). Fail if day is empty or if we've
    **  gone too far.
    */
    Day_VM = Cab->DWC$a_dcab_current_day_vm;
    Item_VM = Day_VM->DWC$a_dbvm_vm_flink;
    End_minute = Start_time + Duration_minutes - 1;
    Stat = DWC$k_db_normal;
    while (TRUE)
	{
	if (Item_VM == (VM_record *)&Day_VM->DWC$a_dbvm_vm_flink)
	    {
	    break;
	    }
	Item = _Bind(Item_VM, DWCDB_entry);
	if (Start_time < Item->DWC$w_dben_start_minute)
	    {
	    if (End_minute >= Item->DWC$w_dben_start_minute)
		{
		*Over_id = Item->DWC$w_dben_entry_id;
		Stat = DWC$k_db_overlap;
		break;
		}
	    }
	else
	    {
	    if (Start_time < (Item->DWC$w_dben_start_minute +
			      Item->DWC$w_dben_delta_minutes))
		{
		*Over_id = Item->DWC$w_dben_entry_id;
		Stat = DWC$k_db_overlap;
		break;
		}
	    else
		{
		break;
		}
	    }
	Item_VM = Item_VM->DWC$a_dbvm_vm_flink;
	}
    /*
    **  Restore old day context
    */
    Cab->DWC$a_dcab_current_day_vm = Old_day_vm;
    Cab->DWC$l_dcab_current_day = Old_day;
    Cab->DWC$a_dcab_current_item_vm = Old_item_vm;

    /*
    **  Get rid of Stub, if any
    */
    if (Day_VM != Old_day_vm)
	{
	DWC$$DB_Fry_stub_day(Cab, Day_VM);
	}

    /*
    **  Back to caller
    */
    _Pop_cause;
    return (Stat);
	
}	

int
DWC$DB_Examine_r_params
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block  *Cab,
	int Item_id,
	int *P4,
	int *P5,
	int *P6)
#else	/* no prototypes */
	(Cab, Item_id, P4, P5, P6)
	struct DWC$db_access_block  *Cab;
	int Item_id;
	int *P4;
	int *P5;
	int *P6;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine examines the additional (read-only) repeat parameters
**	for an expression. The item being examined must be the instance
**	of a repeat expression.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Item_id : Id of repeat to examine
**	P4 : Parameter P4
**	P5 : Parameter P5
**	P6 : Parameter P6
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
**	DWC$k_db_normal	    -- Parameters examined
**	DWC$k_db_failure    -- General failure; please check errno (and
**			       possibly vaxc$errno).
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    
    struct DWC$db_repeat_control *Repc;	    /* Work repeat control block    */
    int Temp_status;			    /* Temp work status		    */
    
    /*
    **  Setup context, in case we fail
    */
    _Set_base(DWC$_EVALREP);
    
    /*
    **  Locate the expression
    */
    if ((Temp_status = DWC$$DB_Find_repeat(Cab, Item_id, &Repc)) !=
		DWC$k_db_normal)
	{
	_Pop_cause;
	return (Temp_status);
	}

    /*
    **  Dispatch on expression type and examine params
    */
    *P6 = 0;
    switch (Repc->DWC$b_dbrc_type)
	{

	/*
	**  By n:th day of month (from start or end) + conditional weekday
	*/
	case DWC$k_db_nth_day :
	case DWC$k_db_nth_day_end :
	    {
	    *P4 = Repc->DWC$b_dbrc_base_month;
	    *P5 = Repc->DWC$l_dbrc_n;
	    break;
	    }

	/*
	**  By n:th x:day in month
	*/
	case DWC$k_db_nth_xday :
	    {
	    *P4 = Repc->DWC$b_dbrc_base_month;
	    *P5 = Repc->DWC$b_dbrc_weekday;
	    *P6 = Repc->DWC$l_dbrc_n;
	    break;
	    }	    

	/*
	**  Last weekday in month
	*/
	case DWC$k_db_last_weekday :
	    {
	    *P4 = Repc->DWC$b_dbrc_base_month;
	    *P5 = Repc->DWC$b_dbrc_weekday;
	    break;
	    }	    

	/*
	**  n:th day of month conditional weekday
	*/
	case DWC$k_db_nth_day_cwd :
	    {
	    *P4 = Repc->DWC$b_dbrc_base_month;
	    *P5 = Repc->DWC$b_dbrc_weekday;
	    break;
	    }
	/*
	**  Unknown, no repeat, absolute and day of certain type
	*/
	default :
	    {
	    *P4 = 0;
	    *P5 = 0;
	    break;
	    }
	}

    /*
    **  All done, back to caller
    */
    _Pop_cause;
    return (DWC$k_db_normal);
}

int
DWC$DB_Evaluate_r_params
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block  *Cab,
	int P1,
	int Day,
	int *P4,
	int *P5,
	int *P6)
#else	/* no prototypes */
	(Cab, P1, Day, P4, P5, P6)
	struct DWC$db_access_block  *Cab;
	int P1;
	int Day;
	int *P4;
	int *P5;
	int *P6;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine evaluates the additional repeat parameters that would
**	be used for a repeat expression of type P1 on day Day.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	P1 : Repeat type
**	Day : Day on which to evaluate parameters
**	P4 : Parameter P4
**	P5 : Parameter P5
**	P6 : Parameter P6
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
**	DWC$k_db_normal	    -- Parameters evaluated
**	DWC$k_db_notrigger  -- a) Specified repeat of type DWC$k_db_last_weekday
**			          does not trigger on this day.
**			       b) Specified repeat of type DWC$k_db_nth_xday
**				  generates an 'n' greater than 4.
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    struct DWC$db_time_ctx Timec;   /* Time context for evaluate    */
    int stat;			    /* Return status		    */
    
    /*
    **  Build time context for evaluate
    */
    DWC$DB_Build_time_ctx(Cab, Day, 0, &Timec);

    /*
    **  Dispatch on expression type and evaluate params
    */
    stat = DWC$k_db_normal;
    *P6 = 0;
    switch (P1)
	{

	/*
	**  By n:th day of month (from start)
	*/
	case DWC$k_db_nth_day :
	    {
	    *P4 = Timec.DWC$b_dbtc_month;
	    *P5 = Timec.DWC$b_dbtc_day;
	    break;
	    }

	/*
	**  By n:th day of month (from end)
	*/
	case DWC$k_db_nth_day_end :
	    {
	    *P4 = Timec.DWC$b_dbtc_month;
	    *P5 = Timec.DWC$b_dbtc_days_in_month - Timec.DWC$b_dbtc_day + 1;
	    break;
	    }	    

	/*
	**  By n:th x:day in month
	*/
	case DWC$k_db_nth_xday :
	    {
	    if (Timec.DWC$b_dbtc_widx > 4)
		{
		stat = DWC$k_db_notrigger;
		}
	    else
		{
		*P4 = Timec.DWC$b_dbtc_month;
		*P5 = Timec.DWC$b_dbtc_weekday;
		*P6 = Timec.DWC$b_dbtc_widx;
		}
	    break;
	    }	    


	/*
	**  n:th day of month, conditional weekday
	*/
	case DWC$k_db_nth_day_cwd :
	    {
	    *P4 = Timec.DWC$b_dbtc_month;
	    *P5 = Timec.DWC$b_dbtc_weekday;
	    break;
	    }

	/*
	**  Last weekday in month. Check to see if this really does trigger.
	*/
	case DWC$k_db_last_weekday :
	    {
	    if (Timec.DWC$b_dbtc_flags & DWC$m_dbtc_last)
		{
		*P4 = Timec.DWC$b_dbtc_month;
		*P5 = Timec.DWC$b_dbtc_weekday;
		}
	    else
		{
		stat = DWC$k_db_notrigger;
		}
	    break;
	    }	    

	/*
	**  Unknown, no repeat, absolute and day of certain type
	*/
	default :
	    {
	    *P4 = 0;
	    *P5 = 0;
	    break;
	    }
	}

    /*
    **  All done, back to caller
    */
    return (stat);
}

int
DWC$DB_Check_trigger
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block  *Cab,
	int Repeat_id,
	unsigned long Day)
#else	/* no prototypes */
	(Cab, Repeat_id, Day)
	struct DWC$db_access_block  *Cab;
	int Repeat_id;
	unsigned long Day;
#endif	/* prototypes */

/*
**++
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine will determine if a given (existing) repeat expression
**	triggers on a given day.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Repeat_id : Id of repeat to check
**	Day : Day on which to check
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
**	DWC$k_db_triggers   -- Repeat does trigger on specified day
**	DWC$k_db_notrigger  -- Repeat does not trigger on specified day
**	DWC$k_db_failure    -- General failure; please check errno (and
**			       possibly vaxc$errno).
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    int stat;			    /* Temp work status		*/


    /*
    **  Declare error base, in case of failure
    */
    _Set_base(DWC$_CNCHKR);

    /*
    **  Check trigger and back to caller
    */
    stat = DWC$$DB_Check_single_expr(Cab, Repeat_id, Day);
    _Pop_cause;
    return (stat);
}


int DWCDB_PutRItem
#ifdef _DWC_PROTO_
(
    DWC$CAB	(Cab),
    int				*Item_id,
    int				Start_day,
    int				Start_min,
    int				Duration_days,
    int				Duration_min,
    int				Alarm_vec_len,
    unsigned short int		Alarm_vec[],
    int				Flags,
    unsigned char		*Input_text_ptr,	/* XmString */
    int				Input_text_class,
    unsigned int		Input_icons_num,
    unsigned char		Input_icon_ptr[],
    int				P1,
    int				P2,
    int				P3,
    int				End_day,
    int				End_min
)
#else
(
    Cab, Item_id, Start_day, Start_min, Duration_days, Duration_min,
    Alarm_vec_len, Alarm_vec, Flags,
    Input_text_ptr, Input_text_class, Input_icons_num, Input_icon_ptr,
    P1, P2, P3,
    End_day, End_min
)
    struct DWC$db_access_block	*Cab;
    int				*Item_id;
    int				Start_day;
    int				Start_min;
    int				Duration_days;
    int				Duration_min;
    int				Alarm_vec_len;
    unsigned short int		Alarm_vec[];
    int				Flags;
    unsigned char		*Input_text_ptr;	/* XmString */
    int				Input_text_class;
    unsigned int		Input_icons_num;
    unsigned char		Input_icon_ptr[];
    int				P1;
    int				P2;
    int				P3;
    int				End_day;
    int				End_min;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine adds a new item to a day. It can also be used to define
**	a repeat expression. The alarm database is updated.
**
**	It is not recommended to create repeat expressions that start before
**	current time.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Item_id : Caller field to receive id of item just created
**	Start_day : Starting day for this entry or repeat expression
**	Start_min : Starting minute for this entry or repeat expression
**	Duration_days : Length in days of one instansiations
**	Duration_min : Length in minutes (within days) of one instansiation
**	Alarm_vec_len : Number of alarms associated with each instansiation
**	Alarm_vec : Word vector of delta minutes before instansiation to alarm
**	Flags : The following flags are known:
**
**			DWC$m_item_insignif	-- Item is not significant;
**						   please refer to routine
**						   DWC$DB_Get_day_attr for
**						   more details.
**
**	Input_text_ptr : Text string to be associated with item
**	Input_text_class : Type of text: The following types are
**			   recognized:
**
**			DWC$k_item_text		-- 8bit text (MCS); without
**						   trailing NULL
**			DWC$k_item_texti	-- Text and icon special.
**						   The first byte indicates
**						   the number of icons
**						   associated with item.
**						   There is then one byte
**						   for each icon (index). The
**						   actual MCS string
**						   starts after this ASCIC
**					           icon data. Please note
**						   that Input_text_length
**						   is the COMBINED length
**						   of these.
**			DWC$k_item_cstr		-- Compound string special.
**						   same as DWC$k_item_texti
**						   but text is in the form
**						   of a compond string.
**
**	Input_icon_ptr : Icon vector to be combined with the string.
**
**	P1 : Repeat parameter #1; see separate description
**	P2 : Repeat parameter #2; see separate description
**	P3 : Repeat parameter #3; see separate description
**	End_day : Ending day for repeat expression (ignored if not). If this
**		  parameter and End_min are both zero then the repeat expression
**		  will be infinite. Please note that this is interpreted as
**		  the starting time of the last instansiation.
**	End_min : Ending minute within Ending day of last instansiation. Please
**		  refer to End_day parameter.
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
**	DWC$k_db_normal	    -- Item added
**	DWC$k_db_insdisk    -- Could not extend file to accomodate item
**	DWC$k_db_dayfull    -- Target day full; item could not be added
**	DWC$k_db_toobig	    -- Too big item
**	DWC$k_db_failure    -- General failure; please check errno (and
**			       possibly vaxc$errno).
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    VM_record *Day_VM;			/* Virtual day for entry	    */
    VM_record *Item_VM;			/* Virtual item			    */
    VM_record *Next_item_VM;		/* Next VM item in scan		    */
    struct DWCDB_day *Day_data;	/* The daymap itself		    */
    struct DWCDB_entry *Entry;		/* Entry buffer, if adding entry    */
    int Item_size;			/* Number of databytes for new item */
    int Item_header_size;		/* Fixed length header (item specif)*/
    char *Item_text;			/* Where to store text in item	    */
    int Alarm_len;			/* Number of bytes of Alarm data    */
    int Status;				/* Return status		    */
    int flush_stat;			/* Status from DB flush		    */
    unsigned char	*xm_text;	/* XmString */
    char		*fc;
    long		byte_count, cvt_status;
    int			icon_len;

    /*
    **  Setup error context
    */
    _Set_base(DWC$_PUTIFAIL);

    /*
    **  Is this a repeat expression?
    */
    if (P1 != DWC$k_db_none)
    {

	/*
	**  Yes, add repeat and back to caller
	*/
	Status = DWCDB__AddRepeat
	(
	    Cab,
	    Item_id,
	    Start_day,
	    Start_min,
	    End_day,
	    End_min,
	    Duration_min,
	    Alarm_vec_len,
	    Alarm_vec,
	    Flags,
	    Input_text_ptr,
	    Input_text_class,
	    Input_icons_num,
	    Input_icon_ptr,
	    P1,
	    P2,
	    P3
	);
	_Pop_cause;
	return (Status);
    }

    /*
    ** For now, all conversions are to fc.  At some point, this will be
    ** replaced with the appropriate target conversions.
    */
    if (Input_text_class != DWC$k_item_cstr)
	fc = (char *)DXmCvtCStoFC (Input_text_ptr, &byte_count, &cvt_status);
    else
	fc = (char *)DXmCvtCStoDDIF (Input_text_ptr, &byte_count, &cvt_status);

    /*
    **  Determine length of work buffer and compute length of alarm data array
    */
    Alarm_len = 0;
    Item_header_size = _Field_offset(Entry,DWC$t_dben_data[0]);
    if (Alarm_vec_len != 0) Alarm_len = (Alarm_vec_len * 2) + 1;

    /*
    **  Compute the length of the buffer. Make sure that we do not exceed
    **	the 64Kb limit
    */
    if (Input_text_class == DWC$k_item_text)
	icon_len = 0;
    else
	icon_len = Input_icons_num + 1;

    Item_size = Item_header_size + byte_count + icon_len + Alarm_len;
    if (Item_size > DWC$k_db_max_16bit_unsigned)
    {
	_Pop_cause;
	return (DWC$k_db_toobig);
    }
    
    /*
    **  This is not a repeat expression. Locate the target day
    */
    if (!DWC$$DB_Locate_day(Cab, &Day_VM, Start_day, TRUE))
    {
	_Pop_cause;
	return (DWC$k_db_failure);
    }

    /*
    **  Allocate work buffer
    */
    DWC$$DB_Get_free_buffer_z(Cab, Item_size, &Item_VM);

    /*
    **  Fill in the item, both the generic part and then the type specific part
    */
    Day_data = _Bind(Day_VM, DWCDB_day);
    Day_data->DWC$w_dbda_item_idx ++;
    Item_VM->DWC$a_dbvm_special = (char *)Day_VM;
    Entry = _Bind(Item_VM, DWCDB_entry);
    Item_text = &(Item_VM->DWC$t_dbvm_data[Item_header_size]);
    Entry->DWC$b_dben_blocktype = DWC$k_dbit_entry;
    Entry->DWC$w_dben_size = Item_size;
    Entry->DWC$w_dben_entry_id	= Day_data->DWC$w_dbda_item_idx ;
    if (Alarm_len != 0)
    {
	Entry->DWC$b_dben_flags = Entry->DWC$b_dben_flags |
	    DWC$m_dben_alarm_present;
	Item_text[0] = Alarm_vec_len;
	memcpy(&Item_text[1], Alarm_vec, (Alarm_len - 1));
    }
    if (Input_text_class != DWC$k_item_text)
    {
	Item_text[Alarm_len] = Input_icons_num;
	if (Input_icons_num != 0)
	{
	    memcpy(&Item_text[Alarm_len + 1], Input_icon_ptr, Input_icons_num);
	}
    }
    memcpy(&Item_text[Alarm_len+icon_len], fc, byte_count);
    Entry->DWC$w_dben_start_minute = Start_min;
    Entry->DWC$l_dben_delta_days = Duration_days;
    Entry->DWC$w_dben_delta_minutes	= Duration_min;
    Entry->DWC$w_dben_text_class = Input_text_class;	
    if (Flags & DWC$m_item_insignif)
    {
	Entry->DWC$b_dben_flags = Entry->DWC$b_dben_flags |
	    DWC$m_dben_insignif;
    }	    

    /*
    **  Find the right place to hook up: after the last note,
    **	in the right start time order. Having found the insertion
    **	point, insert and call UNLOAD DAY to write out to disk.
    */
    Next_item_VM = Day_VM->DWC$a_dbvm_vm_flink;
    while (Next_item_VM != (VM_record *)&Day_VM->DWC$a_dbvm_vm_flink)
    {
	Entry = _Bind(Next_item_VM, DWCDB_entry);
	if (Entry->DWC$w_dben_start_minute > Start_min) break;
	Next_item_VM = Next_item_VM->DWC$a_dbvm_vm_flink;
    }
    DWC$$DB_Insque
    (
	(struct DWC$db_queue_head *) Item_VM,
	(struct DWC$db_queue_head *) Next_item_VM->DWC$a_dbvm_vm_blink
    );

    flush_stat = DWC$$DB_Flush_day (Cab, Day_VM, Start_day);
    if (flush_stat != DWC$k_db_normal)
    {
	_Pop_cause;
	return (flush_stat);
    }
    DWC$$DB_Insert_alarm_item (Cab, Item_VM, Start_day);
    *Item_id = Day_data->DWC$w_dbda_item_idx;
	
    /*
    **  Done, back to caller in success
    */
    _Pop_cause;
    return (DWC$k_db_normal);
}

int DWCDB_GetRItem
#ifdef	_DWC_PROTO_
(
    struct DWC$db_access_block  *Cab,
    int				From_day,
    int				*Next_item_id,
    int				*Rep_start_day,
    int				*Rep_start_min,
    int				*Start_min,
    int				*Duration_days,
    int				*Duration_min,
    int				*Alarm_vec_len,
    unsigned short int		**Alarm_vec,
    int				*Flags,
    unsigned char		**Output_text_ptr,	/* XmString * */
    int				*Output_text_class,
    unsigned int		*Output_icons_num,
    unsigned char		**Output_icon_ptr,
    int				*P1,
    int				*P2,
    int				*P3,
    int				*End_day,
    int				*End_min
)
#else	/* no prototypes */
(
    Cab, From_day, Next_item_id, Rep_start_day,
    Rep_start_min, Start_min, Duration_days,
    Duration_min, Alarm_vec_len, Alarm_vec, Flags,
    Output_text_ptr, Output_text_class, Output_icons_num, Output_icon_ptr,
    P1, P2, P3, End_day, End_min
)
    struct DWC$db_access_block  *Cab;
    int				From_day;
    int				*Next_item_id;
    int				*Rep_start_day;
    int				*Rep_start_min;
    int				*Start_min;
    int				*Duration_days;
    int				*Duration_min;
    int				*Alarm_vec_len;
    unsigned short int		**Alarm_vec;
    int				*Flags;
    unsigned char		**Output_text_ptr;	/* XmString * */
    int				*Output_text_class;
    unsigned int		*Output_icons_num;
    unsigned char		**Output_icon_ptr;
    int				*P1;
    int				*P2;
    int				*P3;
    int				*End_day;
    int				*End_min;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine reads the next item from the specified day. The routine
**	does have its own context and it is thefore possible to read the items
**	assigned to a particular day, once by one.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	From_day : Day in which item appears
**	Next_item_id : Id of next item
**	Rep_start_day : Starting day of repeat expression, if repeated, else
**			the value of From_day.
**	Rep_start_min : Starting minute of repeat expression, if repeated, else
**			the value of Start_min.
**	Start_min : Starting minute for this instansiation
**	Duration_days : Length in days of one instansiations
**	Duration_min : Length in minutes (within days) of one instansiation
**	Alarm_vec_len : Number of alarms associated with each instansiation
**	Alarm_vec : Word vector of delta minutes before instansiation to alarm
**	Flags : The following flags are known:
**
**			DWC$m_item_insignif	-- Item is not significant;
**						   please refer to routine
**						   DWC$DB_Get_day_attr for
**						   more details.
**
**			DWC$m_item_rpos		-- Repeat position value;
**						   can contain one of the
**						   following values:
**			  DWC$k_item_last	-- Last or only entry in
**						   repeat sequence.
**			  DWC$k_item_first	-- First entry in repeat
**						   sequence.
**			  DWC$k_item_middle	-- Neither first nor last
**						   entry.
**
**	Output_text_ptr : Pointer to text data associated with item. 
**	Output_text_class : Type of text: The following types are
**			   recognized:
**
**			DWC$k_item_text		-- 8bit text (MCS); without
**						   trailing NULL
**			DWC$k_item_texti	-- Text and icon special.
**						   The first byte indicates
**						   the number of icons
**						   associated with item.
**						   There is then one byte
**						   for each icon (index). The
**						   actual MCS string
**						   starts after this ASCIC
**					           icon data. Please note
**						   that Output_text_length
**						   is the COMBINED length
**						   of these.
**			DWC$k_item_cstr		-- Compound string special.
**						   same as DWC$k_item_texti
**						   but text is in the form
**						   of a compond string.
**
**	Output_icon_ptr : Pointer to icon data associated with item
**
**	P1 : Repeat parameter #1; see separate description
**	P2 : Repeat parameter #2; see separate description
**	P3 : Repeat parameter #3; see separate description
**	End_day : Ending day for repeat expression (ignored if not). If this
**		  parameter and End_min are both zero then the repeat expression
**		  will be infinite. Please note that this is interpreted as
**		  the starting time of the last instansiation.
**	End_min : Ending minute within Ending day of last instansiation. Please
**		  refer to End_day parameter.
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
**	DWC$k_db_normal	    -- Item retreived
**	DWC$k_db_nomore	    -- No more items on day
**	DWC$k_db_failure    -- General failure; please check errno (and
**			       possibly vaxc$errno).
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    VM_record				*Day_VM;
    VM_record				*Item_VM;
    struct DWCDB_entry			*Entry;
    VM_record				*Rep_blo_vm;
    struct DWC$db_repeat_control	*Repc;
    struct DWCDB_repeat_expr		*Rep_exp;
    unsigned char			*xm_text;	/* XmString */
    static char				*fc;
    static int				fclen = 0;
    long				byte_count, cvt_status;
    int					icon_len;
    int					text_len;
    char				temp_char;
    int					header_len;

    /*
    **  Seup error context
    */
    _Set_base(DWC$_GETIFAIL);

    /*
    **	If the Current Day or Current Item vM in the CAB is zero then we start
    **	by finding the day. Otherwise we assume those pointers are correct, and
    **	follow the backpointer to the day structure.
    */
    if (( Cab->DWC$l_dcab_current_day != From_day ) ||
	( Cab->DWC$a_dcab_current_item_vm == 0))
    {
	if (!DWC$$DB_Locate_day(Cab, &Day_VM, From_day, FALSE))
	{
	    _Pop_cause;
	    return (DWC$k_db_nomore);
	}
    }

    /*
    **  As this point we have a valid Day header pointer, and a valid
    **  current item pointer. Advance to the next item, check for end and
    **  update the Cab.
    */
    Day_VM = Cab->DWC$a_dcab_current_day_vm;
    Item_VM = Cab->DWC$a_dcab_current_item_vm;
    Item_VM = Item_VM->DWC$a_dbvm_vm_flink;
    if (Item_VM == (VM_record *)&Day_VM->DWC$a_dbvm_vm_flink)
    {
	Cab->DWC$a_dcab_current_item_vm = 0;
	_Pop_cause;
	return (DWC$k_db_nomore);
    }
    Cab->DWC$a_dcab_current_item_vm = Item_VM;
    Entry = _Bind(Item_VM, DWCDB_entry);
    header_len = _Field_offset(Entry,DWC$t_dben_data[0]);

    /*
    **  Unpack an entry
    */
    *Next_item_id = Entry->DWC$w_dben_entry_id;
    *Start_min = Entry->DWC$w_dben_start_minute;
    *Duration_min = Entry->DWC$w_dben_delta_minutes;
    *Duration_days = Entry->DWC$l_dben_delta_days;
    *Output_text_class = Entry->DWC$w_dben_text_class;

    if (Entry->DWC$b_dben_flags & DWC$m_dben_insignif)
    {
	*Flags = (DWC$m_item_insignif | Item_VM->DWC$b_dbvm_special3);
    }
    else
    {
	*Flags = Item_VM->DWC$b_dbvm_special3;
    }
	
    if (Entry->DWC$b_dben_flags & DWC$m_dben_alarm_present)
    {
	*Alarm_vec_len = Entry->DWC$t_dben_data[0];
	*(char **)Alarm_vec = &Entry->DWC$t_dben_data[1];
	/*
	** For now, all conversions are from fc.  At some point, this will be
	** replaced with the appropriate target conversions.
	*/
	if (*Output_text_class != DWC$k_item_text)
	{
	    *Output_icons_num = Entry->DWC$t_dben_data[1+(*Alarm_vec_len * 2)];
	    icon_len = *Output_icons_num +  1;
	}
	else
	{
	    *Output_icons_num = 0;
	    icon_len = 0;
	}

	if (*Output_icons_num != 0)
	    *Output_icon_ptr = (unsigned char *)
		&Entry->DWC$t_dben_data[2+(*Alarm_vec_len * 2)];
	else
	    *Output_icon_ptr = NULL;

	text_len = (Entry->DWC$w_dben_size - header_len) -
	    (1+(*Alarm_vec_len*2)+icon_len);
	if (fclen = 0)
	{
	    fc = (char *)XtMalloc (text_len + 1);
	    fclen = text_len + 1;
	}
	else if (text_len + 1 > fclen)
	{
	    fc = (char *)XtRealloc (fc, text_len + 1);
	    fclen = text_len + 1;
	}
	memcpy
	(
	    fc,
	    &Entry->DWC$t_dben_data[1+(*Alarm_vec_len*2)+icon_len],
	    text_len
	);
	fc[text_len] = 0;

	if (*Output_text_class != DWC$k_item_cstr)
	    xm_text = (unsigned char *)
		DXmCvtFCtoCS (fc, &byte_count, &cvt_status);
	else
	    xm_text = (unsigned char *)
		DXmCvtDDIFtoCS (fc, &byte_count, &cvt_status);

	*Output_text_ptr = xm_text;
    }
    else
    {
	*Alarm_vec_len = 0;

	if (*Output_text_class != DWC$k_item_text)
	{
	    *Output_icons_num = Entry->DWC$t_dben_data[0];
	    icon_len = *Output_icons_num + 1;
	}
	else
	{
	    *Output_icons_num = 0;
	    icon_len = 0;
	}

	if (*Output_icons_num != 0)
	    *Output_icon_ptr = (unsigned char *)&(Entry->DWC$t_dben_data[1]);
	else
	    *Output_icon_ptr = NULL;

	text_len = (Entry->DWC$w_dben_size - header_len) - (icon_len);
	if (fclen = 0)
	{
	    fc = (char *)XtMalloc (text_len + 1);
	    fclen = text_len + 1;
	}
	else if (text_len + 1 > fclen)
	{
	    fc = (char *)XtRealloc (fc, text_len + 1);
	    fclen = text_len + 1;
	}
	memcpy (fc, &Entry->DWC$t_dben_data[icon_len], text_len);
	fc[text_len] = 0;

	if (*Output_text_class != DWC$k_item_cstr)
	    xm_text = (unsigned char *)
		DXmCvtFCtoCS (fc, &byte_count, &cvt_status);
	else
	    xm_text = (unsigned char *)
		DXmCvtDDIFtoCS (fc, &byte_count, &cvt_status);

	*Output_text_ptr = xm_text;
    }

    if (Entry->DWC$b_dben_flags & DWC$m_dben_repeat)
    {
	Rep_blo_vm = (VM_record *)Item_VM->DWC$a_dbvm_parent_vm;
	Repc = (struct DWC$db_repeat_control *)Rep_blo_vm->DWC$l_dbvm_special2;
	Rep_exp = Repc->DWC$a_dbrc_vector_vm;
	*Rep_start_day = Rep_exp->DWC$l_dbre_baseday;
	*Rep_start_min = Rep_exp->DWC$w_dbre_basemin;
	*End_day = Rep_exp->DWC$l_dbre_endday;
	*End_min = Rep_exp->DWC$w_dbre_endmin;
	*P1 = Repc->DWC$b_dbrc_type;
	switch (Repc->DWC$b_dbrc_type)
	{
	case DWC$k_db_absolute :
		*P2 = Rep_exp->DWC$l_dbre_repeat_interval;
		*P3 = 0;
		break;
	case DWC$k_db_abscond :
		*P2 = Repc->DWC$l_dbrc_n;
		*P3 = Repc->DWC$w_dbrc_daycond;
		break;
	case DWC$k_db_nth_day_cwd :
		*P2 = Repc->DWC$b_dbrc_month_int;
		*P3 = Repc->DWC$w_dbrc_daycond |
			(Repc->DWC$l_dbrc_n << DWC$v_cond_day);
		break;
	default :
		*P2 = Repc->DWC$b_dbrc_month_int;
		*P3 = Repc->DWC$w_dbrc_daycond;
	}
    }
    else
    {
	*Rep_start_day = From_day;
	*Rep_start_min = Entry->DWC$w_dben_start_minute;
	*P1 = DWC$k_db_none;
    }

    /*
    **  Done, back to caller
    */
    _Pop_cause;
    return (DWC$k_db_normal);

}

int DWCDB_ModifyRItem
#ifdef	_DWC_PROTO_
(
    struct DWC$db_access_block  *Cab,
    int				Item_id,
    int				Start_day,
    int				Start_min,
    int				Not_used_1,
    int				Not_used_2,
    int				Duration_days,
    int				Duration_min,
    int				Alarm_vec_len,
    unsigned short int		Alarm_vec[],
    int				Flags,
    unsigned char		*Input_text_ptr,	/* XmString */
    int				Input_text_class,
    unsigned int		Input_icons_num,
    unsigned char		Input_icon_ptr[],
    int				P1,
    int				P2,
    int				P3,
    int				End_day,
    int				End_min
)
#else	/* no prototypes */
(
    Cab, Item_id, Start_day, Start_min, Not_used_1,
    Not_used_2, Duration_days, Duration_min,
    Alarm_vec_len, Alarm_vec, Flags,
    Input_text_ptr, Input_text_class, Input_icons_num, Input_icon_ptr,
    P1, P2, P3,
    End_day, End_min
)
    struct DWC$db_access_block  *Cab;
    int				Item_id;
    int				Start_day;
    int				Start_min;
    int				Not_used_1;
    int				Not_used_2;
    int				Duration_days;
    int				Duration_min;
    int				Alarm_vec_len;
    unsigned short int		Alarm_vec[];
    int				Flags;
    unsigned char		*Input_text_ptr;	/* XmString */
    int				Input_text_class;
    unsigned int		Input_icons_num;
    unsigned char		Input_icon_ptr[];
    int				P1;
    int				P2;
    int				P3;
    int				End_day;
    int				End_min;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine modifies one existing item/expression. Please note that
**	this routine cannot be used to:
**
**	    1. Convert a repeat expression to a non-repeating expression. For
**	       that you should Modify the repeat expression to go up to
**	       min-1 of the last repeat. Then add a non-repeating clone at
**	       this point.
**
**	    2. Convert a non-repeating expression to a repeat expression. The
**	       preferred method for doing this is to "clone" the parameters for
**	       the non-repeating and create a new repeating expression starting
**	       at the same time as the source for the clone. Then you can delete
**	       the non-repeating source.
**
**	It is not recommended to create repeat expressions that start before
**	current time.
**	
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Item_id : Caller field to receive id of item just created
**	Start_day : Starting day for this entry or repeat expression,
**		    input only; cannot be modified.
**	Start_min : Starting minute for this entry or repeat expression
**	Not_used_1 : Currently not used
**	Not_used_2 : Currently not used
**	Duration_days : Length in days of one instansiations
**	Duration_min : Length in minutes (within days) of one instansiation
**	Alarm_vec_len : Number of alarms associated with each instansiation
**	Alarm_vec : Word vector of delta minutes before instansiation to alarm
**	Flags : The following flags are known:
**
**			DWC$m_item_insignif	-- Item is not significant;
**						   please refer to routine
**						   DWC$DB_Get_day_attr for
**						   more details.
**
**	Input_text_ptr : Text string to be associated with item
**	Input_text_class : Type of text: The following types are
**			   recognized:
**
**			DWC$k_item_text		-- 8bit text (MCS); without
**						   trailing NULL
**			DWC$k_item_texti	-- Text and icon special.
**						   The first byte indicates
**						   the number of icons
**						   associated with item.
**						   There is then one byte
**						   for each icon (index). The
**						   actual MCS string
**						   starts after this ASCIC
**					           icon data. Please note
**						   that Input_text_length
**						   is the COMBINED length
**						   of these.
**			DWC$k_item_cstr		-- Compound string special.
**						   same as DWC$k_item_texti
**						   but text is in the form
**						   of a compond string.
**	Input_icon_ptr
**
**	P1 : Repeat parameter #1; see separate description
**	P2 : Repeat parameter #2; see separate description
**	P3 : Repeat parameter #3; see separate description
**	End_day : Ending day for repeat expression (ignored if not). If this
**		  parameter and End_min are both zero then the repeat expression
**		  will be infinite. Please note that this is interpreted as
**		  the starting time of the last instansiation.
**	End_min : Ending minute within Ending day of last instansiation. Please
**		  refer to End_day parameter.
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
**	DWC$k_db_normal	    -- Item modified
**	DWC$k_db_insdisk    -- Could not extend file to accomodate modification
**	DWC$k_db_dayfull    -- Target day full; modification could not be
**			       completed
**	DWC$k_db_nsitem	    -- No such item
**	DWC$k_db_toobig	    -- Item is too big
**	DWC$k_db_failure    -- General failure; please check errno (and
**			       possibly vaxc$errno).
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    VM_record *Day_VM;			/* Virtual day for entry	    */
    VM_record *Item_VM;			/* Virtual item			    */
    VM_record *Next_item_VM;		/* Next VM item in scan		    */
    struct DWCDB_day *Day_data;	/* The daymap itself		    */
    struct DWCDB_entry *Entry;		/* Entry buffer, if adding entry    */
    int Item_size;			/* Number of databytes for new item */
    int Item_header_size;		/* Fixed length header (item specif)*/
    char *Item_text;			/* Where to store text in item	    */
    int Alarm_len;			/* Number of bytes of Alarm data    */
    int Status;				/* Return status		    */
    int flush_stat;			/* Status from Db flush		    */
    unsigned char	*xm_text;	/* XmString */
    char		*fc;
    long		byte_count, cvt_status;
    int			icon_len;


    /*
    **  Setup error context, in case we fail
    */
    _Set_cause(DWC$_MODIFAIL);

    /*
    **  Is this a repeat expression?
    */
    if (Item_id < 0)
    {
	/*
	**  Yes, use special routine for this and back to caller
	*/
	Status = DWCDB__ModifyRepeat
	(
	    Cab,
	    Item_id,
	    Start_day,
	    Start_min,
	    End_day,
	    End_min,
	    Duration_min,
	    Alarm_vec_len,
	    Alarm_vec,
	    Flags,
	    Input_text_ptr,
	    Input_text_class,
	    Input_icons_num,
	    Input_icon_ptr,
	    P1,
	    P2,
	    P3
	);
	_Pop_cause;
	return (Status);
    }

    /*
    ** For now, all conversions are to fc.  At some point, this will be
    ** replaced with the appropriate target conversions.
    */
    if (Input_text_class != DWC$k_item_cstr)
	fc = (char *)DXmCvtCStoFC (Input_text_ptr, &byte_count, &cvt_status);
    else
	fc = (char *)DXmCvtCStoDDIF (Input_text_ptr, &byte_count, &cvt_status);

    /*
    **  Determine length of work buffer and compute length of alarm data array
    */
    Item_header_size = _Field_offset(Entry,DWC$t_dben_data[0]);
    if (Alarm_vec_len != 0)
	Alarm_len = (Alarm_vec_len * 2) + 1;
    else
	Alarm_len = 0;

    /*
    **  Compute the length of the buffer. Make sure that we do not exceed
    **	the 64Kb limit
    */
    if (Input_text_class == DWC$k_item_text)
	icon_len = 0;
    else
	icon_len = Input_icons_num + 1;

    Item_size = Item_header_size + byte_count + icon_len + Alarm_len;

    if (Item_size > DWC$k_db_max_16bit_unsigned)
    {
	_Pop_cause;
	return (DWC$k_db_toobig);
    }

    /*
    **  Call find_day with the allocation flag set to FALSE. Thus the day
    **  will note be created if it is not there.
    */
    if (!DWC$$DB_Locate_day(Cab, &Day_VM, Start_day, FALSE))
    {
	_Pop_cause;
	return (DWC$k_db_nsitem);
    }

    /*
    **	Find the right item to unhook. Remove it from queue and release it's
    **	buffer.
    */
    Status = DWC$k_db_nsitem;
    Item_VM = Day_VM->DWC$a_dbvm_vm_flink;
    while (Item_VM != (VM_record *)&Day_VM->DWC$a_dbvm_vm_flink)
    {
	Entry = _Bind(Item_VM, DWCDB_entry);
	if (Entry->DWC$w_dben_entry_id == Item_id)
	{
	    DWC$$DB_Remque
		((struct DWC$db_queue_head *) Item_VM->DWC$a_dbvm_vm_blink, 0);
	    DWC$$DB_Freelist_buffer (Cab, Item_VM);
	    Status = DWC$k_db_normal;
	    break;
	}
	Item_VM = Item_VM->DWC$a_dbvm_vm_flink;
    }

    /*
    **  Remove alarm item and flush the modified day.
    */
    if (Status == DWC$k_db_normal)
    {
	DWC$$DB_Remove_alarm_item(Cab, Start_day, Item_id);
	Status = DWC$$DB_Flush_day(Cab, Day_VM, Start_day);
    }
        
    /*
    **  Back to caller if anyting is wrong
    */
    if (Status != DWC$k_db_normal)
    {
	DWC$$DB_Release_day(Cab, Day_VM);
	_Pop_cause;
	return (Status);
    }
	
    /*
    **  Allocate work buffer
    */
    DWC$$DB_Get_free_buffer_z(Cab, Item_size, &Item_VM);

    /*
    **  Fill in the item, both the generic part and then the type specific part
    */
    Day_data = _Bind(Day_VM, DWCDB_day);
    Entry = _Bind(Item_VM, DWCDB_entry);
    Item_text = &(Item_VM->DWC$t_dbvm_data[Item_header_size]);
    Entry->DWC$b_dben_blocktype = DWC$k_dbit_entry;
    Entry->DWC$w_dben_size = Item_size;
    Entry->DWC$w_dben_entry_id	= Item_id;

    if (Alarm_len != 0)
    {
	Entry->DWC$b_dben_flags = (Entry->DWC$b_dben_flags |
					DWC$m_dben_alarm_present);
	Item_text[0] = Alarm_vec_len;
	memcpy(&Item_text[1], Alarm_vec, (Alarm_len - 1));
    }
    if (Input_text_class != DWC$k_item_text)
    {
	Item_text[Alarm_len] = Input_icons_num;
	if (Input_icons_num != 0)
	{
	    memcpy(&Item_text[Alarm_len+1], Input_icon_ptr, icon_len);
	}
    }
    memcpy(&Item_text[Alarm_len+icon_len], fc, byte_count);
    Entry->DWC$w_dben_start_minute = Start_min;
    Entry->DWC$l_dben_delta_days = Duration_days;
    Entry->DWC$w_dben_delta_minutes = Duration_min;
    Entry->DWC$w_dben_text_class = Input_text_class;
    if (Flags & DWC$m_item_insignif)
    {
	Entry->DWC$b_dben_flags = (Entry->DWC$b_dben_flags
				    | DWC$m_dben_insignif);
    }	    

    /*
    **  Find the right place to hook up: after the last note,
    **	in the right start time order. Having found the insertion
    **	point, insert and call UNLOAD DAY to write out to disk.
    */
    Next_item_VM = Day_VM->DWC$a_dbvm_vm_flink;
    while (Next_item_VM != (VM_record *)&Day_VM->DWC$a_dbvm_vm_flink)
    {
	Entry = _Bind(Next_item_VM, DWCDB_entry);
	if (Entry->DWC$w_dben_start_minute > Start_min) break;
	Next_item_VM = Next_item_VM->DWC$a_dbvm_vm_flink;
    }
    DWC$$DB_Insque
    (
	(struct DWC$db_queue_head *) Item_VM,
	(struct DWC$db_queue_head *) Next_item_VM->DWC$a_dbvm_vm_blink
    );

    flush_stat = DWC$$DB_Flush_day (Cab, Day_VM, Start_day);
    if (flush_stat != DWC$k_db_normal)
    {
	_Pop_cause;
	return (flush_stat);
    }
    DWC$$DB_Insert_alarm_item(Cab, Item_VM, Start_day);
	
    /*
    **  Done, back to caller in success
    */
    _Pop_cause;
    return (DWC$k_db_normal);
}

int DWCDB_GetSpecificRItem
#ifdef	_DWC_PROTO_
(
    struct DWC$db_access_block  *Cab,
    int				From_day,
    int				Item_id,
    int				*Rep_start_day,
    int				*Rep_start_min,
    int				*Start_min,
    int				*Duration_days,
    int				*Duration_min,
    int				*Alarm_vec_len,
    unsigned short int		**Alarm_vec,
    int				*Flags,
    unsigned char		**Output_text_ptr,	/* XmString */
    int				*Output_text_class,
    unsigned int		*Output_icons_num,
    unsigned char		**Output_icon_ptr,
    int				*P1,
    int				*P2,
    int				*P3,
    int				*End_day,
    int				*End_min
)
#else	/* no prototypes */
(
    Cab, From_day, Item_id, Rep_start_day,
    Rep_start_min, Start_min, Duration_days,
    Duration_min, Alarm_vec_len, Alarm_vec, Flags,
    Output_text_ptr, Output_text_class, Output_icons_num, Output_icon_ptr,
    P1, P2, P3,
    End_day, End_min
)
    struct DWC$db_access_block  *Cab;
    int				From_day;
    int				Item_id;
    int				*Rep_start_day;
    int				*Rep_start_min;
    int				*Start_min;
    int				*Duration_days;
    int				*Duration_min;
    int				*Alarm_vec_len;
    unsigned short int		**Alarm_vec;
    int				*Flags;
    unsigned char		**Output_text_ptr;	/* XmString */
    int				*Output_text_class;
    unsigned int		*Output_icons_num;
    unsigned char		**Output_icon_ptr;
    int				*P1;
    int				*P2;
    int				*P3;
    int				*End_day;
    int				*End_min;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine retreives a specific item on a specific day, without
**	changing the current Get_item context.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	From_day : Day in which item appears
**	Item_id : Id of item to retreive
**	Rep_start_day : Starting day of repeat expression, if repeated, else
**			the value of From_day.
**	Rep_start_min : Starting minute of repeat expression, if repeated, else
**			the value of Start_min.
**	Start_min : Starting minute for this instansiation
**	Duration_days : Length in days of one instansiations
**	Duration_min : Length in minutes (within days) of one instansiation
**	Alarm_vec_len : Number of alarms associated with each instansiation
**	Alarm_vec : Word vector of delta minutes before instansiation to alarm
**	Flags : The following flags are known:
**
**			DWC$m_item_insignif	-- Item is not significant;
**						   please refer to routine
**						   DWC$DB_Get_day_attr for
**						   more details.
**
**			DWC$m_item_rpos		-- Repeat position value;
**						   can contain one of the
**						   following values:
**			  DWC$k_item_last	-- Last or only entry in
**						   repeat sequence.
**			  DWC$k_item_first	-- First entry in repeat
**						   sequence.
**			  DWC$k_item_middle	-- Neither first nor last
**						   entry.
**
**	Output_text_ptr : Pointer to text data associated with item. 
**	Output_text_class : Type of text: The following types are
**			   recognized:
**
**			DWC$k_item_text		-- 8bit text (MCS); without
**						   trailing NULL
**			DWC$k_item_texti	-- Text and icon special.
**						   The first byte indicates
**						   the number of icons
**						   associated with item.
**						   There is then one byte
**						   for each icon (index). The
**						   actual MCS string
**						   starts after this ASCIC
**					           icon data. Please note
**						   that Output_text_length
**						   is the COMBINED length
**						   of these.
**			DWC$k_item_cstr		-- Compound string special.
**						   same as DWC$k_item_texti
**						   but text is in the form
**						   of a compond string.
**	Output_icon_ptr : Pointer to icon data associated with item.
**
**	P1 : Repeat parameter #1; see separate description
**	P2 : Repeat parameter #2; see separate description
**	P3 : Repeat parameter #3; see separate description
**	End_day : Ending day for repeat expression (ignored if not). If this
**		  parameter and End_min are both zero then the repeat expression
**		  will be infinite. Please note that this is interpreted as
**		  the starting time of the last instansiation.
**	End_min : Ending minute within Ending day of last instansiation. Please
**		  refer to End_day parameter.
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
**	DWC$k_db_normal	    -- Item retreived
**	DWC$k_db_nsitem	    -- No such item on day
**	DWC$k_db_failure    -- General failure; please check errno (and
**			       possibly vaxc$errno).
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    
    VM_record *Day_VM;			/* Virtual day for entry	    */
    VM_record *Item_VM;			/* Virtual item			    */
    struct DWCDB_entry *Entry;		/* Entry buffer, if adding entry    */
    VM_record *Rep_blo_vm;		/* Repeat block for rep expr	    */
    struct DWC$db_repeat_control *Repc;	/* Repeat expression control block  */
    struct DWCDB_repeat_expr *Rep_exp;	/* Repeat expression vector entry   */
    VM_record *Old_day_vm;		/* Saved day context		    */
    int Old_day;			/* Saved day number		    */
    VM_record *Old_item_vm;		/* Save item context		    */
    VM_record *Sub_VM;			/* Virtual subrangemap		    */
    int Alarm_len;			/* Number of bytes of alarm data    */
    unsigned char	*xm_text;	/* XmString */
    static char		*fc = NULL;
    static int		fclen = 0;
    long		byte_count, cvt_status;
    int			icon_len;
    int			text_len;
    char		temp_char;
    int			header_len;

    /*
    **  Seup error context
    */
    _Set_base(DWC$_GETIFAIL);

    /*
    **  Save old context, if any
    */
    Old_day_vm = Cab->DWC$a_dcab_current_day_vm;
    Old_day = Cab->DWC$l_dcab_current_day;
    Old_item_vm = Cab->DWC$a_dcab_current_item_vm;

    /*
    **	If there is a day item context, different from the one we are about to
    **	retreive, force it to the front of the VMS cache so that it will not be
    **	purged out in case we need to look for the requested day. This action
    **	should not be taken if the current daymap is a stub.
    */
    if (((Day_VM = Cab->DWC$a_dcab_current_day_vm) != 0) &&
	(Cab->DWC$l_dcab_current_day != From_day) &&
	(Day_VM->DWC$l_dbvm_rec_addr != 0))
    {
	DWC$$DB_Touch_buffer(Cab, Day_VM);
	Sub_VM = (VM_record *)Day_VM->DWC$a_dbvm_special;
	DWC$$DB_Touch_buffer (Cab, Sub_VM);
	DWC$$DB_Touch_buffer (Cab, (VM_record *)Sub_VM->DWC$a_dbvm_special);
    }

    /*
    **  Get target day and context loaded
    */
    if (( Cab->DWC$l_dcab_current_day != From_day ) ||
	( Cab->DWC$a_dcab_current_item_vm == 0))
    {
	Cab->DWC$a_dcab_current_day_vm = 0;
	if (!DWC$$DB_Locate_day(Cab, &Day_VM, From_day, FALSE))
	{
	    Cab->DWC$a_dcab_current_day_vm = Old_day_vm;
	    Cab->DWC$l_dcab_current_day = Old_day;
	    Cab->DWC$a_dcab_current_item_vm = Old_item_vm;
	    _Pop_cause;
	    return (DWC$k_db_nsitem);
	}
    }

    /*
    **  Scan for specificed item
    */
    Day_VM = Cab->DWC$a_dcab_current_day_vm;
    Item_VM = Day_VM->DWC$a_dbvm_vm_flink;
    while (TRUE)
    {
	if (Item_VM == (VM_record *)&Day_VM->DWC$a_dbvm_vm_flink)
	{
	    if (Day_VM != Old_day_vm)
	    {
		DWC$$DB_Fry_stub_day(Cab, Day_VM);
	    }
	    Cab->DWC$a_dcab_current_day_vm = Old_day_vm;
	    Cab->DWC$l_dcab_current_day = Old_day;
	    Cab->DWC$a_dcab_current_item_vm = Old_item_vm;
	    _Pop_cause;
	    return (DWC$k_db_nsitem);
	}
	Entry = _Bind(Item_VM, DWCDB_entry);
	if (Item_id == Entry->DWC$w_dben_entry_id) break;
	Item_VM = Item_VM->DWC$a_dbvm_vm_flink;
    }
    header_len = _Field_offset(Entry,DWC$t_dben_data[0]);

    /*
    **  Restore old day context
    */
    Cab->DWC$a_dcab_current_day_vm = Old_day_vm;
    Cab->DWC$l_dcab_current_day = Old_day;
    Cab->DWC$a_dcab_current_item_vm = Old_item_vm;

    /*
    **  Unpack an entry
    */
    *Start_min = Entry->DWC$w_dben_start_minute;
    *Duration_min = Entry->DWC$w_dben_delta_minutes;
    *Duration_days = Entry->DWC$l_dben_delta_days;
    *Output_text_class = Entry->DWC$w_dben_text_class;

    if (Entry->DWC$b_dben_flags & DWC$m_dben_insignif)
    {
	*Flags = (DWC$m_item_insignif | Item_VM->DWC$b_dbvm_special3);
    }
    else
    {
	*Flags = Item_VM->DWC$b_dbvm_special3;
    }
	
    if (Entry->DWC$b_dben_flags & DWC$m_dben_alarm_present)
    {

	Alarm_len = (Entry->DWC$t_dben_data[0] * 2) + 1;
	*Alarm_vec_len = Entry->DWC$t_dben_data[0];

	text_len =
	    Entry->DWC$w_dben_size -
	    _Field_offset(Entry,DWC$t_dben_data[0]) - Alarm_len;
    }
    else
    {
	Alarm_len = 0;
	*Alarm_vec_len = 0;
	text_len =
	    Entry->DWC$w_dben_size - _Field_offset(Entry,DWC$t_dben_data[0]);
    }

    if (*Output_text_class != DWC$k_item_text)
    {
	*Output_icons_num = Entry->DWC$t_dben_data[Alarm_len];
	icon_len = *Output_icons_num + 1;
    }
    else
    {
	*Output_icons_num = 0;
	icon_len = 0;
    }

    text_len -= icon_len;

    if (fclen = 0)
    {
	fc = (char *)XtMalloc (text_len + 1);
	fclen = text_len + 1;
    }
    else if (text_len + 1 > fclen)
    {
	fc = (char *)XtRealloc (fc, text_len + 1);
	fclen = text_len + 1;
    }
    memcpy (fc, &Entry->DWC$t_dben_data[Alarm_len+icon_len], text_len);
    fc[text_len] = 0;

    if (Entry->DWC$b_dben_flags & DWC$m_dben_repeat)
    {
	Rep_blo_vm = (VM_record *)Item_VM->DWC$a_dbvm_parent_vm;
	Repc = (struct DWC$db_repeat_control *)Rep_blo_vm->DWC$l_dbvm_special2;
	Rep_exp = Repc->DWC$a_dbrc_vector_vm;
	*Rep_start_day = Rep_exp->DWC$l_dbre_baseday;
	*Rep_start_min = Rep_exp->DWC$w_dbre_basemin;
	*End_day = Rep_exp->DWC$l_dbre_endday;
	*End_min = Rep_exp->DWC$w_dbre_endmin;
	*P1 = Repc->DWC$b_dbrc_type;
	switch (Repc->DWC$b_dbrc_type)
	{
	case DWC$k_db_absolute :
	    *P2 = Rep_exp->DWC$l_dbre_repeat_interval;
	    *P3 = 0;
	    break;
	case DWC$k_db_abscond :
	    *P2 = Repc->DWC$l_dbrc_n;
	    *P3 = Repc->DWC$w_dbrc_daycond;
	    break;
	case DWC$k_db_nth_day_cwd :
	    *P2 = Repc->DWC$b_dbrc_month_int;
	    *P3 = Repc->DWC$w_dbrc_daycond |
		    (Repc->DWC$l_dbrc_n << DWC$v_cond_day);
	    break;
	default :
	    *P2 = Repc->DWC$b_dbrc_month_int;
	    *P3 = Repc->DWC$w_dbrc_daycond;
	}
    }
    else
    {
	*Rep_start_day = From_day;
	*Rep_start_min = Entry->DWC$w_dben_start_minute;
	*P1 = DWC$k_db_none;
    }

    /*
    **  Save string and alarm vector in safe place before we fry stub
    **  day
    */
    DWCDB__FryTempArrays(Cab);
    if (*Alarm_vec_len != 0)
    {
	Cab->DWC$a_dcab_temp_alarm = (char *)XtMalloc(Alarm_len - 1);
	if (Cab->DWC$a_dcab_temp_alarm == 0)
	{
	    _Record_error;
	    _Signal(DWC$_INSVIRMEM);
	}
	memcpy
	(
	    Cab->DWC$a_dcab_temp_alarm,
	    &Entry->DWC$t_dben_data[1],
	    Alarm_len - 1
	);
	*(char **)Alarm_vec = Cab->DWC$a_dcab_temp_alarm;
    }

    if (*Output_icons_num != 0)
    {
	Cab->DWC$a_dcab_temp_icons = (char *)XtMalloc(*Output_icons_num);
	if (Cab->DWC$a_dcab_temp_icons == NULL)
	{
	    _Record_error;
	    _Signal(DWC$_INSVIRMEM);
	}
	*Output_icon_ptr = (unsigned char *)Cab->DWC$a_dcab_temp_icons;
	memcpy
	(
	    *Output_icon_ptr,
	    &(Entry->DWC$t_dben_data[Alarm_len+1]),
	    *Output_icons_num
	);
    }
    else
    {
	*Output_icon_ptr = NULL;
    }

    if (text_len != 0)
    {
	if (*Output_text_class != DWC$k_item_cstr)
	    Cab->DWC$a_dcab_temp_text = (char *) DXmCvtFCtoCS
		(fc, &byte_count, &cvt_status);
	else
	    Cab->DWC$a_dcab_temp_text = (char *) DXmCvtDDIFtoCS
		(fc, &byte_count, &cvt_status);

	if (Cab->DWC$a_dcab_temp_text == NULL)
	{
	    _Record_error;
	    _Signal(DWC$_INSVIRMEM);
	}
	*Output_text_ptr = (unsigned char *)Cab->DWC$a_dcab_temp_text;
    }
    else
    {
	*Output_text_ptr = NULL;
    }

    /*
    **  Get rid of temp daymap
    */
    if (Day_VM != Old_day_vm)
    {
	DWC$$DB_Fry_stub_day(Cab, Day_VM);
    }

    /*
    **  Done, back to caller
    */
    _Pop_cause;
    return (DWC$k_db_normal);
}

/*
**  End of Module
*/
