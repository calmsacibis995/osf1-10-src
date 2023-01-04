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
static char	*sccsid = "@(#)$RCSfile: Errors.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:59:30 $";
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
#if SEC_BASE

/*
	filename:
		Errors.c
	
	copyright:
		Copyright (c) 1989-1990 SKM, L.P.
		Copyright (c) 1989-1990 SecureWare Inc.
		Copyright (c) 1989-1990 MetaMedia Inc.
		ALL RIGHTS RESERVED

	function:
		provide aif user interface common error mechanisms
		based on XErrors.c

	define's used:
		SUPPORT_ERROR_NUMBER
		TRAVERSAL
		
	notes:
		to provide easy transportability into other systems, not 
		dependent upon message text file, instead text which the
		user may see has been added at the top of the file
		
	entry points:
		CodeError()
		
		ConfirmationStart();
		ConfirmationOpen();
		ConfirmationClose();
		ConfirmationStop();

		ErrorMessageStart();
		ErrorMessageOpen();
		ErrorMessageClose();
		ErrorMessageStop();

		PleaseWaitOpen();
		PleaseWaitClose();
		
		SystemErrorMessageOpen();   Routed through WaitErrorMessageOpen
		WaitConfirmationOpen();     Supports inline confirmation
		WaitErrorMessageOpen();     Supports inline confirmation

	crucial parameters:
		ConfirmationOpen(class, i_index, CharPtr, ok_callback, cancel_callback)
		ErrorMessageOpen(tag, class, i_index, CharPtr)
		void PleaseWaitOpen(char **class, int i_index)
		SystemErrorMessageOpen(tag, class, i_index, CharPtr)

		int WaitConfirmationOpen(class, i_index, CharPtr) 
		TRUE == OK, FALSE == CANCEL
		void WaitErrorMessageOpen(tag, class, i_index, CharPtr)
*/

/* Common C include files */
#include <sys/types.h>
#include <sys/security.h>
#include <sys/audit.h>
#include <malloc.h>


/* include files for the main program this is linked with */
#include "UIMain.h"

/* Needed functional declarations */
extern void
		MemoryError(),
		StopFunctionalUnits();

void    SystemErrorMessageOpen(),
		WaitErrorMessageOpen();

static void
		ConfirmationClose(),
		ErrorMessageCallback(),
		WaitCallback();

/* Local variables */
static int
		confirm_open_flag,
		error_open_flag,
		please_wait_open_flag,
		user_wait;

static char 
		*msg[] = {
		 /* 0*/ "Insufficient memory.",
		 /* 1*/ "Code error!",
		 /* 2*/ "OK",
		 /* 3*/ "Cancel",
		 /* 4*/ "Help",
		 /* 5*/ "(Message Number %d)",
		 /* 6*/ "(System Error Number %d)",
				""
				} ;

void CodeError()
{
	/* AUDIT code failure */
	audit_no_resource("Role program", OT_PROCESS, "Bad code trapped");
	/* Post dialog with error message */
	SystemErrorMessageOpen(1310, msg, 1, NULL);
	/* Dies */
}

void ConfirmationStart() 
{
}

void 
ConfirmationOpen(class, i_index, CharPtr, ConfirmOKCallback, ConfirmCancelCallback)
	char      **class;
	int         i_index;
	char       *CharPtr;
	void       (*ConfirmOKCallback)(),
			   (*ConfirmCancelCallback)();
{
	DispMessage(class, i_index, CharPtr);
}

static void 
ConfirmationClose() 
{
}

void 
ConfirmationStop() 
{
}  

void 
ErrorMessageStart() 
{
}

void 
ErrorMessageOpen(tag, class, i_index, CharPtr)
	int         tag;
	char      **class;
	int         i_index;
	char       *CharPtr; /* client_data */
{
#ifdef SUPPORT_ERRROR_NUMBER
	if (tag != -1)    
		count_total++;
#endif
#ifdef SUPPORT_ERROR_NUMBER
	if (tag != -1) {
		sprintf(buf, msg[5], tag);
	}
#endif
	DispMessage(class, i_index, CharPtr);
}

void 
ErrorMessageClose() 
{
}

void 
ErrorMessageStop() 
{
}  


void 
SystemErrorMessageOpen(tag, class, i_index, CharPtr)
	int         tag;
	char        **class;
	int         i_index;
	char        *CharPtr;
{
	ErrorMessageOpen(tag, class, i_index, CharPtr);
	exit(1);
}

#endif /* SEC_BASE */
