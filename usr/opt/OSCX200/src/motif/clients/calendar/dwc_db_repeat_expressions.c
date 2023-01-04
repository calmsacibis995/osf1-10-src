/* dwc_db_repeat_expressions.c */
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
**	This module implements the support for Repeat expressions
**
**--
*/

/*
**  Include Files
*/
#include "dwc_compat.h"

#include <stddef.h>
#include <string.h>

#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"
#include "dwc_ui_datefunctions.h"

/*
**  Possible return codes from repeat expression into day merge
*/
#define _MERGE_CONFL -1	    /* Conflicting merge -- timeslot used	    */
#define _MERGE_FAIL 0	    /* Failed to perform merge			    */
#define _MERGE_OK 1	    /* Merge was successful			    */

int
DWC$$DB_Compress_params PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int P1,
	int P2,
	int P3,
	int Day));


int
DWC$$DB_Load_repeats
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block  *Cab)
#else	/* no prototypes */
	(Cab)
	struct DWC$db_access_block  *Cab;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine is invoked when the calendar file is opened for access. It
**	brings in all repeat expressions into memory and builds a list of repeat
**	expression control blocks.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
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
**	TRUE	- Repeat expressions loaded
**	FALSE	- Failed to load repeat expressions
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    VM_record *Headvm;			    /* Virtual header block	    */
    struct DWCDB_header *Head;		    /* Real header block	    */
    void *Next_vec_ptr;			    /* Pointer to next repeat vector*/
    int Rep_cluster;			    /* Current repeat cluster	    */
    VM_record *Next_vec_vm;		    /* Next repeat vector block	    */
    struct DWCDB_repeat_vector *Next_vec;  /* Real repeat vector	    */
    struct DWCDB_repeat_expr *Next_exp;    /* Next repeat expression	    */
    int i;				    /* Local loop variable	    */
    struct DWC$db_repeat_control *New_rep_ctl; /* New repeat control blk    */


    /*
    **  Setup context in case we fail
    */
    _Set_cause(DWC$_REPLOAD);
    
    /*
    **  Point at head of vector queue and enter loading loop
    */
    Headvm = Cab->DWC$a_dcab_header;
    Head = _Bind(Headvm, DWCDB_header);
    Next_vec_ptr = (void *) _Ptr((CARD32)Head->DWC$l_dbhd_repeat_head);
    Rep_cluster = 0;
    while (Next_vec_ptr != (int)NULL)
	{
	
	/*
	**  Bring in next vector block, but do not place it in the cache
	*/
	Next_vec_vm = DWC$$DB_Follow_pointer_NC(Cab, &Next_vec_ptr,
				DWC$k_db_repeat_expr_vec);
	if (Next_vec_vm == (VM_record *)-1)
	    {
	    _Pop_cause;
	    return (FALSE);
	    }
	
	/*
	**  Start looking at the first vector entry
	*/
	Next_vec = _Bind(Next_vec_vm, DWCDB_repeat_vector);
	Next_exp = (struct DWCDB_repeat_expr *)&Next_vec->DWC$t_dbrv_data[0];
	
	/*
	**  Enter loop to build corresponding repeat expression control blocks
	**  for the entries that are marked as used.
	*/
	for (i=1; i<=DWC$k_db_repeats_per_vector; i++)
	    {
	    
	    /*
	    **  Is this entry marked as used?
	    */
	    if (Next_exp->DWC$b_dbre_flags & DWC$m_dbre_used)
		{
		
		/*
		**  Yes. Allocate VM for repeat expression control block
		*/
		if ((New_rep_ctl = (struct DWC$db_repeat_control *)XtMalloc (sizeof(*New_rep_ctl))) == 0)
		    {
		    _Record_error;
		    _Signal(DWC$_INSVIRMEM);
		    return (FALSE);
		    }
		
		/*
		**  Fill in the repeat expression control block and insert it in
		**  the queue off the Cab.
		*/
		New_rep_ctl->DWC$a_dbrc_vector_block = Next_vec_vm;
		New_rep_ctl->DWC$a_dbrc_vector_vm = Next_exp;
		New_rep_ctl->DWC$w_dbrc_id = -((Rep_cluster *
			DWC$k_db_repeats_per_vector) + i);
		DWC$$DB_Expand_interval(Cab, Next_exp, New_rep_ctl);
		DWC$$DB_Insque
		(
		    (struct DWC$db_queue_head *) New_rep_ctl,
		    (struct DWC$db_queue_head *)
			&(Cab->DWC$a_dcab_repeat_ctl_flink)
		);

		/*
		**  Patch old absolute repeats in case the duration causes
		**  instances to cross midnight.
		*/
		if ((Next_exp->DWC$w_dbre_basemin +
		     Next_exp->DWC$w_dbre_duration) >=
			DWC$k_db_calendar_precision)
		    {
		    Next_exp->DWC$w_dbre_duration =
			(DWC$k_db_calendar_precision - 1) -
			Next_exp->DWC$w_dbre_basemin;
		    }
		}
	    
	    /*
	    **  Advance to next expression in vector block
	    */
	    Next_exp = (struct DWCDB_repeat_expr *)(
		    (char *)Next_exp + DWC$k_db_repeat_expr_len);
	    }
	
	/*
	**  All entries have been processed in this vector block. Add the
	**  virtual vector block to queue off Cab (so that we can easily write
	**  it out when we update one repeat expression.
	*/
	DWC$$DB_Insque
	(
	    (struct DWC$db_queue_head *) Next_vec_vm,
	    (struct DWC$db_queue_head *) Cab->DWC$a_dcab_repeat_vec_blink
	);
	Next_vec_ptr = (void *) _Ptr(Next_vec->DWC$l_dbrv_flink);
	Rep_cluster++;
	}
    
    /*
    **  Expressions are loaded (if any). Indicate this by setting a nonzero
    **	repeat context. This will force days to check this list as they are
    **	being accessed.
    */
    Cab->DWC$l_dcab_repeat_ctx = 1;
    
    /*
    **  Done, back to caller.
    */
    _Pop_cause;
    return (TRUE);
}

int
DWC$$DB_Expand_interval
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	struct DWCDB_repeat_expr *Exp,
	struct DWC$db_repeat_control *Repc)
#else	/* no prototypes */
	(Cab, Exp, Repc)
	struct DWC$db_access_block	*Cab;
	struct DWCDB_repeat_expr *Exp;
	struct DWC$db_repeat_control *Repc;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine will decompress the repeat values stored in the
**	repeat expression for easier use when checking triggering.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Exp : Repeat expression to expand
**	Repc : Repeat expression control block (filled in)
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
**	TRUE	- Expanded
**	FALSE	- Not expanded
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
    
{
    int i;		/* Work value for expand		*/
    int d;		/* Day of month				*/
    int m;		/* Month of year			*/
    int y;		/* Year					*/
    
    /*
    **  Get encoded value and copy repeat expression type
    */
    i = Exp->DWC$l_dbre_repeat_interval;
    Repc->DWC$b_dbrc_type = Exp->DWC$b_dbre_reptype;

    /*
    **  Decompress based on expression type
    */
    switch ((int)Repc->DWC$b_dbrc_type)
	{

	/*
	**  Absolute
	**
	**	Compressed form of data is using the following 32-bit mask:
	**
	**	Start_bit   Bit_count	Use
	**	---------------------------------------------------
	**
	**	    0		32	Number of minutes between start times
	*/
	case DWC$k_db_absolute :
	    {
	    Repc->DWC$l_dbrc_n = i;
	    break;
	    }

	/*
	**  Conditional absolute
	**
	**	Compressed form of data is using the following 32-bit mask:
	**
	**	Start_bit   Bit_count	Use
	**	---------------------------------------------------
	**
	**	    0		16	Repeat interval in days
	**	   16		10	Conditions
	*/
	case DWC$k_db_abscond :
	    {
	    Repc->DWC$l_dbrc_n = (i & 65535);
	    i = i >> 16;
	    Repc->DWC$w_dbrc_daycond = (i & DWC$m_dbrc_valbits);
	    break;
	    }	    

	/*
	**  By n:th day of month (from start or end)
	**
	**	Compressed form of data is using the following 32-bit mask:
	**
	**	Start_bit   Bit_count	Use
	**	---------------------------------------------------
	**
	**	    0		5	Day number (from start or end)
	**	    5		4	Month interval
	**	    9		4	Base month
	**	   13	       10	Conditions
	*/
	case DWC$k_db_nth_day :
	case DWC$k_db_nth_day_end :
	    {
	    Repc->DWC$l_dbrc_n = (i & 31);
	    i = i >> 5;
	    Repc->DWC$b_dbrc_month_int = (i & 15);
	    i = i >> 4;
	    Repc->DWC$b_dbrc_base_month = (i & 15);
	    i = i >> 4;
	    Repc->DWC$w_dbrc_daycond = (i & DWC$m_dbrc_valbits);
	    break;
	    }	    

	/*
	**  By n:th x:day in month
	**
	**	Compressed form of data is using the following 32-bit mask:
	**
	**	Start_bit   Bit_count	Use
	**	---------------------------------------------------
	**
	**	    0		3	Day of week
	**	    3		3	'n'
	**	    6		4	Month interval
	**	   10		4	Base month
	**	   14	       10	Conditions
	*/
	case DWC$k_db_nth_xday :
	    {
	    Repc->DWC$b_dbrc_weekday = (i & 7);
	    i = i >> 3;
	    Repc->DWC$l_dbrc_n = (i & 7);
	    i = i >> 3;
	    Repc->DWC$b_dbrc_month_int = (i & 15);
	    i = i >> 4;
	    Repc->DWC$b_dbrc_base_month = (i & 15);
	    i = i >> 4;
	    Repc->DWC$w_dbrc_daycond = (i & DWC$m_dbrc_valbits);
	    break;
	    }	    

	/*
	**  Last weekday in month
	**
	**	Compressed form of data is using the following 32-bit mask:
	**
	**	Start_bit   Bit_count	Use
	**	---------------------------------------------------
	**
	**	    0		3	Day of week
	**	    3		4	Month interval
	**	    7		4	Base month
	**	   11	       10	Conditions
	*/
	case DWC$k_db_last_weekday :
	    {
	    Repc->DWC$l_dbrc_n = 0;
	    Repc->DWC$b_dbrc_weekday = (i & 7);
	    i = i >> 3;
	    Repc->DWC$b_dbrc_month_int = (i & 15);
	    i = i >> 4;
	    Repc->DWC$b_dbrc_base_month = (i & 15);
	    i = i >> 4;
	    Repc->DWC$w_dbrc_daycond = (i & DWC$m_dbrc_valbits);
	    break;
	    }	    

	/*
	**  By n:th day of month conditional weekday
	**
	**	Compressed form of data is using the following 32-bit mask:
	**
	**	Start_bit   Bit_count	Use
	**	---------------------------------------------------
	**
	**	    0		3	Desired weekday
	**	    3		4	Month interval
	**	    7		4	Base month
	**	   11	       10	Target day of month conditions
	**	   21	       10	Conditions 
	*/
	case DWC$k_db_nth_day_cwd :
	    {
	    Repc->DWC$b_dbrc_weekday = (i & 7);
	    i = i >> 3;
	    Repc->DWC$b_dbrc_month_int = (i & 15);
	    i = i >> 4;
	    Repc->DWC$b_dbrc_base_month = (i & 15);
	    i = i >> 4;
	    Repc->DWC$l_dbrc_n = (i & DWC$m_dbrc_valbits);
	    i = i >> DWC$v_cond_day;
	    Repc->DWC$w_dbrc_daycond = (i & DWC$m_dbrc_valbits);
	    break;
	    }	    

	/*
	**  Trap unexpected values
	*/
	default :
	    {
	    _Signal(DWC$_UNEXPREP);
	    }
	}

    /*
    **  Done, back to caller
    */
    return (TRUE);
}

int
DWC$$DB_Compress_params
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	int P1,
	int P2,
	int P3,
	int Day)
#else	/* no prototypes */
	(Cab, P1, P2, P3, Day)
	struct DWC$db_access_block	*Cab;
	int P1;
	int P2;
	int P3;
	int Day;
#endif	/* prototypes */    
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine will compress the expanded repeat expression params
**	into a compressed format (for storage in expression).
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	P1 : P1 (as for Add or Modify repeat)
**	P2 : P2 (as for Add or Modify repeat)
**	P3 : P3 (as for Add or Modify repeat)
**	Day : Date on which formula is to be based
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
**	Compressed value
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    int i;	    /* Compressed result			*/
    int d;	    /* Day in month				*/
    int m;	    /* Month in year				*/
    int y;	    /* Year					*/
    int tm;	    /* True month for conditional weekday	*/
    int rd;	    /* Required day of month for CWD		*/
    int tdc;	    /* Target day conditions			*/
        
    /*
    **  Unless absolute, convert day number into year, month and day
    */
    if ((P1 != DWC$k_db_absolute) && (P1 != DWC$k_db_abscond))
	{
	DATEFUNCDateForDayNumber(Day, &d, &m, &y);
	}

    /*
    **  Compress code is specific to expression type
    */
    i = 0;
    switch (P1)
	{

	/*
	**  Absolute
	**
	**	Compressed form of data is using the following 32-bit mask:
	**
	**	Start_bit   Bit_count	Use
	**	---------------------------------------------------
	**
	**	    0		32	Number of minutes between start times
	*/
	case DWC$k_db_absolute :
	    {
	    i = P2;
	    break;
	    }

	/*
	**  Conditional absolute
	**
	**	Compressed form of data is using the following 32-bit mask:
	**
	**	Start_bit   Bit_count	Use
	**	---------------------------------------------------
	**
	**	    0		16	Repeat interval in days
	**	   16		10	Conditions
	*/
	case DWC$k_db_abscond :
	    {
	    i = P3;
	    i = i << 16;
	    i = i | (P2 & 65535);
	    break;
	    }

	/*
	**  By n:th day of month (from start)
	**
	**	Compressed form of data is using the following 32-bit mask:
	**
	**	Start_bit   Bit_count	Use
	**	---------------------------------------------------
	**
	**	    0		5	Day number in month
	**	    5		4	Month interval
	**	    9		4	Base month
	**	   13	       10	Conditions
	*/
	case DWC$k_db_nth_day :
	    {
	    i = P3;
	    i = i << 4;
	    i = (i | m);
	    i = i << 4;
	    i = (i | P2);
	    i = i << 5;
	    i = (i | d);
	    break;
	    }

	/*
	**  By n:th x:day in month
	**
	**	Compressed form of data is using the following 32-bit mask:
	**
	**	Start_bit   Bit_count	Use
	**	---------------------------------------------------
	**
	**	    0		3	Day of week
	**	    3		3	'n'
	**	    6		4	Month interval
	**	   10		4	Base month
	**	   14	       10	Conditions
	*/
	case DWC$k_db_nth_xday :
	    {
	    i = P3;
	    i = i << 4;
	    i = (i | m);
	    i = i << 4;
	    i = (i | P2);
	    i = i << 3;
	    i = (i | ((d + 6) / 7));
	    i = i << 3;
	    i = (i | DATEFUNCDayOfWeek(d, m, y));
	    break;
	    }	    

	/*
	**  Last weekday in month
	**
	**	Compressed form of data is using the following 32-bit mask:
	**
	**	Start_bit   Bit_count	Use
	**	---------------------------------------------------
	**
	**	    0		3	Day of week
	**	    3		4	Month interval
	**	    7		4	Base month
	**	   11	       10	Conditions
	*/
	case DWC$k_db_last_weekday :
	    {
	    i = P3;
	    i = i << 4;
	    i = (i | m);
	    i = i << 4;
	    i = (i | P2);
	    i = i << 3;
	    i = (i | DATEFUNCDayOfWeek(d, m, y));
	    break;
	    }	    

	/*
	**  By n:th day of month (from end)
	**
	**	Compressed form of data is using the following 32-bit mask:
	**
	**	Start_bit   Bit_count	Use
	**	---------------------------------------------------
	**
	**	    0		5	'n'
	**	    5		4	Month interval
	**	    9		4	Base month
	**	   13	       10	Conditions
	*/
	case DWC$k_db_nth_day_end :
	    {
	    i = P3;
	    i = i << 4;
	    i = (i | m);
	    i = i << 4;
	    i = (i | P2);
	    i = i << 5;
	    i = (i | ((DATEFUNCDaysInMonth(m, y) - d) + 1));
	    break;
	    }

	/*
	**  By n:th day of month conditional weekday
	**
	**	Compressed form of data is using the following 32-bit mask:
	**
	**	Start_bit   Bit_count	Use
	**	---------------------------------------------------
	**
	**	    0		3	Desired weekday
	**	    3		4	Month interval
	**	    7		4	Base month
	**	   11	       10	Target day of month conditions
	**	   21	       10	Conditions 
	*/
	case DWC$k_db_nth_day_cwd :
	    {
	    /*
	    **  First, sort out True base month. The reason being that the user
	    **	may specify a base-day which may already be valid for an
	    **  instance that has "floated". For example, the user creates
	    **	a repeat on the 2:nd with the parameters first friday after
	    **	the 26th. In this case, the month of the 26th must be taken as
	    **	the True base month.
	    */
	    tm = m;
	    tdc = P3 >> DWC$v_cond_day;
	    rd = ((tdc & DWC$m_cond_mask) & ~DWC$m_cond_flags);
	    if ((tdc & DWC$m_cond_flags) == DWC$m_cond_fwd)
		{
		if (rd > d)
		    {
		    tm--;
		    if (tm == 0)
			{
			tm = 12;
			}
		    }
		}
	    else
		{
		if (rd < d)
		    {
		    tm++;
		    if (tm == 13)
			{
			tm = 1;
			}
		    }
		}
	    /*
	    **  And now .. fill in the rest.
	    */
	    i = (P3 & DWC$m_dbrc_valbits);
	    i = i << DWC$v_cond_day;
	    i = i | (P3 >> DWC$v_cond_day);
	    i = i << 4;
	    i = (i | tm);
	    i = i << 4;
	    i = (i | P2);
	    i = i << 3;
	    i = (i | DATEFUNCDayOfWeek(d, m, y));
	    break;
	    }

	}

    /*
    **  Return compressed value to caller
    */
    return (i);

}

int
DWC$$DB_Unload_repeats
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
**	This routine unloads the last parts of the repeat expressions when the
**	calendar file is being closed (called from close).
**
**	It is very important that this routine does not get called until the
**	entire cache has been purged but not cleaned up.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
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
**	TRUE	- Success in unloading the repeat expression database
**	FALSE	- Failed to unload the repeat expression database
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    VM_record *Repvec_vm;		    /* Work VM repeat vector blk    */
    struct DWC$db_repeat_control *Repctl;   /* Work repeat control block    */
    struct DWCDB_repeat_vector *Rblk;	    /* Repeat block (without head)  */
    struct DWCDB_repeat_expr *Next_exp;    /* Repeat expression itself	    */
    int i;				    /* General loop variable	    */
    

    /*
    **  Establish context for failure
    */
    _Set_cause(DWC$_UNLREPS);

    /*
    **  Transfer all repeat vector blocks to free list. In the process,
    **	release all memory associated with repeat expression exception days.
    */
    while
    (
	DWC$$DB_Remque
	(
	    (struct DWC$db_queue_head *) &Cab->DWC$a_dcab_repeat_vec_flink,
	    (struct DWC$db_queue_head **) &Repvec_vm
	)
    )
	{
	Rblk = _Bind(Repvec_vm, DWCDB_repeat_vector);
	Next_exp = (struct DWCDB_repeat_expr *)Rblk->DWC$t_dbrv_data;
	for (i=1; i<=DWC$k_db_repeats_per_vector; i++)
	    {
	    if (Next_exp->DWC$b_dbre_flags & DWC$m_dbre_used)
		{
		DWC$$DB_Release_exceptions(Cab, Next_exp);
		}
	    Next_exp = (struct DWCDB_repeat_expr *)(
		(char *)Next_exp + DWC$k_db_repeat_expr_len);
	    }
	DWC$$DB_Freelist_buffer(Cab, Repvec_vm);
	}

    /*
    **  Return all repeat expression control blocks to CRTL.
    */
    while
    (
	DWC$$DB_Remque
	(
	    (struct DWC$db_queue_head *) &Cab->DWC$a_dcab_repeat_ctl_flink,
	    (struct DWC$db_queue_head **) &Repctl
	)
    )
	{
	XtFree (Repctl);
	}

    /*
    **  We're done. Back to caller
    */
    _Pop_cause;
    return (TRUE);
}

int
DWC$$DB_Locate_repeat_buf
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	struct DWC$db_repeat_control **Repc)
#else	/* no prototypes */
	(Cab, Repc)
	struct DWC$db_access_block	*Cab;
	struct DWC$db_repeat_control **Repc;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine locates a free repeat expression vector entry and creates
**	an associated repeat expression control block. If there are no more
**	free vector entries, a new (virtual) one will be created. The repeat
**	expression control block is not linked in with the rest of the repeat
**	expression control blocks. The link in is not done until the expression
**	is flushed out.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Repc : User pointer to receive pointer to repeat expression control
**	       block
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
**	TRUE	- Slot located
**	FALSE	- Failed to locate a free slot
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    VM_record *Next_vec_vm;		    /* Next virtual repeat vector   */
    int Rep_cluster;			    /* Repeat cluster sequence no   */
    struct DWCDB_repeat_vector *Next_vec;  /* Next real repeat vector	    */
    struct DWCDB_repeat_expr *Next_exp;    /* Next repeat expression	    */
    int i;				    /* Local loop variable	    */
    struct DWC$db_repeat_control *New_rep_ctl; /* New repeat control blk    */
    VM_record *Prev_vec_vm;		    /* Previous vector (blink)	    */
    struct DWCDB_repeat_vector *Prev_vec;  /* Previous real vector	    */
    VM_record *Headvm;			    /* Virtual header block	    */
    struct DWCDB_header *Head;		    /* Real header		    */



    /*
    **  Start by pointing at the first virtual vector. Assume that there
    **	are no vectors and enter scan
    */
    Next_vec_vm = Cab->DWC$a_dcab_repeat_vec_flink;
    Rep_cluster = -1;
    while (Next_vec_vm != (VM_record *)&Cab->DWC$a_dcab_repeat_vec_flink)
	{

	/*
	**  One (more) vector of expressions. Scan each individual vector
	**  entry.
	*/
	Rep_cluster++;
	Next_vec = _Bind(Next_vec_vm, DWCDB_repeat_vector);
	Next_exp = (struct DWCDB_repeat_expr *)Next_vec->DWC$t_dbrv_data;
	for (i=1; i<=DWC$k_db_repeats_per_vector; i++)
	    {

	    /*
	    **  Is this entry free?
	    */
	    if (!(Next_exp->DWC$b_dbre_flags & DWC$m_dbre_used))
		{

		/*
		**  Yes, allocate a bit of memory for it and fill it in.
		*/
		if ((New_rep_ctl = (struct DWC$db_repeat_control *)
			    XtMalloc (sizeof(*New_rep_ctl))) == 0)
		    {
		    _Record_error;
		    _Signal(DWC$_INSVIRMEM);
		    return (FALSE);
		    }
		Next_exp->DWC$l_dbre_repeat_interval2 = 0;
		New_rep_ctl->DWC$a_dbrc_flink = 0;
		New_rep_ctl->DWC$a_dbrc_vector_block = Next_vec_vm;
		New_rep_ctl->DWC$a_dbrc_vector_vm = Next_exp;
		New_rep_ctl->DWC$w_dbrc_id = -((Rep_cluster *
			DWC$k_db_repeats_per_vector) + i);

		/*
		**  Pass pointer to caller and return
		*/
		*Repc = New_rep_ctl;
		return (TRUE);
		}

	    /*
	    **  Nope. This entry was used. Skip to next.
	    */
	    Next_exp = (struct DWCDB_repeat_expr *)(
		    (char *)Next_exp + DWC$k_db_repeat_expr_len);
	    }

	/*
	**  All entries used in this vector. Skip to next.
	*/
	Next_vec_vm = Next_vec_vm->DWC$a_dbvm_vm_flink;
	}

    /*
    **  Ok. There are no free slots in any of the vectors. We have to create
    **	a new vector and an entry in that. Get a free VM buffer for the
    **	new vector.
    */
    DWC$$DB_Get_free_buffer_z(Cab, DWC$k_db_record_length, &Next_vec_vm);

    /*
    **  Setup for joining in with the rest of the vectors. Note the difference
    **	between first vector ever and second.
    */
    Prev_vec_vm = Cab->DWC$a_dcab_repeat_vec_blink;
    if (Prev_vec_vm != (VM_record *)&Cab->DWC$a_dcab_repeat_vec_flink)
	{
	Prev_vec = _Bind(Prev_vec_vm, DWCDB_repeat_vector);
	Next_vec_vm->DWC$a_dbvm_parent_vm = (char **)&Prev_vec->DWC$l_dbrv_flink;
	Next_vec_vm->DWC$a_dbvm_special = (char *)Prev_vec_vm;
	}
    else
	{
	Headvm = Cab->DWC$a_dcab_header;
	Head = _Bind(Headvm, DWCDB_header);
	Next_vec_vm->DWC$a_dbvm_parent_vm = (char **)&Head->DWC$l_dbhd_repeat_head;
	Next_vec_vm->DWC$a_dbvm_special = (char *)Headvm;
	}

    /*
    **  Fill block type and compute pointer to new expression
    */
    Next_vec = _Bind(Next_vec_vm, DWCDB_repeat_vector);
    Next_vec->DWC$b_dbrv_blocktype = DWC$k_db_repeat_expr_vec;
    Next_vec->DWC$w_dbrv_size = 1;
    Next_exp = (struct DWCDB_repeat_expr *)Next_vec->DWC$t_dbrv_data;

    /*
    **  Get a repeat expression control block and initialize it.
    */
    if ((New_rep_ctl = (struct DWC$db_repeat_control *)
		XtMalloc (sizeof(*New_rep_ctl))) == 0)
	{
	_Record_error;
	_Signal(DWC$_INSVIRMEM);
	DWC$$DB_Freelist_buffer(Cab, Next_vec_vm);
	return (FALSE);
	}
    New_rep_ctl->DWC$a_dbrc_flink = 0;
    New_rep_ctl->DWC$a_dbrc_vector_block = Next_vec_vm;
    New_rep_ctl->DWC$a_dbrc_vector_vm = Next_exp;
    New_rep_ctl->DWC$w_dbrc_id = -(((Rep_cluster + 1) *
		 DWC$k_db_repeats_per_vector) + 1);

    /*
    **  Done. Pass pointer to caller and return
    */
    *Repc = New_rep_ctl;
    return (TRUE);
}

int
DWC$$DB_Flush_repeat_buf
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	struct DWC$db_repeat_control *Repc)
#else	/* no prototypes */
	(Cab, Repc)
	struct DWC$db_access_block	*Cab;
	struct DWC$db_repeat_control *Repc;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine flushes out a repeat expression to disk. Physical disk
**	records are assigned to virtual buffers and data is written out. The
**	routine makes sure that the operation can be backed out at any time and
**	still have a consistent database (both VM and disk).
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Repc : Repeat expression control block, by ref, associated with
**	       expression to be written out.
**	
**  IMPLICIT INPUTS:
**
**      The first item linked off the repeat block will be used to create
**	the disk copy.
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION CODES:
**
**	DWC$k_db_normal	    -- Expression written
**	DWC$k_db_insdisk    -- Could not extend file to accomodate item
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

    struct DWCDB_repeat_block Work_blk;    /* Working copy of repeat block */
    struct DWCDB_repeat_expr *Rep_exp;	    /* Repeat expression vec entr   */
    VM_record *Rep_blo_vm;		    /* Virtual repeat block	    */
    struct DWCDB_repeat_block *Rep_blo;    /* Original repeat block	    */
    VM_record *Item_VM;			    /* Work item for flush	    */
    struct DWCDB_entry *Entry;		    /* Entry for repeat		    */
    int Remcnt;				    /* Remaining bytes for extend   */
    int Needed;				    /* Number of records for extend */
    VM_record *Ext_blk_vm;		    /* Virtual extension block	    */
    struct DWCDB_extension *Ext_data;	    /* Real extension block	    */
    int Phys_write;			    /* Result from extension write  */
    int Old_rep_addr;			    /* Old rec addr of repeat block */
    VM_record *Rep_vec_vm;		    /* Virtual repeat vector buffer */
    int Old_vec_addr;			    /* Old rec addr of repeat vect  */


    /*
    **  Point at repeat expression vector entry.
    */
    Rep_exp = Repc->DWC$a_dbrc_vector_vm;

    /*
    **  Is this an extended repeat block
    */
    if (Rep_exp->DWC$l_dbre_repeat_interval2 != 0)
	{
	
	/*
	**  Point at template item
	*/
	Rep_blo_vm = Make_VM_address(Rep_exp->DWC$l_dbre_repeat_interval2);
	Rep_blo = _Bind(Rep_blo_vm, DWCDB_repeat_block);
	Item_VM = (VM_record *)((char *)Rep_blo_vm->DWC$a_dbvm_vm_flink -
			    _Field_offset(Item_VM, DWC$a_dbvm_lru_flink));
	Entry = _Bind(Item_VM, DWCDB_entry);

	/*
	**  Fill in fixed part of repeat block (we're using a copy, so that
	**  we can recover in case of failure)
	*/
	memset(&Work_blk, 0, DWC$k_db_record_length);
	Work_blk.DWC$b_dbrb_blocktype = DWC$k_db_repeat_expr_block;
	Work_blk.DWC$b_dbrb_ext_sanity = Rep_blo->DWC$b_dbrb_ext_sanity + 1;
	Work_blk.DWC$w_dbrb_size = Entry->DWC$w_dben_size + 2
				    - _Field_offset(Entry, DWC$t_dben_data [0]);
	Work_blk.DWC$w_dbrb_text_class = Entry->DWC$w_dben_text_class;
	Work_blk.DWC$b_dbrb_flags = Entry->DWC$b_dben_flags;
	Work_blk.DWC$t_dbrb_data[0] = Work_blk.DWC$b_dbrb_ext_sanity;

	/*
	**  Will the template item fit into the repeat block?
	*/
	if (Work_blk.DWC$w_dbrb_size <= DWC$k_db_repeat_data_len)
	    {

	    /*
	    **  Yes. Indicate that we do not have an extension and move data
	    **	into repeat block
	    */
	    Work_blk.DWC$l_dbrb_flink = 0;
	    Work_blk.DWC$w_dbrb_ext_count = 0;
	    memcpy( &Work_blk.DWC$t_dbrb_data[1],
		    &Entry->DWC$t_dben_data[0],
		    Work_blk.DWC$w_dbrb_size - 2);
	    Work_blk.DWC$t_dbrb_data[Work_blk.DWC$w_dbrb_size - 1] =
		    Work_blk.DWC$b_dbrb_ext_sanity;
	    }
	else
	    {
	    
	    /*
	    **  Yes, we do need an extension. Fill the repeat block as much as
	    **	we can.
	    */
	    memcpy( &Work_blk.DWC$t_dbrb_data[1],
		    &Entry->DWC$t_dben_data[0],
		    DWC$k_db_repeat_data_len - 1);
	    
	    /*
	    **  Compute the number of extra bytes that we need and then compute
	    **	the number of records that corresponds to. Ask for the specified
	    **	number of records. Back out if it failed.
	    */
	    Remcnt = Work_blk.DWC$w_dbrb_size - DWC$k_db_repeat_data_len;
	    Needed = (int)(_Field_offset(Ext_data, DWC$t_dbex_data[0]));
	    Needed = (Remcnt + (DWC$k_db_record_length - 1)) /
		     DWC$k_db_record_length;
	    Work_blk.DWC$l_dbrb_flink = _UnPtr(DWC$$DB_Alloc_records(Cab, Needed));
	    if (Work_blk.DWC$l_dbrb_flink == 0)
		{
		return (DWC$k_db_insdisk);
		}

	    /*
	    **  Get a work buffer for the extension and fill it in.
	    */
	    DWC$$DB_Get_free_buffer(Cab, Needed * DWC$k_db_record_length,
			    &Ext_blk_vm);
	    Ext_data = _Bind(Ext_blk_vm, DWCDB_extension);
	    Ext_data->DWC$b_dbex_blocktype = DWC$k_db_repeat_extension;
	    Ext_data->DWC$w_dbex_ext_count = Needed;
	    Ext_data->DWC$b_dbex_sanity = Work_blk.DWC$b_dbrb_ext_sanity;
	    Work_blk.DWC$w_dbrb_ext_count = Needed;
	    memcpy( &Ext_data->DWC$t_dbex_data[0],
		    &Entry->DWC$t_dben_data[DWC$k_db_repeat_data_len - 1],
		    Remcnt - 1);
	    Ext_data->DWC$t_dbex_data[Remcnt - 1] =
		    Work_blk.DWC$b_dbrb_ext_sanity;
	    
	    /*
	    **  Write out the extension and release the VM extension buffer we
	    **	used for the write.
	    */
	    Phys_write = DWC$$DB_Write_physical_record
		(Cab, (CARD32)Work_blk.DWC$l_dbrb_flink, Needed, (char *)Ext_data);
	    DWC$$DB_Freelist_buffer(Cab, Ext_blk_vm);
	    if (Phys_write == _Failure)
		{
		DWC$$DB_Deallocate_records( Cab,
					    Needed,
					    (CARD32)Work_blk.DWC$l_dbrb_flink);
		return (DWC$k_db_failure);
		}
	    }
	
	/*
	**  It is now time to work on the repeat block itself. We are gong to
	**  allocate a new disk record for it so that the database is intact if
	**  we fail.
	*/
	Old_rep_addr = Rep_blo_vm->DWC$l_dbvm_rec_addr;
	Rep_blo_vm->DWC$l_dbvm_rec_addr = DWC$$DB_Alloc_records(Cab, 1);
	if (Rep_blo_vm->DWC$l_dbvm_rec_addr == 0)
	    {
	    if (Work_blk.DWC$l_dbrb_flink != 0)
		{
		DWC$$DB_Deallocate_records(	Cab,
					    Work_blk.DWC$w_dbrb_ext_count,
					    (CARD32)Work_blk.DWC$l_dbrb_flink);
		}
	    Rep_blo_vm->DWC$l_dbvm_rec_addr = Old_rep_addr;
	    return (DWC$k_db_insdisk);
	    }
	
	/*
	**  Write out the new repeat block.
	*/
	if (DWC$$DB_Write_physical_record
	    (Cab, (CARD32)Rep_blo_vm->DWC$l_dbvm_rec_addr, 1, (char *)&Work_blk) ==
	    _Failure)
	    {
	    DWC$$DB_Deallocate_records( Cab, 1,
					(CARD32)Rep_blo_vm->DWC$l_dbvm_rec_addr);
	    Rep_blo_vm->DWC$l_dbvm_rec_addr = Old_rep_addr;
	    if (Work_blk.DWC$l_dbrb_flink != 0)
		{
		DWC$$DB_Deallocate_records(	Cab,
					    Work_blk.DWC$w_dbrb_ext_count,
					    (CARD32)Work_blk.DWC$l_dbrb_flink);
		}
	    return (DWC$k_db_failure);
	    }
	
	/*
	**  At this point we should make sure that I/O buffers get flushed.
	*/
	_Write_back(Cab);
	}
    
    /*
    **  The repeated item is now store on disk (if any). It is now time to
    **	update the repeat expression itself. If we do not have a record assigned
    **	to this vector block, ask for it.
    */ 
    Rep_vec_vm = Repc->DWC$a_dbrc_vector_block;
    Old_vec_addr = Rep_vec_vm->DWC$l_dbvm_rec_addr;
    if (Rep_vec_vm->DWC$l_dbvm_rec_addr == 0)
	{
	Rep_vec_vm->DWC$l_dbvm_rec_addr = DWC$$DB_Alloc_records(Cab, 1);
	if (Rep_vec_vm->DWC$l_dbvm_rec_addr == 0)
	    {
	    if (Rep_exp->DWC$l_dbre_repeat_interval2 != 0)
		{
		DWC$$DB_Deallocate_records( Cab, 1,
					    (CARD32)Rep_blo_vm->DWC$l_dbvm_rec_addr);
		Rep_blo_vm->DWC$l_dbvm_rec_addr = Old_rep_addr;
		if (Work_blk.DWC$l_dbrb_flink != 0)
		    {
		    DWC$$DB_Deallocate_records(	Cab,
						Work_blk.DWC$w_dbrb_ext_count,
						(CARD32)Work_blk.DWC$l_dbrb_flink);
		    }
		}
	    return (DWC$k_db_insdisk);
	    }
	}
    
    /*
    **  Ok, write out the vector block. The repeat expression now points at the
    **	newly allocated vector block, if any.
    */
    Rep_exp->DWC$b_dbre_flags = (Rep_exp->DWC$b_dbre_flags |
				    DWC$m_dbre_used);
    if (DWC$$DB_Write_virtual_record(Cab, Rep_vec_vm) == _Failure)
	{
        Rep_exp->DWC$b_dbre_flags = (Rep_exp->DWC$b_dbre_flags &
				    ~DWC$m_dbre_used);
	if (Old_vec_addr == 0)
	    {
	    DWC$$DB_Deallocate_records(Cab, 1, (CARD32)Rep_vec_vm->DWC$l_dbvm_rec_addr);
	    Rep_vec_vm->DWC$l_dbvm_rec_addr = 0;
	    }
	if (Rep_exp->DWC$l_dbre_repeat_interval2 != 0)
	    {
	    DWC$$DB_Deallocate_records( Cab, 1,
					(CARD32)Rep_blo_vm->DWC$l_dbvm_rec_addr);
	    Rep_blo_vm->DWC$l_dbvm_rec_addr = Old_rep_addr;
	    if (Work_blk.DWC$l_dbrb_flink != 0)
		{
		DWC$$DB_Deallocate_records(	Cab,
					    Work_blk.DWC$w_dbrb_ext_count,
					    (CARD32)Work_blk.DWC$l_dbrb_flink);
		}
	    }
	return (DWC$k_db_failure);
	}
    
    /*
    **  We should now give back the records held for the old repeat expression,
    **	if any. Before we go about to delete the records we should make sure
    **	that the I/O buffers are flushed.
    */
    _Write_back(Cab);
    if (Rep_exp->DWC$l_dbre_repeat_interval2 != 0)
	{
	if (Rep_blo->DWC$l_dbrb_flink != 0)
	    {
	    DWC$$DB_Deallocate_records(Cab, Rep_blo->DWC$w_dbrb_ext_count,
					   (CARD32)Rep_blo->DWC$l_dbrb_flink);
	    }
	memcpy(Rep_blo, &Work_blk, DWC$k_db_record_length);
	if (Old_rep_addr == 0)
	    {
	    DWC$$DB_Cache_buffer(Cab, Rep_blo_vm);
	    }
	else
	    {
	    DWC$$DB_Deallocate_records(Cab, 1, (CARD32)Old_rep_addr);
	    }
	}
    
    /*
    **  If there was no record associated with the vector block, before this
    **	routine was called, it means that this vector block is new. Join this
    **	record with the rest of the structure and hook this new vector block
    **	in at the end of the VM vector queue.
    */
    if (Old_vec_addr == 0)
	{
	(*(int *)Rep_vec_vm->DWC$a_dbvm_parent_vm) =
	    _UnPtr((char *)Rep_vec_vm->DWC$l_dbvm_rec_addr);
	DWC$$DB_Write_virtual_record
	(
	    Cab,
	    (VM_record *) Rep_vec_vm->DWC$a_dbvm_special
	);
	DWC$$DB_Insque
	(
	    (struct DWC$db_queue_head *) Rep_vec_vm,
	    (struct DWC$db_queue_head *) Cab->DWC$a_dcab_repeat_vec_blink
	);
	}
    
    /*
    **  If this is a new repeat expression the repeat control block is not yet
    **	linked into the queue. Do that.
    */
    if (Repc->DWC$a_dbrc_flink == 0)
	{
	DWC$$DB_Insque
	(
	    (struct DWC$db_queue_head *) Repc,
	    (struct DWC$db_queue_head *) &(Cab->DWC$a_dcab_repeat_ctl_flink)
	);
	}
    return (DWC$k_db_normal);
}

int DWC$$DB_Add_repeat
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	int *Return_id,
	int Base_day,
	int Base_min,
	int End_day,
	int End_min,
	int Duration_min,
	int Alarm_vec_len,
	unsigned short int Alarm_vec[],
	int Flags,
	char Input_text[],
	int Input_text_len,
	int Input_text_class,
	int P1,
	int P2,
	int P3)
#else	/* no prototypes */
	(Cab, Return_id, Base_day, Base_min, End_day,
			       End_min, Duration_min, Alarm_vec_len, Alarm_vec,
			       Flags, Input_text, Input_text_len, Input_text_class,
			       P1, P2, P3)
	struct DWC$db_access_block	*Cab;
	int *Return_id;
	int Base_day;
	int Base_min;
	int End_day;
	int End_min;
	int Duration_min;
	int Alarm_vec_len;
	unsigned short int Alarm_vec[];
	int Flags;
	char Input_text[];
	int Input_text_len;
	int Input_text_class;
	int P1;
	int P2;
	int P3;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine adds a new repeat expression to the list of repeat
**	expressions. No checks are made to ensure that this repeat expression
**	does not conflict with any existing meeting or repeat expression.
**	The new repeat expression is also made available to the in-memory
**	structures maintained for days and entries.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Return_id : User field to receive the id of the new repeat expression
**	Base_day : Starting day for repeat expression
**	Base_min : Starting minute for repeat expression
**	End_day : Ending day for repeat (last)
**	End_min : Ending minute for repeat (last)
**	Duration_min : Duration of entry each time it is repeated
**	Alarm_vec_len : Length of alarm vector (number of entries)
**	Alarm_vec : Vector of alarms
**	Flags : Entry flags;
**		    DWC$m_item_insignif
**	Input_text : Pointer to user buffer containing the repeated text
**	Input_text_len : Length of the text (bytes)
**	Input_text_class : Type of textual information
**	P1 : Repeat parameter #1 (see special description above)
**	P2 : Repeat parameter #2 (see special description above)
**	P3 : Repeat parameter #3 (see special description above)
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

    struct DWC$db_repeat_control *Repc;	    /* Work repeat control block    */
    int Alarm_len;			    /* Length of alarm data	    */
    int Item_size;			    /* Bytes of item data	    */
    VM_record *Item_vm;			    /* Virtual item for expression  */
    struct DWCDB_entry *Entry;		    /* Entry for repeat expression  */
    struct DWCDB_repeat_expr *Rep_exp;	    /* Repeat exp vector entry	    */
    VM_record *Rep_blo_vm;		    /* Virtual repeat block	    */
    VM_record *Old_item_vm;		    /* VM ptr of item before modify */
    VM_record *Rep_vec_vm;		    /* VM pointer to repeat vector  */
    VM_record *Day_vm;			    /* Work daymap		    */
    int Temp_status;			    /* Temp work status		    */


    /*
    **  Save some trace in case we bomb out.
    */
    _Set_cause(DWC$_ADDRF);

    /*
    **  Compute the number of bytes for alarm data.
    */
    Alarm_len = 0;
    if (Alarm_vec_len != 0) Alarm_len = ((Alarm_vec_len * 2) + 1);
    
    /*
    **  Compute length of buffer. Determine if data will fit. If not,
    **	back to caller with a soft error
    */
    Item_size = (int)(_Field_offset(Entry,DWC$t_dben_data[0]) + Input_text_len +
		Alarm_len);
    if (Item_size > DWC$k_db_max_16bit_unsigned)
	{
	_Pop_cause;
	return (DWC$k_db_toobig);
	}    

    /*
    **  Ask for a fresh vector entry.
    */
    if (!DWC$$DB_Locate_repeat_buf(Cab, &Repc))
	{
	_Pop_cause;
	return (DWC$k_db_failure);
	}

    /*
    **  Get template item buffer.
    */
    DWC$$DB_Get_free_buffer_z(Cab, Item_size, &Item_vm);

    /*
    **  Fill in parts of the item's header
    */
    Entry = _Bind(Item_vm, DWCDB_entry);
    Entry->DWC$b_dben_blocktype = DWC$k_dbit_entry;
    Entry->DWC$w_dben_size = Item_size;
    Entry->DWC$b_dben_flags = DWC$m_dben_repeat;
    Entry->DWC$w_dben_delta_minutes = Duration_min;
    Entry->DWC$w_dben_text_class = Input_text_class;
    Entry->DWC$w_dben_entry_id = Repc->DWC$w_dbrc_id;

    /*
    **  Copy alarm data, if any
    */
    if (Alarm_len != 0)
	{
	Entry->DWC$b_dben_flags = (Entry->DWC$b_dben_flags |
				    DWC$m_dben_alarm_present);
	Entry->DWC$t_dben_data[0] = Alarm_vec_len;
	memcpy(&Entry->DWC$t_dben_data[1], Alarm_vec, (Alarm_len - 1));
	}

    /*
    **  Append text part.
    */
    memcpy(&Entry->DWC$t_dben_data[Alarm_len], Input_text, Input_text_len);

    /*
    **  Get a repeat block for this.
    */
    DWC$$DB_Get_free_buffer_z(  Cab,
				    DWC$k_db_record_length,
				    &Rep_blo_vm);

    /*
    **  Save backlink to repeat expression control block, for later use
    */
    Rep_blo_vm->DWC$l_dbvm_special2 = (unsigned long)Repc;
    
    /*
    **  Hook the template item off the repeat block
    */
    Rep_blo_vm->DWC$a_dbvm_vm_flink = (VM_record *)&Rep_blo_vm->DWC$a_dbvm_vm_flink;
    Rep_blo_vm->DWC$a_dbvm_vm_blink = (VM_record *)&Rep_blo_vm->DWC$a_dbvm_vm_flink;
    DWC$$DB_Insque
    (
	(struct DWC$db_queue_head *) &(Item_vm->DWC$a_dbvm_lru_flink),
	(struct DWC$db_queue_head *) &(Rep_blo_vm->DWC$a_dbvm_vm_flink)
    );

    /*
    **  Fill in the repeat expression itself
    */
    Rep_exp = Repc->DWC$a_dbrc_vector_vm;
    Rep_exp->DWC$l_dbre_baseday = Base_day;
    Rep_exp->DWC$w_dbre_basemin = Base_min;
    Rep_exp->DWC$l_dbre_endday = End_day;
    Rep_exp->DWC$w_dbre_endmin = End_min;
    Rep_exp->DWC$w_dbre_duration = Duration_min;
    Rep_exp->DWC$b_dbre_reptype = P1;
    Rep_exp->DWC$l_dbre_repeat_interval =
	DWC$$DB_Compress_params(Cab, P1, P2, P3, Base_day);
    Rep_exp->DWC$l_dbre_repeat_interval2 = _UnPtr(Make_VM_pointer(Rep_blo_vm));
    Rep_blo_vm->DWC$a_dbvm_parent_vm = (char **)&Rep_exp->DWC$l_dbre_repeat_interval2;
    Rep_exp->DWC$b_dbre_flags = 0;
    if ((End_day == 0) && (End_min == 0))
	{
	Rep_exp->DWC$b_dbre_flags = (Rep_exp->DWC$b_dbre_flags |
					DWC$m_dbre_open_ended);
	}
    if (Flags & DWC$m_item_insignif)
	{
	Rep_exp->DWC$b_dbre_flags = (Rep_exp->DWC$b_dbre_flags |
					DWC$m_dbre_insignif);
	Entry->DWC$b_dben_flags = (Entry->DWC$b_dben_flags |
				    DWC$m_dben_insignif);
	}

    /*
    **  Flush the data.
    */
    if ((Temp_status = DWC$$DB_Flush_repeat_buf(Cab, Repc)) != DWC$k_db_normal)
	{    
	DWC$$DB_Freelist_buffer(Cab, Rep_blo_vm);
	DWC$$DB_Freelist_buffer(Cab, Item_vm);
	Rep_vec_vm = Repc->DWC$a_dbrc_vector_block;
	if (Rep_vec_vm->DWC$l_dbvm_rec_addr == 0)
	    {
	    DWC$$DB_Freelist_buffer(Cab, Rep_vec_vm);
	    }
	else
	    {
	    Rep_exp->DWC$b_dbre_flags = (Rep_exp->DWC$b_dbre_flags &
					 ~DWC$m_dbre_used);
	    }	    
	XtFree (Repc);
	_Pop_cause;
	return (Temp_status);
	}

    /*
    **  Fine, copy repeat data into Repc
    */
    DWC$$DB_Expand_interval(Cab, Rep_exp, Repc);
    
    /*
    **  Increment running repeat context.
    */
    Cab->DWC$l_dcab_repeat_ctx++;

    /*
    **  Instansiate this expression into today, if there is a current
    **	day.
    */
    if (Cab->DWC$a_dcab_current_day_vm != 0)
	{
	struct DWC$db_time_ctx Timec;

	DWC$DB_Build_time_ctx(Cab, Cab->DWC$l_dcab_current_day, 0, &Timec);
	DWC$$DB_Set_centerpoint(Cab, Cab->DWC$l_dcab_current_day);
	if (DWC$$DB_Insert_one_repeat(Cab, Cab->DWC$a_dcab_current_day_vm,
				    Cab->DWC$l_dcab_current_day,
				    Repc, &Timec))
	    {
	    DWC$$DB_Determine_instances(Cab, Cab->DWC$a_dcab_current_day_vm,
				    Cab->DWC$l_dcab_current_day);
	    }
	Day_vm = Cab->DWC$a_dcab_current_day_vm;
	Day_vm->DWC$l_dbvm_special2 = Cab->DWC$l_dcab_repeat_ctx;
	}

    /*
    **  Pass id back to caller and return
    */
    *Return_id = Repc->DWC$w_dbrc_id;
    _Pop_cause;
    return (DWC$k_db_normal);
}

int DWCDB__AddRepeat
#ifdef	_DWC_PROTO_
(
    struct DWC$db_access_block	*Cab,
    int				*Return_id,
    int				Base_day,
    int				Base_min,
    int				End_day,
    int				End_min,
    int				Duration_min,
    int				Alarm_vec_len,
    unsigned short int		Alarm_vec[],
    int				Flags,
    unsigned char		*Input_text,		/* XmString */
    int				Input_text_class,
    unsigned int		Input_icons_num,
    unsigned char		Input_icon_ptr[],
    int				P1,
    int				P2,
    int				P3
)
#else	/* no prototypes */
(
    Cab, Return_id, Base_day, Base_min, End_day,
    End_min, Duration_min, Alarm_vec_len, Alarm_vec,
    Flags, Input_text, Input_text_class, Input_icons_num, Input_icon_ptr,
    P1, P2, P3
)
    struct DWC$db_access_block	*Cab;
    int				*Return_id;
    int				Base_day;
    int				Base_min;
    int				End_day;
    int				End_min;
    int				Duration_min;
    int				Alarm_vec_len;
    unsigned short int		Alarm_vec[];
    int				Flags;
    unsigned char		*Input_text;		/* XmString */
    int				Input_text_class;
    unsigned int		Input_icons_num;
    unsigned char		Input_icon_ptr[];
    int				P1;
    int				P2;
    int				P3;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine adds a new repeat expression to the list of repeat
**	expressions. No checks are made to ensure that this repeat expression
**	does not conflict with any existing meeting or repeat expression.
**	The new repeat expression is also made available to the in-memory
**	structures maintained for days and entries.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Return_id : User field to receive the id of the new repeat expression
**	Base_day : Starting day for repeat expression
**	Base_min : Starting minute for repeat expression
**	End_day : Ending day for repeat (last)
**	End_min : Ending minute for repeat (last)
**	Duration_min : Duration of entry each time it is repeated
**	Alarm_vec_len : Length of alarm vector (number of entries)
**	Alarm_vec : Vector of alarms
**	Flags : Entry flags;
**		    DWC$m_item_insignif
**	Input_text : Pointer to user buffer containing the repeated text
**	Input_text_len : Length of the text (bytes)
**	Input_text_class : Type of textual information
**	P1 : Repeat parameter #1 (see special description above)
**	P2 : Repeat parameter #2 (see special description above)
**	P3 : Repeat parameter #3 (see special description above)
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
    struct DWC$db_repeat_control *Repc;	    /* Work repeat control block    */
    int Alarm_len;			    /* Length of alarm data	    */
    int Item_size;			    /* Bytes of item data	    */
    int Item_header_size;		/* Fixed length header (item specif)*/
    char *Item_text;			/* Where to store text in item	    */
    VM_record *Item_vm;			    /* Virtual item for expression  */
    struct DWCDB_entry *Entry;		    /* Entry for repeat expression  */
    struct DWCDB_repeat_expr *Rep_exp;	    /* Repeat exp vector entry	    */
    VM_record *Rep_blo_vm;		    /* Virtual repeat block	    */
    VM_record *Old_item_vm;		    /* VM ptr of item before modify */
    VM_record *Rep_vec_vm;		    /* VM pointer to repeat vector  */
    VM_record *Day_vm;			    /* Work daymap		    */
    int Temp_status;			    /* Temp work status		    */
    char    *fc;
    long    byte_count, cvt_status;
    int	    icon_len;

    /*
    ** For now, all conversions are to fc.  At some point, this will be
    ** replaced with the appropriate target conversions.
    */
    if (Input_text_class != DWC$k_item_cstr)
	fc = (char *)DXmCvtCStoFC (Input_text, &byte_count, &cvt_status);
    else
	fc = (char *)DXmCvtCStoDDIF (Input_text, &byte_count, &cvt_status);

    /*
    **  Save some trace in case we bomb out.
    */
    _Set_cause(DWC$_ADDRF);

    /*
    **  Compute the number of bytes for alarm data.
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

    Item_size = Item_header_size + icon_len + byte_count + Alarm_len;
    if (Item_size > DWC$k_db_max_16bit_unsigned)
    {
	_Pop_cause;
	return (DWC$k_db_toobig);
    }    

    /*
    **  Ask for a fresh vector entry.
    */
    if (!DWC$$DB_Locate_repeat_buf(Cab, &Repc))
    {
	_Pop_cause;
	return (DWC$k_db_failure);
    }

    /*
    **  Get template item buffer.
    */
    DWC$$DB_Get_free_buffer_z(Cab, Item_size, &Item_vm);

    /*
    **  Fill in parts of the item's header
    */
    Entry = _Bind(Item_vm, DWCDB_entry);
    Item_text = &Entry->DWC$t_dben_data[0];
    Entry->DWC$b_dben_blocktype = DWC$k_dbit_entry;
    Entry->DWC$w_dben_size = Item_size;
    Entry->DWC$b_dben_flags = DWC$m_dben_repeat;
    Entry->DWC$w_dben_delta_minutes = Duration_min;
    Entry->DWC$w_dben_text_class = Input_text_class;
    Entry->DWC$w_dben_entry_id = Repc->DWC$w_dbrc_id;

    /*
    **  Copy alarm data, if any
    */
    if (Alarm_len != 0)
    {
	Entry->DWC$b_dben_flags = Entry->DWC$b_dben_flags |
	    DWC$m_dben_alarm_present;
	Item_text[0] = Alarm_vec_len;
	memcpy(&Item_text[1], Alarm_vec, (Alarm_len - 1));
    }

    /*
    **  Append text part.
    */
    if (Input_text_class != DWC$k_item_text)
    {
	Item_text[Alarm_len] = Input_icons_num;
	if (Input_icons_num != 0)
	{
	    memcpy(&Item_text[Alarm_len+1], Input_icon_ptr, Input_icons_num);
	}
    }
    memcpy(&Item_text[Alarm_len+icon_len], fc, byte_count);

    /*
    **  Get a repeat block for this.
    */
    DWC$$DB_Get_free_buffer_z (Cab, DWC$k_db_record_length, &Rep_blo_vm);

    /*
    **  Save backlink to repeat expression control block, for later use
    */
    Rep_blo_vm->DWC$l_dbvm_special2 = (unsigned long)Repc;
    
    /*
    **  Hook the template item off the repeat block
    */
    Rep_blo_vm->DWC$a_dbvm_vm_flink = (VM_record *)&Rep_blo_vm->DWC$a_dbvm_vm_flink;
    Rep_blo_vm->DWC$a_dbvm_vm_blink = (VM_record *)&Rep_blo_vm->DWC$a_dbvm_vm_flink;
    DWC$$DB_Insque
    (
	(struct DWC$db_queue_head *) &(Item_vm->DWC$a_dbvm_lru_flink),
	(struct DWC$db_queue_head *) &(Rep_blo_vm->DWC$a_dbvm_vm_flink)
    );

    /*
    **  Fill in the repeat expression itself
    */
    Rep_exp = Repc->DWC$a_dbrc_vector_vm;
    Rep_exp->DWC$l_dbre_baseday = Base_day;
    Rep_exp->DWC$w_dbre_basemin = Base_min;
    Rep_exp->DWC$l_dbre_endday = End_day;
    Rep_exp->DWC$w_dbre_endmin = End_min;
    Rep_exp->DWC$w_dbre_duration = Duration_min;
    Rep_exp->DWC$b_dbre_reptype = P1;
    Rep_exp->DWC$l_dbre_repeat_interval =
	DWC$$DB_Compress_params(Cab, P1, P2, P3, Base_day);
    Rep_exp->DWC$l_dbre_repeat_interval2 = _UnPtr(Make_VM_pointer(Rep_blo_vm));
    Rep_blo_vm->DWC$a_dbvm_parent_vm = (char **)&Rep_exp->DWC$l_dbre_repeat_interval2;
    Rep_exp->DWC$b_dbre_flags = 0;
    if ((End_day == 0) && (End_min == 0))
    {
	Rep_exp->DWC$b_dbre_flags = (Rep_exp->DWC$b_dbre_flags |
					DWC$m_dbre_open_ended);
    }
    if (Flags & DWC$m_item_insignif)
    {
	Rep_exp->DWC$b_dbre_flags = (Rep_exp->DWC$b_dbre_flags |
					DWC$m_dbre_insignif);
	Entry->DWC$b_dben_flags = (Entry->DWC$b_dben_flags |
				    DWC$m_dben_insignif);
    }

    /*
    **  Flush the data.
    */
    if ((Temp_status = DWC$$DB_Flush_repeat_buf(Cab, Repc)) != DWC$k_db_normal)
    {    
	DWC$$DB_Freelist_buffer(Cab, Rep_blo_vm);
	DWC$$DB_Freelist_buffer(Cab, Item_vm);
	Rep_vec_vm = Repc->DWC$a_dbrc_vector_block;
	if (Rep_vec_vm->DWC$l_dbvm_rec_addr == 0)
	{
	    DWC$$DB_Freelist_buffer(Cab, Rep_vec_vm);
	}
	else
	{
	    Rep_exp->DWC$b_dbre_flags = (Rep_exp->DWC$b_dbre_flags &
					 ~DWC$m_dbre_used);
	}	    
	XtFree (Repc);
	_Pop_cause;
	return (Temp_status);
    }

    /*
    **  Fine, copy repeat data into Repc
    */
    DWC$$DB_Expand_interval(Cab, Rep_exp, Repc);
    
    /*
    **  Increment running repeat context.
    */
    Cab->DWC$l_dcab_repeat_ctx++;

    /*
    **  Instansiate this expression into today, if there is a current
    **	day.
    */
    if (Cab->DWC$a_dcab_current_day_vm != 0)
    {
	struct DWC$db_time_ctx Timec;

	DWC$DB_Build_time_ctx(Cab, Cab->DWC$l_dcab_current_day, 0, &Timec);
	DWC$$DB_Set_centerpoint(Cab, Cab->DWC$l_dcab_current_day);
	if (DWC$$DB_Insert_one_repeat(Cab, Cab->DWC$a_dcab_current_day_vm,
				    Cab->DWC$l_dcab_current_day,
				    Repc, &Timec))
	{
	    DWC$$DB_Determine_instances(Cab, Cab->DWC$a_dcab_current_day_vm,
				    Cab->DWC$l_dcab_current_day);
	}
	Day_vm = Cab->DWC$a_dcab_current_day_vm;
	Day_vm->DWC$l_dbvm_special2 = Cab->DWC$l_dcab_repeat_ctx;
    }

    /*
    **  Pass id back to caller and return
    */
    *Return_id = Repc->DWC$w_dbrc_id;
    _Pop_cause;
    return (DWC$k_db_normal);
}

int DWC$$DB_Modify_repeat
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	int Repeat_id,
	int Curr_day,
	int Base_min,
	int End_day,
	int End_min,
	int Duration_min,
	int Alarm_vec_len,
	unsigned short int Alarm_vec[],
	int Flags,
	char *Input_text,
	int Input_text_len,
	int Input_text_class,
	int P1,
	int P2,
	int P3)
#else	/* no prototypes */
	(Cab, Repeat_id, Curr_day, Base_min,
				  End_day, End_min, Duration_min, Alarm_vec_len,
				  Alarm_vec, Flags, Input_text, Input_text_len,
				  Input_text_class, P1, P2, P3)
	struct DWC$db_access_block	*Cab;
	int Repeat_id;
	int Curr_day;
	int Base_min;
	int End_day;
	int End_min;
	int Duration_min;
	int Alarm_vec_len;
	unsigned short int Alarm_vec[];
	int Flags;
	char *Input_text;
	int Input_text_len;
	int Input_text_class;
	int P1;
	int P2;
	int P3;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine modifies an existing repeat expression. The old
**	instansiations of the repeat expression are removed from the virtual
**	days and the new data is made available for later retreival.
**
**	Before the expression is modified, the old expression is instansiated
**	up to the current time, thereby not loosing any old "floating"
**	instansiations of the old repeat expression.
**
**	Please note that the base-day cannot be modified.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Repeat_id : Id of repeat expression to modify
**	Curr_day : Current day (for recalculation of base month)
**	Base_min : Starting minute for repeat expression
**	End_day : Ending day for repeat (last)
**	End_min : Ending minute for repeat (last)
**	Duration_min : Duration of entry each time it is repeated
**	Alarm_vec_len : Length of alarm vector (number of entries)
**	Alarm_vec : Vector of alarms
**	Flags : Entry flags;
**		    DWC$m_item_insignif
**	Input_text : Pointer to user buffer containing the repeated text
**	Input_text_len : Length of the text (bytes)
**	Input_text_class : Type of textual information
**	P1 : Repeat parameter #1 (see special description above)
**	P2 : Repeat parameter #2 (see special description above)
**	P3 : Repeat parameter #3 (see special description above)
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

    struct DWC$db_repeat_control *Repc;	    /* Work repeat control block    */
    int Alarm_len;			    /* Length of alarm data	    */
    int Item_size;			    /* Bytes of item data	    */
    VM_record *Item_vm;			    /* Virtual item for expression  */
    struct DWCDB_entry *Entry;		    /* Entry for repeat expression  */
    struct DWCDB_repeat_expr *Rep_exp;	    /* Repeat exp vector entry	    */
    VM_record *Rep_blo_vm;		    /* Virtual repeat block	    */
    VM_record *Old_item_vm;		    /* VM ptr of item before modify */
    struct DWCDB_repeat_expr Rep_exp_saved;/* Copy of repeat expression    */
    VM_record *Day_vm;			    /* Work daymap		    */
    int Temp_status;			    /* Temp work status		    */

    
    /*
    **  Setup context, in case we fail
    */
    _Set_cause(DWC$_MODREPF);

    /*
    **  Determine how many bytes we need for alarm data
    */
    Alarm_len = 0;
    if (Alarm_vec_len != 0) Alarm_len = ((Alarm_vec_len * 2) + 1);
    
    /*
    **  Compute length of buffer. Determine if data will fit. If not,
    **	back to caller with a soft error
    */
    Item_size = (int)(_Field_offset(Entry,DWC$t_dben_data[0]) + Input_text_len +
		Alarm_len);
    if (Item_size > DWC$k_db_max_16bit_unsigned)
	{
	_Pop_cause;
	return (DWC$k_db_toobig);
	}    

    /*
    **  Locate the expression
    */
    if ((Temp_status = DWC$$DB_Find_repeat(Cab, Repeat_id, &Repc)) !=
		DWC$k_db_normal)
	{
	_Pop_cause;
	return (Temp_status);
	}

    /*
    **  Get buffer for new template item
    */
    DWC$$DB_Get_free_buffer_z(Cab, Item_size, &Item_vm);

    /*
    **  Fill in some of the header of the new template item
    */
    Entry = _Bind(Item_vm, DWCDB_entry);
    Entry->DWC$b_dben_blocktype = DWC$k_dbit_entry;
    Entry->DWC$w_dben_size = Item_size;
    Entry->DWC$b_dben_flags = DWC$m_dben_repeat;
    Entry->DWC$w_dben_delta_minutes = Duration_min;
    Entry->DWC$w_dben_text_class = Input_text_class;
    Entry->DWC$w_dben_entry_id = Repc->DWC$w_dbrc_id;

    /*
    **  If there is any alarm data, copy it into buffer and indicate that there
    **	is an alarm vector associated with it.
    */
    if (Alarm_len != 0)
	{
	Entry->DWC$b_dben_flags = (Entry->DWC$b_dben_flags |
				    DWC$m_dben_alarm_present);
	Entry->DWC$t_dben_data[0] = Alarm_vec_len;
	memcpy(&Entry->DWC$t_dben_data[1], Alarm_vec, (Alarm_len - 1));
	}

    /*
    **  Copy rest of text
    */
    memcpy(&Entry->DWC$t_dben_data[Alarm_len], Input_text, Input_text_len);

    /*
    **  Insert new template item in front of queue of items
    */
    Rep_exp = Repc->DWC$a_dbrc_vector_vm;
    Rep_blo_vm = Make_VM_address(Rep_exp->DWC$l_dbre_repeat_interval2);
    DWC$$DB_Insque
    (
	(struct DWC$db_queue_head *) &(Item_vm->DWC$a_dbvm_lru_flink),
	(struct DWC$db_queue_head *) &(Rep_blo_vm->DWC$a_dbvm_vm_flink)
    );

    /*
    **  Update repeat expression for new values (save copy first)
    */
    memcpy(&Rep_exp_saved, Rep_exp, DWC$k_db_repeat_expr_len);
    Rep_exp = Repc->DWC$a_dbrc_vector_vm;
    Rep_exp->DWC$w_dbre_basemin = Base_min;
    Rep_exp->DWC$l_dbre_endday = End_day;
    Rep_exp->DWC$w_dbre_endmin = End_min;
    Rep_exp->DWC$w_dbre_duration = Duration_min;
    Rep_exp->DWC$b_dbre_reptype = P1;
    Rep_exp->DWC$l_dbre_repeat_interval =
	DWC$$DB_Compress_params(Cab, P1, P2, P3, Curr_day);
    Rep_exp->DWC$b_dbre_flags = 0;
    if ((End_day == 0) && (End_min == 0))
	{
	Rep_exp->DWC$b_dbre_flags = (Rep_exp->DWC$b_dbre_flags |
					DWC$m_dbre_open_ended);
	}	
    if (Flags & DWC$m_item_insignif)
	{
	Rep_exp->DWC$b_dbre_flags = (Rep_exp->DWC$b_dbre_flags |
					DWC$m_dbre_insignif);
	Entry->DWC$b_dben_flags = (Entry->DWC$b_dben_flags |
				    DWC$m_dben_insignif);
	}


    /*
    **  Write this out. Restore if we failed.
    */
    if ((Temp_status = DWC$$DB_Flush_repeat_buf(Cab, Repc)) != DWC$k_db_normal)
	{    
	DWC$$DB_Remque
	(
	    (struct DWC$db_queue_head *) Item_vm->DWC$a_dbvm_lru_blink,
	    (struct DWC$db_queue_head **) 0
	);
	DWC$$DB_Freelist_buffer(Cab, Item_vm);
	memcpy(Rep_exp, &Rep_exp_saved, DWC$k_db_repeat_expr_len);
	_Pop_cause;
	return (Temp_status);
	}

    /*
    **  Update repeat control block with new params
    */
    DWC$$DB_Expand_interval(Cab, Rep_exp, Repc);
    
    /*
    **  Ok. Get rid of previous virtual instansiations
    */
    DWC$$DB_Back_out_repeats(Cab, Rep_blo_vm);

    /*
    **  Get rid of old template item and bump context counter.
    */
    DWC$$DB_Remque
    (
	(struct DWC$db_queue_head *) Rep_blo_vm->DWC$a_dbvm_vm_flink,
	(struct DWC$db_queue_head **) &Old_item_vm
    );
    Old_item_vm = (VM_record *)((char *)Old_item_vm -
			    _Field_offset(Old_item_vm, DWC$a_dbvm_lru_flink));
    DWC$$DB_Freelist_buffer(Cab, Old_item_vm);
    Cab->DWC$l_dcab_repeat_ctx++;

    /*
    **  If we have a current day, insert this in the current day
    */
    if (Cab->DWC$a_dcab_current_day_vm != 0)
	{
	struct DWC$db_time_ctx Timec;

	DWC$DB_Build_time_ctx(Cab, Cab->DWC$l_dcab_current_day, 0, &Timec);
	DWC$$DB_Set_centerpoint(Cab, Cab->DWC$l_dcab_current_day);
	if (DWC$$DB_Insert_one_repeat(Cab, Cab->DWC$a_dcab_current_day_vm,
				    Cab->DWC$l_dcab_current_day,
				    Repc, &Timec))
	    {
	    DWC$$DB_Determine_instances(Cab, Cab->DWC$a_dcab_current_day_vm,
				    Cab->DWC$l_dcab_current_day);
	    }
	Day_vm = Cab->DWC$a_dcab_current_day_vm;
	Day_vm->DWC$l_dbvm_special2 = Cab->DWC$l_dcab_repeat_ctx;
	}

    /*
    **  Done, back to caller
    */
    _Pop_cause;
    return (DWC$k_db_normal);
}

int DWCDB__ModifyRepeat
#ifdef	_DWC_PROTO_
(
    struct DWC$db_access_block	*Cab,
    int				Repeat_id,
    int				Curr_day,
    int				Base_min,
    int				End_day,
    int				End_min,
    int				Duration_min,
    int				Alarm_vec_len,
    unsigned short int		Alarm_vec[],
    int				Flags,
    unsigned char		*Input_text,		/* XmString */
    int				Input_text_class,
    unsigned int		Input_icons_num,
    unsigned char		Input_icon_ptr[],
    int			P1,
    int				P2,
    int				P3
)
#else	/* no prototypes */
(
    Cab, Repeat_id, Curr_day, Base_min,
    End_day, End_min, Duration_min, Alarm_vec_len,
    Alarm_vec, Flags,
    Input_text, Input_text_class, Input_icons_num, Input_icon_ptr,
    P1, P2, P3
)
    struct DWC$db_access_block	*Cab;
    int				Repeat_id;
    int				Curr_day;
    int				Base_min;
    int				End_day;
    int				End_min;
    int				Duration_min;
    int				Alarm_vec_len;
    unsigned short int		Alarm_vec[];
    int				Flags;
    unsigned char		*Input_text;		/* XmString */
    int				Input_text_class;
    unsigned int		Input_icons_num;
    unsigned char		Input_icon_ptr[];
    int				P1;
    int				P2;
    int				P3;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine modifies an existing repeat expression. The old
**	instansiations of the repeat expression are removed from the virtual
**	days and the new data is made available for later retreival.
**
**	Before the expression is modified, the old expression is instansiated
**	up to the current time, thereby not loosing any old "floating"
**	instansiations of the old repeat expression.
**
**	Please note that the base-day cannot be modified.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Repeat_id : Id of repeat expression to modify
**	Curr_day : Current day (for recalculation of base month)
**	Base_min : Starting minute for repeat expression
**	End_day : Ending day for repeat (last)
**	End_min : Ending minute for repeat (last)
**	Duration_min : Duration of entry each time it is repeated
**	Alarm_vec_len : Length of alarm vector (number of entries)
**	Alarm_vec : Vector of alarms
**	Flags : Entry flags;
**		    DWC$m_item_insignif
**	Input_text : Pointer to user buffer containing the repeated text
**	Input_text_len : Length of the text (bytes)
**	Input_text_class : Type of textual information
**	P1 : Repeat parameter #1 (see special description above)
**	P2 : Repeat parameter #2 (see special description above)
**	P3 : Repeat parameter #3 (see special description above)
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

    struct DWC$db_repeat_control	*Repc;
    int					Alarm_len;
    int					Item_size;
    int					Item_header_size;
    char				*Item_text;
    VM_record *Item_vm;			    /* Virtual item for expression  */
    struct DWCDB_entry *Entry;		    /* Entry for repeat expression  */
    struct DWCDB_repeat_expr *Rep_exp;	    /* Repeat exp vector entry	    */
    VM_record *Rep_blo_vm;		    /* Virtual repeat block	    */
    VM_record *Old_item_vm;		    /* VM ptr of item before modify */
    struct DWCDB_repeat_expr Rep_exp_saved;/* Copy of repeat expression    */
    VM_record *Day_vm;			    /* Work daymap		    */
    int Temp_status;			    /* Temp work status		    */
    char    *fc;
    long    byte_count, cvt_status;
    int	    icon_len;


    /*
    ** For now, all conversions are to fc.  At some point, this will be
    ** replaced with the appropriate target conversions.
    */
    if (Input_text_class != DWC$k_item_cstr)
	fc = (char *)DXmCvtCStoFC (Input_text, &byte_count, &cvt_status);
    else
	fc = (char *)DXmCvtCStoDDIF (Input_text, &byte_count, &cvt_status);

    
    /*
    **  Setup context, in case we fail
    */
    _Set_cause(DWC$_MODREPF);

    /*
    **  Determine how many bytes we need for alarm data
    */
    Item_header_size = _Field_offset(Entry,DWC$t_dben_data[0]);
    if (Alarm_vec_len != 0)
	Alarm_len = ((Alarm_vec_len * 2) + 1);
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
    **  Locate the expression
    */
    Temp_status = DWC$$DB_Find_repeat(Cab, Repeat_id, &Repc);
    if (Temp_status != DWC$k_db_normal)
    {
	_Pop_cause;
	return (Temp_status);
    }

    /*
    **  Get buffer for new template item
    */
    DWC$$DB_Get_free_buffer_z(Cab, Item_size, &Item_vm);

    /*
    **  Fill in some of the header of the new template item
    */
    Entry = _Bind(Item_vm, DWCDB_entry);
    Item_text = &Entry->DWC$t_dben_data[0];
    Entry->DWC$b_dben_blocktype = DWC$k_dbit_entry;
    Entry->DWC$w_dben_size = Item_size;
    Entry->DWC$b_dben_flags = DWC$m_dben_repeat;
    Entry->DWC$w_dben_delta_minutes = Duration_min;
    Entry->DWC$w_dben_text_class = Input_text_class;
    Entry->DWC$w_dben_entry_id = Repc->DWC$w_dbrc_id;

    /*
    **  If there is any alarm data, copy it into buffer and indicate that there
    **	is an alarm vector associated with it.
    */
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
	    memcpy(&Item_text[Alarm_len+1], Input_icon_ptr, Input_icons_num);
	}
    }
    /*
    **  Copy rest of text
    */
    memcpy(&Entry->DWC$t_dben_data[Alarm_len+icon_len], fc, byte_count);

    /*
    **  Insert new template item in front of queue of items
    */
    Rep_exp = Repc->DWC$a_dbrc_vector_vm;
    Rep_blo_vm = Make_VM_address(Rep_exp->DWC$l_dbre_repeat_interval2);
    DWC$$DB_Insque
    (
	(struct DWC$db_queue_head *) &(Item_vm->DWC$a_dbvm_lru_flink),
	(struct DWC$db_queue_head *) &(Rep_blo_vm->DWC$a_dbvm_vm_flink)
    );

    /*
    **  Update repeat expression for new values (save copy first)
    */
    memcpy(&Rep_exp_saved, Rep_exp, DWC$k_db_repeat_expr_len);
    Rep_exp = Repc->DWC$a_dbrc_vector_vm;
    Rep_exp->DWC$w_dbre_basemin = Base_min;
    Rep_exp->DWC$l_dbre_endday = End_day;
    Rep_exp->DWC$w_dbre_endmin = End_min;
    Rep_exp->DWC$w_dbre_duration = Duration_min;
    Rep_exp->DWC$b_dbre_reptype = P1;
    Rep_exp->DWC$l_dbre_repeat_interval =
	DWC$$DB_Compress_params (Cab, P1, P2, P3, Curr_day);
    Rep_exp->DWC$b_dbre_flags = 0;
    if ((End_day == 0) && (End_min == 0))
    {
	Rep_exp->DWC$b_dbre_flags = (Rep_exp->DWC$b_dbre_flags |
					DWC$m_dbre_open_ended);
    }	
    if (Flags & DWC$m_item_insignif)
    {
	Rep_exp->DWC$b_dbre_flags = (Rep_exp->DWC$b_dbre_flags |
					DWC$m_dbre_insignif);
	Entry->DWC$b_dben_flags = (Entry->DWC$b_dben_flags |
				    DWC$m_dben_insignif);
    }


    /*
    **  Write this out. Restore if we failed.
    */
    if ((Temp_status = DWC$$DB_Flush_repeat_buf(Cab, Repc)) != DWC$k_db_normal)
    {    
	DWC$$DB_Remque
	(
	    (struct DWC$db_queue_head *) Item_vm->DWC$a_dbvm_lru_blink,
	    (struct DWC$db_queue_head **) 0
	);
	DWC$$DB_Freelist_buffer(Cab, Item_vm);
	memcpy(Rep_exp, &Rep_exp_saved, DWC$k_db_repeat_expr_len);
	_Pop_cause;
	return (Temp_status);
    }

    /*
    **  Update repeat control block with new params
    */
    DWC$$DB_Expand_interval(Cab, Rep_exp, Repc);
    
    /*
    **  Ok. Get rid of previous virtual instansiations
    */
    DWC$$DB_Back_out_repeats(Cab, Rep_blo_vm);

    /*
    **  Get rid of old template item and bump context counter.
    */
    DWC$$DB_Remque
    (
	(struct DWC$db_queue_head *) Rep_blo_vm->DWC$a_dbvm_vm_flink,
	(struct DWC$db_queue_head **) &Old_item_vm
    );
    Old_item_vm = (VM_record *)((char *)Old_item_vm -
			    _Field_offset(Old_item_vm, DWC$a_dbvm_lru_flink));
    DWC$$DB_Freelist_buffer(Cab, Old_item_vm);
    Cab->DWC$l_dcab_repeat_ctx++;

    /*
    **  If we have a current day, insert this in the current day
    */
    if (Cab->DWC$a_dcab_current_day_vm != 0)
    {
	struct DWC$db_time_ctx Timec;

	DWC$DB_Build_time_ctx(Cab, Cab->DWC$l_dcab_current_day, 0, &Timec);
	DWC$$DB_Set_centerpoint(Cab, Cab->DWC$l_dcab_current_day);
	if (DWC$$DB_Insert_one_repeat
	    (
		Cab,
		Cab->DWC$a_dcab_current_day_vm,
		Cab->DWC$l_dcab_current_day,
		Repc,
		&Timec
	    ))
	{
	    DWC$$DB_Determine_instances
	    (
		Cab,
		Cab->DWC$a_dcab_current_day_vm,
		Cab->DWC$l_dcab_current_day
	    );
	}
	Day_vm = Cab->DWC$a_dcab_current_day_vm;
	Day_vm->DWC$l_dbvm_special2 = Cab->DWC$l_dcab_repeat_ctx;
    }

    /*
    **  Done, back to caller
    */
    _Pop_cause;
    return (DWC$k_db_normal);
}

int
DWC$$DB_Delete_repeat
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	int Repeat_id)
#else	/* no prototypes */
	(Cab, Repeat_id)
	struct DWC$db_access_block	*Cab;
	int Repeat_id;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine deletes all instances of specified repeat expression.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Repeat_id : Id of repeat expression to delete
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
**	DWC$k_db_insdisk    -- Could not extend file to accomodate "safe" space
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
    struct DWC$db_repeat_control *Repc;	    /* Repeat control block	    */
    int Sts;				    /* Work status		    */
    int Temp_status;			    /* Temp work status		    */


    /*
    **  Get ready for failure.
    */
    _Set_cause(DWC$_DELREPF);

    /*
    **  Locate expression
    */
    if ((Temp_status = DWC$$DB_Find_repeat(Cab, Repeat_id, &Repc)) !=
		DWC$k_db_normal)
	{
	_Pop_cause;
	return (Temp_status);
	}

    /*
    **  Get rid of it
    */
    Sts = DWC$$DB_Free_expr(Cab, Repc);

    /*
    **  Update repeat context
    */
    Cab->DWC$l_dcab_repeat_ctx++;
    
    /*
    **  And exit
    */
    _Pop_cause;
    if (Sts)
	{
	return (DWC$k_db_normal);
	}
    else
	{
	return (DWC$k_db_failure);
	}
}

int
DWC$$DB_Find_repeat
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	int Repeat_id,
	struct DWC$db_repeat_control **Repc)
#else	/* no prototypes */
	(Cab, Repeat_id, Repc)
	struct DWC$db_access_block	*Cab;
	int Repeat_id;
	struct DWC$db_repeat_control **Repc;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine routine locates a repeat expression by its unique id. A
**	pointer to the associated repeat expression control block is returned.
**
**	When the requested repeat expression is found, it is brought into
**	memory.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Repeat_id : Id of repeat expression to locate
**	Repc : User pointer to receive pointer to repeat expression control
**	       block just located
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
**	DWC$k_db_normal	    -- Item located
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
    struct DWC$db_repeat_control *Rep_ctl;  /* Work repeat control expression */
    
    /*
    **  Start with first control block and then scan for desired item.
    */
    Rep_ctl = Cab->DWC$a_dcab_repeat_ctl_flink;
    while (Rep_ctl != (struct DWC$db_repeat_control *)&Cab->DWC$a_dcab_repeat_ctl_flink)
	{
	if (Rep_ctl->DWC$w_dbrc_id == Repeat_id)
	    {

	    /*
	    **  We got it. Bring it into memory.
	    */
	    *Repc = Rep_ctl;
	    if (DWC$$DB_Page_in_repeat(Cab, Rep_ctl))
		{
		return (DWC$k_db_normal);
		}
	    else
		{
		return (DWC$k_db_failure);
		}
	    }
	Rep_ctl = Rep_ctl->DWC$a_dbrc_flink;
	}

    /*
    **  Nope. Not found. This was not expected.
    */
    _Signal(DWC$_NOSUCHREP);
    return (DWC$k_db_failure);
}

int
DWC$$DB_Page_in_repeat
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	struct DWC$db_repeat_control *Repc)
#else	/* no prototypes */
	(Cab, Repc)
	struct DWC$db_access_block	*Cab;
	struct DWC$db_repeat_control *Repc;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine routine brings in the rest of a repeat expression into
**	memory, based on the repeat expression vector entry. If the rest of
**	the data is already in memory, this routine will not do anything.
**
**	A virtual buffer is allocated and the repeat expression block is
**	brought in. An item buffer is then built from the repeat expression
**	block (and its extension, if any). The item is then hooked off the
**	repeat expression block.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Repc : Repeat expression control block
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
**	TRUE	- Repeat expression paged in
**	FALSE	- Failed to bring in specified repeat expression
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{			    
    VM_record *Rep_blo_vm;		    /* Virtual repeat block	    */
    struct DWCDB_repeat_block *Rep_blo;    /* Real repeat block	    */
    int Item_size;			    /* Number of databytes in item  */
    VM_record *Item_vm;			    /* Virtual item for repeat	    */
    struct DWCDB_entry *Entry;		    /* Entry part of item	    */
    VM_record *Ext_buf_vm;		    /* Extension record for repeat  */
    struct DWCDB_extension *Ext_buf;	    /* Real extension buffer	    */
    int Data_size;			    /* Number of databytes	    */
    struct DWCDB_repeat_expr *Rep_exp;	    /* Repeat expr vector entry	    */

    /*
    **  First of all. Does this expression have a virtual extension. If so,
    **	make sure that it is not already in memory.
    */
    Rep_exp = Repc->DWC$a_dbrc_vector_vm;
    if ((Rep_exp->DWC$l_dbre_repeat_interval2 != 0) &&
	(!Is_VM_pointer(Rep_exp->DWC$l_dbre_repeat_interval2)))
	{

	/*
	**  Ok. There is an extension and it is not in memory. Bring it it.
	*/
	Rep_blo_vm = _Follow(	&Rep_exp->DWC$l_dbre_repeat_interval2,
				DWC$k_db_repeat_expr_block);

	/*
	**  Save backlink to repeat expression control block
	*/
	Rep_blo_vm->DWC$l_dbvm_special2 = (unsigned long)Repc;
	
	/*
	**  Determine the size of the datapart of the item buffer and ask for
	**  an item buffer.
	*/
	Rep_blo = _Bind(Rep_blo_vm, DWCDB_repeat_block);
	Data_size = Rep_blo->DWC$w_dbrb_size - 2;
	Item_size = Data_size + _Field_offset(Entry, DWC$t_dben_data [0]);
	DWC$$DB_Get_free_buffer_z(Cab, Item_size, &Item_vm);

	/*
	**  Fill in header of day entry
	*/
	Entry = _Bind(Item_vm, DWCDB_entry);
	Entry->DWC$b_dben_blocktype = DWC$k_dbit_entry;
	Entry->DWC$w_dben_size = Item_size;
	Entry->DWC$b_dben_flags = 0;
	if (Rep_blo->DWC$b_dbrb_flags & DWC$m_dbrb_alarm)
	    {
	    Entry->DWC$b_dben_flags = DWC$m_dben_alarm_present;
	    }
	if (Rep_blo->DWC$b_dbrb_flags & DWC$m_dbrb_repeat)
	    {
	    Entry->DWC$b_dben_flags = (Entry->DWC$b_dben_flags |
					DWC$m_dben_repeat);
	    }
	if (Rep_exp->DWC$b_dbre_flags & DWC$m_dbre_insignif)
	    {
	    Entry->DWC$b_dben_flags = (Entry->DWC$b_dben_flags |
					DWC$m_dben_insignif);
	    }
	    
	Entry->DWC$w_dben_text_class = Rep_blo->DWC$w_dbrb_text_class;
	Entry->DWC$w_dben_entry_id = Repc->DWC$w_dbrc_id;
	Entry->DWC$w_dben_delta_minutes = Rep_exp->DWC$w_dbre_duration;

	/*
	**  If repeat block does not have an extension, then this is
	**  easy. Just validate the checksum and move the data.
	*/
	if (Rep_blo->DWC$l_dbrb_flink == 0)
	    {
	    if ((Rep_blo->DWC$t_dbrb_data[0] !=
			Rep_blo->DWC$b_dbrb_ext_sanity) ||
	        (Rep_blo->DWC$t_dbrb_data[Data_size + 1] !=
			Rep_blo->DWC$b_dbrb_ext_sanity))
		{
		_Signal(DWC$_BADRCHK);
		return (FALSE);
		}
	    memcpy( &Entry->DWC$t_dben_data[0],
		    &Rep_blo->DWC$t_dbrb_data[1],
		    Data_size);
	    }
	else
	    {

	    /*
	    **  Bring in extension too. Validate sanity fields and move
	    **	data from extension buffer to item.
	    */
	    Ext_buf_vm = DWC$$DB_Read_physical_record(
			    Cab,
			    (CARD32)Rep_blo->DWC$l_dbrb_flink,
			    Rep_blo->DWC$w_dbrb_ext_count,
			    DWC$k_db_repeat_extension);
	    Ext_buf = _Bind(Ext_buf_vm, DWCDB_extension);
	    if ((Rep_blo->DWC$t_dbrb_data[0] !=
			Rep_blo->DWC$b_dbrb_ext_sanity) ||
		(Ext_buf->DWC$t_dbex_data[  Data_size -
					    DWC$k_db_repeat_data_len +
					    1] !=
			Rep_blo->DWC$b_dbrb_ext_sanity) ||
		(Ext_buf->DWC$b_dbex_sanity !=
			Rep_blo->DWC$b_dbrb_ext_sanity) ||
		(Ext_buf->DWC$w_dbex_ext_count !=
			Rep_blo->DWC$w_dbrb_ext_count))
		{
		_Signal(DWC$_BADRCHK);
		return (FALSE);
		}
	    memcpy( &Entry->DWC$t_dben_data[0],
		    &Rep_blo->DWC$t_dbrb_data[1],
		    DWC$k_db_repeat_data_len - 1);
	    memcpy( &Entry->DWC$t_dben_data[DWC$k_db_repeat_data_len - 1],
		    &Ext_buf->DWC$t_dbex_data[0],
		    Data_size - (DWC$k_db_repeat_data_len - 1));
	    DWC$$DB_Freelist_buffer(Cab, Ext_buf_vm);
	    }

	/*
	**  Make this the template item
	*/
	Rep_blo_vm->DWC$a_dbvm_vm_flink = (VM_record *)&Rep_blo_vm->DWC$a_dbvm_vm_flink;
	Rep_blo_vm->DWC$a_dbvm_vm_blink = (VM_record *)&Rep_blo_vm->DWC$a_dbvm_vm_flink;
	DWC$$DB_Insque
	(
	    (struct DWC$db_queue_head *) &(Item_vm->DWC$a_dbvm_lru_flink),
	    (struct DWC$db_queue_head *) &(Rep_blo_vm->DWC$a_dbvm_vm_flink)
	);
	}

    /*
    **  Done, back to caller
    */
    return (TRUE);
}

int
DWC$$DB_Next_repeat_after
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	struct DWCDB_repeat_expr *Rep_exp,
	unsigned int After_day,
	unsigned int After_min,
	unsigned int *Next_day,
	unsigned int *Next_min)
#else	/* no prototypes */
	(Cab, Rep_exp, After_day, After_min, Next_day, Next_min)
	struct DWC$db_access_block	*Cab;
	struct DWCDB_repeat_expr *Rep_exp;
	unsigned int After_day;
	unsigned int After_min;
	unsigned int *Next_day;
	unsigned int *Next_min;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine computes the next time a specific repeat expression is
**	to be triggered, after a given time.
**
**	Please note that this routine will skip instansiations of the repeat
**	expression that causes the repeated item to cross midnight.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Rep_exp : Repeat expression
**	After_day : Starting day for which expression is desired
**	After_min ; Starting minute for which expression is desired
**	Next_day : Computed next instansiation day
**	Next_min : Computed next instansiation min
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
**	TRUE	- Next instansiation computed
**	FALSE	- No more instansiations
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    unsigned int Absolute_min;	    /* Base minute (since zero) of next	    */
    unsigned int After_abs_min;	    /* Suggested base for repeat	    */
    unsigned int Delta;		    /* Delta minutes for increase to next   */
    unsigned int End_abs_min;	    /* Absolute ending minute		    */
    unsigned int New_absolute_min;  /* New absolute min for expr	    */
    unsigned int Diff;		    /* Difference between suggested and real*/
    unsigned int Prev_day_checked;  /* Previous day checked for exception   */
    unsigned int New_day;	    /* Next computed day		    */

    /*
    **  Compute absolute minute for next instansiation.
    */
    Absolute_min = (Rep_exp->DWC$l_dbre_baseday * DWC$k_db_calendar_precision)
			+ Rep_exp->DWC$w_dbre_basemin;

    /*
    **  Compute time from which we're going to start looking. If time is before
    **	our current base time, then use the one that we already have, else
    **	compute the time of the one immediately following the suggested time.
    */
    After_abs_min = (After_day * DWC$k_db_calendar_precision) + After_min;
    if (After_abs_min > Absolute_min)
	{
	Diff = (After_abs_min - Absolute_min) +
		    (Rep_exp->DWC$l_dbre_repeat_interval - 1);
	Diff = (Diff / Rep_exp->DWC$l_dbre_repeat_interval) *
		    Rep_exp->DWC$l_dbre_repeat_interval;
	Absolute_min = Absolute_min + Diff;
	}

    /*
    **  Start looking for the first one that does not cross midnight.
    */
    Prev_day_checked = 0;
    for (Delta=0; ;Delta+=Rep_exp->DWC$l_dbre_repeat_interval)
	{
	New_absolute_min = Absolute_min + Delta;
	if (((New_absolute_min % DWC$k_db_calendar_precision) +
		Rep_exp->DWC$w_dbre_duration) <=
		    DWC$k_db_calendar_precision)
	    {
	    New_day = New_absolute_min / DWC$k_db_calendar_precision;
	    if (New_day != Prev_day_checked)
		{
		if (!DWC$$DB_Check_exception(Cab, Rep_exp, New_day))
		    {
		    break;
		    }
		Prev_day_checked = New_day;
		}
	    }
	}

    /*
    **  Make sure that this is not past the end of the repeat. If not, pass the
    **	computed time back to caller.
    */
    End_abs_min = (Rep_exp->DWC$l_dbre_endday * DWC$k_db_calendar_precision) +
		    Rep_exp->DWC$w_dbre_endmin;
    if ((End_abs_min != 0) && (New_absolute_min > End_abs_min))
	{
	return (FALSE);
	}
    else
	{
	*Next_day = New_absolute_min / DWC$k_db_calendar_precision;
	*Next_min = New_absolute_min % DWC$k_db_calendar_precision;
	return (TRUE);	
	}	
}

int
DWC$$DB_Next_repeat
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	struct DWCDB_repeat_expr *Rep_exp,
	unsigned int Base_day,
	unsigned int Base_min,
	unsigned int *Next_day,
	unsigned int *Next_min)
#else	/* no prototypes */
	(Cab, Rep_exp, Base_day, Base_min, Next_day, Next_min)
	struct DWC$db_access_block	*Cab;
	struct DWCDB_repeat_expr *Rep_exp;
	unsigned int Base_day;
	unsigned int Base_min;
	unsigned int *Next_day;
	unsigned int *Next_min;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine computes the next time a specific repeat expression is
**	to be triggered, relative to a given base time.
**
**	Please note that this routine will skip instansiations of the repeat
**	expression that causes the repeated item to cross midnight.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Rep_exp : Repeat expression vector block
**	Base_day : Start day used for evaluating the next instansiation
**	Base_min : Base minute used for evaluating the next instansiation
**	Next_day : Computed next instansiation day
**	Next_min : Computed next instansiation min
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
**	TRUE	- Next instansiation computed
**	FALSE	- No more instansiations
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    unsigned int Absolute_min;	    /* Base minute (since zero) of next	    */
    unsigned int Delta;		    /* Delta minutes for increase to next   */
    unsigned int End_abs_min;	    /* Absolute ending minute		    */
    unsigned int New_absolute_min;  /* New absolute min for expr	    */
    unsigned int Prev_day_checked;  /* Previous day checked for exception   */
    unsigned int New_day;	    /* Next computed day		    */
    

    /*
    **  Compute starting point for scan.
    */
    Absolute_min = (Base_day * DWC$k_db_calendar_precision) + Base_min;

    /*
    **  Enter main scan loop.
    */
    Prev_day_checked = 0;
    for (Delta=Rep_exp->DWC$l_dbre_repeat_interval;
	    ;Delta+=Rep_exp->DWC$l_dbre_repeat_interval)
	{

	/*
	**  Take first instansiation that does not overlap midnight.
	*/
	New_absolute_min = Absolute_min + Delta;
	if (((New_absolute_min % DWC$k_db_calendar_precision) +
		    Rep_exp->DWC$w_dbre_duration) <=
		    DWC$k_db_calendar_precision)
	    {
	    New_day = New_absolute_min / DWC$k_db_calendar_precision;
	    if (New_day != Prev_day_checked)
		{
		if (!DWC$$DB_Check_exception(Cab, Rep_exp, New_day))
		    {
		    break;
		    }
		Prev_day_checked = New_day;
		}
	    }
	}

    /*
    **  Determine if this was the last one or not. If not, pass the time
    **	of the computed instansiation back to caller.
    */
    End_abs_min = (Rep_exp->DWC$l_dbre_endday * DWC$k_db_calendar_precision) +
	 Rep_exp->DWC$w_dbre_endmin;
    if ((End_abs_min != 0) && (New_absolute_min > End_abs_min))
	{
	return (FALSE);
	}
    else
	{
	*Next_day = New_absolute_min / DWC$k_db_calendar_precision;
	*Next_min = New_absolute_min % DWC$k_db_calendar_precision;
	return (TRUE);	
	}	
}

int
DWC$$DB_Free_expr
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	struct DWC$db_repeat_control *Repc)
#else	/* no prototypes */
	(Cab, Repc)
	struct DWC$db_access_block	*Cab;
	struct DWC$db_repeat_control *Repc;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine frees up the memory and disp-space used by one
**	repeat expression.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Repc : Repeat control block for repeat to free
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
**	TRUE	- Success in instansiating expression
**	FALSE	- Failed to instansiate expression
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    struct DWCDB_repeat_expr *Rep_exp;	    /* Work repeat expression	    */
    VM_record *Rep_blo_vm;		    /* Repeat block for expr to free*/
    struct DWCDB_repeat_block *Rep_blo;    /* Associated repeat block	    */
    
    /*
    **  Pick up pointer to expression itself.
    */
    Rep_exp = Repc->DWC$a_dbrc_vector_vm;

    /*
    **  Get rid of list of exception days
    */
    DWC$$DB_Purge_exceptions(Cab, Repc, 0);
    
    /*
    **	Indicate that this vector entry is no longer used and release the rest
    **	of the used blocks, if any.
    */
    Rep_exp->DWC$b_dbre_flags = (Rep_exp->DWC$b_dbre_flags &
				 ~DWC$m_dbre_used);
    if (DWC$$DB_Write_virtual_record(	Cab,
					Repc->DWC$a_dbrc_vector_block) == _Failure)
	{
	Rep_exp->DWC$b_dbre_flags = (Rep_exp->DWC$b_dbre_flags |
					DWC$m_dbre_used);
	return (FALSE);
	}
    _Write_back(Cab);
    Rep_blo_vm = Make_VM_address(Rep_exp->DWC$l_dbre_repeat_interval2);
    Rep_blo = _Bind(Rep_blo_vm, DWCDB_repeat_block);
    if (Rep_blo->DWC$l_dbrb_flink != 0)
	{
	DWC$$DB_Deallocate_records( Cab,
				    Rep_blo->DWC$w_dbrb_ext_count,
				    (CARD32)Rep_blo->DWC$l_dbrb_flink);
	}
    DWC$$DB_Deallocate_records( Cab,
				1,
				(CARD32)Rep_blo_vm->DWC$l_dbvm_rec_addr);

    /*
    **  Release the VM buffers and back to caller
    */
    DWC$$DB_Release_buffer(Cab, Rep_blo_vm);

    /*
    **  Finally, get rid of the repeat expression control block
    */
    DWC$$DB_Remque
    (
	(struct DWC$db_queue_head *) Repc->DWC$a_dbrc_blink,
	(struct DWC$db_queue_head **) 0
    );
    XtFree (Repc);
    return (TRUE);
}	    

int
DWC$$DB_Merge_repeat
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	VM_record *Day_vm,
	struct DWC$db_repeat_control *Repc,
	unsigned int Day,
	unsigned int Min)
#else	/* no prototypes */
	(Cab, Day_vm, Repc, Day, Min)
	struct DWC$db_access_block	*Cab;
	VM_record *Day_vm;
	struct DWC$db_repeat_control *Repc;
	unsigned int Day;
	unsigned int Min;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine merges one instansiation of one repeat expression into the
**	specified day, unless there is a conflict. If the expression is to be
**	cast in concrete, it is removed from its associated repeat expression
**	and marked as 'no' repeat.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Day_vm : Virtual day into which expression is to be inserted
**	Repc : Repeat expression control block
**	Day : Day in which to insert (number)
**	Min : Minute (within input day) of this instansiation
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
**	_MERGE_CONFL	- Conflicting merge, no merge done
**	_MERGE_FAIL	- Failed to perform merge
**	_MERGE_OK	- Merge successful
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    VM_record *Item_vm;			    /* Work item pointer	    */
    struct DWCDB_entry *Item;		    /* Entry under test		    */
    VM_record *Rep_blo_vm;		    /* VM repeat block		    */
    VM_record *First_item_vm;		    /* First item in rep block	    */
    int Databytes;			    /* Number of databytes to clone */
    VM_record *New_item_vm;		    /* Cloned day entry (item)	    */
    struct DWCDB_day *Daymap;		    /* Target day		    */
    struct DWCDB_repeat_expr *Rep_exp;	    /* Repeat expression vector ent */
    

    /*
    **	Start looking for the "slot" where this expression is to be inserted.
    */
    Rep_exp = Repc->DWC$a_dbrc_vector_vm;
    Item_vm = Day_vm->DWC$a_dbvm_vm_flink;
    while (TRUE)
	{

	/*
	**  If we're at the end of the list then this implies that the
	**  requested slot is free. Exit from this loop.
	*/
	if (Item_vm == (VM_record *)&Day_vm->DWC$a_dbvm_vm_flink)
	    {
	    break;
	    }

	/*
	**  Got next entry. Is it an entry?
	*/
	if (Item_vm->DWC$t_dbvm_data[0] == DWC$k_dbit_entry)
	    {

	    /*
	    **  Yes. First, check starting time (in order to sort).
	    */
	    Item = _Bind(Item_vm, DWCDB_entry);
	    if (Min < Item->DWC$w_dben_start_minute)
		{
		break;
		}

	    /*
	    **  Does this entry start at the same time?
	    */
	    if (Min == Item->DWC$w_dben_start_minute)
		{

		/*
		**  Yes. If it has the same Id as the one that we are looking
		**  for then we do not need to merge it.
		*/
		if (Item->DWC$w_dben_entry_id == Repc->DWC$w_dbrc_id)
		    {
		    return (_MERGE_OK);
		    }

		/*
		**  Make sure that Poor-mans-notes (at 0:00; dur=0) comes before
		**  real text items
		*/
		if (Min == 0)
		    {
		    if ((Rep_exp->DWC$w_dbre_duration == 0) ||
			(Item->DWC$w_dben_delta_minutes != 0))
			{
			break;
			}
		    }			
		}
	    }

	/*
	**  Not done yet, advance to next item in the day
	*/
	Item_vm = Item_vm->DWC$a_dbvm_vm_flink;
	}

    /*
    **  Point at the first item hooked off this repeat block (it should
    **	be the template item)
    */
    Rep_blo_vm = Make_VM_address(Rep_exp->DWC$l_dbre_repeat_interval2);
    First_item_vm = (VM_record *)((char *)Rep_blo_vm->DWC$a_dbvm_vm_flink -
			_Field_offset(New_item_vm, DWC$a_dbvm_lru_flink));

    /*
    **  Are we just going to join the template or do we need a clone?
    */
    if (Rep_blo_vm->DWC$a_dbvm_special == 0)
	{
	New_item_vm = First_item_vm;
	}
    else
	{
	Databytes = First_item_vm->DWC$l_dbvm_rec_len - DWC$K_VM_HEADER;
	DWC$$DB_Get_free_buffer(    Cab,
					Databytes,
					&New_item_vm);
	memcpy( &New_item_vm->DWC$t_dbvm_data[0],
		&First_item_vm->DWC$t_dbvm_data[0],
		Databytes);
	DWC$$DB_Insque
	(
	    (struct DWC$db_queue_head *) &New_item_vm->DWC$a_dbvm_lru_flink,
	    (struct DWC$db_queue_head *) &Rep_blo_vm->DWC$a_dbvm_vm_flink
	);
	}

    /*
    **  Setup last stuff in item and hook it off the day
    */
    Item = _Bind(New_item_vm, DWCDB_entry);
    Item->DWC$w_dben_start_minute = Min;
    DWC$$DB_Insque
    (
	(struct DWC$db_queue_head *) New_item_vm,
	(struct DWC$db_queue_head *) Item_vm->DWC$a_dbvm_vm_blink
    );

    /*
    **  Increment the reference count
    */
    Rep_blo_vm->DWC$a_dbvm_special++;

    /*
    **  Define some backlinks
    */
    New_item_vm->DWC$a_dbvm_special = (char *)Day_vm;
    New_item_vm->DWC$l_dbvm_special2 = Day;
    New_item_vm->DWC$a_dbvm_parent_vm = (char **)Rep_blo_vm;

    /*
    **  Setup alarms for this item
    */
    DWC$$DB_Insert_alarm_item(Cab, New_item_vm, Day);
    
    /*
    **  All done, back to caller
    */
    return (_MERGE_OK);
}

int
DWC$$DB_Insert_repeats
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	VM_record *Day_vm,
	int Day)
#else	/* no prototypes */
	(Cab, Day_vm, Day)
	struct DWC$db_access_block	*Cab;
	VM_record *Day_vm;
	int Day;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine merges all instansiations of all repeat expressions that
**	fire on specified day in that day's associated data structures. This
**	routine is typically called when one wants to know about all repeat
**	expressions that happen on a particular day.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Day_vm : Virtual day into which expressions is to be inserted
**	Day : Day number of day
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
**	TRUE	- All expressions merged into the day
**	FALSE	- Failed to merge
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    struct DWC$db_repeat_control *Repctl;   /* Repeat control block	    */
    int Total;				    /* Total repeats inserted	    */
    VM_record *Item_vm;			    /* Work item		    */
    VM_record *Next_item_vm;		    /* Saved forward pointer	    */
    VM_record *Par_vm;			    /* Parent block		    */
    struct DWCDB_entry *Entry;		    /* Next entry		    */
    

    /*
    **  Has this day been updated already? If so, no need to make any
    **	more inserts. V3.0-66 this was commented out since it was causing alarms
    **	for repeats not to be found. See edit history for more details.
    */
/*    if (Day_vm->DWC$l_dbvm_special2 != Cab->DWC$l_dcab_repeat_ctx)
	{
*/
	struct DWC$db_time_ctx Timec;	/* Time context for test	*/

	/*
	**  Back out old repeat instances on day (if any)
	*/
	Item_vm = Day_vm->DWC$a_dbvm_vm_flink;
	while (Item_vm != (VM_record *)&Day_vm->DWC$a_dbvm_vm_flink)
	    {

	    /*
	    **  Save forward link, in case of remove
	    */
	    Next_item_vm = Item_vm->DWC$a_dbvm_vm_flink;

	    /*
	    **  This this a repeat?
	    */
	    Entry = _Bind(Item_vm, DWCDB_entry);
	    if (Entry->DWC$b_dben_flags & DWC$m_dben_repeat)
		{
		
		/*
		**  Yes, remove item from day
		*/
		DWC$$DB_Remque
		(
		    (struct DWC$db_queue_head *) Item_vm->DWC$a_dbvm_vm_blink,
		    (struct DWC$db_queue_head **) 0
		);

		/*
		**  Point at associated parent block and decrement reference
		**  count
		*/
		Par_vm = (VM_record *)Item_vm->DWC$a_dbvm_parent_vm;
		Par_vm->DWC$a_dbvm_special--;

		/*
		**  Are there any instances left for this repeat? If so, then
		**  this instance can be removed (else it should be saved as a
		**  template)
		*/
		if (Par_vm->DWC$a_dbvm_special != 0)
		    {
		    DWC$$DB_Remque
		    (
			(struct DWC$db_queue_head *)
			    Item_vm->DWC$a_dbvm_lru_blink,
			(struct DWC$db_queue_head **) 0
		    );
		    Item_vm->DWC$a_dbvm_lru_flink = 0;
		    DWC$$DB_Freelist_buffer(Cab, Item_vm);
		    }
		}
	    
	    /*
	    **  Advance to next item
	    */
	    Item_vm = Next_item_vm;
	    }
		
	/*
	**  Build time context. Do also prepare daytype cache
	*/
	DWC$DB_Build_time_ctx(Cab, Day, 0, &Timec);
	DWC$$DB_Set_centerpoint(Cab, Day);

	/*
	**  A new merge is needed.
	*/
	Total = 0;
	Repctl = Cab->DWC$a_dcab_repeat_ctl_flink;
	while (Repctl != (struct DWC$db_repeat_control *)&Cab->DWC$a_dcab_repeat_ctl_flink)
	    {
	    Total += DWC$$DB_Insert_one_repeat(Cab, Day_vm, Day, Repctl,
						&Timec);
	    Repctl = Repctl->DWC$a_dbrc_flink;
	    }

	/*
	**  Update the context value assoiated with the day to indicate that
	**  we're now compatible with whatever is in the repeat database.
	*/
/*	Day_vm->DWC$l_dbvm_special2 = Cab->DWC$l_dcab_repeat_ctx;
 */
	/*
	**  Determine the instance of each repeat inserted
	*/
	if (Total)
	    {
	    DWC$$DB_Determine_instances(Cab, Day_vm, Day);
	    }
/*	}
 */
    /*
    **  Done. Back to caller
    */
    return (TRUE);
}

int
DWC$$DB_Insert_one_repeat
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	VM_record *Day_vm,
	int Day,
	struct DWC$db_repeat_control *Repc,
	struct DWC$db_time_ctx *Timec)
#else	/* no prototypes */
	(Cab, Day_vm, Day, Repc, Timec)
	struct DWC$db_access_block	*Cab;
	VM_record *Day_vm;
	int Day;
	struct DWC$db_repeat_control *Repc;
	struct DWC$db_time_ctx *Timec;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine merges all instansiations of a particular repeat
**	expression that trigger during the specified day.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Day_vm : Virtual day into which expression is to be inserted
**	Day : Day number
**	Repc : Repeat expression control block
**	Timec : Time context handle for target day
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
**	Count	- Number of instances inserted
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    unsigned int Next_day;		/* Next day for expression	*/
    unsigned int Next_min;		/* Next minute for expression	*/
    struct DWCDB_repeat_expr *Rep_exp;	/* Repeat expression vect entry	*/
    int Count;				/* Number of instances inserted	*/
    

    /*
    **  Is is an absolute repeat?
    */
    Count = 0;
    Rep_exp = Repc->DWC$a_dbrc_vector_vm;
    if (Repc->DWC$b_dbrc_type != DWC$k_db_absolute)
	{

	/*
	**  Does this repeat trigger on this day?
	*/
	if (DWC$$DB_Triggers_on(Cab, Repc, Timec))
	    {

	    /*
	    **  Yes. Insert it today
	    */
	    DWC$$DB_Page_in_repeat(Cab, Repc);
	    DWC$$DB_Merge_repeat(   Cab,
				    Day_vm,
				    Repc,
				    Day,
				    Rep_exp->DWC$w_dbre_basemin);
	    Count++;
	    }
	}
    else
	{
	
	/*
	**  Compute the next instansiation of this repeat expression, starting
	**  midnight of the input day. Are there any instansiations at this
	**  point ..  and if so, is it at the input day?
	*/
	if ((DWC$$DB_Next_repeat_after( Cab,
					Rep_exp,
					Day,
					0,
					&Next_day,
					&Next_min)) &&
	    (Next_day == Day))
	    {

	    /*
	    **	Yes, there is at least one instansiation of this repeat
	    **	expression in the input day. Page in rest, in case it is not in
	    **	memory yet.
	    */
	    DWC$$DB_Page_in_repeat(Cab, Repc);
	    
	    /*
	    **	Store this instansiation. Then store any additional
	    **	instansiations of this expression that happen during this same
	    **	day
	    */
	    while (TRUE)
		{
		if (DWC$$DB_Merge_repeat(   Cab,
					    Day_vm,
					    Repc,
					    Next_day,
					    Next_min) == _MERGE_FAIL)
		    {
		    return (Count);
		    }
		Count++;
		if ((!DWC$$DB_Next_repeat(  Cab,
					    Rep_exp,
					    Day,
					    Next_min,
					    &Next_day,
					    &Next_min)) ||
		    (Next_day != Day))
		    {
		    break;
		    }
		}
	    }
	}
    /*
    **  All done. back to caller
    */
    return (Count);
}

int
DWC$$DB_Determine_instances
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	VM_record *Day_VM,
	int Day)
#else	/* no prototypes */
	(Cab, Day_VM, Day)
	struct DWC$db_access_block	*Cab;
	VM_record *Day_VM;
	int Day;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Determine the instance type (first, last or middle) of each repeat
**	in a given day. This value is computed by checking nearby instances.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Day_VM : Virtual day
**	Day : Day number
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
**	TRUE	- Done, no problems
**	FALSE	- Failed to compute
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    
    VM_record *Rep_blo_vm;		/* Virtual repeat block		    */
    VM_record *Item_VM;			/* Virtual item			    */
    struct DWCDB_item *Item;		/* Item wihout VM header	    */
    struct DWCDB_entry *Entry;		/* Entry buffer, if adding entry    */
    struct DWC$db_repeat_control *Repc;	/* Repeat expression control block  */
    struct DWCDB_repeat_expr *Rep_exp;	/* Repeat expression vector entry   */
    int instance;			/* Instance type		    */
    int wd;				/* Work day (loop variable)	    */
    unsigned int Next_day;		/* Next day for expression	    */
    unsigned int Next_min;		/* Next minute for expression	    */
    struct DWC$db_time_ctx Timec;	/* Time context for test	    */
    

    /*
    **  Look through all items on day
    */
    Item_VM = Day_VM->DWC$a_dbvm_vm_flink;
    while (Item_VM != (VM_record *)&Day_VM->DWC$a_dbvm_vm_flink)
	{
	Item = _Bind(Item_VM, DWCDB_item);
	if (Item->DWC$b_dbit_blocktype == DWC$k_dbit_entry)
	    {
	    Entry = (struct DWCDB_entry *)Item;

	    /*
	    **  This is a repeating entry. Work on it
	    */
	    if (Entry->DWC$b_dben_flags & DWC$m_dben_repeat)
		{
		Rep_blo_vm = (VM_record *)Item_VM->DWC$a_dbvm_parent_vm;
		Repc = (struct DWC$db_repeat_control *)Rep_blo_vm->DWC$l_dbvm_special2;
		Rep_exp = Repc->DWC$a_dbrc_vector_vm;

		instance = DWC$k_item_last;
		if (Day != Rep_exp->DWC$l_dbre_endday)
		    {
		    if ((Rep_exp->DWC$l_dbre_endday == 0) &&
			(Rep_exp->DWC$w_dbre_endmin == 0))
			{
			instance = DWC$k_item_middle;
			}
		    else
			{
			for (wd=Day+1; wd<=Rep_exp->DWC$l_dbre_endday; wd++)
			    {
			    if (Repc->DWC$b_dbrc_type != DWC$k_db_absolute)
				{
				DWC$DB_Build_time_ctx(Cab, wd, 0, &Timec);
				DWC$$DB_Set_centerpoint(Cab, wd);
				if (DWC$$DB_Triggers_on(Cab, Repc, &Timec))
				    {
				    instance = DWC$k_item_middle;
				    break;
				    }
				}
			    else
				{
				if ((DWC$$DB_Next_repeat_after( Cab,
								Rep_exp,
								wd,
								0,
								&Next_day,
								&Next_min)) &&
				    (Next_day == wd))
				    {
				    instance = DWC$k_item_middle;
				    break;
				    }
				}
			    }
			}
		    }

		if (instance == DWC$k_item_middle)
		    {
		    if (Day == Rep_exp->DWC$l_dbre_baseday)
			{
			instance = DWC$k_item_first;
			}
		    else
			{
			instance = DWC$k_item_first;
			for (wd=Rep_exp->DWC$l_dbre_baseday; wd<Day; wd++)
			    {
			    if (Repc->DWC$b_dbrc_type != DWC$k_db_absolute)
				{
				DWC$DB_Build_time_ctx(Cab, wd, 0, &Timec);
				DWC$$DB_Set_centerpoint(Cab, wd);
				if (DWC$$DB_Triggers_on(Cab, Repc, &Timec))
				    {
				    instance = DWC$k_item_middle;
				    break;
				    }
				}
			    else
				{
				if ((DWC$$DB_Next_repeat_after( Cab,
								Rep_exp,
								wd,
								0,
								&Next_day,
								&Next_min)) &&
				    (Next_day == wd))
				    {
				    instance = DWC$k_item_middle;
				    break;
				    }
				}
			    }
			}
		    }
		Item_VM->DWC$b_dbvm_special3 = instance;
		}
	    }
	Item_VM = Item_VM->DWC$a_dbvm_vm_flink;
	}

    /*
    **  Done, back to caller
    */
    return (TRUE);
}

int
DWC$$DB_Back_out_repeats
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	VM_record *Rep_blo_vm)
#else	/* no prototypes */
	(Cab, Rep_blo_vm)
	struct DWC$db_access_block	*Cab;
	VM_record *Rep_blo_vm;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine removes all cloned instansiations, of a particular repeat
**	expression, from all they virtual days into which is has been magically
**	merged into. This routine is typically called when an item is modified
**	or deleted.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Rep_blo_vm : Virtual repeat block
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
**	TRUE	- Expression backed out from assiated days
**	FALSE	- Failed to back out expression
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    char *Nit;				/* Next item, as removed from queue */
    VM_record *Item_VM;			/* Recalculated item VM record	    */
    struct DWCDB_entry *Entry;		/* Data part of entry		    */

    /*
    **  Coninue with this until all references to days are removed
    */
    while (Rep_blo_vm->DWC$a_dbvm_special != 0)
	{

	/*
	**  Point at the next entry (from the tail) and recalculate the real
	**  item block pointer.
	*/
	Nit = (char *)Rep_blo_vm->DWC$a_dbvm_vm_blink;
	Item_VM = (VM_record *)
		(Nit - _Field_offset(Item_VM, DWC$a_dbvm_lru_flink));

	/*
	**  Remove the item from its associated day.
	*/
	DWC$$DB_Remque
	(
	    (struct DWC$db_queue_head *) Item_VM->DWC$a_dbvm_vm_blink,
	    (struct DWC$db_queue_head **) 0
	);

	/*
	**  If we've got more than one item we should delete the item block too.
	**  The last item should be kept there for "cloning".
	*/
	if (Rep_blo_vm->DWC$a_dbvm_special != (char *)1)
	    {
	    DWC$$DB_Remque
	    (
		(struct DWC$db_queue_head *) Item_VM->DWC$a_dbvm_lru_blink,
		(struct DWC$db_queue_head **) 0
	    );
	    Item_VM->DWC$a_dbvm_lru_flink = 0;
	    DWC$$DB_Freelist_buffer(Cab, Item_VM);
	    }

	/*
	**  Indicate that we have one less item assigned to a day
	*/
	Rep_blo_vm->DWC$a_dbvm_special--;
	}

    /*
    **  All done, back to caller
    */
    return (TRUE);
}

int
DWC$$DB_Check_day
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	int Day,
	int Options)
#else	/* no prototypes */
	(Cab, Day, Options)
	struct DWC$db_access_block	*Cab;
	int Day;
	int Options;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine determines if there is at least one repeat expression
**	instansiated on a given day. This can be useful when determining what to
**	do when loading the daymap for a day with no items.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Day : Day to check
**	Options : Check options:
**
**		    DWC$k_use_empty	    -- Look for anything that triggers
**		    DWC$k_use_normal	    -- Look for the best
**		    DWC$k_use_significant   -- Look only for significant data
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
**	DWC$k_use_empty		-- Nothing found
**	DWC$k_use_normal	-- An insignificant entry triggers on day
**	DWC$k_use_significant	-- A significant entry triggers on day
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    unsigned int Next_day;		    /* Next day for expression	    */
    unsigned int Next_min;		    /* Next minute for expression   */
    struct DWC$db_repeat_control *Repc;	    /* Repeat expression ctl block  */
    struct DWCDB_repeat_expr *Rep_exp;	    /* Repeat expr vector entry	    */
    struct DWC$db_time_ctx Timec;	    /* Time context for test	    */
    int Look_level;			    /* Level being looked at	    */
    int Found_level;			    /* Max level found		    */
    int This_level;			    /* Level of this repeat	    */
    
    /*
    **  Build time context for lookup. Do also prepare daytype cache
    */
    DWC$DB_Build_time_ctx(Cab, Day, 0, &Timec);
    DWC$$DB_Set_centerpoint(Cab, Day);
        
    /*
    **  Start looking for repeat expressions that instansiate on this day.
    */
    Look_level = Options;
    Found_level = DWC$k_use_empty;
    Repc = Cab->DWC$a_dcab_repeat_ctl_flink;
    while (Repc != (struct DWC$db_repeat_control *)&Cab->DWC$a_dcab_repeat_ctl_flink)
	{

	/*
	**  We now have one more repeat expression.
	*/
	Rep_exp = Repc->DWC$a_dbrc_vector_vm;

	/*
	**  Determine if this is a significant repeat or not
	*/
	This_level = DWC$k_use_significant;
	if (Rep_exp->DWC$b_dbre_flags & DWC$m_dbre_insignif)
	    {
	    This_level--;
	    }

	/*
	**  Is it worth examining this or not?
	*/
	if (This_level >= Look_level)
	    {
	    
	    /*
	    **  Yes. Is this an absolute repeat or not?
	    */
	    if (Repc->DWC$b_dbrc_type != DWC$k_db_absolute)
		{

		/*
		**  This is a logical repeat. Does it trigger today?
		*/
		if (DWC$$DB_Triggers_on(Cab, Repc, &Timec))
		    {

		    /*
		    **  Record level found and see if we're done or not
		    */
		    Found_level = This_level;
		    /*
		    if ((Found_level == DWC$k_use_significant) ||
			(Options == DWC$k_use_empty))
			{
			break;
			}
		    */
		    if ( Found_level == DWC$k_use_significant )
			{
			break;
			}
		    Look_level = This_level;
		    }
		}
	    else
		{
	    
		/*
		**	Compute the next instansiation of this repeat
		**	expression, starting midnight of the input day. Are
		**	there any instansiations at this point ..  and if so,
		**	is it at the input day?
		*/
		if ((DWC$$DB_Next_repeat_after( Cab,
						Rep_exp,
						Day,
						0,
						&Next_day,
						&Next_min)) &&
		    (Next_day == Day))
		    {

		    /*
		    **  Record level found and see if we're done or not
		    */
		    Found_level = This_level;
		    /*
		    if ((Found_level == DWC$k_use_significant) ||
			(Options == DWC$k_use_empty))
			{
			break;
			}
		    */
		    if ( Found_level == DWC$k_use_significant )
			{
			break;
			}
		    Look_level = This_level;
		    }
		}
	    }
	/*
	**  Nope. No repeats here. Advance to next expression
	*/
	Repc = Repc->DWC$a_dbrc_flink;
	}
    
    /*
    **  Back to caller with whatever level that we found
    */
    return (Found_level);
}

int
DWC$$DB_Load_exceptions
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	struct DWCDB_repeat_expr *Rep_exp)
#else	/* no prototypes */
	(Cab, Rep_exp)
	struct DWC$db_access_block	*Cab;
	struct DWCDB_repeat_expr *Rep_exp;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine loads the exception days associated with a particular
**	repeat expression. Associated vector blocks are read in. The exception
**	days are put in a sorted linked list.
**
**	Loading of the exception days is usually deferred until the
**	associated repeat expression is being evaluated.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Rep_exp : Repeat expression for which to load
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
**	TRUE	- Exceptions loaded
**	FALSE	- Failed to load exceptions
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    struct DWC$db_exception_head *Ex_head;  /* Exceptions header	    */
    VM_record *Work_buf_vm;		    /* VM buffer with next exc vect */
    struct DWCDB_repeat_exceptions *Work_buf; /* Exception vector block    */
    int i;				    /* General loop variable	    */
    struct DWC$db_exception_day *New_exd;   /* New exception day added	    */
    struct DWC$db_exception_day *Next_exd;  /* Next exception day in queue  */
    int Flink;				    /* Record link to next vector   */
    struct DWC$db_saved_record *Next_rec_ptr; /* Next saved record block    */
    
    
    /*
    **  Allocate one exception header and initialize it
    */
    if ((Ex_head = (struct DWC$db_exception_head *)XtMalloc (sizeof(*Ex_head))) == 0)
	{
	_Record_error;
	_Signal(DWC$_INSVIRMEM);
	}
    Ex_head->DWC$a_dbeh_flink = (struct DWC$db_exception_day *)&Ex_head->DWC$a_dbeh_flink;
    Ex_head->DWC$a_dbeh_blink = (struct DWC$db_exception_day *)&Ex_head->DWC$a_dbeh_flink;
    Ex_head->DWC$a_dbeh_rec_flink = (struct DWC$db_saved_record *)&Ex_head->DWC$a_dbeh_rec_flink;
    Ex_head->DWC$a_dbeh_rec_blink = (struct DWC$db_saved_record *)&Ex_head->DWC$a_dbeh_rec_flink;
    Ex_head->DWC$l_dbeh_first_rec = Rep_exp->DWC$l_dbre_exceptions;
    Ex_head->DWC$l_dbeh_work_rec = Rep_exp->DWC$l_dbre_exceptions;

    /*
    **  Bring in the first exception vector from disk
    */
    Work_buf_vm = DWC$$DB_Read_physical_record(	Cab,
						(CARD32)Rep_exp->DWC$l_dbre_exceptions,
						1,
						DWC$k_db_repeat_exception_vec);

    /*
    **  Get one block for the record pointer, fill it in and save
    **	it
    */
    if ((Next_rec_ptr = (struct DWC$db_saved_record *)XtMalloc (sizeof(*Next_rec_ptr))) == 0)
	{
	_Record_error;
	_Signal(DWC$_INSVIRMEM);
	}
    Next_rec_ptr->DWC$l_dbsr_record_addr = (CARD32)Rep_exp->DWC$l_dbre_exceptions;
    DWC$$DB_Insque
    (
	(struct DWC$db_queue_head *) Next_rec_ptr,
	(struct DWC$db_queue_head *) Ex_head->DWC$a_dbeh_rec_blink
    );

    /*
    **  Process the exception vector(s)
    */
    Ex_head->DWC$l_dbeh_rec_count = 1;
    while (TRUE)
	{

	/*
	**  Point at work buffer
	*/
	Work_buf = _Bind(Work_buf_vm, DWCDB_repeat_exceptions);

	/*
	**  For each entry present in vector ..
	*/
	for (i=0; i<Work_buf->DWC$b_dbrx_next_free; i++)
	    {

	    /*
	    **  Get one exception day block and fill in
	    */
	    if ((New_exd = (struct DWC$db_exception_day *)XtMalloc (sizeof(*New_exd))) == 0)
		{
		_Record_error;
		_Signal(DWC$_INSVIRMEM);
		}
	    New_exd->DWC$l_dbed_day = Work_buf->DWC$l_dbrx_exvec[i];

	    /*
	    **  Sort this in, since the entries are scrambled on disk
	    */
	    Next_exd = Ex_head->DWC$a_dbeh_flink;
	    while (Next_exd != (struct DWC$db_exception_day *)&Ex_head->DWC$a_dbeh_flink)
		{
		if (Next_exd->DWC$l_dbed_day > New_exd->DWC$l_dbed_day)
		    {
		    break;
		    }
		Next_exd = Next_exd->DWC$a_dbed_flink;
		}
	    DWC$$DB_Insque
	    (
		(struct DWC$db_queue_head *) New_exd,
		(struct DWC$db_queue_head *) Next_exd->DWC$a_dbed_blink
	    );
	    }

	/*
	**  Advance to next exception vector. Exit loop if there are no more
	*/
	if ((Flink = Work_buf->DWC$l_dbrx_flink) == 0)
	    {
	    break;
	    }

	/*
	**  There are more exception vectors. Release the current buffer,
	**  since we're going to get one more in the next read.
	*/
	DWC$$DB_Freelist_buffer(Cab, Work_buf_vm);

	/*
	**  Bring in the vector record from disk
	*/
	Work_buf_vm = DWC$$DB_Read_physical_record(Cab, (CARD32)Flink, 1,
			    DWC$k_db_repeat_exception_vec);

	/*
	**  Get one more record pointer buffer, record this record
	**  number.
	*/
	Ex_head->DWC$l_dbeh_work_rec = Flink;
	if ((Next_rec_ptr = (struct DWC$db_saved_record *)XtMalloc (sizeof(*Next_rec_ptr))) == 0)
	    {
	    _Record_error;
	    _Signal(DWC$_INSVIRMEM);
	    }
	Next_rec_ptr->DWC$l_dbsr_record_addr = Flink;
	DWC$$DB_Insque
	(
	    (struct DWC$db_queue_head *) Next_rec_ptr,
	    (struct DWC$db_queue_head *) Ex_head->DWC$a_dbeh_rec_blink
	);
	Ex_head->DWC$l_dbeh_rec_count++;
	}

    /*
    **  Get a new work buffer (for more permanent use), move the most recent
    **	buffer data here and freelist the one we got from the most recent
    **	read.
    */
    if ((Work_buf = (struct DWCDB_repeat_exceptions *)XtMalloc (DWC$k_db_record_length)) == 0)
	{
	_Record_error;
	_Signal(DWC$_INSVIRMEM);
	}
    memcpy(Work_buf, &Work_buf_vm->DWC$t_dbvm_data[0], DWC$k_db_record_length);
    DWC$$DB_Freelist_buffer(Cab, Work_buf_vm);

    /*
    **  Save pointer to new work buffer. Save pointer to exception header
    **	in repeat expression block and back to caller
    */
    Ex_head->DWC$a_dbeh_work_buff = Work_buf;
    Rep_exp->DWC$l_dbre_exceptions = Make_VM_pointer(Ex_head);
    return (TRUE);
}

int
DWC$$DB_Release_exceptions
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	struct DWCDB_repeat_expr *Rep_exp)
#else	/* no prototypes */
	(Cab, Rep_exp)
	struct DWC$db_access_block	*Cab;
	struct DWCDB_repeat_expr *Rep_exp;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine releases all virtual memory associated with the
**	exeception days bound to a particular repeat expression.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Rep_exp : Repeat expression for which exception days are to be unloaded.
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
**	TRUE	- Exception days unloaded
**	FALSE	- Failed to unload exception days
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    struct DWC$db_exception_head *Ex_head;  /* Exceptions header	    */
    struct DWC$db_exception_day *Next_exd;  /* Next exception day in queue  */
    struct DWC$db_saved_record *Next_rec_ptr; /* Next saved record block    */


    /*
    **  If there are no exceptions associated with this repeat expression
    **	then there is no need to clean up.
    */
    if (!Is_VM_pointer(Rep_exp->DWC$l_dbre_exceptions))
	{
	return (TRUE);
	}

    /*
    **  First, get rid of the work buffer
    */
    Ex_head = (struct DWC$db_exception_head *)Make_VM_address(Rep_exp->DWC$l_dbre_exceptions);
    XtFree (Ex_head->DWC$a_dbeh_work_buff);

    /*
    **  Get rid of the list of exception days in VM
    */
    while
    (
	DWC$$DB_Remque
	(
	    (struct DWC$db_queue_head *) &Ex_head->DWC$a_dbeh_flink,
	    (struct DWC$db_queue_head **) &Next_exd
	)
    )
	{
	XtFree (Next_exd);
	}

    /*
    **  Get rid of the list of records used
    */
    while
    (
	DWC$$DB_Remque
	(
	    (struct DWC$db_queue_head *) &Ex_head->DWC$a_dbeh_rec_flink,
	    (struct DWC$db_queue_head **) &Next_rec_ptr
	)
    )
	{
	XtFree (Next_rec_ptr);
	}

    /*
    **  Finally, release the header
    */
    Rep_exp->DWC$l_dbre_exceptions = Ex_head->DWC$l_dbeh_first_rec;
    XtFree (Ex_head);
    return (TRUE);
}

int
DWC$$DB_Add_exception
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	struct DWC$db_repeat_control *Repc,
	unsigned int Except_day)
#else	/* no prototypes */
	(Cab, Repc, Except_day)
	struct DWC$db_access_block	*Cab;
	struct DWC$db_repeat_control *Repc;
	unsigned int Except_day;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine adds a day to the list of exception days associated with
**	a particular repeat expression. Data is written out to disk too.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Repc : Repeat expression control block
**	Except_day : Day to except
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
**	DWC$k_db_normal	    -- Day excepted
**	DWC$k_db_insdisk    -- Could not extend file to accomodate item
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

    struct DWC$db_exception_head *Ex_head;  /* Exceptions header	    */
    struct DWCDB_repeat_exceptions *Work_buf; /* Exception vector block    */
    struct DWC$db_exception_day *New_exd;   /* New exception day added	    */
    struct DWC$db_exception_day *Next_exd;  /* Next exception day in queue  */
    struct DWCDB_repeat_expr *Rep_exp;	    /* Repeat expression vector ent */
    int New_rec;			    /* New vector block		    */
    struct DWC$db_saved_record *Next_rec_ptr; /* Next saved record block    */


    /*
    **  Bring in current list of excepted days into VM, if any
    */
    Rep_exp = Repc->DWC$a_dbrc_vector_vm;
    if ((Rep_exp->DWC$l_dbre_exceptions != 0) &&
	(!Is_VM_pointer(Rep_exp->DWC$l_dbre_exceptions)))
	{
	DWC$$DB_Load_exceptions(Cab, Rep_exp);
	}

    /*
    **  Point to header for excepted days. Do we have any?
    */
    Ex_head = (struct DWC$db_exception_head *)Make_VM_address(Rep_exp->DWC$l_dbre_exceptions);
    if (Ex_head == 0)
	{

	/*
	**  No. Allocate a piece of VM and initialize it
	*/
	if ((Ex_head = (struct DWC$db_exception_head *)XtMalloc (sizeof(*Ex_head))) == 0)
	    {
	    _Record_error;
	    _Signal(DWC$_INSVIRMEM);
	    return (DWC$k_db_failure);
	    }
	Ex_head->DWC$a_dbeh_flink = (struct DWC$db_exception_day *)&Ex_head->DWC$a_dbeh_flink;
	Ex_head->DWC$a_dbeh_blink = (struct DWC$db_exception_day *)&Ex_head->DWC$a_dbeh_flink;
	Ex_head->DWC$a_dbeh_rec_flink = (struct DWC$db_saved_record *)&Ex_head->DWC$a_dbeh_rec_flink;
	Ex_head->DWC$a_dbeh_rec_blink = (struct DWC$db_saved_record *)&Ex_head->DWC$a_dbeh_rec_flink;
	Ex_head->DWC$l_dbeh_first_rec = 0;
	Ex_head->DWC$a_dbeh_work_buff = 0;
	Ex_head->DWC$l_dbeh_rec_count = 0;
	Next_exd = Ex_head->DWC$a_dbeh_flink;
	}
    else
	{

	/*
	**  Scan current list of excepted days for two things:
	**	    1. Make sure we're not inserting a duplicate
	**	    2. A good place to add this new exception day (sorted)
	*/
	Next_exd = Ex_head->DWC$a_dbeh_flink;
	while (Next_exd != (struct DWC$db_exception_day *)&Ex_head->DWC$a_dbeh_flink)
	    {
	    if (Next_exd->DWC$l_dbed_day >= Except_day)
		{
		if (Next_exd->DWC$l_dbed_day == Except_day)
		    {
		    return (DWC$k_db_normal);
		    }
		break;
		}
	    Next_exd = Next_exd->DWC$a_dbed_flink;
	    }
	}

    /*
    **  Allocate a piece of memory for the exception day
    */
    if ((New_exd = (struct DWC$db_exception_day *)XtMalloc (sizeof(*New_exd))) == 0)
	{
	_Record_error;
	_Signal(DWC$_INSVIRMEM);
	}

    /*
    **  If we don't have a work buffer yet, get one
    */
    Work_buf = Ex_head->DWC$a_dbeh_work_buff;
    if (Work_buf == 0)
	{
	if ((Work_buf = (struct DWCDB_repeat_exceptions *)XtMalloc (DWC$k_db_record_length)) == 0)
	    {
	    _Record_error;
	    _Signal(DWC$_INSVIRMEM);
	    }
	Work_buf->DWC$b_dbrx_next_free = DWC$k_db_max_exceptions;
	Ex_head->DWC$a_dbeh_work_buff = Work_buf;
	}

    /*
    **  Is the current work buffer full?
    */
    if (Work_buf->DWC$b_dbrx_next_free == DWC$k_db_max_exceptions)
	{
	struct DWCDB_repeat_exceptions Work_buf_ex;  /* Next exception vector */

	/*
	**  Yes. Get a record for the next buffer
	*/
	if ((New_rec = DWC$$DB_Alloc_records(Cab, 1)) == 0)
	    {
	    return (DWC$k_db_insdisk);
	    }

	/*
	**  Initialize the new buffer and write it out
	*/
	memset(&Work_buf_ex, 0, DWC$k_db_record_length);
	Work_buf_ex.DWC$b_dbrx_blocktype = DWC$k_db_repeat_exception_vec;
	Work_buf_ex.DWC$b_dbrx_next_free = 1;
	Work_buf_ex.DWC$l_dbrx_exvec[0] = Except_day;
	DWC$$DB_Write_physical_record (Cab, (CARD32)New_rec, 1, (char *)&Work_buf_ex);
	_Write_back(Cab);
	if ((Next_rec_ptr = (struct DWC$db_saved_record *)XtMalloc (sizeof(*Next_rec_ptr))) == 0)
	    {
	    _Record_error;
	    _Signal(DWC$_INSVIRMEM);
	    }

	/*
	**  Save this record number
	*/
	Next_rec_ptr->DWC$l_dbsr_record_addr = New_rec;
	DWC$$DB_Insque
	(
	    (struct DWC$db_queue_head *) Next_rec_ptr,
	    (struct DWC$db_queue_head *) Ex_head->DWC$a_dbeh_rec_blink
	);
	Ex_head->DWC$l_dbeh_rec_count++;

	/*
	**  Did we have a previous record? If so, update it's forward link
	**  and save the record number of this record, else just update
	**  the vector block itself.
	*/
	if (Ex_head->DWC$l_dbeh_first_rec != 0)
	    {
	    Work_buf->DWC$l_dbrx_flink = New_rec;
	    DWC$$DB_Write_physical_record
		(Cab, (CARD32)Ex_head->DWC$l_dbeh_work_rec, 1, (char *)Work_buf);
	    Ex_head->DWC$l_dbeh_work_rec = New_rec;
	    }
	else
	    {
	    Ex_head->DWC$l_dbeh_first_rec = New_rec;
	    Ex_head->DWC$l_dbeh_work_rec = New_rec;
	    Rep_exp->DWC$l_dbre_exceptions = Make_VM_pointer(Ex_head);
	    DWC$$DB_Write_virtual_record(Cab, Repc->DWC$a_dbrc_vector_block);
	    }

	/*
	**  Copy the new buffer over the old one
	*/
	memcpy(Work_buf, &Work_buf_ex, DWC$k_db_record_length);
	}
    else
	{

	/*
	**  There is room. Use next slot and write out record to disk
	*/
	Work_buf->DWC$l_dbrx_exvec[Work_buf->DWC$b_dbrx_next_free] = Except_day;
	Work_buf->DWC$b_dbrx_next_free++;
	DWC$$DB_Write_physical_record
	    (Cab, (CARD32)Ex_head->DWC$l_dbeh_work_rec, 1, (char *)Work_buf);
	}

    /*
    **  Flush data
    */
    _Write_back(Cab);

    /*
    **  Put this day on the list of exception days in VM
    */
    New_exd->DWC$l_dbed_day = Except_day;
    DWC$$DB_Insque
    (
	(struct DWC$db_queue_head *) New_exd,
	(struct DWC$db_queue_head *) Next_exd->DWC$a_dbed_blink
    );

    /*
    **  Back to caller
    */
    return (DWC$k_db_normal);
}

int
DWC$$DB_Add_exception_day
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	int Rep_id,
	unsigned int Except_day)
#else	/* no prototypes */
	(Cab, Rep_id, Except_day)
	struct DWC$db_access_block	*Cab;
	int Rep_id;
	unsigned int Except_day;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine adds one exception day to a repeat expression. Please
**	note that this routine does not make sure that the day being
**	added is within the current boundaries of the target expression.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Rep_id : The Id of the expression for which exception day is to
**	         be added
**	Except_day : Day to put on the list of excepted days
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
**	DWC$k_db_normal	    -- Day excepted
**	DWC$k_db_insdisk    -- Could not extend file to accomodate item
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
    struct DWC$db_repeat_control *Repc;	/* Repeat expression control block  */
    int Sts;				/* Final status from this routine   */

    
    /*
    **  Locate the repeat expression in VM. Did we find it?
    */
    if ((Sts = DWC$$DB_Find_repeat(Cab, Rep_id, &Repc)) == DWC$k_db_normal)
	{

	/*
	**  Yes, add specified day to list of excepted days
	*/
	Sts = DWC$$DB_Add_exception(Cab, Repc, Except_day);
	}

    /*
    **  Done, back to caller
    */
    return (Sts);
}

int
DWC$$DB_Purge_exceptions
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	struct DWC$db_repeat_control *Repc,
	unsigned int Before_day)
#else	/* no prototypes */
	(Cab, Repc, Before_day)
	struct DWC$db_access_block	*Cab;
	struct DWC$db_repeat_control *Repc;
	unsigned int Before_day;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine removes all exception days before a certain date. This
**	is usually done when the basedate for a given repeat expression is
**	moved forwards.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Repc : Repeat expression control block
**	Before_day : Remove all exceptions before this day. If zero then
**		     delete all exceptions
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
**	TRUE	- Purged
**	FALSE	- Failed to purge
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    struct DWC$db_exception_head *Ex_head;  /* Exceptions header	    */
    struct DWCDB_repeat_exceptions Work_buf; /* Exception vector block	    */
    struct DWC$db_exception_day *New_exd;   /* New exception day added	    */
    struct DWC$db_exception_day *Next_exd;  /* Next exception day in queue  */
    struct DWCDB_repeat_expr *Rep_exp;	    /* Repeat expression vector ent */
    int New_rec;			    /* New vector block		    */
    struct DWC$db_exception_day *Last_exd;  /* Last Ok exception day	    */
    int Needed;				    /* Records needed after purge   */
    int Start_rec;			    /* Starting record after purge  */
    int i;				    /* General loop variable	    */
    struct DWC$db_saved_record *Next_rec_ptr; /* Saved record number struct */


    /*
    **  Get exceptions brought into memory, if any
    */
    Rep_exp = Repc->DWC$a_dbrc_vector_vm;
    if ((Rep_exp->DWC$l_dbre_exceptions != 0) &&
	(!Is_VM_pointer(Rep_exp->DWC$l_dbre_exceptions)))
	{
	DWC$$DB_Load_exceptions(Cab, Rep_exp);
	}

    /*
    **  If no exceptions, then we don't need to purge. Back to caller
    */
    Ex_head = (struct DWC$db_exception_head *)Make_VM_address(Rep_exp->DWC$l_dbre_exceptions);
    if (Ex_head == 0)
	{
	return (TRUE);
	}

    /*
    **  Assume that we need no space after the purge
    */
    Needed = 0;

    /*
    **  If the "before" day is specified as non-zero it means that we
    **	shall do a real purge (as opposed to a delete).
    */
    if (Before_day != 0)
	{

	/*
	**  Find the first exception that survives after the purge.
	*/
	Last_exd = Ex_head->DWC$a_dbeh_flink;
	while (Last_exd != (struct DWC$db_exception_day *)&Ex_head->DWC$a_dbeh_flink)
	    {
	    if (Last_exd->DWC$l_dbed_day >= Before_day)
		{
		break;
		}
	    Last_exd = Last_exd->DWC$a_dbed_flink;
	    }
	if (Last_exd == Ex_head->DWC$a_dbeh_flink)
	    {
	    return (TRUE);
	    }

	/*
	**  Count the number of exception days there are after we have removed
	**  the ones before the specified day
	*/
	Next_exd = Last_exd;
	while (Next_exd != (struct DWC$db_exception_day *)&Ex_head->DWC$a_dbeh_flink)
	    {
	    Needed++;
	    Next_exd = Next_exd->DWC$a_dbed_flink;
	    }

	/*
	**  Convert the count from number of exception days into the number
	**  of physical exception vector records we will need to store this.
	*/
	Needed = (Needed + (DWC$k_db_max_exceptions - 1)) /
			DWC$k_db_max_exceptions;

	/*
	**  Do we need any?
	*/
	if (Needed != 0)
	    {

	    /*
	    **  Yes, allocate the number we need. Please note that we
	    **	are about reallocate the records (this is improves database
	    **	reliability). Back to caller if we failed to allocate
	    **	the space we needed.
	    */
	    Start_rec = DWC$$DB_Alloc_records(Cab, Needed);
	    if (Start_rec == 0)
		{
		return (FALSE);
		}

	    /*
	    **  Shuffle the exception days into their physical vector records
	    **	and write out the vector blocks, starting with the first
	    **	one. Meanwhile, save the record numbers of the records
	    **	being written.
	    */
	    Next_exd = Last_exd;
	    for (i=0; i<Needed; i++)
		{
		memset(&Work_buf, 0, DWC$k_db_record_length);
		Work_buf.DWC$b_dbrx_blocktype = DWC$k_db_repeat_exception_vec;
		if (i != (Needed - 1))
		    {
		    Work_buf.DWC$l_dbrx_flink = Start_rec + i;
		    }
		while (Next_exd != (struct DWC$db_exception_day *)&Ex_head->DWC$a_dbeh_flink)
		    {
		    Work_buf.DWC$l_dbrx_exvec[Work_buf.DWC$b_dbrx_next_free] =
			Next_exd->DWC$l_dbed_day;
		    Work_buf.DWC$b_dbrx_next_free++;
		    Next_exd = Next_exd->DWC$a_dbed_flink;
		    if (Work_buf.DWC$b_dbrx_next_free = DWC$k_db_max_exceptions)
			{
			break;
			}
		    }
		DWC$$DB_Write_physical_record
		    (Cab, (CARD32)(Start_rec + i), 1, (char *)&Work_buf);
		if ((Next_rec_ptr = (struct DWC$db_saved_record *)XtMalloc (sizeof(*Next_rec_ptr))) == 0)
		    {
		    _Record_error;
		    _Signal(DWC$_INSVIRMEM);
		    }
		Next_rec_ptr->DWC$l_dbsr_record_addr = Start_rec + i;
		DWC$$DB_Insque
		(
		    (struct DWC$db_queue_head *) Next_rec_ptr,
		    (struct DWC$db_queue_head *) Ex_head->DWC$a_dbeh_rec_blink
		);
		}
	    
	    /*
	    **  Save the record number of the first vector block and the
	    **	block of the one we're currently using (for additional adds)
	    */
	    Ex_head->DWC$l_dbeh_first_rec = Start_rec;
	    Ex_head->DWC$l_dbeh_work_rec = Start_rec + Needed - 1;
	    memcpy(Ex_head->DWC$a_dbeh_work_buff, &Work_buf, DWC$k_db_record_length);

	    /*
	    **  Make sure that we synch this to disk, in case we would fail.
	    */
	    _Write_back(Cab);

	    }
	}

    /*
    **  Clear record number of first one, in case we deleted all
    */
    if (Needed == 0) Ex_head->DWC$l_dbeh_first_rec = 0;
    
    /*
    **  Update the repeat expression block, with the new pointer
    */
    DWC$$DB_Write_virtual_record(Cab, Repc->DWC$a_dbrc_vector_block);

    /*
    **  Ok, phyiscal DB should now be in a good state. Remove all
    **	VM exception days that got purged.
    */
    while (Ex_head->DWC$a_dbeh_flink != Last_exd)
	{
	if
	(
	    !DWC$$DB_Remque
	    (
		(struct DWC$db_queue_head *) &Ex_head->DWC$a_dbeh_flink,
		(struct DWC$db_queue_head **) &Next_exd
	    )
	)
	    {
	    break;
	    }
	XtFree (Next_exd);
	}

    /*
    **  Get rid of the physical records used by the previous exception
    **	vectors
    */
    _Write_back(Cab);
    while (Ex_head->DWC$l_dbeh_rec_count != Needed)
	{
	DWC$$DB_Remque
	(
	    (struct DWC$db_queue_head *) &Ex_head->DWC$a_dbeh_rec_flink,
	    (struct DWC$db_queue_head **) &Next_rec_ptr
	);
	DWC$$DB_Deallocate_records(Cab, 1, (CARD32)Next_rec_ptr->DWC$l_dbsr_record_addr);
	XtFree (Next_rec_ptr);
	Ex_head->DWC$l_dbeh_rec_count--;
	}

    /*
    **  If there are no more exceptions here we should release the
    **	exception head too
    */
    if (Needed == 0)
	{
	XtFree (Ex_head);
	Rep_exp->DWC$l_dbre_exceptions = 0;
	}	

    /*
    **  All done, back to caller
    */
    return (TRUE);
}

int
DWC$$DB_Check_exception
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	struct DWCDB_repeat_expr *Rep_exp,
	unsigned int Check_day)
#else	/* no prototypes */
	(Cab, Rep_exp, Check_day)
	struct DWC$db_access_block	*Cab;
	struct DWCDB_repeat_expr *Rep_exp;
	unsigned int Check_day;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine checks if a given day is on the list of excepted days
**	for a given repeat expression.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Rep_exp : Associated repeat expression
**	Check_day : Day to check
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
**	TRUE	- Day is on the list
**	FALSE	- Day is not excepted
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    struct DWC$db_exception_head *Ex_head;  /* Exceptions header	    */
    struct DWC$db_exception_day *Next_exd;  /* Next exception day in queue  */


    /*
    **  No need for checking if this repeat expression does not have
    **	any exceptions
    */
    if (Rep_exp->DWC$l_dbre_exceptions == 0)
	{
	return (FALSE);
	}

    /*
    **  Bring exceptions into VM in case that has not been done already
    */
    if (!Is_VM_pointer(Rep_exp->DWC$l_dbre_exceptions))
	{
	DWC$$DB_Load_exceptions(Cab, Rep_exp);
	}

    /*
    **  Scan queue of exception days for the one in question. Leave
    **	loop and return TRUE as soon as the specified day is found.
    */
    Ex_head = (struct DWC$db_exception_head *)Make_VM_address(Rep_exp->DWC$l_dbre_exceptions);
    Next_exd = Ex_head->DWC$a_dbeh_flink;
    while (Next_exd != (struct DWC$db_exception_day *)&Ex_head->DWC$a_dbeh_flink)
	{
	if (Next_exd->DWC$l_dbed_day == Check_day)
	    {
	    return (TRUE);
	    }
	Next_exd = Next_exd->DWC$a_dbed_flink;
	}

    /*
    **  If we get here in means that the specified day is not listed as
    **	an exception day.
    */
    return (FALSE);
}

int
DWC$$DB_Remove_specific_expr
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	int Rep_id,
	VM_record *Day_vm)
#else	/* no prototypes */
	(Cab, Rep_id, Day_vm)
	struct DWC$db_access_block	*Cab;
	int Rep_id;
	VM_record *Day_vm;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine removes all instansiations of a given expression on
**	a given day, including its associated alarms. This give support
**	to adding exception days for repeat expressions.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Rep_id : Repeat Id to remove
**	Day_vm : VM day for which to remove stuff
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
**	TRUE	- Day is now empty
**	FALSE	- Day is not empty
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    VM_record *Item_vm;			    /* Current work item	    */
    VM_record *Next_item_vm;		    /* Next work item		    */
    struct DWCDB_item *Item;		    /* Item part of buffer	    */
    VM_record *Par_vm;			    /* Repeat block associated	    */
    

    /*
    **  Point at first item on day
    */
    Item_vm = Day_vm->DWC$a_dbvm_vm_flink;

    /*
    **  Scan the list of items on this day
    */
    while (Item_vm != (VM_record *)&Day_vm->DWC$a_dbvm_vm_flink)
	{

	/*
	**  Save forward link to next item, in case we need to fry this
	**  one item
	*/
	Next_item_vm = Item_vm->DWC$a_dbvm_vm_flink;

	/*
	**  Is this the Id that we're looking for?
	*/
	Item = _Bind(Item_vm, DWCDB_item);
	if (Item->DWC$w_dbit_id == Rep_id)
	    {

	    /*
	    **  Yes, remove this repeated item from the daymap.
	    */
	    DWC$$DB_Remque
	    (
		(struct DWC$db_queue_head *) Item_vm->DWC$a_dbvm_vm_blink,
		(struct DWC$db_queue_head **) 0
	    );

	    /*
	    **  Get rid of the associated alarms
	    */
	    DWC$$DB_Remove_alarm_item(Cab, Item_vm->DWC$l_dbvm_special2,
					    Rep_id);

	    /*
	    **  Point to the associated repeat block and decrement the
	    **	reference count
	    */
	    Par_vm = (VM_record *)Item_vm->DWC$a_dbvm_parent_vm;
	    Par_vm->DWC$a_dbvm_special--;

	    /*
	    **  Is the reference count still non-zero? If so, this item need
	    **	not be saved as a template item
	    */
	    if (Par_vm->DWC$a_dbvm_special != 0)
		{

		/*
		**  Remove this item from the repeat block too. Release
		**  the buffer as well.
		*/
		DWC$$DB_Remque
		(
		    (struct DWC$db_queue_head *) Item_vm->DWC$a_dbvm_lru_blink,
		    (struct DWC$db_queue_head **) 0
		);
		Item_vm->DWC$a_dbvm_lru_flink = 0;
		DWC$$DB_Freelist_buffer(Cab, Item_vm);
		}

	    }

	/*
	**  Advance to next item on this day
	*/
	Item_vm = Next_item_vm;
	}

    /*
    **  All items have now been checked. The completion code of this routine
    **	is now computed based on if there are still more items associated
    **	with this daymap
    */
    return (Day_vm->DWC$a_dbvm_vm_flink == (VM_record *)&Day_vm->DWC$a_dbvm_vm_flink);
}

int
DWC$DB_Build_time_ctx
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	int Day,
	int Min,
	struct DWC$db_time_ctx *Timec)
#else	/* no prototypes */
	(Cab, Day, Min, Timec)
	struct DWC$db_access_block	*Cab;
	int Day;
	int Min;
	struct DWC$db_time_ctx *Timec;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine fills in a time context block. The time context block
**	is used when determining if a repeat expression triggers on a
**	given day.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Day : Day for which fields are to be computed
**	Min : Minute of day
**	Timec : Time context to be filled in (allocated by caller)
**	
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      Timec.DWC$l_dbtc_dwcday		    -- Copy of Day parameter
**	Timec.DWC$w_dbtc_dwcmin		    -- Copy of Min parameter
**	Timec.DWC$b_dbtc_day		    -- Day of month (1..31)
**	Timec.DWC$b_dbtc_month		    -- Month (1..12)
**	Timec.DWC$b_dbtc_days_in_month	    -- Days in current month
**	Timec.DWC$b_dbtc_days_prev_month    -- Days in previous month
**	Timec.DWC$b_dbtc_days_next_month    -- Days in next month
**	Timec.DWC$b_dbtc_weekday	    -- Day of week; 1..7; 1=Sunday
**	Timec.DWC$b_dbtc_widx		    -- instance of weekday; 1..5
**	Timec.DWC$b_dbtc_flags		    -- Flags :
**	      DWC$m_dbtc_last		     --  Weekday is also last in month
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    int d;				    /* Day of month		    */
    int m;				    /* Month of year		    */
    int y;				    /* Year			    */
    int nm;				    /* Next month		    */
    int pm;				    /* Previous month		    */
    int ny;				    /* Next year		    */
    int py;				    /* Previous year		    */
    
    
    /*
    **  Save DB time and compute the Western style date for this day number
    */
    Timec->DWC$l_dbtc_dwcday = Day;
    Timec->DWC$w_dbtc_dwcmin = Min;
    DATEFUNCDateForDayNumber(Day, &d, &m, &y);
    
    /*
    **  Save day and month
    */
    Timec->DWC$b_dbtc_day = d;
    Timec->DWC$b_dbtc_month = m;

    /*
    **  Save days in this month
    */
    Timec->DWC$b_dbtc_days_in_month = DATEFUNCDaysInMonth(m, y);

    /*
    **  Determine weekday and instance of weekday (i.e., 1st, 2nd, etc)
    */
    Timec->DWC$b_dbtc_weekday = DATEFUNCDayOfWeek(d, m, y);
    Timec->DWC$b_dbtc_widx = ((int)Timec->DWC$b_dbtc_day + 6) / 7;

    /*
    **  Determine if this is also the last instance of the weekday in this
    **	month.
    */
    Timec->DWC$b_dbtc_flags = 0;
    if (((int)Timec->DWC$b_dbtc_days_in_month - (int)Timec->DWC$b_dbtc_day) <= 6)
	{
	Timec->DWC$b_dbtc_flags = DWC$m_dbtc_last;
	}

    /*
    **  Save days in previous month
    */
    pm = m - 1;
    py = y;
    if (pm == 0)
	{
	pm = 12;
	py--;
	}
    Timec->DWC$b_dbtc_days_prev_month = DATEFUNCDaysInMonth(pm, py);

    /*
    **  Save days in next month
    */
    nm = m + 1;
    ny = y;
    if (nm == 13)
	{
	nm = 1;
	ny++;
	}
    Timec->DWC$b_dbtc_days_next_month = DATEFUNCDaysInMonth(nm, ny);
    
}

int
DWC$$DB_Triggers_on
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	struct DWC$db_repeat_control *Repc,
	struct DWC$db_time_ctx *Timec)
#else	/* no prototypes */
	(Cab, Repc, Timec)
	struct DWC$db_access_block	*Cab;
	struct DWC$db_repeat_control *Repc;
	struct DWC$db_time_ctx *Timec;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine determines if a given repeat expression triggers on
**	the time specified through the Time context
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Repc : Repeat control block
**	Timec : Time context
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
**	TRUE	- Triggers on day
**	FALSE	- Does not trigger on day
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    struct DWCDB_repeat_expr *Rep_exp;	    /* Repeat expression itself	    */
    int stat;				    /* Temp work status		    */
    int i;				    /* Loop variable		    */
    int diff;				    /* Difference in days	    */
    int daytype;			    /* Daytype we're looking for    */
    int start_day;			    /* Start day for slide check    */
    int could_slide;			    /* Boolean: TRUE if could slide */
    int this_month;			    /* This month is Ok for interv  */
    int prev_month;			    /* Prev month is Ok for interv  */
    int next_month;			    /* Next month is Ok for interv  */
    int temp_month;			    /* Temp month variable	    */
    int inv_day;			    /* Inverted day number	    */
    int real_day;			    /* Real day for instance	    */
    int first_day;			    /* First day of type	    */
    int delta;				    /* Delta days to trigger	    */
    int dom;				    /* Day of month		    */
    int nth;				    /* DWCday of n:th day in month  */
    int month_indicator;		    /* prev, curr or next	    */

/*
**  These constants are used for clarity in this routine only
*/
#define DWC$k_db_prev_month 0
#define DWC$k_db_this_month 1
#define DWC$k_db_next_month 2

    /* 
    ** If repeats are currently disabled, just return false.
    */
    if ((Cab->DWC$l_dcab_flags & DWC$m_dcab_rptdisable))
    	return (FALSE);

    /*
    **  Does this repeat start after the time that we are checking?
    */
    Rep_exp = Repc->DWC$a_dbrc_vector_vm;
    if (Timec->DWC$l_dbtc_dwcday < Rep_exp->DWC$l_dbre_baseday)
	{
	return (FALSE);
	}

    /*
    **  Does this repeat end before the time that we are checking?
    */
    if ((Rep_exp->DWC$l_dbre_endday != 0) &&
	(Rep_exp->DWC$w_dbre_endmin != 0) &&
	(Rep_exp->DWC$l_dbre_endday < Timec->DWC$l_dbtc_dwcday))
	{
	return (FALSE);
	}
	
    /*
    **  If we are looking for a given daytype, make sure that this is one of
    **	those (unless we are willing to look for it). Otherwise this day
    **	cannot possibly trigger. Do also check the repeating every day of
    **	certain type (piggy backed on some tests to save code).
    */
    could_slide = FALSE;
    daytype = ((Repc->DWC$w_dbrc_daycond & DWC$m_dbrc_daytype) &
		    DWC$m_mark_type_mask);
    if (daytype != DWC$k_day_default)
	{
	if (DWC$$DB_Get_daytype(Cab, Timec->DWC$l_dbtc_dwcday)
		    != daytype)
	    {
	    return (FALSE);
	    }
	could_slide = ((Repc->DWC$w_dbrc_daycond &
			    DWC$m_cond_flags) != 0);
	start_day = (int)Timec->DWC$l_dbtc_dwcday - 1;
	}

    /*
    **  Determine if this month matches interval unless this may need
    **	to be slid in time. Compute for next and previous month too
    **	in case we need to slide.
    */
    if (Repc->DWC$b_dbrc_type != DWC$k_db_abscond)
	{
	this_month = TRUE;
        if (((((int)Timec->DWC$b_dbtc_month + 12) -
		(int)Repc->DWC$b_dbrc_base_month) %
		(int)Repc->DWC$b_dbrc_month_int) != 0)
	    {
	    this_month = FALSE;
	    }
	if (!could_slide)
	    {
	    if (!this_month)
		{
		return (FALSE);
		}
	    }
	else
	    {
	    temp_month = (int)Timec->DWC$b_dbtc_month - 1;
	    if (temp_month == 0)
		{
		temp_month = 12;
		}
	    prev_month = TRUE;
	    if ((((temp_month + 12) -
		(int)Repc->DWC$b_dbrc_base_month) %
		(int)Repc->DWC$b_dbrc_month_int) != 0)
		{
		prev_month = FALSE;
		}
	    temp_month = (int)Timec->DWC$b_dbtc_month + 1;
	    if (temp_month == 13)
		{
		temp_month = 1;
		}
	    next_month = TRUE;
	    if ((((temp_month + 12) -
		(int)Repc->DWC$b_dbrc_base_month) %
		(int)Repc->DWC$b_dbrc_month_int) != 0)
		{
		next_month = FALSE;
		}
	    }
	}

    /*
    **  Now, go into specifics for each repeat type
    */
    stat = FALSE;
    switch ((int)Repc->DWC$b_dbrc_type)
	{

	/*
	**  Absolute conditional
	*/
	case DWC$k_db_abscond :
	    {

	    /*
	    **  Determine how far off we are from a direct hit.
	    */
	    delta = (Timec->DWC$l_dbtc_dwcday - Rep_exp->DWC$l_dbre_baseday) %
			Repc->DWC$l_dbrc_n;
			
	    /*
	    **  Direct hit?
	    */
	    if (delta == 0)
		{

		/*
		**  Yes. 
		*/
		stat = TRUE;
		}
	    else
		{

		/*
		**  Not a direct hit. Has the user requested a slide?
		*/
		if (could_slide)
		    {
		    
		    /*
		    **  Yes. Determine how many days we would need to slide in
		    **	order to find a day that does match the conditions.
		    */
		    if ((Repc->DWC$w_dbrc_daycond & DWC$m_cond_flags) ==
			    DWC$m_cond_fwd)
			{
			diff = delta;
			}
		    else
			{
			diff = Repc->DWC$l_dbrc_n - delta;
			start_day = start_day + diff + 1;
			}

		    /*
		    **  Check days in between
		    */
		    if (diff <= DWC$k_db_max_rcond_check)
			{
			stat = TRUE;
			for (i=0; i<diff; i++)
			    {
			    if (DWC$$DB_Get_daytype(Cab,
				    (start_day - i)) == daytype)
				{
				stat = FALSE;
				break;
				}
			    }
			}
		    }
		}
	    break;
	    }

	/*
	**  n:th day in month
	*/
	case DWC$k_db_nth_day :
	    {
	    /*
	    **  First, check if day matches the one that we are looking for. No
	    **	need to do a slide check if the day is already correct.
	    */
	    if ((int)Timec->DWC$b_dbtc_day == (int)Repc->DWC$l_dbrc_n)
		{
		stat = this_month;
		}
	    else
		{
		if (could_slide)
		    {

		    /*
		    **  This is not the exact day that we are looking for. But,
		    **	it could have been moved to this day. Verify that all
		    **	days between the real one and current one are not
		    **	of the type that we are looking for.
		    */
		    if ((Repc->DWC$w_dbrc_daycond & DWC$m_cond_flags) ==
			    DWC$m_cond_fwd)
			{
			if ((int)Timec->DWC$b_dbtc_day > Repc->DWC$l_dbrc_n)
			    {
			    if (stat = this_month)
				{
				diff = (int)Timec->DWC$b_dbtc_day - Repc->DWC$l_dbrc_n;
				}
			    }
			else
			    {
			    diff = (int)Timec->DWC$b_dbtc_days_prev_month -
				    Repc->DWC$l_dbrc_n;
			    if (prev_month)
				{
				stat = (diff >= 0);
				diff = diff + (int)Timec->DWC$b_dbtc_day;
				}
			    }
			}
		    else
			{
			if (Timec->DWC$b_dbtc_day < Repc->DWC$l_dbrc_n)
			    {
			    if (stat = this_month)
				{
				diff = Repc->DWC$l_dbrc_n - (int)Timec->DWC$b_dbtc_day;
				}
			    }
			else
			    {
			    if (next_month)
				{
				stat = (Repc->DWC$l_dbrc_n <=
				    Timec->DWC$b_dbtc_days_next_month);
				diff = Repc->DWC$l_dbrc_n +
					(int)Timec->DWC$b_dbtc_days_in_month -
					(int)Timec->DWC$b_dbtc_day;
				}
			    }
			start_day = start_day + diff + 1;
			}
		    if (stat)
			{
			if (stat = (diff <= DWC$k_db_max_rcond_check))
			    {
			    for (i=0; i<diff; i++)
				{
				if (DWC$$DB_Get_daytype(Cab,
					(start_day - i)) == daytype)
				    {
				    stat = FALSE;
				    break;
				    }
				}
			    }
			}
		    }
		}
	    break;
	    }

	/*
	**  n:th x:day in month
	*/
	case DWC$k_db_nth_xday :
	    {
	    /*
	    **  First, check if day matches the one that we are looking for. No
	    **	need to do a slide check if the day is already correct.
	    */
	    if ((Timec->DWC$b_dbtc_weekday == Repc->DWC$b_dbrc_weekday) &&
		(Timec->DWC$b_dbtc_widx == Repc->DWC$l_dbrc_n))
		{
		stat = this_month;
		}
	    else
		{
		if (could_slide)
		    {

		    /*
		    **  This is not the exact day that we are looking for. But,
		    **	it could have been moved to this day. Verify that all
		    **	days between the real one and current one are not
		    **	of the type that we are looking for.
		    */
   		    real_day = ((((((int)Timec->DWC$b_dbtc_day - 1) % 7) +
				    (int)Repc->DWC$b_dbrc_weekday +
				    7 - (int)Timec->DWC$b_dbtc_weekday) %
				    7) + ((((int)Repc->DWC$l_dbrc_n - 1) *
				    7) + 1));
		    if ((Repc->DWC$w_dbrc_daycond & DWC$m_cond_flags) ==
			    DWC$m_cond_fwd)
			{
			if (real_day < (int)Timec->DWC$b_dbtc_day)
			    {
			    if (stat = this_month)
				{
				diff = (int)Timec->DWC$b_dbtc_day - real_day;
				}
			    }
			else
			    {
			    if (stat = prev_month)
				{
				real_day = ((((((int)Timec->DWC$b_dbtc_day - 1 + (
					(int)Timec->DWC$b_dbtc_days_prev_month
					% 7))
					% 7) +
					(int)Repc->DWC$b_dbrc_weekday +
					7 - (int)Timec->DWC$b_dbtc_weekday) %
					7) + ((((int)Repc->DWC$l_dbrc_n - 1) *
					7) + 1));
				diff = (int)Timec->DWC$b_dbtc_days_prev_month -
					real_day + (int)Timec->DWC$b_dbtc_day;
				}
			    }
			}
		    else
			{
			if (real_day > (int)Timec->DWC$b_dbtc_day)
			    {
			    if (stat = this_month)
				{
				diff = real_day - (int)Timec->DWC$b_dbtc_day;
				}
			    }
			else
			    {
			    if (stat = next_month)
				{
				real_day = ((((((int)Timec->DWC$b_dbtc_day - 1 + 7 -
					((int)Timec->DWC$b_dbtc_days_in_month
					% 7))
					% 7) +
					(int)Repc->DWC$b_dbrc_weekday +
					7 - (int)Timec->DWC$b_dbtc_weekday) %
					7) + ((((int)Repc->DWC$l_dbrc_n - 1) *
					7) + 1));
				diff = (int)Timec->DWC$b_dbtc_days_in_month -
					(int)Timec->DWC$b_dbtc_day +
					real_day;
				}
			    }
			start_day = start_day + diff + 1;
			}
		    if (stat)
			{
			if (stat = (diff <= DWC$k_db_max_rcond_check))
			    {
			    for (i=0; i<diff; i++)
				{
				if (DWC$$DB_Get_daytype(Cab,
					(start_day - i)) == daytype)
				    {
				    stat = FALSE;
				    break;
				    }
				}
			    }
			}
		    }
		}
	    break;
	    }

	/*
	**  Last weekday (x:day) in month
	*/
	case DWC$k_db_last_weekday :
	    {
	    /*
	    **  First, check if day matches the one that we are looking for. No
	    **	need to do a slide check if the day is already correct.
	    */
	    if ((Timec->DWC$b_dbtc_weekday == Repc->DWC$b_dbrc_weekday) &&
		(Repc->DWC$l_dbrc_n == 0) &&
		(Timec->DWC$b_dbtc_flags & DWC$m_dbtc_last))
		{
		stat = this_month;
		}
	    else
		{
		if (could_slide)
		    {

		    /*
		    **  This is not the exact day that we are looking for. But,
		    **	it could have been moved to this day. Verify that all
		    **	days between the real one and current one are not
		    **	of the type that we are looking for.
		    */
		    first_day = (((((int)Timec->DWC$b_dbtc_day - 1) % 7) +
				    (int)Repc->DWC$b_dbrc_weekday +
				    7 - (int)Timec->DWC$b_dbtc_weekday) %
				    7) + 1;
		    real_day = ((((int)Timec->DWC$b_dbtc_days_in_month - first_day)
				    / 7) * 7) + first_day;
		    if ((Repc->DWC$w_dbrc_daycond & DWC$m_cond_flags) ==
			    DWC$m_cond_fwd)
			{
			if (real_day < (int)Timec->DWC$b_dbtc_day)
			    {
			    if (stat = this_month)
				{
				diff = (int)Timec->DWC$b_dbtc_day - real_day;
				}
			    }
			else
			    {
			    if (stat = prev_month)
				{
				first_day = (((((int)Timec->DWC$b_dbtc_day - 1 +
					((int)Timec->DWC$b_dbtc_days_prev_month % 7))
					% 7) +
					(int)Repc->DWC$b_dbrc_weekday +
					7 - (int)Timec->DWC$b_dbtc_weekday) %
					7) + 1;
				real_day = ((((int)Timec->DWC$b_dbtc_days_prev_month -
					first_day) / 7) * 7) + first_day;
				diff = (int)Timec->DWC$b_dbtc_days_prev_month -
					real_day + (int)Timec->DWC$b_dbtc_day;
				}
			    }
			}
		    else
			{
			if (real_day > (int)Timec->DWC$b_dbtc_day)
			    {
			    if (stat = this_month)
				{
				diff = real_day - (int)Timec->DWC$b_dbtc_day;
				}
			    }
			else
			    {
			    if (stat = next_month)
				{
				first_day = (((((int)Timec->DWC$b_dbtc_day - 1 +
					((int)Timec->DWC$b_dbtc_days_next_month % 7))
					% 7) +
					(int)Repc->DWC$b_dbrc_weekday +
					7 - (int)Timec->DWC$b_dbtc_weekday) %
					7) + 1;
				real_day = ((((int)Timec->DWC$b_dbtc_days_next_month -
					first_day) / 7) * 7) + first_day;
				diff = (int)Timec->DWC$b_dbtc_days_in_month -
					(int)Timec->DWC$b_dbtc_day + real_day;
				}
			    }
			start_day = start_day + diff + 1;
			}
		    if (stat)
			{
			if (stat = (diff <= DWC$k_db_max_rcond_check))
			    {
			    for (i=0; i<diff; i++)
				{
				if (DWC$$DB_Get_daytype(Cab,
					(start_day - i)) == daytype)
				    {
				    stat = FALSE;
				    break;
				    }
				}
			    }
			}
		    }
		}
	    break;
	    }

	/*
	**  n:th day of month, conditional weekday
	*/
	case DWC$k_db_nth_day_cwd :
	    {

	    /*
	    **  Types of conditionals in this category:
	    **
	    **	  (1)	1:st x:day >= n:th; if not z:day then skip
	    **	  (2)	1:st x:day >= n:th; if not z:day then move fwd
	    **	  (3)	1:st x:day >= n:th; if not z:day then move bck
	    **	  (4)	1:st x:day <= n:th; if not z:day then skip
	    **	  (5)	1:st x:day <= n:th; if not z:day then move fwd
	    **	  (6)	1:st x:day <= n:th; if not z:day then move bck
	    **
	    **	Super-conditionals: (2), (3), (5), (6)
	    */

	    /*
	    **  Determine target day of month
	    */
	    dom = (Repc->DWC$l_dbrc_n & ~DWC$m_cond_flags);

	    /*
	    **  Is this a super conditional?
	    */
	    if (could_slide)
		{

		/*
		**  Yes, moving forwards or backwards if the ideal target is
		**  not of specified day type? Determine in what month n:th is
		**  located.
		*/
		if ((Repc->DWC$w_dbrc_daycond & DWC$m_cond_flags) ==
		       DWC$m_cond_fwd)
		    {

		    /*
		    **  Move forwards to find z:day from n:th?
		    */
		    if ((Repc->DWC$l_dbrc_n & DWC$m_cond_flags) ==
		        DWC$m_cond_fwd)
		        {

			/*
			**  Yes. This is now a repeat of type (2)
			*/
			if ((int)Timec->DWC$b_dbtc_day >= dom)
			    {
			    month_indicator = DWC$k_db_this_month;
			    stat = this_month;
			    }
			else
			    {
			    month_indicator = DWC$k_db_prev_month;
			    stat = prev_month;
			    }
			}
		    else
			{

			/*
			**  No. This is now a repeat of type (5)
			*/
			if ((int)Timec->DWC$b_dbtc_day > dom)
			    {
			    if ((dom + (int)Timec->DWC$b_dbtc_days_in_month -
					(int)Timec->DWC$b_dbtc_day) <= 6)
				{
				month_indicator = DWC$k_db_next_month;
				stat = next_month;
				}
			    else
				{
				month_indicator = DWC$k_db_this_month;
				stat = this_month;
				}
			    }
			else
			    {
			    if ((dom - (int)Timec->DWC$b_dbtc_day) <= 6)
				{
				month_indicator = DWC$k_db_this_month;
				stat = this_month;
				}
			    else
				{
				month_indicator = DWC$k_db_prev_month;
				stat = prev_month;
				}
			    }				
			}
		    }
		else
		    {

		    /*
		    **  Move backwards to find z:day from n:th?
		    */
		    if ((Repc->DWC$l_dbrc_n & DWC$m_cond_flags) ==
		        DWC$m_cond_bck)
		        {

			/*
			**  Yes. This is now a repeat of type (6)
			*/
			if ((int)Timec->DWC$b_dbtc_day > dom)
			    {
			    month_indicator = DWC$k_db_next_month;
			    stat = next_month;
			    }
			else
			    {
			    month_indicator = DWC$k_db_this_month;
			    stat = this_month;
			    }
			}
		    else
			{

			/*
			**  No. This is now a repeat of type (3)
			*/
			if ((int)Timec->DWC$b_dbtc_day >= dom)
			    {
			    month_indicator = DWC$k_db_this_month;
			    stat = this_month;
			    }
			else
			    {
			    if (((int)Timec->DWC$b_dbtc_days_prev_month +
				    (int)Timec->DWC$b_dbtc_day) <= 6)
				{
				month_indicator = DWC$k_db_prev_month;
				stat = prev_month;
				}
			    else
				{
				month_indicator = DWC$k_db_this_month;
				stat = this_month;
				}
			    }
			}
		    }

		/*
		**  Compute the distance from current day to 1:st
		*/
		if (stat)
		    {

		    /*
		    **	Now that we know the month of 'n', compute the absolute
		    **	day of n.
		    */
		    switch (month_indicator)
			{
			case DWC$k_db_prev_month :
			    {
			    diff = -((int)Timec->DWC$b_dbtc_days_prev_month - dom +
					(int)Timec->DWC$b_dbtc_day);
			    break;
			    }
			case DWC$k_db_this_month :
			    {
			    diff = dom - (int)Timec->DWC$b_dbtc_day;
			    break;
			    }
			case DWC$k_db_next_month :
			    {
			    diff = (int)Timec->DWC$b_dbtc_days_in_month -
					(int)Timec->DWC$b_dbtc_day + dom;
			    break;
			    }
			}
		    nth = Timec->DWC$l_dbtc_dwcday + diff;
		    
		    /*
		    **  Where is 1:st relative to n:th? Determine the true
		    **	delta from current day to the 1:st
		    */
		    if ((Repc->DWC$l_dbrc_n & DWC$m_cond_flags) ==
			DWC$m_cond_fwd)
			{

			/*
			**  1:st is past or at n:th. Compute number
			**  of days from nth to the 1st.
			*/
			delta = (((int)Repc->DWC$b_dbrc_weekday + 7 -
				(((nth + 5) % 7) + 1)) % 7);

			/*
			**  Filter out impossible combinations.
			*/
			if ((((Repc->DWC$w_dbrc_daycond & DWC$m_cond_flags) ==
			         DWC$m_cond_fwd) &&
			    ((int)Timec->DWC$l_dbtc_dwcday < (nth + delta))) ||
			    (((Repc->DWC$w_dbrc_daycond & DWC$m_cond_flags) ==
			         DWC$m_cond_bck) &&
			    ((int)Timec->DWC$l_dbtc_dwcday > (nth + delta))))
			    {
			    stat = FALSE;
			    break;
			    }
			diff += delta;
			}
		    else
			{

			/*
			**  1:st is before or at n:th. Filter out impossible
			**  case of double backward
			*/
			delta = (((nth + 5) % 7) + 1 + 7 -
				(int)Repc->DWC$b_dbrc_weekday) % 7;

			/*
			**  Filter out impossible combinations
			*/
			if ((((Repc->DWC$w_dbrc_daycond & DWC$m_cond_flags) ==
			         DWC$m_cond_fwd) &&
			    ((int)Timec->DWC$l_dbtc_dwcday < (nth - delta))) ||
			    (((Repc->DWC$w_dbrc_daycond & DWC$m_cond_flags) ==
			         DWC$m_cond_bck) &&
			    ((int)Timec->DWC$l_dbtc_dwcday > (nth - delta))))
			    {
			    stat = FALSE;
			    break;
			    }
			diff -= delta;
			}
		    
		    /*
		    **  Update start_day and determine an absolute diff
		    */
		    if (diff < 0)
			{
			diff = -diff;
			}
		    else
			{
			start_day += diff + 1;
			}

		    /*
		    **  Check days between actual and target
		    */
		    if (stat = (diff <= DWC$k_db_max_rcond_check))
			{
			for (i=0; i<diff; i++)
			    {
			    if (DWC$$DB_Get_daytype(Cab,
					(start_day - i)) == daytype)
				{
				stat = FALSE;
				break;
				}
			    }
			}
		    }
		}
	    else
		{

		/*
		**  Normal conditional weekday: Did we find the correct weekday?
		*/
		if (Timec->DWC$b_dbtc_weekday != Repc->DWC$b_dbrc_weekday)
		    {

		    /*
		    **  No. Then this cannot be the type that we are looking for
		    */
		    break;
		    }

		/*
		**  Did the user request to move forwards or backwards?
		*/
		if ((Repc->DWC$l_dbrc_n & DWC$m_cond_flags) ==
			DWC$m_cond_fwd)
		    {
		    if ((int)Timec->DWC$b_dbtc_day >= dom)
			{
			if (stat = this_month)
			    {
			    diff = (int)Timec->DWC$b_dbtc_day - dom;
			    }
			}
		    else
			{
			diff = (int)Timec->DWC$b_dbtc_days_prev_month - dom;
			if (prev_month)
			    {
			    stat = (diff >= 0);
			    diff = diff + (int)Timec->DWC$b_dbtc_day;
			    }
			}
		    }
		else
		    {
		    if ((int)Timec->DWC$b_dbtc_day <= dom)
			{
			if (stat = this_month)
			    {
			    diff = dom - (int)Timec->DWC$b_dbtc_day;
			    }
			}
		    else
			{
			if (next_month)
			    {
			    stat = (dom <= (int)Timec->DWC$b_dbtc_days_next_month);
			    diff = dom + (int)Timec->DWC$b_dbtc_days_in_month -
				    (int)Timec->DWC$b_dbtc_day;
			    }
			}
		    }
		/*
		**  If we are less than 7 days from the date that we are looking for
		**	then we are ok.
		*/
		if (stat)
		    {
		    stat = (diff < 7);
		    }
		}
	    break;
	    }
	    
	/*
	**  n:th day before end of month
	*/
	case DWC$k_db_nth_day_end :
	    {
	    /*
	    **  First, check if day matches the one that we are looking for. No
	    **	need to do a slide check if the day is already correct.
	    */
	    inv_day = (int)Timec->DWC$b_dbtc_days_in_month -
			(int)Timec->DWC$b_dbtc_day + 1;
	    if (inv_day == Repc->DWC$l_dbrc_n)
		{
		stat = this_month;
		}
	    else
		{
		if (could_slide)
		    {

		    /*
		    **  This is not the exact day that we are looking for. But,
		    **	it could have been moved to this day. Verify that all
		    **	days between the real one and current one are not
		    **	of the type that we are looking for.
		    */
		    if ((Repc->DWC$w_dbrc_daycond & DWC$m_cond_flags) ==
			    DWC$m_cond_fwd)
			{
			if (inv_day < Repc->DWC$l_dbrc_n)
			    {
			    if (stat = this_month)
				{
				diff = Repc->DWC$l_dbrc_n - inv_day;
				}
			    }
			else
			    {
			    if (stat = prev_month)
				{
				diff = Repc->DWC$l_dbrc_n - 1 +
					(int)Timec->DWC$b_dbtc_day;
				}				
			    }
			}
		    else
			{
			if (inv_day > Repc->DWC$l_dbrc_n)
			    {
			    if (stat = this_month)
				{
				diff = inv_day - Repc->DWC$l_dbrc_n;
				}
			    }
			else
			    {
			    diff = (int)Timec->DWC$b_dbtc_days_next_month -
				    Repc->DWC$l_dbrc_n + 1;
			    if (next_month)
				{
				stat = (diff >= 0);
				diff = diff + inv_day - 1;
				}
			    }
			start_day = start_day + diff + 1;
			}
		    if (stat)
			{
			if (stat = (diff <= DWC$k_db_max_rcond_check))
			    {
			    for (i=0; i<diff; i++)
				{
				if (DWC$$DB_Get_daytype(Cab,
					(start_day - i)) == daytype)
				    {
				    stat = FALSE;
				    break;
				    }
				}
			    }
			}
		    }
		}
	    break;
	    }
	}

    /*
    **  If this is Ok so far, make sure that this day is not excepted
    */
    if (stat)
	{
	stat = (!DWC$$DB_Check_exception(Cab, Rep_exp, (unsigned int)Timec->DWC$l_dbtc_dwcday));
	}
	
    /*
    **  Back to caller with test status
    */
    return (stat);
}

int
DWC$$DB_Check_single_expr
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	int Repeat_id,
	int Day)
#else	/* no prototypes */
	(Cab, Repeat_id, Day)
	struct DWC$db_access_block	*Cab;
	int Repeat_id;
	int Day;
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
    
    struct DWC$db_repeat_control *Repc;	    /* Work repeat control block    */
    int Temp_status;			    /* Temp work status		    */

    /*
    **  Locate the expression
    */
    if ((Temp_status = DWC$$DB_Find_repeat(Cab, Repeat_id, &Repc)) !=
		DWC$k_db_normal)
	{
	return (Temp_status);
	}

    /*
    **	Is this an absolute repeat?
    */
    Temp_status = DWC$k_db_notrigger;
    if (Repc->DWC$b_dbrc_type != DWC$k_db_absolute)
	{
	struct DWC$db_time_ctx Timec;	    /* Time context for test	    */

	/*
	**  No. Build time context for lookup. Do also prepare daytype cache
	*/
	DWC$DB_Build_time_ctx(Cab, Day, 0, &Timec);
	DWC$$DB_Set_centerpoint(Cab, Day);

	/*
	**  Check for trigger and save status
	*/
	if (DWC$$DB_Triggers_on(Cab, Repc, &Timec))
	    {
	    Temp_status = DWC$k_db_triggers;
	    }
	}
    else
	{
	struct DWCDB_repeat_expr *Rep_exp; /* Repeat expr vector entry	    */
	unsigned int Next_day;		    /* Next day for expression	    */
	unsigned int Next_min;		    /* Next minute for expression   */

	/*
	**  Yes. Check if it does trigger and save status
	*/
	Rep_exp = Repc->DWC$a_dbrc_vector_vm;
	if ((DWC$$DB_Next_repeat_after( Cab,
					Rep_exp,
					Day,
					0,
					&Next_day,
					&Next_min)) &&
	    (Next_day == Day))
	    {
	    Temp_status = DWC$k_db_triggers;
	    }
	}

    /*
    **  Done, back to caller with status
    */
    return (Temp_status);
}

int DWC$DB_Get_next_repeat_expr
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	int *Next_item_id,
	int *Rep_start_day,
	int *Rep_start_min,
	int *Duration_days,
	int *Duration_min,
	int *Alarm_vec_len,
	unsigned short int **Alarm_vec,
	int *Flags,
	int *Output_text_len,
	char **Output_text_ptr,
	int *Output_text_class,
	int *P1,
	int *P2,
	int *P3,
	int *P4,
	int *P5,
	int *P6,
	int *End_day,
	int *End_min,
	struct DWC$db_exception_head **Exception_list_head_ptr)
#else	/* no prototypes */
	(Cab, Next_item_id, Rep_start_day,
	    Rep_start_min, Duration_days,
	    Duration_min, Alarm_vec_len, Alarm_vec, Flags,
	    Output_text_len, Output_text_ptr, Output_text_class,
	    P1, P2, P3, P4, P5, P6, End_day, End_min, Exception_list_head_ptr)
	struct DWC$db_access_block	*Cab;
	int *Next_item_id;
	int *Rep_start_day;
	int *Rep_start_min;
	int *Duration_days;
	int *Duration_min;
	int *Alarm_vec_len;
	unsigned short int **Alarm_vec;
	int *Flags;
	int *Output_text_len;
	char **Output_text_ptr;
	int *Output_text_class;
	int *P1;
	int *P2;
	int *P3;
	int *P4;
	int *P5;
	int *P6;
	int *End_day;
	int *End_min;
	struct DWC$db_exception_head **Exception_list_head_ptr;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine reads the next entry from the repeat expressions list. The routine
**	does have its own context and it is thefore possible to read the entries in
**	the queue sequentially.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Next_item_id : Id of next item
**	Rep_start_day : Starting day of repeat expression.
**	Rep_start_min : Starting minute of repeat expression.
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
**	Exception_list_head_ptr: Address of pointer to circular linked list of exception days.
**		             Last entry in list points to address of list head.
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

    VM_record *Repeat_VM;		/* Virtual Repeat for entry	    */
    struct DWCDB_repeat_block *Repeat; /* Real repeat block	    */
    VM_record *Item_VM;			/* Virtual item			    */
    struct DWCDB_entry *Entry;		/* Entry buffer, if adding entry    */
    struct DWC$db_repeat_control *Rep_ctl;	/* Repeat expression control block  */
    struct DWCDB_repeat_expr *Rep_exp;	/* Repeat expression vector entry   */
    struct DWC$db_exception_head *exception_head;	/* Pointer to exception head block. */

    /*
    **  Setup error context
    */
    _Set_base(DWC$_GETIFAIL);

    /*
    **	If the Current Repeat in the CAB is NULL then we start
    **	by finding the first entry  in the Repeat list.  Otherwise 
    **  we assume this pointer is correct, and
    **	follow the forward pointer to the next Repeat structure.
    */
    if ( Cab->DWC$a_dcab_current_repeat == NULL )
	{ /* Get start of Repeat list. */
	Rep_ctl = Cab->DWC$a_dcab_repeat_ctl_flink;
	}
    else
	{ /*  Get the next entry in repeat list. */
	Rep_ctl = (Cab->DWC$a_dcab_current_repeat)->DWC$a_dbrc_flink;
	}

    /* 
     * Determine whether this is the end of the list of repeat expressions.
     * If not, save the current location in the list as context for next call.
     */
    if ( Rep_ctl == (struct DWC$db_repeat_control *)&Cab->DWC$a_dcab_repeat_ctl_flink)
	{ /* The next repeat entry  is the list head, so all done. */
	Cab->DWC$a_dcab_current_repeat = NULL;
	return (DWC$k_db_nomore);
	}
    else
    	{
	/* Save this as context for the next call. */
        Cab->DWC$a_dcab_current_repeat = Rep_ctl;
	}

    /* 
     * Load the repeat expression.
     */
    if (! (DWC$$DB_Page_in_repeat(Cab, Rep_ctl)))
	{
	return (DWC$k_db_failure);
	}

    /*
    **  As this point we have a valid Repeat control block pointer, and 
    **  the other structures associated with the repeat should be memory
    **  resident. Next set up pointers to these other structures.
    */
    Rep_exp = Rep_ctl->DWC$a_dbrc_vector_vm;
    Repeat_VM = _Follow( &Rep_exp->DWC$l_dbre_repeat_interval2, DWC$k_db_repeat_expr_block);
    Repeat = _Bind(Repeat_VM, DWCDB_repeat_block);

    /* 
    ** The Repeat_VM block contains a pointer to a field within the Item_VM
    ** block, but not the start of it. calculate the start of it by
    ** subtracting the offset for the field. It shouldn't matter that
    ** Item_VM is actually an invalid address at this point since we're only
    ** getting the difference between the base and the start of the field in
    ** the _Field_offset macro.
    */
    Item_VM = (VM_record *)((char *) Repeat_VM->DWC$a_dbvm_vm_flink -
		(char *)(_Field_offset( Item_VM, DWC$a_dbvm_lru_flink)));
    Entry = _Bind(Item_VM, DWCDB_entry);

    /*
    **  Unpack an entry
    */
    *Next_item_id = Entry->DWC$w_dben_entry_id;
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
	*Rep_start_day = Rep_exp->DWC$l_dbre_baseday;
	*Rep_start_min = Rep_exp->DWC$w_dbre_basemin;
	*End_day = Rep_exp->DWC$l_dbre_endday;
	*End_min = Rep_exp->DWC$w_dbre_endmin;
	*P1 = Rep_ctl->DWC$b_dbrc_type;
	/* 
	 * Init. P4-P6 since normally unused.
	 */
	*P4 = 0;
	*P5 = 0;
	*P6 = 0;
	switch (Rep_ctl->DWC$b_dbrc_type)
	    {
	    case DWC$k_db_absolute :
		{
		*P2 = Rep_exp->DWC$l_dbre_repeat_interval;
		*P3 = 0;
		break;
		}
	    case DWC$k_db_abscond :
		{
		*P2 = Rep_ctl->DWC$l_dbrc_n;
		*P3 = Rep_ctl->DWC$w_dbrc_daycond;
		break;
		}
	    /*
	    **  n:th day of month conditional weekday
	    */
	    case DWC$k_db_nth_day_cwd :
		{
		*P2 = Rep_ctl->DWC$b_dbrc_month_int;
		*P3 = Rep_ctl->DWC$w_dbrc_daycond |
			(Rep_ctl->DWC$l_dbrc_n << DWC$v_cond_day);
	    	*P4 = Rep_ctl->DWC$b_dbrc_base_month;
	    	*P5 = Rep_ctl->DWC$b_dbrc_weekday;
		break;
		}
	    /*
	    **  By n:th day of month (from start or end) + conditional weekday
	    */
	    case DWC$k_db_nth_day :
	    case DWC$k_db_nth_day_end :
	    	{
		*P2 = Rep_ctl->DWC$b_dbrc_month_int;
		*P3 = Rep_ctl->DWC$w_dbrc_daycond;
	    	*P4 = Rep_ctl->DWC$b_dbrc_base_month;
	    	*P5 = Rep_ctl->DWC$l_dbrc_n;
	    	break;
	    	}
	    /*
	    **  By n:th x:day in month
	    */
    	    case DWC$k_db_nth_xday :
	    	{
		*P2 = Rep_ctl->DWC$b_dbrc_month_int;
		*P3 = Rep_ctl->DWC$w_dbrc_daycond;
	    	*P4 = Rep_ctl->DWC$b_dbrc_base_month;
	    	*P5 = Rep_ctl->DWC$b_dbrc_weekday;
	    	*P6 = Rep_ctl->DWC$l_dbrc_n;
	    	break;
	    	}	    
	    /*
	    **  Last weekday in month
	    */
	    case DWC$k_db_last_weekday :
	    	{
		*P2 = Rep_ctl->DWC$b_dbrc_month_int;
		*P3 = Rep_ctl->DWC$w_dbrc_daycond;
	    	*P4 = Rep_ctl->DWC$b_dbrc_base_month;
	    	*P5 = Rep_ctl->DWC$b_dbrc_weekday;
	    	break;
	    	}	    
	    default :
		{
		*P2 = Rep_ctl->DWC$b_dbrc_month_int;
		*P3 = Rep_ctl->DWC$w_dbrc_daycond;
		}
	    }
	}

    /*
    **  Get repeat exception information, if any.
    */
    if (Rep_exp->DWC$l_dbre_exceptions == 0)
	{
	*Exception_list_head_ptr = NULL;
	}
    else
    	{
    	/*
    	**  Bring exceptions into VM in case that has not been done already
    	*/
    	if (!Is_VM_pointer(Rep_exp->DWC$l_dbre_exceptions))
	    {
	    DWC$$DB_Load_exceptions(Cab, Rep_exp);
	    }
	/* 
	 * Get address of exception head block.
	 */
	*Exception_list_head_ptr = (struct DWC$db_exception_head *)Make_VM_address(Rep_exp->DWC$l_dbre_exceptions);
	}

    /*
    **  Done, back to caller
    */
    _Pop_cause;
    return (DWC$k_db_normal);

}

int DWCDB_GetNextRepeatExpr
#ifdef	_DWC_PROTO_
(
    struct DWC$db_access_block		*Cab,
    int					*Next_item_id,
    int					*Rep_start_day,
    int					*Rep_start_min,
    int					*Duration_days,
    int					*Duration_min,
    int					*Alarm_vec_len,
    unsigned short int			**Alarm_vec,
    int					*Flags,
    unsigned char			**Output_text_ptr,  /* XmString * */
    int					*Output_text_class,
    unsigned int			*Output_icons_num,
    unsigned char			**Output_icon_ptr,
    int					*P1,
    int					*P2,
    int					*P3,
    int					*P4,
    int					*P5,
    int					*P6,
    int					*End_day,
    int					*End_min,
    struct DWC$db_exception_head	**Exception_list_head_ptr
)
#else	/* no prototypes */
(
    Cab, Next_item_id, Rep_start_day,
    Rep_start_min, Duration_days,
    Duration_min, Alarm_vec_len, Alarm_vec, Flags,
    Output_text_ptr, Output_text_class, Output_icons_num, Output_icon_ptr,
    P1, P2, P3, P4, P5, P6, End_day, End_min, Exception_list_head_ptr
)
    struct DWC$db_access_block		*Cab;
    int					*Next_item_id;
    int					*Rep_start_day;
    int					*Rep_start_min;
    int					*Duration_days;
    int					*Duration_min;
    int					*Alarm_vec_len;
    unsigned short int			**Alarm_vec;
    int					*Flags;
    unsigned char			**Output_text_ptr;  /* XmString * */
    int					*Output_text_class;
    unsigned int			*Output_icons_num;
    unsigned char			**Output_icon_ptr;
    int					*P1;
    int					*P2;
    int					*P3;
    int					*P4;
    int					*P5;
    int					*P6;
    int					*End_day;
    int					*End_min;
    struct DWC$db_exception_head	**Exception_list_head_ptr;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine reads the next entry from the repeat expressions list. The routine
**	does have its own context and it is thefore possible to read the entries in
**	the queue sequentially.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Next_item_id : Id of next item
**	Rep_start_day : Starting day of repeat expression.
**	Rep_start_min : Starting minute of repeat expression.
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
**	Exception_list_head_ptr: Address of pointer to circular linked list of exception days.
**		             Last entry in list points to address of list head.
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

    VM_record *Repeat_VM;		/* Virtual Repeat for entry	    */
    struct DWCDB_repeat_block *Repeat; /* Real repeat block	    */
    VM_record *Item_VM;			/* Virtual item			    */
    struct DWCDB_entry *Entry;		/* Entry buffer, if adding entry    */
    struct DWC$db_repeat_control *Rep_ctl;	/* Repeat expression control block  */
    struct DWCDB_repeat_expr *Rep_exp;	/* Repeat expression vector entry   */
    struct DWC$db_exception_head *exception_head;	/* Pointer to exception head block. */
    unsigned char			*xm_text;	/* XmString */
    static char				*fc;
    static int				fc_len = 0;
    long				byte_count, cvt_status;
    int					icon_len;
    int					text_len;
    char				temp_char;
    int					header_len;

    /*
    **  Setup error context
    */
    _Set_base(DWC$_GETIFAIL);

    /*
    **	If the Current Repeat in the CAB is NULL then we start
    **	by finding the first entry  in the Repeat list.  Otherwise 
    **  we assume this pointer is correct, and
    **	follow the forward pointer to the next Repeat structure.
    */
    if ( Cab->DWC$a_dcab_current_repeat == NULL )
    { /* Get start of Repeat list. */
	Rep_ctl = Cab->DWC$a_dcab_repeat_ctl_flink;
    }
    else
    { /*  Get the next entry in repeat list. */
	Rep_ctl = (Cab->DWC$a_dcab_current_repeat)->DWC$a_dbrc_flink;
    }

    /* 
     * Determine whether this is the end of the list of repeat expressions.
     * If not, save the current location in the list as context for next call.
     */
    if ( Rep_ctl == (struct DWC$db_repeat_control *)&Cab->DWC$a_dcab_repeat_ctl_flink)
    { /* The next repeat entry  is the list head, so all done. */
	Cab->DWC$a_dcab_current_repeat = NULL;
	return (DWC$k_db_nomore);
    }
    else
    {
	/* Save this as context for the next call. */
        Cab->DWC$a_dcab_current_repeat = Rep_ctl;
    }

    /* 
     * Load the repeat expression.
     */
    if (! (DWC$$DB_Page_in_repeat(Cab, Rep_ctl)))
    {
	return (DWC$k_db_failure);
    }

    /*
    **  As this point we have a valid Repeat control block pointer, and 
    **  the other structures associated with the repeat should be memory
    **  resident. Next set up pointers to these other structures.
    */
    Rep_exp = Rep_ctl->DWC$a_dbrc_vector_vm;
    Repeat_VM = _Follow( &Rep_exp->DWC$l_dbre_repeat_interval2, DWC$k_db_repeat_expr_block);
    Repeat = _Bind(Repeat_VM, DWCDB_repeat_block);

    /* 
    ** The Repeat_VM block contains a pointer to a field within the Item_VM
    ** block, but not the start of it. calculate the start of it by
    ** subtracting the offset for the field. It shouldn't matter that
    ** Item_VM is actually an invalid address at this point since we're only
    ** getting the difference between the base and the start of the field in
    ** the _Field_offset macro.
    */
    Item_VM = (VM_record *)((char *) Repeat_VM->DWC$a_dbvm_vm_flink -
		(char *)(_Field_offset( Item_VM, DWC$a_dbvm_lru_flink)));
    Entry = _Bind(Item_VM, DWCDB_entry);
    header_len = _Field_offset(Entry,DWC$t_dben_data[0]);

    /*
    **  Unpack an entry
    */
    *Next_item_id = Entry->DWC$w_dben_entry_id;
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
	    icon_len = *Output_icons_num + 1;
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
	    (3+(*Alarm_vec_len*2)+icon_len);

	if (fc_len == 0)
	{
	    fc_len = text_len+1;
	    fc = (char *)XtMalloc (fc_len);
	}
	else if (fc_len < text_len+1)
	{
	    fc_len = text_len+1;
	    fc = (char *)XtRealloc(fc,fc_len);
	}
	memcpy
	(
	    fc,
	    &Entry->DWC$t_dben_data[3+(*Alarm_vec_len*2)+icon_len],
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

	if (fc_len == 0)
	{
	    fc_len = text_len+1;
	    fc = (char *)XtMalloc (fc_len);
	}
	else if (fc_len < text_len+1)
	{
	    fc_len = text_len+1;
	    fc = (char *)XtRealloc(fc,fc_len);
	}
	memcpy (fc, &Entry->DWC$t_dben_data[icon_len], text_len);
	fc [text_len] = 0;

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
	*Rep_start_day = Rep_exp->DWC$l_dbre_baseday;
	*Rep_start_min = Rep_exp->DWC$w_dbre_basemin;
	*End_day = Rep_exp->DWC$l_dbre_endday;
	*End_min = Rep_exp->DWC$w_dbre_endmin;
	*P1 = Rep_ctl->DWC$b_dbrc_type;
	/* 
	 * Init. P4-P6 since normally unused.
	 */
	*P4 = 0;
	*P5 = 0;
	*P6 = 0;
	switch (Rep_ctl->DWC$b_dbrc_type)
	{
	case DWC$k_db_absolute :
	    *P2 = Rep_exp->DWC$l_dbre_repeat_interval;
	    *P3 = 0;
	    break;

	case DWC$k_db_abscond :
	    *P2 = Rep_ctl->DWC$l_dbrc_n;
	    *P3 = Rep_ctl->DWC$w_dbrc_daycond;
	    break;
	case DWC$k_db_nth_day_cwd :
	    /*
	    **  n:th day of month conditional weekday
	    */
	    *P2 = Rep_ctl->DWC$b_dbrc_month_int;
	    *P3 = Rep_ctl->DWC$w_dbrc_daycond |
		    (Rep_ctl->DWC$l_dbrc_n << DWC$v_cond_day);
	    *P4 = Rep_ctl->DWC$b_dbrc_base_month;
	    *P5 = Rep_ctl->DWC$b_dbrc_weekday;
	    break;

	case DWC$k_db_nth_day :
	case DWC$k_db_nth_day_end :
	    /*
	    **  By n:th day of month (from start or end) + conditional weekday
	    */
	    *P2 = Rep_ctl->DWC$b_dbrc_month_int;
	    *P3 = Rep_ctl->DWC$w_dbrc_daycond;
	    *P4 = Rep_ctl->DWC$b_dbrc_base_month;
	    *P5 = Rep_ctl->DWC$l_dbrc_n;
	    break;

	case DWC$k_db_nth_xday :
	    /*
	    **  By n:th x:day in month
	    */
	    *P2 = Rep_ctl->DWC$b_dbrc_month_int;
	    *P3 = Rep_ctl->DWC$w_dbrc_daycond;
	    *P4 = Rep_ctl->DWC$b_dbrc_base_month;
	    *P5 = Rep_ctl->DWC$b_dbrc_weekday;
	    *P6 = Rep_ctl->DWC$l_dbrc_n;
	    break;

	case DWC$k_db_last_weekday :
	    /*
	    **  Last weekday in month
	    */
	    *P2 = Rep_ctl->DWC$b_dbrc_month_int;
	    *P3 = Rep_ctl->DWC$w_dbrc_daycond;
	    *P4 = Rep_ctl->DWC$b_dbrc_base_month;
	    *P5 = Rep_ctl->DWC$b_dbrc_weekday;
	    break;

	default :
	    *P2 = Rep_ctl->DWC$b_dbrc_month_int;
	    *P3 = Rep_ctl->DWC$w_dbrc_daycond;
	}
    }

    /*
    **  Get repeat exception information, if any.
    */
    if (Rep_exp->DWC$l_dbre_exceptions == 0)
    {
	*Exception_list_head_ptr = NULL;
    }
    else
    {
    	/*
    	**  Bring exceptions into VM in case that has not been done already
    	*/
    	if (!Is_VM_pointer(Rep_exp->DWC$l_dbre_exceptions))
	{
	    DWC$$DB_Load_exceptions(Cab, Rep_exp);
	}
	/* 
	 * Get address of exception head block.
	 */
	*Exception_list_head_ptr = (struct DWC$db_exception_head *)Make_VM_address(Rep_exp->DWC$l_dbre_exceptions);
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
