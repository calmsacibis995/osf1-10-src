/* DWC_DB_PROFILE_MANAGEMENT "V3.0-007" */
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
**	This module contains the routines for Modifying and Retreiving
**	user profile data.
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**  V3.0-007	Paul Ferwerda					    13-Sep-1990
**		Change to ANSI declarations
**		
**	V2-006	Per Hamnqvist					26-Feb-1989
**		Use new return codes
**		
**	V1-005	Per Hamnqvist					06-Sep-1988
**		First shot at porting to pcc.
**		
**	V1-004	Per Hamnqvist					02-Feb-1988
**		A few functional changes, after review with Marios.
**		
**	V1-003	Per Hamnqvist					01-Feb-1988
**		Add routines for retreiving and modifying the profile
**		
**	V1-002	Per Hamnqvist					20-Jan-1988
**		A few changes to make it compile under Ultrix.
**		
**	V1-001	Per Hamnqvist					30-Nov-1987
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

int
DWC$DB_Get_profile
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	int Request_len,
	void *Profile_ptr)
#else	/* no prototypes */
	(Cab, Request_len, Profile_ptr)
	struct DWC$db_access_block *Cab;
	int Request_len;
	void *Profile_ptr;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine returns a copy of the user profile, as stored in the
**	calendar database. The caller of this routine is responsible for
**	allocation of the return buffer.
**
**	A counter in the profile keeps track of how much data space of the
**	profile that has actually been used.  If more data is requested than
**	was stored, only what was stored is returned. The user buffer beyond
**	the last byte copied from the Profile is left unaltered.  If less data
**	than is stored is requested, only the requested number of bytes are
**	returned.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	Request_len : Number of bytes to return.
**	Profile_ptr : Pointer to return byffer
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
**	DWC$k_db_normal	    -- Profile retreived
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

    VM_record *Head_vm;			/* VM header			*/
    VM_record *Profile_vm;		/* VM Profile			*/
    struct DWCDB_header *Head;		/* Real database header record	*/
    struct DWCDB_profile *Profile;	/* Real profile record		*/
    int Copy_len;			/* Number of bytes to return	*/
        
    /*
    **  Indicate what we're doing, in case we fail
    */
    _Set_base(DWC$_GETPROF);

    /*
    **  Get current profile into memory
    */
    Head_vm = Cab->DWC$a_dcab_header;
    Head = _Bind(Head_vm, DWCDB_header);
    Profile_vm = _Follow(&Head->DWC$l_dbhd_profile, DWC$k_db_profile);
    if (Profile_vm == (VM_record *)_Failure) return (DWC$k_db_failure);
    Profile = _Bind(Profile_vm, DWCDB_profile);


    /*
    **  Do not return more than was reqeusted, unless that amount of data
    **	is not present.
    */
    if (Request_len > Profile->DWC$b_dbpr_used)
	{
	Copy_len = Profile->DWC$b_dbpr_used;
	}
    else
	{
	Copy_len = Request_len;
	}

    /*
    **  Copy data from profile to return buffer and return in success
    */
    memcpy ((char *)Profile_ptr, Profile->DWC$t_dbpr_data, Copy_len);

    _Pop_cause;
    return (DWC$k_db_normal);
}

int
DWC$DB_Modify_profile
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	void *Profile_ptr,
	int Profile_len)
#else	/* no prototypes */
	(Cab, Profile_ptr, Profile_len)
	struct DWC$db_access_block *Cab;
	void *Profile_ptr;
	int Profile_len;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine replaces the user profile record in the calendar. All
**	that was there before is lost. The number of bytes requested to be
**	written is also saved in the profile.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	Profile_ptr : Pointer to new profile
**	Profile_len : Number of bytes in new profile
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
**	DWC$k_db_normal	    -- Profile modified
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

    VM_record *Head_vm;			/* VM header			*/
    VM_record *Profile_vm;		/* VM Profile			*/
    struct DWCDB_header *Head;		/* Real database header record	*/
    struct DWCDB_profile *Profile;	/* Real profile record		*/

    /*
    **  In case we fail
    */
    _Set_base(DWC$_MODPROF);

    /*
    **  Make sure caller is not requesting to store more data than we
    **	have room for. If so, signal.
    */
    if (Profile_len > DWC$k_db_max_profile_data)
	{
	_Signal(DWC$_PROFOVER);
	return (DWC$k_db_failure);
	}

    /*
    **  Bring old profile into memory (for update)
    */
    Head_vm = Cab->DWC$a_dcab_header;
    Head = _Bind(Head_vm, DWCDB_header);
    Profile_vm = _Follow(&Head->DWC$l_dbhd_profile, DWC$k_db_profile);
    if (Profile_vm == (VM_record *)_Failure) return (DWC$k_db_failure);
    Profile = _Bind(Profile_vm, DWCDB_profile);

    /*
    **  Update profile data area and save number of bytes that were
    **	actually written
    */
    memcpy (Profile->DWC$t_dbpr_data, (char *) Profile_ptr, Profile_len);
    Profile->DWC$b_dbpr_used = Profile_len;

    /*
    **  Update the database and return back to caller
    */
    if (DWC$$DB_Write_virtual_record(Cab, Profile_vm) == _Failure)
	return (DWC$k_db_failure);
    _Pop_cause;
    return (DWC$k_db_normal);
}

/*
**  End of Module
*/
