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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: mkdir.c,v $ $Revision: 4.2.11.3 $ (DEC) $Date: 1993/10/11 17:26:07 $";
#endif
/*
 * HISTORY
 */
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: mkdir
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.5  com/cmd/files/mkdir.c, 9121320k, bos320 5/13/91 13:52:33";
 */

#include <signal.h>
#include <stdio.h>
#include <string.h>

#include <ctype.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <locale.h>

#include "mkdir_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_MKDIR,Num,Str)

/*  Defines for getmode() */
#define USER	05700
#define GROUP	02070
#define OTHER	00007
#define ALL	07777
#define READ	00444
#define WRITE	00222
#define EXECUTE	00111
#define SETID	06000
#define SETUID	04000
#define SETGID	02000
#define STICKY	01000
/* end of defines for getmode() */

int	status = 0;				/* return code from mkdir */

int	usflag;					/* used in conjunction with set user and group ids */

int		pflag = 0;		/* create path flag */
int		mflag = 0;		/* mode flag */

mode_t	defmode = S_IRWXU | S_IRWXG | S_IRWXO;	/* sets default mode to 777 */
mode_t	mask;					/* mode mask  */
mode_t	mode;					/* mode */
char	*modestr;				/* mode string */
char	*msptr;					/* used for advancing through modestring */

int madedir=0;                  /* ww-001 */
char	m_op();

extern 	char	*optarg;			/* getopt ptr to the flag's argument */
extern	int	optind; 			/* getopt:  argv index to next argument */

/*
 * NAME: mkdir [-p] [-m mode] Dir1 ... 
 * 
 * FUNCTION: makes the specified directories.
 *
 * NOTES:
 *	  -m mode 	Sets the file permission bits of the
 *			newly created directory to the specified mode.
 *			If the -m option is not specified, rwx is assumed.
 *
 *	  -p		Creates any missing intermediate directories 
 *			in the pathname.
 */

main(argc, argv)
char *argv[];
{
    int		c, i;

    (void) setlocale (LC_ALL,"");
    catd = catopen(MF_MKDIR,NL_CAT_LOCALE);

    /* ignore interrupts */
    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    while ((c=getopt(argc,argv,"pm:")) != EOF) {
	switch(c) {
	   case 'p':
		++pflag;
		break;
	   case 'm':
		if (mflag) {
		     (void)fprintf (stderr, MSGSTR(NOMULT, "mkdir: cannot have multiple -%c options.\n"), c);
		     exit(2);
		}
		++mflag;
		if (strchr("01234567", *optarg)) {
		    char *p;

		    i = strtoul(optarg, &p, 8);
		    if ( i<0 || i>07777 || *p )	/* bad range or extra chars */
			usage();
		}

		modestr = optarg;
		break;
	   default:
		usage();
	}
    } 


    if(argc == 1 || argc == optind) 	/* check for insufficient no. of arguments */
        usage();

 /* if -m was specified, call getmode to obtain a
  * translated mode for use in making the directory.
  */
    
   if (mflag) {
	int int_mode;
	int_mode = getmode();
	if (int_mode < 0) {
	     (void)fprintf(stderr,MSGSTR(BADMODE, "mkdir: incorrect syntax in mode argument\n"));
	     exit(2);
	}
	mode = (mode_t)int_mode;
   }
   else mode = defmode; 

 /* process the remaining arguments 
  * (considered to be the file names)
  */

   for (i = optind; i < argc; i++) {
	if (pflag) {
	/* Obtain the file mode creation mask by
	 * calling umask.  umask sets the file creation
         * mode to the specified parameter and returns 
         * the previous mask.  umask must then be called a
         * second time to reset the mask to its original value.
         * The file mode creation mask is needed because it may
         * affect file/directory creation modes
         */
   	    mask = umask(0);
            (void)umask(mask);

         madedir=0;             /* ww-001 */

	  /* Create intermediate path components */
	    if(mkdirp(argv[i]) < 0) {
		status = 2;
		continue;
	    }	
	}	/* end pflag */

        if(madedir) {                           /* ww-001 */
               chmod((char *)argv[i],mode);     /* ww-001 */
               break;                           /* ww-001 */
        }                                       /* ww-001 */

	/* Create target directory */
	if(Mkdir(argv[i],mode) < 0)
	    status = 2;
   }			/* end processing of file arguments */

    exit(status);

}

/*
 * NAME: usage
 *
 * FUNCTION: print usage message and exit.
 *
 */
usage()
{
	(void) fprintf (stderr, MSGSTR(USAGE, "Usage: mkdir [-p] [-m mode] Directory ... \n"));
	exit(1);
}

/*
 * NAME: Mkdir
 *
 * FUNCTION: Calls mkdir with a pathname and a mode. 
 *           Calls report to generate error messages.
 *
 * RETURN VALUES: -1 on error, 0 for successful
 */
int
Mkdir(d,m)
char *d;
mode_t m;
{

    if (mkdir(d,m) == 0)
    {
	if (mflag) 				/* chg 001 */
	{
	    if (chmod ((char *)d,m) == 0)
		return(0);
	    else 
	    {
		report(d);
		return(-1);
	    }
	}
	return(0);
    }
		       
    report(d);
    return(-1);
}

/*
 * NAME: mkdirp
 *
 * FUNCTION: creates intermediate path directories if the -p
 *           option is specified.  All directories are created
 *           using the default mode of 0777.
 *           If the umask prevents the user wx bits from being set, chmod is
 *           called to ensure that those mode bits are set so that following
 *           path directories can be created.  If any directory already
 *           exists, it is silently ignored.
 *
 * RETURN VALUES: -1 on error, 0 for successful
 */
int
mkdirp(dir)
char *dir;
{
    mode_t dmode = 0777;
    char *dirp;
    int	save_errno = errno;

    /*
     * Skip any leading '/' characters
     */

    for (dirp = dir; *dirp == '/'; dirp++)
	continue;

    /*
     * For each component of the path, make sure the component
     * exists.  If it doesn't exist, create it.
     */

    while ((dirp = strchr(dirp, '/')) != NULL) {
	*dirp = '\0';
	if (mkdir(dir, dmode) != 0 && errno != EEXIST) {
	     (void)fprintf(stderr, MSGSTR(NOMAKE, "Cannot create %s.\n"), dir);
	     perror(dir);
	     return (-1);
	}
	/*  If this directory did not already exist AND
	 *  the umask prevented the user wx bits from being set,
	 *  then chmod it to set at least u=wx so the next one can be
	 *  created.
	 */
	if((mask & 0300) && errno != EEXIST)
	    chmod(dir,(mode ^ mask) | 0300);

        if(*(dirp+1)=='\0')  madedir=1;         /* ww-001 */

	for (*dirp++ = '/'; *dirp == '/'; dirp++)
	    continue;
    }
    errno = save_errno;
    return(0);
}

/*
 * NAME: getmode
 *
 * FUNCTION: takes the modestring (supplied by -m mode) and the default mode
 *           and returns a mode suitable for use as the mode
 *           argument to mkdir().  getmode has to know the default
 *           because the '+' and '-' operators are interpreted
 *           relative to the default for the file being created.
 *
 * RETURNS:  integer mode or -1 if unable to decipher modestring
 *
 * NOTES:    Some examples of the current mode formats currently 
 *           supported in getmode:
 *            
 *	     	    ug+x,o-w
 *	            755 (octal)
 *                  =w
 *                  g+x-w	
 */
int
getmode()
{
	char *ms; 		/* ptr to modestring */
	mode_t um;
	int m,md,mode_who,mode_perm;
	char mode_op;

	um = ~mask;

	/* check for an octal mode */
	/* return if octal */
	m=intmode(modestr);		
	if (m >= 0) 
	   return (m);
	m=(int)defmode;

	/*  Separate parts of the modestring -- may be comma-separated */
	msptr = strtok(modestr,",");
	while (msptr && *msptr) {
		usflag=0;

		/*  Determine to whom this part applies  */
		mode_who=who();

		/*  Loop for formats such as o-w+x where multiple operators
                 *  for a given who exist.
		 */
		do {
	 	      /*  Get operator  */
		      if ((mode_op=m_op())=='\0') {
			   return(-1);
		      }

		      /*  get permissions */
		      mode_perm=perm();

		      if (!mode_who) {
			   switch(mode_op) {
				case '+':
					md=mode_perm&(um|SETID); break;
				case '-':
					md=mode_perm; break;
				case '=':
					md=mode_perm&um; break;
				default:
					return(-1);
			   }
		      } 
		      else md = mode_perm&(usflag|mode_who);

		      switch (mode_op) {
			   case '+':
				   m|=md;
				   break;
			   case '-':
				   m&=(~md);
				   break;
			   case '=':
				   m=(m&~mode_who)|md;
				   break;
		       }
		       if ((ms=strtok(NULL,",")) != NULL) {
			     msptr = ms;
			     break;
		       }	
		} while (*msptr != '\0');
	}
	return(m);	
}

/*
 * NAME: who
 *
 * FUNCTION: for symbolic mode, determines who mode is for.
 *
 * RETURNS:  the results from bitwise or operation
 *
 */
int
who()
{
	char *ms = msptr;
	int md=0;

	for (;*ms;ms++) {
		switch(*ms) {
			case 'u':
				usflag |= SETUID;
				md |= USER;
				msptr++;
				break;
			case 'g':
				usflag |= SETGID;
				md |= GROUP;
				msptr++;
				break;
			case 'o':
				md |= OTHER;
				msptr++;
				break;
			case 'a':
				md = ALL;
				msptr++;
				break;
			default:
				return(md);
		}
	}
	return(md);
}

/*
 * NAME: m_op
 *
 * FUNCTION: checks for an operator in the mode string. 
 *
 * RETURNS:  the operator is found, or null
 *
 */
char
m_op()
{
	char *ms = msptr;

	switch(*ms) {
		case '+':
		case '-':
		case '=':
			msptr++;
			return(*ms);
	}
	return('\0');
}

/*
 * NAME: perm
 *
 * FUNCTION: creates permissions mask from mode string. 
 *
 * RETURNS: mask
 *
 */
int
perm()
{
	char *ms = msptr;
	int md=0;

	for (;*ms;ms++) {
		switch(*ms) {
			case 'r':
				md |= READ;
				msptr++;
				break;
			case 'w':
				md |= WRITE;
				msptr++;
				break;
			case 'x':
				md |= EXECUTE;
				msptr++;
				break;
			case 's':
				md |= SETID;
				msptr++;
				break;
			case 't':
				md |= STICKY;
				msptr++;
				break;
			default:
				return(md);
		}
	}
	return(md);
}

/*
 * NAME: intmode
 *
 * FUNCTION: strip out mode if supplied in octal integer form. 
 *
 * RETURNS:  the octal integer mode or -1 
 *           if mode is not in octal integer form. 
 */
int
intmode(ms)
char *ms;
{
        register c, i, j;

        i = j = 0;
        while ((c = *ms++) >= '0' && c <= '7') {
                i = (i << 3) + (c - '0');
		j++;
	}

	return(j ? i : -1);
}

/*
 * NAME: report
 *
 * FUNCTION: generates an error message if the directory could 
 *           not be created
 */
int
report(d)
char *d;
{
	int save_errno = errno;

	/*
	 * For EACCES, ENOENT and ENOTDIR errors, print the name
	 * of the parent of the target directory instead of the
	 * target directory.
	 */
	if (errno == EACCES || errno == ENOENT || errno == ENOTDIR) {
		char *slash;

		if ((slash = strrchr(d, '/')) != NULL) {
			if (slash == d)
			      slash++;
			*slash = '\0';
		}
		else {
			d[0] = '.';
			d[1] = '\0';
		}

		(void)fprintf(stderr,
		    MSGSTR(NOACCESS, "mkdir: cannot access directory %s.\n"), d);
	}
	else
		(void)fprintf(stderr,
		    MSGSTR(NOMAKE, "mkdir: cannot create %s.\n"), d);

	errno = save_errno;
	perror(d);
}
