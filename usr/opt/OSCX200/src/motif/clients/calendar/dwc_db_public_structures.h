#ifndef _dwc_db_public_structures_h_
#define _dwc_db_public_structures_h_
/* $Header$ */
/* DWC_DB_PUBLIC_STRUCTURES.H "V3.0-031"  */
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
**	This module defines the public (for UI) data structures of
**	the DECwindows calendar database routines.
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
** V3.0-031 Paul Ferwerda				    21-Nov-1990
**		Converted from SDL to make Ultrix builds easier.
**		
**	V2-030	Per Hamnqvist					    17-Aug-1989
**		Make most fields in the time-control block signed, so that
**		proper arithmetic will be performed by C
**
**	V2-029	Per Hamnqvist					    01-May-1989
**		Move out most constants to new module DWC_DB_PUBLIC_CONST.SDL
**
**	V2-028	Per Hamnqvist					    08-Apr-1989
**		a) Add constant for max output time/date line
**		b) Remove Itok hints
**
**	V2-027	Per Hamnqvist					    03-Apr-1989
**		Add new fields for repeat instance position
**
**	V2-026	Per Hamnqvist					    31-Mar-1989
**		a) Define bitmask for move conditions
**		b) Remove weekday definitions
**
**	V2-025	Per Hamnqvist					    29-Mar-1989
**		Add new text class.
**
**	V2-024	Per Hamnqvist					    29-Mar-1989
**		Add new flag indicating that daytype was defaulted.
**
**	V2-023	Per Hamnqvist					    25-Mar-1989
**		a) Add new repeat type
**		b) Define constants for weekdays
**
**	V2-022	Per Hamnqvist					    25-Mar-1989
**		a) Replace DWC$k_db_daytype with new DWC$k_db_abscond
**		b) Order repeat types by likelyhood of use
**
**	V2-021	Per Hamnqvist					    24-Mar-1989
**		Change significant into insignificant.
**
**	V2-020	Per Hamnqvist					    23-Mar-1989
**		Add new repeat type (conditional absolute)
**
**	V2-019	Per Hamnqvist					    23-Mar-1989
**		The daymarks have been redefined. Define new flags for them.
**
**	V2-018	Per Hamnqvist					    23-Mar-1989
**		Add definition of values returned for Day_used.
**
**	V2-017	Per Hamnqvist					    21-Mar-1989
**		Add two constants for defining data type associated with item.
**
**	V2-016	Per Hamnqvist					    21-Mar-1989
**		Transferred structure DWC$db_time_ctx from work_structures
**		into this module.
**
**	V2-015	Per Hamnqvist					    14-Mar-1989
**		Add item flags and definition of P3
**
**	V2-014	Per Hamnqvist					    02-Mar-1989
**		Add repeat undefined
**
**	V2-013	Per Hamnqvist					    01-Mar-1989
**		Move repeat type into this module from WORK_STRUCTURES
**
**	V1-012	Per Hamnqvist					    20-Sep-1988
**		The name of the public Cab structure is now changed not to
**		conflict with the internal definition. The public one has
**		now become a dummy only. The real Cab is defined in the
**		module DWC_DB_WORK_STRUCTURES.SDL
**
**	V1-011	Per Hamnqvist					    09-Apr-1988
**		A new fields to Cab for:
**		    a) Name of Calendar (for rename)
**		    b) Temp alarm array for Get_specific_r_item
**		    c) Temp text array for Get_specific_r_item
**
**	V1-010	Per Hamnqvist					    16-Mar-1988
**		Change names of fields pointing at virtual repeat vectors.
**
**	V1-009	Per Hamnqvist					    15-Mar-1988
**		Add repeat context field so that it can easily be determined
**		if the repeat expressions for a given day needs to be
**		recomputed.
**
**	V1-008	Per Hamnqvist					    09-Mar-1988
**		Add new fields for in-memory repeat expressions.
**
**	V1-007	Per Hamnqvist					    01-Mar-1988
**		Add fields for in-memory Alarm management.
**
**	V1-006	Per Hamnqvist					    11-Feb-1988
**		Change the DWC$a_dcab_fp to DWC$l_dcab_fd, since we changed
**		from Standard I/O to Ultrix I/O.
**
**	V1-005	Per Hamnqvist					    26-Jan-1988
**		Added DWC$a_dcab_current_day_vm field to keep additional
**		track of Get item
**
**	V1-004	Hein van den Heuvel				    18-Jan-1988
**		added statistics, changed some fields
**
**	V1-003	Per Hamnqvist					    17-Dec-1987
**		Complete the declaration of all pointers by supplying a
**		target for each pointer
**
**	V1-002	Per Hamnqvist					    11-Dec-1987
**		Use ADDRESS instead of UNSIGNED LONGWORD
**
**	V1-001	Per Hamnqvist					    30-Nov-1987
**		Use VMS style headers
**--
*/

/*
** This defines the dummy structure used for representing the Calendar access
** block, the structure that is used to pass context accross calls to various
** database access routines.
*/
struct DWC$db_cab {
    unsigned long int DWC$l_dcab_dummy; /* Dummy field */
    } ;

/*
** This structure is contains an expanded view of a given DECwindows Calendar
** day.
*/
#define DWC$m_dbtc_last 1               /*  Last weekday of month */


struct DWC$db_time_ctx {
    unsigned long int DWC$l_dbtc_dwcday; /* DWC db day */
    unsigned short int DWC$w_dbtc_dwcmin; /* DWC db min */
    char DWC$b_dbtc_day;                /* Day in month */
    char DWC$b_dbtc_month;              /* Month in year */
    char DWC$b_dbtc_days_in_month;      /* Number of days in month */
    char DWC$b_dbtc_days_prev_month;    /* Days in previous month */
    char DWC$b_dbtc_days_next_month;    /* Days in next month */
    char DWC$b_dbtc_weekday;            /* Weekday */
    unsigned char DWC$b_dbtc_flags;     /* Flags: */
    char DWC$b_dbtc_widx;               /* Weekday index in month */
    } ;

#endif /* end of dwc_db_public_structures_h_ */
