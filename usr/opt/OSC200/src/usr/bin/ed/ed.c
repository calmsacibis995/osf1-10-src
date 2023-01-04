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
static char rcsid[] = "@(#)$RCSfile: ed.c,v $ $Revision: 4.3.11.12 $ (DEC) $Date: 1994/01/14 21:47:11 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDEDIT) ed.c
 *
 * FUNCTIONS: main, address, append, blkio, chktime, clear, commands, compsub,
 * crblock, crinit, delete, dosub, eclose, eopen, error, error1, execute,
 * exfile, expnd, filecopy, filename, fspec, gdelete, getblock, getchr,
 * getcopy, getfile, getime, getkey, getline, getsub, gettty, global, globaln,
 * init, join, lenchk, makekey, move, newline, newtime, nonzero, numb,
 * onhup, onintr, onpipe, place, putchr, putd, putfile, putline, puts, quit,
 * rdelete, red, reverse, save, setall, setdot, setnoaddr, stdtab, strcopy,
 * strequal, substitute, targ, tincr, tlist, tstd, undo, unixc, re_compile,
 * re_error
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1984, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *      Unpublished work.
 *      (c) Copyright INTERACTIVE Systems Corp. 1983, 1985
 *      Licensed Material - Program property of INTERACTIVE Systems Corp.
 *      All rights reserved.
 *
 *      RESTRICTED RIGHTS
 *      These programs are supplied under a license.  They may be used or
 *      copied only as permitted under such license agreement.  Any
 *      Authorized copy must contain the above notice and this restricted
 *      rights notice.  Disclosure of the programs is strictly prohibited
 *      unless otherwise provided in the license agreement.
 *
 *      Encryption used to be done by recognizing that text had high-order
 *      bits on.  Now that can't be the method; not clear how to solve this.
 *
 *      Present version does not support encryption.
 * 
 *  1.31 com/cmd/edit/ed.c, cmdedit, bos320, 9146320d 9/27/91 12:23:20
 */
#undef CRYPT
/*
** Editor
*/

#include <locale.h>
#include "pathnames.h"
#include "ed_msg.h"
nl_catd catd;

char    *msgtab[] =
{
    "Cannot create a pipe.",
    "Use the w subcommand to write and save the file.",
    "The mark name must be a lower case alphabetic character.",
    "Cannot find or open the input file.",
    "A tab specification is not formatted correctly.",
    "There is no subcommand to undo.",
    "Cannot run the specified subcommand with a restricted shell.",
    "Cannot create the output file.",
    "Cannot write to the output file due to lack of filesystem space.",
    "Cannot find or open the output file.",
    "Cannot link to the output file.",
    "Specify a range end point that is less than 256.",
    "The specified subcommand does not exist.",
    "The search string is not found.",
    "-",
    "The specified line does not exist.",
    "The character or characters between \\{ and \\} must be numeric.",
    "The first address cannot exceed the second in an address pair.",
    "The specified subcommand does not require an address.",
    "The global subcommand requires a pattern.",
    "The subcommand contains added characters that are not valid.",
    "The specified subcommand requires a file name.",
    "Use a space between the subcommand and the file name.",
    "Cannot create a process at this time.",
    "File names cannot be longer than %i characters.",
    "Specify \\digit between 1 and 9 not greater than number of subpatterns.",
    "An interrupt signal was received. Returning to command mode.",
    "The editor ignores lines longer than 512 characters.",
    "The input file contains characters which are not valid.",
    "Cannot write to the specified file.",
    "There is not enough memory available for the append.",
    "The temporary file buffer cannot contain more than 6144 blocks(512 bytes).",
    "Cannot read to or write from the temporary file buffer.",
    "Cannot use multiple global subcommands.",
    "The global subcommand list cannot exceed 256 characters.",
    "There is no matching pattern.",
    "A delimiter is not correct or is missing.",
    "-",
    "The replacement string cannot be longer than 512 characters.",
    "Illegal move destination.", 
    "-",
    "There is no remembered search string.",
    "There is a missing \\( or \\).",
    "Specify \\( no more than 9 times.",
    "Specify no more than 2 numbers between \\{ and \\}.",
    "An opening \\{ must have a closing \\}.",
    "The first number cannot exceed the second between \\{ and \\}.",
    "The substitute is not complete.",
    "Newline was not expected.",
    "There is a missing [ or ].",
    "The regular expression is too large.",
    "There is a regular expression error.\n",
    "The G and V subcommands require a subcommand.",
    "The a, i, or c subcommands cannot be used with G and V subcommands.",
    "An end-of-line character was expected.",
    "There is no remembered replacement string.",
    "There is no remembered subcommand.",
    "File names cannot begin with a ! character.",
    "A simultaneous edit is being performed on the file.",
    "The specified command cannot be used when the -y flag is set.",
    "The X subcommand replaced the x subcommand.",
    "Warning: write to file may destroy it ( `illegal char' read earlier ).", 
    "Warning: Quitting may cause the loss of data.",
    "File not written. It was truncated. Try write to another file.",
    0
};

int     peekc;
int     getchr(void);
void    error1(int);

/*
 * Define some macros for the regular expression
 * routines to use for input and error stuff.
 */

#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <regex.h>
#include <stdio.h>
#include <sys/file.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <termio.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <setjmp.h>

/* macro for message catalog access */
#define MSGSTR(num, str)	catgets(catd, MS_ED, num, str)
#define PUTM(num) do{ if (num >= 0) \
		puts(MSGSTR(num+1, msgtab[num]));} while(0) \


#define MAXLINES 65512
#define FNSIZE PATH_MAX+1
#define LBSIZE  512
#define GBSIZE  256
#define GLOBAL_SUB -1	/* global substitution	*/
#define KSIZE   9

#define READ    0
#define WRITE   1

#define PRNT    02

int     Xqt = 0;
int     lastc;
char    savedfile[FNSIZE+1];
char    file[FNSIZE+1];
char    origfile[FNSIZE+1];	/* holds name of originally edited file */
char    funny[LBSIZE];
char    *tempfile;              /* holds full name of temporary file */
unsigned char    linebuf[LBSIZE];

char   	expbuf[LBSIZE];		/* Buffer for input regular expression	*/
regex_t		preg;		/* Holds compiled regular expression	*/
regmatch_t 	pmatch[_REG_SUBEXP_MAX + 1];	/* Holds substring offsets for subexpressions	*/

static char    rhsbuf[LBSIZE];
struct  lin     {
	int cur;
	int sav;
};
typedef struct lin *LINE;

LINE    address(void);
unsigned char    *getline(int);
unsigned char    *getblock(int, int);
unsigned char    *place(register unsigned char *, register unsigned char *, register unsigned char *);

struct  stat Fl, Tf;
int     Short = 0;		/* Flag that we were in midst of write to full FS */
int     oldmask;		/* No umask while writing */

jmp_buf savej;
#ifdef  NULLS
int     nulls;  /* Null count */
#endif

struct  Fspec   {
	char    Ftabs[22];
	char    Fdel;
	unsigned char   Flim;
	char    Fmov;
	char    Ffill;
};
struct  Fspec   fss;

int     errcnt=0;



int     maxlines;
LINE    zero;
LINE    dot;
LINE    dol;
LINE    endcore;
LINE    fendcore;
LINE    addr1;
LINE    addr2;
LINE    savdol, savdot;
int     globflg;
int     initflg;
unsigned char    genbuf[LBSIZE];
long    count;
unsigned char    *nextip;
unsigned char    *linebp;
int     ninbuf;
int     io;
void    (*oldhup)(int);
void    (*oldquit)(int), (*oldpipe)(int);
int     vflag = 1;

#define YWCOUNT	1	/* Indicates writing counted strings */
#define YSEEN	2	/* Indicates that -y seen on cmd line */
int     yflag;		/* Possible states: 0,2,3 */

#ifdef TRACE
char	*tracefile;
FILE	*trace;
#endif	
#ifdef CRYPT
int     xflag;
int     xtflag;
int     kflag;
#endif
int     hflag;
int     xcode = -1;
int	nowrite=0;
char    key[KSIZE + 1];
char    crbuf[512];
char    perm[768];
char    tperm[768];
int     col;
char    *globp;
int     tfile = -1;
int     tline;
char    *tfname;
char	tempfilename[L_tmpnam];
unsigned char    ibuff[512];
int     iblock = -1;
unsigned char    obuff[512];
int     oblock = -1;
int     ichanged;
int     nleft;
int     names[26];
int     anymarks;
int     subnewa;
int     fchange;
int     nline;
int     fflg, shflg;
/* Define the length of the prompt string */
#define	PRLEN	16
char    prompt[PRLEN+1] = "*";
int     rflg;
int     readflg;
int     eflg;
int     ncflg;
int     listn;
int     listf;
int     pflag;
int     flag28 = 0; /* Prevents write after a partial read */
int     save28 = 0; /* Flag whether buffer empty at start of read */
long    savtime;
char    *name = "SHELL";
char	*rshell1 = "/bin/rsh";
char	*rshell2 = "/bin/Rsh";
char	*rshell3 = "/usr/bin/rsh";
char	*rshell4 = "/usr/bin/Rsh";
char	*rshell5 = "/sbin/Rsh";
char    *val;
char    *home;


int	scommand;	/* Indicates when a substitute command is in progress	*/
char	*offset_base;	/* Stores the current base for Regular Expression offsets */
char 	*expend;	/* Stores the end element position in the expression buffer */

typedef ssize_t (*blkiof)(int, ...);

void    init(void);
void    commands(void);
void    quit(void);  
void    setdot(void);
void    newline(void);
void    save(void);
void    append(int (*)(void), LINE);
void    delete(void);
void    setnoaddr(void);
int     error(register);
void    filename(register);
void    setall(void);
void    nonzero(void);
void    exfile(void);
int     gettty(void);
int     getfile(void);
void    putfile(void);
void    unixcom(void);
void    rdelete(LINE, LINE);
void    gdelete(void);
int     putline(void);
void 	blkio(int, char *, blkiof);
void    global(int);
void    join(void);
void    substitute(int);
int     compsub(void);
int     getsub(void);
void    dosub(void);
void    move(int);
void    reverse(register LINE, register LINE);
int     getcopy(void);
int     execute(int, LINE);
void    putd(long int);
int     puts(const char *);
void    putchr(int);
void	putwchr(wchar_t);
void    crblock(char *, char *, int *, long *);
void    getkey(void);
int     crinit(char *, char *);
void    makekey(char *, char *);
void    globaln(int);
int     eopen(char *, int);
void    eclose(int);
void    getime(void);
void    chktime(void);
void    newtime(void);
void    red(char *);
int     fspec(char *, struct Fspec *, int);
int     numb(void);
void    targ(struct Fspec *);
void    tincr(int, struct Fspec *);
void    tstd(struct Fspec *);
void    tlist(struct Fspec *);
int     expnd(char *, char *,int *, struct Fspec *);
void    clear(struct Fspec *);
int     lenchk(char *, struct Fspec *);
int     stdtab(char *, char *);
void    undo(void);
void	re_compile(wchar_t);
void	re_error(int);

void 	onintr(int);
void	onhup(int);

void	onpipe(int dummy)
{
	(void) error(0);
}

int
main(int argc, char **argv)
{
	register        char *p1, *p2;

	void     (*oldintr)(int);
	int	n, len;
	int	flag;

	(void) setlocale(LC_ALL,"");		/* required by NLS environment tests */
	catd = catopen(MF_ED,NL_CAT_LOCALE);

#ifdef  STANDALONE
	if (argv[0][0] == '\0')
		argc = getargv("ed",&argv,0);
#endif

	oldquit = signal(SIGQUIT, SIG_IGN);
	oldhup = signal(SIGHUP, SIG_IGN);
	oldintr = signal(SIGINT, SIG_IGN);
	oldpipe = signal(SIGPIPE, onpipe);
	(void) signal(SIGTERM, SIG_IGN);

	expend = &expbuf[LBSIZE];
	p1 = *argv;
	while(*p1++);
	while(--p1 >= *argv)
		if(*p1 == '/')
			break;
	*argv = p1 + 1;
	/* if SHELL set in environment and is either rsh or Rsh, set rflg */
	if((val = getenv(name)) != NULL)
		if ((strcmp(val, rshell1) == 0) || 
		    (strcmp(val, rshell2) == 0) ||
		    (strcmp(val, rshell3) == 0) ||
		    (strcmp(val, rshell4) == 0) ||
		    (strcmp(val, rshell5) == 0))
			rflg++;
	if (**argv == 'r')
		rflg++;
	home = getenv("HOME");

	/*
	 * Since getopt() is going to try to interpret "-" as a flag indicating
	 * that there are no more options, we pre-read the arglist and change
	 * "-" to it's POSIX equivalent "-s"
	 */
	for (n=0; n<argc; n++)
	    if ( !strcmp(argv[n], "-") )
		argv[n] = "-s";

	opterr = 0;		/* we will print our own errors */
	while ((flag = getopt(argc, argv, "sp:qy")) != -1) {
		switch(flag) {

		case '?':
		    	switch (optopt) {
			  case 'p':
			    fprintf(stderr, MSGSTR(M_EDPARG,
				    "The -p flag requires a parameter.\n"));
			    exit(2);
#ifdef CRYPT
			  case 'x':
			    xflag = 1;
			    break;
#endif
#ifdef TRACE
			  case 'T':
			    tracefile = "trace";
			    trace = fopen(tracefile, "w");
			    break;
#endif
			  default:
			    fprintf(stderr, MSGSTR(M_USAGE, "USAGE: ed [-p string] [-s] [file]\n"));
			    exit(2);
			}
			break;

		case 's':
			vflag = 0;
			break;

 		case 'p':
			(void) strncpy(prompt, optarg, PRLEN);
			/* Ensure that the prompt string has not been 	*/
			/* truncated in the middle of a multi-byte character */
			if (strlen(prompt) >= PRLEN) {
			    n = PRLEN - 1;
			    while (((len = mblen(&prompt[n], MB_CUR_MAX)) == -1) || (n + len > PRLEN)) 
			      n--;
			    prompt[n+len] = '\0';
			}
			shflg = 1;
			break;

		case 'q':
			signal(SIGQUIT, SIG_DFL);
			vflag = 1;
			break;

		case 'y':
			yflag = YWCOUNT|YSEEN;
			break;
		
		}
	}
#ifdef CRYPT
	if(xflag){
		getkey();
		kflag = crinit(key, perm);
	}
#endif

	argc -= optind;
	argv += optind;

	if (argc>0) {
		p1 = *argv;
		if(strlen(p1) > FNSIZE) {
		    	fprintf(stderr, MSGSTR(M_EDFILE
					       ,"A file name cannot exceed %i characters in length.")
				,FNSIZE);
			/* close the message catalog */
			catclose(catd);
			exit(2);
		}
		p2 = savedfile;
		while (*p2++ = *p1++);
		globp = "r";
		fflg++;
	}
	else    /* editing with no file so set savtime to 0 */
		savtime = 0;
	eflg++;
	maxlines = MAXLINES;
	while (!(fendcore = (LINE )malloc(maxlines*sizeof(struct lin))))
		maxlines-=1024;
        memset(fendcore,NULL,(maxlines*sizeof(struct lin)));
	tfname = tmpnam(tempfilename);
	init();

	if ( oldintr == SIG_DFL )
		(void) signal(SIGINT, onintr);
	if ( oldhup == SIG_DFL )
		(void) signal(SIGHUP, onhup);
	preg.re_comp = 0;
	(void) setjmp(savej);
	commands();
	quit();
        return(0);
}

/* filecopy: copy file ifp to file ofp */
void filecopy(FILE *ifp, FILE *ofp)
{
	int c;
#ifdef TRACE
	if (trace)
		fprintf(trace, "in filecopy\n");
#endif

	while ((c = getc(ifp)) != EOF)
		putc(c,ofp);
}

void commands(void)
{
    struct termio	tty;
    register LINE a1;
    register c;
    register char *p1, *p2;
    struct statfs sb;
    int n;
    FILE *outfi;
    FILE *tmpfi;
    char *tmpdir;                   /* path set in ENV var TMPDIR */

    tmpdir = getenv("TMPDIR");

    
    for (;;) {
	scommand = 0;	/* Initialises and clears this flag	*/
	if ( pflag ) {
	    pflag = 0;
	    addr1 = addr2 = dot;
	    goto print;
	}
	if (shflg && globp==0 && (ioctl(0, TCGETA, &tty) != -1) )
	  write(STDOUT_FILENO, prompt, strlen(prompt));
	addr1 = 0;
	addr2 = 0;
	if((c=getchr()) == ',') {
	    addr1 = zero + 1;
	    addr2 = dol;
	    c = getchr();
	    goto swch;
	} else if(c == ';') {
	    addr1 = dot;
	    addr2 = dol;
	    c = getchr();
	    goto swch;
	} else
	  peekc = c;

	do {
	    addr1 = addr2;
	    if ((a1 = address())==0) {
		c = getchr();
		break;
	    }
	    addr2 = a1;
	    if ((c=getchr()) == ';') {
		c = ',';
		dot = a1;
	    }
	} while (c==',');

	if (addr1==0)
	  addr1 = addr2;
swch:
	switch(c) {
	    
	  case 'a':
	    setdot();
	    newline();
	    if (!globflg) save();
	    append(gettty, addr2);
	    continue;
	    
	  case 'c':
	    delete();
	    append(gettty, addr1-1);
	    continue;
	    
	  case 'd':
	    delete();
	    continue;
	    
	  case 'E':
	    fchange = 0;
	    c = 'e';
	  case 'e':
	    fflg++;
	    setnoaddr();
	    if (vflag && fchange) {
		fchange = 0;
		(void) error(1);
	    }
	    filename(c);
	    eflg++;
	    init();
	    addr2 = zero;
	    goto caseread;
	    
	  case 'f':
	    setnoaddr();
	    filename(c);
	    if (!ncflg)  /* there is a filename */
	      getime();
	    else
	      ncflg--;
	    puts(savedfile);
	    continue;
	    
	  case 'g':
	    global(1);
	    continue;
	  case 'G':
	    globaln(1);
	    continue;
	    
	  case 'h':
	    newline();
	    setnoaddr();
	    PUTM(xcode);
	    continue;
	    
	  case 'H':
	    newline();
	    setnoaddr();
	    if(!hflag) {
		hflag = 1;
		PUTM(xcode);
	    }
	    else
	      hflag = 0;
	    continue;
	    
	  case 'i':
	    setdot();
	    nonzero();
	    newline();
	    if (!globflg) save();
	    append(gettty, addr2-1);
	    if (dot == addr2-1)
	      dot += 1;
	    continue;
	    
	    
	  case 'j':
	    if (addr2==0) {
		addr1 = dot;
		addr2 = dot+1;
	    }
	    setdot();
	    newline();
	    nonzero();
	    if (!globflg) save();
	    join();
	    continue;
	    
	  case 'k':
	    if ((c = getchr()) < 'a' || c > 'z')	/* Must be from portable set */
	      (void) error(2);
	    newline();
	    setdot();
	    nonzero();
	    names[c-'a'] = addr2->cur & ~01;
	    anymarks |= 01;
	    continue;
	    
	  case 'm':
	    move(0);
	    continue;
	    
	  case '\n':
	    if (addr2==0)
	      addr2 = dot+1;
	    addr1 = addr2;
	    goto print;
	    
	  case 'n':
	    listn++;
	    newline();
	    goto print;
	    
	  case 'l':
	    listf++;
	  case 'p':
	    newline();
	  print:
	    setdot();
	    nonzero();
	    a1 = addr1;
	    do {
		if (listn) {
		    count = a1 - zero;
		    putd(count);
		    putchr('\t');
		}
		puts((char *)getline((a1++)->cur));
	    }
	    while (a1 <= addr2);
	    dot = addr2;
	    pflag = 0;
	    listn = 0;
	    listf = 0;
	    continue;
	    
	  case 'Q':
	    fchange = 0;
	  case 'q':
	    setnoaddr();
	    newline();
	    quit();
	    
	  case 'r':
	    filename(c);
	  caseread:
	    readflg = 1;
	    save28 = (dol != fendcore);
	    if ((io = eopen(file, 0)) < 0) {
		lastc = '\n';
		/* if first entering editor and file does not exist */
		/* set saved access time to 0 */
		if (eflg) {
		    savtime = 0;
		    eflg  = 0;
		}
		(void) error(3);
	    }
	    /* get last mod time of file */
	    /* eflg - entered editor with ed or e  */
	    if (eflg) {
		eflg = 0;
		getime();
		strcpy(origfile,file);
	    }
	    setall();
	    ninbuf = 0;
	    n = zero != dol;
#ifdef NULLS
	    nulls = 0;
#endif
	    if (!globflg && (c == 'r')) save();
	    append(getfile, addr2);
	    exfile();
	    readflg = 0;
	    fchange = n;
	    continue;
	    
	  case 's':
	    scommand = 1;
	    setdot();
	    nonzero();
	    if (!globflg) save();
	    substitute(globp!=0);
	    continue;
	    
	  case 't':
	    move(1);
	    continue;
	    
	  case 'u':
	    setdot();
	    newline();
	    if (!initflg) undo();
	    else (void) error(5);
	    fchange = 1;
	    continue;
	    
	  case 'v':
	    global(0);
	    continue;
	  case 'V':
	    globaln(0);
	    continue;
	    
	  case 'w':

	    if (flag28) {
		flag28 = 0;
		fchange = 0;
		(void) error(61);
	    }
	    setall();
	    if((zero != dol) && (addr1 <= zero || addr2 > dol))
	      (void) error(15);
	    filename(c);
	    if (nowrite && (!strcmp(file, savedfile))) {
		fchange=0;
		error(63);
	    }
	    if(Xqt) {
		io = eopen(file, 1);
		n = 1;  /* set n so newtime will not execute */
	    } else {
		fstat(tfile, &Tf);
		if(stat(file, &Fl) < 0) {
		    if((io = open(file, O_CREAT|O_RDWR|O_TRUNC, 0666)) < 0)
		      (void) error(7);
		    fstat(io, &Fl);
		    Fl.st_mtime = 0;
		    close(io);
		}
		else {
		    oldmask = umask(0);
		}
		
		/*
		 * Check for space on file-system to do the write
		 */
		(void) statfs(file, &sb);
		if(!Short &&
		   sb.f_blocks <		
		   ((Tf.st_size / sb.f_bsize) +
		    (50000 / sb.f_bsize))) {
		    Short = 1;
		    (void) error(8);
		}
		Short = 0;
		
		if(Fl.st_nlink == 1 && (Fl.st_mode & S_IFMT) == S_IFREG)
		  {       if (close(open(file, O_WRONLY)))
			    (void) error(9);
			  p1 = savedfile;
			  p2 = file;
#ifdef TRACE
			  if (trace)
			    fprintf(trace, "origfile = %s, p1 = %s, p2 = %s\n",origfile,p1,p2);
#endif
			  if (!(n=strcmp(p1, p2)))
			    chktime();
			  
                          /* create temporary filename  FPM/DD001 */
                          tempfile = tempnam(tmpdir,"edtmp");
 
			  if ((io = open(tempfile, O_CREAT|O_RDWR|O_TRUNC, 
                                         Fl.st_mode)) >= 0) {
			      
#ifdef TRACE
			      if (trace)
				fprintf(trace, "tempfile = %s\n",tempfile);/* GZ001 */
#endif
			      putfile();
			      exfile();

			      outfi = fopen(file,"w");
			      tmpfi = fopen(tempfile,"r");/* GZ001 */
			      if ( !outfi || !tmpfi )
				(void) error(7);		/* longjmps away */
			      else
				filecopy(tmpfi,outfi);
			      fclose(outfi);
			      fclose(tmpfi);
			      unlink(tempfile);/* GZ001 */
			      free(tempfile);
#ifdef TRACE
			      if (trace)
				fprintf(trace, "after fclose(outfi)\n");
#endif
			      
			      /* if filenames are the same */
			      if (!n)
				newtime();
			      /* check if entire buffer was written */
			      fchange = ((addr1==zero || addr1==zero+1) && addr2==dol)?0:fchange;
			      continue;
			  }
		      }
		else   n = 1;   /* set n so newtime will not execute*/
		if((io = open(file, O_CREAT|O_RDWR|O_TRUNC, 0666)) < 0)
		  (void) error(7);
	    }
	    putfile();
	    exfile();
	    if (!n) newtime();
	    fchange = ((addr1==zero||addr1==zero+1)&&addr2==dol)?0:fchange;
	    continue;
	    
#ifdef CRYPT
	  case 'x':
	    error(60);

	  case 'X':
	    setnoaddr();
	    newline();
	    xflag = 1;
	    getkey();
	    kflag = crinit(key, perm);
	    continue;
#endif
	    
	    
	  case '=':
	    setall();
	    newline();
	    count = (dot-zero)&077777;
	    putd(count);
	    putchr('\n');
	    continue;
	    
	  case '!':
	    unixcom();
	    continue;
	    
	  case EOF:
	    return;
	    
	  case 'P':
	    if (yflag)
	      (void) error(59);
	    setnoaddr();
	    newline();
	    if (shflg)
	      shflg = 0;
	    else
	      shflg++;
	    continue;
	}

	/**** Not a legal command character ****/
	(void) error(12);
    }
}

LINE
address(void)
{
	register minus, c;
	register LINE a1;
	int n, relerr;

	minus = 0;
	a1 = 0;
	for (;;) {
		c = getchr();
		if ('0'<=c && c<='9') {
			n = 0;
			do {
				n *= 10;
				n += c - '0';
			} while ((c = getchr())>='0' && c<='9');
			peekc = c;
			if (a1==0)
				a1 = zero;
			if (minus<0)
				n = -n;
			a1 += n;
			minus = 0;
			continue;
		}
		relerr = 0;
		if (a1 || minus)
			relerr++;
		switch(c) {
		case ' ':
		case '\t':
			continue;

		case '+':
			minus++;
			if (a1==0)
				a1 = dot;
			continue;

		case '-':
		case '^':
			minus--;
			if (a1==0)
				a1 = dot;
			continue;

		case '?':
		case '/':
			re_compile((wchar_t) c);  /* Compile the Regular Expression */
			a1 = dot;
			for (;;) {
				if (c=='/') {
					a1++;
					if (a1 > dol)
						a1 = zero;
				} else {
					a1--;
					if (a1 < zero)
						a1 = dol;
				}
				if (execute(0, a1))
					break;
				if (a1==dot)
					(void) error(13);
			}
			break;

		case '$':
			a1 = dol;
			break;

		case '.':
			a1 = dot;
			break;

		case '\'':
			if ((c = getchr()) < 'a' || c > 'z')
				(void) error(2);
			for (a1=zero; a1<=dol; a1++)
				if (names[c-'a'] == (a1->cur & ~01))
					break;
			break;

		case 'y' & 037:
			if(yflag) {
				newline();
				setnoaddr();
				yflag &= ~YWCOUNT;	/* Turn off writing count */
				continue;
			}

		default:
			peekc = c;
			if (a1==0)
				return(0);
			a1 += minus;
			if (a1<zero || a1>dol)
				(void) error(15);
			return(a1);
		}
		if (relerr)
			(void) error(16);
	}
}

void setdot(void)
{
	if (addr2 == 0)
		addr1 = addr2 = dot;
	if (addr1 > addr2)
		(void) error(17);
}

void setall(void)
{
	if (addr2==0) {
		addr1 = zero+1;
		addr2 = dol;
		if (dol==zero)
			addr1 = zero;
	}
	setdot();
}

void setnoaddr(void)
{
	if (addr2)
		(void) error(18);
}

void nonzero(void)
{
	if (addr1<=zero || addr2>dol)
		(void) error(15);
}

void newline(void)
{
	register c;

	c = getchr();
	if ( c == 'p' || c == 'l' || c == 'n' ) {
		pflag++;
		if ( c == 'l') listf++;
		if ( c == 'n') listn++;
		c = getchr();
	}
	if ( c != '\n')
		(void) error(20);
}

void filename(register comm)
{
	register char *p1, *p2;
	register c;
	register i = 0;

	count = 0;
	c = getchr();
	if (c=='\n' || c==EOF) {
		p1 = savedfile;
		if (*p1==0 && comm!='f')
			(void) error(21);
		/* ncflg set means do not get mod time of file */
		/* since no filename followed f */
		if (comm == 'f')
			ncflg++;
		p2 = file;
		while (*p2++ = *p1++);
		red(savedfile);
		return;
	}
	if (c!=' ')
		(void) error(22);
	while ((c = getchr()) == ' ');
	if(c == '!')
		++Xqt, c = getchr();
	if (c=='\n')
		(void) error(21);
	p1 = file;
	do {
		if(++i >= FNSIZE)
			(void) error(24);
		*p1++ = c;
		if(c==EOF || (c==' ' && !Xqt))
			(void) error(21);
	} while ((c = getchr()) != '\n');
	*p1++ = 0;
	if(Xqt)
		if (comm=='f') {
			--Xqt;
			(void) error(57);
		}
		else
			return;
	if (savedfile[0]==0 || comm=='e' || comm=='f') {
		p1 = savedfile;
		p2 = file;
		while (*p1++ = *p2++);
	}
	red(file);
}

void exfile(void)
{
	if(oldmask) {
		umask(oldmask);
		oldmask = 0;
	}

	eclose(io);
	io = -1;
	if (vflag) {
		putd(count);
		putchr('\n');
#ifdef NULLS
		if(nulls) {
			putd(nulls);
			nulls = 0;
			puts(MSGSTR(M_EDNULLS,
                		" null characters were replaced by the \\0 character."));
			/* Leave space at beginning, message follows a number */
		}
#endif
	}
}

void onintr(int dummy)
{
	putchr('\n');
	lastc = '\n';
	if (*funny) unlink(funny); /* remove tmp file */
	/* if interruped a read, only part of file may be in buffer */
	if ( readflg ) {
		fprintf(stderr,MSGSTR(M_EDINCREAD,
  	 		"\007read was interupted and may be incomplete.\007"));
		fchange = 0;
	}
	(void) error(26);
}

void onhup(int dummy)
{
	(void) signal(SIGINT, SIG_IGN);
	/* if there are lines in file and file was */
	/* not written since last update, save in ed.hup, or $HOME/ed.hup */
	if (dol > zero && fchange == 1) {
		addr1 = zero+1;
		addr2 = dol;
		io = open("ed.hup", O_CREAT|O_RDWR|O_TRUNC, 0666);
		if(io < 0 && home) {
			char    *fn;

			fn = calloc(strlen(home) + 8, sizeof(char));
			if(fn) {
				strcpy(fn, home);
				strcat(fn, "/ed.hup");
				io = open(fn, O_CREAT|O_RDWR|O_TRUNC, 0666);
				free(fn);
			}
		}
		if (io)
			putfile();
	}
	fchange = 0;
	quit();
}

/*
 * Checks the error status returned from the regcomp() routine and invokes the
 * error() function with the appropriate error code.
 *
 */
void re_error(int regcomp_status)
{
	switch(regcomp_status) {

	case REG_ESUBREG:
			(void) error(25);
			break;

	case REG_EBRACK:
			(void) error(49);
			break;

	default:
			(void) error(51);
			break;
	}
}

int error(register code)
{
	register c;

	if (code == 28 && save28 == 0) {
		fchange = 0; 
		flag28++;
	}
	readflg = 0;
	++errcnt;
	listf = listn = 0;
	pflag = 0;

	if(oldmask) {
		umask(oldmask);
		oldmask = 0;
	}

#ifdef NULLS    /* Not really nulls, but close enough */
	/* This is a bug because of buffering */
	if(code == 28) /* illegal char. */
		putd(count);
#endif
	putchr('?');
	if(code == 3)   /* Cant open file */
		puts(file);
	else
		putchr('\n');
	count = 0;
	lseek(STDIN_FILENO, (long)0, SEEK_END);
	if (globp)
		lastc = '\n';
	globp = 0;
	peekc = lastc;
	if(lastc)
		while ((c = getchr()) != '\n' && c != EOF);
	if (io) {
		eclose(io);
		io = -1;
	}
	xcode = code;
	if(hflag)
		PUTM(xcode);
	if(code==4)return(0);   /* Non-fatal error. */
	longjmp(savej, 1);
	return(1);
}


int getchr(void)
{
	unsigned char c;

	if (lastc=peekc) {
		peekc = 0;
		return(lastc);
	}
	if (globp) {
		if ((lastc = *globp++) != 0)
			return(lastc);
		globp = 0;
		return(EOF);
	}
	if (read(STDIN_FILENO, (char *)&c, 1) <= 0)
		return(lastc = EOF);
	lastc = (int)c; 
	return(lastc);
}

int gettty(void)
{
	register c;
	register char *gf;
	register unsigned char *p;

	p = linebuf;
	gf = globp;
	while ((c = getchr()) != '\n') {
		if (c==EOF) {
			if (gf)
				peekc = c;
			return(c);
		}
		if (c == 0)
			continue;
		*p++ = c;
		if (p >= &linebuf[LBSIZE-2])
			(void) error(27);
	}
	*p++ = 0;
	if (linebuf[0]=='.' && linebuf[1]==0)
		return(EOF);
	/*
	 * POSIX.2 Draft 11 explicitly says no to this.
	 * 
	 * if (linebuf[0]=='\\' && linebuf[1]=='.' && linebuf[2]==0) {
	 * 	linebuf[0] = '.';
	 * 	linebuf[1] = 0;
	 * }
	 */
	return(0);
}

int getfile(void)
{
	register c;
	register unsigned char *lp, *fp;
	int crflag;

	crflag = 0;
	lp = linebuf;
	fp = nextip;
	do {
		if (--ninbuf < 0) {
			if ((ninbuf = read(io, (char *)genbuf, LBSIZE)-1) < 0)
				return(EOF);
			fp = genbuf;
		}
		if (lp >= &linebuf[LBSIZE]) {
			lastc = '\n';
			(void) error(27);
		}
		if ((*lp++ = c = *fp++) == 0) { 
#ifdef NULLS
			lp[-1] = '\\';
			*lp++ = '0';
			nulls++;
#else
			lp--;
			continue;
#endif
		}
		count++;
	} while (c != '\n');
	*--lp = 0;
	nextip = fp;
	if (fss.Ffill && fss.Flim && lenchk((char *)linebuf,&fss) < 0) {
	    	fprintf(stderr, MSGSTR(M_EDLINE,
			"line cannot exceed 512 characters. Line number "));

		putd( ((dot+1)-zero) & 077777 );
		putchr('\n');
	}
	return(0);
}

void putfile(void)
{
	int n;
	LINE a1;
	register unsigned char *fp, *lp;
	register nib;

	nib = 512;
	fp = genbuf;
	a1 = addr1;
	do {
		lp = getline((a1++)->cur);
		if (fss.Ffill && fss.Flim && lenchk((char *)linebuf,&fss) < 0) {
		    	fprintf(stderr, MSGSTR(M_EDLINE,
				"line cannot exceed 512 characters. Line number "));

			putd( ((dot+1)-zero) & 077777 );
			putchr('\n');
		}
		for (;;) {
			if (--nib < 0) {
				n = fp-genbuf;
#ifdef CRYPT
				if(kflag)
					crblock(perm, genbuf, n, count-n);
#endif
				if(write(io, (char *)genbuf, n) != n)
					(void) error(29);
				nib = 511;
				fp = genbuf;
			}
			if(dol->cur == 0)break; /* Allow write of null file */
			count++;
			if ((*fp++ = *lp++) == 0) {
				fp[-1] = '\n';
				break;
			}
		}
	} while (a1 <= addr2);
	n = fp-genbuf;
#ifdef CRYPT
	if(kflag)
		crblock(perm, genbuf, n, count-n);
#endif
	if(write(io, (char *)genbuf, n) != n || fsync(io))
		(void) error(29);
}

void append(int (*f10)(void), LINE a)
{
	register LINE a1, a2, rdot;
	int tl;

	nline = 0;
	dot = a;
	while ((*f10)() == 0) {
		if (dol >= endcore) {
			lastc = '\n';
			(void) error(30);
		}
		tl = putline();
		nline++;
		a1 = ++dol;
		a2 = a1+1;
		rdot = ++dot;
		while (a1 > rdot)
			(--a2)->cur = (--a1)->cur;
		rdot->cur = tl;
	}
}

void unixcom(void)
{
	void (*savint)(int);
	register  pid, rpid;
	int retcode;
	static char savcmd[LBSIZE];     /* last command */
	char curcmd[LBSIZE];            /* current command */
	char *psavcmd, *pcurcmd, *psavedfile;
	register c, shflag=0;
	int	i, len;
	char	str[MB_LEN_MAX];
		
	setnoaddr();
	if(rflg)
		(void) error(6);
	pcurcmd = curcmd;

	/* A '!' found in beginning of command is replaced with the */
	/* saved command. A '%' found in command is replaced with   */
	/* the current filename					    */
	c = getchr();
	if (c == '!') {
		if (savcmd[0]==0)
			(void) error(56);
		else {
			psavcmd = savcmd;
			while (*pcurcmd++ = *psavcmd++);
			--pcurcmd;
			shflag = 1;
		}
	} else
		 peekc = c;  /* put c back */

	for (;;) {
		if ((c = getchr()) == '\n') 	/* end of command */
			break;
		else if (c == '%') {		
			/* insert current filename into command string */
			if (savedfile[0]==0)
				/* no remembered filename */
				(void) error(21);
			else {
				psavedfile = savedfile;
				while (*pcurcmd++ = *psavedfile++);
				--pcurcmd;
				shflag = 1;
			}
			continue;
		} else if (c == '\\') {
			/* '\\' has special meaning only if preceding a '%' */
			if ((c = getchr()) != '%') 
				*pcurcmd++ = '\\';
			else {
				*pcurcmd++ = c;
				continue;
			}
		} 

		/*
		 * Copy multi-byte character to pcurcmd
		 * Trick is to keep trying to mblen a byte stream til it says
		 * you have a valid character.
		 */
		len = 1;
		str[0] = c;
		while (mblen(str, MB_CUR_MAX) != len) {
			if (++len > MB_CUR_MAX)
				error(20);
			str[len-1] = getchr();
		}
		for (i=0; i<len; i++)
			*pcurcmd++ = str[i];
		if (pcurcmd >= &curcmd[LBSIZE - 1])
			error(27);
	}
	*pcurcmd++ = 0;
	if (shflag == 1)
		puts(curcmd);
	/* save command */
	strcpy(savcmd,curcmd);

	if ((pid = fork()) == 0) {
		(void) signal(SIGHUP, oldhup);
		(void) signal(SIGQUIT, oldquit);
		execlp(_PATH_SH, "sh", "-c", curcmd, (char *) 0);
		exit(0100);
	}
	savint = signal(SIGINT, SIG_IGN);
	while ((rpid = wait(&retcode)) != pid && rpid != -1);
	(void) signal(SIGINT, savint);
	if (vflag) puts("!");
}

void quit(void)
{
	if (vflag && fchange) {
		fchange = 0;
		/* For case where user reads in BOTH a good file & a bad file */
		if (flag28) {
			flag28 = 0;
			(void) error(62);
		}
		(void) error(1);
	}
	unlink(tfname);
#ifdef TRACE
	fclose(trace);
#endif
	/* close the message catalog */
	catclose(catd);
	exit(errcnt? 2: 0);
}

void delete(void)
{
	setdot();
	newline();
	nonzero();
	if (!globflg) save();
	rdelete(addr1, addr2);
}

void rdelete(LINE ad1, LINE ad2)
{
	register LINE a1, a2, a3;

	a1 = ad1;
	a2 = ad2+1;
	a3 = dol;
	dol -= a2 - a1;
	do
		(a1++)->cur = (a2++)->cur;
	while (a2 <= a3);
	a1 = ad1;
	if (a1 > dol)
		a1 = dol;
	dot = a1;
	fchange = 1;
}

void gdelete(void)
{
	register LINE a1, a2, a3;

	a3 = dol;
	for (a1=zero+1; (a1->cur&01)==0; a1++)
		if (a1>=a3)
			return;
	for (a2=a1+1; a2<=a3;) {
		if (a2->cur&01) {
			a2++;
			dot = a1;
		} else
			(a1++)->cur = (a2++)->cur;
	}
	dol = a1-1;
	if (dot>dol)
		dot = dol;
	fchange = 1;
}


/*
 * Obtains a pattern string for Regular Expression compilation and compiles
 * the pattern 
 *
 */
void re_compile(wchar_t eof)
{
	char *temp;
	register c;
	int status;
	int	i, len;
	wchar_t wch;
	char	str[MB_LEN_MAX];

	status = 0;
	temp = expbuf;
	for (;;) {
		if ((c = getchr()) == '\n')
			break;
		len = 1;
		if (c == '\\') {
			*temp++ = c;
			str[0] = getchr();
			while (mbtowc(&wch, str, MB_CUR_MAX) != len) {
				if (++len > MB_CUR_MAX)
					error(51);
				str[len-1] = getchr();
			}
		} else {
			str[0] = c;
			while (mbtowc(&wch, str, MB_CUR_MAX) != len) {
				if (++len > MB_CUR_MAX)
					error(51);
				str[len-1] = getchr();
			}
			if (wch == eof)
				break;
		}
		for (i=0; i<len; i++)
			*temp++ = str[i];
		if (temp > expend)
			error(50);
	}
	*temp = '\0';
	if (c == '\n') {
		if (scommand)	/* Substitute expression incomplete */
			(void) error1(36);
		else	/* Remember for line display execution in commands() */
			peekc = c;  
	}
	if (*expbuf != '\0') {
		if (preg.re_comp != 0)
			regfree(&preg);
		if ((status = regcomp(&preg, expbuf, 0)) != 0)
			(void) re_error(status);
	}
	else {
		if (preg.re_comp == 0)
			error(41);
	}
}
 

unsigned char *
getline(int tl)
{
	register unsigned char *bp, *lp;
	register nl;

	lp = linebuf;
	bp = getblock(tl, READ);
	nl = nleft;
	tl &= ~0377;
	while (*lp++ = *bp++)
		if (--nl == 0) {
			bp = getblock(tl+=0400, READ);
			nl = nleft;
		}
	return(linebuf);
}

int putline(void)
{
	register unsigned char *bp, *lp;
	register nl;
	int tl;

	fchange = 1;
	lp = linebuf;
	tl = tline;
	bp = getblock(tl, WRITE);
	nl = nleft;
	tl &= ~0377;
	while (*bp = *lp++) {
		if (*bp++ == '\n') {
			*--bp = 0;
			linebp = lp;
			break;
		}
		if (--nl == 0) {
			bp = getblock(tl+=0400, WRITE);
			nl = nleft;
		}
	}
	nl = tline;
	tline += (((lp-linebuf)+03)>>1)&077776;
	return(nl);
}

unsigned char *
getblock(int atl, int iof)
{
	register off;
	int	bno;
#ifdef CRYPT
	register char *p1, *p2;
	register int n;
#endif /* CRYPT */


	bno = (atl>>8);
	off = (atl<<1)&0774;
	if (bno >= 6144) {
		lastc = '\n';
		nowrite=1;
		(void) error(31);
	}
	nleft = 512 - off;
	if (bno==iblock) {
		ichanged |= iof;
		return(ibuff+off);
	}
	if (bno==oblock)
		return(obuff+off);
	if (iof==READ) {
		if (ichanged) {
#ifdef CRYPT
			if(xtflag
)
				crblock(tperm, ibuff, 512, (long)0);
#endif
			blkio(iblock, (char *)ibuff, (blkiof) write);
		}
		ichanged = 0;
		iblock = bno;
		blkio(bno, (char *)ibuff, (blkiof) read);
#ifdef CRYPT
		if(xtflag)
			crblock(tperm, ibuff, 512, (long)0);
#endif
		return(ibuff+off);
	}
	if (oblock>=0) {
#ifdef CRYPT
		if(xtflag) {
			p1 = obuff;
			p2 = crbuf;
			n = 512;
			while(n--)
				*p2++ = *p1++;
			crblock(tperm, crbuf, 512, (long)0);
			blkio(oblock, crbuf, (blkiof) write);
		} else
#endif
			blkio(oblock, (char *)obuff, (blkiof) write);
	}
	oblock = bno;
	return(obuff+off);
}

void blkio(int b, char *buf, blkiof iofcn)
{

	lseek(tfile, (long)b<<9, SEEK_SET);
	if ((*iofcn)(tfile, buf, 512) != 512) {
		if(dol != zero) (void) error(32); /* Bypass this if writing null file */
	}
}

void init(void)
{
	register *markp;
	int omask;

	close(tfile);
	tline = 2;
	for (markp = names; markp < &names[26]; )
		*markp++ = 0;
	subnewa = 0;
	anymarks = 0;
	iblock = -1;
	oblock = -1;
	ichanged = 0;
	initflg = 1;
	omask = umask(0);
	close(open(tfname, O_CREAT|O_RDWR|O_TRUNC, 0600));
	umask(omask);
	tfile = open(tfname, O_RDWR);
#ifdef CRYPT
	if(xflag) {
		xtflag = 1;
		makekey(key, tperm);
	}
#endif
	dot = zero = dol = savdot = savdol = fendcore;
	flag28 = save28 = 0;
	endcore = fendcore + maxlines - 1;
}

void global(int k)
{
	register char *gp;
	register c;
	register LINE a1;
	int i, len;
	wchar_t wch;
	char globuf[GBSIZE], str[MB_LEN_MAX];

	if (globp)
		(void) error(33);
	setall();
	nonzero();
	if ((c=getchr())=='\n')
		(void) error(19);
	save();
	len = 1;
	str[0] = c;
	while (mbtowc(&wch, str, MB_CUR_MAX) != len) {
		if (++len > MB_CUR_MAX)
			(void) error(19);
		str[len-1] = getchr();
	}	
	re_compile(wch);	/* Compile the Regular Expression	*/
	gp = globuf;
	for (;;) {
		if ((c = getchr()) == '\n') 
			break;
		if (c == EOF)
			(void) error(19);
		if (c == '\\') {
			/* '\\' has special meaning only if preceding a '\n' */
			c = getchr();
			if (c != '\n')
				*gp++ = '\\';
			else {
				*gp++ = c;
				if (gp >= &globuf[GBSIZE-2])
					(void) error(34);
				continue;
			}
		}
		/* The processing of this stream is done on a 		*/
		/* multi-byte character by character basis instead of 	*/
		/* byte by byte entirely because '\\' is not in the 	*/
		/* unique code-point range 				*/
		len = 1;
		str[0] = c;
		while (mblen(str, MB_CUR_MAX) != len) {
			if (++len > MB_CUR_MAX)
				(void) error(19);
			str[len-1] = getchr();
		}
		for (i=0; i<len; i++)
			*gp++ = str[i];
		if (gp >= &globuf[GBSIZE-2])
			(void) error(34);
	}
	if (gp == globuf)
		*gp++ = 'p';
	*gp++ = '\n';
	*gp++ = 0;
	for (a1=zero; a1<=dol; a1++) {
		a1->cur &= ~01;
		if (a1>=addr1 && a1<=addr2 && execute(0, a1)==k)
			a1->cur |= 01;
	}
	/*
	 * Special case: g/.../d (avoid n^2 algorithm)
	 */
	if (globuf[0]=='d' && globuf[1]=='\n' && globuf[2]=='\0') {
		gdelete();
		return;
	}
	for (a1=zero; a1<=dol; a1++) {
		if (a1->cur & 01) {
			a1->cur &= ~01;
			dot = a1;
			globp = globuf;
			globflg = 1;
			commands();
			globflg = 0;
			a1 = zero;
		}
	}
}

void join(void)
{
	register unsigned char *gp, *lp;
	register LINE a1;

	if (addr1 == addr2) return;
	gp = genbuf;
	for (a1=addr1; a1<=addr2; a1++) {
		lp = getline(a1->cur);
		while (*gp = *lp++)
			if (gp++ >= &genbuf[LBSIZE-2])
				(void) error(27);
	}
	lp = linebuf;
	gp = genbuf;
	while (*lp++ = *gp++);
	addr1->cur = putline();
	if (addr1<addr2)
		rdelete(addr1+1, addr2);
	dot = addr1;
}

void substitute(int inglob)
{
	register gsubf, nl;
	register LINE a1;
	int *markp;
	int gf, n;

	gsubf = compsub();
	for (a1 = addr1; a1 <= addr2; a1++) {
		gf = n = 0;
		do {
			if (execute(gf++, a1) == 0)
				break;
			if (gsubf == GLOBAL_SUB || gsubf == gf) {
				n++;
				dosub();
			}
			else {
				offset_base += pmatch[0].rm_eo;
			}
			/* if matched null string, increment offset_base */
			if (pmatch[0].rm_so == pmatch[0].rm_eo)
			  	offset_base++;
		} while (*offset_base != '\0' && (gsubf == GLOBAL_SUB || gsubf > gf));
		if (n == 0)
			continue;
		inglob |= 01;
		subnewa = putline();
		a1->cur &= ~01;
		if (anymarks) {
			for (markp = names; markp < &names[26]; markp++)
				if (*markp == a1->cur)
					*markp = subnewa;
		}
		a1->cur = subnewa;
		append(getsub, a1);
		nl = nline;
		a1 += nl;
		addr2 += nl;
	}
	if (inglob==0)
		(void) error(35);
}

short remem[LBSIZE] = {-1};

int compsub(void)
{
	register c;
	register unsigned char *p;
	wchar_t	seof, wch;
	int n, i, len;
	char str[MB_LEN_MAX];

	if ((c = getchr()) == '\n' || c == ' ')
		(void) error(36);
	len = 1;
	str[0] = c;
	while (mbtowc(&seof, str, MB_CUR_MAX) != len) {
		if (++len > MB_CUR_MAX)
			(void) error(36);
		str[len-1] = getchr();
	}
	re_compile(seof);  /* Compile the Regular Expression	*/
	p = (unsigned char *) rhsbuf;
	for (;;) {
		len = 1;
		if ((c = getchr()) == '\\') {
				/* copy '\\' into p and read next char */
			*p++ = c;
			c = getchr();
			if (c >= preg.re_nsub + '1' && c < '9') 
				error(25);
			str[0] = c;
			while (mblen(str, MB_CUR_MAX) != len) {
				if (++len > MB_CUR_MAX)
					error(51);
				str[len-1] = getchr();
			}	
		} else if (c == '\n') {
			if (globp && globp[0]) {
				*p++ = '\\';
				str[0] = c;
			} else {
				peekc = c;
				pflag++;
				break;
			}
		} else {
			/* read in a multi-byte character */
			str[0] = c;
			while (mbtowc(&wch, str, MB_CUR_MAX) != len) {
				if (++len > MB_CUR_MAX)
					error(51);
				str[len-1] = getchr();
			}	
			if (wch == seof)
				break;
		}
		for (i=0; i<len; i++)
			*p++ = str[i];
		if (p >= (unsigned char *)&rhsbuf[LBSIZE])
			(void) error(38);
	}
	*p++ = 0;
	if(rhsbuf[0] == '%' && rhsbuf[1] == 0)
		(remem[0] != -1) ? (void) strcpy(rhsbuf, (char *)remem) : (void) error(55);
	else
		(void) strcpy((char *)remem, rhsbuf);
	for (n=0; (c = getchr()) >= '0' && c <= '9';)
		n = n * 10 + c - '0';
	peekc = c;
	if (n == 0)
		if (peekc == 'g') {
			peekc = 0;
			n = GLOBAL_SUB;
		} else
			n = 1;
	newline();
	return(n);
}

int getsub(void)
{
	register unsigned char *p1, *p2;

	p1 = linebuf;
	if ((p2 = linebp) == 0)
		return(EOF);
	while (*p1++ = *p2++);
	linebp = 0;
	return(0);
}

void dosub(void)
{
	register unsigned char *lp, *sp, *rp;
	char 	c;
	int	len;

	lp = linebuf;
	sp = genbuf;
	rp = (unsigned char *) rhsbuf;

	while (lp < (unsigned char *)(offset_base + pmatch[0].rm_so))
		*sp++ = *lp++;
	while (c = *rp) {
		if (c == '&') {
			rp++;
			sp = place(sp, (unsigned char *)(offset_base + pmatch[0].rm_so), 
					(unsigned char *)(offset_base + pmatch[0].rm_eo));
			continue;
		} else if (c == '\\') { 
			c = *++rp;	/* discard '\' */
			if(c >= '1' && c < preg.re_nsub + '1') {
				rp++;
				sp = place(sp, (unsigned char *)(offset_base + pmatch[c-'0'].rm_so),
						(unsigned char *)(offset_base +  pmatch[c-'0'].rm_eo));
				continue;
			}
		}
		if ((len = mblen((char *)rp, MB_CUR_MAX)) == -1 ) 
			(void) error(28);     /* illegal multi-byte character */
		while (len--) 
			*sp++ = *rp++;
		if (sp >= &genbuf[LBSIZE]) 
			(void) error(27);
	}
	lp = (unsigned char *)(offset_base + pmatch[0].rm_eo);
	offset_base = (char *)(sp - genbuf + linebuf);
	while (*sp++ = *lp++)
		if (sp >= &genbuf[LBSIZE]) 
			(void) error(27);
	lp = linebuf;
	sp = genbuf;
	while (*lp++ = *sp++);
}

unsigned char *
place(register unsigned char *sp, register unsigned char *l1, register unsigned char *l2)
{

	while (l1 < l2) {
		*sp++ = *l1++;
		if (sp >= &genbuf[LBSIZE])
			(void) error(27);
	}
	return(sp);
}

void move(int cflag)
{
	register LINE adt, ad1, ad2;
	int getcopy(void);

	setdot();
	nonzero();
	if ((adt = address())==0)
		(void) error(39);
	newline();
	if (!globflg) save();
	if (cflag) {
		ad1 = dol;
		append(getcopy, ad1++);
		ad2 = dol;
	} else {
		ad2 = addr2;
		for (ad1 = addr1; ad1 <= ad2;)
			(ad1++)->cur &= ~01;
		ad1 = addr1;
	}
	ad2++;
	if (adt<ad1) {
		dot = adt + (ad2-ad1);
		if ((++adt)==ad1)
			return;
		reverse(adt, ad1);
		reverse(ad1, ad2);
		reverse(adt, ad2);
	} else if (adt >= ad2) {
		dot = adt++;
		reverse(ad1, ad2);
		reverse(ad2, adt);
		reverse(ad1, adt);
	} else
		(void) error(39);
	fchange = 1;
}

void reverse(register LINE a1, register LINE a2)
{
	register int t;

	for (;;) {
		t = (--a2)->cur;
		if (a2 <= a1)
			return;
		a2->cur = a1->cur;
		(a1++)->cur = t;
	}
}

int getcopy(void)
{

	if (addr1 > addr2)
		return(EOF);
	(void) getline((addr1++)->cur);
	return(0);
}


void error1(int code)
{
	if (preg.re_comp != 0) {
		regfree(&preg);
		preg.re_comp = 0;
	}
	(void) error(code);
}

int execute(int gf, LINE addr)
{
	register char *p1;
	int status;

	if (gf) {
		p1 = offset_base;
		status = regexec(&preg, p1, (preg.re_nsub + 1), pmatch, REG_NOTBOL); 
	} else {
		if (addr==zero)
			return(0);
		p1 = (char *)getline(addr->cur);
		status = regexec(&preg, p1, (preg.re_nsub + 1), pmatch, 0); 
		offset_base = (char *)linebuf;
	}
	if (status == 0)
		return(1);
	else
		return(0);
}


void putd(long int c)
{
	register r;

	r = (int)(c%10);
	c /= 10;
	if (c)
		putd(c);
	putchr(r + '0');
}

int puts(const char *s)
{
	char	*msg_ptr;		/* pointer to message text */
	register char *sp = (char *)s;
	int sz, i, ch_len;

	if (fss.Ffill && (listf == 0)) {
		if ((i = expnd(sp,funny,&sz,&fss)) == -1) {
			write(1,funny,fss.Flim & 0377); putchr('\n');
			msg_ptr =  MSGSTR(M_EDTOOLONG,
			    "Cannot write a line greater than 512 characters.");
			write(1, msg_ptr, strlen(msg_ptr));
		}
		else
			write(1,funny,sz);
		putchr('\n');
		if (i == -2)
		{
			msg_ptr =  MSGSTR(M_EDTAB,
			    "Cannot write a line greater than 512 characters.\n");
			write(1, msg_ptr, strlen(msg_ptr));
		}
		return(0);
	}
	col = 0;
	while (*sp) {
	    wchar_t	wp;

	    ch_len = mbtowc(&wp, sp, MB_CUR_MAX);
	    if (listf) {
		if (ch_len < 1) 
		  (void) error(28);	/* illegal character */
		else if (ch_len == 1)
		  putchr(*sp++);
		else {
		    sp += ch_len;
		    putwchr(wp);
		}
	    } else {
		putchr(*sp++);
	    }
	}
	if (listf)
	    putchr('$');
	putchr('\n');
        return(1);
}

char    line[70];
char    *linp = line;

/*
 * This routine will never get multibyte characters, they go to
 * putwchr().
 * 
 * Per POSIX.2 Draft 11:
 * 	nonprintable characters will be written as one three-digit
 * 	octal number (with a preceding <backslash> character) for
 * 	each byte in the character (MSB first) except for the following
 * 	characters:
 * 	  '\\'		<backslash>
 * 	  '\a'		<alert>
 * 	  '\b'		<backspace>
 * 	  '\f'		<form-feed>
 * 	  '\r'		<carrage return>
 * 	  '\t'		<tab>
 * 	  '\v'		<verticle tab>
 * 
 */

void putchr(int ac)
{
	register char *lp;
	register c;
	short len;
	int chrwid;

	lp = linp;
	c = ac;
	if (listf && c != '\n') {
		switch (c) {
			case '\\' :
				*lp++ = '\\';
				*lp++ = '\\';
				chrwid = 2;
				break;
#ifdef __STDC__
			case '\a' :
#else
			case '\007' :
#endif
				*lp++ = '\\';
				*lp++ = 'a';
				chrwid = 2;
				break;
			case '\b' :
				*lp++ = '\\';
				*lp++ = 'b';
				chrwid = 2;
				break;
			case '\f' :
				*lp++ = '\\';
				*lp++ = 'f';
				chrwid = 2;
				break;
			case '\r' :
				*lp++ = '\\';
				*lp++ = 'r';
				chrwid = 2;
				break;
			case '\t' :
				*lp++ = '\\';
				*lp++ = 't';
				chrwid = 2;
				break;
			case '\v' :
				*lp++ = '\\';
				*lp++ = 'v';
				chrwid = 2;
				break;
			default:
				if (isprint(c)) {
					*lp++ = c;
					chrwid = 1;
				} else {
					*lp++ = '\\';
					*lp++ = ((c&0300)>>6)+'0';
					*lp++ = ((c&070)>>3)+'0';
					*lp++ = (c&007)+'0';
					chrwid = 4;
				}
				break;
		}
			
		col += chrwid;
		if (col >= 72) {
			col = 0;
			*lp++ = '\\';
			*lp++ = '\n';
		}
	} else
		*lp++ = c;
	if(c == '\n' || lp >= &line[65]) {
		linp = line;
		len = lp - line;
		if(yflag & YWCOUNT)
			write(1, (char *) &len, sizeof(len));
		write(1, line, len);
		return;
	}
	linp = lp;
}

void putwchr(wchar_t ac)
{
	register char *lp;
	char	buf[MB_LEN_MAX], *p;
	wchar_t c;
	short len;

	lp = linp;
	c = ac;
	if (listf) {
		if (!iswprint(c)) {
			p = &buf[0];
			len = wctomb(p, c);
			while (len--) {
				if (col + 4 >= 72) {
					col = 0;
					*lp++ = '\\';
					*lp++ = '\n';
				}
				sprintf(lp, "\\%03o", *(unsigned char *)p++);
				col += 4;
				lp += 4;
			}
		} else {
			len = wcwidth(c);
			if (col	+ len >= 72) {
				col = 0;
				*lp++ = '\\';
				*lp++ = '\n';
			}
			col += len;
			lp += wctomb(lp, c);
		}
	} else
		lp += wctomb(lp, c);
	if(lp >= &line[65]) {
		linp = line;
		len = lp - line;
		if(yflag & YWCOUNT)
			write(1, (char *) &len, sizeof(len));
		write(1, line, len);
		return;
	}
	linp = lp;
}
 
#ifdef CRYPT
void crblock(char *permp, char *buf, int nchar, long startn)
{
	register char   *p1;
	register int n1, n2;
	register char   *t1, *t2, *t3;

	t1 = permp;
	t2 = &permp[256];
	t3 = &permp[512];

	n1 = (int)(startn&0377);
	n2 = (int)((startn>>8)&0377);
	p1 = buf;
	while(nchar--) {
		*p1 = t2[(t3[(t1[(*p1+n1)&0377]+n2)&0377]-n2)&0377]-n1;
		n1++;
		if(n1==256){
			n1 = 0;
			n2++;
			if(n2==256) n2 = 0;
		}
		p1++;
	}
}

void getkey(void)
{
	char	*msg_ptr;		/* pointer to message text */
	struct termio b;
	int save;


	/* int (*sig)(); changed to void for ANSI compatibility */
	int (*sig)();
	char *p;
	int c;

	sig = signal(SIGINT, SIG_IGN);
	ioctl(0, TCGETA, &b);
	save = b.c_lflag;
	b.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
	ioctl(0, TCSETA, &b);
	msg_ptr =  MSGSTR(M_EDKEY,
		    "Enter the file encryption key: ");
	write(1, msg_ptr, strlen(msg_ptr));
	p = key;
	while(((c=getchr()) != EOF) && (c!='\n')) {
		if(p < &key[KSIZE])
			*p++ = c;
	}
	*p = 0;
	write(1, "\n", 1);
	b.c_lflag = save;
	ioctl(0, TCSETA, &b);
	(void) signal(SIGINT, (void (*)(int))sig);
	/* this line used to say "return(key[0] != 0);" */
}

/*
 * Besides initializing the encryption machine, this routine
 * returns 0 if the key is null, and 1 if it is non-null.
 */
void crinit(char *keyp, char *permp)
{
	register char *t1, *t2, *t3;
	int ic, i, k, temp, pf[2];
	unsigned random;
	char buf[13];
	long seed;

	if (yflag)
		(void) error(59);
	t1 = permp;
	t2 = &permp[256];
	t3 = &permp[512];

	if (*keyp == 0)
		return(0);

	(void) strncpy(buf, keyp, 8);
	while (*keyp)
		*keyp++ = '\0';
	buf[8] = buf[0];
	buf[9] = buf[1];
	if(pipe(pf) < 0)
		(void) error(0);
	i = fork();
	if(i == -1)
		(void) error(23);
	if(i == 0) {
		close(0);
		close(1);
		dup(pf[0]);
		dup(pf[1]);
		execl("/usr/lib/makekey", "-", (char *) 0, (char *) 0, (char *) 0);
		execl("/lib/makekey", "-", (char *) 0, (char *) 0, (char *) 0);
		/* close the message catalog */
		catclose(catd);
		exit(2);
	}
	write(pf[1], buf, 10);
	if (wait((int *) 0)== -1 || read(pf[0], buf, 13)!=13) {
		puts(MSGSTR(M_EDGENKEY,
		    "crypt: Cannot generate the encryption key."));
		/* close the message catalog */
		catclose(catd);
		exit(2);
	}
	close(pf[0]);
	close(pf[1]);
	seed = 123;
	for (i=0; i<13; i++)
		seed = seed*buf[i] + i;
	for(i=0;i<256;i++) {
		t1[i] = i;
		t3[i] = 0;
	}
	for(i=0;i<256;i++) {
		seed = 5*seed + buf[i%13];
		random = (int)(seed % 65521);
		k = 256-1 -i;
		ic = (random&0377)%(k+1);
		random >>= 8;
		temp = t1[k];
		t1[k] = t1[ic];
		t1[ic] = temp;
		if(t3[k]!=0) continue;
		ic = (random&0377) % k;
		while(t3[ic]!=0) ic = (ic+1) % k;
		t3[k] = ic;
		t3[ic] = k;
	}
	for(i=0;i<256;i++)
		t2[t1[i]&0377] = i;
	return(1);
}

void makekey(char *a, char *b)
{
	register int i;
	long gorp;
	char temp[KSIZE + 1];

	for(i = 0; i < KSIZE; i++)
		temp[i] = *a++;
	time(&gorp);
	gorp += getpid();

	for(i = 0; i < 4; i++)
		temp[i] ^= (char)((gorp>>(8*i))&0377);

	i = crinit(temp, b);
}
#endif

void globaln(int k)
{
	register char *gp;
	register c;
	register LINE a1;
	int  i, len, nfirst;
	wchar_t wch;
	char globuf[GBSIZE], str[MB_LEN_MAX];

	if (yflag)
		(void) error(59);
	if (globp)
		(void) error(33);
	setall();
	nonzero();
	if ((c=getchr())=='\n')
		(void) error(19);
	save();
	len = 1;
	str[0] = c;
	while (mbtowc(&wch, str, MB_CUR_MAX) != len) {
		if (++len > MB_CUR_MAX)
			(void) error(19);
		str[len-1] = getchr();
	}	
	re_compile(wch);	/* Compile the Regular Expression	*/
	for (a1=zero; a1<=dol; a1++) {
		a1->cur &= ~01;
		if (a1>=addr1 && a1<=addr2 && execute(0, a1)==k)
			a1->cur |= 01;
	}
	nfirst = 0;
	newline();
	for (a1=zero; a1<=dol; a1++) {
		if (a1->cur & 01) {
			a1->cur &= ~01;
			dot = a1;
			puts((char *)getline(a1->cur));
			if ((c=getchr()) == EOF)
				(void) error(52);
			if(c=='a' || c=='i' || c=='c')
				(void) error(53);
			if (c == '\n') {
				a1 = zero;
				continue;
			}
			if (c != '&') {
				gp = globuf;
				/* c is first byte of a multi-byte character */
				len = 1;
				str[0] = c;
				while (mblen(str, MB_CUR_MAX) != len) {
					if (++len > MB_CUR_MAX)
						error(52);
					str[len-1] = getchr();
				}
				for (i=0; i<len; i++)
					*gp++ = str[i];
				while ((c = getchr()) != '\n') {
					if (c == '\\') {
						if ((c = getchr()) != '\n')
							*gp++ = '\\';
						else {
							*gp++ = c;
							if (gp >= &globuf[GBSIZE-2])
								(void) error(34);
							continue;
						}
					}
					/* c is first byte of a multi-byte character */
					len = 1;
					str[0] = c;
					while (mblen(str, MB_CUR_MAX) != len) {
						if (++len > MB_CUR_MAX)
							error(52);
						str[len-1] = getchr();
					}
					for (i=0; i<len; i++)
						*gp++ = str[i];
					if (gp >= &globuf[GBSIZE-2])
						(void) error(34);
				}
				*gp++ = '\n';
				*gp++ = 0;
				nfirst = 1;
			}
			else
				if ((c=getchr()) != '\n')
					(void) error(54);
			globp = globuf;
			if (nfirst) {
				globflg = 1;
				commands();
				globflg = 0;
			}
			else (void) error(56);
			globp = 0;
			a1 = zero;
		}
	}
}

int eopen(char *string, int rw)
{
#define w_or_r(a,b) (rw?a:b)
	int pf[2];
	int i;
	int fio;
	int chcount;    /* # of char read. */
	int crflag;
#ifdef CRYPT
	char *fp;
#endif
	crflag = 0;      /* Is file encrypted flag; 1=yes. */
	if (rflg) {     /* restricted shell */
		if (Xqt) {
			Xqt = 0;
			(void) error(6);
		}
	}
	if(!Xqt) {
		if((fio=open(string, rw)) >= 0) {
			if (fflg) {
				chcount = read(fio,funny,LBSIZE);
#ifdef CRYPT
/* Verify that line just read IS an encrypted file. */
/* This seems to be a heuristic; if the user specified -x on a file */
/* that is not encrypted, ed will politely ignore it.  Under NLS,   */
/* the file will be assumed to be encrypted if it contains NLS chars. */
/* Not knowing the properties of encryption it is hard to know a better */
/* way to do this analysis. */
			fp = funny; /* Set fp to start of buffer. */
			while(fp < &funny[chcount])
				if(*fp++ & 0200)crflag = 1;
/* If is is encrypted, & -x option was used, & key is not null, decode it. */
		if(crflag & xflag & kflag)crblock(perm,funny,chcount,0L);
#endif
				if (fspec(funny,&fss,0) < 0) {
					fss.Ffill = 0;
					fflg = 0;
					(void) error(4);
				}
				lseek(fio,0L,0);
			}
		}
		fflg = 0;
		return(fio);
	}
	if(pipe(pf) < 0)
xerr:           (void) error(0);
	if((i = fork()) == 0) {
		(void) signal(SIGHUP, oldhup);
		(void) signal(SIGQUIT, oldquit);
		(void) signal(SIGPIPE, oldpipe);
		(void) signal(SIGINT, SIG_DFL);

		close(w_or_r(pf[1], pf[0]));
		close(w_or_r(STDIN_FILENO, STDOUT_FILENO));
		dup(w_or_r(pf[0], pf[1]));
		close(w_or_r(pf[0], pf[1]));
		execlp(_PATH_SH, "sh", "-c", string, (char *) 0);
		exit(1);
	}
	if(i == -1)
		goto xerr;
	close(w_or_r(pf[0], pf[1]));
	return w_or_r(pf[1], pf[0]);
}

void eclose(int f1)
{
	close(f1);
	if(Xqt)
		Xqt = 0, wait((int *) 0);
}

void getime(void) /* get modified time of file and save */
{
	if (stat(file,&Fl) < 0)
		savtime = 0;
	else
		savtime = Fl.st_mtime;
}

void chktime(void) /* check saved mod time against current mod time */
{
	if (savtime != 0 && Fl.st_mtime != 0) {
		if (savtime != Fl.st_mtime)
			(void) error(58);
	}
}

void newtime(void) /* get new mod time and save */
{
	stat(file,&Fl);
	savtime = Fl.st_mtime;
}

void red(char *op) /* restricted - check for '/' in name */
	/* and delete trailing '/' */
{
	register char *p;

	p = op;
	while(*p)
		if(*p++ == '/' && strncmp(op, "/var/tmp/", 5) && rflg) {
			*op = 0;
			(void) error(6);
		}
	/* delete trailing '/' */
	while(p > op) {
		if (*--p == '/')
			*p = '\0';
		else break;
	}
}

/*
 * Searches thru beginning of file looking for a string of the form
 *	<: values... :>
 *
 * where "values" are
 *
 *	\b	ignored
 *	s<num>	sets the Flim to <num>
 *	t???	sets tab stop stuff
 *	d	ignored
 *	m<num>	ignored
 *	e	ignored
 */

char *fsp, fsprtn;

int fspec(char line3[], struct Fspec *f2, int up)
{
	struct termio arg;
	register int havespec, n;

	if(!up) clear(f2);

	havespec = fsprtn = 0;
	for(fsp=line3; *fsp && *fsp != '\n'; fsp++)
		switch(*fsp) {

			case '<':       if(havespec) return(-1);
					if(*(fsp+1) == ':') {
						havespec = 1;
						clear(f2);
						if(!ioctl(1, TCGETA, &arg) &&
							((arg.c_oflag&TAB3) == TAB3))
						  f2->Ffill = 1;
						fsp++;
						continue;
					}

			case ' ':       continue;

			case 's':       if(havespec && (n=numb()) >= 0)
						f2->Flim = n;
					continue;

			case 't':       if(havespec) targ(f2);
					continue;

			case 'd':       continue;

			case 'm':       if(havespec)  n = numb();
					continue;

			case 'e':       continue;
			case ':':       if(!havespec) continue;
					if(*(fsp+1) != '>') fsprtn = -1;
					return(fsprtn);

			default:        if(!havespec) continue;
					return(-1);
		}
	return(1);
}

int numb(void)
{
	register int n;

	n = 0;
	while(*++fsp >= '0' && *fsp <= '9')
		n = 10*n + *fsp-'0';
	fsp--;
	return(n);
}


void targ(struct Fspec *f3)
{


	if(*++fsp == '-') {
		if(*(fsp+1) >= '0' && *(fsp+1) <= '9') tincr(numb(),f3);
		else tstd(f3);
		return;
	}
	if(*fsp >= '0' && *fsp <= '9') {
		tlist(f3);
		return;
	}
	fsprtn = -1;
	fsp--;
	return;
}


void tincr(int n, struct Fspec *f4)
{
	register int l, i;

	l = 1;
	for(i=0; i<20; i++)
		f4->Ftabs[i] = l += n;
	f4->Ftabs[i] = 0;
}


void tstd(struct Fspec *f5)
{
	char std[3];

	std[0] = *++fsp;
	if (*(fsp+1) >= '0' && *(fsp+1) <= '9')  {
						std[1] = *++fsp;
						std[2] = '\0';
	}
	else std[1] = '\0';
	fsprtn = stdtab(std,f5->Ftabs);
	return;
}


void tlist(struct Fspec *f6)
{
	register int n, last, i;

	fsp--;
	last = i = 0;

	do {
		if((n=numb()) <= last || i >= 20) {
			fsprtn = -1;
			return;
		}
		f6->Ftabs[i++] = last = n;
	} while(*++fsp == ',');

	f6->Ftabs[i] = 0;
	fsp--;
}


int expnd(char line2[], char buf[], int *sz, struct Fspec *f7)
{
	register char *l, *t;
	register int b;

	l = line2 - 1;
	b = 1;
	t = f7->Ftabs;
	fsprtn = 0;

	while(*++l && *l != '\n' && b < 511) {
		if(*l == '\t') {
			while(*t && b >= *t) t++;
			if (*t == 0) fsprtn = -2;
			do buf[b-1] = ' '; while(++b < *t);
		}
		else buf[b++ - 1] = *l;
	}

	buf[b] = '\0';
	*sz = b;
	if(*l != '\0' && *l != '\n') {
		buf[b-1] = '\n';
		return(-1);
	}
	buf[b-1] = *l;
	if(f7->Flim && b-1 > f7->Flim) return(-1);
	return(fsprtn);
}



void clear(struct Fspec *f8)
{
	f8->Ftabs[0] = f8->Fdel = f8->Fmov = f8->Ffill = 0;
	f8->Flim = 0;
}


int lenchk(char line4[], struct Fspec *f9)
{
	register char *l, *t;
	register int b;

	l = line4 - 1;
	b = 1;
	t = f9->Ftabs;

	while(*++l && *l != '\n' && b < 511) {
		if(*l == '\t') {
			while(*t && b >= *t) t++;
			while(++b < *t);
		}
		else b++;
	}

	if((*l!='\0'&&*l!='\n') || (f9->Flim&&b-1>f9->Flim))
		return(-1);
	return(0);
}
#define NTABS 21

/*      stdtabs: standard tabs table
	format: option code letter(s), null, tabs, null */
char stdtabs[] = {
'a',    0,1,10,16,36,72,0,                      /* IBM 370 Assembler */
'a','2',0,1,10,16,40,72,0,                      /* IBM Assembler alternative*/
'c',    0,1,8,12,16,20,55,0,                    /* COBOL, normal */
'c','2',0,1,6,10,14,49,0,                       /* COBOL, crunched*/
'c','3',0,1,6,10,14,18,22,26,30,34,38,42,46,50,54,58,62,67,0,
'f',    0,1,7,11,15,19,23,0,                    /* FORTRAN */
'p',    0,1,5,9,13,17,21,25,29,33,37,41,45,49,53,57,61,0, /* PL/I */
's',    0,1,10,55,0,                            /* SNOBOL */
'u',    0,1,12,20,44,0,                         /* UNIVAC ASM */
0};
/*      stdtab: return tab list for any "canned" tab option.
	entry: option points to null-terminated option string
		tabvect points to vector to be filled in
	exit: return(0) if legal, tabvect filled, ending with zero
		return(-1) if unknown option
*/

int stdtab(char option[], char tabvect[NTABS])
{
	char *scan;
	int	n;

	tabvect[0] = 0;
	scan = stdtabs;
	while (*scan) {
		n = strcmp(scan, option);
		/* set scan to character after null */
		scan += (strlen(scan) + 1);
		if (n == 0) {
			strcpy(tabvect, scan);
			break;
		} else 
			while(*scan++) ;    /* skip over tab specs */
	}

/*      later: look up code in /etc/something */
	return(tabvect[0]?0:-1);
}

/* This is called before a buffer modifying command so that the */
/* current array of line ptrs is saved in sav and dot and dol are saved */
void save(void)
{
	LINE i;

	savdot = dot;
	savdol = dol;
	for (i=zero+1; i<=dol; i++)
		i->sav = i->cur;
	initflg = 0;
}

/* The undo command calls this to restore the previous ptr array sav */
/* and swap with cur - dot and dol are swapped also. This allows user to */
/* undo an undo */
void undo(void) 
{
	int tmp;
	LINE i, tmpdot, tmpdol;

	tmpdot = dot; dot = savdot; savdot = tmpdot;
	tmpdol = dol; dol = savdol; savdol = tmpdol;
	/* swap arrays using the greater of dol or savdol as upper limit */
	for (i=zero+1; i<=((dol>savdol) ? dol : savdol); i++) {
		tmp = i->cur;
		i->cur = i->sav;
		i->sav = tmp;
	}
}
