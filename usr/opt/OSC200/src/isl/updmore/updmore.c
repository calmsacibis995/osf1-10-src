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
static char *rcsid = "@(#)$RCSfile: updmore.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/11/22 22:49:24 $";
#endif

#include        <stdio.h>
#include	<ctype.h>
#include	<dirent.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<string.h>
#include	<stdlib.h>
#include	<varargs.h> 
#include	<sys/ioctl.h> 
#include	<sys/param.h> 
#include	<sys/stat.h>
#include	<sys/termios.h> 
#include	<sys/types.h> 

#define		GCROWS	60
#define		TCROWS	24

static void	MoreFile();
static void	ScreenRows();
static void	ISLScreenRows();

int	screenrows;
char	*prog;

extern char     *getenv();


/*
*	Main -
*/
main(argc,argv)
int	argc;
char	*argv[];
{
	prog = *argv;

	/*
	* 	Only one argument is acceptable.  The name of the file
	*	that is going to be viewed.
	*/
	if (argc != 2)
	{
		(void) fprintf (stderr,"Usage: %s <filename>\n", prog);
		(void) fflush (stderr);
		exit(1);
	}
	else
		++argv;

	/*
	* Check to see if the UPDFLAG is set in the environment.  If it
	* is, it means we're doing an update installation.  And are
	* working from the console.  (More than likely)
	*/
	if (getenv("UPDFLAG"))
		ISLScreenRows();
	else
		ScreenRows();

	/*
	* Ok, lets look at the file.
	*/
	MoreFile(*argv);

	exit(0);
}



static void
MoreFile(f)
char	*f;
{
	int	c=0;
	char	ans[BUFSIZ];
	char	line[BUFSIZ];
	FILE	*fp1;

	if ((fp1 = fopen(f,"r")) == NULL )
	{
		(void) fprintf (stderr,"%s: cannot open %s\n", prog, f);
		(void) fflush (stderr);
		exit(1);
	}

	for(c=1; fgets(line,BUFSIZ,fp1) != NULL;)
	{
		if (c < screenrows-2)
		{
			(void) printf ("%s",line);
			(void) fflush (stderr);
			c++;
		}
		else
		{
			c=1;
			(void) printf("\n------------------------");
			(void) printf("----------------");
			(void) printf("\nPress <RETURN> to continue ");
			(void) printf("viewing: ");
			(void) fflush (stdout);
			gets (ans);
			if(strcmp (ans,"r") == 0) break;
			(void) printf("------------------------");
			(void) printf("----------------\n\n");
			(void) fflush (stdout);
			(void) printf ("%s",line);
			(void) fflush (stderr);
		}
	}
	exit(0);
}



/*
*
*	static void
*	ScreenRows()
*
*	given:	Nothing
*
*	does:	finds number of rows of the screen and sets
*		global 'screenrows'.
*
*	return: Nothing
*
*/
static void
ScreenRows()
{

	/* structure to store window info */
	struct winsize 	win;

	/* get size info of controlling terminal */
	if ((ioctl (0, TIOCGWINSZ, &win) < 0) || (win.ws_row == 0))
	{
		screenrows = TCROWS; 
		return;
	}

	screenrows = win.ws_row;
}



/*
*	static	void
*	ISLScreenRows ()
*
*	given:	nothing
*
*	does:	Finds out number of screen rows of the console device
*		and sets global variable 'screenrows'.  This is used
*		if the system is in single user mode.
*
*	return: Nothing
*
*/ 
static	void
ISLScreenRows ()
{
	struct 	devget	devget, *pdevget;
	int	console;

	/* by default, console device is a terminal */
	screenrows = TCROWS; 

	console = open ("/dev/console", (O_RDONLY|O_NDELAY));
	if (console < 0)  
		console = open ("/dev/console", (O_WRONLY|O_NDELAY));
	if (console < 0)
		return;

	pdevget = &devget;

	if (ioctl (console, DEVIOCGET, (char *) pdevget) < 0)
		return;

	/*
	*  if console is connected to the system through the serial line.
	*  we assume it's a terminal just to be safe
	*/ 
 	if (!strcmp("COLOR",pdevget->device))
		/*
		*	console is connected to the graphic controller.
		*	All graphic controller(s) share the same number
		*	of screen rows in console mode. 
		*
		*	Note: the device field is set to "COLOR" even if
		*	the monitor is B&W
		*/
		screenrows = GCROWS;  
}
