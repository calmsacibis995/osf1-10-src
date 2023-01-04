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
static char rcsid[] = "@(#)$RCSfile: od.c,v $ $Revision: 4.2.8.6 $ (DEC) $Date: 1993/12/21 21:06:54 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */

/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 * 
 * 1.24  com/cmd/scan/od.c, cmdscan, bos320, 9143320 10/17/91 14:32:24
 *
 */

/*
 * od -- octal, hex, decimal, character dump of data in a file.
 *
 * usage:  od [-abBCcdDefFhHiIlLopPs[n]vw[n]xX] [file] [[+]offset[.][b] [label]]
 *
 * where the option flags have the following meaning:
 *   character	object	radix	signed?
 *	a	byte	(10)	(n.a.)	ASCII named byte stream
 *	b	byte	  8	 no	byte octal
 *	c	byte	 (8)	(no)	character with octal non-graphic bytes
 *	d	short	 10	 no
 *	D	long	 10	 no
 *	e,F	double	(10)		double precision floating pt.
 *	f	float	(10)		single precision floating pt.
 *	h,x	short	 16	 no
 *	H,X	long	 16	 no
 *	i	short	 10	yes
 *	I,l,L	long	 10	yes
 *	o,B	short	  8	 no	(default conversion)
 *	O	long	  8	 no
 *	s[n]	string	 (8)		ASCII graphic strings
 *
 *	p				indicate EVEN parity on 'a' conversion
 *	P				indicate ODD parity on 'a' conversion
 *	v				show all data - don't skip like lines.
 *	w[n]				bytes per display line
 *
 * More than one format character may be given.
 * If {file} is not specified, standard input is read.
 * If {file} is not specified, then {offset} must start with '+'.
 * {Offset} may be HEX (0xnnn), OCTAL (0nn), or decimal (nnn.).
 * The default is octal. The same radix will be used to display the address.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <locale.h>
#include <nl_types.h>
#include "od_msg.h"

nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_OD,Num,Str)

#define DBUF_SIZE	BUFSIZ
#define BIG_DBUF	32
#define NO		0
#define YES		1
#define EVEN	       -1
#define ODD		1
#define UNSIGNED	0
#define SIGNED		1
#define PADDR		1
#define MIN_SLEN	3
#define MAX_CONV	32	/* Limit on number of conversions */

static int	a_put();
static int	b_put();
static int	c_put();
static int	C_put();
static int	do_C_put();
static int	s_put();
static int	us_put();
static int	c1_put();
static int	l_put();
#if defined(__alpha)
static int	q_put();
#endif /* defined(__alpha) */
static int	f_put();
static int	d_put();
static int	st_put();
static int	parity();
static int	canseek();
static void	put_sbuf();
static void	dumbseek();
static void	offset();
static void	pr_sbuf();
static void	put_addr();
static void	line();
static void	usage(char *progname);

struct dfmt {
	int	df_field;	/* external field required for object */
	int	df_size;	/* size (bytes) of object */
	int	df_radix;	/* conversion radix */
	int	df_signed;	/* signed? flag */
	int	df_paddr;	/* "put address on each line?" flag */
	int	(*df_put)();	/* function to output object */
	char	*df_fmt;	/* output string format */
} *conv_vec[MAX_CONV];		/* vector of conversions to be done */

struct dfmt	ascii	= { 3, sizeof (char),   10,        0, PADDR,  a_put, 0};
struct dfmt	byte	= { 3, sizeof (char),    8, UNSIGNED, PADDR,  b_put, 0};
struct dfmt	hbyte	= { 2, sizeof (char),   16, UNSIGNED, PADDR,  b_put, 0};
struct dfmt	cchar	= { 3, sizeof (char),    8, UNSIGNED, PADDR,  c_put, 0};
struct dfmt	hcchar	= { 2, sizeof (char),   16, UNSIGNED, PADDR,  c_put, 0};
struct dfmt	Cchar	= { 3, sizeof (char),    8, UNSIGNED, PADDR,  C_put, 0};
struct dfmt	u_s_oct	= { 6, sizeof (short),   8, UNSIGNED, PADDR, us_put, 0};
struct dfmt	u_c_dec	= { 3, sizeof (char),   10, UNSIGNED, PADDR, c1_put, 0};
struct dfmt	u_c_oct	= { 3, sizeof (char),    8, UNSIGNED, PADDR, c1_put, 0};
struct dfmt	u_s_dec	= { 5, sizeof (short),  10, UNSIGNED, PADDR, us_put, 0};
struct dfmt	u_s_hex	= { 4, sizeof (short),  16, UNSIGNED, PADDR, us_put, 0};
struct dfmt	u_c_hex	= { 2, sizeof (char),   16, UNSIGNED, PADDR, c1_put, 0};
struct dfmt	u_l_oct	= {11, sizeof (int ),    8, UNSIGNED, PADDR,  l_put, 0};
struct dfmt	u_l_dec	= {10, sizeof (int ),   10, UNSIGNED, PADDR,  l_put, 0};
struct dfmt	u_l_hex	= { 8, sizeof (int ),   16, UNSIGNED, PADDR,  l_put, 0};
#if defined(__alpha)
struct dfmt	u_q_hex = {16, sizeof (long),   16, UNSIGNED, PADDR,  q_put, 0};
#endif /* defined(__alpha) */
struct dfmt	s_c_dec	= { 3, sizeof (char),   10,   SIGNED, PADDR, c1_put, 0};
struct dfmt	s_s_dec	= { 6, sizeof (short),  10,   SIGNED, PADDR,  s_put, 0};
struct dfmt	s_l_dec	= {11, sizeof (int ),   10,   SIGNED, PADDR,  l_put, 0};
struct dfmt	flt	= {14, sizeof (float),  10,   SIGNED, PADDR,  f_put, 0};
struct dfmt	dble	= {21, sizeof (double), 10,   SIGNED, PADDR,  d_put, 0};
struct dfmt	string	= { 0,               0,  8,        0,    NO, st_put, 0};


char	dbuf[DBUF_SIZE];		/* input buffer */
char	mbuf[DBUF_SIZE];		/* buffer used if 2-byte crosses line*/
char	lastdbuf[DBUF_SIZE];
int	addr_base	= 8;		/* default address base is OCTAL */
int	base		= -1;
long	addr		= 0L;		/* current file offset */
long	label		= -1L;		/* current label; -1 is "off" */
int	dbuf_size	= 16;		/* file bytes / display line */
int	_parity		= NO;		/* show parity on ascii bytes */
char	fmt[]	= "                        %s";	/* 24 blanks */

/* we must save global values and reset them for each call to double byte
 * routines.  For example if you call -CCCCCC, you want print the same line
 * six times.
 */
int straggle = 0;			/* Did word cross line boundary?*/
int straggle_save = 0;
int nls_shift_save = 0;			/* last character was a shift char */
int nls_skip_save = 0;			/* need to skip this byte          */
int already_read = 0;			/* next buffer already read in.    */
int nls_skip = 0;
int changed = 0;
int nls_shift= 0;
int Aflag = 0;
int jflag = 0;
static char	*icvt();
static char	*scvt();
static char	*underline();		/* underline because of parity    */
static long	get_addr();

#ifndef _SBCS
#undef	isdigit
#undef	isprint
#define isdigit(c)	iswdigit(c)
#define isprint(c)	iswprint(c)
#endif

int  bytes;
int  hdflg;		/* Set if command name = hd */
char pname[] = "od" ;	/* Program name		    */
int  mbmax	    ;	/* = MB_CUR_MAX		    */

extern int isatty(int);
extern int mbswidth( char *, size_t );

void
main(argc, argv)
int	argc;
char	**argv;
{
	register char *p;
	register char *l;
	register same;
	struct dfmt	*d = NULL;
	struct dfmt	**cv = conv_vec;
	int	showall = NO;
	int	field, llen, nelm;
	int	max_llen = 0;
	int 	max_bytes = 0;
	int 	Nflag = 0;
	int	tflag = 0;
	int	posixflag = 0;
	char 	*ptr;
	char 	*addr_ptr;
	int	xbytes;
	extern 	char *optarg;		/* getopt support */
	extern	int optind;
	int	c;
	int sflag=0, snumber=0, svalue=0;
	int wflag=0, wnumber=0, wvalue=0;

	(void) setlocale (LC_ALL,"");
	catd  = catopen(MF_OD,NL_CAT_LOCALE);
	errno = 0		;
	mbmax = MB_CUR_MAX	;
	/*
	 * Check if command name = hd and set variables accordingly
	 */
	if ((ptr = strrchr(argv[0], '/')) == NULL)
		ptr = argv[0] ;
	else
		ptr++ ;	/* Skip the last '/' */
	if (strcmp(ptr, "hd") == 0) {
		*pname	  = 'h' ;
		addr_base = 16  ;
		hdflg	  = 1   ;
	}
       /*  
	* Parse arguments and set up conversion vector
	* 
	* A tad bit of work to make the old -s[n] and -w[n] syntax
	* work with getopts.
	*/

	while ((c=getopt(argc,argv,
		":A:abBCcDdeFfhHiIj:lLN:oOpPQst:wvxX?0123456789")) != -1) {

	if (c=='w') wnumber=1;
	else if (wnumber && !isdigit(c)) wnumber = 0;
	if (c=='s') snumber=1;
	else if (snumber && !isdigit(c)) snumber = 0;

	switch(c)
		{
	       /*
		* Specify the input offset base 
		*/
		case 'A':
			Aflag = 1;
			switch(*optarg)
			{
			case 'd':
				addr_base = 10;
				break;
			case 'o':
				addr_base = 8;
				break;
		       /*
			* Do not display labels
			*/
			case 'n':
				Aflag = 2;
				break;
			case 'x':
				addr_base = 16;
				break;
			default:
				(void)fprintf(stderr, MSGSTR(AOPT, "-A option only accepts the following:  d, o, n, and x\n"));
				usage(pname);
			}
			continue;
		case 'a':
			d = &ascii;
			break;
		case 'b':
			d = hdflg ? &hbyte : &byte;
			break;
		case 'C':
			d = &Cchar;
			break;
		case 'c':
			d = hdflg ? &hcchar : &cchar;
			break;
		case 'd':
			d = &u_s_dec;
			break;
		case 'D':
			d = &u_l_dec;
			break;
		case 'e':
		case 'F':
			d = &dble;
			break;
		case 'f':
			d = &flt;
			break;
		case 'h':
		case 'x':
			d = &u_s_hex;
			break;
		case 'H':
		case 'X':
			d = &u_l_hex;
			break;
#if defined(__alpha)
		case 'Q':
			d = &u_q_hex;
			break;
#endif /* defined(__alpha) */
		case 'i':
			d = &s_s_dec;
			break;
		case 'I':
		case 'l':
		case 'L':
			d = &s_l_dec;
			break;
	       /*
		* Specify number of bytes to interpret
		*/
		case 'N':
			Nflag++;
			max_bytes = strtol(optarg, &ptr, 0);
			if ((max_bytes <= 0) || errno || (optarg == ptr))   {
				(void)fprintf(stderr, MSGSTR(INVNUM, "Invalid number of bytes to interpret\n"));
				usage(pname);
			}
			continue;
		case 'o':
		case 'B':
			d = &u_s_oct;
			break;
		case 'O':
			d = &u_l_oct;
			break;
		case 'p':
			_parity = EVEN;
			continue;
		case 'P':
			_parity = ODD;
			continue;
	       /*
		* Specify number of bytes to skip
		*/
		case 'j':
			jflag++;
			addr_ptr = optarg;
			continue;
		case 's':
			if (sflag) 
				continue ; /* Set before, skip extra 's' */
			sflag++;
			d = &string;
			showall = YES;
			break;
	       /*
		* Parse type_string
		*/
		case 't':
			tflag++;
			p = optarg;
			for (; *p; p++) {
				switch(*p) {
					case 'a':
						d = &ascii;
						break;
					case 'c':
						d = &cchar;
						break;

				   /*
					*  Type specifier d may be followed
					*  by a letter or number indicating 
					*  the number
					*  of bytes to be transformed.  If
					*  invalid, use default.
					*/
					case 'd':
						d = &s_l_dec;
						if (isupper(*(p+1)) || isdigit(*(p+1))) {
							switch (*(++p)) {
							case '1':
							case 'C':
								d = &s_c_dec;
								break;
							case '2':
							case 'S':
								d = &s_s_dec;
								break;
							case '4':
							case 'I':
								break;
							case 'L':
								d = &s_l_dec;
								break;
							default:
								(void)fprintf(stderr, MSGSTR(DOPT, "d may only be followed with C, S, I, L, 1, 2, or 4\n"));
								break;
							}
						} 
						break;

				   /*
					*  Type specifier f may be followed
					*  by F, D, or L indicating that
					*  coversion should be applied to 
					*  an item of type float, double
					*  or long double.  OR a number
					*  indicating the number of bytes.
					*/
					case 'f':
						d = &dble;
						if (isupper(*(p+1)) || isdigit(*(p+1))) {
							switch (*(++p)) {
							case '4':
							case 'F':
								d = &flt;
								break;
							case '8':
							case 'D':
								break;
							case 'L':
								d = &dble;
								break;
							default:
								(void)fprintf(stderr, MSGSTR(FOPT, "f may only be followed with F, D, L, 4, or 8\n"));
								break;
							}
						} 

						break;

				   /*
					*  Type specifier o may be followed
					*  by a letter or a number indicating the number
					*  of bytes to be transformed.  If
					*  invalid, use default.
					*/
					case 'o':
						d = &u_l_oct;
						if (isupper(*(p+1)) || isdigit(*(p+1))) {
							switch (*(++p)) {
							case '1':
							case 'C':
								d = &u_c_oct;
								break;
							case '2':
							case 'S':
								d = &u_s_oct;
								break;
							case '4':
							case 'I':
								break;
							case 'L':
								d = &u_l_oct;
								break;
							default:
								(void)fprintf(stderr, MSGSTR(OOPT, "o may only be followed with C, S, I, L, 1, 2, or 4\n"));
								break;
							}
						} 

						break;

				   /*
					*  Type specifier u may be followed
					*  by a letter or a number indicating the number
					*  of bytes to be transformed.  If
					*  invalid, use default.
					*/
					case 'u':
						d = &u_l_dec;
						if (isupper(*(p+1)) || isdigit(*(p+1))) {
							switch (*(++p)) {
							case '1':
							case 'C':
								d = &u_c_dec;
								break;
							case '2':
							case 'S':
								d = &u_s_dec;
								break;
							case '4':
							case 'I':
								break;
							case 'L':
								d = &u_l_dec;
								break;
							default:
								(void)fprintf(stderr, MSGSTR(UOPT, "u may only be followed with C, S, I, L, 1, 2, or 4\n"));
							}
						} 

						break;

				   /*
					*  Type specifier x may be followed
					*  by a letter or a number indicating the number
					*  of bytes to be transformed.  If
					*  invalid, use default.
					*/
					case 'x':
						d = &u_l_hex;
						if (isupper(*(p+1)) || isdigit(*(p+1))) {
							switch (*(++p)) {
							case '1':
							case 'C':
								d = &u_c_hex;
								break;
							case '2':
							case 'S':
								d = &u_s_hex;
								break;
							case '4':
							case 'I':
								break;
							case 'L':
								d = &u_l_hex;
								break;
							default:
								(void)fprintf(stderr, MSGSTR(XOPT, "x may only be followed with C, S, I, L, 1, 2, or 4\n"));
							}
						} 

						break;
					default:
						usage(pname);
					}
					if ( (cv - conv_vec) < MAX_CONV)
					    *(cv++) = d;
					else {
					    fprintf(stderr,MSGSTR(TOOMANY,"Too many conversions specified.\n"));
					    exit(1);
					}
				}
				continue;
		case 'w':
			wflag = 1;
			continue;
		case 'v':
			showall = YES;
			continue;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (snumber) {
				svalue = (svalue * 10) + (c - '0');
				if (svalue < 0) {
				    fprintf(stderr,
					    MSGSTR(TOOBIG,"-%c value too large\n"),
					    's'
					    );
				    exit(1);
				}
			} else if (wnumber) {
				wvalue = (wvalue * 10) + (c - '0');
				if (wvalue < 0) {
				    fprintf(stderr,
					    MSGSTR(TOOBIG,"-%c value too large\n"),
					    'w'
					    );
				    exit(1);
				}
			} else {
				fprintf(stderr, MSGSTR(BADFLAG, "%s: -%c is not a recognized flag.\n"), pname, c) ;
				usage(pname);
			}
			continue;

		default:
			fprintf(stderr, MSGSTR(BADFLAG, "%s: -%c is not a recognized flag.\n"), pname, c) ;
		case '?':
			usage(pname);
		}
		if (cv - conv_vec < MAX_CONV)
		    *(cv++) = d;
		else {
		    fprintf(stderr, MSGSTR(TOOMANY, "Too many conversions specified\n"));
		    exit(1);
		}
	} /* while getopt */

	if (sflag) 
		string.df_size = (svalue == 0 ? MIN_SLEN : svalue);
	if (wflag)
		dbuf_size = (wvalue == 0 ? BIG_DBUF : (wvalue > DBUF_SIZE)? DBUF_SIZE : wvalue);
		
	if (Aflag || jflag || Nflag || tflag)
		posixflag++;			/* Filename can begin w/ '+' */

	argv += optind;
	argc -= optind;

	/*
	 * if nothing spec'd, setup default conversion.
	 */
	if (cv == conv_vec)
		*(cv++) = &u_s_oct;

	*cv = (struct dfmt *)0;

	/*
	 * calculate display parameters
	 */
	for (cv = conv_vec; d = *cv; cv++)
	{
		if (d->df_size == 0)
			continue ;	/* Skip this entry if size is 0 */

		nelm = (dbuf_size + d->df_size - 1) / d->df_size;
		llen = nelm * (d->df_field + 1);
		if (llen > max_llen)
			max_llen = llen;
	}

	/*
	 * Parse once again to make sure that max_llen will always be an
	 * integer multiple of the nelm.
	 */
adjust_len:
	for (cv = conv_vec; d = *cv; cv++)
	{
		if (d->df_size == 0)
			continue ;	/* Skip this entry if size is 0 */

		nelm = (dbuf_size + d->df_size - 1) / d->df_size;
		if (max_llen % nelm != 0) {
			max_llen = nelm * (max_llen / nelm + 1) ;
			goto adjust_len ;
		}
	}

	/*
	 * setup df_fmt to point to uniform output fields.
	 */
	for (cv = conv_vec; d = *cv; cv++)
	{	/* only if external field is known */
		if (d->df_field && d->df_size)
		{
			nelm = (dbuf_size + d->df_size - 1) / d->df_size;
			field = max_llen / nelm;
			d->df_fmt = fmt + 24 - (field - d->df_field);
		}
	}

       /*
 	* Check whether input file specified.  If so, freopen 
 	* stdin as new file.  If any of the POSIX flags are specified,
	* (-A, -j, -N, -t) a filename may start with a +.
 	*/
	if (argc > 0 && ((**argv != '+') || posixflag))
	{
		if (freopen(*argv, "r", stdin) == NULL)
		{
			perror(*argv);
			exit(1);
		}
		argv++;
		argc--;
	}

       /*
	*  Advance into the file the number of bytes
	*  specified via -S, -j (XPG4)
	*/
	if (jflag) {
		addr = get_addr(addr_ptr);
		offset(addr, &argc, &argv);
		addr = 0;
	}

       /*
	*  If 'Old Style' look for offset beginning with a 
	*  + or a digit
	*/
	else if (argc > 0 && ((**argv == '+' || isdigit(**argv)) && !(posixflag)))
	{
		addr = get_addr(*argv);
		offset(addr, &argc, &argv);
		argv++;
		argc--;

		if (argc > 0 && (**argv == '+' || isdigit(**argv)))
		{
			label = get_addr(*argv);
			argv++;
			argc--;
		}
	}

       /* 
	*  Process either file or stdin.  Open new files and
	*  continue processing as necessary.
	*/
	while (argc >= 0)
	{
	       /*
	 	* main dump loop
	 	*/
		same = -1;
		while (already_read || (bytes = fread((void *)dbuf, (size_t)1, (size_t)dbuf_size, stdin)) > 0)
		{
			register int saved_bytes = bytes ;

			if (already_read)
			{
				already_read = 0;
				(void)memcpy (dbuf,mbuf,dbuf_size);
			}
			dbuf[saved_bytes] = '\0' ;

		       /*
			*  If more than one file is specified and
			*  the current file does not fill the buffer,
			*  open the next file and continue processing.
			*/
			while ((saved_bytes < dbuf_size) && (argc > 0)) 
			{
				if (freopen(*argv, "r", stdin) == NULL)
				{
					perror(*argv);
					exit(1);
				}
				ptr = dbuf + saved_bytes;
	        		if ((xbytes = fread((void *)ptr, (size_t)1, (size_t)(dbuf_size-saved_bytes), stdin)) > 0) {
					saved_bytes    += xbytes ;
					dbuf[saved_bytes] = '\0' ;
				}
				argv++;
				argc--;
			}
			/* Clear unused bytes in buffer */
			if (saved_bytes < dbuf_size)
				bzero(&dbuf[saved_bytes],
				       dbuf_size - saved_bytes) ;

			if (same>=0 && bcmp(dbuf, lastdbuf, dbuf_size) == 0 && !showall)
			{
				if (same==0)
				{
					(void)printf("*\n");
					same = 1;
				}
			}
			else
			{
			       /*
				* If Nflag is specified, only print
				* max_bytes bytes
				*/
				if (!Nflag) 
					line(saved_bytes);
	
				else {
					if (max_bytes > saved_bytes) {
						line(saved_bytes);
						max_bytes -= saved_bytes;
					}
					else {
						/* Clear extra bytes */
						bzero(&dbuf[max_bytes],
						      saved_bytes - max_bytes) ;
						line(max_bytes);
						addr += max_bytes;
						if (label >= 0)
							label += max_bytes;
						put_addr(addr, label, '\n');
						exit(0);
					}
				}
	
				same = 0;
				p = dbuf;
				l = lastdbuf;
				for (nelm = 0; nelm < dbuf_size; nelm++)
				{
					*l++ = *p;
					*p++ = '\0';
				}
			}
			addr += saved_bytes;
			if (label >= 0)
				label += saved_bytes;
	
		}

	       /*
		* If there are more files, open and
		* continue processing.
		*/
		if (argc == 0)
			break;
		else 
		{
			if (freopen(*argv, "r", stdin) == NULL)
			{
				perror(*argv);
				exit(1);
			}
			argv++;
			argc--;
		}
	}
	/*
	 * Some conversions require "flushing".
	 */
	bytes = 0;
	for (cv = conv_vec; *cv; cv++)
	{
		if ((*cv)->df_paddr)
		{
			if (bytes++ == 0) {
			       /* 
				* If -An is specified, don't print labels
				*/
				if (Aflag == 2) 
					fputs("\n", stdout);
				else if (argc <= 0)
					put_addr(addr, label, '\n');
			}
		}
		else 
			(*((*cv)->df_put))(0, *cv);
	}
	exit(0);
}

/*
 * NAME: put_addr
 *                                                                    
 * FUNCTION:  Print out the current file address.
 *                                                                    
 */  

static void
put_addr(addrs, labl, chr)
long	addrs;
long	labl;
char	chr;
{
	(void)fputs(icvt(addrs, addr_base, UNSIGNED, 7), stdout);
	if (labl >= 0)
		(void)printf(" (%s)", icvt(labl, addr_base, UNSIGNED, 7));
	putchar(chr);
	
}

/*
 * NAME: line
 *                                                                    
 * FUNCTION:  When line is called we have determined the line is different
 *		from the previous line.  We then print it out in all the
 *		formats called for in the command line.
 *                                                                    
 */  
static void
line(n)
int	n;
{
	register i, first;
	register struct dfmt *c;
	register struct dfmt **cv = conv_vec;
	int 	newstraggle = 0,
		newnls_skip = 0,
		newnls_shift =0;

	straggle_save = straggle;
	nls_shift_save = nls_shift;
	nls_skip_save = nls_skip;
	first = YES;
	while (c = *cv++)
	{
		straggle = straggle_save;
		nls_shift = nls_shift_save;
		nls_skip = nls_skip_save;
		if (straggle == 1)
			straggle = 2 ;	/* MB chars wrapped to beginning of 
					   next line */
		if (c->df_paddr)
		{
			if (first)
			{
			       /* 
				* If -An is specified, don't print labels
				*/
				if (Aflag == 2)
					(void)fputs("\t", stdout);
				else
					put_addr(addr, label, ' ');
				first = NO;
			}
			else
			{
				putchar('\t');
				if (label >= 0)
					fputs("\t  ", stdout);
			}
		}
		i = 0;
		changed = 0;
		while (i < n)
			i += (*(c->df_put))(dbuf+i, c);

		if (changed) {
			newstraggle |= straggle;
			newnls_shift |= nls_shift;
			newnls_skip |= nls_skip;
		}

		if (c->df_paddr)
			putchar('\n');
	}
	straggle = newstraggle;
	nls_skip = newnls_skip;
	nls_shift = newnls_shift;
}

/*
 * NAME: s_put
 *                                                                    
 * FUNCTION: Print out a signed short.
 *                                                                    
 * RETURN VALUE DESCRIPTION:  size of a short.
 *			    
 */  

static int
s_put(n, d)
short	*n;
struct dfmt	*d;
{
	(void)printf(d->df_fmt, icvt((long)*n, d->df_radix, d->df_signed, d->df_field));
	return(d->df_size);
}

/*
 * NAME: us_put
 *                                                                    
 * FUNCTION: Print out an unsigned short in the "d" base.  hex, oct, dec.
 *                                                                    
 * RETURN VALUE DESCRIPTION: size of a short
 *			    
 */  
static int
us_put(n, d)
unsigned short	*n;
struct dfmt	*d;
{
	(void)printf(d->df_fmt, icvt((long)*n, d->df_radix, d->df_signed, d->df_field));
	return(d->df_size);
}

/*
 * NAME: l_put
 *                                                                    
 * FUNCTION:  print out an long.  -D -H -X -O options
 *                                                                    
 * RETURN VALUE DESCRIPTION:  size of a long.
 *			    
 */  
static int
l_put(n, d)
int		*n;
struct dfmt	*d;
{
	(void)printf(d->df_fmt, icvt(*n, d->df_radix, d->df_signed, d->df_field));
	return(d->df_size);
}

#if defined(__alpha)
/*
 * NAME: q_put
 *
 * FUNCTION:  print out an quad.  -Q options
 *
 * RETURN VALUE DESCRIPTION:  size of a quad.
 *
 */
static
q_put(n, d)
long    *n;
struct dfmt     *d;
{
       (void)printf(d->df_fmt, icvt(*n, d->df_radix, d->df_signed, d->df_field));
       return(d->df_size);
}
#endif /* defined(__alpha) */

/*
 * NAME: d_put
 *                                                                    
 * FUNCTION: print out a double  -E -F option.
 *                                                                    
 * RETURN VALUE DESCRIPTION: size of a double
 *			    
 */  
static int
d_put(f, d)
double	*f;
struct dfmt *d;
{
	char fbuf[24];
	struct l { long n[2]; };

#if	vax
	if ((((struct l *)f)->n[0] & 0xff00) == 0x8000)	/* Vax illegal f.p. */
		(void)sprintf(fbuf, "    %08x %08x",
			((struct l *)f)->n[0], ((struct l *)f)->n[1]);
	else
#endif

		(void)sprintf(fbuf, "%21.14e", *f);
	(void)printf(d->df_fmt, fbuf);
	return(d->df_size);
}

/*
 * NAME: f_put
 *                                                                    
 * FUNCTION: print out a floating point. in D format.
 *                                                                    
 * RETURN VALUE DESCRIPTION: size of a float.
 *			    
 */  
static int
f_put(f, d)
float	*f;
struct dfmt *d;
{
	char fbuf[16];

#if	vax
	if ((*(long *)f & 0xff00) == 0x8000)	/* Vax illegal f.p. form */
		(void)sprintf(fbuf, "      %08x", *(long *)f);
	else
#endif
		(void)sprintf(fbuf, "%14.7e", *f);
	(void)printf(d->df_fmt, fbuf);
	return(d->df_size);
}


char	asc_name[34][4] = {
/* 000 */	"nul",	"soh",	"stx",	"etx",	"eot",	"enq",	"ack",	"bel",
/* 010 */	" bs",	" ht",	" nl",	" vt",	" ff",	" cr",	" so",	" si",
/* 020 */	"dle",	"dc1",	"dc2",	"dc3",	"dc4",	"nak",	"syn",	"etb",
/* 030 */	"can",	" em",	"sub",	"esc",	" fs",	" gs",	" rs",	" us",
/* 040 */	" sp",	"del"
};

/*
 * NAME: a_put
 *                                                                    
 * FUNCTION:  print out value in ascii, using known values for unprintables.
 *                                                                    
 * RETURN VALUE:  1
 *			    
 */  
static int
a_put(cc, d)
char	*cc;
struct dfmt *d;
{
	int c = *cc;
	char	s[4] = {' ', ' ', ' ', 0 };	/* 3 spaces, NUL terminated */

	register pbit = parity((int)c & 0377);

	c &= 0177;
	if (isgraph(c))
	{
		s[2] = c;
		if (pbit == _parity)
			(void)printf(d->df_fmt, underline(s));
		else
			(void)printf(d->df_fmt, s);
	}
	else
	{
		if (c == 0177)
			c = ' ' + 1;
		if (pbit == _parity)
			(void)printf(d->df_fmt, underline(asc_name[c]));
		else
			(void)printf(d->df_fmt, asc_name[c]);
	}
	return(1);
}

/*
 * NAME: parity
 *                                                                    
 * FUNCTION: return the parity of a given word.
 *                                                                    
 * RETURN VALUE:  	ODD - Odd parity
 *			EVEN - Even parity
 */  
static int
parity(word)
int	word;
{
	register int p = 0;
	register int w = word;
	register int hw;

	if (w)
		do
		{
			p ^= 1;
			hw = (~(-w));	/* lint complains about undefined   */
		} while(w &= hw);	/* evaluation order for w &=(~(-w)) */
	return (p? ODD:EVEN);
}

/*
 * NAME: underline
 *                                                                    
 * FUNCTION: underline a given string.
 *                                                                    
 * RETURN VALUE:  return a pointer to the underlined string.
 *			    
 */  
static char *
underline(s)
char	*s;
{
	static char ulbuf[16];
	register char *u = ulbuf;

	while (*s)
	{
		if (*s != ' ')
		{
			*u++ = '_';
			*u++ = '\b';
		}
		*u++ = *s++;
	}
	*u = '\0';
	return(ulbuf);
}

/*
 * NAME: b_put
 *                                                                    
 * FUNCTION:  print out a byte in octal
 *                                                                    
 * RETURN VALUE:  1
 *			    
 */  
static int
b_put(b, d)
char	*b;
struct dfmt *d;
{
	(void)printf(d->df_fmt, icvt((long)*b & 0377, d->df_radix, d->df_signed, d->df_field));
	return(1);
}

/*
 * NAME: C_put
 *                                                                    
 * FUNCTION: print out a NLS character in ascii escape sequences.
 *                                                                    
 * RETURN VALUE:  number of bytes in character (usually 1)
 *			    
 */  

static int 
C_put(cc, d)
char	*cc;
struct dfmt *d;
{
	return (do_C_put (cc,d,1));
}

/*
 * NAME: do_C_put
 *                                                                    
 * FUNCTION: print out a value in ascii or ascii escape sequences.
 *                                                                    
 * RETURN VALUE:  number of bytes in character (usually 1)
 *			    
 */  
static int 
do_C_put(cc, d,doesq)
unsigned char	*cc;
struct dfmt *d;
int doesq;
{
	register char	*s;
	register int	n;
	register int	c = *cc;
	char buffer[8];
	char buffer2[32];
	int mbcnt;
	int mboff;	/* Offset of first read ahead byte */
	int mbufcnt = 0;
	int i;

	changed++;
	mbcnt = mblen((const char *)cc, mbmax);
	if ( (!nls_shift) && (mbcnt != 1 && mbcnt != 0) )
	  /*
	   * Either found the start of a multibyte character (mbcnt > 1)
	   * or an illegal sequence, _OR_ we have a character crossing a
	   * block-boundary (and need to read some more in...)
	   */
	{
		char mbchar[MB_LEN_MAX+1];
		int mbl = 1;


		nls_shift += (mbcnt > 0)? (mbcnt - 1) : 0;
		nls_skip += (mbcnt > 0)? (mbcnt - 1) : 0;
		mbchar[0] = c;

loop:
		if (cc[mbl] == '\0') /* Was end of buffer, get some more */
		{
			straggle = 1 ;
			if (!already_read)
			{
				mboff = mbl ;
				if ((bytes = fread((void *)mbuf, (size_t)1, (size_t)dbuf_size, stdin)) <= 0)
				{
				/* Only part of a double byte character found. */
					straggle = 0;
					already_read = 0;
					mbchar[mbl] = 0;
					mbcnt = mbl;
				}
				else
				{
					already_read++;
					mbchar[mbl++] = mbuf[mbufcnt++];
					mbuf  [bytes] = '\0' ;
				}
			}
			else {
				mbchar[mbl++] = mbuf[mbufcnt++];
				if (mbl > bytes + mboff) {
					mbchar[mbl] = '\0';
					mbcnt = nls_skip = nls_shift = mbl;
					goto next;
				}
			}
		}
		 else
			mbchar[mbl] = cc[mbl++];

		mbchar[mbl] = '\0';
		mbcnt = mblen(mbchar, MB_CUR_MAX); 
		if(!nls_shift)
			nls_shift += (mbcnt > 0)? (mbcnt - 1) : 0;
		if(!nls_skip)
			nls_skip += (mbcnt > 0)? (mbcnt - 1) : 0;
		/*
		 *  we must handle the case if the
		 *  multi byte char is split on a line
		 *  boundry, mblen will return -1, but
		 *  we have to check next char to see if
		 *  it's part of multi byte char therefore
		 *  we go to loop.
		 */
		if (mbcnt == -1) {
			if (mbl < MB_CUR_MAX)
				goto loop;
			else {
				mbcnt = 1;
				mbchar[mbcnt] = '\0';
				nls_skip = 0;
				nls_shift = 0;
				}
			}
next:
		if (doesq)
		{
			bzero(buffer, sizeof(buffer));
			bzero(buffer2, sizeof(buffer2));
			strcpy(buffer2,"\\");
			strcat(buffer2,"<");
			for (i=0; i<mbcnt; i++) {
				sprintf(buffer, "%2.2x", (uchar_t)mbchar[i]); 
				strcat(buffer2, buffer);
				}
			strcat(buffer2, ">");
			s = buffer2;
		}
		else
			s = mbchar;

	}
	else if (nls_skip)
	{
		nls_skip = (nls_skip > 0)? nls_skip - 1: 0;
		nls_shift = (nls_shift > 0)? nls_shift - 1: 0;
		if (!doesq) 
			printf(d->df_fmt, (d->df_field == 2) ? "**" : " **") ;
		else if (straggle == 2)
			printf(d->df_fmt, (d->df_field == 2) ? "  " : "   ") ;
		if (nls_skip == 0)
			straggle = 0;
		return (1);
	}
	else
	{
		s = scvt(c, d);
	}
	for (n = d->df_field - mbswidth(s, strlen(s)); n > 0; n--)
		putchar(' ');
	(void)printf(d->df_fmt, s);
	if (doesq && !straggle && (n < 0)) {
		/*
		 * Calculate the number of spaces to be padded between
		 * successive characters.
		 */
		n += nls_skip * (d->df_field + 1) ;
		while (--n >= 0)
			putchar(' ') ;
	}

	return(1) ;
}

/*
 * NAME: c_put
 *                                                                    
 * FUNCTION: display value as character.
 *                                                                    
 * RETURN VALUE: 1
 *			    
 */  
static int
c_put(cc, d)
char	*cc;
struct dfmt *d;
{
	return (do_C_put(cc,d,0));
}

/*
 * NAME: c1_put
 *                                                                    
 * FUNCTION: display value as signed decimal number.
 *                                                                    
 * RETURN VALUE: 1
 *			    
 */  
static int
c1_put(cc, d)
char	    *cc;
struct dfmt *d ;
{
	if (d->df_signed == UNSIGNED)
	    printf(d->df_fmt, icvt(*cc & 0xff, d->df_radix, d->df_signed,
	    	   d->df_field));
	else
	    printf(icvt(*cc, d->df_radix, d->df_signed, d->df_field + 1));

	return(1) ;
}

/*
 * NAME: scvt
 *                                                                    
 * FUNCTION:  convert the character to a representable string.
 *                                                                    
 * RETURN VALUE:  A pointer to the string.
 *			    
 */  
static char *scvt(c, d)
int	c;
struct dfmt	*d;
{
	static char s[2];

	switch(c)
	{
		case '\0':
			return("\\0");

#ifdef __STDC__
		case '\a':
			return("\\a");
#else
		case '\007':
			return("\007");
#endif 

		case '\b':
			return("\\b");

		case '\f':
			return("\\f");

		case '\n':
			return("\\n");

		case '\r':
			return("\\r");

		case '\t':
			return("\\t");

		case '\v':
			return("\\v");

		default:
			if (isprint(c))
			{
				s[0] = c; /* Depends on "s" being STATIC initialized to zero */
				return(s);
			}
			return(icvt((long)c, d->df_radix, d->df_signed, d->df_field));
	}
}

/*
 * Look for strings.
 * A string contains bytes > 037 && < 177, and ends with a null.
 * The minimum length is given in the dfmt structure.
 */

#define CNULL		'\0'
#define S_EMPTY	0
#define S_FILL	1
#define	S_CONT	2
#define SBUFSIZE	1024

#undef	isstring
#define	isstring(c)	((((c) > 037) && ((c) < 0177)) || \
			(((c) != 013) && ((c) > 007) && ((c) <016)))

static char	str_buf[SBUFSIZE];
static int	str_mode = S_EMPTY;
static char	*str_ptr;
static long	str_addr;
static long	str_label;

/*
 * NAME: st_put
 *                                                                    
 * FUNCTION:  print a string if it is at least d characters long.
 *                                                                    
 * RETURN VALUE:  1
 *			    
 */  
static int 
st_put(cc, d)
char	*cc;
struct dfmt	*d;
{
	register int	c ;

	if (cc == 0)
	{
		pr_sbuf(d, YES);
		return(1);
	}

	c = *(unsigned char *)cc;

	if (str_mode & S_FILL)
	{
		if (isstring(c))
			put_sbuf(c, d);
		else
		{
			*str_ptr = CNULL;
			if ( !c )		/* Null string */
				pr_sbuf(d, YES);
			str_mode = S_EMPTY;
		}
	}
	else if (isstring(c))
	{
		str_mode = S_FILL;
		str_addr = addr + (cc - dbuf);	  /* ugly */
		if ((str_label = label) >= 0)
			str_label += (cc - dbuf); /*  ''  */
		str_ptr = str_buf;
		put_sbuf(c, d);
	}
	return(1);
}

static void
put_sbuf(c, d)
int	c;
struct dfmt	*d;
{
	*str_ptr++ = c;
	if (str_ptr >= (str_buf + SBUFSIZE))
	{
		pr_sbuf(d, NO);
		str_ptr = str_buf;
		str_mode |= S_CONT;
	}
}

/*
 * NAME: pr_sbuf
 *                                                                    
 * FUNCTION: print out the string buffer.
 */  

static void
pr_sbuf(d, end)
struct dfmt	*d;
int	end;
{
	register char	*p = str_buf;

	if (str_mode == S_EMPTY
	    || (!(str_mode & S_CONT) && (str_ptr - str_buf) < d->df_size))
		return;

	if (!(str_mode & S_CONT))
	       /* 
		* If -An is specified, don't print labels
		*/
		if (Aflag == 2) 
			(void)fputs("\t", stdout);
		else
			put_addr(str_addr, str_label, ' ');

	while (p < str_ptr)
		(void)fputs(scvt(*p++, d), stdout);

	if (end)
		putchar('\n');
}

/*
 * integer to ascii conversion
 *
 * This code has been rearranged to produce optimized runtime code.
 */

#define MAXINTLENGTH	32
static char	_digit[] = "0123456789abcdef";
static char	_icv_buf[MAXINTLENGTH+1];
static long	_mask = 0x7fffffff;

/*
 * NAME: icvt
 *                                                                    
 * FUNCTION: return from a given stream the first value.
 *                                                                    
 * RETURN VALUE:  the value is an ascii stream printed in the RADIX given.
 */  
static char *
icvt (value, radix, issigned, ndigits)
long	value;
int	radix;
int	issigned;
int	ndigits;
{
	register long	val = value;
	register long	rad = radix;
	register char	*b = &_icv_buf[MAXINTLENGTH];
	register char	*d = _digit;
	register long	tmp1;
	register long	tmp2;
	long	rem;
	long	kludge;
	int	sign;

	if (val == 0)
	{
		*--b = '0';
		sign = 0;
		goto done; /*return(b);*/
	}

	if (issigned && (sign = (val < 0)))	/* signed conversion */
	{
		/*
		 * It is necessary to do the first divide
		 * before the absolute value, for the case -2^31
		 *
		 * This is actually what is being done...
		 * tmp1 = (int)(val % rad);
		 * val /= rad;
		 * val = -val
		 * *--b = d[-tmp1];
		 */
		tmp1 = val / rad;
		*--b = d[(tmp1 * rad) - val];
		val = -tmp1;
	}
	else				/* unsigned conversion */
	{
		sign = 0;
		if (val < 0)
		{	/* ALL THIS IS TO SIMULATE UNSIGNED LONG MOD & DIV */
			kludge = _mask - (rad - 1);
			val &= _mask;
			/*
			 * This is really what's being done...
			 * rem = (kludge % rad) + (val % rad);
			 * val = (kludge / rad) + (val / rad) + (rem / rad) + 1;
			 * *--b = d[rem % rad];
			 */
			tmp1 = kludge / rad;
			tmp2 = val / rad;
			rem = (kludge - (tmp1 * rad)) + (val - (tmp2 * rad));
			val = ++tmp1 + tmp2;
			tmp1 = rem / rad;
			val += tmp1;
			*--b = d[rem - (tmp1 * rad)];
		}
	}

	while (val)
	{
		/*
		 * This is really what's being done ...
		 * *--b = d[val % rad];
		 * val /= rad;
		 */
		tmp1 = val / rad;
		*--b = d[val - (tmp1 * rad)];
		val = tmp1;
	}

done:
	if (sign)
		*--b = '-';

	tmp1 = ndigits - (&_icv_buf[MAXINTLENGTH] - b);
	tmp2 = issigned? ' ':'0';
	while (tmp1 > 0)
	{
		*--b = tmp2;
		tmp1--;
	}

	return(b);
}

/*
 * NAME: get_addr
 *                                                                    
 * FUNCTION: return the address of the given string in the file.
 */  
static long
get_addr(s)
register char *s;
{
	register long a;
	char *ptr;

	if (*s=='+')
		s++;

       /*
	*  Parse type_string to determine input base
	*/
	if ((s[0] == '0') && ((s[1] == 'x') || (s[1] == 'X'))) {
		s += 2;
		base = 16;
	}
	else if (strchr(s, '.')) 
		base = 10;
	else if (*s == '0')
		base = 8;
	else if (s[0] == 'x') {
		s++;
		base = 16;
	} else if (jflag) {	/* POSIX says -j defaults to decimal */
	    	base = 10;
	} else if (base < 0)
		base=8;


       /*
	*  Parse input base
	*/
	errno = 0;
	a = strtol(s, &ptr, base);
	
	if ((a < 0) || errno || (s == ptr)) {
		(void)fprintf(stderr, MSGSTR(INVOFF, "Invalid offset\n"));
		usage(pname);
	}

       /*
	*  If A flag is not specified, use input base parsed above as
	*  base for labels.  Otherwise use base specified via -A option.
	*/
	if (!Aflag)
		addr_base = base;

	s = ptr;

       /*
	*  Offset may be followed by a multiplier
	*/
	switch (*s) {
	case '.':
		s++;
		break;
	case 'b':
		a *= 512;
		break;
	case 'k':
	case 'B':
		a *= 1024;
		break;
	case 'm':
		a *= 1048576;
		break;
	}

	return(a);
}


/*
 * NAME: offset
 *                                                                    
 * FUNCTION:  seek to the appropriate starting place.
 *                                                                    
 */  

static void
offset(skip_bytes, arg_cnt, arg_ptr)
long	skip_bytes;
int	*arg_cnt;
char	***arg_ptr;
{
	struct stat statb;
	int bad_file = 0;

	if (! fstat(fileno(stdin), &statb)) {

		/* If in POSIX/XPG mode (can have multiple input       */
		/* files specified) AND the file is a regular file AND */
		/* the offset is >= than the size of the file ...      */

		while ( (jflag) && (S_ISREG(statb.st_mode)) && 
					(skip_bytes >= statb.st_size) ) {

			/* Adjust skip bytes by size of file */
			skip_bytes -= statb.st_size;
			
			/* Open next file in argument list, if available */
			if (*arg_cnt > 0) {
				if (freopen(**arg_ptr, "r", stdin) == NULL)
				{
					perror(**arg_ptr);
					exit(1);
				}
				(*arg_ptr)++;
				(*arg_cnt)--;
			}
			else {
				/* Still more skip bytes, but no more files */
				(void)fprintf(stderr, "EOF\n");
				exit(1);
			}

			/* Get info on next file */
			if (fstat((int)fileno(stdin), &statb) != 0) {
				bad_file = 1;
				break;
			}
		}

		if (!bad_file && canseek(stdin, &statb)) {
			/*
			 * in case we're accessing a raw disk,
			 * we have to seek in multiples of a physical block.
			 */
			(void)fseek(stdin, skip_bytes & 0xfffffe00L, 0);
			skip_bytes &= 0x1ffL;
		}
	}
	dumbseek(stdin, skip_bytes);
}

/*
 * NAME: dumbseek
 *                                                                    
 * FUNCTION:  just read from stdin until the appropriate spot is reached.
 */  

static void
dumbseek(s, s_offset)
FILE	*s;
long	s_offset;
{
	char	buf[BUFSIZ];
	int	n;
	int	nr;

	while (s_offset > 0)
	{
		nr = (s_offset > BUFSIZ) ? BUFSIZ : (int)s_offset;
		if ((n = fread((void *)buf, (size_t)1, (size_t)nr, s)) != nr)
		{
			(void)fprintf(stderr, "EOF\n");
			exit(1);
		}
		s_offset -= n;
	}
}


/*
 * NAME: canseek
 *                                                                    
 * FUNCTION: check to see if we are reading from a pipe, file, or stdin.
 *                                                                    
 * RETURN VALUE:	1 if file 
 *			0 if pipe or stdin			    
 */  
static int
canseek(f, statb_ptr)
FILE	*f;
struct stat *statb_ptr;
{

	return( (statb_ptr->st_nlink > 0) &&		/*!pipe*/
		(!isatty(fileno(f))) );
}

static void
usage(char *progname)
{
	(void)fprintf(stderr, MSGSTR(USAGE,"usage: %s [-bCcdfhiloQs[n]vw[n]x] [file] [+]offset[.|b|B] [+]label[.|b|B]\n"),progname);
	(void)fprintf(stderr, MSGSTR(USAGE1,"       %s [-v] [-A address_base] [-j skip] [-N count] [-t type_string] [file...]\n"), progname);
	exit(1);
}
