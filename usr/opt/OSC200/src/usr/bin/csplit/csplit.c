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
static char rcsid[] = "@(#)$RCSfile: csplit.c,v $ $Revision: 4.2.7.5 $ (DEC) $Date: 1993/10/11 16:36:24 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: csplit
 *
 *
 * (C) COPYRIGHT International Business Machines Corp.  1985, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.17  com/cmd/files/csplit.c, cmdfiles, bos320, 9132320b 7/30/91 10:41:43
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include <signal.h>
#include <sys/dir.h>
#include <regex.h>
#include <nl_types.h>
#include <sys/wait.h>
#include "csplit_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_CSPLIT,Num,Str)

#define LAST	0L
#define ERR	-1
#define FALSE	0
#define TRUE	1
#define EXPMODE	2
#define LINMODE	3
#define EXPSIZ	(128*5)
#define	LINSIZ	256
#define MAXFLS	99
#define _PATH_RM	"/sbin/rm"

	/* Globals */

char linbuf[LINSIZ];		/* Input line buffer */
regex_t regxp;  		/* compiled regular expression structure */
regex_t *regxpptr = &regxp;	/* pointer to regexp */
char file[PATH_MAX+1] = "xx";	/* File name buffer */
char csptmpfile[PATH_MAX+1];    /* stdin tmp file name buffer */
char *tmpdir;                   /* path set in ENV var TMPDIR */
char *targ;			/* Arg ptr for error messages */
FILE *infile, *outfile;		/* I/O file streams */
int silent, keep, create;	/* Flags: -s(ilent), -k(eep), (create) */
long offset;			/* Regular expression offset value */
long curline;			/* Current line in input file */
long suffix_size = 2;		/* number of suffix digits */
long max_files = MAXFLS;	/* maximum number of files that can be */
				/* created given the suffix's precision */

extern char *optarg;
extern int   optind;

int mb_cur_max, mbflag;
int stdin_flag = FALSE;
int regstat;
static void prntregerr();
char *getline();
extern char *getenv();

/*
 * NAME: csplit [-s] [-k] [-f prefix] [-n number] file arg1 [... argn]
 *                                                                    
 * FUNCTION: splits files by context
 *           -s         suppresses the output of file size messages
 *           -k         leaves created file segments intact in the event
 *                      of an error
 *           -f prefix  specifies the prefix name for the created
 *                      file segments  xx is the default prefix
 *	     -n number  specifies the decimal digits to form filenames for
 *			the file pieces. The default is 2
 */  
main(argc,argv)
int argc;
char **argv;
{
	int c, mode, sig(void);
	char *f;

	(void ) setlocale(LC_ALL,"");

	catd = catopen(MF_CSPLIT,NL_CAT_LOCALE);

	mb_cur_max = MB_CUR_MAX;
        mbflag = mb_cur_max > 1;

	while((c=getopt(argc, argv, "f:skn:")) != EOF) {
		int	i;

		switch(c) {
		case 'f':
			if (name_check(optarg) == ERR) {
			       fatal( MSGSTR(PRELONG,
				       "Prefix %s too long\n"),*argv);
			}
			strcpy(file, optarg);
			break;
		case 'n':
			if (natol(optarg, &suffix_size) == ERR) {
			    fatal( MSGSTR(BADSUFFIX,
				   "Illegal value for filename digits\n"),
				   *argv);			/*MSG*/
			}
			if (name_check(file) == ERR) {
			       fatal( MSGSTR(SUFLONG,
				   "Numerical suffix size %d too long\n"),
				   suffix_size);
			}
			for (i = 0, max_files = 1; i < suffix_size; i++) {
				max_files = 10 * max_files;
			}
			max_files--;
			break;
		case 's':
			silent++;
			break;
		case 'k':
			keep++;
			break;
		default:
			fatal(MSGSTR(USAGE,
			"Usage: csplit [-s] [-k] [-f prefix] [-n number] file args ...\n"),
			NULL); /*MSG*/
		}
	}
	
	argc -= optind;
	argv = &argv[optind];


	if(argc <= 1) {
		fatal(MSGSTR(USAGE,
			"Usage: csplit [-s] [-k] [-f prefix] [-n number] file args ...\n"),
			NULL); /*MSG*/
	}

	if (*argv[0] == '-') {
		infile = stdin;

                /* following code builds a tmp file from stdin            */

                stdin_flag = TRUE;
                create = TRUE;

                if ((tmpdir = getenv("TMPDIR")) == NULL)
                  tmpdir = "./";
                sprintf(csptmpfile, "%s/cspXXXXXX", tmpdir);
                mktemp(csptmpfile);
                if((outfile = fopen(csptmpfile,"w")) == NULL)
                  fatal(MSGSTR(CANTCREAT,"Cannot create %s\n"),
                               outfile); /*MSG*/
                while(getline(FALSE) != NULL)
                  flush();
                fclose(outfile);

                if((infile = fopen(csptmpfile,"r")) == NULL)
                  fatal(MSGSTR(CANTOPEN,"Cannot open %s\n"),csptmpfile); /*MSG*/

		argv++;
	}

	if(infile==(FILE *)NULL) {
		if((infile = fopen(*argv,"r")) == NULL)
			fatal(MSGSTR(CANTOPEN,"Cannot open %s\n"),*argv); /*MSG*/
		--argc; ++argv;
	}

	curline = 1L;
	signal(SIGINT,(void (*)(int))sig);

	/*
	*	The following for loop handles the different argument types.
	*	A switch is performed on the first character of the argument
	*	and each case calls the appropriate argument handling routine.
	*/

	for(; *argv; ++argv) {
		targ = *argv;
		switch(**argv) {
		case '/':
			mode = EXPMODE;
			create = TRUE;
			re_arg(*argv);
			break;
		case '%':
			mode = EXPMODE;
			create = FALSE;
			re_arg(*argv);
			break;
		case '{':
			num_arg(*argv,mode);
			mode = FALSE;
			break;
		default:
			mode = LINMODE;
			create = TRUE;
			line_arg(*argv);
			break;
		}
	}
	create = TRUE;
	to_line(LAST);
        exit_csplit(0);
}

/*
 * NAME: name_check
 *                                                                    
 * FUNCTION:  name_check does a size check on the input string.
 *            It returns ERR if either the filename or the full
 *	      pathname is too long, 0 if everything is OK.  This
 *	      routine exits with an error if pathconf fails for
 *	      any reason.  This routine always uses pathconf
 *	      to find the file system dependent maximum sizes
 *	      for the filename and pathname.
 */
name_check(name)
	char	*name;			/* full pathname for size check */
{
	char	*dir_name;		/* directory name, for pathconf */
	char	*file_name;		/* file name, for size check */
	long	name_max;		/* maximum filename size */
	long	path_max;		/* maximum pathname size */


	/* Parse the directory and file names.
	 */
	if ((dir_name = dirname(name)) == NULL) {
		dir_name  = ".";
		file_name = name;
	} else {
		file_name = basename(name);
	}

	/* Find out maximum filename size and pathname size
	 * for the target directory.
	 */
	name_max = pathconf(dir_name, _PC_NAME_MAX );
	if (name_max == -1) {
		perror("csplit");
		exit_csplit(1);
	}
	path_max = pathconf(dir_name, _PC_PATH_MAX );
	if (path_max == -1) {
		perror("csplit");
		exit_csplit(1);
	}

	/* Check the file name size.  Then restore last slash
	 * in name, if necessary, and check the path name size.
	 */
	if ((long)strlen(file_name) > (name_max - suffix_size))
		return(ERR);
	if ((long)strlen(name) > (path_max - suffix_size))
		return(ERR);
	return(TRUE);	/* not error */
}

/*
 * NAME: natol
 *                                                                    
 * FUNCTION:  natol takes an ascii argument (str) and converts it to a 
 *            long (plc).  It returns ERR if an illegal character.  
 *            The reason that atol does not return an answer (long) is 
 *            that any value for the long is legal, and this version of 
 *            atol detects error strings.
 */

natol(str,plc)
char *str;
long *plc;
{
	int f;
	*plc = 0;
	f = 0;
	for(;;str++) {
		switch(*str) {
		case ' ':
		case '\t':
			continue;
		case '-':
			f++;
		case '+':
			str++;
		}
		break;
	}
	for(; *str != '\0'; str++)
		if(*str >= '0' && *str <= '9')
			*plc = *plc * 10 + *str - '0';
		else
			return(ERR);
	if(f)
		*plc = -(*plc);
	return(TRUE);	/* not error */
}

/*
 * NAME: closefile
 *                                                                    
 * FUNCTION:
 *	Closefile prints the byte count of the file created, (via fseek
 *	and ftell), if the create flag is on and the silent flag is not on.
 *	If the create flag is on closefile then closes the file (fclose).
 */

closefile()
{
	if(!silent && create) {
		fseek(outfile,0L,2);
		fprintf(stdout,"%ld\n",ftell(outfile));
	}
	if(create)
		fclose(outfile);
}

/*
 * NAME: fatal
 *                                                                    
 * FUNCTION: 
 *	Fatal handles error messages and cleanup.
 *	Because "arg" can be the global file, and the cleanup processing
 *	uses the global file, the error message is printed first.  If the
 *	"keep" flag is not set, fatal unlinks all created files.  If the
 *	"keep" flag is set, fatal closes the current file (if there is one).
 *	Fatal exits with a value of 1.
 */

fatal(string,arg)
char *string, *arg;
{
	char *fls;
	int num;

	if ((string == (char *) 0) && (arg == (char *) 0))
		prntregerr(regstat, regxpptr);
	else
		fprintf(stderr,string,arg);

	if(!keep) {
		if(outfile) {
			fclose(outfile);
			fls = file + strlen(file);
			fls -= suffix_size;
			for(num=atoi(fls); num >= 0; num--) {
				sprintf(fls,"%0*d", suffix_size, num);
				unlink(file);
			}
		}
	} else
		if(outfile)
			closefile();
	exit_csplit(1);
}

/* NAME:  prntregerr
 * 
 * FUNCTION:
 *		Print the error message produced by regerror().
 */

static void
prntregerr(regstat, preg)
int regstat;
regex_t *preg;
{
	char *err_msg_buff;
	size_t sobuff; 		/* size of buffer needed */

	sobuff = regerror(regstat, preg, 0, 0);
	err_msg_buff = (char *) malloc(sizeof(char) *sobuff);
	sobuff = regerror(regstat, preg, err_msg_buff, sobuff);

	fprintf(stderr, "%s\n", err_msg_buff);
}

/*
 * NAME: findline
 *                                                                    
 * FUNCTION:
 *	Findline returns the line number referenced by the current argument.
 *	Its arguments are a pointer to the compiled regular expression (expr),
 *	and an offset (oset).  The variable lncnt is used to count the number
 *	of lines searched.  First the current stream location is saved via
 *	ftell(), and getline is called so that R.E. searching starts at the
 *	line after the previously referenced line.  The while loop checks
 *	that there are more lines (error if none), bumps the line count, and
 *	checks for the R.E. on each line.  If the R.E. matches on one of the
 *	lines the old stream location is restored, and the line number
 *	referenced by the R.E. and the offset is returned.
 */

long findline(expr,oset)
regex_t *expr;  		/* compiled regular expression structure */
long oset;
{
	static int benhere;
	long lncnt = 0, saveloc;
	char *getline();

	saveloc = ftell(infile);
	if(curline != 1L || benhere)		/* If first line, first time, */
		getline(FALSE);			/* then don't skip */
	else
		lncnt--;
	benhere = 1;
	while(getline(FALSE) != NULL) {
		lncnt++;
		if((regexec(expr, linbuf, (size_t) 0, (regmatch_t *) NULL, 0))==0) {
			fseek(infile,saveloc,0);
			return(curline+lncnt+oset);
		}
	}
	fseek(infile,saveloc,0);
	return(curline+lncnt+oset+2);
}

/*
 * NAME: flush
 *                                                                    
 * FUNCTION: 
 *	Flush uses fputs to put lines on the output file stream (outfile)
 *	Since fputs does its own buffering, flush doesn't need to.
 *	Flush does nothing if the create flag is not set.
 */

flush()
{
	if(create)
		fputs(linbuf,outfile);
}

/*
 * NAME: getfile
 *                                                                    
 * FUNCTION:
 *	Getfile does nothing if the create flag is not set.  If the
 *	create flag is set, getfile positions the file pointer (fptr) at
 *	the end of the file name prefix on the first call (fptr=0).
 *	Next the file counter (ctr) is tested for max_files, fatal if too
 *	many file creations are attempted.  Then the file counter is
 *	stored in the file name and incremented.  If the subsequent
 *	fopen fails, the file name is copied to tfile for the error
 *	message, the previous file name is restored for cleanup, and
 *	fatal is called.  If the fopen succecedes, the stream (opfil)
 *	is returned.
 */

FILE *getfile()
{
	static char *fptr;
	static int ctr;
	FILE *opfil;
	char tfile[PATH_MAX+1];

	if(create) {
		if(fptr == 0)
			fptr = file + strlen(file);
		if(ctr > max_files)
			fatal(MSGSTR(FILELIM,"%d file limit reached\n"),
				    max_files+1); /*MSG*/
		sprintf(fptr,"%0*d", suffix_size, ctr++);
		if((opfil = fopen(file,"w")) == NULL) {
			strcpy(tfile,file);
			sprintf(fptr,"%0*d", suffix_size, (ctr-2));
			fatal(MSGSTR(CANTCREAT,"Cannot create %s\n"),
				    tfile); /*MSG*/
		}
		return(opfil);
	}
	return(NULL);
}

/*
 * NAME: getline
 *                                                                    
 * FUNCTION:
 *	Getline gets a line via fgets from the input stream "infile".
 *	The line is put into linbuf and may not be larger than LINSIZ.
 *	If getline is called with a non-zero value, the current line
 *	is bumped, otherwise it is not (for R.E. searching).
 */

char *getline(bumpcur)
int bumpcur;
{
	char *ret;
	if(bumpcur)
		curline++;
	ret=fgets(linbuf,LINSIZ,infile);
	return(ret);
}

/*
 * NAME: line_arg
 *                                                                    
 * FUNCTION:
 *	Line_arg handles line number arguments.
 *	line_arg takes as its argument a pointer to a character string
 *	(assumed to be a line number).  If that character string can be
 *	converted to a number (long), to_line is called with that number,
 *	otherwise error.
 */

line_arg(line)
char *line;
{
	long to;

	if(natol(line,&to) == ERR)
		fatal(MSGSTR(BADLNUM,"%s: bad line number\n"),line); /*MSG*/
	to_line(to);
}

/*
 * NAME: num_arg
 *                                                                    
 * FUNCTION: 
 *	Num_arg handles repeat arguments.
 *	Num_arg copies the numeric argument to "rep" (error if number is
 *	larger than 11 characters or } is left off).  Num_arg then converts
 *	the number and checks for validity.  Next num_arg checks the mode
 *	of the previous argument, and applys the argument the correct number
 *	of times. If the mode is not set properly its an error.
 */

num_arg(arg,md)
char *arg;
int md;
{
	long repeat, toline;
	char rep[12];
	char *ptr;

	ptr = rep;
	for(++arg; *arg != '}'; arg++) {
		if(ptr == &rep[11])
			fatal(MSGSTR(RPT2LNG,"%s: Repeat count too large\n"),targ); /*MSG*/
		if(*arg == '\0')
			fatal(MSGSTR(MISSBRKT,"%s: missing '}'\n"),targ); /*MSG*/
		*ptr++ = *arg;
	}
	*ptr = '\0';
	if((natol(rep,&repeat) == ERR) || repeat < 0L)
		fatal(MSGSTR(ILLRPT,"Illegal repeat count: %s\n"),targ); /*MSG*/
	if(md == LINMODE) {
		toline = offset = curline;
		for(;repeat > 0L; repeat--) {
			toline += offset;
			to_line(toline);
		}
	} else	if(md == EXPMODE)
			for(;repeat > 0L; repeat--)
				to_line(findline(regxpptr,offset));
		else
			fatal(MSGSTR(NOOP,"No operation for %s\n"),targ); /*MSG*/
}

/*
 * NAME: re_arg
 *                                                                    
 * FUNCTION:
 *	Re_arg handles regular expression arguments.
 *	Re_arg takes a csplit regular expression argument.  It checks for
 *	delimiter balance, computes any offset, and compiles the regular
 *	expression.  Findline is called with the compiled expression and
 *	offset, and returns the corresponding line number, which is used
 *	as input to the to_line function.
 */

re_arg(string)
char *string;
{
	char *ptr;
	char ch;
	int l;
	
	ch = *string;

      if(mbflag)
      {
	ptr = string+1;
	while (*ptr != ch) {
		if(*ptr == '\\')
			++ptr;
		if ((l=mblen(ptr, mb_cur_max))>0) 
			ptr += l;
		else
			fatal(MSGSTR(ILLCHAR,"Illegal character in pattern.\n"),"");
		if(*ptr == '\0')
			fatal(MSGSTR(MISSDEL,"%s: missing delimiter\n"),targ);
		}
      }
      else 
      {
	ptr = string; 
	while(*(++ptr) != ch) {
		if(*ptr == '\\')
			++ptr;
		if(*ptr == '\0')
			fatal(MSGSTR(MISSDEL,"%s: missing delimiter\n"),targ);
		}
      }
	*ptr='\0';
	if(natol(ptr+1,&offset) == ERR)
		fatal(MSGSTR(ILLOFF,"%s: illegal offset\n"),string); /*MSG*/
	if ((regstat=regcomp(regxpptr, string+1, REG_NEWLINE)) != 0)
		fatal((char *) 0, (char *) 0);	/* will call prntregerr */
	*ptr = ch;
	to_line(findline(regxpptr,offset));
}

/*
 * NAME: sig
 *                                                                    
 * FUNCTION:
 *	Sig handles breaks.  When a break occurs the signal is reset,
 *	and fatal is called to clean up and print the argument which
 *	was being processed at the time the interrupt occured.
 */

sig(void)
{
	signal(SIGINT,(void (*)(int))sig);
	fatal(MSGSTR(INTSIG,"Interrupt - program aborted at arg '%s'\n"),targ);	/*MSG*/
}

/*
 * NAME: to_line
 *                                                                    
 * FUNCTION: 
 *	To_line creates split files.
 *	To_line gets as its argument the line which the current argument
 *	referenced.  To_line calls getfile for a new output stream, which
 *	does nothing if create is False.  If to_line's argument is not LAST
 *	it checks that the current line is not greater than its argument.
 *	While the current line is less than the desired line to_line gets
 *	lines and flushes (error if EOF is reached).
 *	If to_line's argument is LAST, it checks for more lines, and gets
 *	and flushes lines till the end of file.
 *	Finally, to_line calls closefile to close the output stream.
 */

to_line(ln)
long ln;
{
	outfile = getfile();
	if(ln != LAST) {
		if(curline > ln)
			fatal(MSGSTR(OUTRNG,"%s - out of range\n"),targ); /*MSG*/
		while(curline < ln) {
			if(getline(TRUE) == NULL)
				fatal(MSGSTR(OUTRNG,"%s - out of range\n"),targ); /*MSG*/
			flush();
		}
	} else		/* last file */
		if(getline(TRUE) != NULL) {
			flush();
			while(TRUE) {
				if(getline(TRUE) == NULL)
					break;
				flush();
			}
		} else
			fatal(MSGSTR(OUTRNG,"%s - out of range\n"),targ); /*MSG*/
	closefile();
}

/*
 * NAME: regerr
 *                                                                    
 * FUNCTION:  REGEXP ERR ROUTINE
 */

regerr(c)
int c;
{
fprintf(stderr,MSGSTR(BOGUSERR,"%d This is the error code\n"),c); /*MSG*/
fprintf(stderr,MSGSTR(ILLRE,"Illegal Regular Expression\n")); /*MSG*/
exit_csplit(1);
}

/*
 * NAME: exit_csplit
 *
 * FUNCTION:  EXIT CSPLIT AND CLEANUP
 */

exit_csplit(rc)
int rc;
{
    int pid, status = 0;

    if (stdin_flag) {                   /* rm file created for stdin */
        if (!(pid = fork())) {
            execl(_PATH_RM, "csplit", "-f", csptmpfile, 0);
            (void)fprintf(stderr, MSGSTR(NOEXEC,
                          "csplit: can't exec %s.\n"), _PATH_RM);
            rc = 1;
        }
        (void)waitpid(pid, &status, 0);
        if (WIFEXITED(status))
          rc = WEXITSTATUS(status);
    }
    exit(rc);
}
