/* dwc_db_open_close.c */
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
**	This module implements the routines for opening and closing
**	the DECwindows Calendar database
**
**--
*/

/*
**  Include Files
*/
#include "dwc_compat.h"

#include <stdio.h>
#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

#include <string.h>

#if defined(VMS)
#include <file.h>
#include <unixio.h>
#include <rmsdef.h>
#else
#include <sys/file.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/uio.h>
#endif

#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"

/*
**  Min supported database version (which can be upgraded)
*/
#define DWC$k_db_min_supported 4

/*
**  Database version string for version after upgrade
*/
#define DWC$t_database_version_5 "DECwindows Calendar database; V1.0, rev #5."
#define DWC$t_database_version_6 "DECwindows Calendar database; V1.0, rev #6."
#define DWC$t_database_version_7 "DECwindows Calendar database; V1.0, rev #7."
#define DWC$t_database_version_8 "DECwindows Calendar database; V1.0, rev #8."
#define DWC$t_database_version_9 "DECwindows Calendar database; V1.0, rev #9."
#define DWC$t_database_version_10 "DECwindows Calendar database; V2.0, rev #10."
#define DWC$t_database_version_11 "DECwindows Calendar database; V3.0, rev #11."

/*
**  Macro to pass errors back to caller
*/
#define _Return_errors { \
			if (C_errno != 0) \
			    { \
			    *C_errno = Wcab->DWC$l_dcab_errno; \
			    } \
			if (Vaxc_errno != 0) \
			    { \
			    *Vaxc_errno = Wcab->DWC$l_dcab_vaxc; \
			    } \
			}

/*
**  Table of contents
*/
int DWC$$DB_First_open_check PROTOTYPE ((
    char			*Name,
    struct DWC$db_access_block	*Cab));

int DWC$$DB_Read_bitmaps PROTOTYPE ((struct DWC$db_access_block *Cab));

int DWC$$DB_Allocate_cab PROTOTYPE ((struct DWC$db_access_block **Cab));

int DWC$$DB_Upgrade_db PROTOTYPE ((struct DWC$db_access_block *Cab));


int DWC$DB_Open_calendar
#ifdef	_DWC_PROTO_
	(
	char *Name,
	struct DWC$db_access_block **Cab,
	int Force_upg,
	int *Callback,
	int User_param,
	int *C_errno,
	int *Vaxc_errno)
#else	/* no prototypes */
	(Name, Cab, Force_upg, Callback, User_param, C_errno, Vaxc_errno)
	char *Name;
	struct DWC$db_access_block **Cab;
	int Force_upg;
	int *Callback;
	int User_param;
	int *C_errno;
	int *Vaxc_errno;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine opens the DECwindows calendar database for access. The
**	routine will determine what access mode can be granted, if any. The
**	two possible modes are: Shared read or Exclusive write.
**
**  FORMAL PARAMETERS:
**
**	Name : Pointer to ASCIZ name string
**	Cab : User pointer, by ref, to receive pointer to DWC$db_access_block
**	Force_upg : Boolean; FALSE if to return _OPEN_UPGRADE if database
**		    needs to be upgraded else upgrade "on the fly".
**	Callback : Routine to be called in case of fatal error. Routine
**		   will be called:
**			Callback(Cab, User_param, C_errno, Vaxc_errno)
**	User_param : Paramter to be passed to callback routine
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
**	DWC$k_db_open_rd    -- Open for shared read
**	DWC$k_db_open_wt    -- Open for exclusive write
**	DWC$k_db_upgrade    -- This database needs to be upgraded
**	DWC$k_db_upgread    -- Could not upgrade, because no write access
**	DWC$k_db_locked	    -- File locked by another user
**	DWC$k_db_badfile    -- This is not a DWC database
**	DWC$k_db_insdisk    -- Not enough disk space for upgrade
**	DWC$k_db_insmem	    -- Could not allocate memory for Cab
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

    struct DWC$db_access_block *Wcab;	/* Work Cab		    */
    int access;				/* Granted access mode	    */
    int retstat;			/* Return status	    */
    VM_record *Head_vm;			/* Virtual header	    */
    struct DWCDB_header *Head;		/* Header for database	    */

    /*
    **  Attempt to allocate the Cab and fill it in. Return to caller if this
    **	failed
    */
    if (DWC$$DB_Allocate_cab(&Wcab) != _Success)
	{
	return (DWC$k_db_insmem);
	}

    /*
    **  Record callback routine and user parameter
    */
    Wcab->DWC$a_dcab_ecallback = Callback;
    Wcab->DWC$l_dcab_callparam = User_param;
    
    /*
    **	Open the calendar and load the bitmaps. If any of these operations
    **	fail, cleanup and exit (with a failure code)
    */
    DWC$$DB_Set_error_base(Wcab, DWC$_OPENFAIL);
    access = DWC$$DB_First_open_check(Name, Wcab);
    if ((access != DWC$k_db_open_rd) && (access != DWC$k_db_open_wt))
	{
	_Return_errors;
	return DWC$$DB_Cleanup_exit(Wcab, access);
	}
    if ((retstat = DWC$$DB_Read_bitmaps(Wcab)) != DWC$k_db_normal)
	{
	_Return_errors;
	return DWC$$DB_Cleanup_exit(Wcab, retstat);
	}

    /*
    **  Abort here if database needs to be upgraded but user did not request
    **	that it should be done right now.
    */
    Head_vm = Wcab->DWC$a_dcab_header;
    Head = _Bind(Head_vm, DWCDB_header);
    if ((Head->DWC$b_dbhd_version != DWC$k_db_version) &&
	(!(Force_upg)))
	{
	return DWC$$DB_Cleanup_exit(Wcab, DWC$k_db_upgrade);
	}
    
    /*
    **  Allocate a write buffer, if needed. Cleanup and exit if this failed.
    */
    if (Wcab->DWC$l_dcab_flags & DWC$m_dcab_write)
	{
	if ((Wcab->DWC$a_dcab_write_buff = (char *)XtMalloc(DWC$k_db_record_length)) == 0)
	    {
	    _Return_errors;
	    return DWC$$DB_Cleanup_exit(Wcab, DWC$k_db_failure);
	    }
	}

    /*
    **  Attempt to perform automatic profile upgrade (if needed)
    */
    retstat = DWC$$DB_Upgrade_db(Wcab);
    if (retstat != DWC$k_db_normal)
	{
	_Return_errors;
	return (DWC$$DB_Cleanup_exit(Wcab, retstat));
	}

    /*
    **  Load repeat expressions
    */
    if (!DWC$$DB_Load_repeats(Wcab))
	{
	_Return_errors;
	return DWC$$DB_Cleanup_exit(Wcab, DWC$k_db_failure);
	}

    /*
    **  Open worked just fine. Pass Cab pointer back to caller and return
    **	with the granted access mode indicator (read or write)
    */
    *Cab = Wcab;
    DWC$$DB_Pop_error_cause(Wcab);
    return(access);

}

int
DWC$$DB_First_open_check
#ifdef	_DWC_PROTO_
	(
	char *Name,
	struct DWC$db_access_block *Cab)
#else	/* no prototypes */
	(Name, Cab)
	char *Name;
	struct DWC$db_access_block *Cab;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine does the first open check. It opens the file, brings in the
**	header and validates it.
**
**  FORMAL PARAMETERS:
**
**	Name : Pointer to ASCIZ name string
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
**	DWC$k_db_open_rd    -- Open for shared read
**	DWC$k_db_open_wt    -- Open for exclusive write
**	DWC$k_db_upgrade    -- This database needs to be upgraded
**	DWC$k_db_locked	    -- File locked by another user
**	DWC$k_db_badfile    -- This is not a DWC database
**	DWC$k_db_nosuchfile -- File not found
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
    struct DWCDB_header	*Head;			/* Work header */
    struct DWC$db_vm_record	*Headvm;		/* Header in VM */
    int				flen;			/* Bytes in file */
    char			*File_name;		/* Current filename */
    char			Complete_name[1024];	/* Complete filespec */
    size_t			read_val;
    int				seek_val;
    int				nlen;
    
    /*
    **  Open the file and determine what access mode can be granted.
    **	This is a bit of Unix hackery. The following steps are taken:
    **
    **	    1. Attempt to open the file for read/update access. This mode
    **	       should allow both read and write on an existing file. There
    **	       is one slight problem, though: The routine will not tell me if
    **	       the fileprotection will not allow write ..
    **	    2. If Read/Update was granted, then double check that we indeed
    **	       have write access. Do so by asking for the name of the file we
    **	       just opened and then inquire if we have write access.
    **	    3. If the file could not be opened for read/update, attempt to open
    **	       it for read-only. If this fails, return failure.
    **
    **				    NOTE !!
    **
    **	It should be noted that this hackery differs on VMS and on Ultrix
    **	because the routine fgetname is only present in the VAX-C RTL under
    **	VMS. The code below therefore uses a C compiletime conditional
    **	which will produce the use of fgetname under VMS, but not under
    **	Ultrix.
    */
    File_name = (char *)Name;
#if defined(VMS)
    Cab->DWC$l_dcab_fd = fopen (File_name, "r+b", DWC$t_db_file_attributes);
#else
    Cab->DWC$l_dcab_fd = fopen (File_name, "r+b");
#endif
    if (Cab->DWC$l_dcab_fd != NULL)
    {

#if defined(VMS)
	if (fgetname(Cab->DWC$l_dcab_fd, Complete_name) == 0)
	{
	    _Record_error;
	    _Signal(DWC$_GETNAME);
	}
	File_name = Complete_name;
#else
	if ((flock(fileno(Cab->DWC$l_dcab_fd), LOCK_EX | LOCK_NB)) != 0)
	{
	    return (DWC$k_db_locked);
	}
#endif

	Cab->DWC$l_dcab_flags = (Cab->DWC$l_dcab_flags | DWC$m_dcab_write);
    }
    else
    {
#if defined(VMS)
	if (vaxc$errno == RMS$_FLK)
	{
	    return (DWC$k_db_locked);
	}
#endif

#if defined(VMS)
	Cab->DWC$l_dcab_fd = fopen
	    (File_name, "rb", DWC$t_db_file_attributes_read);
#else
	Cab->DWC$l_dcab_fd = fopen (File_name, "rb");
#endif
	if (Cab->DWC$l_dcab_fd == NULL)
	{
	    _Record_error;
	    if (errno == ENOENT)
	    {
		return (DWC$k_db_nosuchfile);
	    }
	    else
	    {
		return (DWC$k_db_failure);
	    }
	}
#if defined(VMS)
	if (fgetname(Cab->DWC$l_dcab_fd, Complete_name) == 0)
	{
	    _Record_error;
	    _Signal(DWC$_GETNAME);
	}
	File_name = Complete_name;
#else
	if ((flock(fileno(Cab->DWC$l_dcab_fd), LOCK_SH | LOCK_NB)) != 0)
	{
	    return (DWC$k_db_locked);
	}
#endif
    }

    /*
    **  File is now opened. Save filename
    */
    nlen = strlen(File_name) + 1;
    if ((Cab->DWC$a_dcab_filename = (char *)XtMalloc(nlen)) == NULL)
    {
	_Signal(DWC$_INSVIRMEM);
    }
    memcpy (Cab->DWC$a_dcab_filename, File_name, nlen);
    
    /*
    **  Find out how long the file is. Make sure it has at least minimum
    **	length and if so that the length is record aligned. If not, close
    **	the file and return error.
    */
    flen = -1;
    seek_val = fseek (Cab->DWC$l_dcab_fd, 0, SEEK_END);
    if (seek_val == 0)
    {
	flen = ftell (Cab->DWC$l_dcab_fd);
    }

    if (flen == -1)
    {
	_Record_error;
	return (DWC$k_db_failure);
    }
    if ((flen <= ((DWC$k_db_record_length*4) - 1)) ||	/* too short	    */
	((flen & (DWC$k_db_record_length - 1)) != 0))	/* not aligned	    */
	return (DWC$k_db_badfile);
    Cab->DWC$l_dcab_eof = flen; /* Save this for bitmap validation */

    /*
    **	Position to read header. Then allocate a buffer for the header. Back out
    **	if any of this failed.
    */
    seek_val = fseek (Cab->DWC$l_dcab_fd, DWC$k_db_record_length, SEEK_SET);
    if (seek_val != 0)
    {
	_Record_error;
	return (DWC$k_db_failure);
    }
		
    DWC$$DB_Get_free_buffer
	(Cab, DWC$k_db_record_length, &(Cab->DWC$a_dcab_header));
    Headvm = Cab->DWC$a_dcab_header;
    Headvm->DWC$l_dbvm_rec_addr = 1*DWC$k_db_record_length;
    Head = _Bind(Headvm, DWCDB_header);

    /*
    **  Read the header block and validate the fields in the header. If the read
    **	failed or if any of the fields are wrong, return with failure
    */
    read_val = fread (Head, DWC$k_db_record_length, 1, Cab->DWC$l_dcab_fd);

    if ((read_val != 1) || (Head->DWC$b_dbhd_blocktype != DWC$k_db_header))
	return (DWC$k_db_badfile);

#if BYTESWAP
    DWC$$DB_Byte_swap_buffer (Head, TRUE);
#endif

    if ((Head->DWC$l_dbhd_profile != (DWC$k_db_record_length * 2)) ||
	(Head->DWC$b_dbhd_version > DWC$k_db_version) ||
	(_Mbz(Head->DWC$t_dbhd_align2[0]) == -1))
	return (DWC$k_db_badfile);

    /*
    **  Check version number now. If it is at supported level then continue,
    **	indicate, to caller, that version needs to be fixed.
    */
    if (Head->DWC$b_dbhd_version != DWC$k_db_version)
	{
	if ( Head->DWC$b_dbhd_version < DWC$k_db_min_supported )
	    {
	    return (DWC$k_db_badfile);
	    }
	else
	    {
	    if (Head->DWC$b_dbhd_version > DWC$k_db_version)
		return (DWC$k_db_upgrade);
	    }
	}

    /*
    **  Ok, we got this far. That this means that at least the header is in good
    **	shape. Return to caller with in indication weather we got write access
    **	or not.
    */
    if (Cab->DWC$l_dcab_flags & DWC$m_dcab_write)
	{
	return (DWC$k_db_open_wt);
	}
    else
	{
	return (DWC$k_db_open_rd);
	}

}

int
DWC$$DB_Read_bitmaps
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
**	This routine brings in the record allocation bitmaps for the calendar
**	database. As the bitmaps are brought in, they are validated. The true
**	end of file is computed (and verified). The bitmaps are placed in
**	backwards order in a doubly linked list off the Cab
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
**	DWC$k_db_normal	    -- Read Ok
**	DWC$k_db_badfile    -- This is not a DWC database
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
    
    int bitmaps;			    /* Expected number of bitmaps   */
    int lasteob;			    /* Expected last end-of-bitmap  */
    int i;				    /* Loop count when reading	    */
    int recpos;				    /* Byte pointer to work bitmap  */
    struct DWC$db_vm_record *Bitvm;	    /* Virtual bitmap header	    */
    struct DWCDB_bitmap *Bitm;		    /* Work bitmap		    */
    size_t  read_val;
    int	    seek_val;
        
    /*
    **  Compute the number of bitmaps there ought to be in this file. Do
    **	also compute what the end of bitmap ought to be for the last bitmap.
    */
    bitmaps = Cab->DWC$l_dcab_eof / DWC$k_db_record_length;
    lasteob = (bitmaps - 1) % DWC$k_db_bitmap_entries;
    bitmaps = (bitmaps + DWC$k_db_bitmap_entries - 1) / DWC$k_db_bitmap_entries;
   
    /*
    **  Time to bring in the bitmaps
    */
    for (i=0; i<bitmaps; i++)
    {

	/*
	**  Compute point where record is supposed to be. Request the
	**  file system to position. Then, allocate a buffer for the record.
	**  If any of these operations failed, return failure
	*/
	recpos = ( i * DWC$k_db_bitmap_entries ) * DWC$k_db_record_length;
	seek_val = fseek(Cab->DWC$l_dcab_fd, recpos, SEEK_SET);
	if (seek_val != 0)
	{
	    return (DWC$k_db_badfile);
	}
	DWC$$DB_Get_free_buffer(Cab, DWC$k_db_record_length, &Bitvm);

	/*
	**  Ok. Insert this buffer at the head of the queue of bitmaps. This
	**  allows us to both find the bitmaps in memory (easily) and to
	**  cleanup in case something goes wrong. 
	*/
	DWC$$DB_Insque
	(
	    (struct DWC$db_queue_head *) &(Bitvm->DWC$a_dbvm_vm_flink),
	    (struct DWC$db_queue_head *) &(Cab->DWC$a_dcab_bitmap_flink)
	);

	/*
	**  Bring in the bitmap and check blocktype. Make sure the end of bitmap
	**  pointer does not point beoynd the bitmap. Also, if this is the last
	**  bitmap, make sure that the end of bitmap does not point beyond the
	**  end of file. If we discover any error, return with failure
	**  Save the physical address of the record that will be read
	*/
	Bitvm->DWC$l_dbvm_rec_addr = recpos;
	Bitm = _Bind(Bitvm, DWCDB_bitmap);
	read_val = fread (Bitm, DWC$k_db_record_length, 1, Cab->DWC$l_dcab_fd);
#if BYTESWAP
	DWC$$DB_Byte_swap_buffer (Bitm, TRUE);
#endif
	if ((read_val != 1) ||
	    (Bitm->DWC$b_dbbi_blocktype != DWC$k_db_bitmap) ||
	    (Bitm->DWC$w_dbbi_eob > DWC$k_db_bitmap_entries))
	{
	    return (DWC$k_db_badfile);
	}
    }

    /*
    **  Issue a warning message here. This is something we need to work on.
    */
    if (Bitm->DWC$w_dbbi_eob != lasteob)
	_Dbgprintf ("last bitmap EOB misaligned; continuing ...");
	
    /*
    **  Done, back to caller
    */
    return (DWC$k_db_normal);

}	

int DWC$$DB_Allocate_cab
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block **Cab)
#else	/* no prototypes */
	(Cab)
	struct DWC$db_access_block **Cab;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine allocates and initializes the Cab.
**
**  FORMAL PARAMETERS:
**
**	Cab : User pointer, by ref, to receive pointer to DWC$db_access_block
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
**	 0 -- No problem
**	-1 -- Failed to allocate memory
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

{
    struct DWC$db_access_block *NCab;	/* New access block	    */
    

    /*
    **  Try to allocate the memory. Return failure is this failed. Zero
    **	the memory segment allocated
    */
    NCab = (struct DWC$db_access_block *)XtCalloc
	(1, sizeof(struct DWC$db_access_block));
    if (NCab == NULL) return (_Failure);

    /*
    **  Initialize the various absolute queue heads in the Cab.
    */
    NCab->DWC$a_dcab_lru_flink =
	(VM_record *)&(NCab->DWC$a_dcab_lru_flink);
    NCab->DWC$a_dcab_lru_blink =
	(VM_record *)&(NCab->DWC$a_dcab_lru_flink);
    NCab->DWC$a_dcab_range_flink =
	(VM_record *)&(NCab->DWC$a_dcab_range_flink);
    NCab->DWC$a_dcab_range_blink =
	(VM_record *)&(NCab->DWC$a_dcab_range_flink);
    NCab->DWC$a_dcab_free_flink =
	(VM_record *)&(NCab->DWC$a_dcab_free_flink);
    NCab->DWC$a_dcab_free_blink =
	(VM_record *)&(NCab->DWC$a_dcab_free_flink);
    NCab->DWC$a_dcab_bitmap_flink =
	(VM_record *)&(NCab->DWC$a_dcab_bitmap_flink);
    NCab->DWC$a_dcab_bitmap_blink =
	(VM_record *)&(NCab->DWC$a_dcab_bitmap_flink);
    NCab->DWC$a_dcab_alarm_flink =
	(struct DWC$db_alarm *)&(NCab->DWC$a_dcab_alarm_flink);
    NCab->DWC$a_dcab_alarm_blink =
	(struct DWC$db_alarm *)&(NCab->DWC$a_dcab_alarm_flink);
    NCab->DWC$a_dcab_free_alarm_f =
	(struct DWC$db_alarm *)&(NCab->DWC$a_dcab_free_alarm_f);
    NCab->DWC$a_dcab_free_alarm_b =
	(struct DWC$db_alarm *)&(NCab->DWC$a_dcab_free_alarm_f);
    NCab->DWC$a_dcab_repeat_ctl_flink =
	(struct DWC$db_repeat_control *)&(NCab->DWC$a_dcab_repeat_ctl_flink);
    NCab->DWC$a_dcab_repeat_ctl_blink =
	(struct DWC$db_repeat_control *)&(NCab->DWC$a_dcab_repeat_ctl_flink);
    NCab->DWC$a_dcab_repeat_vec_flink =
	(VM_record *)&(NCab->DWC$a_dcab_repeat_vec_flink);
    NCab->DWC$a_dcab_repeat_vec_blink =
	(VM_record *)&(NCab->DWC$a_dcab_repeat_vec_flink);
        
    /*
    **  Pass pointer to Cab back to caller and return with success
    */
    *Cab = NCab;
    return (_Success);
}

int
DWC$$DB_Upgrade_db
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
**	This routine performs an automatic database upgrade. The fixup happens
**	without the user noticing it. The update is implemented such that
**	it can fail half way without distroying the database. The upgrade will
**	not work if read-only access is granted.
**
**	Currently, the upgrade handles:
**
**	    #4 --> #5	- Profile fixes
**	    #5 --> #6	- Repeat vector misalignment
**	    #6 --> #7	- Repeat vector reshuffling
**	    #7 --> #8	- DWCDB_extension shuffling for repeat expr:s
**	    #8 --> #9	- DWCDB_day+DWCDB_item shuffling
**	    #9 --> #10	- No real upgrade. Just to make sure that rev #9 code
**			  will not attempt to decode rev #10 repeats
**
**			   !!!!!! VERY IMPORTANT !!!!!!
**
**		AS THIS MODULE DEPENDS ON DATA DEFINITIONS FROM A COMMON
**		INCLUDE FILE, ALL PARTS OF THE UPGRADE WILL, BY DEFAULT, USE
**		THE COMMON DEFINITION. IF A DATA STRUCTURE IS CHANGED MORE THAN
**		ONCE (OR A CONSTANT) THE PREVIOUS ONE MUST BE "FROZEN" IN THE
**		APPROPRIATE UPGRADE PROCEDURE.
**
**		FOR EXAMPLE, THE REPEAT STRUCTURES HAVE CHANGED TWO TIMES SO
**		FAR. THE FIRST UPGRADE DID NOT KNOW THAT THE SECOND ONE WAS
**		GOING TO HAPPEN SO IT PREVIOUSLY USED THE COMMON DEFINITION FOR
**		THE REPEAT STRUCTURES. BUT, AFTER THE SECOND CHANGE WAS MADE
**		THE COMMON DEFINITION THAT THE FIRST UPGRADE REFERENCED HAD
**		CHANGED. THIS MUST BE AVOIDED.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block
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
**	DWC$k_db_upgread    -- Could not upgrade, because no write access
**	DWC$k_db_insdisk    -- Not enough disk space for upgrade
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

    VM_record *Head_vm;		    /* Virtual header			*/
    struct DWCDB_header *Head;	    /* Header for database		*/

    /*
    **  No need to upgrade if the version is up to rev
    */
    Head_vm = Cab->DWC$a_dcab_header;
    Head = _Bind(Head_vm, DWCDB_header);
    if (Head->DWC$b_dbhd_version == DWC$k_db_version)
    {
	return (DWC$k_db_normal);
    }

    /*
    **  If we do not have write access to take care of this, tell caller.
    */
    if (!(Cab->DWC$l_dcab_flags & DWC$m_dcab_write))
    {
	return (DWC$k_db_upgread);
    }
	
    /*
    **  Perform #4 to #5 upgrade, if needed.
    */
    if (Head->DWC$b_dbhd_version == 4)
    {
	struct DWC$db_profile_4
	{
	    unsigned char DWC$b_dbpr_blocktype;
	    unsigned char DWC$b_dbpr_used;
	    char DWC$t_dbpr_data [254];
	};

	VM_record *Profile_vm;		/* Virtual profile record	*/
	struct DWC$db_profile_4 *Profile; /* Real profile record	*/
	int i;				/* Loop variable		*/
	
	/*
	**  Get the profile
	*/
#if BYTESWAP
	Profile_vm = _Follow(&Head->DWC$l_dbhd_profile, 0);
#else
	Profile_vm = _Follow(&Head->DWC$l_dbhd_profile, DWC$k_db_profile);
#endif
	if (Profile_vm == (VM_record *)_Failure)
	{
	    return (DWC$k_db_failure);
	}
	Profile = _Bind(Profile_vm, DWC$db_profile_4);
    
	/*
	**  Shuffle fields in profile to match rev #5. Please note that this
	**  has to be done "from behind" since we are just moving the segment
	**  a few bytes forward.
	*/
	if (Profile->DWC$b_dbpr_used != 0)
	{
	    for (i=51; i>=43; i--)
	    {
		Profile->DWC$t_dbpr_data[i+1] = Profile->DWC$t_dbpr_data[i];
	    }
	    Profile->DWC$t_dbpr_data[43] = 0;
	    Profile->DWC$t_dbpr_data[53] = 0;
	    Profile->DWC$b_dbpr_used = 54;
	    
	    /*
	    **  Update the profile record on disk
	    */
	    if (DWC$$DB_Write_virtual_record(Cab, Profile_vm) == _Failure)
	    {
		return (DWC$k_db_failure);
	    }
	}
		
	/*
	**  Update ident field and do also update the textual "rev" string in
	**  the header to reflect rev #5.
	*/
	Head->DWC$b_dbhd_version = 5;
	strcpy(Head->DWC$t_dbhd_ident, DWC$t_database_version_5);

	/*
	**  Write out header and flush buffers
	*/
	if (DWC$$DB_Write_virtual_record(Cab, Head_vm) == _Failure)
	{
	    return (DWC$k_db_failure);
	}
	_Write_back(Cab);
    }

    /*
    **  Perform #5 to #6 upgrade
    */
    if (Head->DWC$b_dbhd_version == 5)
    {

	struct _old_vect
	{
	    unsigned char DWC$b_dbzz_blocktype;
	    char DWC$t_dbzz_data [243];
	    unsigned long int DWC$l_dbzz_flink;
	} *Next_vec;		    /* Next input vector	*/

#define DWC$k_db_repeats_per_vector_5 9
#define DWC$k_db_repeat_expr_len_5 27

	struct DWC$db_repeat_vector_5
	{
	    unsigned char DWC$b_dbrv_blocktype;
	    char DWC$t_dbrv_data [243];
	    char DWC$t_dbrv_padd [8];
	    unsigned long int DWC$l_dbrv_flink;
	};

	struct DWC$db_repeat_expr_5
	{
	    unsigned long int DWC$l_dbre_baseday;
	    unsigned long int DWC$l_dbre_endday;
	    unsigned short int DWC$w_dbre_basemin;
	    unsigned short int DWC$w_dbre_endmin;
	    unsigned long int DWC$l_dbre_repeat_interval;
	    unsigned long int DWC$l_dbre_repeat_interval2;
	    unsigned long int DWC$l_dbre_exceptions;
	    unsigned short int DWC$w_dbre_duration;
	    union
	    {
		unsigned char DWC$b_dbre_flags;
		struct
		{
		    unsigned DWC$v_dbre_used : 1;
		    unsigned DWC$v_dbre_open_ended : 1;
		    unsigned DWC$v_dbre_logical : 1;
		    unsigned DWC$V_fill_2 : 5;
		} DWC$r_dbre_flag_bits;
	    } DWC$r_dbre_flags_overlay;
	} ;

	int Next_repeat_vector;		    /* Pointer to next repeat	*/
	int First_vector;		    /* First vector block	*/
	VM_record *Next_vec_vm;		    /* Next virtual record	*/

	/*
	**  We only need to do this upgrade if there are any repeat
	**  expressions defined.
	*/
	Next_repeat_vector = 0;
	First_vector = 0;
	if (Head->DWC$l_dbhd_repeat_head != 0)
	{
	    
	    int Prev_vector;			    /* Previous vector	    */
						    /* block */
	    int New_rec_addr;			    /* Addr of next record  */
	    struct DWC$db_repeat_vector_5 Work_vec; /* Work block for write */
	    char *Next_exp;			    /* Next input expression*/
	    struct DWC$db_repeat_expr_5 *Next_out_entr; /* Next output expr */
	    int i;				    /* Loop variable	    */


	    /*
	    **  Make a first pass of the conversion. This conversion may
	    **	fail if we do not have enough disk space (or quota).
	    */
	    Next_repeat_vector = Head->DWC$l_dbhd_repeat_head;
	    New_rec_addr = 0;
	    Prev_vector = 0;
	    while (Next_repeat_vector != 0)
	    {
		Next_vec_vm = DWC$$DB_Read_physical_record
#if BYTESWAP
		    (Cab, Next_repeat_vector, 1, 0);
#else
		    (Cab, Next_repeat_vector, 1, DWC$k_db_repeat_expr_vec);
#endif
		Prev_vector = New_rec_addr;
		New_rec_addr = DWC$$DB_Alloc_records(Cab, 1);
		if (New_rec_addr == 0)
		{
		    return (DWC$k_db_insdisk);
		}
		if (First_vector == 0)
		{
		    First_vector = New_rec_addr;
		}
		else
		{
		    Work_vec.DWC$l_dbrv_flink = New_rec_addr;
		    DWC$$DB_Write_physical_record
			(Cab, Prev_vector, 1, (char *)&Work_vec);
		}
		memset(&Work_vec, 0, DWC$k_db_record_length);
		Work_vec.DWC$b_dbrv_blocktype = DWC$k_db_repeat_expr_vec;
		Next_vec = (struct _old_vect *)Next_vec_vm->DWC$t_dbvm_data;

#if defined(VMS)

		memcpy
		(
		    Work_vec.DWC$t_dbrv_data,
		    Next_vec->DWC$t_dbzz_data,
		    sizeof(Next_vec->DWC$t_dbzz_data)
		);

#else

		Next_exp = Next_vec->DWC$t_dbzz_data;
		Next_out_entr = (struct DWC$db_repeat_expr_5 *)Work_vec.DWC$t_dbrv_data;
		for (i=0; i<(DWC$k_db_repeats_per_vector_5 - 1); i++)
		{
		    memcpy(Next_out_entr, Next_exp, DWC$k_db_repeat_expr_len_5);
		    Next_out_entr = (struct DWC$db_repeat_expr_5 *)
				((char *)Next_out_entr +
				DWC$k_db_repeat_expr_len_5);
		    Next_exp = (char *)(Next_exp + 28);
		}

#endif

		Next_repeat_vector = Next_vec->DWC$l_dbzz_flink;
		DWC$$DB_Freelist_buffer(Cab, Next_vec_vm);
	    }

	    if (New_rec_addr != 0)
	    {
		DWC$$DB_Write_physical_record
		    (Cab, New_rec_addr, 1, (char *)&Work_vec);
	    }
	    Next_repeat_vector = Head->DWC$l_dbhd_repeat_head;
	}
	    
	/*
	**  Flush buffers, in case we fail here
	*/
	_Write_back(Cab);
	
	/*
	**  Update ident field and do also update the textual "rev" string in
	**  the header to reflect rev #6.
	*/
	Head->DWC$b_dbhd_version = 6;
	strcpy(Head->DWC$t_dbhd_ident, DWC$t_database_version_6);

	/*
	**  Add creator database O/S
	*/
	Head->DWC$b_dbhd_creator = DWC$k_db_creator;
	strcpy(Head->DWC$t_dbhd_creator, DWC$t_database_creator);
	
	/*
	**  Zero some MBZ fields in the header, in case they are non-zero
	*/
	Head->DWC$l_dbhd_fill1 = 0;
	Head->DWC$l_dbhd_fill2 = 0;

	/*
	**  Set new forward repeat expression link
	*/
	Head->DWC$l_dbhd_repeat_head = First_vector;
	
	/*
	**  Write out header and flush buffers
	*/
	if (DWC$$DB_Write_virtual_record(Cab, Head_vm) == _Failure)
	{
	    return (DWC$k_db_failure);
	}
	_Write_back(Cab);
	
	/*
	**  We should deallocate all the old repeat expression vectors. No big
	**  deal if this fails, since the data being deleted is no longer
	**  referenced.
	*/
	while (Next_repeat_vector != 0)
	{
	    Next_vec_vm = DWC$$DB_Read_physical_record
#if BYTESWAP
		(Cab, Next_repeat_vector, 1, 0);
#else
		(Cab, Next_repeat_vector, 1, DWC$k_db_repeat_expr_vec);
#endif
	    Next_vec = (struct _old_vect *)Next_vec_vm->DWC$t_dbvm_data;
	    Next_repeat_vector = Next_vec->DWC$l_dbzz_flink;
	    DWC$$DB_Deallocate_records
		(Cab, 1, Next_vec_vm->DWC$l_dbvm_rec_addr);
	    DWC$$DB_Freelist_buffer(Cab, Next_vec_vm);
	}
    }

    /*
    **  Perform #6 to #7 upgrade
    */
    if (Head->DWC$b_dbhd_version == 6)
	{

	struct local_next {
	    unsigned char DWC$b_dbzz_blocktype;
	    char DWC$t_dbzz_data [243];
	    char DWC$t_dbzz_padd [8];
	    unsigned long int DWC$l_dbzz_flink;
	    } *Next_vec;		    /* Next input vector	*/

	struct _old_repeat_expr {
	    unsigned long int DWC$l_xxre_baseday;
	    unsigned long int DWC$l_xxre_endday;
	    unsigned short int DWC$w_xxre_basemin;
	    unsigned short int DWC$w_xxre_endmin;
	    unsigned long int DWC$l_xxre_repeat_interval;
	    unsigned long int DWC$l_xxre_repeat_interval2;
	    unsigned long int DWC$l_xxre_exceptions;
	    unsigned short int DWC$w_xxre_duration;
	    unsigned char DWC$b_xxre_flags;
	    } *Next_exp;
	    
	int Next_repeat_vector;		    /* Pointer to next repeat	*/
	int First_vector;		    /* First vector block	*/
	VM_record *Next_vec_vm;		    /* Next virtual record	*/

	/*
	**  We only need to do this upgrade if there are any repeat
	**  expressions defined.
	*/
	Next_repeat_vector = 0;
	First_vector = 0;
	if (Head->DWC$l_dbhd_repeat_head != 0)
	    {
	    
	    int Prev_vector;			    /* Previous vector	    */
						    /* block */
	    int New_rec_addr;			    /* Addr of next record  */
	    struct DWCDB_repeat_vector Work_vec;   /* Work block for write */
	    struct DWCDB_repeat_expr *Next_out_entr; /* Next output expr   */
	    int i;				    /* Loop variable	    */
	    int free_slots;			    /* Free slots in output */


	    /*
	    **  Make a first pass of the conversion. This conversion may
	    **	fail if we do not have enough disk space (or quota).
	    */
	    Next_repeat_vector = Head->DWC$l_dbhd_repeat_head;
	    New_rec_addr = 0;
	    Prev_vector = 0;
	    free_slots = 0;
	    while (Next_repeat_vector != 0)
		{
		Next_vec_vm = DWC$$DB_Read_physical_record
#if BYTESWAP
		    (Cab, Next_repeat_vector, 1, 0);
#else
		    (Cab, Next_repeat_vector, 1, DWC$k_db_repeat_expr_vec);
#endif
		Next_vec = (struct local_next *)&(Next_vec_vm->DWC$t_dbvm_data[0]);
		Next_exp = (struct _old_repeat_expr *)Next_vec->DWC$t_dbzz_data;
		for (i=0; i<9; i++)
		    {
		    if (Next_exp->DWC$b_xxre_flags & 1) /* Used? */
			{
			if (free_slots == 0)
			    {
			    Prev_vector = New_rec_addr;
			    New_rec_addr = DWC$$DB_Alloc_records(Cab, 1);
			    if (New_rec_addr == 0)
				{
				return (DWC$k_db_insdisk);
				}
			    if (First_vector == 0)
				{
				First_vector = New_rec_addr;
				}
			    else
				{
				Work_vec.DWC$l_dbrv_flink = New_rec_addr;
				DWC$$DB_Write_physical_record
				    (Cab, Prev_vector, 1, (char *)&Work_vec);
				}
			    memset(&Work_vec, 0, DWC$k_db_record_length);
			    Work_vec.DWC$b_dbrv_blocktype = DWC$k_db_repeat_expr_vec;
			    Work_vec.DWC$w_dbrv_size = 1;
			    free_slots = DWC$k_db_repeats_per_vector;
			    Next_out_entr = (struct DWCDB_repeat_expr *)Work_vec.DWC$t_dbrv_data;
			    }
			memcpy(Next_out_entr, Next_exp, DWC$k_db_repeat_expr_len);
			Next_out_entr->DWC$b_dbre_reptype = DWC$k_db_absolute;
			Next_out_entr = (struct DWCDB_repeat_expr *)
				((char *)Next_out_entr +
				DWC$k_db_repeat_expr_len);
			free_slots--;
			}
		    Next_exp = (struct _old_repeat_expr *)((char *)Next_exp + 27);
		    }

		Next_repeat_vector = Next_vec->DWC$l_dbzz_flink;
		DWC$$DB_Freelist_buffer(Cab, Next_vec_vm);
		}

	    if (New_rec_addr != 0)
		{
		DWC$$DB_Write_physical_record
		    (Cab, New_rec_addr, 1, (char *)&Work_vec);
		}
	    Next_repeat_vector = Head->DWC$l_dbhd_repeat_head;
	    }
	    
	/*
	**  Flush buffers, in case we fail here
	*/
	_Write_back(Cab);
	
	/*
	**  Update ident field and do also update the textual "rev" string in
	**  the header to reflect rev #7.
	*/
	Head->DWC$b_dbhd_version = 7;
	strcpy(Head->DWC$t_dbhd_ident, DWC$t_database_version_7);

	/*
	**  Set new forward repeat expression link
	*/
	Head->DWC$l_dbhd_repeat_head = First_vector;
	
	/*
	**  Write out header and flush buffers
	*/
	if (DWC$$DB_Write_virtual_record(Cab, Head_vm) == _Failure)
	    {
	    return (DWC$k_db_failure);
	    }
	_Write_back(Cab);
	
	/*
	**  We should deallocate all the old repeat expression vectors. No big
	**  deal if this fails, since the data being deleted is no longer
	**  referenced.
	*/
	while (Next_repeat_vector != 0)
	    {
	    Next_vec_vm = DWC$$DB_Read_physical_record
#if BYTESWAP
		(Cab, Next_repeat_vector, 1, 0);
#else
		(Cab, Next_repeat_vector, 1, DWC$k_db_repeat_expr_vec);
#endif
	    Next_vec = (struct local_next *)&(Next_vec_vm->DWC$t_dbvm_data[0]);
	    Next_repeat_vector = Next_vec->DWC$l_dbzz_flink;
	    DWC$$DB_Deallocate_records(Cab, 1,
		    Next_vec_vm->DWC$l_dbvm_rec_addr);
	    DWC$$DB_Freelist_buffer(Cab, Next_vec_vm);
	    }
	}

    /*
    **  Perform #7 to #8 upgrade
    */
    if (Head->DWC$b_dbhd_version == 7)
	{
	struct _old_extension {
	    unsigned char DWC$b_xxex_blocktype;
	    char DWC$b_xxex_sanity;
	    unsigned short int DWC$w_xxex_ext_count;
	    unsigned short int DWC$w_xxex_nextfree;
	    char DWC$t_xxex_data [1];
	    } ;
	int Next_repeat_vector;			/* Next repeat vector ptr   */
	VM_record *Next_vec_vm;			/* Next vector (VM)	    */
	struct DWCDB_repeat_vector *Next_vec;	/* Next real vector buffer  */
	struct DWCDB_repeat_expr *Next_ent;	/* Next repeat expr	    */
	int i;					/* Local loop variable	    */
	VM_record *Next_blo_vm;			/* Next repeat block (VM)   */
	struct DWCDB_repeat_block *Next_blo;	/* Next repeat block buff   */
	int Old_rec_addr;			/* Old extension addr	    */
	VM_record *Next_ext_vm;			/* Next vector (VM)	    */
	int To_move;				/* Bytes/record to move	    */
	struct DWCDB_extension *New_ext;	/* New extension record	    */
	int New_rec_addr;			/* New rec ptr for ext	    */
	int Old_ext_cnt;			/* Old count for ext	    */
	struct _old_extension *Old_ext;		/* Old extension record	    */
	

	Next_repeat_vector = Head->DWC$l_dbhd_repeat_head;
	while (Next_repeat_vector != 0)
	    {
	    Next_vec_vm = DWC$$DB_Read_physical_record
#if BYTESWAP
		(Cab, Next_repeat_vector, 1, 0);
#else
		(Cab, Next_repeat_vector, 1, DWC$k_db_repeat_expr_vec);
#endif
	    Next_vec = _Bind(Next_vec_vm, DWCDB_repeat_vector);
	    Next_ent = (struct DWCDB_repeat_expr *)(Next_vec->DWC$t_dbrv_data);
	    for (i=0; i<DWC$k_db_repeats_per_vector; i++)
		{
		if (Next_ent->DWC$b_dbre_flags & DWC$m_dbre_used)
		    {
		    Next_blo_vm = DWC$$DB_Read_physical_record
#if BYTESWAP
			(Cab, Next_ent->DWC$l_dbre_repeat_interval2, 1, 0);
#else
		    (
			Cab,
			Next_ent->DWC$l_dbre_repeat_interval2,
			1,
			DWC$k_db_repeat_expr_block
		    );
#endif
		    Next_blo = _Bind(Next_blo_vm, DWCDB_repeat_block);
		    if (!(Next_blo->DWC$b_dbrb_flags & DWC$m_dbrb_r8done))
			{
			Next_blo->DWC$b_dbrb_flags = (Next_blo->DWC$b_dbrb_flags
							| DWC$m_dbrb_r8done);
			Old_rec_addr = 0;
			if (Next_blo->DWC$l_dbrb_flink != 0)
			    {
			    Next_blo->DWC$w_dbrb_size -=2;
			    Next_ext_vm = DWC$$DB_Read_physical_record
#if BYTESWAP
			    (
				Cab,
				Next_blo->DWC$l_dbrb_flink,
				Next_blo->DWC$w_dbrb_ext_count,
				0
			    );
#else
			    (
				Cab,
				Next_blo->DWC$l_dbrb_flink,
				Next_blo->DWC$w_dbrb_ext_count,
				DWC$k_db_repeat_extension
			    );
#endif
			    To_move = Next_blo->DWC$w_dbrb_size -
					DWC$k_db_repeat_data_len;
			    Old_ext = _Bind(Next_ext_vm, _old_extension);
			    New_ext = _Bind(Next_ext_vm, DWCDB_extension);
			    memcpy(New_ext->DWC$t_dbex_data,
				    Old_ext->DWC$t_xxex_data,
				    To_move - 1);
			    New_ext->DWC$t_dbex_data[To_move - 1] =
				Next_blo->DWC$b_dbrb_ext_sanity;
			    To_move += _Field_offset(New_ext, DWC$t_dbex_data[0]);
			    To_move += (DWC$k_db_record_length - 1);
			    To_move = To_move / DWC$k_db_record_length;
			    Old_rec_addr = Next_ext_vm->DWC$l_dbvm_rec_addr;
			    New_rec_addr = DWC$$DB_Alloc_records(Cab, To_move);
			    if (New_rec_addr == 0)
				{
				return (DWC$k_db_insdisk);
				}
			    Old_ext_cnt = Next_blo->DWC$w_dbrb_ext_count;
			    Next_blo->DWC$w_dbrb_ext_count = To_move;
			    Next_blo->DWC$l_dbrb_flink = New_rec_addr;
			    DWC$$DB_Write_physical_record
				(Cab, New_rec_addr, To_move, (char *)New_ext);
			    DWC$$DB_Freelist_buffer(Cab, Next_ext_vm);
			    }
			DWC$$DB_Write_physical_record
			(
			    Cab,
			    Next_blo_vm->DWC$l_dbvm_rec_addr,
			    1,
			    (char *)Next_blo
			);
			_Write_back(Cab);
			if (Old_rec_addr != 0)
			    {
			    DWC$$DB_Deallocate_records(Cab, Old_ext_cnt,
							Old_rec_addr);
			    }
			}
		    DWC$$DB_Freelist_buffer(Cab, Next_blo_vm);
		    }
		Next_ent++;
		}			
	    Next_repeat_vector = Next_vec->DWC$l_dbrv_flink;
	    DWC$$DB_Freelist_buffer(Cab, Next_vec_vm);
	    }
	/*
	**  Update ident field and do also update the textual "rev" string in
	**  the header to reflect rev #8.
	*/
	Head->DWC$b_dbhd_version = 8;
	strcpy(Head->DWC$t_dbhd_ident, DWC$t_database_version_8);

	/*
	**  Write out header and flush buffers
	*/
	if (DWC$$DB_Write_virtual_record(Cab, Head_vm) == _Failure)
	    {
	    return (DWC$k_db_failure);
	    }
	_Write_back(Cab);
	}

    /*
    **  Perform #8 to #9 upgrade
    */
    if (Head->DWC$b_dbhd_version == 8)
	{
	int Next_rmap_ptr;			/* Pointer to next rangemap */
	VM_record *Next_rmap_vm;		/* Next virtual rangemap    */
	struct DWCDB_rangemap *Next_rmap;	/* Next rmap buffer	    */
	int i;					/* Local loop variable	    */
	VM_record *Next_smap_vm;		/* Next virtual subrangemap */
	struct DWCDB_subrangemap *Next_smap;	/* Next submap buffer	    */
	int j;					/* Local loop variable	    */
	
	Next_rmap_ptr = Head->DWC$l_dbhd_first_range;
	while (Next_rmap_ptr != 0)
	    {
	    Next_rmap_vm = DWC$$DB_Read_physical_record
#if BYTESWAP
		    (Cab, Next_rmap_ptr, 1, 0);
#else
		    (Cab, Next_rmap_ptr, 1, DWC$k_db_rangemap);
#endif
	    Next_rmap = _Bind(Next_rmap_vm, DWCDB_rangemap);
	    for (i=0; i<DWC$k_db_rangemap_entries; i++)
		{
		if (Next_rmap->DWC$l_dbra_subvec[i] != 0)
		    {
		    Next_smap_vm = DWC$$DB_Read_physical_record
#if BYTESWAP
			(Cab, Next_rmap->DWC$l_dbra_subvec[i], 1, 0);
#else
		    (
			Cab,
			Next_rmap->DWC$l_dbra_subvec[i],
			1,
			DWC$k_db_subrangemap
		    );
#endif
		    Next_smap = _Bind(Next_smap_vm, DWCDB_subrangemap);
		    for (j=0; j<DWC$k_db_submap_entries; j++)
			{
			if (Next_smap->DWC$l_dbsu_dayvec[j] != 0)
			    {
			    if (!(DWC$$DB_Adjust_daymap(Cab,
				    Next_smap->DWC$l_dbsu_dayvec[j])))
				 return (DWC$k_db_insdisk);
			    }
			}
		    DWC$$DB_Freelist_buffer(Cab, Next_smap_vm);
		    }
		}
	    Next_rmap_ptr = Next_rmap->DWC$l_dbra_flink;
	    DWC$$DB_Freelist_buffer(Cab, Next_rmap_vm);
	    }
	/*
	**  Update ident field and do also update the textual "rev" string in
	**  the header to reflect rev #9.
	*/
	Head->DWC$b_dbhd_version = 9;
	strcpy(Head->DWC$t_dbhd_ident, DWC$t_database_version_9);

	/*
	**  Write out header and flush buffers
	*/
	if (DWC$$DB_Write_virtual_record(Cab, Head_vm) == _Failure)
	    {
	    return (DWC$k_db_failure);
	    }
	_Write_back(Cab);
	}

    /*
    **  Perform #9 to #10 upgrade. This one is done to prevent that rev #9
    **	code attempts to decode rev #10 repeats. The danger is mostly in
    **	the Ultrix environment where they could be running UWS2.0/MAX, UWS2.1
    **	and UWS2.2 (pre SDC) over the same NFS system.
    */
    if (Head->DWC$b_dbhd_version == 9)
	{

	/*
	**  Update ident field and do also update the textual "rev" string in
	**  the header to reflect rev #10.
	*/
	Head->DWC$b_dbhd_version = 10;
	strcpy(Head->DWC$t_dbhd_ident, DWC$t_database_version_10);

	/*
	**  Write out header and flush buffers
	*/
	if (DWC$$DB_Write_virtual_record(Cab, Head_vm) == _Failure)
	    {
	    return (DWC$k_db_failure);
	    }
	_Write_back(Cab);
	}

    /*
    **  Perform #10 to #11 upgrade. This one is done to prevent that rev #10
    **	code attempts to decode rev #11 cstext entries. The old code simply
    **	won't work on new databases.
    */
    if (Head->DWC$b_dbhd_version == 10)
	{

	/*
	**  Update ident field and do also update the textual "rev" string in
	**  the header to reflect rev #10.
	*/
	Head->DWC$b_dbhd_version = 11;
	strcpy(Head->DWC$t_dbhd_ident, DWC$t_database_version_11);

	/*
	**  Write out header and flush buffers
	*/
	if (DWC$$DB_Write_virtual_record(Cab, Head_vm) == _Failure)
	    {
	    return (DWC$k_db_failure);
	    }
	_Write_back(Cab);
	}

    /*
    **  All done, back to caler
    */
    return (DWC$k_db_normal);
}

int
DWC$$DB_Adjust_daymap
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	int Dayptr)
#else	/* no prototypes */
	(Cab, Dayptr)
	struct DWC$db_access_block *Cab;
	int Dayptr;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine rebuilds a daymap for the new #9 format that should
**	be compatible with the PMAX.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block
**	Dayptr : Starting record of daymap
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
**	TRUE	    - Daymap is now adjusted
**	FALSE	    - Could not adjust daymap because not enough disk space
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{


    /*
    **  Old datastructures
    */
    struct _old_day {
	unsigned char DWC$b_xxda_blocktype;
	unsigned char DWC$b_xxda_flags;
	unsigned short int DWC$w_xxda_baseday;
	unsigned long int DWC$l_xxda_prev_day;
	short int DWC$w_xxda_prev_idx;
	short int DWC$w_xxda_item_idx;
	unsigned long int DWC$l_xxda_flink;
	unsigned short int DWC$w_xxda_ext_count;
	unsigned short int DWC$w_xxda_fill1;
	char DWC$b_xxda_ext_sanity;
	char DWC$t_xxda_data [235];
	} ;
    struct _old_extension {
	unsigned char DWC$b_xxex_blocktype;
	char DWC$b_xxex_sanity;
	unsigned short int DWC$w_xxex_ext_count;
	unsigned short int DWC$w_xxex_nextfree;
	char DWC$t_xxex_data [1];
	} ;
    struct _old_item {
	unsigned char DWC$b_xxit_blocktype;
	unsigned char DWC$b_xxit_specific;
	short int DWC$w_xxit_id;
	unsigned short int DWC$w_xxit_size;
	char DWC$t_xxit_data [1];
	} ;
    struct _old_entry {
	unsigned char DWC$b_xxen_blocktype;
	unsigned char DWC$b_xxen_flags;
	short int DWC$w_xxen_entry_id;
	unsigned short int DWC$w_xxen_size;
	unsigned short int DWC$w_xxen_start_minute;
	unsigned long int DWC$l_xxen_delta_days;
	unsigned short int DWC$w_xxen_delta_minutes;
	unsigned short int DWC$w_xxen_text_class;
	char DWC$t_xxen_data [1];
	} ;

    /*
    **  Variable declarations
    */
    VM_record Work_queue;	    /* Used as queue head and for size calc */
    VM_record *Daymap_vm;	    /* Virtual daymap for day to fix	    */
    struct DWCDB_day *Daymap_new;  /* Daymap (new format)		    */
    struct _old_day *Daymap_old;    /* Daymap (old format)		    */
    VM_record *Old_ext_vm;	    /* Old virtual extension buffer	    */
    char *Input_ptr;		    /* Input convert pointer		    */
    struct _old_item *Old_item;	    /* Old item record			    */
    struct _old_entry *Old_entry;   /* Old entry record			    */
    struct DWCDB_entry *New_entry; /* New entry record			    */
    int Max_data;		    /* Max data left in daymap		    */
    int Old_item_size;		    /* Size of old item (in bytes)	    */
    VM_record *Old_item_vm;	    /* Virtual old item			    */
    char *Output_ptr;		    /* Output work pointer		    */
    struct _old_extension *Old_ext; /* Old extension buffer		    */
    int New_item_size;		    /* Size of new item (in bytes)	    */
    VM_record *New_item_vm;	    /* New virtual item buffer		    */
    struct DWCDB_item *New_item;   /* New item buffer			    */
    int Needed;			    /* Number of bytes/records needed	    */
    struct DWCDB_extension *New_ext; /* New daymap extension		    */
    VM_record *New_ext_vm;	    /* Virtual buffer for new extension	    */
    int Old_ext_cnt;		    /* Previous number of extension recs    */
    int Old_ext_ptr;		    /* Start of previous extension bucket   */
    int New_ext_ptr;		    /* New extension pointer		    */


    /*
    **  Get started
    */
#if BYTESWAP
    Daymap_vm = DWC$$DB_Read_physical_record(Cab, Dayptr, 1, 0);
#else
    Daymap_vm = DWC$$DB_Read_physical_record(Cab, Dayptr, 1, DWC$k_db_day_data);
#endif
    Daymap_new = _Bind(Daymap_vm, DWCDB_day);
    if (Daymap_new->DWC$b_dbda_flags & DWC$m_dbda_r9done)
	{
	DWC$$DB_Freelist_buffer(Cab, Daymap_vm);
	return (TRUE);
	}
    Daymap_old = _Bind(Daymap_vm, _old_day);

    /*
    **  Unpack items in day and build queue. The items are unpacked in the
    **	old format.
    */
    Work_queue.DWC$a_dbvm_vm_flink = (VM_record *)&(Work_queue.DWC$a_dbvm_vm_flink);
    Work_queue.DWC$a_dbvm_vm_blink = (VM_record *)&(Work_queue.DWC$a_dbvm_vm_flink);
    Old_ext_vm = 0;
    Input_ptr = (char *)(Daymap_old->DWC$t_xxda_data);
    Max_data = 235;
    Old_item = (struct _old_item *)Input_ptr;
    while (Old_item->DWC$b_xxit_blocktype != 0)
	{
	switch (Old_item->DWC$b_xxit_blocktype)
	    {
	    case DWC$k_dbit_entry:
		{
		Old_item_size = Old_item->DWC$w_xxit_size;
		DWC$$DB_Get_free_buffer_z(Cab, Old_item_size, &Old_item_vm);
		DWC$$DB_Insque
		(
		    (struct DWC$db_queue_head *) Old_item_vm,
		    (struct DWC$db_queue_head *) Work_queue.DWC$a_dbvm_vm_blink
		);
		Output_ptr = (char *)Old_item_vm->DWC$t_dbvm_data;
		if (Old_item_size >= Max_data)
		    {
		    memcpy(Output_ptr, Input_ptr, Max_data);
		    Output_ptr += Max_data;
		    Old_item_size -= Max_data;
		    Old_ext_vm = DWC$$DB_Read_physical_record
#if BYTESWAP
		    (
			Cab,
			Daymap_old->DWC$l_xxda_flink,
			Daymap_old->DWC$w_xxda_ext_count,
			0
		    );
#else
		    (
			Cab,
			Daymap_old->DWC$l_xxda_flink,
			Daymap_old->DWC$w_xxda_ext_count,
			DWC$k_db_day_extension
		    );
#endif
		    Old_ext = _Bind(Old_ext_vm, _old_extension);
		    if (Old_ext->DWC$b_xxex_sanity !=
			    Daymap_old->DWC$b_xxda_ext_sanity) return (FALSE);
		    Max_data = Daymap_old->DWC$w_xxda_ext_count *
				DWC$k_db_record_length;
		    Input_ptr = (char *)Old_ext->DWC$t_xxex_data;
		    }
		memcpy(Output_ptr, Input_ptr, Old_item_size);
		Max_data -= Old_item_size;
		Input_ptr += Old_item_size;
		break;
		}
	    case DWC$k_dbit_extension:
		{
		Old_ext_vm = DWC$$DB_Read_physical_record
#if BYTESWAP
		(
		    Cab,
		    Daymap_old->DWC$l_xxda_flink,
		    Daymap_old->DWC$w_xxda_ext_count,
		    DWC$k_db_day_extension
		);
#else
		(
		    Cab,
		    Daymap_old->DWC$l_xxda_flink,
		    Daymap_old->DWC$w_xxda_ext_count,
		    DWC$k_db_day_extension
		);
#endif
		Old_ext = _Bind(Old_ext_vm, _old_extension);
		if (Old_ext->DWC$b_xxex_sanity !=
			Daymap_old->DWC$b_xxda_ext_sanity) return (FALSE);
		Max_data = Daymap_old->DWC$w_xxda_ext_count *
			    DWC$k_db_record_length;
		Input_ptr = (char *)Old_ext->DWC$t_xxex_data;
		break;
		}
	    default :
		{
		_Signal(DWC$_UNKDAYIT);
		}
	    }
	Old_item = (struct _old_item *)Input_ptr;
	}
    if (Old_ext_vm != 0)
	{
	DWC$$DB_Freelist_buffer(Cab, Old_ext_vm);
	}

    /*
    **  Rebuild items for the new format. This is done by building up an
    **	entirely new set of items in a new queue. All the old virtual items
    **	are discarded.
    */
    Daymap_vm->DWC$a_dbvm_vm_flink = (VM_record *)&(Daymap_vm->DWC$a_dbvm_vm_flink);
    Daymap_vm->DWC$a_dbvm_vm_blink = (VM_record *)&(Daymap_vm->DWC$a_dbvm_vm_flink);
    while
    (
	DWC$$DB_Remque
	(
	    (struct DWC$db_queue_head *) &Work_queue.DWC$a_dbvm_vm_flink,
	    (struct DWC$db_queue_head **) &Old_item_vm
	)
    )
	{
	Old_entry = _Bind(Old_item_vm, _old_entry);
	New_item_size = Old_entry->DWC$w_xxen_size + 1;
	DWC$$DB_Get_free_buffer_z(Cab, New_item_size, &New_item_vm);
	DWC$$DB_Insque
	(
	    (struct DWC$db_queue_head *) New_item_vm,
	    (struct DWC$db_queue_head *) Daymap_vm->DWC$a_dbvm_vm_blink
	);
	New_entry = _Bind(New_item_vm, DWCDB_entry);
	memcpy(New_entry, Old_entry, 16);
	New_entry->DWC$w_dben_size = New_item_size;
	New_entry->DWC$b_dben_fill_1 = 0;
	memcpy(	New_entry->DWC$t_dben_data,
		Old_entry->DWC$t_xxen_data,
		Old_entry->DWC$w_xxen_size - 16);
	DWC$$DB_Freelist_buffer(Cab, Old_item_vm);
	}
		
    /*
    **  Determine the amount of extra space we need for the new way
    **	of storing items
    */
    New_item_vm = Daymap_vm->DWC$a_dbvm_vm_flink;
    Needed = 0;
    Max_data = DWC$k_db_max_day_data;
    while (New_item_vm != (VM_record *)&(Daymap_vm->DWC$a_dbvm_vm_flink))
	{
	New_item = _Bind(New_item_vm, DWCDB_item);
	if (Max_data < 17)
	    {
	    Needed += New_item->DWC$w_dbit_size;
	    Needed += (New_item->DWC$w_dbit_size & 1);
	    }
	else
	    {
	    Max_data -= New_item->DWC$w_dbit_size;
	    Max_data -= (New_item->DWC$w_dbit_size & 1);
	    if (Max_data < 0) Needed = -Max_data;
	    }
	New_item_vm = New_item_vm->DWC$a_dbvm_vm_flink;
	}

    /*
    **  Allocate diskspace for extension, if needed
    */
    if (Needed != 0)
	{
	Needed += _Field_offset(New_ext, DWC$t_dbex_data[0]);
	Needed += (DWC$k_db_record_length - 1);
	Needed = Needed / DWC$k_db_record_length;
	DWC$$DB_Get_free_buffer_z(Cab, Needed * DWC$k_db_record_length,
				&New_ext_vm);
	Old_ext_cnt = Daymap_new->DWC$w_dbda_ext_count;
	Old_ext_ptr = Daymap_new->DWC$l_dbda_flink;
	New_ext = _Bind(New_ext_vm, DWCDB_extension);
	New_ext->DWC$b_dbex_blocktype = DWC$k_db_day_extension;
	New_ext->DWC$w_dbex_ext_count = Needed;
	New_ext->DWC$b_dbex_sanity = Daymap_new->DWC$b_dbda_ext_sanity;
	Daymap_new->DWC$w_dbda_ext_count = Needed;
	New_ext_ptr = DWC$$DB_Alloc_records(Cab, Needed);
	if (New_ext_ptr == 0) return (FALSE);
	Daymap_new->DWC$l_dbda_flink = New_ext_ptr;
	}

    /*
    **  Pack new items back into daymap and new extension
    */
    memset(Daymap_new->DWC$t_dbda_data, 0, DWC$k_db_max_day_data);
    Max_data = DWC$k_db_max_day_data;
    Output_ptr = Daymap_new->DWC$t_dbda_data;
    while
    (
	DWC$$DB_Remque
	(
	    (struct DWC$db_queue_head *) &Daymap_vm->DWC$a_dbvm_vm_flink,
	    (struct DWC$db_queue_head **) &New_item_vm
	)
    )
	{
	New_item = _Bind(New_item_vm, DWCDB_item);
	New_item_size = New_item->DWC$w_dbit_size;
	Input_ptr = (char *)New_item;
	if (New_item_size > Max_data)
	    {
	    if (Max_data < 17)
		{
		*Output_ptr = DWC$k_dbit_extension;
		}
	    else
		{
		memcpy(Output_ptr, Input_ptr, Max_data);
		New_item_size -= Max_data;
		Input_ptr += Max_data;
		}
	    Output_ptr = New_ext->DWC$t_dbex_data;
	    Max_data = (Needed * DWC$k_db_record_length);
	    }
	memcpy(Output_ptr, Input_ptr, New_item_size);
	New_item_size += (New_item->DWC$w_dbit_size & 1);
	Output_ptr += New_item_size;
	Max_data -= New_item_size;
	DWC$$DB_Freelist_buffer(Cab, New_item_vm);
	}

    /*
    **  Write out extension and make sure that it is indeed on disk
    **	before continuing
    */
    if (Needed != 0)
	{
	DWC$$DB_Write_physical_record
	    (Cab, New_ext_ptr, Needed, (char *)New_ext);
	DWC$$DB_Freelist_buffer(Cab, New_ext_vm);
	_Write_back(Cab);
	}

    /*
    **  Indicate that we have performed the rev #9 upgrade on this
    **	daymap and write it out.
    */
    Daymap_new->DWC$b_dbda_flags = (Daymap_new->DWC$b_dbda_flags |
					DWC$m_dbda_r9done);
    DWC$$DB_Write_physical_record
	(Cab, Daymap_vm->DWC$l_dbvm_rec_addr, 1, (char *)Daymap_new);
    _Write_back(Cab);

    /*
    **  Get rid of old extension records (if any)
    */
    if (Needed != 0)
	{
	DWC$$DB_Deallocate_records(Cab, Old_ext_cnt, Old_ext_ptr);
	}

    /*
    **  Return the daymap vm buffer and back to caller
    */
    DWC$$DB_Freelist_buffer(Cab, Daymap_vm);
    return (TRUE);
}

int
DWC$$DB_Cleanup_exit
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	unsigned long Retsts)
#else	/* no prototypes */
	(Cab, Retsts)
	struct DWC$db_access_block *Cab;
	unsigned long Retsts;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine cleans up all virtual memory used by one database session.
**	It then closes the database file, if it was open.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	Retsts : Status code that this routine should return, if all its
**		 operations worked
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
**	 -1 -- Failed to cleanup
**	[other] -- Whatever the caller passed in Retsts
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

{    


    struct DWC$db_vm_record *Wbuf;	/* Temp pointer to work buffer  */
    
    /*
    **  Clear day context. It is possible that we are pointing at a day
    **	which contains only repeat expressions and therefore is not linked
    **	in with the rest of the VM structures.
    */
    DWC$$DB_Clear_day_context(Cab);
    
    /*
    **  Get rid of context used by Get_specific_r_item
    */
    DWC$$DB_Fry_temp_arrays(Cab);
    
    /*
    **  Purge the cache until nothing more can be purged. The reason why we do
    **	this rather than just moving all records from the cache to the free list
    **	is that some records have non-cached records hanging off them. By
    **	purging, we are also removing the "tails"
    */
    while (DWC$$DB_Purge_cache(Cab));   /* no code in while loop! */

    /*
    **  Get all the bitmaps off the list. They were not cached.
    */
    while
    (
	DWC$$DB_Remque
	(
	    (struct DWC$db_queue_head *) &(Cab->DWC$a_dcab_bitmap_flink),
	    (struct DWC$db_queue_head **) &Wbuf
	)
    )
    {
	DWC$$DB_Freelist_buffer (Cab, Wbuf);
    }


    /*
    **  Get rid of all repeat expressions
    */
    DWC$$DB_Unload_repeats(Cab);
    
    /*
    **  Do we have a header block? If so, place it on the freelist (for later
    **	delete)
    */
    if (Cab->DWC$a_dcab_header != 0)
	DWC$$DB_Freelist_buffer(Cab, Cab->DWC$a_dcab_header);

    /*
    **  All our record buffers are now on the free list. The shorter buffers
    **	have already been deleted through the Purge. Delete all the buffers
    **	hanging off the free list.
    */
    DWC$$DB_Delete_all_buffers(Cab);

    /*
    **  If this access block has a write buffer associated with it, release it.
    **	Signal an error if the deallocation failed.
    */
    if (Cab->DWC$a_dcab_write_buff != 0)
    {
	XtFree(Cab->DWC$a_dcab_write_buff);
    }

    /*
    **  Get rid of all in-memory alarms
    */
    DWC$$DB_Unload_alarms(Cab);
    
    /*
    **  If the file was ever opened, close it. Check for completion code. If
    **	completion code indicates failure, signal an error. Do also get rid of
    **	the file lock (but do not check for success).
    */
    if (Cab->DWC$l_dcab_fd != NULL)
    {

#if !defined(VMS)
	flock(fileno(Cab->DWC$l_dcab_fd), LOCK_UN);
#endif

	if (fclose(Cab->DWC$l_dcab_fd) != 0) _Signal(DWC$_BADCLOSE);
    }

    /*
    **  Get rid of filename string
    */
    if (Cab->DWC$a_dcab_filename != 0)
    {
	XtFree(Cab->DWC$a_dcab_filename);
    }

    /*
    **  Rundown error messages, in case they have been activated
    */
    DWC$$DB_Rundown_errors(Cab);
    
    /*
    **  And .. finally .. deallocate the Cab itself. Display an error
    **	message if this fails (and exit)
    */
    XtFree(Cab);

    /*
    **  Return to caller with the status indicated by caller
    */
    return (Retsts);
}    

int DWC$DB_Close_calendar
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
**	This routine update the statistics and closes the calendar. 
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	
**  COMPLETION CODES:
**
**	DWC$k_db_normal	    -- Database closed
**	DWC$k_db_failure    -- General failure; please check errno (and
**			       possibly vaxc$errno).
**
**  SIDE EFFECTS:
**
**--
**/
{

    /*
    **  Define the current error base, in case something would happen
    */
    _Set_base(DWC$_CLOSEFAIL);

    /*
    **  Call the cleanup routine and indicate that we want it to return
    **	success if all the cleanup worked fine
    */
    if (Cab->DWC$l_dcab_flags & DWC$m_dcab_write)
	{
	DWC$DB_Update_statistics (Cab);
	}

    return (DWC$$DB_Cleanup_exit(Cab, DWC$k_db_normal));

}

int
DWC$DB_Update_statistics
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block  *Cab)
#else	/* no prototypes */
	(Cab)
	struct DWC$db_access_block  *Cab;
#endif	/* prototypes */

/*
**  Update the calendar header with the statistic information
**  stored in the CAB. Write out to the disk.
*/
{
    struct DWC$db_vm_record *Header_vm;	    /* VM header		*/
    struct DWCDB_header *Head;		    /* Header itself		*/
#if 0
    int (*New_stat)[];
    int (*Old_stat)[];
#else
    int *New_stat;
    int *Old_stat;
#endif
    int	i;

    Header_vm = Cab->DWC$a_dcab_header;
    Head = _Bind(Header_vm, DWCDB_header);
    Old_stat = (int *)&(Head->DWC$t_statistics);
    New_stat = (int *)&(Cab->DWC$t_statistics);
    for (i=0; i<DWC$k_statistics_length; i++)
    {
#if 0
	(*Old_stat)[i] += (*New_stat)[i];
	(*New_stat)[i] = 0;
#else
	Old_stat[i] += New_stat[i];
	New_stat[i] = 0;
#endif
    }
    return (DWC$$DB_Write_virtual_record(Cab, Header_vm));
}

void
DWC$DB_Print_statistics
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block  *Cab)
#else	/* no prototypes */
	(Cab)
	struct DWC$db_access_block  *Cab;
#endif	/* prototypes */

/*
**  Print the calendar statistics information for session and total.
*/
{
    struct DWC$db_vm_record *Header_vm;	    /* VM header		*/
    struct DWCDB_header *Head;		    /* Header itself		*/
    static char *Name[] =
		{ "Writes", "Reads", "Cache hits", "Allocations", "Frees"};
#if 0
    int (*New_stat)[];
    int (*Old_stat)[];
#else
    int *New_stat;
    int *Old_stat;
#endif
    int	i;

    Header_vm = Cab->DWC$a_dcab_header;
    Head = _Bind(Header_vm, DWCDB_header);
    Old_stat = (int *)&(Head->DWC$t_statistics);
    New_stat = (int *)&(Cab->DWC$t_statistics);
    printf ( "\n Calendar statistics:    Total   Session\n");
    printf (   "---------------------+---------+---------+\n");
    /*         12345678901234567890  12345678  12345678		*/
    for (i=0; i<DWC$k_statistics_length; i++)
    {
#if 0
	printf ("%20s |%8d |%8d |\n", Name[i], (*Old_stat)[i], (*New_stat)[i]);
#else
	printf ("%20s |%8d |%8d |\n", Name[i], Old_stat[i], New_stat[i]);
#endif
    }
    printf ("%20s |%8d |%8d |\n",
	"Cache in use", 0, Cab->DWC$l_dcab_cache_count);
    printf (   "---------------------+---------+---------+\n\n");
    return;
}

/*
**  End of Module
*/
