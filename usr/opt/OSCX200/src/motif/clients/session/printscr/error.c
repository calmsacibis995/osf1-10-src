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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader = "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/session/printscr/error.c,v 1.1 90/01/01 00:00:00 devrcs Exp $";
#endif		/* BuildSystemHeader */
/*
**++
**  COPYRIGHT (c) 1987 BY
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
**  ALL RIGHTS RESERVED.
**
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
**  ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
**  OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
**  TRANSFERRED.
**
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
**  AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
**  CORPORATION.
**
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
**--
**/


/*
**++
**  MODULE:
**
**	error.c
**
**  FACILITY:
**      PrintScreen
**
**  ABSTRACT:
**
**	Print error message and exit or print warning string
**
**  ENVIRONMENT
**
**		VMS V5, Ultrix 2.2, DW FT1
**  AUTHORS:
**      Kathy Robinson, Mark Antonelli
**
**  RELEASER:
**	Kathy Robinson
**
**  CREATION DATE:
**	April 26, 1987
**
**  MODIFICATION HISTORY:
**--
**/
/*
 *	error( status )
 *		status  - (RO) longword error status
 *	warning( string )
 *		string	- (RO) warning string
 *
 *	environment:
 *	requires:
 * 		none
 *	returns:
 *		exit status codes
 *	notes:
 *		this is DECWindows and its undefined
 *		(dialog boxes might be nice)
 */

#include <stdio.h>
#include "iprdw.h"

#define BUFSIZE 512

error( status )
long 	status;
{
	char message[BUFSIZE];	/*  message buffer size		*/

	switch( status )
	{
		case DECW$_PRSC_INVDEV_ID:
			strcpy( message, "invalid device id" );
			break;
		case DECW$_PRSC_INVWIN_ID:
			strcpy( message, "invalid window id" );
			break;
		case DECW$_PRSC_INVASPECT:
			strcpy( message, "invalid aspect ratio" );
			break;
		case DECW$_PRSC_INVPR_COLOR:
			strcpy( message, "invalid printer color" );
			break;
		case DECW$_PRSC_INVPR_DEST:
			strcpy( message, "invalid print destination" );
			break;
		case DECW$_PRSC_INVPR_Q:
			strcpy( message, "invalid queue managment" );
			break;
		case DECW$_PRSC_INVREVIMG:
			strcpy( message, "invalid reverse image");
			break;
		case DECW$_PRSC_INVSTORAGE:
			strcpy( message, "invalid storage format" );
			break;
		case DECW$_PRSC_INVFORM:
			strcpy( message, "invalid form control" );
			break;
		case dxPrscInvItem:
			strcpy( message, "invalid code in item list" );
			break;
		case DECW$_PRSC_FATERRLIB:
			strcpy( message, "fatal internal library error" );
			break;
		case DECW$_PRSC_NOIMAGE:
			strcpy( message, "cannot get an image" );
			break;
		case DECW$_PRSC_BADFILEIO:
			strcpy( message, "file i/o error" );
			break;
		case DECW$_PRSC_INTCHKFAIL:
			strcpy( message, "internal consistancy check error" );
			break;
		case DECW$_PRSC_NOMEMORY:
			strcpy( message, "cannot allocate memory" );
			break;
		case XERROR:
			strcpy( message, "internal X error" );
			break;
		case FUNERROR:
			strcpy( message, "unimplemented functionality" );
			break;
		default:
			sprintf( message,"unknown error %d", status );
			break;
	}
	fprintf( stderr, "printscreen: %s \n", message );
#ifdef VMS
	/*
	 * VMS will choke on an error code, until we do it right
	 * return a normal code
	 */
	exit( SS$_NORMAL );
#else if ULTRIX
	exit( status );
#endif
}

/*
 * what we eventually want is a dialog box
 */
warning( string )
char	*string;
{
	fprintf( stderr, "printscreen: %s \n", string );
	return( SS$_NORMAL );
}
