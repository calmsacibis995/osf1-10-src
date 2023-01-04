/* dwc_db_create_calendar.c */
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
**	This module contains the code for creating a new and empty
**	DECwindows Calendar database.
**
**--
*/

/*
** Include Files
*/
#include "dwc_compat.h"

#include <string.h>

#ifdef VMS
#include <unixio.h>
#include <file.h>
#else
#include <limits.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/uio.h>
#endif

#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"

/*
**  Error handling macros
*/
#ifdef VMS
#define _Report_error  {\
		    *C_errno = errno; \
		    *Vaxc_errno = vaxc$errno; \
		    }
#else
#define _Report_error  {\
		    *C_errno = errno; \
		    *Vaxc_errno = 0; \
		    }
#endif
#define _Writer(Fd,Rec) {if ((write(Fd, &Rec, DWC$k_db_record_length))\
			    != DWC$k_db_record_length) {\
				_Report_error; \
				return (DWC$k_db_failure);}}

int DWC$DB_Create_calendar
#ifdef	_DWC_PROTO_
	(
	char *Calname,
	int Delta_now,
	int *C_errno,
	int *Vaxc_errno)
#else	/* no prototypes */
	(Calname, Delta_now, C_errno, Vaxc_errno)
	char *Calname;
	int Delta_now;
	int *C_errno;
	int *Vaxc_errno;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This routine creates a new, empty calendar database file.
**
**  FORMAL PARAMETERS:
**
**	Calname : Name of calendar database to create
**	Delta_now : Todays day; used when creating first part of
**		    infra structure
**	C_errno : User integer, by ref, to receive "errno" in case
**		  creation would fail.
**	Vaxc_errno : User integer, by ref, to receive "vaxc$errno" in
**		     case creation would fail. This variable has meaning
**		     only on VMS.
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
**	DWC$k_db_normal	    -- Database created
**	DWC$k_db_failure    -- General failure; please check C_errno and
**			       possibly Vaxc_errno).
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    struct DWCDB_header Header;	/* Header block */
    struct DWCDB_bitmap Bitmap;	/* Bitmap block */
    struct DWCDB_profile Profile;	/* Profile block */
    struct DWCDB_rangemap Range;	/* Rangemap block */
    int fd;				/* File descriptor */
    int loop;				/* Loop counter */

    /*
    **  Create the file, empty. Set protection to be OWNER Read+Write
    */
    /* O:READ+WRITE, others: no-acc */
    fd = creat(Calname, DWC$t_db_file_mode, DWC$t_db_file_attributes);
    if (fd == -1)
    {
	_Report_error;
	return (DWC$k_db_failure);
    }
    
    /*
    **  Attempt to extend the file 4 records and in the meantime, fill
    **  those records with data indicating that the records are not used
    **	at all.
    */
    memset(&Header, 0, DWC$k_db_record_length);
    Header.DWC$b_dbhd_blocktype = DWC$k_db_uninitialized;
    for (loop = 1; loop <= 4; loop++)
    {
	if ((write(fd, &Header, DWC$k_db_record_length))
		!= DWC$k_db_record_length)

	{
	    _Report_error;
	    close(fd);
	    return (DWC$k_db_failure);
	}		    
    }

    /*
    **  Initialize the rangemap and write it out
    */
    memset(&Range, 0, DWC$k_db_record_length);
    Range.DWC$b_dbra_blocktype = DWC$k_db_rangemap;
    Range.DWC$l_dbra_baseday =
	(Delta_now - (Delta_now % DWC$k_db_days_per_rangemap));
    if (lseek(fd, (long) (DWC$k_db_record_length * 3), 0) == -1)
    {
	_Report_error;
	return (DWC$k_db_failure);
    }
#if BYTESWAP
    DWC$$DB_Byte_swap_buffer (&Range, FALSE);
#endif
    _Writer(fd, Range);
#if BYTESWAP
    DWC$$DB_Byte_swap_buffer (&Range, TRUE);
#endif

    /*
    **  Initialize Profile and write it out
    */
    memset(&Profile, 0, DWC$k_db_record_length);
    Profile.DWC$b_dbpr_blocktype = DWC$k_db_profile;
    if (lseek(fd, (long) (DWC$k_db_record_length * 2), 0) == -1)
    {
	_Report_error;
	return (DWC$k_db_failure);
    }
#if BYTESWAP
    DWC$$DB_Byte_swap_buffer (&Profile, FALSE);
#endif
    _Writer(fd, Profile);
#if BYTESWAP
    DWC$$DB_Byte_swap_buffer (&Profile, TRUE);
#endif

    /*
    **  Initialize the in-memory copy of the header. The _Rzero above has
    **  already cleared out fields that should be zero
    */
    Header.DWC$b_dbhd_blocktype = DWC$k_db_header;
    Header.DWC$b_dbhd_version = DWC$k_db_version;
    Header.DWC$b_dbhd_creator = DWC$k_db_creator;
    Header.DWC$l_dbhd_current_range = (DWC$k_db_record_length * 3);
    Header.DWC$l_dbhd_first_range = (DWC$k_db_record_length * 3);
    Header.DWC$l_dbhd_profile = (DWC$k_db_record_length * 2);
    Header.DWC$l_dbhd_deq = DWC$k_db_deq;
    Header.DWC$l_dbhd_current_base = Range.DWC$l_dbra_baseday;
    (void)strcpy(Header.DWC$t_dbhd_ident, DWC$t_database_version);
    (void)strcpy(Header.DWC$t_dbhd_creator, DWC$t_database_creator);
    if (lseek(fd, (long) DWC$k_db_record_length, 0) == -1)
    {
	_Report_error;
	return (DWC$k_db_failure);
    }
#if BYTESWAP
    DWC$$DB_Byte_swap_buffer(&Header, FALSE);
#endif
    _Writer(fd, Header);    
#if BYTESWAP
    DWC$$DB_Byte_swap_buffer(&Header, TRUE);
#endif

    /*
    **  Initialize bitmap and write it out
    */
    memset(&Bitmap, 0, DWC$k_db_record_length);
    Bitmap.DWC$b_dbbi_blocktype = DWC$k_db_bitmap;
    Bitmap.DWC$w_dbbi_eob = 3; /* 3rd (starting with zero) */
    Bitmap.DWC$t_dbbi_bitmap[0] = 15; /* First four bits are used */
    if (lseek(fd, 0L, 0) == -1)
    {
	_Report_error;
	return (DWC$k_db_failure);
    }
#if BYTESWAP
    DWC$$DB_Byte_swap_buffer(&Bitmap, FALSE);
#endif
    _Writer(fd, Bitmap);
#if BYTESWAP
    DWC$$DB_Byte_swap_buffer(&Bitmap, TRUE);
#endif

    /**  Done. Close the calendar and return back to caller
    */
    if (close(fd) != 0)
    {
	_Report_error;
	return (DWC$k_db_failure);
    }

    return (DWC$k_db_normal);
}

/*
**  End of Module
*/
