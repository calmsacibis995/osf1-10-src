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
static char *rcsid = "@(#)$RCSfile: xpg4demo.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/09/03 22:10:54 $";
#endif

#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <nl_types.h>
#include "xpg4demo.h"
#include "xpg4demo_msg.h"

/*
 * Generic global variables.
 */
nl_catd		MsgCat;
char		*ProgName;
char		*DatabaseName;
EmployeeNode	*EmployeeRoot;		/* Root of employee tree */
Employee	*CurrentEmployee;
Boolean		SurnameFirst;		/* True if surname comes first */

/*
 * Variables specific to this module.
 */
static const char *xpg4demo_file = "xpg4demo.dat";
static const char *xpg4demo_msg = MF_XPG4DEMO;

static void Usage(const char *);

int
main(int argc, const char *argv[])
{
	FileMode filemode;

	ProgName = (char *) argv[0];

	/*
	 * The practice is to ignore failure of setlocale() and
	 * catopen() so that the C locale environment is silently set
	 * up.
	 */
	(void) setlocale(LC_ALL, "");

	MsgCat = catopen(xpg4demo_msg, NL_CAT_LOCALE);

	switch (argc) {
	case 1:
		DatabaseName = (char *) xpg4demo_file;
		break;
	case 2:
		DatabaseName = (char *) argv[1];
		break;
	default:
		Usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/*
	 * Get the file mode of the specified database.
	 */
	switch (filemode = CheckFileMode(DatabaseName)) {
	case FileReadonly:
		Warning(GetErrorMsg(E_XPG_DBREAD, "Database is readonly"));
		break;

	case FileWritable:
		break;

	case FileNew:
		Warning(GetErrorMsg(E_XPG_NEWFILE, "New data file created"));
		break;

	case FileBad:
		perror(ProgName);
		exit(2);
	}

	/*
	 * Initialize the command environment.
	 */
	if (InitCommand() < 0) {
		Fatal(GetErrorMsg(E_XPG_INIT_COM,
				  "Cannot initialize command environment"));
		/*NOTREACHED*/
	}

	/*
	 * If the employee data file exists, read all
	 * the records in the file.
	 */
	if (filemode != FileNew)
		if (ReadDatabase(DatabaseName) < 0) {
			Fatal(GetErrorMsg(E_XPG_READ,
					  "Cannot read employee data file %s"),
			      DatabaseName);
			/*NOTREACHED*/
		}

	/*
	 * Initialize the screen environment.
	 */
	if (InitScreen() < 0) {
		Fatal(GetErrorMsg(E_XPG_INIT_SCR,
				  "Cannot initialize screen"));
		/*NOTREACHED*/
	}

	/*
	 * If there is a message to display, do not change to full
	 * screen mode until the user presses the Return key.
	 */
	if (NErrors) {
		wchar_t buf[80];

		Ask(GetMsg(I_XPG_CONTINUE,
			   "Press Return to continue. "));
		(void) fgetws(buf, NUM_ELEMENTS(buf), stdin);
	}

	/*
	 * Display an employee record, if any, for the initial screen. 
	 */
	ShowEmployee(CurrentEmployee);

	/*
	 * The main command loop.
	 */
	CommandLoop();

	/*
	 * If the employee data is modified, update the database.
	 */
	if (ModifiedFlag == TRUE && filemode != FileReadonly)
	        if (WriteDatabase(DatabaseName) < 0) {
			Warning(GetErrorMsg(E_XPG_WRITE, "Write error!"));
		}

	(void) catclose(MsgCat);
	exit(0);
}

/*
 * Print a usage message.
 */
static void
Usage(const char *myname)
{
	Warning(GetErrorMsg(E_XPG_USAGE, "Usage: %s [database]\n"),
		myname);
}
