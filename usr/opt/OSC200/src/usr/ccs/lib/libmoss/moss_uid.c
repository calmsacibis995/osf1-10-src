/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
#ifndef lint
static char *rcsid = "@(#)$RCSfile: moss_uid.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:33:56 $";
#endif
/*
**+
**
**  Copyright (c) Digital Equipment Corporation, 1991, 1992
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
**
**-
*/

/*
**++
**  FACILITY:  MOSS, Managed Object Support Routines
**
**  MODULE DESCRIPTION:
**
**      This module contains the VMS-supplied extensions to the
**	MOSS routine set which deal with Universal Identifiers (UIDs):
**
**		moss_free_uid()
**		moss_get_uid()
**		moss_uid_to_text()
**
**  AUTHORS:
**
**      Rich Bouchard, Jr.
**
**  CREATION DATE:  May 24, 1991
**
**  MODIFICATION HISTORY:
**
**	F-11	RJB1213		Richard J. Bouchard Jr.	22-Jul-1992
**		Incorporate misc. Mold IV Formal Inspection comments
**
**	F-10	RNM		Russell N. Murray	 6-May-1992
**		Only unlock pages in moss_get_uid() that were locked by 
**		moss_get_uid(). 
**
**	F-9	STP		Steve Pitcher		07-Apr-1992
**		SPRINTF is thread safe.  Don't bother putting CMA locks
**		around it.
**
**	F-8	DMR1168		David M. Rosenberg	 6-Mar-1992
**		Add moss_uid_to_text().
**		Add copyright notice.
**		Add SEQUENCE_MODULO to moss_get_uid() to prevent overflow.
**
**	F-7	DMR1163		David M. Rosenberg	25-Feb-1992
**		Include the value of the SCSSYSTEMID SYSGEN parameter in
**			the fabrication of "facimile" UIDs in moss_get_uid()
**			to reduce the likelihood of collisions (duplicates)
**			in a cluster.
**		Add a counter to reduce the likelihood of two calls
**			to moss_get_uid() [within a very short period
**			of time] getting the same UID (timestamp).
**
**	F-6	RJB1161	    Richard J. Bouchard Jr.	30-Dec-1991
**		Add a condition compilation around the call to the
**		UID system service, so it is not included on the
**		pre-V5.4 version of the kit.
**
**	F-5	RJB1104	    Richard J. Bouchard Jr.	06-Dec-1991
**		Remove directory references from #include statements.
**
**	F-4	RJB1093	    Richard J. Bouchard Jr.	13-Nov-1991
**		Make up a better fake UID, using $GETTIM
**
**	F-3	RJB1092	    Richard J. Bouchard Jr.	03-Nov-1991
**		Make up a fake UID if the UID service is not
**		available.
**
**	F-2	RJB1082	    Richard J. Bouchard Jr.	10-Oct-1991
**		Update moss_get_uid() to lock UID into working set
**		before calling system service, which runs at IPL 8.
**
**      F-1	RJB1023	    Richard J. Bouchard Jr.	24-May-1991
**--
*/

#include "man_data.h"

#include <stdio.h>
#include <stdlib.h>

#include "man.h"
#include "moss.h"
#include "moss_extend.h"

#ifdef VMS
#include "itmlst.h"
#include "sme_common.h"
#include "sme_builtins.h"
#include <psldef>
/*
 * Temporary includes until true UID service
 * is available on all VMS versions.
 */
#include <ssdef>
#include <syidef>
/*
 * [end] Temporary includes...
 */
#endif

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine deallocates a Universal Identifier (UID)
**
**  FORMAL PARAMETERS:
**
**      uid_value	A pointer to a pointer to a UID.  If this is a
**			pointer to NULL then space is allocated for the
**			UID, otherwise the same storage space is reused.
**
**  RETURN VALUE:
**
**	MAN_C_SUCCESS		    Normal successful completion.
**      MAN_C_BAD_PARAMETER	    The pointer is NULL
**      MAN_C_PROCESSING_FAILURE    Error allocating memory
**
**  DESIGN:
**
**--
*/   
man_status moss_free_uid(uid_value)

uid **uid_value;

{
    /*
     * bugout immediately with error if no output pointer is provided
     */
    if (uid_value == NULL)
        return (MAN_C_BAD_PARAMETER);
    if (*uid_value == NULL)
	return (MAN_C_BAD_PARAMETER);

    /*
     * Deallocate UID
     */
    free(*uid_value);

    *uid_value = NULL;

    return (MAN_C_SUCCESS);
}

#ifdef VMS
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine returns a Universal Identifier (UID)
**
**  FORMAL PARAMETERS:
**
**      uid_value	A pointer to a pointer to a UID.  If this is a
**			pointer to NULL then space is allocated for the
**			UID, otherwise the same storage space is reused.
**
**  RETURN VALUE:
**
**	MAN_C_SUCCESS		    Normal successful completion.
**      MAN_C_BAD_PARAMETER	    The pointer is NULL
**      MAN_C_PROCESSING_FAILURE    Error on get time or get SCSSYSTEMID
**				    SYSGEN parameter
**
**  DESIGN:
**
**	Note: This code is currently VMS-specific
**
**	This routine attempts to use the SYS$CREATE_UID system service
**	to retrieve a UID.  If that fails, a 'fake' UID is created using
**	a time value and an ascending integer.
**
**--
*/   
man_status moss_get_uid(uid **uid_value)
{
    uid	*uid_ptr;
    int vms_status;
    int lkwset1_status;
    int lkwset2_status;
    char *to_be_locked_va1[2]; /* Memory that we wish to be locked */
    char *locked_va1[2]; /* Memory that was actually locked */
    char *to_be_locked_va2[2]; /* Memory that we wish to be locked */
    char *locked_va2[2]; /* Memory that was actually locked */
    char *page1;
    char *page2;

    /*
     *	Temporary variables for getting the SCSSYSTEMID
     */
    static unsigned long int	scssystemid		= 0;
    static unsigned long int	scssystemid_length	= 0;
    static unsigned long int	sequence		= 0;

#define SEQUENCE_MODULO		0xffff


    /*
     * bugout immediately with error if no output pointer is provided
     */

    if (uid_value == NULL)
        return (MAN_C_BAD_PARAMETER);

    /*
     * Assume a non-NULL *uid_value is valid and re-use it.
     * Otherwise, allocate a new UID buffer.
     */
    uid_ptr = *uid_value;
    if (uid_ptr == NULL)
    {
	uid_ptr = (uid *)malloc(sizeof(uid));
	if (uid_ptr == NULL)
	    return (MAN_C_PROCESSING_FAILURE);
	*uid_value = uid_ptr;
    }

/*
 * Only include a call to the UID system service on VMS V5.4
 * or greater.
 */
#ifndef VMS_PRE_54
    page1 = (char *) uid_ptr;
    page2 = page1 + sizeof(uid) - 1;
    to_be_locked_va1[0] = page1;
    to_be_locked_va1[1] = page1;
    
    to_be_locked_va2[0] = page2;
    to_be_locked_va2[1] = page2;
    
    /*
     * Lock pages into the working set
     */
    lkwset1_status = SYS$LKWSET(to_be_locked_va1, locked_va1, 0);
    if ERROR_CONDITION(lkwset1_status)
        return MAN_C_PROCESSING_FAILURE;

    lkwset2_status = SYS$LKWSET(to_be_locked_va2, locked_va2, 0);
    if ERROR_CONDITION(lkwset2_status)
    {
	if (lkwset1_status != SS$_WASSET)
	    SYS$ULWSET(locked_va1, 0, 0);
        return MAN_C_PROCESSING_FAILURE;
    }

    vms_status = sys$create_uid(uid_ptr);

    /*
     * Unlock pages from the working set ONLY if they were
     * not already locked on entry to moss_get_uid().
     */
    if (lkwset1_status != SS$_WASSET)
        SYS$ULWSET(locked_va1, NULL, 0);

    if (lkwset2_status != SS$_WASSET)
        SYS$ULWSET(locked_va2, NULL, 0);
#else
    vms_status = SS$_UNSUPPORTED;
#endif

    /*
     * If the SYS$CREATE_UID system service is not supported then
     * return a fake UID.
     */
    if (vms_status == SS$_UNSUPPORTED)
    {
	/* get the current time */
	SYS$GETTIM(uid_ptr);

	/* "qualify" the time with a sequence number */
	((int *) uid_ptr)[2] = sequence;
	sequence = (++sequence) & SEQUENCE_MODULO;			/* "bump" the counter */

	/* get the SCSSYSTEMID if we do not already have it */
	if (scssystemid_length == 0)
	    {
	    ITMLST_DECL(argument_list, 1);
	    unsigned long int		iosb [2]		= {0, 0};

	    ITMLST_INIT(argument_list, 0, sizeof(unsigned long int), SYI$_SCSSYSTEMID, &scssystemid, &scssystemid_length);
	    ITMLST_INIT(argument_list, 1, 0,                         0,                0,            0);

	    vms_status = sys$getsyiw (0, NULL, NULL, argument_list, iosb, NULL, 0);
	    if ERROR_CONDITION (vms_status)
		return MAN_C_PROCESSING_FAILURE;
	    }

	/* copy the SCSSYSTEMID into the high end of the UID */
	((int *) uid_ptr)[3] = scssystemid;

        return MAN_C_SUCCESS;
    }

    /*
     * vms_status is either return from sys$create_uid or sys$getsyiw
     */
    if ERROR_CONDITION(vms_status)
	return (MAN_C_PROCESSING_FAILURE);

    return (MAN_C_SUCCESS);
}
#endif /* VMS */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine formats a Universal Identifier (UID) into a textual
**	representation.
**
**  FORMAL PARAMETERS:
**
**      opaque_uid	A pointer to a UID.  
**	text_buffer	Address of a pointer to a buffer to receive the
**			textual representation of the UID.  If it is a
**			pointer to NULL then space is allocated for the
**			buffer, otherwise the same storage space is reused.
**	format		Display format specifier.  MAN_K_UID_FORMAT_{DNA, 
**			OSF, DCE, NCS, HP, APOLLO, MICROSOFT} are the
**			known formats.  MAN_K_UID_FORMAT_NONE results in
**			a hex-dump format, as do any unrecognized format
**			specifiers.
**
**  RETURN VALUE:
**
**	MAN_C_SUCCESS		    Normal successful completion.
**      MAN_C_BAD_PARAMETER	    The pointer to opaque_uid is NULL or
**				    the address of the pointer to text_buffer
**				    is NULL.
**      MAN_C_PROCESSING_FAILURE    Error allocating memory
**
**  DESIGN:
**
**	This routine is based on the definition of the UID structure in the
**	DNA Phase V Unique Identifier Functional Specification, version TX1.0.3,
**	dated August 27, 1991.
**
**--
*/   

man_status moss_uid_to_text (opaque_uid,
			     format,
			     text_buffer)

uid *opaque_uid;
int format;
char **text_buffer;

{
    /*
     *	Local type definitions
     */
    /* Internal DNA UID data type - taken from the UID func spec section 2.2, "Internal	*/
    /* Interface and Operation."								*/
    typedef struct DNA_UID {
	unsigned int			timeLow;
	unsigned short int		timeMid;
	unsigned short int		timeHiAndVersion;
	unsigned char			clockSeqHiAndReserved;
	unsigned char			clockSeqLow;
	unsigned char			node [6];
	} DNA_UID;


    /*
     *	Local variables
     */
    DNA_UID *				dna_uid		= (DNA_UID           *) NULL;	/* DNA, etc format UID	*/
    unsigned int *			unknown_uid	= (unsigned int *) NULL;	/* unknown format UID	*/


    /*
     *	Validate parameters
     */
    /* Make sure there is a UID provided */
    if (opaque_uid == (uid *) NULL)
	return MAN_C_BAD_PARAMETER;


    /* Make sure there is a pointer to (receive) a text buffer */
    if (text_buffer == (char **) NULL)
	return MAN_C_BAD_PARAMETER;

    /* Allocate a text buffer if none is provided */
    if (*text_buffer == (char *) NULL)
	{
	/* Allocate maximum size that we need for any format */
	*text_buffer = (char *) malloc (MAN_S_UID_TEXT_BUFFER);

	/* Return indicating failure if we don't have one */
	if (*text_buffer == (char *) NULL)
	    return MAN_C_PROCESSING_FAILURE;
	}


    /*
     *	Create the textual representation of the UID based on the specified format
     */
    switch (format)
	{
	/*
	 *	This is somewhat arbitrary.  Since all of the following are interoperable
	 *	(per the UID func spec), all are treated the same for formatting.
	 */
	case MAN_K_UID_FORMAT_DNA:		/* 1 = DNA Phase V 		*/
	case MAN_K_UID_FORMAT_OSF:		/* 2 = MAN_K_UID_FORMAT_DCE	*/
	case MAN_K_UID_FORMAT_NCS:		/* 3 = MAN_K_UID_FORMAT_HP	*/
						/*   = MAN_K_UID_FORMAT_APOLLO	*/
	case MAN_K_UID_FORMAT_MICROSOFT:	/* 4				*/

		/* cast the opaque UID structure pointer into a DNA UID pointer */
		dna_uid = (DNA_UID *) opaque_uid;		/* DNA format UID	*/

		/* Format the UID - taken directly from the UID func spec section 2.3, "Visual	*/
		/* Representation of UIDs."							*/
		(void) sprintf (*text_buffer, "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
				dna_uid -> timeLow,
				dna_uid -> timeMid,
				dna_uid -> timeHiAndVersion,
				dna_uid -> clockSeqHiAndReserved,
				dna_uid -> clockSeqLow,
				dna_uid -> node [0],
				dna_uid -> node [1],
				dna_uid -> node [2],
				dna_uid -> node [3],
				dna_uid -> node [4],
				dna_uid -> node [5]);

		break;

	/*
	 *	Treat an explicit format of NONE and all unrecognized formats the same.
	 */
	case MAN_K_UID_FORMAT_NONE:		/* 0 = DNA_K_UID_FORMAT_UNKNOWN	*/
	default:				/* ? = unknown			*/

		/* cast the opaque UID structure pointer into an unknown format UID pointer */
		unknown_uid = (unsigned int *) opaque_uid;		/* unknown format UID	*/

		(void) sprintf (*text_buffer, "%08x, %08x, %08x, %08x",
				unknown_uid [0],
				unknown_uid [1],
				unknown_uid [2],
				unknown_uid [3]);

		break;
	}

    /*
     *	Return indicating success
     */
    return MAN_C_SUCCESS;
}
