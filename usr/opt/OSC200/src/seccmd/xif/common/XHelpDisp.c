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
static char	*sccsid = "@(#)$RCSfile: XHelpDisp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:05:03 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#ifdef SEC_BASE

/*
	filename:
		XHelpDisp.c
	
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED
	
	function:
		provide interface for motif help requests
  
	structure:
		Help is accessed via the callback function:
			HelpDisplayOpen()
			
		Data is passed in via user data as a pointer to a char string:
			<type>,<file_name>
		The type controls the directory in which information is searched
		and file is the name of the file within the type subdirectory
		
		Any string missing the ',' will generate a request of type
			"." 
*/

/* Common C include files */
#include <sys/types.h>
#ifdef AUX
#include <string.h>
#else
#include <strings.h>
#endif
#include <limits.h>

/* X Include files */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>

/* role program include file */
#include "XMain.h"

/* External routines */

/* Defines */
#define TOTAL_LEN   60

/* Local variables */
static char
	**msg_helperr,
	*msg_helperr_text,
	**help_type,
	*help_type_text,
	**help_dir,
	*help_dir_text;
		
void 
HelpDisplayStart()
{
	/* Nothing needed here */
}

void 
HelpDisplayOpen(w, user_data, info)
	Widget      w;
	char        *user_data;
	XmAnyCallbackStruct *info;
{

	char        *cp,
			fname[PATH_MAX+1],
			type[TOTAL_LEN+1];
	int         i,
			n;

	/* Pick up help messages */
	if (! msg_helperr)
		LoadMessage("msg_help_err", &msg_helperr, &msg_helperr_text);
		
	/* Pick up translation tables for help directories */
	if (! help_dir)
		LoadMessage("msg_help_dir", &help_dir, &help_dir_text);
	if (! help_type)
		LoadMessage("msg_help_type", &help_type, &help_type_text);
	
	/**********************************************************************/
	/* Parse out type and file_name from user data passed in              */
	/**********************************************************************/
	
	strncpy(type, user_data, TOTAL_LEN);
	type[TOTAL_LEN] = '\0';
	cp = strchr(type, ',');
	if (! cp) {
		strcpy(type, ".");
		cp = user_data;
	}
	else
		*cp++ = '\0';

	/**********************************************************************/
	/* Load data first to check for problems before messing w/ windows    */
	/**********************************************************************/

	/* Build directory name */
	strcpy(fname, type);
	for (i = 0; *help_type[i]; i++)
		if (! strcmp(type, help_type[i]) ) {
			strcpy(fname, help_dir[i]);
			break;
		}
	strcat(fname, "/");
	n = PATH_MAX - strlen(fname);
	
	/* Add on help file name */
	strncat(fname, cp, n);
	fname[PATH_MAX] = '\0';
	
	/* Let file display utility actually show the help file */
	/* file display err msg = "Failed to access help file!" */
	FileDisplayOpen(fname, "Help", msg_helperr, 0, NULL, 1234);
}

void 
HelpDisplayClose() 
{
	/* Nothing needed here */
}

void 
HelpDisplayStop() 
{
	/* Nothing needed here */
}
#endif /* SEC_BASE */
