/* dwc_db_calendar_maintenance.c */
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
**	This module contains the Calendar database maintenance functions,
**	such as Delete, Copy and Purge Calendar.
**
**--
*/

/*
**  Include Files
*/
#include "dwc_compat.h"

#if defined(VMS)
#include <file.h>
#include <unixio.h>
#include <string.h>
#else
#include <sys/file.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <string.h>
#endif

#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"

int DWC$DB_Rename_calendar
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block		*Cab,
	char		New_name[])
#else	/* no prototypes */
	(Cab, New_name)
	struct DWC$db_access_block		*Cab;
	char		New_name[];
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine renames an open Calendar database and maintains the
**	context for the database.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	New_name : New name of calendar file (as specific as possible)
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
**	DWC$k_db_normal	    -- Datafile renamed
**	DWC$k_db_notren	    -- Failed to rename, but context preserved
**	DWC$k_db_renfail    -- Failed to rename, context lost (e.g., calendar
**			       closed, run-down, et al)
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    char *File_name;		/* Pointer to current filename		    */
#if !defined(VMS)
    int Old_lmode;		/* Old Lock mode			    */
#else
    char new_dna[1024];		/* New dna specification		    */
#endif
        
    /*
    **  Setup error context, in case we fail.
    */
    _Set_base(DWC$_RENFAIL);

#if !defined(VMS)
    /*
    **  Release file lock
    */
    if (flock(fileno(Cab->DWC$l_dcab_fd), LOCK_UN) != 0)
    {
	_Signal(DWC$_UNLFAIL);
    }
#endif
        
    /*
    **  Close the current calendar file and zero file descriptor for
    **	recovery purposes.
    */
    if (fclose(Cab->DWC$l_dcab_fd) != 0)
    {
	_Record_error;
	_Signal(DWC$_BADCLOSE);
    }
    Cab->DWC$l_dcab_fd = NULL;

    /*
    **  Initialize pointer to the filename we're going to use
    */
    File_name = Cab->DWC$a_dcab_filename;

    /*
    **  Determine what access mode we used for the file before the rename. It
    **	is important the we continue access the result in the same manner.
    */
#if !defined(VMS)
    Old_lmode = (LOCK_EX | LOCK_NB);
    if (!(Cab->DWC$l_dcab_flags & DWC$m_dcab_write))
    {
	Old_lmode = (LOCK_SH | LOCK_NB);
    }
#endif

    /*
    **  Rename the file. Did it work?
    */
    if (rename (File_name, New_name) != 0)
    {
	/*
	**  No. Try and reopen the calendar using the old filename.
	*/
	_Record_error;
	if (Cab->DWC$l_dcab_flags & DWC$m_dcab_write)
	{
#if defined(VMS)
	    Cab->DWC$l_dcab_fd = fopen
		(File_name, "r+b", DWC$t_db_file_attributes);
#else
	    Cab->DWC$l_dcab_fd = fopen
		(File_name, "r+b");
#endif
	}
	else
	{
#if defined(VMS)
	    Cab->DWC$l_dcab_fd = fopen
		(File_name, "r", DWC$t_db_file_attributes_read);
#else
	    Cab->DWC$l_dcab_fd = fopen
		(File_name, "r");
#endif
	}
	    
	/*
	**  Did it work?
	*/
	if (Cab->DWC$l_dcab_fd != NULL)
	{

	    /*
	    **  Yes. 
	    */

#if !defined(VMS)
	    /*
	    **  On Ultrix, re-request file lock.
	    */
	    if (flock(fileno(Cab->DWC$l_dcab_fd), Old_lmode) != 0)
	    {
		_Record_error;
		_Signal(DWC$_RENFAIL);
	    }
#endif

	    /*
	    **  Indicate that the operation failed.
	    */
	    _Pop_cause;
	    return (DWC$k_db_notren);
	}
	else
	{

	    /*
	    **  The reopen did NOT work. Tell caller that the rename failed
	    **	but that we still have access to the file using the old
	    **	name.
	    */
	    _Record_error;
	    _Signal(DWC$_RENFAIL);
	}
    }

    /*
    **  The rename worked. Initialize name pointer for new calendar.
    */
    File_name = New_name;

#if defined(VMS)
    /*
    **  Build default name string for re-open
    */
    sprintf (new_dna, "dna = %s", Cab->DWC$a_dcab_filename);
#endif
    /*
    **  Try and reopen file with new name and same access mode as before.
    */
    if (Cab->DWC$l_dcab_flags & DWC$m_dcab_write)
    {
#if defined(VMS)
	Cab->DWC$l_dcab_fd = fopen
	    (File_name,  "r+b", DWC$t_db_file_attributes, new_dna);
#else
	Cab->DWC$l_dcab_fd = fopen
	    (File_name,  "r+b");
#endif
    }
    else
    {
#if defined(VMS)
	Cab->DWC$l_dcab_fd = fopen
	    (File_name,  "rb", DWC$t_db_file_attributes_read, new_dna);
#else
	Cab->DWC$l_dcab_fd = fopen
	    (File_name,  "rb");
#endif
    }

    /*
    **  Did it work?
    */
    if (Cab->DWC$l_dcab_fd != NULL)
    {
	int	nlen;
	/*
	**  Yes.
	*/
#if defined(VMS)

	/*
	**  Pick up the new complete filename
	*/
	char Complete_name[256];
	if (fgetname(Cab->DWC$l_dcab_fd, Complete_name) == 0)
	{
	    _Record_error;
	    _Signal(DWC$_RENFAIL);
	}
	File_name = Complete_name;
#else
	/*
	**  Take out a lock on the file again
	*/
	if (flock(fileno(Cab->DWC$l_dcab_fd), Old_lmode) != 0)
	{
	    _Record_error;
	    _Signal(DWC$_RENFAIL);
	}
#endif
	/*
	**  Ok. Access granted. Get rid of old filename string and get a
	**  new one.
	*/
	XtFree (Cab->DWC$a_dcab_filename);

	nlen = strlen(File_name) + 1;
	Cab->DWC$a_dcab_filename = (char *) XtMalloc (nlen);
	if (Cab->DWC$a_dcab_filename == NULL)
	{
	    _Record_error;
	    _Signal(DWC$_INSVIRMEM);
	}

	/*
	**  Save new name (for further rename).
	*/
	memcpy (Cab->DWC$a_dcab_filename, File_name, nlen);
    }
    else
    {

	/*
	**  We could not open the file. This is bad. Exit and close.
	*/
	_Record_error;
	_Signal(DWC$_RENFAIL);
    }

    /*
    **  The rename was successful. Tell caller.
    */
    _Pop_cause;
    return (DWC$k_db_normal);
}

int DWC$DB_Get_calendar_name
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	char *Calname[])
#else	/* no prototypes */
	(Cab, Calname)
	struct DWC$db_access_block	*Cab;
	char *Calname[];
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine retreives the name of the calendar file associated with
**	specified Cab. A dynamic memory string is allocated with XtMalloc and
**	the name is copied into that buffer. The caller of this routine is
**	responsible for deallocating that string.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Calname : Pointer by ref
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
**	DWC$k_db_normal	    -- Name obtained
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
    char	*Ubuf;		/* Temp pointer to calendar name string	*/
    int		nlen;
    
    /*
    **  Define error base, in case we fail
    */
    _Set_base(DWC$_GETNAMEF);

    /*
    **  Allocate some VM for the string we're about to copy. Signal if
    **	we could not get the memory we asked for.
    */
    nlen = strlen(Cab->DWC$a_dcab_filename) + 1;
    Ubuf = (char *)XtMalloc (nlen);
    if (Ubuf == 0)
    {
	_Record_error;
	_Signal(DWC$_INSVIRMEM);
	return (DWC$k_db_failure);
    }

    /*
    **  Copy name of calendar into new buffer and pass pointer back to caller
    */
    memcpy (Ubuf, Cab->DWC$a_dcab_filename, nlen);
    *Calname = Ubuf;

    /*
    **  Done. Back to caller.
    */
    _Pop_cause;
    return (DWC$k_db_normal);
}

int DWC$DB_Purge_calendar
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	int Before_day)
#else	/* no prototypes */
	(Cab, Before_day)
	struct DWC$db_access_block	*Cab;
	int Before_day;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine purges all calendar data before (but not including) a
**	given day. This includes:
**
**	    - Daymarks
**	    - Dayflags
**	    - Entries
**	    - Repeating Entries
**	
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	Before_day : Day number before which information is to be removed.
**		     Purge will go up to, but not including, this day.
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
**	DWC$k_db_normal	    -- Name obtained
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
    
    /*
    **  Define error base, in case we fail
    */
    _Set_base(DWC$_PURGE);

/*    DWC$$DB_Purge_hard_data(Cab, Before_day);
    DWC$$DB_Purge_repeats(Cab, Before_day); */
    
    /*
    **  Done. Back to caller.
    */
    _Pop_cause;
    return (DWC$k_db_normal);
}

/*
**  End of Module
*/
