/* dwc_db_trading.c */
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
**	Per Hamnqvist, March 1989
**
**  ABSTRACT:
**
**	This module contains the code for dealing with import and export
**	of parsable text representation of day entries.
**
**--
*/

/*
**  Include Files
*/
#include "dwc_compat.h"

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"
#include "dwc_ui_datefunctions.h"

/*
**  Various ASCII characters part of the tokens that we are parsing.
*/
#define DWC$k_char_space 32
#define DWC$k_char_tab 9
#define DWC$k_char_newline 10
#define DWC$k_char_open_brace '{'
#define DWC$k_char_close_brace '}'
#define DWC$k_char_null 0
#define DWC$k_char_colon ':'
#define DWC$k_char_dash '-'
#define DWC$k_char_dot '.'
#define DWC$k_char_semicolon ';'
#define DWC$k_char_T 'T'

/*
**  Size of work buffer used for intermediate storage before allocating memory
**  with XtMalloc.
*/
#define DWC$k_max_load_line 1000

/*
**  Start and end token for the parse.
*/
#define DWC$t_start_token "DECwindows_Calendar"
#define DWC$t_start_token_hashed "%*[D]%*[E]%*[C]%*[w]%*[i]%*[n]%*[d]%*[o]%*[w]%*[s]%*[_]%*[C]%*[a]%*[l]%*[e]%*[n]%*[d]%*[a]%*[r]%[:]"
#define DWC$t_end_token_hashed "%*[D]%*[E]%*[C]%*[w]%*[i]%*[n]%*[d]%*[o]%*[w]%*[s]%*[_]%*[C]%*[a]%*[l]%*[e]%*[n]%*[d]%*[a]%*[r]%[}]"
#define DWC$k_token_len 20

/*
**  Define constant for length of starting token (for output). It contains
**  the following fields:
**
**	    Field		    Length
**	    -----		    ------
**
**	    New_line		    1
**	    User_date		    ~ [supplied by callback]
**	    New_line		    1 [entry only]
**	    User_from		    ~ [supplied by callback; entry only]
**	    New_line		    1 [entry only]
**	    User_to		    ~ [supplied by callback; entry only]
**	    New_line		    2
**	    Open_brace		    1
**	    Token		    (DWC$k_token_len - 1)
**	    Colon		    1
**	    Year_field		    4
**	    Dot			    1
**	    Month_field		    2
**	    Dot			    1
**	    Day_field		    2
**	    Semicolon		    1
**	    Start_hour		    2 [Not used for daynote]
**	    Colon		    1 [Not used for daynote]
**	    Start_minute	    2 [Not used for daynote]
**	    Dash		    1 [Not used for daynote]
**	    End_hour		    2 [Not used for daynote]
**	    Colon		    1 [Not used for daynote]
**	    End_minute		    2 [Not used for daynote]
**	    Data-type-indic	    1
**	    Ending_brace	    1
**	    New_line		    1
**	    New_line		    1
**				=======================
**	[entry] = 28 + (DWC$k_token_len - 1) + 5 +
**		    strlen(User_date) + strlen(User_from) + strlen(User_to)
**
**	[note] = 17 + (DWC$k_token_len - 1) + 3 +
**		    strlen(User_date)
*/
#define DWC$k_start_token_out_len_ent (28 + 5 + DWC$k_token_len - 1)
#define DWC$k_start_token_out_len_not (17 + 3 + DWC$k_token_len - 1)

/*
**  Define constant length of the ending token (for output). It contains the
**  following fields:
**
**	    Field		    Length
**	    -----		    ------
**
**	    New_line		    1
**	    New_line		    1
**	    Open_brace		    1
**	    Token		    (DWC$k_token_len - 1)
**	    Closing_brace	    1
**	    New_line		    1
**	    New_line		    1
**				=======================
**				6 + DWC$k_token_len
*/
#define DWC$k_end_token_out_len (6 + DWC$k_token_len - 1)


/*
**  Macro to pass back file error on load
*/
#ifdef VMS
#define _Record_errors(c_err,vaxc_err) {\
			*c_err = errno; \
			*vaxc_err = vaxc$errno; \
			}
#else
#define _Record_errors(c_err,vaxc_err) {\
			*c_err = errno; \
			*vaxc_err = 0; \
			}
#endif			

int DWC$DB_Write_interchange
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	void (*date_fmt)(),
	void (*time_fmt)(),
	void *Uparam,
	char **Int_arr)
#else	/* no prototypes */
	(Cab, work_context, date_fmt, time_fmt, Uparam, Int_arr)
	struct DWC$db_access_block *Cab;
	struct DWC$db_interchange *work_context;
	void (*date_fmt)();
	void (*time_fmt)();
	void *Uparam;
	char **Int_arr;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine creates a parsable text string from an interchange work
**	context.  The output from the routine is in the form of a NULL
**	terminated string (that can be fed into the Parse routine).
**
**	This routine will invoke caller supplied callback routines to format
**	date and time strings. The date and time used in these tokes are there
**	to make it more pretty (they have no functional meaning). In general
**	the output token looks like:
**
**	    Date/Time
**
**	    {Token_start}
**
**	    Data
**
**	    {End_token}
**
**	The date/time data depends on what token is being formatted. The
**	caller supplies two formatting routines.
**
**	    1. Produce date string:
**
**		(*date_fmt)(Uparam, year, month, date, ent_flg, outbuf)
**
**		    date_fmt	- address of user supplied callback routine
**		    Uparam	- User context parameter passed
**		    year	- Year of date to format
**		    month	- month of year
**		    day		- day of month
**		    ent_flg	- boolean; true if date is for an entry
**				  else for a daynote
**		    outbuf	- pointer to output buffer to be filled in
**			          by callback routine. this buffer is only
**				  DWC$k_db_max_pretty bytes long.
**
**	    2. Produce time string
**
**		(*time_fmt)(Uparam, time, start_flg, outbuf)
**
**		    time_fmt	- address of user supplied callback routine
**		    Uparam	- User context parameter passed
**		    time	- minute of day (0..1439)
**		    start_flg	- boolean; true if time is starting time
**				  else ending time
**		    outbuf	- pointer to output buffer to be filled in
**			          by callback routine. this buffer is only
**				  DWC$k_db_max_pretty bytes long.
**
**	In both cases, callback routine should fill in a buffer that has already
**	been allocated by this routine with a NULL terminate string. This
**	routine will add a <New_line> character at the end of output line
**	during format.
**	
**	Please note that Caller is responsible for deallocating the
**	output string with "free". Also, any Get_next_r_item context
**	that was established prior to this call is lost.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	work_context : interchange context with data to "print"
**	date_fmt : address of date formatting callback routine
**	time_fmt : address of time formatting callback routine
**	Uparam : user parameter to be passed to callback routine
**	Int_arr : User pointer (by ref) to receive pointer to memory
**		  allocated in text was written.
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
**	DWC$k_db_normal	    -- Data retreived and context created
**	DWC$k_db_insmem	    -- Not enough memory to create string
**	DWC$k_db_notoks	    -- There are no items in this work context
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    struct DWC$db_itoken *Next_token;	    /* Next token being written	    */
    char *Strptr;			    /* Pointer to output string	    */
    int arr_len;			    /* Length of output string	    */
    int y, m, d;			    /* Year, Month and Day	    */
    int start_hour, start_min;		    /* Start hour, Start min	    */
    int end_hour, end_min;		    /* End hour, End minute	    */
    int i;				    /* Loop variable		    */
    char pd[DWC$k_db_max_pretty+1];	    /* Pretty date		    */
    char ps[DWC$k_db_max_pretty+1];         /* Pretty starting time	    */
    char pe[DWC$k_db_max_pretty+1];         /* Pretty ending time	    */

    /*
    **  Determine the length of the output so that we can allocate one big
    **	chunk of memory. Loop through all items on the list and add the
    **	size of each item to the total length.
    */
    arr_len = 0;
    Next_token = work_context->DWC$a_dbin_flink;
    while (Next_token !=
		    (struct DWC$db_itoken *)&work_context->DWC$a_dbin_flink)
	{
	if (Next_token->DWC$l_itok_msglen != 0)
	    {
	    DATEFUNCDateForDayNumber(Next_token->DWC$l_itok_day, &d, &m, &y);
	    if ((Next_token->DWC$w_itok_start != 0) ||
		(Next_token->DWC$w_itok_duration != 0))
		{
		arr_len = arr_len + DWC$k_start_token_out_len_ent;
		(*date_fmt)(Uparam, y, m, d, TRUE, pd);
		arr_len = arr_len + strlen(pd);
		(*time_fmt)(Uparam, Next_token->DWC$w_itok_start, TRUE, ps);
		arr_len = arr_len + strlen(ps);
		(*time_fmt)(Uparam, Next_token->DWC$w_itok_start+
				    Next_token->DWC$w_itok_duration, FALSE, pe);
		arr_len = arr_len + strlen(pe);
		}
	    else
		{
		arr_len = arr_len + DWC$k_start_token_out_len_not;
		(*date_fmt)(Uparam, y, m, d, FALSE, pd);
		arr_len = arr_len + strlen(pd);
		}
	    arr_len = arr_len + DWC$k_end_token_out_len +
				Next_token->DWC$l_itok_msglen;
	    }
	Next_token = Next_token->DWC$a_itok_flink;
	}
    if (arr_len == 0)
	{
	return (DWC$k_db_notoks);
	}

    /*
    **  Account for trailing NULL in output string
    */
    arr_len++;

    /*
    **  Attempt to allocate VM for output string. Back to caller if this failed.
    */
    Strptr = (char *)XtMalloc (arr_len);
    if (Strptr == 0)
	{
	return (DWC$k_db_insmem);
	}

    /*
    **  Pass output pointer back to caller.
    */
    *Int_arr = Strptr;

    /*
    **  Loop through all items and create text for each one that has any data
    **	associated with it.
    */
    Next_token = work_context->DWC$a_dbin_flink;
    while (Next_token !=
		    (struct DWC$db_itoken *)&work_context->DWC$a_dbin_flink)
	{
	if (Next_token->DWC$l_itok_msglen != 0)
	    {
	    DATEFUNCDateForDayNumber(Next_token->DWC$l_itok_day, &d, &m, &y);
	    if ((Next_token->DWC$w_itok_start != 0) ||
		(Next_token->DWC$w_itok_duration != 0))
		{
		(*date_fmt)(Uparam, y, m, d, TRUE, pd);
		(*time_fmt)(Uparam, Next_token->DWC$w_itok_start, TRUE, ps);
		(*time_fmt)(Uparam, Next_token->DWC$w_itok_start+
				    Next_token->DWC$w_itok_duration, FALSE, pe);
		start_hour = Next_token->DWC$w_itok_start / 60;
		start_min = Next_token->DWC$w_itok_start % 60;
		end_hour = (Next_token->DWC$w_itok_start +
				Next_token->DWC$w_itok_duration) / 60;
		end_min = (Next_token->DWC$w_itok_start +
				Next_token->DWC$w_itok_duration) % 60;
				
		sprintf(Strptr, "\n%s\n%s\n%s\n\n{%s:%04d.%02d.%02d;%02d:%02d-%02d:%02dT}\n\n",
				pd, ps, pe,
				DWC$t_start_token, y, m ,d,
				start_hour, start_min,
				end_hour, end_min);
		Strptr = Strptr + DWC$k_start_token_out_len_ent +
			 strlen(pd) + strlen(ps) + strlen(pe);
		}
	    else
		{
		(*date_fmt)(Uparam, y, m, d, FALSE, pd);
		sprintf(Strptr, "\n%s\n\n{%s:%04d.%02d.%02d;T}\n\n",
				pd,
				DWC$t_start_token, y, m ,d);
		Strptr = Strptr + DWC$k_start_token_out_len_not +
			    strlen(pd);
		}
	    memcpy
	    (
		Strptr,
		Next_token->DWC$a_itok_message,
		Next_token->DWC$l_itok_msglen
	    );
	    Strptr = Strptr + Next_token->DWC$l_itok_msglen;
	    sprintf(Strptr, "\n\n{%s}\n\n", DWC$t_start_token);
	    Strptr = Strptr + DWC$k_end_token_out_len;
	    }
	Next_token = Next_token->DWC$a_itok_flink;
	}
    

    /*
    **  Terminate output string with a NULL
    */
    Strptr[0] = DWC$k_char_null;
    
    /*
    **  Done, back to caller
    */
    return (DWC$k_db_normal);
}

int DWC$DB_Create_i_handle
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange **work_context)
#else	/* no prototypes */
	(Cab, work_context)
	struct DWC$db_access_block *Cab;
	struct DWC$db_interchange **work_context;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine creates an empty interchange handle to be used for
**	export of data items. Use DWC$DB_Put_i_item to add data to handle
**	and then DWC$DB_Write_interchange to turn it into an ASCIZ string.
**
**	Please note that caller is responsible for deallocating the
**	interchange context, when done, using DWC$DB_Rundown_interchange.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	work_context : User buffer, by ref, to receive interchange handle
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
**	DWC$k_db_normal	    -- Handle created
**	DWC$k_db_insmem	    -- Not enough memory to create handle
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    struct DWC$db_interchange *Wi_new;		/* Temp interchange context */
    
    /*
    **  Allocate interchange context. Back to caller if this failed
    */
    Wi_new = (struct DWC$db_interchange *)
		XtMalloc (sizeof(struct DWC$db_interchange));
    if (Wi_new == 0)
	{
	return (DWC$k_db_insmem);
	}

    /*
    **  Initialize interchange context
    */
    Wi_new->DWC$l_dbin_line = 1;
    Wi_new->DWC$a_dbin_ntoken = 0;
    Wi_new->DWC$a_dbin_flink =
	    (struct DWC$db_itoken *)&(Wi_new->DWC$a_dbin_flink);
    Wi_new->DWC$a_dbin_blink = Wi_new->DWC$a_dbin_flink;
    Wi_new->DWC$a_dbin_next = Wi_new->DWC$a_dbin_flink;

    /*
    **  Done, back to caller
    */
    *work_context = Wi_new;    
    return (DWC$k_db_normal);
}

int DWC$DB_Put_i_item
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	int Day,
	int Start,
	int Duration,
	unsigned char *Data_ptr,
	int Data_len,
	int Data_class)
#else	/* no prototypes */
	(Cab, work_context, Day, Start, Duration,
	    Data_ptr, Data_len, Data_class)
	struct DWC$db_access_block *Cab;
	struct DWC$db_interchange *work_context;
	int Day;
	int Start;
	int Duration;
	unsigned char *Data_ptr;
	int Data_len;
	int Data_class;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine appends an item to the interchange handle for later
**	export.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	work_context : handle that was returnd by Parse or Create
**	Day : Day of item
**	Start : Starting minute of item
**	Duration : Duration in minutes of item; if Start and Duration are
**		   both zero the item will be treated as a Daynote, else
**		   a normal item
**	Data_ptr : Pointer to data to associate with item
**	Data_len : Number of bytes of data
**	Data_class : Type of data:
**
**			DWC$k_item_text		-- 8bit text (MCS); without
**						   trailing NULL
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
**      DWC$k_db_normal	    -- Item appended to interchange handle
**	DWC$k_db_insmem	    -- Could not allocate memory to append item
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    struct DWC$db_itoken *Next_item;	/* Output item for token processed  */

    /*
    **  Allocate item buffer; back to caller if no memory
    */
    Next_item = (struct DWC$db_itoken *)XtMalloc (sizeof(struct DWC$db_itoken));
    if (Next_item == 0)
	{
	return (DWC$k_db_insmem);
	}

    /*
    **  Fill in first part of item structure
    */
    Next_item->DWC$l_itok_msglen = Data_len;
    Next_item->DWC$a_itok_message = 0;
    Next_item->DWC$l_itok_day = Day;
    Next_item->DWC$b_itok_mode = Data_class;
    Next_item->DWC$w_itok_start = Start;
    Next_item->DWC$w_itok_duration = Duration;

    /*
    **  Now, allocate memory for data and copy it
    */
    Next_item->DWC$a_itok_message = (char *)XtMalloc (Data_len);
    if (Next_item->DWC$a_itok_message == 0)
	{
	XtFree (Next_item);
	return (DWC$k_db_insmem);
	}
    memcpy(Next_item->DWC$a_itok_message, Data_ptr, Data_len);

    /*
    **  Add entry at end of queue and back to caller
    */
    DWC$$DB_Insque
    (
	(struct DWC$db_queue_head *) Next_item,
	(struct DWC$db_queue_head *) work_context->DWC$a_dbin_flink
    );
    return (DWC$k_db_normal);
}

int DWC$$DB_Rundown_wi
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	int status)
#else	/* no prototypes */
	(Cab, work_context, status)
	struct DWC$db_access_block *Cab;
	struct DWC$db_interchange *work_context;
	int status;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This is the internal routine for cleaning up the work context used by a
**	Parse or a Create. All memory is deallocated.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	work_context : work context to run-down
**	status : status code to return on completion.
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
**      status
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    struct DWC$db_itoken *Next_token;		/* Next work token	*/

    /*
    **  Remove next item from queue. Go anything?
    */
    while
    (
	DWC$$DB_Remque
	(
	    (struct DWC$db_queue_head *) &work_context->DWC$a_dbin_flink,
	    (struct DWC$db_queue_head **) &Next_token
	)
    )
    {

	/*
	**  Yes. Free text data, if any
	*/
	if (Next_token->DWC$a_itok_message != 0)
	{
	    if (Next_token->DWC$b_itok_mode == DWC$k_item_cstr)
	    {
		XmStringFree (Next_token->DWC$a_itok_message);
	    }
	    else
	    {
		XtFree (Next_token->DWC$a_itok_message);
	    }
	}

	/*
	**  Release the token itself
	*/
	XtFree (Next_token);
    }

    /*
    **  No more tokens, release the work context too
    */
    XtFree (work_context);

    /*
    **  Done, back to caller with whatever status caller wanted us to return
    */
    return (status);
}    

int DWC$DB_Rundown_interchange
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context)
#else	/* no prototypes */
	(Cab, work_context)
	struct DWC$db_access_block *Cab;
	struct DWC$db_interchange *work_context;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine cleans up a work context after a Parse or Create.
**	All memory is deallocated.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	work_context : work context to run-down
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
**      DWC$k_db_normal	    -- Success
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    int status;		/* Rundown status		*/

    /*
    **  Define error base, in case of failure
    */
    _Set_base(DWC$_RUNDF);

    /*
    **  Do real rundown and back to caller
    */
    status = DWC$$DB_Rundown_wi(Cab, work_context, DWC$k_db_normal);
    _Pop_cause;
    return (status);

}    

int DWC$DB_Parse_interchange
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	char *tokens,
	struct DWC$db_interchange **work_context,
	int *err_line)
#else	/* no prototypes */
	(Cab, tokens, work_context, err_line)
	struct DWC$db_access_block *Cab;
	char *tokens;
	struct DWC$db_interchange **work_context;
	int *err_line;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine parses a NULL terminated string in memory for
**	DECwindows calendar data tokens. The routine will build up
**	a parse context (if any data was found). The context can then
**	be examined with DWC$DB_Get_next_i_item.
**
**	The string in memory will be treated as a "file", where NULL
**	represents EOF. <New_line> characters represent the end of
**	the line. During the parse, the code keeps track of on what
**	line it is. If the parse would fail, due to bad syntax, the
**	routine will indicate on what "line" the failure occurred.
**
**	The caller is responsible for deallocating the context block
**	when done. This is done by calling DWC$DB_Rundown_interchange.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	tokens : pointer to NULL terminated string to be parsed. This
**		 could be the output from the load routine.
**	work_context : user supplied field (by ref) to receive
**		       the work context handle, if parse was successful.
**	err_line : user supplied integer (by ref) to receive the
**		   failing line number (starting with 1) where the
**		   error was detected.
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
**      DWC$k_db_normal	    -- Parse successful
**	DWC$k_db_insmem	    -- Not enough memory to complete parse, aborted
**	DWC$k_db_notoks	    -- Input text string does not contain any valid
**			       segments, or the ones that are there contain
**			       no text.
**	DWC$k_db_invdate    -- One token contains an invalid date format or
**			       the date does not make sense. It is also
**			       possible that the divider between the date
**			       and the starting time is wrong. See err_line
**			       for failing line.
**	DWC$k_db_unexpend   -- Unexpected end of file reached (while
**			       parsing valid tokens). See err_line for
**			       failing line.
**	DWC$k_db_invtime    -- One token contains an invalid time field.
**			       See err_line for failing line.
**	DWC$k_db_badisyn    -- Other part of token is invalid. See err_line
**			       for failing line.
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    struct DWC$db_interchange *Wi_new;		/* Temp interchange context */
    struct DWC$db_itoken *Next_token;		/* Next item on list	    */
    int status;					/* Temp work status	    */
    
    /*
    **  Allocate interchange context. Back to caller if this failed
    */
    status = DWC$DB_Create_i_handle(Cab, &Wi_new);
    if (status != DWC$k_db_normal)
	{
	return (status);
	}

    /*
    **  Save pointer to "next" token to be parsed
    */
    Wi_new->DWC$a_dbin_ntoken = tokens;
    
    /*
    **  Enter main processing loop. For each valid start of DWC data token that
    **	we find, process it. If anything fails, get out of loop.
    */
    status = DWC$k_db_normal;
    while (DWC$$DB_Find_interchange(Cab, Wi_new))
	{
	status = DWC$$DB_Treat_token(Cab, Wi_new);
	if (status != DWC$k_db_normal)
	    {
	    break;
	    }
	}

    /*
    **  Provided things are Ok now, make sure we have at least one entry on the
    **	list with non-zero length text.
    */
    if (status == DWC$k_db_normal)
	{
	status = DWC$k_db_notoks;
	Next_token = Wi_new->DWC$a_dbin_flink;
	while (Next_token !=
		(struct DWC$db_itoken *)&(Wi_new->DWC$a_dbin_flink))
	    {
	    if (Next_token->DWC$l_itok_msglen != 0)
		{
		status = DWC$k_db_normal;
		break;
		}
	    Next_token = Next_token->DWC$a_itok_flink;
	    }
	}
	
    /*
    **  If anything failed, tell caller which line in the input text it was and
    **	get rid of the interchange context
    */
    if (status != DWC$k_db_normal)
	{
	*err_line = Wi_new->DWC$l_dbin_line;
	return (DWC$$DB_Rundown_wi(Cab, Wi_new, status));
	}

    /*
    **  Else, pass pointer to interchange context back to caller and back in
    **	success
    */
    *work_context = Wi_new;
    return (status);
}

int DWC$$DB_Find_interchange
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context)
#else	/* no prototypes */
	(Cab, work_context)
	struct DWC$db_access_block *Cab;
	struct DWC$db_interchange *work_context;
#endif	/* prototypes */
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine searches the input stream for next start of DWC data token.
**	When found or EOF, the routine will return.
**	
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	work_context : Interchange context handle
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
**	TRUE		    -- Next token found
**	FALSE		    -- No more tokens
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    
    unsigned char next_chr;		/* Next byte to process		    */
    int status;				/* Temp work status		    */
    int scan_status;			/* Status from sscanf		    */

    /*
    **  Assume that we're not going to find a starting token and enter loop to
    **	scan for token.
    */
    status = FALSE;
    while (TRUE)
	{

	/*
	**  EOF?
	*/
	if (!(next_chr = *work_context->DWC$a_dbin_ntoken++))
	    {
	    break;
	    }

	/*
	**  Possible start token? Check it and get out of loop if we found
	**  something or if we reached EOF
	*/
	if (next_chr == DWC$k_char_open_brace)
	    {
	    scan_status = sscanf(work_context->DWC$a_dbin_ntoken,
		    DWC$t_start_token_hashed, &next_chr);
	    if (scan_status == EOF)
		{
		break;
		}
	    if (scan_status == 1)
		{
		status = TRUE;
		work_context->DWC$a_dbin_ntoken =
		    work_context->DWC$a_dbin_ntoken + DWC$k_token_len;
		break;
		}
	    }
	}

    /*
    **  Back to caller with scan status
    */
    return (status);
}
	
int DWC$$DB_Treat_token
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context)
#else	/* no prototypes */
	(Cab, work_context)
	struct DWC$db_access_block *Cab;
	struct DWC$db_interchange *work_context;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine is invoked when a start of DWC data token has been
**	recognized (and processed). This routine will extract the data
**	associated with the entry and record this in an item block. This item
**	block is then added to the end of the list of day items associated with
**	this interchange context.
**	
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	work_context : Interchange context handle
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
**      DWC$k_db_normal	    -- Data extracted and item appended to list
**	DWC$k_db_insmem	    -- Not enough memory to complete parse, aborted
**	DWC$k_db_notoks	    -- Input text string does not contain any valid
**			       segments, or the ones that are there contain
**			       no text.
**	DWC$k_db_invdate    -- One token contains an invalid date format or
**			       the date does not make sense. It is also
**			       possible that the divider between the date
**			       and the starting time is wrong. See err_line
**			       for failing line.
**	DWC$k_db_unexpend   -- Unexpected end of file reached (while
**			       parsing valid tokens). See err_line for
**			       failing line.
**	DWC$k_db_invtime    -- One token contains an invalid time field.
**			       See err_line for failing line.
**	DWC$k_db_badisyn    -- Other part of token is invalid. See err_line
**			       for failing line.
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    unsigned char next_chr;		/* Next byte to process		    */
    unsigned int year;			/* Year of entry		    */
    unsigned int month;			/* Month of entry		    */
    unsigned int day;			/* Day of entry			    */
    unsigned int start_hour;		/* Starting hour of entry	    */
    unsigned int start_min;		/* Starting minute of entry	    */
    unsigned int start_time;		/* Starting time (minute of day)    */
    unsigned int end_hour;		/* Ending hour of entry		    */
    unsigned int end_min;		/* Ending minute of entry	    */
    unsigned int end_time;		/* Ending time (minute of day)	    */
    int duration;			/* Duration of entry in minutes	    */
    struct DWC$db_itoken *Next_item;	/* Output item for token processed  */
    int status;				/* Temp work status		    */
    int dwcday;				/* Starting DWC day of entry	    */
    
    /*
    **  Get year
    */
    status = DWC$$DB_Get_inum (Cab, work_context, &year);

    if (status != DWC$k_db_normal)
	{
	if (status == DWC$k_db_shortnum)
	    {
	    status = DWC$k_db_invdate;
	    }
	return (status);
	}
    
    /*
    **  Separation between year and month
    */
    if (!(next_chr = *work_context->DWC$a_dbin_ntoken++))
	{
	return (DWC$k_db_unexpend);
	}
    if (next_chr != DWC$k_char_dot)
	{
	return (DWC$k_db_invdate);
	}

    /*
    **  Get month
    */
    status = DWC$$DB_Get_inum(Cab, work_context, &month);
    if (status != DWC$k_db_normal)
	{
	if (status == DWC$k_db_shortnum)
	    {
	    status = DWC$k_db_invdate;
	    }
	return (status);
	}
    
    /*
    **  Separation between month and day
    */
    if (!(next_chr = *work_context->DWC$a_dbin_ntoken++))
	{
	return (DWC$k_db_unexpend);
	}
    if (next_chr != DWC$k_char_dot)
	{
	return (DWC$k_db_invdate);
	}

    /*
    **  Get day
    */
    status = DWC$$DB_Get_inum(Cab, work_context, &day);
    if (status != DWC$k_db_normal)
	{
	if (status == DWC$k_db_shortnum)
	    {
	    status = DWC$k_db_invdate;
	    }
	return (status);
	}

    /*
    **  Check fields
    */
    if ((month == 0) ||
	(month > 12) ||
	(day == 0))
	{
	return (DWC$k_db_invdate);
	}
    if (year < 100)
	{
	year = year + 1900;
	}
    if (DATEFUNCDaysInMonth(month, year) < day)
	{
	return (DWC$k_db_invdate);
	}
    dwcday = (DATEFUNCDaysSinceBeginOfTime(day, month, year));
    if (dwcday == -1)
	{
	return (DWC$k_db_invdate);
	}
	
    /*
    **  Pick up divider between date and time
    */
    if (!(next_chr = *work_context->DWC$a_dbin_ntoken++))
	{
	return (DWC$k_db_unexpend);
	}
    if (next_chr != DWC$k_char_semicolon)
	{
	return (DWC$k_db_invdate);
	}

    /*
    **  Is this possibly a daynote?
    */
    if (*work_context->DWC$a_dbin_ntoken == DWC$k_char_T)
	{

	/*
	**  Yes, set starting time and duration to zero
	*/
	start_time = 0;
	duration = 0;

	/*
	**  Advance pointer
	*/
	work_context->DWC$a_dbin_ntoken++;
	}
    else
	{

	/*
	**  No, normal entry .. parse out time. Start with starting hour
	*/
	status = DWC$$DB_Get_inum(Cab, work_context, &start_hour);
	if (status != DWC$k_db_normal)
	    {
	    if (status == DWC$k_db_shortnum)
		{
		status = DWC$k_db_invtime;
		}
	    return (status);
	    }

	/*
	**  Separation between hour and minutes
	*/
	if (!(next_chr = *work_context->DWC$a_dbin_ntoken++))
	    {
	    return (DWC$k_db_unexpend);
	    }
	if (next_chr != DWC$k_char_colon)
	    {
	    return (DWC$k_db_invtime);
	    }
	
	/*
	**  Starting minute
	*/
	status = DWC$$DB_Get_inum(Cab, work_context, &start_min);
	if (status != DWC$k_db_normal)
	    {
	    if (status == DWC$k_db_shortnum)
		{
		status = DWC$k_db_invtime;
		}
	    return (status);
	    }
	
	/*
	**  Separation between starting time and ending time
	*/
	if (!(next_chr = *work_context->DWC$a_dbin_ntoken++))
	    {
	    return (DWC$k_db_unexpend);
	    }
	if (next_chr != DWC$k_char_dash)
	    {
	    return (DWC$k_db_invtime);
	    }
	    
	/*
	**  Ending hour
	*/
	status = DWC$$DB_Get_inum(Cab, work_context, &end_hour);
	if (status != DWC$k_db_normal)
	    {
	    if (status == DWC$k_db_shortnum)
		{
		status = DWC$k_db_invtime;
		}
	    return (status);
	    }

	/*
	**  Separation between hour and minutes
	*/
	if (!(next_chr = *work_context->DWC$a_dbin_ntoken++))
	    {
	    return (DWC$k_db_unexpend);
	    }
	if (next_chr != DWC$k_char_colon)
	    {
	    return (DWC$k_db_invtime);
	    }
	
	/*
	**  Ending minute
	*/
	status = DWC$$DB_Get_inum(Cab, work_context, &end_min);
	if (status != DWC$k_db_normal)
	    {
	    if (status == DWC$k_db_shortnum)
		{
		status = DWC$k_db_invtime;
		}
	    return (status);
	    }
	
	/*
	**  Validate fields
	*/
	if ((start_hour > 23) ||
	    (start_min > 59) ||
	    (end_hour > 23) ||
	    (end_min > 59))
	    {
	    return (DWC$k_db_invtime);
	    }
	start_time = (start_hour * 60) + start_min;
	end_time = (end_hour * 60) + end_min;
	if (start_time > end_time)
	    {
	    return (DWC$k_db_invtime);
	    }
	duration = end_time - start_time;
	if (duration == 0)
	    {
	    if (start_time != 0)
		{
		return (DWC$k_db_invtime);
		}
	    }
	/*
	**  Get data type (must be T=Text) for now
	*/
	if (!(next_chr = *work_context->DWC$a_dbin_ntoken++))
	    {
	    return (DWC$k_db_unexpend);
	    }
	if (next_chr != DWC$k_char_T)
	    {
	    return (DWC$k_db_badisyn);
	    }
	}
    
    /*
    **  Get trailing brace
    */
    if (!(next_chr = *work_context->DWC$a_dbin_ntoken++))
	{
	return (DWC$k_db_unexpend);
	}
    if (next_chr != DWC$k_char_close_brace)
	{
	return (DWC$k_db_badisyn);
	}
    
    /*
    **  Allocate item block, initialize it and add it to the end of the queue of
    **	items
    */
    Next_item = (struct DWC$db_itoken *)XtMalloc (sizeof(struct DWC$db_itoken));
    if (Next_item == 0)
	{
	return (DWC$k_db_insmem);
	}
    Next_item->DWC$l_itok_msglen = 0;
    Next_item->DWC$a_itok_message = 0;
    Next_item->DWC$l_itok_day = dwcday;
    Next_item->DWC$b_itok_mode = DWC$k_item_cstr;
    Next_item->DWC$w_itok_start = start_time;
    Next_item->DWC$w_itok_duration = duration;
    DWC$$DB_Insque
    (
	(struct DWC$db_queue_head *) Next_item,
	(struct DWC$db_queue_head *) work_context->DWC$a_dbin_flink
    );
    
    /*
    **  Parse the rest of the text and back to caller
    */
    return (DWC$$DB_Treat_text(Cab, work_context, Next_item));
}    

int DWC$$DB_Treat_text
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	struct DWC$db_itoken *work_item)
#else	/* no prototypes */
	(Cab, work_context, work_item)
	struct DWC$db_access_block *Cab;
	struct DWC$db_interchange *work_context;
	struct DWC$db_itoken *work_item;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine is called when the start of DWC data token has been
**	completely parsed. What should follow now is the text associated with
**	the item.
**
**	This routine extracts the text that follows up to EOF or the ending
**	token. All text is stored in one NULL terminated string. Leading and
**	trailing blanks are removed.
**
**	The routine records data in an intermediate buffer. When the
**	intermediate buffer is full it extends the current output buffer to hold
**	this data too. This reduces the number of memory extends significantly.
**	
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	work_context : Interchange context handle
**	work_item : Current item being worked on
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
**	DWC$k_db_normal	    -- Text extracted
**	DWC$k_db_insmem	    -- Could not allocate memory to hold output string
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    unsigned char next_chr;		/* Next byte from input stream	    */
    unsigned char work_buff[DWC$k_max_load_line]; /* Temp work buffer	    */
    int char_count;			/* Number of chars in this block    */
    int done;				/* Boolean: TRUE if done	    */
    int scan_status;			/* Result from end-of-text check    */
    int needed;				/* Number of bytes needed for next  */
    unsigned char *new_buffer;		/* Output buffer		    */
    int text_seen;			/* Boolean: TRUE if non-blank seen  */
    unsigned char	*temp_xm_text;
    long		byte_count, cvt_status;

    /*
    **  Initialize the numer of output characters to zero.
    */
    char_count = 0;

    /*
    **  Indicate that we're not done and that we have not seen a single
    **	non-space character (yet).
    */
    done = FALSE;
    text_seen = FALSE;

    /*
    **  Enter main loop to pick up chunks of text
    */
    while (!done)
    {

	/*
	**  Enter chunk processing loop. This loop will terminate if one of the
	**  following conditions is satisfied:
	**
	**	1. No more data to process (EOF)
	**	2. End of DWC entry token seen
	**	3. Intermediate "chunk" buffer is full
	*/
	while (TRUE)
	{

	    /*
	    **  EOF?
	    */
	    if (!(next_chr = *work_context->DWC$a_dbin_ntoken++))
	    {
		done = TRUE;
		break;
	    }

	    /*
	    **  Possible end of data token? If so, check it out
	    */
	    if (next_chr == DWC$k_char_open_brace)
	    {
		scan_status = sscanf(work_context->DWC$a_dbin_ntoken,
			DWC$t_end_token_hashed, &next_chr);
		if (scan_status == EOF)
		{
		    done = TRUE;
		    break;
		}
		if (scan_status == 1)
		{
		    work_context->DWC$a_dbin_ntoken =
			work_context->DWC$a_dbin_ntoken + DWC$k_token_len;
		    done = TRUE;
		    break;
		}
	    }

	    /*
	    **  Record character, unless space and we've not seen anything but
	    **	spaces so far. Else record char.
	    */
	    if (!text_seen)
	    {
		switch (next_chr)
		{
		    case DWC$k_char_space :
		    case DWC$k_char_tab :
		    case DWC$k_char_newline :
		    {
			break;
		    }
		    default :
		    {
			work_buff[char_count++] = next_chr;
			text_seen = TRUE;
		    }
		}
	    }
	    else
	    {
		work_buff[char_count++] = next_chr;
	    }

	    /*
	    **  Exit loop if "chunk" buffer is full
	    */
	    if (char_count == DWC$k_max_load_line)
	    {
		break;
	    }
	}

	/*
	**  Any data to append to text buffer?
	*/
	if (char_count == 0)
	{

	    /*
	    **  No. If data was altogether blank, record at least one
	    **	NEW-LINE character
	    */
	    if (work_item->DWC$l_itok_msglen != 0)
	    {
		break;
	    }
	    work_buff[0] = DWC$k_char_newline;
	    char_count++;
	}

	/*
	**  Determine how many bytes we need to allocate for the new buffer. We
	**  need to allocate both for what we found in this last pass, but also
	**  for what we already have saved. Once new buffer is allocated, copy
	**  old data (if any) into it and get rid of old buffer.
	*/
	needed = char_count;
	if (work_item->DWC$l_itok_msglen == 0)
	{
	    work_item->DWC$a_itok_message = (char *)XtMalloc (needed);
	    if (work_item->DWC$a_itok_message == 0)
	    {
		return (DWC$k_db_insmem);
	    }
	}
	else
	{
	    needed = needed + work_item->DWC$l_itok_msglen;
	    new_buffer = (unsigned char *)XtMalloc (needed);
	    if (new_buffer == 0)
	    {
		return (DWC$k_db_insmem);
	    }
	    memcpy(new_buffer, work_item->DWC$a_itok_message,
			work_item->DWC$l_itok_msglen);
	    XtFree (work_item->DWC$a_itok_message);
	    work_item->DWC$a_itok_message = (char *)new_buffer;
	}

	/*
	**  Append new data to it and update cound of bytes we've recorded so
	**  far.
	*/
	memcpy(&work_item->DWC$a_itok_message[work_item->DWC$l_itok_msglen],
		work_buff, char_count);
	work_item->DWC$l_itok_msglen = needed;

	/*
	**  If we're done, get out of loop. Else, zero count of character in
	**  "chunk" buffer (since we're going to pick up another chunk).
	*/
	if (done)
	{
	    break;
	}
	char_count = 0;
    }

    /*
    **  Remove trailing blanks (if any)
    */
    done = FALSE;
    for (char_count=work_item->DWC$l_itok_msglen-1; char_count>=0; char_count--)
    {
	switch (work_item->DWC$a_itok_message[char_count])
	{
	    case DWC$k_char_space :
	    case DWC$k_char_tab :
	    case DWC$k_char_newline :
	    {
		break;
	    }
	    default :
	    {
		work_item->DWC$l_itok_msglen = char_count + 1;
		done = TRUE;
		break;
	    }
	}
	if (done)
	{
	    break;
	}
    }
    
    work_item->DWC$a_itok_message[work_item->DWC$l_itok_msglen] = '\000';

    temp_xm_text = (unsigned char *) DXmCvtFCtoCS
	(work_item->DWC$a_itok_message, &byte_count, &cvt_status);
    XtFree (work_item->DWC$a_itok_message);
    work_item->DWC$a_itok_message = (char *) temp_xm_text;

    /*
    **  Back to caller in success
    */
    return (DWC$k_db_normal);
}

int DWC$$DB_Get_inum
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	unsigned int *outnum)
#else	/* no prototypes */
	(Cab, work_context, outnum)
	struct DWC$db_access_block *Cab;
	struct DWC$db_interchange *work_context;
	unsigned int *outnum;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine retreives a decimal number at the current pointer
**	position and advances the pointer. The number must be at least
**	one digit. The routine will report an error if it reached end-of-file.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	work_context : Interchange context handle
**	outnum : caller integer, by ref, to receive number.
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
**	DWC$k_db_normal	    -- Number extracted
**	DWC$k_db_unexpend   -- Unexpected EOF
**	DWC$k_db_shortnum   -- No number found (too short)
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    unsigned int next_digit;		/* Next decimal digit		*/
    unsigned int result;		/* Total result			*/
    unsigned char next_num;		/* Next byte being worked on	*/
    unsigned char *start_pos;		/* Position of first byte	*/
    

    /*
    **  Record starting byte (so that we can tell if we did not find anything).
    **	Do also initialize result.
    */
    start_pos = (unsigned char *)work_context->DWC$a_dbin_ntoken;
    result = 0;

    /*
    **  Enter extraction loop.
    */
    while (TRUE)
	{

	/*
	**  Pick up next character. Check for EOF
	*/
	if (!(next_num = *work_context->DWC$a_dbin_ntoken))
	    {
	    return (DWC$k_db_unexpend);
	    }

	/*
	**  Leave loop if this is not a decimal digit
	*/
	if (!(isdigit(next_num)))
	    {
	    break;
	    }

	/*
	**  Convert ASCII value into binary digit and merge it with working
	**  number.
	*/
	next_digit = (next_num - '0');
	result = (result * 10) + next_digit;

	/*
	**  Advance input pointer by one character and try next
	*/
	work_context->DWC$a_dbin_ntoken++;
	}	

    /*
    **  Pass result back to caller
    */
    *outnum = result;

    /*
    **  Tell caller if we did not find anything
    */
    if (work_context->DWC$a_dbin_ntoken == (char *)start_pos)
	{
	return (DWC$k_db_shortnum);
	}

    /*
    **  Done, back to caller
    */
    return (DWC$k_db_normal);
}	

int DWC$DB_Create_interchange
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	int Start_day,
	int End_day,
	int Item_id,
	void (*date_fmt)(),
	void (*time_fmt)(),
	void *Uparam,
	char **Int_arr)
#else	/* no prototypes */
	(Cab, Start_day, End_day, Item_id,
					    date_fmt, time_fmt, Uparam, Int_arr)
	struct DWC$db_access_block *Cab;
	int Start_day;
	int End_day;
	int Item_id;
	void (*date_fmt)();
	void (*time_fmt)();
	void *Uparam;
	char **Int_arr;
#endif	/* prototypes */
{
    fprintf (stderr, "DWC$DB_Create_interchange is defunct\n");
    /*
    **  Done; back to caller
    */
    return (DWC$k_db_normal);
}

int DWC$$DB_Append_int_item
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	struct DWCDB_entry *Entry,
	int day)
#else	/* no prototypes */
	(Cab, work_context, Entry, day)
	struct DWC$db_access_block *Cab;
	struct DWC$db_interchange *work_context;
	struct DWCDB_entry *Entry;
	int day;
#endif	/* prototypes */

{
    fprintf (stderr, "DWC$$DB_Append_int_item is defunct\n");
    /*
    **  Done; back to caller
    */
    return (DWC$k_db_normal);
}

int DWCDB_WriteInterchange
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	char *(*date_fmt)(),
	char *(*time_fmt)(),
	void *Uparam,
	unsigned char **Int_arr)    /* XmString * */
#else	/* no prototypes */
	(Cab, work_context, date_fmt, time_fmt, Uparam, Int_arr)
	struct DWC$db_access_block *Cab;
	struct DWC$db_interchange *work_context;
	char *(*date_fmt)();
	char *(*time_fmt)();
	void *Uparam;
	unsigned char **Int_arr;    /* XmString * */
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine creates a parsable text string from an interchange work
**	context.  The output from the routine is in the form of a NULL
**	terminated string (that can be fed into the Parse routine).
**
**	This routine will invoke caller supplied callback routines to format
**	date and time strings. The date and time used in these tokes are there
**	to make it more pretty (they have no functional meaning). In general
**	the output token looks like:
**
**	    Date/Time
**
**	    {Token_start}
**
**	    Data
**
**	    {End_token}
**
**	The date/time data depends on what token is being formatted. The
**	caller supplies two formatting routines.
**
**	    1. Produce date string:
**
**		(*date_fmt)(Uparam, year, month, date, ent_flg, outbuf)
**
**		    date_fmt	- address of user supplied callback routine
**		    Uparam	- User context parameter passed
**		    year	- Year of date to format
**		    month	- month of year
**		    day		- day of month
**		    ent_flg	- boolean; true if date is for an entry
**				  else for a daynote
**
**	    2. Produce time string
**
**		(*time_fmt)(Uparam, time, start_flg, outbuf)
**
**		    time_fmt	- address of user supplied callback routine
**		    Uparam	- User context parameter passed
**		    time	- minute of day (0..1439)
**		    start_flg	- boolean; true if time is starting time
**				  else ending time
**
**	Please note that Caller is responsible for deallocating the
**	output string with "XmStringFree". Also, any Get_next_r_item context
**	that was established prior to this call is lost.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	work_context : interchange context with data to "print"
**	date_fmt : address of date formatting callback routine
**	time_fmt : address of time formatting callback routine
**	Uparam : user parameter to be passed to callback routine
**	Int_arr : User pointer (by ref) to receive pointer to memory
**		  allocated in text was written.
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
**	DWC$k_db_normal	    -- Data retreived and context created
**	DWC$k_db_insmem	    -- Not enough memory to create string
**	DWC$k_db_notoks	    -- There are no items in this work context
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    struct DWC$db_itoken *Next_token;	    /* Next token being written	    */
    char *Strptr;			    /* Pointer to output string	    */
    int arr_len;			    /* Length of output string	    */
    int y, m, d;			    /* Year, Month and Day	    */
    int start_hour, start_min;		    /* Start hour, Start min	    */
    int end_hour, end_min;		    /* End hour, End minute	    */
    int i;				    /* Loop variable		    */

    unsigned char		*temp = NULL;		/* XmString */
    unsigned char		*temp1 = NULL;		/* XmString */
    unsigned char		*header;		/* XmString */
    char			*pd;
    char			*ps;
    char			*pe;
    int				curr_len;

    static int			buff_len = 0;
    static char			*buffer;
    static unsigned char	*trailer = NULL;	/* XmString */
    long			byte_count, cvt_status;

    /*
    **  Loop through all items and create text for each one that has any data
    **	associated with it.
    */
    Next_token = work_context->DWC$a_dbin_flink;
    while (Next_token !=
		    (struct DWC$db_itoken *)&work_context->DWC$a_dbin_flink)
    {
	/*
	** We don't do empty timeslots
	*/
	if (Next_token->DWC$l_itok_msglen == 0)
	{
	    Next_token = Next_token->DWC$a_itok_flink;
	    continue;
	}

	/*
	** Get the numeric value of the date.
	*/
	DATEFUNCDateForDayNumber(Next_token->DWC$l_itok_day, &d, &m, &y);

	/*
	** Is it a timeslot or a daynote.
	*/
	if ((Next_token->DWC$w_itok_start != 0) ||
	    (Next_token->DWC$w_itok_duration != 0))
	{
	    /*
	    ** Convert the time and date into text for the user.
	    */
	    pd = (*date_fmt)(Uparam, y, m, d, TRUE);
	    ps = (*time_fmt)(Uparam, Next_token->DWC$w_itok_start, TRUE);
	    pe = (*time_fmt)(Uparam, Next_token->DWC$w_itok_start+
				Next_token->DWC$w_itok_duration, FALSE);

	    /*
	    ** Get the numeric values for the parsable part.
	    */
	    start_hour = Next_token->DWC$w_itok_start / 60;
	    start_min = Next_token->DWC$w_itok_start % 60;
	    end_hour = (Next_token->DWC$w_itok_start +
			    Next_token->DWC$w_itok_duration) / 60;
	    end_min = (Next_token->DWC$w_itok_start +
			    Next_token->DWC$w_itok_duration) % 60;

	    /*
	    ** Is the text buffer big enough?
	    */
	    curr_len = DWC$k_start_token_out_len_ent +
		strlen(pd) + strlen(ps) + strlen(pe);
	    if (buff_len < curr_len)
	    {
		buff_len = curr_len;
		buffer = (char *)XtRealloc (buffer, buff_len);
	    }

	    /*
	    ** Format the stuff that's treatable as plain text.
	    */
	    sprintf
	    (
		buffer,
		"\n%s\n%s\n%s\n\n{%s:%04d.%02d.%02d;%02d:%02d-%02d:%02dT}\n\n",
		pd,
		ps,
		pe,
		DWC$t_start_token,
		y,
		m,
		d,
		start_hour,
		start_min,
		end_hour,
		end_min
	    );
	}
	else
	{
	    /*
	    ** Convert the date into text.
	    */
	    pd = (*date_fmt)(Uparam, y, m, d, FALSE);

	    /*
	    ** Is the text buffer big enough?
	    */
	    curr_len = DWC$k_start_token_out_len_ent + 	strlen(pd);
	    if (buff_len == 0)
	    {
		buff_len = curr_len;
		buffer = (char *)XtMalloc(buff_len);
	    }
	    else if (buff_len < curr_len)
	    {
		buff_len = curr_len;
		buffer = (char *)XtRealloc (buffer, buff_len);
	    }

	    /*
	    ** Format the stuff that's treatable as plain text.
	    */
	    sprintf
	    (
		buffer,
		"\n%s\n\n{%s:%04d.%02d.%02d;T}\n\n",
		pd,
		DWC$t_start_token,
		y,
		m,
		d
	    );
	}

	/*
	** Convert to XmStrings.
	*/
	header = (unsigned char *) DXmCvtFCtoCS
	    (buffer, &byte_count, &cvt_status);
	if (trailer == NULL)
	{
	    sprintf (buffer, "\n\n{%s}\n\n", DWC$t_start_token);
	    trailer = (unsigned char *) DXmCvtFCtoCS
		(buffer, &byte_count, &cvt_status);
	}

	/*
	** Put the header in.
	*/
	if (temp == NULL)
	{
	    temp = header;
	}
	else
	{
	    temp1 = (unsigned char *) XmStringConcat (temp, header);
	    XmStringFree (header);
	    XmStringFree (temp);
	    temp = temp1;
	}

	/*
	** Put the body in.
	*/
	if (Next_token->DWC$b_itok_mode == DWC$k_item_cstr)
	{
	    temp1 = (unsigned char *) XmStringConcat
		(temp, Next_token->DWC$a_itok_message);
	}
	else
	{
	    unsigned char   *temp2;
	    temp2 = (unsigned char *) DXmCvtFCtoCS
		(Next_token->DWC$a_itok_message, &byte_count, &cvt_status);
	    temp1 = (unsigned char *) XmStringConcat (temp, temp2);
	    XmStringFree (temp2);
	}
	XmStringFree (temp);
	temp = temp1;

	/*
	** Put the trailer on.
	*/
	temp1 = (unsigned char *) XmStringConcat (temp, trailer);
	XmStringFree (temp);
	temp = temp1;

	Next_token = Next_token->DWC$a_itok_flink;
    }

    *Int_arr = temp;

    /*
    **  Done, back to caller
    */
    return (DWC$k_db_normal);
}

int DWCDB_CreateIHandle
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange **work_context)
#else	/* no prototypes */
	(Cab, work_context)
	struct DWC$db_access_block *Cab;
	struct DWC$db_interchange **work_context;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine creates an empty interchange handle to be used for
**	export of data items. Use DWC$DB_Put_i_item to add data to handle
**	and then DWC$DB_Write_interchange to turn it into an ASCIZ string.
**
**	Please note that caller is responsible for deallocating the
**	interchange context, when done, using DWC$DB_Rundown_interchange.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	work_context : User buffer, by ref, to receive interchange handle
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
**	DWC$k_db_normal	    -- Handle created
**	DWC$k_db_insmem	    -- Not enough memory to create handle
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    struct DWC$db_interchange *Wi_new;		/* Temp interchange context */
    
    /*
    **  Allocate interchange context. Back to caller if this failed
    */
    Wi_new = (struct DWC$db_interchange *)
	XtMalloc (sizeof(struct DWC$db_interchange));
    if (Wi_new == 0)
    {
	return (DWC$k_db_insmem);
    }

    /*
    **  Initialize interchange context
    */
    Wi_new->DWC$l_dbin_line = 1;
    Wi_new->DWC$a_dbin_ntoken = 0;
    Wi_new->DWC$a_dbin_flink =
	    (struct DWC$db_itoken *)&(Wi_new->DWC$a_dbin_flink);
    Wi_new->DWC$a_dbin_blink = Wi_new->DWC$a_dbin_flink;
    Wi_new->DWC$a_dbin_next = Wi_new->DWC$a_dbin_flink;

    /*
    **  Done, back to caller
    */
    *work_context = Wi_new;    
    return (DWC$k_db_normal);
}

int DWCDB_PutIItem
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	int Day,
	int Start,
	int Duration,
	unsigned char *Data_ptr,	/* XmString */
	int Data_class)
#else	/* no prototypes */
	(Cab, work_context, Day, Start, Duration,
	    Data_ptr, Data_len, Data_class)
	struct DWC$db_access_block *Cab;
	struct DWC$db_interchange *work_context;
	int Day;
	int Start;
	int Duration;
	unsigned char *Data_ptr;	/* XmString */
	int Data_class;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine appends an item to the interchange handle for later
**	export.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	work_context : handle that was returnd by Parse or Create
**	Day : Day of item
**	Start : Starting minute of item
**	Duration : Duration in minutes of item; if Start and Duration are
**		   both zero the item will be treated as a Daynote, else
**		   a normal item
**	Data_ptr : Pointer to data to associate with item
**	Data_class : Type of data:
**
**			DWC$k_item_text		-- FC string without trailing
**						    NULL
**			DWC$k_item_cstr		-- XmString
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
**      DWC$k_db_normal	    -- Item appended to interchange handle
**	DWC$k_db_insmem	    -- Could not allocate memory to append item
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    struct DWC$db_itoken *Next_item;	/* Output item for token processed  */
    unsigned char	*xm_text;	/* XmString */
    char	*fc;
    long		byte_count, cvt_status;

    /*
    **  Allocate item buffer; back to caller if no memory
    */
    Next_item = (struct DWC$db_itoken *)XtMalloc (sizeof(struct DWC$db_itoken));
    if (Next_item == 0)
    {
	return (DWC$k_db_insmem);
    }

    /*
    **  Fill in first part of item structure
    */
    Next_item->DWC$a_itok_message = 0;
    Next_item->DWC$l_itok_day = Day;
    Next_item->DWC$b_itok_mode = Data_class;
    Next_item->DWC$w_itok_start = Start;
    Next_item->DWC$w_itok_duration = Duration;

    /*
    **  Now, allocate memory for data and copy it
    */
    if (Data_class == DWC$k_item_cstr)
    {
	Next_item->DWC$a_itok_message = (char *) XmStringCopy
	    (Data_ptr);
	if (Next_item->DWC$a_itok_message == 0)
	{
	    XtFree (Next_item);
	    return (DWC$k_db_insmem);
	}
	Next_item->DWC$l_itok_msglen = XmStringLength
	    (Next_item->DWC$a_itok_message);
    }
    else
    {
	fc = (char *) DXmCvtCStoFC
	    (Data_ptr, &byte_count, &cvt_status);
	Next_item->DWC$a_itok_message = fc;
	if (Next_item->DWC$a_itok_message == 0)
	{
	    XtFree (Next_item);
	    return (DWC$k_db_insmem);
	}
	Next_item->DWC$l_itok_msglen = byte_count;
    }

    /*
    **  Add entry at end of queue and back to caller
    */
    DWC$$DB_Insque
    (
	(struct DWC$db_queue_head *) Next_item,
	(struct DWC$db_queue_head *) work_context->DWC$a_dbin_flink
    );
    return (DWC$k_db_normal);
}

int DWCDB_LoadInterchange
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	char *Work_file,
	unsigned char **Int_arr,
	int *c_err,
	int *vaxc_err)
#else	/* no prototypes */
	(Cab, Work_file, Int_arr, c_err,  vaxc_err)
	struct DWC$db_access_block *Cab;
	char *Work_file;
	unsigned char **Int_arr;
	int *c_err;
	int *vaxc_err;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine loads a text file into memory. The routine will open
**	and close the file. The are no defaults for the file specification.
**	The output from this routine can then be fed into the parse
**	routine.
**
**	The caller will be passed a pointer to a NULL terminated string
**	containing the data that was loaded. The caller is responsible for
**	deallocating the string with "free".
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	Work_file : Pointer to ASCIZ name of file to load
**	Int_arr : User pointer (by ref) to receive pointer to memory
**		  allocated in which file has been loaded.
**	c_err : integer (by ref) to receive "errno" at time
**	          of error. A value of 0 indicates no error.
**	vaxc_err : integer (by ref) to receive "vaxc$errno" at time
**		  of error. A value of 0 indicates no error or no
**		  information. This value has meaning only on VMS.
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
**      DWC$k_db_loadf	    -- File I/O error when opening or closing the
**			       file. Please check c_err and vax_cerr for
**			       details.
**	DWC$k_db_insmem	    -- Could not allocate enough memory to hold
**			       all the data in the file. File has been
**			       closed.
**	DWC$k_db_normal	    -- File loaded
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    FILE *work_file;				    /* Input file	    */
    char *Strptr;				    /* Temp output str	    */
    char *Newptr;				    /* Newly allocated	    */
    char temp_line[DWC$k_max_load_line];	    /* Intermediate buffer  */
    int Currlen;				    /* Bytes loaded so far  */
    int Newlen;					    /* New length	    */
    long byte_count, cvt_status;

    /*
    **  Open up input file. Back to caller if this failed
    */
    work_file = fopen(Work_file, "r");
    if (work_file == 0)
    {
	_Record_errors(c_err, vaxc_err);
	return (DWC$k_db_loadf);
    }

    /*
    **  Allocate 1 byte so that we can hold the initial NULL. Back to caller if
    **	this failed.
    */
    Strptr = (char *)XtMalloc (1);
    if (Strptr == 0)
    {
	_Record_errors(c_err, vaxc_err);
	fclose(work_file);
	return (DWC$k_db_insmem);
    }
    Strptr[0] = DWC$k_char_null;
    Currlen = 1;

    /*
    **  Read lines from input file and build up resultant string. Stop if
    **	we cannot allocate memory
    */
    while (fgets(temp_line, DWC$k_max_load_line, work_file))
    {
	Newlen = strlen(temp_line);
	Newptr = (char *)XtMalloc (Currlen + Newlen);
	if (Newptr == 0)
	{
	    _Record_errors(c_err, vaxc_err);
	    fclose(work_file);
	    XtFree (Strptr);
	    return (DWC$k_db_insmem);
	}
	memcpy(Newptr, Strptr, Currlen);
	XtFree (Strptr);
	strcat(Newptr, temp_line);
	Currlen = Currlen + Newlen;
	Strptr = Newptr;
    }

    /*
    **  Done. Close file and pass string pointer back to caller. Return in
    **	success
    */
    fclose(work_file);

    if (((unsigned char)Strptr[0] == 0xff) &&
	((unsigned char)Strptr[1] == 0xff) &&
	((unsigned char)Strptr[2] == 0x7f))
	*Int_arr = (unsigned char *)DXmCvtDDIFtoCS
	    (Strptr, &byte_count, &cvt_status);
    else
	*Int_arr = (unsigned char *)DXmCvtFCtoCS
	    (Strptr, &byte_count, &cvt_status);

    return (DWC$k_db_normal);
}

int DWCDB_RundownInterchange
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context)
#else	/* no prototypes */
	(Cab, work_context)
	struct DWC$db_access_block *Cab;
	struct DWC$db_interchange *work_context;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine cleans up a work context after a Parse or Create.
**	All memory is deallocated.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	work_context : work context to run-down
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
**      DWC$k_db_normal	    -- Success
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    int status;		/* Rundown status		*/

    /*
    **  Define error base, in case of failure
    */
    _Set_base(DWC$_RUNDF);

    /*
    **  Do real rundown and back to caller
    */
    status = DWC$$DB_Rundown_wi (Cab, work_context, DWC$k_db_normal);
    _Pop_cause;
    return (status);

}    

int DWCDB_GetNextIItem
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	int *Day,
	int *Start,
	int *Duration,
	unsigned char **Data_ptr,   /* XmString * */
	int *Data_len,
	int *Data_class)
#else	/* no prototypes */
	(Cab, work_context, Day, Start, Duration,
	    Data_ptr, Data_len, Data_class)
	struct DWC$db_access_block *Cab;
	struct DWC$db_interchange *work_context;
	int *Day;
	int *Start;
	int *Duration;
	unsigned char **Data_ptr;   /* XmString * */
	int *Data_len;
	int *Data_class;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine allows you to sequentially access the results from
**	a parse. When first called for a work context, it returns the
**	first item. Subsequent calls return the "next" entry on the list
**	until there are no more items. After end-of-list has been returned,
**	next call will return the first item on the list.
**
**	The items are not sorted, other than in the order than they
**	appeared in the input text.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	work_context : handle that was returnd by Parse or Create
**	Day : user buffer (by ref) to receive dwc-day for ite,,
**	Start : user buffer (by ref) to receive starting minute for item
**	Duration : user buffer (by ref) to receive duration minutes for
**		   item
**	Data_ptr : User pointer (by ref) to receive pointer to data
**		   associated with item (not NULL terminated)
**	Data_len : User buffer (by ref) to receive number of bytes of data
**		   associated with item
**	Data_class : User buffer (by ref) to receive hint regarding what
**		     type of data it is:
**
**			DWC$k_item_text		-- 8bit text (MCS); without
**						   trailing NULL
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
**      DWC$k_db_normal	    -- Next item retreived
**	DWC$k_db_nomore	    -- No more items
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    struct DWC$db_itoken *Next_token;	    /* Next token work block	    */
    long	byte_count, cvt_status;

    /*
    **  Point at next token (item) in queue and advance pointer. Let caller know
    **	if we've reached the end of the list.
    */
    Next_token = work_context->DWC$a_dbin_next;
    Next_token = Next_token->DWC$a_itok_flink;
    work_context->DWC$a_dbin_next = Next_token;
    if (Next_token == (struct DWC$db_itoken *)&work_context->DWC$a_dbin_flink)
    {
	return (DWC$k_db_nomore);
    }
	
    /*
    **  Pass parameters back to caller
    */
    *Day = Next_token->DWC$l_itok_day;
    *Start = Next_token->DWC$w_itok_start;
    *Duration = Next_token->DWC$w_itok_duration;
    *Data_class = Next_token->DWC$b_itok_mode;
    if (*Data_class == DWC$k_item_cstr)
    {
	*Data_ptr = (unsigned char *) XmStringCopy
	    ((unsigned char *)Next_token->DWC$a_itok_message);
	*Data_len = Next_token->DWC$l_itok_msglen;
    }
    else
    {
	*Data_ptr = (unsigned char *) DXmCvtFCtoCS
	(
	    (unsigned char *)Next_token->DWC$a_itok_message,
	    &byte_count,
	    &cvt_status
	);
	*Data_len = byte_count;
    }

    /*
    **  And back in success
    */
    return (DWC$k_db_normal);
}

int DWCDB_GetAllITexts
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	unsigned char **Int_arr)    /* XmString * */
#else	/* no prototypes */
	(Cab, work_context, Int_arr)
	struct DWC$db_access_block *Cab;
	struct DWC$db_interchange *work_context;
	unsigned char **Int_arr;    /* XmString * */
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine retrieves the text for all items in the interchange
**	context as one big string. The big string is created by sequentially
**	appending the text for each item to the resultant string. The
**	routine will append a <New_line> character after each item's string.
**	The resultant string is in the form of a NULL terminated string.
**	
**	Please note that Caller is responsible for deallocating the
**	output string with "free". Also, any Get_next_r_item context
**	that was established prior to this call is lost.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	work_context : interchange context with data to "print"
**	Int_arr : User pointer (by ref) to receive pointer to memory
**		  allocated in text was written.
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
**	DWC$k_db_normal	    -- Data retreived and context created
**	DWC$k_db_insmem	    -- Not enough memory to create string
**	DWC$k_db_notoks	    -- There are no items in this work context
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    struct DWC$db_itoken *Next_token;	    /* Next token being written	    */
    char *Strptr;			    /* Pointer to output string	    */
    int arr_len;			    /* Length of output string	    */
    unsigned char	*xm_text = NULL;
    unsigned char	*temp_xm_text;
    unsigned char	*sep_text;

    /*
    ** Start with NULL pointer.
    */
    *Int_arr = NULL;

    /*
    **  Loop through all items and create text for each one that has any data
    **	associated with it.
    */
    Next_token = work_context->DWC$a_dbin_flink;
    while (Next_token !=
		    (struct DWC$db_itoken *)&work_context->DWC$a_dbin_flink)
    {
	if (Next_token->DWC$l_itok_msglen != 0)
	{
	    if (Next_token->DWC$b_itok_mode == DWC$k_item_cstr)
	    {
		temp_xm_text = (unsigned char *) XmStringConcat
		    (xm_text, Next_token->DWC$a_itok_message);
	    }
	    else
	    {
		unsigned char	*foo;
		long		byte_count, cvt_status;
		foo = (unsigned char *) DXmCvtFCtoCS
		    (Next_token->DWC$a_itok_message, &byte_count, &cvt_status);
		temp_xm_text = (unsigned char *) XmStringConcat (xm_text, foo);
		XmStringFree (foo);
	    }
	    XmStringFree (xm_text);
	    if (!temp_xm_text)
	    {
		return (DWC$k_db_insmem);
	    }
	    xm_text = temp_xm_text;
	    sep_text = (unsigned char *) XmStringSeparatorCreate ();
	    temp_xm_text = (unsigned char *) XmStringConcat (xm_text, sep_text);
	    XmStringFree (xm_text);
	    XmStringFree (sep_text);
	    if (!temp_xm_text)
	    {
		return (DWC$k_db_insmem);
	    }
	    xm_text = temp_xm_text;
	}
	Next_token = Next_token->DWC$a_itok_flink;
    }
    
    /*
    ** Pass back the XmString.
    */
    *Int_arr = xm_text;

    /*
    **  Done, back to caller
    */
    return (DWC$k_db_normal);
}

int DWCDB__FindInterchange
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context)
#else	/* no prototypes */
	(Cab, work_context)
	struct DWC$db_access_block *Cab;
	struct DWC$db_interchange *work_context;
#endif	/* prototypes */
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine searches the input stream for next start of DWC data token.
**	When found or EOF, the routine will return.
**	
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	work_context : Interchange context handle
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
**	TRUE		    -- Next token found
**	FALSE		    -- No more tokens
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    
    unsigned char next_chr;		/* Next byte to process		    */
    int status;				/* Temp work status		    */
    int scan_status;			/* Status from sscanf		    */

    /*
    **  Assume that we're not going to find a starting token and enter loop to
    **	scan for token.
    */
    status = FALSE;
    while (TRUE)
    {

	/*
	**  EOF?
	*/
	if (!(next_chr = *work_context->DWC$a_dbin_ntoken++))
	{
	    break;
	}

	/*
	**  Possible start token? Check it and get out of loop if we found
	**  something or if we reached EOF
	*/
	if (next_chr == DWC$k_char_open_brace)
	{
	    scan_status = sscanf(work_context->DWC$a_dbin_ntoken,
		    DWC$t_start_token_hashed, &next_chr);
	    if (scan_status == EOF)
	    {
		break;
	    }
	    if (scan_status == 1)
	    {
		status = TRUE;
		work_context->DWC$a_dbin_ntoken =
		    work_context->DWC$a_dbin_ntoken + DWC$k_token_len;
		break;
	    }
	}
    }

    /*
    **  Back to caller with scan status
    */
    return (status);
}

int DWCDB_ParseInterchange
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	unsigned char *tokens,	    /* XmString */
	struct DWC$db_interchange **work_context,
	int *err_line)
#else	/* no prototypes */
	(Cab, tokens, work_context, err_line)
	struct DWC$db_access_block *Cab;
	unsigned char *tokens;	    /* XmString */
	struct DWC$db_interchange **work_context;
	int *err_line;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine parses a NULL terminated string in memory for
**	DECwindows calendar data tokens. The routine will build up
**	a parse context (if any data was found). The context can then
**	be examined with DWC$DB_Get_next_i_item.
**
**	The string in memory will be treated as a "file", where NULL
**	represents EOF. <New_line> characters represent the end of
**	the line. During the parse, the code keeps track of on what
**	line it is. If the parse would fail, due to bad syntax, the
**	routine will indicate on what "line" the failure occurred.
**
**	The caller is responsible for deallocating the context block
**	when done. This is done by calling DWC$DB_Rundown_interchange.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	tokens : pointer to NULL terminated string to be parsed. This
**		 could be the output from the load routine.
**	work_context : user supplied field (by ref) to receive
**		       the work context handle, if parse was successful.
**	err_line : user supplied integer (by ref) to receive the
**		   failing line number (starting with 1) where the
**		   error was detected.
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
**      DWC$k_db_normal	    -- Parse successful
**	DWC$k_db_insmem	    -- Not enough memory to complete parse, aborted
**	DWC$k_db_notoks	    -- Input text string does not contain any valid
**			       segments, or the ones that are there contain
**			       no text.
**	DWC$k_db_invdate    -- One token contains an invalid date format or
**			       the date does not make sense. It is also
**			       possible that the divider between the date
**			       and the starting time is wrong. See err_line
**			       for failing line.
**	DWC$k_db_unexpend   -- Unexpected end of file reached (while
**			       parsing valid tokens). See err_line for
**			       failing line.
**	DWC$k_db_invtime    -- One token contains an invalid time field.
**			       See err_line for failing line.
**	DWC$k_db_badisyn    -- Other part of token is invalid. See err_line
**			       for failing line.
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    struct DWC$db_interchange *Wi_new;		/* Temp interchange context */
    struct DWC$db_itoken *Next_token;		/* Next item on list	    */
    int status;					/* Temp work status	    */
    char		*cvted_tokens;
    long		byte_count, cvt_status;
    
    /*
    ** If there is only one value here, then we can use the "old" approach
    ** else we must fall through and do the hard parse.
    */
    cvted_tokens = (char *) DXmCvtCStoFC (tokens, &byte_count, &cvt_status);
    if (cvt_status == 1 /* DXmCvtStatusOK */)
    {
	status = DWC$DB_Parse_interchange
	    (Cab, cvted_tokens, work_context, err_line);
	XtFree(cvted_tokens);
	return (status);
    }
    else if (cvt_status == 3 /* DXmCvtStatusFail */)
    {
	return (DWC$k_db_notoks);
    }

#if 1
    /*
    ** Hold the place till we figure out the new structure.
    */
    status = DWC$DB_Parse_interchange
	(Cab, cvted_tokens, work_context, err_line);
    XtFree(cvted_tokens);
    return (status);
#else
    /*
    **  Allocate interchange context. Back to caller if this failed
    */
    status = DWCDB_CreateIHandle(Cab, &Wi_new);
    if (status != DWC$k_db_normal)
    {
	return (status);
    }
    /*
    **  Save pointer to "next" token to be parsed
    */
    Wi_new->DWC$a_dbin_ntoken = cvted_tokens;
    /*
    **  Enter main processing loop. For each valid start of DWC data token that
    **	we find, process it. If anything fails, get out of loop.
    */
    status = DWC$k_db_normal;
    while (DWCDB_FindInterchange(Cab, Wi_new))
    {
	status = DWC$$DB_Treat_token(Cab, Wi_new);
	if (status != DWC$k_db_normal)
	{
	    break;
	}
    }

    /*
    **  Provided things are Ok now, make sure we have at least one entry on the
    **	list with non-zero length text.
    */
    if (status == DWC$k_db_normal)
    {
	status = DWC$k_db_notoks;
	Next_token = Wi_new->DWC$a_dbin_flink;
	while (Next_token !=
		(struct DWC$db_itoken *)&(Wi_new->DWC$a_dbin_flink))
	{
	    if (Next_token->DWC$l_itok_msglen != 0)
	    {
		status = DWC$k_db_normal;
		break;
	    }
	    Next_token = Next_token->DWC$a_itok_flink;
	}
    }
	
    /*
    **  If anything failed, tell caller which line in the input text it was and
    **	get rid of the interchange context
    */
    if (status != DWC$k_db_normal)
    {
	*err_line = Wi_new->DWC$l_dbin_line;
	return (DWC$$DB_Rundown_wi(Cab, Wi_new, status));
    }

    /*
    **  Else, pass pointer to interchange context back to caller and back in
    **	success
    */
    *work_context = Wi_new;
    return (status);
#endif
}

/*
**  End of Module
*/
