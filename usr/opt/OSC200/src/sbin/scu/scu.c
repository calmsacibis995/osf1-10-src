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
static char *rcsid = "@(#)$RCSfile: scu.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/11/23 23:08:46 $";
#endif
/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived	from  software	received  from	the	*
 *   University    of	California,   Berkeley,   and	from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is	subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * Program:	scu - SCSI Utility Program
 * Author:	Robin T. Miller
 * Date:	August 8, 1991
 *
 * Description:
 *	This program is used to issue various commands to SCSI devices
 * and the CAM sub-system.
 *
 * Modification History:
 *
 */
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <strings.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <io/common/iotypes.h>
#include <io/cam/scsi_special.h>
#include "scu.h"
#include "scu_device.h"

/*
 * External Declarations:
 */
extern void (*cdbg_printf)();
extern char *getenv(char *name);
extern char *malloc_palign(int size);

extern int CheckDeviceType (u_long dtype_mask);
extern void CloseFile (FILE **fp);
extern u_long CvtStrtoValue (char *str, char **eptr, int base);
extern int DoDeviceSetup (struct scu_device *scu);
extern void Fprintf(), Perror(), Printf();
extern char *GetInput (char *bufptr, int bufsiz);
extern void InitPageTables();
extern void Lprintf();
extern struct scu_device *MakeDeviceEntry();
extern int OpenStartupFile (FILE **fp, char *file);
extern void Pclose (FILE **fp);

/*
 * Forward References:
 */
int GetCommandLine (struct parser_control *pc);
int MakeArgList (char **argv, char *s);
void SignalHandler (int signal_number);
void Terminate (int code);

/************************************************************************
 *									*
 * main()	Process CAM/SCSI Utility Commands.			*
 *									*
 * Inputs:	argc = The argument count.				*
 *		argv = The argument pointers.				*
 *									*
 * Outputs:	Exits with 0 for Success or errno or -1 on failures.	*
 *									*
 ************************************************************************/
int
main (argc, argv)
int argc;
char **argv;
{
	register struct parser_control *pc = &ParserControl;
	register struct scu_device *scu;
	struct passwd *pwent;
	char *tmp;
	int status;

	tmp = rindex (argv[0], '/');
	OurName = tmp ? &(tmp[1]) : argv[0];
	cdbg_printf = Printf;
	PageSize = (long) getpagesize();

	/*
	 * Attempt to get the users' login home directory.
	 */
	if ( (pwent = getpwuid (getuid())) != (struct passwd *) 0) {
	    Home = pwent->pw_dir;
	} else {
	    Home = ".";
	    Fprintf ("Warning, unable to get your login information.");
	}

	/*
	 * Open the User Agent Device first.
	 */
	UagtFileFlags = (FREAD | FWRITE);
	if ( (UagtFd = open (DEFAULT_DEVICE, O_RDWR)) < 0) {
	    if ( (UagtFd = open (DEFAULT_DEVICE, O_RDONLY)) < 0) {
		Perror ("Unable to open device '%s'", DEFAULT_DEVICE);
		exit (errno);
	    } else {
		UagtFileFlags = FREAD;
	    }
	}

	InitPageTables();
	ScuDevice = scu = MakeDeviceEntry();
	if (scu == (struct scu_device *) 0) {
	    exit (ENOMEM);
	}

	/*
	 * Catch a couple signals to do elegant cleanup.
	 */
	(void) signal (SIGHUP, SignalHandler);
	(void) signal (SIGINT, SignalHandler);
	(void) signal (SIGTERM, SignalHandler);
	(void) signal (SIGPIPE, SignalHandler);

	/*
	 * Allocate a work buffer & a temporary buffer.
	 */
	if ((InputBufPtr = (char *) malloc (ARGS_BUFFER_SIZE)) == (char *) 0) {
	    exit (ENOMEM);
	}
	if ((OutputBufPtr = (char *) malloc (BUFFER_SIZE)) == (char *) 0) {
	    exit (ENOMEM);
	}
	if ((TmpBufPtr = malloc_palign (SCU_BUFFER_SIZE)) == (char *) 0) {
	    exit (ENOMEM);
	}
	if ((WrkBufPtr = malloc_palign (SCU_BUFFER_SIZE)) == (char *) 0) {
	    exit (ENOMEM);
	}

	/*
	 * Setup parser parameters and parse the command line.
	 */
	Pc = pc;			/* Global parser control ptr.	*/
	pc->pc_name = OurName;		/* Pointer to program name.	*/
	pc->pc_cp = CmdTable;		/* Set command table address.	*/
	pc->pc_func = (int (*)())CheckDeviceType; /* Control function.	*/
	pc->pc_gets = (char (*)())GetInput; /* Set the input function.	*/
	pc->pc_inpbufptr = InputBufPtr;	/* The input buffer pointer.	*/
	pc->pc_inpbufsiz = ARGS_BUFFER_SIZE; /* The input buffer size.	*/
	pc->pc_printf = (int (*)())Printf; /* Set the printf function.	*/
	pc->pc_fprintf = (int (*)())Fprintf; /* Set fprintf function.	*/
	pc->pc_special = (int (*)())CvtStrtoValue; /* Special numbbers.	*/

	/*
	 * Attempt to open and execute commands in a startup file.
	 */
	if (OpenStartupFile (&InputFp, OurName) == SUCCESS) {
	    ExitFlag = TRUE;		/* Return after end of file.	*/
	    (void) DoCommandLoop (pc);
	}

	/*
	 * Parse the command line option after the startup file to allow
	 * these parameters to override those set in the startup file.
	 */
	if ( (argc > 2) && (equal(argv[1], "-f")) ) {
	    argc -= 2;
	    scu->scu_bus = NEXUS_NOT_SET;
	    if (scu->scu_device_entry) {
		ResetDeviceEntry (scu, HARD);
	    }
	    scu->scu_device_entry = argv[2];
	    argv += 2;
	} else if (scu->scu_device_entry == NULL) {
	    scu->scu_device_entry = getenv ("SCU_DEVICE");
	}

	/*
	 * If device specified, attempt to setup device information.
	 */
	if ( (scu->scu_device_entry) && (scu->scu_bus == NEXUS_NOT_SET) ) {
	    if ( (status = DoDeviceSetup (scu)) != SUCCESS) {
		exit (status);
	    }
	}

	/*
	 * Setup to parse command line arguments (if any).
	 */
	pc->pc_argc = --argc;		/* Setup the argument count.	*/
	pc->pc_argv = ++argv;		/* Setup the argument array.	*/

	if (pc->pc_argc == 0) {
	    ExitFlag = FALSE;		/* Stay in interactive loop.	*/
	    InteractiveFlag = TRUE;	/* Show running interactively.	*/
	    pc->pc_flags |= PC_INTERACTIVE;
	    if (WatchProgress < 0) {
		WatchProgress = TRUE;	/* Watch the I/O progress.	*/
	    }
	} else {
	    ExitFlag = TRUE;		/* Exit after processing cmd.	*/
	    if (WatchProgress < 0) {
		WatchProgress = FALSE;	/* Don't watch I/O progress.	*/
	    }
	}

	status = DoCommandLoop (pc);	/* Do the main command loop.	*/

	Terminate ((ExitStatus != SUCCESS) ? ExitStatus : status);
	/*NOTREACHED*/
}

/************************************************************************
 *									*
 * DoCommandLoop() - Do Main Command Loop.				*
 *									*
 * Description:								*
 *	This function gets and executes command lines until EOF or an	*
 * error occcurs.  The input will be the user or from a command	file.	*
 *									*
 * Inputs:	pc = The parser control block.				*
 *									*
 * Outputs:	Status from last command executed.			*
 *									*
 ************************************************************************/
int
DoCommandLoop (pc)
struct parser_control *pc;
{
	int status;

	do {
	    CmdInProgressFlag = FALSE;
	    CmdInterruptedFlag = FALSE;
	    if ( (InteractiveFlag && pc->pc_argc == 0) || (InputFp != NULL)) {
		if ((status = GetCommandLine (pc)) == END_OF_FILE) {
		    if (InputFp != NULL) {
			CloseFile (&InputFp);
		    } else {
			ExitFlag = TRUE;
		    }
		}
	    }
	    /*
	     * Parse the commands/keywords.
	     */
	    if (pc->pc_argc > 0) {
		if ( (status = ParseCommands (pc)) == PC$_FAILURE) {
		    pc->pc_argc = 0;	/* Flush remaining commands.	*/
		    if (InputFp != NULL) {
			CloseFile (&InputFp);
		    }
		}
	    }
	    if (PipeFp != NULL) Pclose (&PipeFp);
/*	    if (OutputFp != NULL) CloseFile (&OutputFp);	*/
	} while ( (ExitFlag == FALSE) || (InputFp != NULL) );

	return (status);
}

/************************************************************************
 *									*
 * GetCommandLine() - Get Command Line to Execute.			*
 *									*
 * Description:								*
 *	This function gets the next command line to execute.  This can	*
 * come from the user or from a command file.				*
 *									*
 * Inputs:	pc = The parser control block.				*
 *									*
 * Outputs:	Fills in the parser control block with argument count	*
 *		 and list.						*
 *									*
 ************************************************************************/
int
GetCommandLine (pc)
struct parser_control *pc;
{
	static char *arglist[ARGV_BUFFER_SIZE];

	if (InputFp == NULL) {
	    (void) (*pc->pc_printf)("%s> ", pc->pc_name);
	} else if (OutputFp != NULL) {
	    Lprintf ("%s> ", pc->pc_name);
	}
	if ((*pc->pc_gets)(pc->pc_inpbufptr, pc->pc_inpbufsiz) == NULL) {
	    if (pc->pc_flags & PC_INTERACTIVE) {
		(*pc->pc_printf)("\n");
	    }
	    pc->pc_argc = 0;
	    return (END_OF_FILE);
	}
	pc->pc_argv = arglist;
	pc->pc_argc = MakeArgList (pc->pc_argv, pc->pc_inpbufptr);
	return (SUCCESS);
}

/************************************************************************
 *									*
 * MakeArgList() - Make Argument List from String.			*
 *									*
 * Description:								*
 *	This function creates an argument list from the string passed	*
 * in.  Arguments are separated by spaces and/or tabs and single and/or	*
 * double quotes may used to delimit arguments.				*
 *									*
 * Inputs:	argv = Pointer to argument list array.			*
 *		s = Pointer to string buffer to parse.			*
 *									*
 * Outputs:	Fills in parser control block with argument count &	*
 *		  list and returns the number of arguments parsed.	*
 *									*
 ************************************************************************/
int
MakeArgList (argv, s)
char **argv;
register char *s;
{
	register int c, c1;
	register char *cp;
	int nargs = 0;

	/*
	 * Skip over leading tabs and spaces.
	 */
	while ( ((c = *s) == ' ') ||
		 (c == '\t') ) {
			s++;
	}
	if ( (c == '\0') || (c == '\n') ) {
	    return (nargs);
	}
	/*
	 * Strip trailing tabs and spaces.
	 */
	for (cp = s; ( ((c1 = *cp) != '\0') && (c1 != '\n') ); cp++)
		;
	do {
	    *cp-- = '\0';
	} while ( ((c1 = *cp) == ' ') || (c1 == '\t') );

	*argv++ = s;
	for (c = *s++; ; c = *s++) {

	    switch (c) {

		case '\t':
		case ' ':
		    *(s-1) = '\0';
		    while ( ((c = *s) == ' ') ||
			     (c == '\t') ) {
			s++;
		    }
		    *argv++ = s;
		    nargs++;
		    break;

		case '\0':
		case '\n':
		    nargs++;
		    *argv = 0;
		    *(s-1) = '\0';
		    return (nargs);

		case '"':
		case '\'':
		    *(argv-1) = s;
		    while ( (c1 = *s++) != c) {
			if ( (c1 == '\0') || (c1 == '\n') ) {
			    Printf ("Missing trailing quote\n");
			    return (0);
			}
		    }
		    *(s-1) = '\0';
		    break;

		default:
		    break;
	    }
	}
}

/************************************************************************
 *									*
 * SignalHandler() - Catch & Process Program Signals.			*
 *									*
 * Description:								*
 *	This function catches certain signals and determines whether	*
 * we should exit the program on this signal or not.			*
 *									*
 * Inputs:	signal_number = The signal number.			*
 *									*
 * Return Value:							*
 *		Void.							*
 *									*
 ************************************************************************/
void
SignalHandler (signal_number)
int signal_number;
{
	/*
	 * Since there is a window when the CmdInProgressFlag may not be
	 * set during a command, we always set the interrupt flag.
	 */
	CmdInterruptedFlag = TRUE;

	/*
	 * The the signal is SIGPIPE, we MUST close the pipe fd now so
	 * the debug messages below don't attempt writing to the pipe.
	 * ---> Failure to do this creates a dead-lock condition. <---
	 */
	if (signal_number == SIGPIPE) {
	    if (PipeFp != NULL) Pclose (&PipeFp);
	}

	/*
	 * If we are in interactive mode or this is an expected signal,
	 * then do not exit the program.
	 *
	 * Note:  Ordinarily, I would terminate the program on SIGTERM,
	 *	  however the special code must release the Sim Q after
	 *	  the command is aborted.  This must remain until the
	 *	  CAM_SIM_QFRZDIS bit is implemented in the sub-system.
	 */
	if ( (InteractiveFlag == TRUE) || (signal_number == SIGINT) ||
	     (signal_number == SIGPIPE) || (signal_number == SIGTERM) ) {
	    if (DebugFlag) {
		Printf ("Signal %d detected, continuing...\n", signal_number);
	    }
	    return;
	}

	if (DebugFlag) {
	    Printf ("Signal %d was detected, exiting...\n", signal_number);
	}

	Terminate (signal_number);
}

/************************************************************************
 *									*
 * Terminate() - Terminate Program with Specified Exit Code.		*
 *									*
 * Description:								*
 *	This function catches certain signals and exits cleanly.  If we	*
 * don't catch these signals, output files (when redirected) won't get	*
 * flushed and we'll lose some (or all) of the output.			*
 *									*
 * Inputs:								*
 *	code = The exit code or signal number if kill done.		*
 *									*
 ************************************************************************/
void
Terminate (code)
int code;
{
	(void) fflush (stderr);		/* Flush any error messages.	*/
	(void) fflush (stdout);		/* Flush any output messages.	*/
	if (OutputFp != NULL) CloseFile (&OutputFp); /* Close & flush.	*/
	if (CoreDumpFlag && (code != SUCCESS)) {
	    abort();			/* Debug, generate a core dump.	*/
	}
	exit (code);
}
