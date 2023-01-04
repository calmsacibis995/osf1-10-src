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
static char *rcsid = "@(#)$RCSfile: main.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/29 22:05:20 $";
#endif

/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * Copyright (c) 1988, 1989 by Adam de Boor
 * Copyright (c) 1989 by Berkeley Softworks
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*-
 * main.c --
 *	The main file for this entire program. Exit routines etc
 *	reside here.
 *
 * Utility functions defined in this file:
 *	Error			Print a tagged error message. The global
 *				MAKE variable must have been defined. This
 *				takes a format string and two optional
 *				arguments for it.
 *
 *	Fatal			Print an error message and exit. Also takes
 *				a format string and two arguments.
 *
 *	Punt			Aborts all jobs and exits with a message. Also
 *				takes a format string and two arguments.
 */

#include <sys/param.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
/*#include <varargs.h>*/
#include <unistd.h>
#include <stdarg.h> 
#include <stdlib.h>
#include <locale.h>
#include <time.h>
#include "make.h"
#include "pathnames.h"
#include "pmake_msg.h"

nl_catd			catd;		/* Message catalog descriptor */
static Lst		create;		/* Targets to be made */
time_t			now;		/* Time at start of make */
GNode			*DEFAULT;	/* .DEFAULT node */
GNode			*SCCS_GET;	/* .SCCS_GET node ( jed under development */
Boolean			allPrecious;	/* .PRECIOUS given on line by itself */

static Boolean		noBuiltins;	/* -r flag */
static Lst		makefiles;	/* ordered list of makefiles to read */
Boolean			debug;		/* -d flag */
Boolean			noExecute;	/* -n flag */
Boolean			keepgoing;	/* -k flag */
Boolean			queryFlag;	/* -q flag */
int			queryExit;	/* -q exit value */
Boolean			touchFlag;	/* -t flag */
Boolean			ignoreErrors;	/* -i flag */
Boolean			beSilent;	/* -s flag */
Boolean                 checkEnvFirst;  /* -e flag */
Boolean			TrySccsGet;	/* internal .SCCS_GET FLAG */

static Boolean		ReadMakefile(char *, ...);
static Boolean		SccsReadMakefile(char *, char *);
static void		SccsGet(char *);

static void		usage(void);
static void		print_graph_2_header(void);

extern int optind, opterr;
extern char *optarg;

#ifdef DEBUG_FLAG
static void		debug_usage(void);
#endif

/*-
 * MainParseArgs --
 *	Parse a given argument vector.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Various global and local flags will be set depending on the flags
 *	given
 */
static void
MainParseArgs(int argc, char **argv)
{
	/* int argc; */
	/* char **argv; */

	char c;

	optind = 1;	/* since we're called more than once */
#ifdef DEBUG_FLAG
	while((c = getopt(argc, argv, "Sd:ef:iknpqrst")) != EOF) {
#else
	while((c = getopt(argc, argv, "Sef:iknpqrst")) != EOF) {
#endif
		switch(c) {
		case 'S':
			keepgoing = FALSE;
			Var_Append("MAKEFLAGS", "-S", VAR_GLOBAL);
			break;
#ifdef DEBUG_FLAG
		case 'd': {
			char *modules = optarg;

			for (; *modules; ++modules)
				switch (*modules) {
				case 'A':
					debug = ~0;
					printf("Debugging all modules. debug = 0x%x\n",debug);
					break;
				case 'a':
					debug |= DEBUG_ARCH;
					break;
				case 'd':
					debug |= DEBUG_DIR;
					printf("Debugging directories. debug = 0x%x\n",debug);
					break;
				case 'g':
					if (modules[1] == '1') {
						debug |= DEBUG_GRAPH1;
						++modules;
					}
					else if (modules[1] == '2') {
						debug |= DEBUG_GRAPH2;
						++modules;
					}
					break;
				case 'j':
					debug |= DEBUG_JOB;
					break;
				case 'm':
					debug |= DEBUG_MAKE;
					break;
				case 's':
					debug |= DEBUG_SUFF;
					break;
				case 't':
					debug |= DEBUG_TARG;
					break;
				case 'v':
					debug |= DEBUG_VAR;
					break;
				}
			Var_Append("MAKEFLAGS", "-d", VAR_GLOBAL);
			Var_Append("MAKEFLAGS", optarg, VAR_GLOBAL);
			break;
		}
#endif /* DEBUG_FLAG */
		case 'e':
			checkEnvFirst = TRUE;
			Var_Append("MAKEFLAGS", "-e", VAR_GLOBAL);
			break;
		case 'f':
			(void)Lst_AtEnd(makefiles, (ClientData)optarg);
			break;
		case 'i':
			ignoreErrors = TRUE;
			Var_Append("MAKEFLAGS", "-i", VAR_GLOBAL);
			break;
		case 'k':
			keepgoing = TRUE;
			Var_Append("MAKEFLAGS", "-k", VAR_GLOBAL);
			break;
		case 'n':
			allPrecious = TRUE;
			noExecute = TRUE;
			Var_Append("MAKEFLAGS", "-n", VAR_GLOBAL);
			break;
		case 'p':
			allPrecious = TRUE;
			debug |= (DEBUG_GRAPH1|DEBUG_GRAPH2);
			Var_Append("MAKEFLAGS", "-p", VAR_GLOBAL);
			break;
		case 'q':
			allPrecious = TRUE;
			noExecute = TRUE;
			queryFlag = TRUE;
			/* Kind of nonsensical, wot? */
			Var_Append("MAKEFLAGS", "-q", VAR_GLOBAL);
			break;
		case 'r':
			noBuiltins = TRUE;
			Var_Append("MAKEFLAGS", "-r", VAR_GLOBAL);
			break;
		case 's':
			beSilent = TRUE;
			Var_Append("MAKEFLAGS", "-s", VAR_GLOBAL);
			break;
		case 't':
			touchFlag = TRUE;
			Var_Append("MAKEFLAGS", "-t", VAR_GLOBAL);
			break;
		default:
		case '?':
#ifdef DEBUG_FLAG
		debug_usage();
#else
			usage();
#endif
	       
		}
	}

	/*
	 * See if the rest of the arguments are variable assignments and
	 * perform them if so. Else take them to be targets and stuff them
	 * on the end of the "create" list.
	 */
	for (argv += optind, argc -= optind; *argv; ++argv, --argc)
		if (Parse_IsVar(*argv))
			Parse_DoVar(*argv, VAR_CMD);
		else {
			if (!**argv)
				Punt(catgets(catd, MS_MAKE, ILLARG, "illegal (null) argument."));
			(void)Lst_AtEnd(create, (ClientData)*argv);
		}
}

/*-
 * MainParseArgLine --
 *  	Used by main() when reading the MAKEFLAGS envariable.
 *	Takes a line of arguments and breaks it into its
 * 	component words and passes those words and the number of them to the
 *	MainParseArgs function.
 *	The line should have all its leading whitespace removed.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Only those that come from the various arguments.
 */
static void
MainParseArgLine(char *prog, char *line)
  {
	/* char *prog; */
	/* char *line;			Line to fracture */

	char **argv;			/* Manufactured argument vector */
	int argc;			/* Number of arguments in argv */
	char *newline;			/* temp line for -		*/

	if (line == NULL)
		return;
	for (; *line == ' '; ++line);   /* Skip blank */
	if (!*line)
		return;

	/* POSIX/XPG4 states that MAKEFLAGS do not need a leading hyphen.                  */
        /* Because this implementation uses getopt to parse the options                    */
        /* later on down the road, it may be necessary to prepend a leading                */
        /* hyphen. But only if the first option is not a [macro=name] definition           */
 
	if (*line != '-') {		              /* The line may need  a '-' prefix   */
	  if (!Parse_IsVar(line))                     /* There could be a [macro=name]     */
	  {                                           /* If there is not, prepend a '-' to  */
	    newline = (char *)emalloc((strlen(line) + 1));    /* make room for the string          */
	    newline[0]='-';			      /* set the first parameter           */
	    strcat(newline+1,line);		      /* append past the first character   */
	  }	                                      /*                                   */
	  else                                        /* (!Parse_IsVar(line))              */
	    newline=strdup (line);      /* [macro=name] was found so do not add prefix     */
	}                                             /*                                   */
	else                                          /*                                   */
	  newline=strdup (line);                      /* it already had the prefix         */

	                              /* end of prepend hyphen "if needed" functionality.  */

	argv = Str_Break(prog, newline, &argc);              /* changed newline from line  */
	MainParseArgs(argc, argv);
}

/*-
 * main --
 *	The main function, for obvious reasons. Initializes variables
 *	and a few modules, then parses the arguments give it in the
 *	environment and on the command line. Reads the system makefile
 *	followed by either Makefile, makefile or the file given by the
 *	-f argument. Sets the MAKEFLAGS Make variable based on all the
 *	flags it has received by then uses either the Make or the Compat
 *	module to create the initial list of targets.
 *
 * Results:
 *	If -q was given, exits -1 if anything was out-of-date. Else it exits
 *	0.
 *
 * Side Effects:
 *	The program exits when done. Targets are created. etc. etc. etc.
 */
int
main(int argc, char **argv)
  {
	/* int argc; */
	/* char **argv; */

	Lst targs;	/* target nodes to create -- passed to Make_Init */
	struct stat sb;		/* Check fo SCCS directory */
	char *p;

	(void) setlocale(LC_ALL, "");	/* Initialize locale environment */
#ifdef ULTRIX_43
	catd = catopen(MF_PMAKE, 0);	/* Initialize message catalogs	 */
#else
	catd = catopen(MF_PMAKE, NL_CAT_LOCALE);	/*  Initialize message catalogs	 */
#endif
	create = Lst_Init(FALSE);
	makefiles = Lst_Init(FALSE);
	beSilent = FALSE;		/* Print commands as executed */
	ignoreErrors = FALSE;		/* Pay attention to non-zero returns */
	noExecute = FALSE;		/* Execute all commands */
	keepgoing = FALSE;		/* Stop on error */
	allPrecious = FALSE;		/* Remove targets when interrupted */
	queryFlag = FALSE;		/* This is not just a check-run */
	queryExit = 0;			/* Exit value if check-run */
	noBuiltins = FALSE;		/* Read the built-in rules */
	touchFlag = FALSE;		/* Actually update targets */
	debug = 0;			/* No debug verbosity, please. */

    
	/*
	 * Initialize the parsing, directory and variable modules to prepare
	 * for the reading of inclusion paths and variable settings on the
	 * command line
	 */
	Dir_Init();		/* Initialize directory structures so -I flags
				 * can be processed correctly */
	Parse_Init();		/* Need to initialize the paths of #include
				 * directories */
	Var_Init();		/* As well as the lists of variables for
				 * parsing arguments */


				/* Add SCCS dir to the directory search path list if it exists. */
	{
	  TrySccsGet=FALSE;
	  
	  if (lstat(_PATH_SCCS_DIR, &sb) == 0 && S_ISDIR(sb.st_mode) &&
	      lstat(_PATH_SCCS_DIR, &sb) == 0) {
	    dirSccsPath= Lst_Init(FALSE);
	    DirAddDir(dirSccsPath, _PATH_SCCS_DIR);
	    TrySccsGet=TRUE; 
	  }
	  else {
	    TrySccsGet=FALSE;
	  }
	}
         /*
	 * Initialize various variables.
	 *	MAKE also gets this name, for compatibility
	 *	MAKEFLAGS gets set to the empty string just in case.
	 *	MFLAGS also gets initialized empty, for compatibility.
	 */
	Var_Set("MAKEFLAGS", "", VAR_GLOBAL);
	Var_Set("SHELL", _PATH_SHELL, VAR_GLOBAL);

	/*
	 * First snag any flags out of the MAKEFLAGS environment variable.
	 */
	MainParseArgLine(argv[0], getenv("MAKEFLAGS"));
    
	MainParseArgs(argc, argv);

	/*
	 * Initialize archive, target and suffix modules in preparation for
	 * parsing the makefile(s)
	 */
	Arch_Init();
	Targ_Init();
	Suff_Init();

	DEFAULT = NILGNODE;
	SCCS_GET = NILGNODE;	/* Hold the node for the .SCCS_GET special target */

	(void)time(&now);

	/*
	 * Read in the built-in rules first, followed by the specified makefile,
	 * if it was (makefile != (char *) NULL), or the default Makefile and
	 * makefile, in that order, if it wasn't.
	 */


	 if (!noBuiltins && !ReadMakefile(_PATH_DEFSYSMK))
		Fatal(catgets(catd, MS_MAKE, NODEFAULT,
			"make: no default rules (%s)."), _PATH_DEFSYSMK);

	if (!Lst_IsEmpty(makefiles)) {
		LstNode ln;

		ln = Lst_Find(makefiles, (ClientData)NULL, (int (*)(void*,void*))ReadMakefile);
		if (ln != NILLNODE)
			Fatal(catgets(catd, MS_MAKE, MAKEOPENERR,
			      "make: cannot open %s."), (char *)Lst_Datum(ln));
	} else {
	  if (!ReadMakefile("makefile")) 
	    if (!ReadMakefile("Makefile"))
	      if (!SccsReadMakefile("-p . get ", "makefile"))	
		if (!SccsReadMakefile("-p SCCS get ", "makefile"))
		  if (!SccsReadMakefile("-p . get ", "Makefile"))   
		    (void)SccsReadMakefile("-p SCCS get ", "Makefile");
	}

	/* Install all the flags into the MAKE envariable. */
	if ((p = Var_Value("MAKEFLAGS", VAR_GLOBAL)) && *p)
		setenv("MAKEFLAGS", p, 1);

	/* print the initial graph, if the user requested it */

	if (DEBUG(GRAPH1))
	  {
	    printf(catgets(catd, MS_DEBUG, GRAPH01, "\n#**************************************************************\n"));
	      printf(catgets(catd, MS_DEBUG, GRAPH02, "#***** The information below contains macro and target ********\n")); 
              printf(catgets(catd, MS_DEBUG, GRAPH03, "#***** definitions after the default rules were read, *********\n"));
	      printf(catgets(catd, MS_DEBUG, GRAPH04, "#***** and before make processing has occured. ****************\n"));
 	      printf(catgets(catd, MS_DEBUG, GRAPH05, "#**************************************************************\n\n"));
	}
	if (DEBUG(GRAPH1))
		Targ_PrintGraph(1);

	/*
	 * Have now read the entire graph and need to make a list of targets
	 * to create. If none was given on the command line, we consult the
	 * parsing module to find the main target(s) to create.
	 */
	if (Lst_IsEmpty(create))
		targs = Parse_MainName();
	else
		targs = Targ_FindList(create, TARG_CREATE);

	/*
	 * Compat_Run will take care of creating all the targets as
	 * well as initializing the module.
	 */

#ifdef DEBUG_FLAG

	if (DEBUG(MAKE))
	  {
      	  printf("\n#**************************************************************\n");
            printf("#******************* Create all targets      ******************\n");
      	    printf("#**************************************************************\n\n");
	}

#endif				/* DEBUG_FLAG */

	Compat_Run(targs);
    
	/* print the graph now it's been processed if the user requested it */

	if (DEBUG(GRAPH2)) {
	  print_graph_2_header();
	  Targ_PrintGraph(2);
	}

	return(queryFlag?queryExit:0);
}

static void
print_graph_2_header(void)
{	
  printf(catgets(catd, MS_DEBUG, GRAPH01, "\n#**************************************************************\n"));
  printf(catgets(catd, MS_DEBUG, GRAPH02, "#***** The information below contains macro and target ********\n")); 
  printf(catgets(catd, MS_DEBUG, GRAPH06, "#***** definitions after all makefiles were read, and *********\n"));
  printf(catgets(catd, MS_DEBUG, GRAPH07, "#***** after all make processing has occured. *****************\n"));
  printf(catgets(catd, MS_DEBUG, GRAPH05, "#**************************************************************\n\n"));
}

/*-
 * ReadMakefile  --
 *	Open and parse the given makefile.
 *
 * Results:
 *	TRUE if ok. FALSE if couldn't open file.
 *
 * Side Effects:
 *	lots
 */
static Boolean
ReadMakefile(char *fname, ...)
  {
	/* char *fname;		makefile to read */

	FILE *stream;

	if (!strcmp(fname, "-")) {
		Parse_File("(stdin)", stdin);
	} else {
		if ((stream = fopen(fname, "r")) == NULL)
			return(FALSE);
		Parse_File(fname, stream);
		(void)fclose(stream);
	}
	return(TRUE);
}

/*-
 * SccsReadMakefile  --
 *	SCCS get, open, and parse the given makefile.
 *
 * Results:
 *	TRUE if ok. FALSE if couldn't open file.
 *
 *	function added by (jed) 
 *  
 */

static Boolean
SccsReadMakefile(char *sccsopts, char *fname)
{
  /* char *sccsopts;      sccs options*/ 
  /* char *fname;	 makefile to read */
 
  FILE *stream;
  char sccs_opts_fname[25];
  strcpy(sccs_opts_fname,sccsopts);
  strcat(sccs_opts_fname,fname);
  
  SccsGet(sccs_opts_fname);
  if ((stream = fopen(fname, "r")) == NULL)
    return(FALSE);
  
  /* 
   * Unlink now, so if Parse_File dies unexpectedly, the
   * temporarily fetched SCCS file will go away. 
   */
  unlink (fname);	
  
  Parse_File(fname, stream);
  (void)fclose(stream);
  
  return(TRUE);
}

/*-
 * SccsGet -
 *	SCCS get file.
 *
 * Results:
 *	File created in current directory upon success.
 *
 * Side Effects:
 *	
 */
static void
SccsGet(char *sccsoptsname)
  {
	int  cpid, stat, reason, fd;
	char **av;	/* Argument vector for thing to exec */

	av = Str_Break("sccs", sccsoptsname, (int *)NULL);

	cpid = fork();
	if (cpid < 0) {
		Fatal(catgets(catd, MS_MAKE, FORKFAIL, "Could not fork"));
	}
	
	if (cpid == 0) { /* child: fetch makefile */

		/* force sccs command's outputs to /dev/null */
		fd = open(_PATH_DEV_NULL, O_WRONLY);
		dup2(fd, 1); dup2(fd, 2);

		/* try using user's current path */
	    
		execvp("sccs", av);

		/* user's path failed, try using hard coded path */
		
		execv(_PATH_SCCS, av);
		
		exit(1);
	}

	/* parent: wait for child to complete */
	stat = waitpid(cpid, &reason, 0); 
	
	if (stat == -1) {
	    Fatal (catgets(catd, MS_MAKE, WAITERR, "error in wait: %d"), stat);
	}
}

/*-
 * Error --
 *	Print an error message given its format.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The message is printed.
 */

/* VARARGS */

void
Error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fprintf(stderr, "\n");
	(void)fflush(stderr);
}

/*-
 * Fatal --
 *	Produce a Fatal error message. If jobs are running, waits for them
 *	to finish.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	The program exits
 */

/* VARARGS */
void
Fatal(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fprintf(stderr, "\n");
	(void)fflush(stderr);

	if (DEBUG(GRAPH2)) {
	  print_graph_2_header();
	  Targ_PrintGraph(2);
	}
	exit(2);		/* Not 1 so -q can distinguish error */
}

/*
 * Punt --
 *	Major exception once jobs are being created. Kills all jobs, prints
 *	a message and exits.
 *
 * Results:
 *	None 
 *
 * Side Effects:
 *	All children are killed indiscriminately and the program Lib_Exits
 */

/* VARARGS */
void
Punt(const char *fmt, ...)
{
	va_list ap;

	(void)fprintf(stderr, catgets(catd, MS_MAKE, MAKE1, "make: "));
	va_start(ap, fmt);
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fprintf(stderr, "\n");
	(void)fflush(stderr);

	if (DEBUG(GRAPH2)) {
	  print_graph_2_header();
	  Targ_PrintGraph(2);
	}

	exit(2);		/* Not 1, so -q can distinguish error */
}

/*
 * emalloc --
 *	malloc, but die on error.
 */
void *
emalloc(size_t len)
{
	void *p;

	if (!(p = malloc(len)))
		enomem();
	return(p);
}

#ifdef ULTRIX_43
/*
 * strdup --
 *	only needed for ultrix 4.3.
 */

char *
strdup(char *s1)
{
	char *tmp;

	if (!(tmp = malloc(strlen(s1)+1)))
		enomem();
	strcpy(tmp,s1);
	return(tmp);
}
#endif

/*
 * enomem --
 *	die when out of memory.
 */
void
enomem(void)
{
	(void)fprintf(stderr, catgets(catd, MS_MAKE, MAKE2, "make: %s.\n"),
		      strerror(errno));
	exit(2);
}

/*
 * usage --
 *	exit with usage message
 */
static void
usage(void)
{
	(void)fprintf(stderr, catgets(catd, MS_MAKE, USAGE,
		      "usage: make [-einpqrst] [-f makefile ] [-k | -S] [macros=name]...  [target_name]... \n"));
	exit(2);
}


#ifdef DEBUG_FLAG

/*
 * debug_usage --
 *	exit with usage message
 */

static void
debug_usage(void)
{
	(void)fprintf(stderr,
"debugusage: make -d option [-eiknqrst] [-f makefile ] [variable=value ...] [target ...]\n");
	(void)fprintf(stderr,
"where option = \n  \
\t'A': ALL\n \
\t'a': DEBUG_ARCH\n \
\t'd': DEBUG_DIR\n \
\t'g': DEBUG_GRAPH1 DEBUG_GRAPH2\n \
\t'j': DEBUG_JOB\n \
\t'm': DEBUG_MAKE\n \
\t's': DEBUG_SUFF\n \
\t't': DEBUG_TARG\n \
\t'v': DEBUG_VAR\n");
	exit(2);
}

#endif /* DEBUG_FLAG */
