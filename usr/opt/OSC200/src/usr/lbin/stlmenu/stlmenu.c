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
static char *rcsid = "@(#)$RCSfile: stlmenu.c,v $ $Revision: 1.1.9.2 $ (DEC) $Date: 1993/07/30 18:36:49 $";
#endif
/*
 *	stlmenu.c
 *		This routine handles setld menu display/selection. It
 *		has the following syntax:
 *
 *	stlmenu -l|-x -p|-s [-c] [-d] [-i] [-m filename] display|select list1 [list2] 
 * 
 *	Meanings of the switches are:
 *	
 *		-d	debug on	(optional)
 *		-i	in isl		(optional, needed to set screen size)
 *		-c	continue option (optional, when specified, will offer
 *					 a continue option in menu tail)
 *		-l	load		-x	extract,  (must choose one)
 *		-p	package 	-s	subset,   (must choose one)
 *		-m	menufile specification (optional)
 *
 *	It can be used to display a portion of the menu, or to present a full
 *	selection menu. In the latter case, two lists as well as the -m
 *	switch are expected to be provided.
 *
 *	ExamPles: 
 *		1) display the menu part for loading packages
 *			stlmenu -l -p display list1
 *		2) present selection menu for extracting subsets
 *			stlmenu -x -s -m myfile select list1 list2
 *		        	Note: upon function return, 'myfile' contains 
 *				a list of selected packages or subsets (ids) 
 *		
 */

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
#include	<setld/setld.h> 

extern char     *getenv();              
extern int	optind; 		/* getopt(3) */ 
extern char	*optarg;		/* getopt(3) */ 

/* constants */ 
#define	INSTALL	"install" 
#define	EXTRACT	"extract" 
#define	PACKAGE	"package"
#define	SUBSET	"subset"

#define	DISPLAY 1
#define	SELECT	2

#define	TCROWS		24	/* # of rows available on (VT) terminals */

#define DPRMTROWS	2	/* rows for prompt (RETURN) in DISPLAY mode */
#define	SPRMTROWS	4	/* rows for prompt in SELECT mode */

#define	TOKENSPR	" \t\n,"	/* selection token seperator */
#define	RANGESPR	'-'	/* selection range seperator, eg, 2-5 */
#define	RPAR	')'
#define SPACE	' '
#define	STAR	'*'

#define YES	'Y'
#define NO	'N'

#define	SELECTED	1	/* mark value for the subset/package */
#define	UNSELECTED	0
#define	ALL		2

#define	MAND		1	/* indicates if a mandatory list is displayed */
#define	OPT		0


#define	MAXLISTLEN	999	/* max. number of items in menu item list */
#define MAXPOS		3	/* max. number of chars in menu item index */

#define CATDESCSPR	"%"	/* category and description seperator */

extern 	char    *emalloc();             /* error checking malloc */

char		*prog;			/* name of program, i.e. stlmenu */
short		debug = 0;		/* debug flag */
short		cont_option = 0;	/* flag for the continue option */

typedef	struct menuitem {
	int		mark;		/* = SELECTED or = UNSELECTED */
	char		*name;		/* name of subset or package */
	char  		*desc;		/* text desc of subset or package */
	char		*cat;		/* category of subset or package */
} MItemT; 

static	MItemT	*mandlist[MAXLISTLEN];	/* head of mandatory menu item list */
static	MItemT	*optlist[MAXLISTLEN];	/* head of optional menu item list */

static	int	mandcnt;	/* number of items in mandatory list */
static	int	optcnt;		/* number of items in optional list */

static	char	*operation;	/* install or extract */
static	char	*object;	/* package or subset */

static	short	all, mandonly, nothing, again, ext; 

static	char	answer[BUFSIZ];		/* place holder for user input */

static	int	sellist[MAXLISTLEN];  	/* array to hold user selection */
static	int	selcnt = 0;

static	unsigned short	screenrow;	/* number of menu rows to display */
static	short	linecnt = 0;		/* number of lines displayed */
static	short	act;			/* = DISPLAY or SELECT */
static	short	mode = DISPLAY;		/* = DISPLAY or SELECT used to decide 
					   screen overflow message */

static	short	in_isl = 0;		/* in initial system load */
static	short	opt_selected = 0;	/* optional objects selected */
static	short	opt_cat = 1;		/* if categories are specified with
					   optional objects */

static	void 	ISLScreenRows();/* returns # of rows of console device */ 
static	void	MenuChoices();	/* print out selections so far */
static	int	MenuConfirm();	/* asks user to confirm the selection(s) */ 
static 	int	MenuDisplay();	/* displays a menu item list */
static	MItemT 	*MenuItemNew();	/* create a new menu item */
static	int 	MenuLines(); 	/* output lines to menu screen */
static	int	MenuList(); 	/* set up a menu item list */
static	void	MenuNum();	/* determine menu numbers for tail section */
static	int	MenuReturn();	/* returns selected menu item to caller */ 
static	void	MenuScreenRows();/* get screen screen size */
static	int	MenuSelCheck();	/* checks validity of user selection */
static 	void	MenuSelect();	/* display menu list and handles selection */
static 	int	MenuTail();	/* determine and display menu tail portion */
static	void	MenuUsage();	/* prints usage message */

main (argc, argv)
int	argc;
char	*argv[];

{
	
	char	*inlist;
	int	op, i;
	char	*mand, *opt, *outfile;
	FILE	*fptr = NULL;
	int	objflag = 0, opflag = 0; /* flags for object, operation */

	prog = *argv;

	/* get all the switches */ 
	while ((op = getopt (argc, argv, "cdilxpsm:")) != EOF)
	{
		switch (op)
		{
		case 'c':
			cont_option++;
			break;
		case 'd':
			debug++;
			break;
		case 'i':
			in_isl = 1;
			ISLScreenRows(); 
			break;
		case 'l':	
			if (opflag)
				MenuUsage();
			else
			{
				operation = INSTALL;
				opflag = 1;
			}
			break;
		case 'm':
			outfile = optarg;
			if ( (fptr = fopen( outfile, "w" )) == NULL )
			{
				(void) fprintf (stderr,
				      "\n%s: cannot open %s\n", prog, outfile);
				(void) fflush (stderr);
				exit (1);
			}
			break;
		case 'p':
			if (objflag)
				MenuUsage();
			else
			{
				object = PACKAGE;
				objflag = 1;
			}
			break;
		case 's':
			if (objflag)
				MenuUsage();
			else
			{
				object = SUBSET;
				objflag = 1;
			}
			break;
		case 'x':
			if (opflag)
				MenuUsage();
			else
			{
				operation = EXTRACT;
				opflag = 1;
			}
			break;
		default:
			MenuUsage (); 
		}
	}

	/* did not specify object type or operation type */
	if ((!objflag) || (!opflag))
		MenuUsage ();

	if (debug)
	{
		(void) printf ("operation = %s, object = %s, outfile = %s\n", 
			 operation, object, outfile);
		(void) fflush (stdout);
	}
	
	if (optind == argc)
		MenuUsage ();

	if (!strcmp (argv[optind], "select"))
	{
		act = SELECT;  
		/* check for existence of menu output file */
		if (fptr == NULL)
			MenuUsage ();
	}
	else if (!strcmp (argv[optind], "display"))
 		act = DISPLAY;
	else
		MenuUsage ();

	if (debug)
	{
		(void) printf ("act = %d\n", act); 
		(void) fflush (stdout);
	}

	optind++;

	if (optind == argc)
		MenuUsage ();

	/* if not in ISL, set up screen size and signal handler */
	if (!in_isl)
	{
		MenuScreenRows();
		/* watch out for any screen change */
		signal (SIGWINCH,  (void (*) (int))MenuScreenRows);
	}

	/* construct the mandatory menu list, mark all items as SELECTED */ 
	mand = argv[optind++];
	if (debug)
	{
		(void) printf ("mand = %s\n", mand);
		(void) fflush (stdout);
	}

	inlist = getenv (mand);

	mandcnt = MenuList (inlist, mandlist, SELECTED);

	if (debug)
	{
		(void) printf ("mandcnt = %d\n", mandcnt);
		(void) fflush (stdout);
	}

	if (act == DISPLAY)
		(void) MenuDisplay (mandcnt, mandlist, SPACE, ALL, MAND);
	else 		/* SELECT */
	{
		/* when action is SELECT, a second list is required */
		if (optind == argc) 
			MenuUsage ();

		/* construct optional menu list, mark all with UNSELECTED */ 
		opt = argv[optind];
		if (debug)
		{
			(void) printf ("opt = %s\n", opt);
			(void) fflush (stdout);
		}

		inlist = getenv (opt);
	
		optcnt = MenuList (inlist, optlist, UNSELECTED);

		if (debug)
		{
			(void) printf ("optcnt = %d\n", optcnt);
			(void) fflush (stdout);
		}
 
		if ((mandcnt == 0) && (optcnt == 0))
			exit (0);

		/* check if categories are specified in optlist */
		if ((optcnt > 0) && (optlist[0]->cat == NULL))
			opt_cat = 0;

		/* determine the menu numbers for menu tail portion 
		   before the menu is actually displayed. This is
		   because MenuSelCheck() has been added in MenuLines */
		MenuNum ();

		/* display menu item and handle selection */
		MenuSelect();  

		/* return selected item to calling program through a file */
		MenuReturn(fptr);
	} 

	exit (0);

}

/*	static	void	ISLScreenRows ()-
 *		finds out number of screen rows of the console device 
 *	given:	nothing
 *	does:	sets global variable 'screenrow'
 */ 

static	void 	ISLScreenRows ()

{
	struct winsize 	win;		/* structure to store window info */
	int	console;

	/* by default, console device is a terminal */
	screenrow = TCROWS; 

	console = open ("/dev/console", (O_RDONLY|O_NDELAY));
	if (console < 0)  
		console = open ("/dev/console", (O_WRONLY|O_NDELAY));
	if (console < 0)
		return;

	if ((ioctl (console, TIOCGWINSZ, &win) < 0) ||
	    (win.ws_row == 0))		 /* gets size info of controlling
					   	terminal */ 
	{
		screenrow = TCROWS; 
		return;
	}

	/* Only use 7/8 of the screen because of graphics console scrolling */
	screenrow = win.ws_row - (win.ws_row >> 3);

	if (debug)
	{
		(void) printf ("ISL screen rows = %d\n", screenrow);
		(void) fflush (stdout);
	}

}

/* static void		MenuChoices ()-
 *		prints current user selections on the screen
 *	given:	global selcnt and sellist that stores all valid selections
 *    	does:	prints selections so far, if any.
 *	return:	nothing
 */
 
static	void	MenuChoices ()

{
	int	i, start, dflag;

	for (i = 0, start = sellist[0], dflag=0; i < selcnt; i++)
	{
		if (sellist[i] == start + 1) 
			if (!dflag)
			{
			 	(void) printf ("-");
			 	dflag = 1;
			}
			else
			{
				/* do nothing */
			}
		else
		{
			if (dflag) 
			{
				 /* finish the range */
				(void) printf ("%d", start);
				dflag = 0; /* reset deflag */
			}

			(void) printf (" %d", sellist[i]);
		}
		start = sellist[i];
	}

	/* unfinished range */
	if (dflag) 
		/* finish the range */
		(void) printf ("%d", sellist[i-1]);

	(void) printf (" ");
	fflush (stdout); 

}

/* 	static int	MenuConfirm ()-
 *		ask user to confirm selection(s)
 *	given:  global mandlist and optlist with selected objects marked
 *		SELECTED. 
 *	does:	display mandatory and selected optional objects by category 
 *		and ask for user confirmation.
 *	return:	1 if user confirms with 'y', 0 if 'n'
 */

static 	int	MenuConfirm ()

{
	char	pstr[BUFSIZ];
	int	i;

	for (;;)
	{
		linecnt = 0;

		if (mandcnt > 0)
		{
			(void) MenuLines (3, "\nYou are installing the following mandatory %ss:\n\n", object); 
		
			(void) MenuDisplay (mandcnt, mandlist, SPACE, SELECTED, MAND);
		}
		if (opt_selected) /* any optionals selected */
		{
			(void) MenuLines (2, "\nYou are installing the following optional %ss:\n", object); 
			if (!opt_cat)
				(void) MenuLines (1, "\n");
			(void) MenuDisplay (optcnt, optlist, SPACE, SELECTED, OPT);
		}
	
		(void) MenuLines (1, "\nIs this correct? (y/n): ");
	
		gets (pstr);
		if (*pstr == '\0')	/* a RETURN entered at the prompt */
			continue;
		sscanf (pstr, "%s", answer);
		switch (toupper (*answer))
		{
		case YES:
			return 1;
		case NO:
			/* reset selection info on optionals */
			opt_selected = 0;
			for (i = 0; i < optcnt; i++)
				optlist[i]->mark = UNSELECTED;
			return 0;
		}
	}

}


/*	static int	MenuDisplay ()-
*		display a menu item list
 *	given: 	1) the number of items in the list
 *		2) the list containing the menu items
 *		3) prefix before each menu item
 *		4) flag to show whether to display all objects or just
 *		   selected objects.
 *		5) flag to indicate whether a mandatory list is displayed
 *		   category for mandatory objects are not displayed.
 *	does:	displays the items in one column with specified prefix. 
 *	return:	1 if menu successfully displayed, 0 if not
 */ 

static int	MenuDisplay (count, arr, prefix, allflag, mandflag)
int	count;
MItemT	*arr[];
char	prefix;
short 	allflag, mandflag;

{
	int	rowcnt, i; 
	char 	*curcat;/* current category being displayed */

	curcat = strdup ("%%NO-CAT%%");	/* an unlikely category to start with */
	
	switch (prefix)
	{
	case RPAR:
		for (i = 0; i < count; i++) 
		{
			if ((mode == SELECT) && (i == 0))
				/* no optional subsets displayed for
				   selection yet, this affects behavior
				   of MenuLines */
				mode = DISPLAY;
			else 
				mode = SELECT;

			if ((arr[i]->cat != NULL) &&
			    (strcmp (arr[i]->cat, curcat)))
			{
				curcat = arr[i]->cat;
				/* eliminate possibility of an orphaned 
				   category by making one MenuLines call */
				if (!MenuLines (3, "\n - %s:\n   %3d) %-72.72s\n", arr[i]->cat, i+1, arr[i]->desc))
					return 0;
			} 
			else
			{
				if (!MenuLines (1, "   %3d) %-72.72s\n", i+1, arr[i]->desc))
					return 0;
			}
		}
		break;
	default:  
		for (i = 0; i < count; i++) 
		{
			/* if displaying selected items only, jump over
			   unselected items */
			if ((allflag == SELECTED) && (arr[i]->mark != SELECTED))
				continue;
  
			if (!mandflag && (arr[i]->cat != NULL) &&
			    (strcmp (arr[i]->cat, curcat)))
			{
				curcat = arr[i]->cat;
				/* eliminate possibility of an orphaned 
				   category by making one MenuLines call */
				if (!MenuLines (3, "\n - %s:\n      %c %-72.72s\n", arr[i]->cat, prefix, arr[i]->desc))
					return 0; 
			}
			else
			{
				if (!MenuLines (1, "      %c %-72.72s\n", prefix, arr[i]->desc))
					return 0;
			}
		}
		break;
	}

	return 1;
}


/*	static	MItemT	*MenuItemNew () -
 *		allocate storage for a new menu item and initialize it
 *	given:	name, description and proper mark of the new menu item
 *	return:	a pointer to MItemT
 */

static	MItemT	*MenuItemNew (name, desc, mark)
char	*name, *desc;
int	mark;

{
	register MItemT	*node;
	StringT	str; 
	static	char *curcat = NULL;/* current category being processed */
	char 	*spr;
	
	node = (MItemT *) emalloc (sizeof (MItemT));

	/* set name field */
	node->name = strdup (name); 

	/* again, can not use strtok because there's another strtok
	   call in MenuList */
 
	/* set cat field and description field */
	spr = strstr (desc, CATDESCSPR); 
	if (spr == NULL)	/* no category specified */
	{
		if (curcat != NULL) 
		{
			/* put it in the "Other" category */
			node->cat = strdup ("Other");
			curcat = node->cat;
		} 
		else
			node->cat = NULL;
		/* set description field */
		node->desc = strdup (desc);
	}
	else	/* category specified */
	{
		(void) strncpy (str, desc, (size_t) (spr - desc));
		str[spr-desc] = '\0'; /* null terminate the string */

		/* set description field */
		node->desc= strdup (str); 

		/* set categroy field */
		node->cat = strdup (spr + strlen (CATDESCSPR));
		curcat = node->cat; /* set pointer to current category */
		if (debug) 
		{
			(void) printf (" node->cat set to %s\n", node->cat);
			(void) printf (" curcat just set to %s\n", curcat);
			(void) fflush (stdout);
		}
	}

	/* set the mark. mandatory starts with SELECTED and optional with 
	   UNSELECTED */
	node->mark = mark;

	return (node);

}


/*	static int	MenuLines ()-
 *		output line(s) to the menu screen	
 *	given:  1) number of newline(s) to be output
 *		2) format string for the output line(s)
 *		3) arguments for printf	
 *	does: 	this routine outputs lines to the menu screen and is called
 *		whenever overflowing the screen is a concern.
 *	return:	1 if output successfully display, 0 if not.
 *	  	(the return value tells MenuSelect whether the menu should be 
 *		 redisplayed. It's set to 0 when the user enters an invalid
 *		 selection at the prompt )
 */ 

static	int	MenuLines (va_alist) va_dcl

{
	va_list	args;
	int	newlines, availrows;
	char	*format;
	char	ans[BUFSIZ];
	
	va_start (args);

	/* first deduct rows required for prompt */
	if (mode == DISPLAY) 
		availrows = screenrow - DPRMTROWS;
	else 
		availrows = screenrow - SPRMTROWS;

	newlines = va_arg (args, int); 

	/* if screen is full */
	if (linecnt + newlines > availrows)
	{
		if (mode == DISPLAY) 
		{
			(void) printf ("\nPress RETURN to display the next screen: ");
			(void) fflush (stdout);
			gets (ans);  
		}
		else /* mode = SELECT */
		{
			if (selcnt > 0)
			{
				(void) printf ("\n--- MORE TO FOLLOW ---\nAdd to your choices or press RETURN to display the next screen.\n\nChoices (for example, 1 2 4-6): ");
				MenuChoices(); /* prints selections so far */
			}
			else
			{
				(void) printf ("\n--- MORE TO FOLLOW ---\nEnter your choices or press RETURN to display the next screen.\n\nChoices (for example, 1 2 4-6): ");
				fflush (stdout);
			}
			gets (ans);
			/* if we get an answer here other than just a RETURN,
			   check if selection is valid. If not, we return with
		  	   0 and the menu will be redisplayed. */
			if ((*ans != '\0') && (!MenuSelCheck (ans)))
				return 0;
		}
		(void) printf ("\n");
		linecnt = 1;
	}

	format = va_arg (args, char *);

	/* print the menu lines specified */
	(void) vprintf (format, args);
	(void) fflush (stdout);
	va_end (args);

	/* update linecnt with the number of new lines just added */
	linecnt += newlines;

	return 1; 

}


/*	static int 	MenuList ()-
 *		sets up a menu item list
 *	given: 	1) a string containing menu items, i.e., subset names or package
 *		   names.
 *		2) the array to store information of the menu items 
 *		3) mark each item with SELECTED or UNSELECTED, mandatory list 
 *		   will start with all SELECTED, and optional list with all
 *		   UNSELECTED. 
 *	does:	seperate menu items from the string, get each item's data,
 *		store the data in a dynamic node and store pointer to the
 *		node in the specified array.		
 *	return: number of items stored in the list.
 */

static int 	MenuList (str, arr, mark)
char	*str;
MItemT	*arr[];
int	mark;	

{
	char	*desc, *name; 
	NameT	field;
	int	count;

	count = 0;

	if (str == NULL)
	{
		if (debug)
		{
			(void) printf ("can not getenv from %s.\n", str);
			(void) fflush (stdout);
		}
		return (count);
	}

	if (debug)
	{
		(void) printf ("in MenuList, str = %s\n", str);
		(void) fflush (stdout);
	}

	/* set up the menu list through getenv */ 
	name = StringToken (str, TOKENSPR);
	for ( ; name != NULL; name = StringToken ((char *) NULL, TOKENSPR))
	{

		/* get the text description of each, add DESC to
		   the front of the subset(package) name  */
		(void) sprintf (field, "%s%s", "DESC", name);

		desc = getenv (field);

		if (desc == NULL)
		{
			(void) fprintf (stderr, "\n%s: cannot getenv from %s.\n", prog, field);
			(void) fflush (stderr);
			exit (1);
		}

		/* put item in list and initialize it with values */
		arr[count++] = MenuItemNew (name, desc, mark);
	}

	return (count);

}

/*	static	void	MenuNum ()-
 *		determines menu item numbers for menu tail portion: all, 
 *		mandonly, again, nothing, and ext 
 *	given: 	optcnt (global, number of items in optional list)
 *	does: 	sets global variables: all, mandonly, again, nothing, and ext.
 *
 */

static	void	MenuNum ()

{ 
	all = optcnt + 1;
	
	if ((mandcnt > 0) && (optcnt > 0))
		mandonly = all + 1;
	else
		mandonly = all;

	again = mandonly + 1;

	if (cont_option)
		nothing = again + 1;
	else
		nothing = again;

	if (in_isl)
		ext = nothing;
	else 
		ext = nothing + 1;
}

/*	static  int	MenuReturn ()-
 *		returns selected menu items to calling program
 *	given:	file pointer to the output file 
 *	does:	go through mandlist and optlist, find items marked with 
 *		SELECTED and write their names to a temp file to be picked up 
 *		by the calling program 
 */

static	int	MenuReturn(fptr)
FILE	*fptr;
{
	int	i;

	for (i = 0; i < mandcnt; i++)
		(void) fprintf (fptr, "%s\n", mandlist[i]->name);

	for (i = 0; i < optcnt; i++)
		if (optlist[i]->mark == SELECTED)
			(void) fprintf (fptr, "%s\n", optlist[i]->name);
		
	(void) fclose (fptr);

	return 0;
}


/*	static void 	MenuScreenRows()-
 *		finds number of rows of the screen and set global 'screenrow'.
 *	given:	 
 *	does:	ditto	
 */

static void MenuScreenRows ()
{

	struct winsize 	win;		/* structure to store window info */

	if ((ioctl (0, TIOCGWINSZ, &win) < 0) ||
	    (win.ws_row == 0))		 /* gets size info of controlling
					   	terminal */ 
	{
		screenrow = TCROWS; 
		return;
	}

	/* Only use 7/8 of the screen because of graphics consoles scrolling */
	screenrow = win.ws_row - (win.ws_row >> 3);

	if (debug)
	{
		(void) printf ("\nscreen row = %d\n", screenrow);
		(void) fflush (stdout);
	}
}


/*	static 	int 	MenuSelCheck ()-
 *		checks the validity of user selections
 *	given:	1) the string containing the selection
 *	does:	go through each selection and check its validity, do
 *		translations if necessary.
 *	returns: 1 if all selections are valid, else return 0
 */

static	int	MenuSelCheck (str)
char	*str;

{
	int	i, num;
	int	tail_sel = 0;
	char	pstr[BUFSIZ], *item;
	char	fromstr[BUFSIZ], tostr[BUFSIZ];
	int	val, onespr, sproffset, cnt, itemlen, from, to;
	char	ans[BUFSIZ];
	

	/* parse the input string, check for illegal numbers/expressions */

	item = StringToken (str, TOKENSPR);
        for ( ; item != NULL; item = StringToken ((char *) NULL, TOKENSPR))
	{

		/* stop checking if selection does not start with a digit */
		if (!isdigit (item[0]))
		{
			(void) fprintf (stderr, "\n%s: invalid choice: %s. Press RETURN to restart the selection.\n", prog, item);
			(void) fflush (stderr);
			gets (ans);  
			return 0;
		}

		itemlen = strlen (item);   
		onespr = 0;

		/* check for existence of range seperator */
		for (i = 1; i < itemlen; i++)
		{
			if (!isdigit (item[i]))
				if (item[i] == RANGESPR)
				{
					/* parsed one RANGESPR already */
					if (onespr)
					{
						(void) fprintf (stderr, "\n%s: invalid range specification: %s. Press RETURN to restart the selection.\n", prog, item);
						(void) fflush (stderr);
						gets (ans);  
						return 0;
					}
					else 
						onespr = 1;
					sproffset = i;
				}
				else
				{
					(void) fprintf (stderr, "\n%s: invalid choice: %s. Press RETURN to restart the selection.\n", prog, item);
					(void) fflush (stderr);
					gets (ans);  
					return 0;
				}
		}

		/* if range specified */
		if (onespr)
		{
			/* no number following the RANGESPR */
			if (sproffset == (itemlen - 1))
			{
				(void) fprintf (stderr, "\n%s: invalid choice: %s. Press RETURN to restart the selection.\n", prog, item);
				(void) fflush (stderr); 
				gets (ans);  
				return 0;
			}

			/* get the two numbers specified in the range */
			/* Note: can not use strtok here, it will interrupt 
			   the outer for loop */
			(void) strncpy (fromstr, item, (size_t) sproffset);
			fromstr[sproffset]='\0';
			(void) strcpy (tostr, item + sproffset + 1);
			from = atoi (fromstr);
			to = atoi (tostr);

			if (debug)
			{
				(void) printf ("from = %d, to = %d\n", from, to);	
				(void) fflush (stdout);
			}
			if (to < from)
			{
				(void) fprintf (stderr, "\n%s: invalid range specification: %d-%d. Press RETURN to restart the selection.\n", prog, from, to);
				(void) fflush (stderr);
				gets (ans);  
				return 0;
			}

			/* store all the numbers in the range in sellist */
			cnt = to - from + 1;
			for (i = 0; i < cnt; i++)
				sellist[selcnt++] = from + i;
			
		}
		else
		{
			/* store the number in sellist */
			sellist[selcnt++] = atoi (item);
		}
	}
	
	/* further check validity of each selection */
	for (i = 0; i < selcnt; i++)
	{
		num = sellist[i];
		
		if (debug)
		{
			(void) printf ("sellist num = %d \n", num);
			(void) fflush (stdout);
		}
		
		if ((num > ext) || (num == 0))
		{
			(void) fprintf (stderr, "\n%s: %d is out of range. Press RETURN to restart the selection.\n", prog, num);
			(void) fflush (stderr);
			gets (ans);  
			return 0;
		}
		else if ((tail_sel) && (num >= all && num <= ext))
		{
			/* more than one tail options selected */
			(void) fprintf (stderr, "\n%s: Choices are ambiguous. Press RETURN to restart the selection.\n", prog);
			(void) fflush (stderr);
			gets (ans);  
			return 0;
		}
		/* check validity of combination of selections */
		else if ((num >= all) && (num <= ext))  
			tail_sel = 1;
	}

	/* if menu tail selection is made, translate that into action */
	if (tail_sel)
	{

		if (num == all)
		{
			opt_selected = 1;
			for (i = 0; i < optcnt; i++)
				optlist[i]->mark = SELECTED;
		}
		else if (num == mandonly)
		{
			opt_selected = 0;
			/* this check needs to be here in case we are
			   doing package selection when nothing == mandonly 
			   (no entry for "Nothing from this menu") */ 
		}
		else if (num == again)
		{
			/* whenever 'again' is selected, even if mixed
			   with other selections, we reset the selection */
			for (i = 0; i < optcnt; i++)
				optlist[i]->mark = UNSELECTED;
			return 0;
		}
		else if (num == nothing)
			exit (0);
		else if (num == ext)
		{
			for (;;)
			{
				(void) printf ("\nYou have chosen to exit without %sing any %ss.\n\nIs this correct (y/n): ", operation, object);
				(void) fflush (stdout);
				gets (pstr);
				if (*pstr == '\0')	
					/* a RETURN entered at the prompt */
					continue;
				sscanf (pstr, "%s", answer);
				switch (toupper (*answer))
				{
				case YES:
					exit (1);
				case NO:
					return 0;
				}
			}

		}
	}
 	else
	{
		opt_selected = 1;

		/* mark selected menu item */
		for (i = 0; i < selcnt; i++)
			optlist[sellist[i] - 1]->mark = SELECTED;
	}

	return 1;

}


/*	static void 	MenuSelect ()-
 *		prompt for a menu selection (including menu display	
 *	given:	global data  
 *	does:	display menu items in 1-column format and prompt for selection. 
 */

static void	MenuSelect () 

{
	char	str[BUFSIZ];
	int	i;

	for (;;)	
	{
		linecnt = 0;
		mode = DISPLAY;

		/* reset selection flags */
		if (selcnt > 0)
		{
			/* clear previous selected menu item */
			for (i = 0; i < optcnt; i++)
				optlist[i]->mark = UNSELECTED;
			selcnt = 0; 
			opt_selected = 0;
		}

		if (mandcnt > 0)
	 	{
			if ((optcnt == 0) && (object == PACKAGE))
			{
				(void) MenuLines (2, "\nThe following %ss will be %sed:\n", object, operation);

				(void) MenuDisplay (mandcnt, mandlist, STAR, ALL, MAND);

				(void) MenuLines (1, "\nDo you wish to continue? (y/n): ");

				for (;;)
				{
					gets (str);
					sscanf (str, "%s", answer);
					switch (toupper (*answer))
					{
					case YES:
						return;
					case NO:
						exit (1);
					}
				}
			}
			else
			{
				(void) MenuLines (2, "\n*** Enter %s selections ***\n",object); 
				if (in_isl)
					(void) MenuLines (3, "\nThe following %ss are mandatory and will be %sed by default:\n\n", object, operation);
				else
					(void) MenuLines (4, "\nThe following %ss are mandatory and will be %sed automatically\nunless you choose to exit without %sing any %ss:\n\n", object, operation, operation, object);

				(void) MenuDisplay (mandcnt, mandlist, STAR, ALL, MAND);
			}
		}

		/* there are optionals to choose from */
		if (optcnt > 0)
		{
			(void) MenuLines (7, "\nThe %ss listed below are optional:\n\n     There may be more optional %ss than can be presented on a single\n     screen. If this is the case, you can choose %ss screen by screen\n     or all at once on the last screen. All of the choices you make will\n     be collected for your confirmation before any %ss are %sed.\n", object, object, object, object, operation);

			if (!opt_cat)
				(void) MenuLines (1, "\n");

			mode = SELECT;
						
			if (!MenuDisplay (optcnt, optlist, RPAR, ALL, OPT))
				continue;
		}

		/* display tail portion of the menu, scenario dependent */
		if (!MenuTail()) 
			continue;
 
		mode = DISPLAY; /* reset back to DISPLAY */
		if (selcnt > 0)
		{
			(void) MenuLines (4, "\nAdd to your choices, choose an overriding action or\npress RETURN to confirm previous selections.\n\nChoices (for example, 1 2 4-6): ");
			MenuChoices(); /* prints selections so far */
		}
		else
		{
			(void) MenuLines (3, "\nEnter your choices or press RETURN to redisplay menus.\n\nChoices (for example, 1 2 4-6): ");
			fflush (stdout);
		}
	
		gets (str);

		if (*str == '\0')	/* a RETURN entered at the prompt */
			if (selcnt == 0)
				/* no selection yet, redisplay menu */
				continue;
			else
				/* valid selection exists */
				if (MenuConfirm())
					return;
				else
					continue;

		/* check validity of each selection */
		/* if input not valid or user does not confirm choice, 
		  start over again */
		if (MenuSelCheck (str) && MenuConfirm())
			return;
	}
} 


/*	static int	MenuTail ()-
 *		determine and display the tail portion of the menu.
 *	given:	global data  
 *	does:	depending on the scenario, display the menu tail.	
 *	return:	1 if tail portion successfully display, 0 if not
 */

static	int	MenuTail()

{
	int	cnt = 0;
	char	tail[BUFSIZ];		/* stores menu tail */
	char	temp[BUFSIZ]; 

	tail[0]='\0';
 
	if (selcnt > 0)
		(void) strcat (tail, "\nThe following choices override your previous selections:\n\n");
	else 
		if (optcnt > 0)
			(void) strcat (tail, "\nOr you may choose one of the following options:\n\n");
		else
			(void) strcat (tail, "\nYou may choose one of the following options:\n\n");
	cnt+=3;

	if ((mandcnt > 0) && (optcnt > 0))
	{
		
		(void) sprintf (temp, "   %3d) ALL mandatory and all optional %ss\n   %3d) MANDATORY %ss only \n", all, object, mandonly, object); 
		(void) strcat (tail, temp);
		cnt+=2;
	} 
	else
	{
		(void) sprintf (temp, "   %3d) ALL of the above \n", all);
		(void) strcat (tail, temp);
		cnt++; 
	}

	(void) sprintf (temp, "   %3d) CANCEL selections and redisplay menus\n", again);
	(void) strcat (tail, temp);
	cnt++; 

	if (cont_option)
	{
		(void) sprintf (temp, "   %3d) CONTINUE without %sing any %ss from this menu \n", nothing, operation, object); 
		(void) strcat (tail, temp);
		cnt++;
	}

	/* while in ISL, user can not exit without installing the mandatory */
	if (!in_isl)
	{
		(void) sprintf (temp, "   %3d) EXIT without %sing any %ss \n", ext,operation,object);
		(void) strcat (tail,temp); 
		cnt++;
	}

	if (!MenuLines (cnt, tail))
		return 0;

	return 1;
}


/*	static void 	MenuUsage ()-
 *		prints usage message on the screen.
 */

static void MenuUsage ()

{
	(void) fprintf(stderr,
	"\nUsage: %s -l|-x -p|-s [-m file] [-c] [-d] [-i] display|select list [list]\n", prog);
	(void) fflush (stderr);

	exit (1);
}





