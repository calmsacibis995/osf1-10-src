/* dwc_db_error_handling.c */
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
**	This module contains the error handling and signalling code used
**	by the DECwindows calendar database routines.
**
**--
*/

/*
**  Include Files
*/
#include "dwc_compat.h"

#include <stdio.h>
#include <string.h>

#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"

/*
**  External routines
*/
extern LIB$SIGNAL();	    /* Need to be declared to be passed by ref */
extern SYS$PUTMSG();

void
DWC$$DB_Set_error_base
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block		*Cab,
	unsigned long				Errcode)
#else	/* no prototypes */
	(Cab, Errcode)
	struct DWC$db_access_block		*Cab;
	unsigned long				Errcode;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine declares a starting error code for the error handling
**	package. It makes sure that the current error level is zero. If not,
**	this call is made out of phase and needs to be signalled.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	Errcode : 32bit error code
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
**
**--
**/
{

    /*
    **  Clear saved errors
    */
    Cab->DWC$l_dcab_errno = 0;
    Cab->DWC$l_dcab_vaxc = 0;

    /*
    **  Is this call "in phase"?
    */
    if (Cab->DWC$l_dcab_errlev == 0)

	/*
	**  We're in Phase. Everything is Ok. Store the error code
	**  and prepare for exit.
	*/
	{
	Cab->DWC$l_dcab_errlev = 1;
	Cab->DWC$l_dcab_errstack[0] = Errcode;
	}
    
    else

	/*
	**  We've received an out of phase call. This means that the
	**  caller has started a new operation before finnishing the
	**  previous one OR we have a bug in the database access routines.
	**  Whatever the reason we need to signal this.
	*/
	_Signal(DWC$_OUTOFPHASE);
		
}

void
DWC$$DB_Set_error_cause
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block		*Cab,
	unsigned long				Errcode)
#else	/* no prototypes */
	(Cab, Errcode)
	struct DWC$db_access_block		*Cab;
	unsigned long				Errcode;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine pushes one more error code on the preventive
**	Error stack. It also makes sure that the stack is not over
**      flowed.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	Errcode : 32bit error code
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
**
**--
**/
{

    /*
    **  Make sure our stack is not full. If so, signal it.
    */
    if (Cab->DWC$l_dcab_errlev == DWC$K_MAX_ERRORS)

	/*
	**  Yes. We're overflowed.
	*/
	{
	Cab->DWC$l_dcab_errlev = 0;	
	_Signal(DWC$_OVERFLOW);
	}

    else

	/*
	**  There is stil room. Push one more error on the stack
	*/
	{
	Cab->DWC$l_dcab_errstack[Cab->DWC$l_dcab_errlev] = Errcode;
	Cab->DWC$l_dcab_errlev++;
	}
}

void
DWC$$DB_Pop_error_cause
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block		*Cab)
#else	/* no prototypes */
	(Cab)
	struct DWC$db_access_block		*Cab;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine pops one error off the error stack. It will make
**	sure the stack does not underflow.
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
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    /*
    **  Make sure there is something on the stack. Is it?
    */
    if (Cab->DWC$l_dcab_errlev == 0)

	/*
	**  No. The stack is empty. Signal empty stack.
	*/
	{
	_Signal(DWC$_UNDERFLOW);
	}

    else

	/*
	**  Pop one error off the stack
	*/
	{
	Cab->DWC$l_dcab_errlev--;
	}
}

void
DWC$$DB_Add_error_code
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block		*Cab,
	unsigned long				Errcode,
	char *Errid,
	char *Msgptr)
#else	/* no prototypes */
	(Cab, Errcode, Errid, Msgptr)
	struct DWC$db_access_block		*Cab;
	unsigned long				Errcode;
	char *Errid;
	char *Msgptr;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine adds one error code and associated text message to
**	the linked list of error messages for the DECwindows Calendar.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	Errcode : Errorcode to be added to the list (complete with facility
**	          and severity)
**      Errid : Pointer to identification text part of the error message,
**	        such as "ACCVIO"
**	Msgptr : Pointer to textual part of error message.
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
**
**--
**/
{

    struct DWC$db_error_buffer *Wrkerr;	    /* Work buffer with error */
    
    /*
    **  Allocate new buffer
    */
    Wrkerr = (struct DWC$db_error_buffer *)XtMalloc(sizeof(*Wrkerr));
    if (Wrkerr == 0)
	{
	printf("%%DWC-F-FATERR, could not allocate memory in error reporting\n");
	exit(1);
	}

    /*
    **  Fill in fields
    */
    Wrkerr->DWC$l_dbeb_ecode = Errcode;
    Wrkerr->DWC$a_dbeb_eid = Errid;
    Wrkerr->DWC$a_dbeb_eptr = Msgptr;

    /*
    **  Link buffer into queue and return back to caller
    */
    DWC$$DB_Insque
    (
	(struct DWC$db_queue_head *) Wrkerr,
	(struct DWC$db_queue_head *) Cab->DWC$a_dcab_eflink
    );
    return;
}

void
DWC$$DB_Init_errors
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block		*Cab)
#else	/* no prototypes */
	(Cab)
	struct DWC$db_access_block		*Cab;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine will initialize the linked list of error messages
**	that the message handling of the DECwindows Calendar can handle.
**	The queue will only be initialized when the first error needs to
**	be printed, which ought to be rare.
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
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    /*
    **  Have we been initialized before? If so, just return
    */
    if (Cab->DWC$a_dcab_eflink != 0)
	{
	return;
	}

    /*
    **  No. Initialize head of queue in Cab
    */
    Cab->DWC$a_dcab_eflink = (struct DWC$db_error_buffer*)&(Cab->DWC$a_dcab_eflink);
    Cab->DWC$a_dcab_eblink = (struct DWC$db_error_buffer*)&(Cab->DWC$a_dcab_eflink);

    /*
    **  Add error codes to list. The corresponding status codes are
    **	defined in module DWC_DB_PRIVATE_INCLUDE.H
    */
    DWC$$DB_Add_error_code( Cab,
			    DWC$_NORMAL,
			    "NORMAL",
			    "normal successful completion"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_TESTOK,
			    "TESTOK",
			    "test completed in success"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_INFO,
			    "INFO",
			    "non-standard successful completion"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_WARNING,
			    "WARNING",
			    "warning"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_NOFIXUP,
			    "NOFIXUP",
			    "no VA-->record address fixup present for requested record type"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_NOWRITE,
			    "NOWRITE",
			    "not allowed to add or modify record to the database"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_RECERR,
			    "RECERR",
			    "a (semi) recoverable error has ocurred. Data may have been lost"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_CRECAL,
			    "CRECAL",
			    "error creating Calendar database"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_ALLOCREC,
			    "ALLOCREC",
			    "failed to extend calendar database"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_CREFAIL,
			    "CREFAIL",
			    "failed to create calendar file itself"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_BADCLOSE,
			    "BADCLOSE",
			    "file system refused to close file"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_PUTLBUF,
			    "PUTLBUF",
			    "failed to write virtual record"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_WRITEREC,
			    "WRITEREC",
			    "error occured while WRITING database record"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_READREC,
			    "READREC",
			    "error occured while READING database record"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_INVPTR,
			    "INVPTR",
			    "encountered invalid pointer to database record"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_INSVIRMEM,
			    "INSVIRMEM",
			    "insufficient virtual memory"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_FILEIOERR,
			    "FILEIOERR",
			    "system signalled file IO error"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_INVRTYPE,
			    "INVRTYPE",
			    "invalid or Unexpected record type encountered"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_UNKDAYIT,
			    "UNKDAYIT",
			    "unknown day Item encountered"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_DLENMM,
			    "DLENMM",
			    "day data Length Mismatch"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_UNXDIE,
			    "UNXDIE",
			    "unexpexted End of Day Item Entry"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_BADDCHK,
			    "BADDCHK",
			    "bad day checksum"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_EXTENDFAIL,
			    "EXTENDFAIL",
			    "failed to extend database"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_CREF,
			    "CREF",
			    "failed to create specified calendar file"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_OPENF,
			    "OPENF",
			    "faield to open specified calendar file"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_NOCURCAL,
			    "NOCURCAL",
			    "you have not opened a calendar yet"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_REPLOAD,
			    "REPLOAD",
			    "failed to load repeat expressions"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_UNLREPS,
			    "UNLREPS",
			    "failed to unload repeat expressions"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_NOSUCHREP,
			    "NOSUCHREP",
			    "no such repeat expression"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_BADRCHK,
			    "BADRCHK",
			    "bad checksum detected in repeat expression"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_TESTFAIL,
			    "TESTFAIL",
			    "test failed"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_OUTOFPHASE,
			    "OUTOFPHASE",
			    "previous db operation not complete. Pending errors follow:"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_OVERFLOW,
			    "OVERFLOW",
			    "error stack is FULL. This requires a quick fix and a recompile"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_UNDERFLOW,
			    "UNDERFLOW",
			    "an attempt was made to pop an empty error stack"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_WRITEERR,
			    "WRITEERR",
			    "failed to write a record even though the file is open for write"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_CLOSEFAIL,
			    "CLOSEFAIL",
			    "could not close calendar database file"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_MBZFAIL,
			    "MBZFAIL",
			    "nonzero contents found in MBZ field in a database record"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_FREEFAIL,
			    "FREEFAIL",
			    "could not deallocate virtual memory"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_OPENFAIL,
			    "OPENFAIL",
			    "could not open the calendar database"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_READONLY,
			    "READONLY",
			    "an attempt was made to write into the database without write access"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_PURGEFAIL,
			    "PURGEFAIL",
			    "unexpected failure to purge cache"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_MODPROF,
			    "MODPROF",
			    "failed to update user profile"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_PROFOVER,
			    "PROFOVER",
			    "an attempt was made to write too much data into profile"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_GETPROF,
			    "GETPROF",
			    "failed to read user profile record"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_GETIFAIL,
			    "GETIFAIL",
			    "failed to retreive day item"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_PUTIFAIL,
			    "PUTIFAIL",
			    "failed to add an item to a day"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_DELIFAIL,
			    "DELIFAIL",
			    "failed to delete an item from a day"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_DAYATAF,
			    "DAYATAF",
			    "failed to add/modify daymarks and day flags"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_DAYATGF,
			    "DAYATGF",
			    "failed to retreive day mark or day attributes"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_ALARMINCON,
			    "ALARMINCON",
			    "in-memory copy of alarm database is inconsistent"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_OCFAIL,
			    "OCFAIL",
			    "failed to determine if specified time overlaps existing item"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_ADDRF,
			    "ADDRF",
			    "failed to add new repeat expression"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_MODREPF,
			    "MODREPF",
			    "failed to modify existing repeat expression"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_DELREPF,
			    "DELREPF",
			    "failed to delete existing repeat expression"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_MODIFAIL,
			    "MODIFAIL",
			    "failed to modify item"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_RENFAIL,
			    "RENFAIL",
			    "unexpected error when renaming calendar file"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_FLUSHF,
			    "FLUSHF",
			    "unable to flush file I/O buffers -- aborting here to be safe"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_GETNAMEF,
			    "GETNAMEF",
			    "unable to retreive name of specified calendar file"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_UNLFAIL,
			    "UNLFAIL",
			    "failed to release file lock on database"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_RENFAIL,
			    "RENFAIL",
			    "failed to reopen calendar after bad rename"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_GETNAME,
			    "GETNAME",
			    "failed to retreive file name"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_PURGE,
			    "PURGE",
			    "failed to purge database"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_UNEXPREP,
			    "UNEXPREP",
			    "invalid repeat interval detected"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_DTCBAD,
			    "DTCBAD",
			    "day type cache invalid or in bad state"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_EVALREP,
			    "EVALREP",
			    "failed to examine special repeat parameters"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_LOADFIL,
			    "LOADFIL",
			    "failed to load text file into memory"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_RUNDF,
			    "RUNDF",
			    "failed to rundown interchange context"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_ICREF,
			    "ICREF",
			    "interchange create failed"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_CSTR,
			    "CSTR",
			    "don't know how to deal with compound string"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_UNSTR,
			    "UNSTR",
			    "unrecognized text class in item"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_CNCHKR,
			    "CNCHKR",
			    "could not check if repeat triggers on day"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_PUTIF,
			    "PUTIF",
			    "failed to add interchange item"
			    );
    DWC$$DB_Add_error_code( Cab,
			    DWC$_FGNALT,
			    "FGNALT",
			    "failed to obtain next alarm time"
			    );

    /*
    **  All codes have been defined, back to caller
    */
    return;
}

void
DWC$$DB_Rundown_errors
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block		*Cab)
#else	/* no prototypes */
	(Cab)
	struct DWC$db_access_block		*Cab;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine will deallocate all memory used by in memory messages,
**	in case they were activated during the session. Normally, this
**	rundown is only done if there has been a soft_signal during the
**	session. Normal signals terminate the image and needs no rundown.
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
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    struct DWC$db_error_buffer *Curr_err;   /* Pointer to current work	    */
					    /* error			    */

    /*
    **  Have we been initialized at all. If not, just return
    */
    if (Cab->DWC$a_dcab_eflink == 0)
	{
	return;
	}

    /*
    **  Yes, we have been initialized. Get rid of all the recorded status
    **	codes. If a deallocation fails, signal. Please note that the
    **	signalling still works, even though the text strings may be
    **	gone.
    */
    while (DWC$$DB_Remque
    (
	(struct DWC$db_queue_head *)&Cab->DWC$a_dcab_eflink,
	(struct DWC$db_queue_head **)&Curr_err
    ))
	{
	XtFree(Curr_err);
	}

    /*
    **  Done, back to caller
    */
    return;
}

void
DWC$DB_Get_error_strings
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block		*Cab,
	unsigned long				Errcode,
	int Cont,
	char *Outbuf,
	char *Token)
#else	/* no prototypes */
	(Cab, Errcode, Cont, Outbuf, Token)
	struct DWC$db_access_block		*Cab;
	unsigned long				Errcode;
	int Cont;
	char *Outbuf;
	char *Token;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine builds one line of an error traceback. Such a line
**	is equivalent with a normal VMS error message. The routine formats
**	the error message (but does no handling of FAO stuff). If the
**	associated error text cannot be found a NONAME message is built.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	Errcode : Errorcode to print
**	Cont : Boolean. True if this is a continuation line or the
**	       first line (determines if to start with "%" or with "-")
**	Outbuf : Pointer to char buffer (allocated by caller) to receive
**	         string. This buffer should be at least 256 bytes.
**	Token : Pointer to char buffer (allocated by caller) to receive
**		error token (for example "DWC$_OPENF")
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
**
**--
**/
{
    int Found;				    /* Error text found		*/
    struct DWC$db_error_buffer *Curr_err;   /* Error buffer for Errcode	*/
    
    /*
    **  Initialize output buffers for out string concatenations
    */
    Outbuf[0] = '\0';
    Token[0] = '\0';
    
    /*
    **  Add leading character; depends on if this is the first line of
    **	the traceback or not
    */
    if (Cont)
	{
	strcat(Outbuf, "-");
	}
    else
	{
	strcat(Outbuf, "%");
	}

    /*
    **  Next, add the facility text string.
    */
    strcat(Outbuf, "DWC-");
    strcat(Token, "DWC$_");

    /*
    **  Now, add the severity of the error
    */
    switch (_VMS_STATUS_SEVERITY(Errcode))
	{
	case STS$K_WARNING :
	    {
	    strcat(Outbuf, "W");
	    break;
	    }
	case STS$K_SUCCESS :
	    {
	    strcat(Outbuf, "S");
	    break;
	    }
	case STS$K_ERROR :
	    {
	    strcat(Outbuf, "E");
	    break;
	    }
	case STS$K_INFO :
	    {
	    strcat(Outbuf, "I");
	    break;
	    }
	case STS$K_SEVERE :
	    {
	    strcat(Outbuf, "F");
	    break;
	    }
	}	    

    /*
    **  Assume that we do not have the text for this and start looking
    **	for this error text.
    */
    Found = FALSE;
    Curr_err = (struct DWC$db_error_buffer*)&(Cab->DWC$a_dcab_eflink);
    while ( (Curr_err = Curr_err->DWC$a_dbeb_flink) != 
	    ((struct DWC$db_error_buffer*)&(Cab->DWC$a_dcab_eflink)))
	{
	if (_VMS_STATUS_COND_ID(Curr_err->DWC$l_dbeb_ecode) ==
	    _VMS_STATUS_COND_ID(Errcode))
	    {
	    Found = TRUE;
	    break;
	    }
	}

    /*
    **  Did we find the text?
    */
    strcat(Outbuf, "-");
    if (Found)
	{

	/*
	**  Yes, add Id and Text
	*/
	strcat(Outbuf, Curr_err->DWC$a_dbeb_eid);
	strcat(Outbuf, ", ");
	strcat(Outbuf, Curr_err->DWC$a_dbeb_eptr);
	strcat(Token, Curr_err->DWC$a_dbeb_eid);
	}
    else
	{

	char temp_num[20];
	
	/*
	**  No, build a generic error message
	*/
	strcat(Outbuf, "NONAME, message number %X");
	sprintf(temp_num, "'%x'", Errcode);
	strcat(Outbuf, temp_num);
	strcat(Token, "NONAME");
	}

    /*
    **  Done, back to caller
    */
    return;
}    	

void
DWC$$DB_Print_error_stack
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block		*Cab)
#else	/* no prototypes */
	(Cab)
	struct DWC$db_access_block		*Cab;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine prints a DECwindows Calendar error stack.
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
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    int loop;				/* General loop variable    */
    char token[20];			/* Error token		    */
    char eline[256];			/* Error line		    */
    

    /*
    **  Print first line of message, which should start with "%"
    */
    DWC$DB_Get_error_strings(Cab, Cab->DWC$l_dcab_errstack[0],
				FALSE, eline, token);
    printf("%s\n", eline);

    /*
    **  Print the rest of the errors, all starting with "-"
    */
    for (loop = 1; loop < Cab->DWC$l_dcab_errlev; loop++)
	{
	DWC$DB_Get_error_strings(Cab, Cab->DWC$l_dcab_errstack[loop],
				    TRUE, eline, token);
	printf("%s\n", eline);
	}

    /*
    **  Done, back to caller
    */
    return;
}

void
DWC$$DB_Signal_error
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block		*Cab,
	unsigned long				Errcode)
#else	/* no prototypes */
	(Cab, Errcode)
	struct DWC$db_access_block		*Cab;
	unsigned long				Errcode;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine signals the stack of errors built up by calls to
**	DWC$$DB_Set_error_base and DWC$$DB_Set_error_cause. Special
**	care is taken of DWC$_OUTOFPHASE.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	Errcode : 32bit error code
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
**
**--
**/
{

    int loop;
    int top_stack;

    /*
    **  Initialize error handling, if we have not done it already
    */
    DWC$$DB_Init_errors(Cab);

    /*
    **  Is this normal case of signal or it an out of phase error?
    */
    if (Errcode == DWC$_OUTOFPHASE)

	/*
	**  This is an out of phase error. Build a new callback stack,
	**  which includes the OUTOFPHASE error message
	*/
	{
	
	top_stack = Cab->DWC$l_dcab_errlev + 1;
	if (top_stack > DWC$K_MAX_ERRORS) top_stack--;
	for (loop = top_stack; loop!=0; loop--)
	    Cab->DWC$l_dcab_errstack[loop] =
		Cab->DWC$l_dcab_errstack[loop - 1];
	Cab->DWC$l_dcab_errstack[0] = DWC$_OUTOFPHASE;
	Cab->DWC$l_dcab_errlev = top_stack;				
	
	}
    else

	/*
	**  This is not a special case. Append the last error to the error
	**  stack
	*/
	{
	Cab->DWC$l_dcab_errstack[Cab->DWC$l_dcab_errlev] = Errcode;
	Cab->DWC$l_dcab_errlev++;
	}

    /*
    **  If user callback routine is present, call it else use our own
    **	dumper routine
    */
    if (Cab->DWC$a_dcab_ecallback != 0)
	{
	typedef int dwc_callback();
	dwc_callback *cproc;

	cproc = (dwc_callback *) Cab->DWC$a_dcab_ecallback;
	(*cproc)(Cab, Cab->DWC$l_dcab_callparam,
			Cab->DWC$l_dcab_errno,
			Cab->DWC$l_dcab_vaxc);
	}
    else
	{
	DWC$$DB_Print_error_stack(Cab);
	}
	
    /*
    **  Abort now
    */
    exit(1);
    return;
}

int
DWC$DB_Get_error_codes
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block *Cab,
	int *c_errno,
	int *vaxc_errno)
#else	/* no prototypes */
	(Cab, c_errno, vaxc_errno)
	struct DWC$db_access_block *Cab;
	int *c_errno;
	int *vaxc_errno;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine retreives the most recently recorded system errors.
**	This routine can be called to obtain additional information about
**	a soft error and it can also be used by the catch_all handler when
**	displaying information just before run-down.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	c_errno : integer (by ref) to receive "errno" at time
**	          of error. A value of 0 indicates no error.
**	vaxc_errno : integer (by ref) to receive "vaxc$errno" at time
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
**	DWC$k_db_normal	    -- Codes returned
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{

    /*
    **  Pass error codes back to caller
    */
    *c_errno = Cab->DWC$l_dcab_errno;
    *vaxc_errno = Cab->DWC$l_dcab_vaxc;

    /*
    **  Done.
    */
    return (DWC$k_db_normal);
}    

/*
**  End of Module
*/
