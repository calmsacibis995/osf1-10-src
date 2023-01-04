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
static char	*sccsid = "@(#)$RCSfile: llib-port.c,v $ $Revision: 4.2.6.4 $ (DEC) $Date: 1993/11/17 15:50:38 $";
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

/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilities
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27; 10
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define _NO_PROTO

/*NOTUSED*/
/*NOTDEFINED*/

#include <stdio.h>

/* EXIT(2) */
void	exit(s) {}
void	_exit(s) {}
/* SIGNAL(2) */
void (*	signal(s, a))() int (*a)(); { return (a); }
/* TIME(2) */
time_t	time(t) time_t *t; { return ((time_t)0); }
/* UNLINK(2) */
int	unlink(s) char *s; { return (0); }

/* ABORT(3C) */
	/*VARARGS0*/
int	abort() { return (0); }
/* ABS(3C) */
int	abs(i) { return (i); }
/* ASSERT(3X) */
void	_assert(a, f, n) char *a, *f; {}
/* CONV(3C) */
#undef _toupper
#undef _tolower
#undef toascii
int	toupper(i) { return (i); }
int	tolower(i) { return (i); }
int	_toupper(i) { return (i); }
int	_tolower(i) { return (i); }
int	toascii(i) { return (i); }
/* CLOCK(3C) */
clock_t	clock() { return (0L); }
/* CRYPT(3C) */
char *	crypt(k, s) char *k, *s; { return (k); }
void	setkey(k) char *k; {}
void	encrypt(b, e) char *b; {}
/* CTERMID(3S) */
char *	ctermid(s) char *s; { return (s); }
/* CTIME(3C) */
char *	ctime(t) long *t; { return (""); }
#include <time.h>
struct tm *	localtime(c) long *c; { static struct tm x; return (&x); }
struct tm *	gmtime(c) long *c; { static struct tm x; return (&x); }
char *	asctime(t) struct tm *t; { return (""); }
void	tzset() {}
long	timezone;
int	daylight;
char *	tzname[2];
/* CTYPE(3C) */
#undef isalpha
#undef isupper
#undef islower
#undef isdigit
#undef isxdigit
#undef isalnum
#undef isspace
#undef ispunct
#undef isprint
#undef isgraph
#undef iscntrl
#undef isascii
int	isalpha(c) { return (c); }
int	isupper(c) { return (c); }
int	islower(c) { return (c); }
int	isdigit(c) { return (c); }
int	isxdigit(c) { return (c); }
int	isalnum(c) { return (c); }
int	isspace(c) { return (c); }
int	ispunct(c) { return (c); }
int	isprint(c) { return (c); }
int	isgraph(c) { return (c); }
int	iscntrl(c) { return (c); }
int	isascii(c) { return (c); }
char	_ctype[129];
/* CUSERID(3S) */
char *	cuserid(s) char *s; { return (s); }
/* DRAND48(3C) */
double	drand48() { return (0.0); }
double	erand48(x) unsigned short x[3]; { return (0.0); }
long	lrand48() { return (0L); }
long	nrand48(x) unsigned short x[3]; { return (0L); }
long	mrand48() { return (0L); }
long	jrand48(x) unsigned short x[3]; { return (0L); }
void	srand48(s) long s; {}
unsigned short *	seed48(s) unsigned short s[3]; {}
void	lcong48(p) unsigned short p[7]; {}
/* FCLOSE(3S) */
int	fclose(f) FILE *f; { return (0); }
int	fflush(f) FILE *f; { return (0); }
/* FERROR(3S) */
#undef feof
#undef ferror
#undef clearerr
#undef fileno
int	feof(f) FILE *f; { return (0); }
int	ferror(f) FILE *f; { return (0); }
void	clearerr(f) FILE *f; {}
int	fileno(f) FILE *f; { return (0); }
/* FOPEN(3S) */
FILE *	fopen(f, t) char *f, *t; { return (stdin); }
FILE *	freopen(f, t, s) char *f, *t; FILE *s; { return (s); }
/* FREAD(3S) */
#ifdef _NONSTD_TYPES
int	fread(b, s, n, f) char *b; FILE *f; { return (n); }
int	fwrite(b, s, n, f) char *b; FILE *f; { return (n); }
#else
size_t	fread(b, s, n, f) char *b; FILE *f; { return (n); }
size_t	fwrite(b, s, n, f) char *b; FILE *f; { return (n); }
#endif
/* FREXP(3C) */
double	frexp(x, e) double x; int *e; { return (x); }
double	ldexp(v, e) double v; { return (v); }
double	modf(v, i) double v, *i; { return (v); }
/* FSEEK(3S) */
int	fseek(f, o, p) FILE *f; long o; { return (p); }
long	ftell(f) FILE *f; { return (0L); }
void	rewind(f) FILE *f; {}
/* GETC(3S) */
#undef getc
#undef getchar
int	getc(f) FILE *f; { return (0); }
int	getchar() { return (0); }
int	fgetc(f) FILE *f; { return (0); }
int	getw(f) FILE *f; { return (0); }
/* GETOPT(3C) */
int	getopt(c, v, o) char **v, *o; { return (c); }
char *	optarg;
int	optind;
int	opterr, optopt; /* undocumented */
/* GETS(3S) */
char *	gets(s) char *s; { return (s); }
char *	fgets(s, n, f) char *s; FILE *f; { return (s); }
/* MALLOC(3C) */
char *	malloc(s) unsigned s; { return (""); }
void	free(s) char *s; {}
char *	realloc(s, n) char *s; unsigned n; { return (s); }
char *	calloc(n, e) unsigned n, e; { return (""); }
#include <string.h>
/* MEMORY(3C) */
#ifdef	_NONSTD_TYPES
char *	memccpy(a, b, c, n) void *a, *b; int c; size_t n; { return (a); }
char *	memchr(s, c, n) const void *s; int c; size_t n; { return (s); }
char *	memcpy(a, b, n) char *a, *b; size_t n; { return (a); }
char *	memset(s, c, n) char *s; int c; size_t n; { return (s); }
#else
void *	memccpy(a, b, c, n) void *a, *b; int c; size_t n; { return (a); }
void *	memchr(s, c, n) const void *s; int c; size_t n; { return (s); }
void *	memcpy(a, b, n) char *a, *b; size_t n; { return (a); }
void *	memset(s, c, n) char *s; int c; size_t n; { return (s); }
#endif /* _NONSTD_TYPES */
int	memcmp(a, b, n) char *a, *b; size_t n; { return (n); }
void	*memmove(s1,s2,n) void *s1; const void *s2; size_t n; { return s1; }
/* POPEN(3S) */
FILE *	popen(c, t) char *c, *t; { return (stdin); }
int	pclose(f) FILE *f; { return (0); }
/* PRINTF(3S) */
	/*VARARGS1 PRINTFLIKE1*/
int	printf(s) char *s; { return (0); }
	/*VARARGS2 PRINTFLIKE2*/
int	fprintf(f, s) FILE *f; char *s; { return (0); }
	/*VARARGS2 PRINTFLIKE2*/
int	sprintf(p, s) char *p, *s; { return (0); }
/* PUTC(3S) */
#undef putc
#undef putchar
int	putc(c, f) FILE *f; { return (c); }
int	putchar(c) { return (c); }
int	fputc(c, f) FILE *f; { return (c); }
int	putw(w, f) FILE *f; { return (w); }
/* PUTS(3S) */
int	puts(s) char *s; { return (0); }
int	fputs(s, f) char *s; FILE *f; { return (0); }
/* QSORT(3C) */
void	qsort(b, n, w, c) char *b; unsigned n, w; int (*c)(); {}
/* RAND(3C) */
void	srand(s) unsigned s; {}
int	rand() { return (0); }
/* SCANF(3S) */
	/*VARARGS1 SCANFLIKE1*/
int	scanf(s) char *s; { return (0); }
	/*VARARGS2 SCANFLIKE2*/
int	fscanf(f, s) FILE *f; char *s; { return (0); }
	/*VARARGS2 SCANFLIKE2*/
int	sscanf(p, s) char *p, *s; { return (0); }
/* SETBUF(3S) */
void	setbuf(f, b) FILE *f; char *b; {}
/* SETJMP(3C) */
#include <setjmp.h>
int	setjmp(e) jmp_buf e; { return (0); }
void	longjmp(e, v) jmp_buf e; {}
/* SLEEP(3C) */
unsigned	sleep(n) unsigned n; { return (n); }
/* SSIGNAL(3C) */
int (*	ssignal(s, a))() int (*a)(); { return (a); }
int	gsignal(s) { return (s); }
/* STDIO(3S) */
#ifndef	_NIOBRW
#define	_NIOBRW	16
#endif
FILE	 _iob[_NIOBRW];
/* STRING(3C) */
char 	*strdup(a) char *a; { return (a); }
int	strcasecmp(a, b) char *a, *b; { return (0); }
int	strncasecmp(a, b, n) const char *a, *b; size_t n; { return (n); }
char *	strcat(a, b) char *a, *b; { return (a); }
char *	strncat(a, b, n) char *a, *b; { return (a); }
int	strcmp(a, b) char *a, *b; { return (0); }
int	strncmp(a, b, n) char *a, *b; { return (n); }
char *	strcpy(a, b) char *a, *b; { return (a); }
char *	strncpy(a, b, n) char *a, *b; { return (a); }
#ifdef	_NONSTD_TYPES
int	strlen(s) char *s; { return (0); }
#else
size_t	strlen(s) char *s; { return (0); }
#endif /*_NONSTD_TYPES*/
char *	strchr(a, b) char *a, b; { return (a); }
char *	strrchr(a, b) char *a, b; { return (a); }
char *	strpbrk(a, b) char *a, *b; { return (a); }
#ifdef	_NONSTD_TYPES
int	strspn(a, b) const char *a, *b; { return (0); }
int	strcspn(a, b) const char *a, *b; { return (0); }
#else
size_t	strspn(a, b) const char *a, *b; { return (0); }
size_t	strcspn(a, b) const char *a, *b; { return (0); }
#endif /*_NONSTD_TYPES*/
char *	strtok(a, b) char *a, *b; { return (a); }
/* STRTOD(3C) */
double	strtod(s, t) char *s, **t; { return (0.0); }
double	atof(s) char *s; { return (0.0); }
/* STRTOL(3C) */
long	strtol(s, t, b) char *s, **t; { return (0L); }
long	atol(s) char *s; { return (0L); }
int	atoi(s) char *s; { return (0); }
/* SYSTEM(3S) */
int	system(s) char *s; { return (0); }
/* TMPNAM(3S) */
char *	tmpnam(s) char *s; { return (s); }
/* TTYNAME(3C) */
char *	ttyname(f) { return (""); }
int	isatty(f) { return (f); }
/* UNGETC(3S) */
int	ungetc(c, f) FILE *f; { return (c); }

/* UNDOCUMENTED -- declared in UNIX stdio.h, not really in the port lib */
FILE *	fdopen(f, t) char *t; { return (stdin); }
char *	tempnam(d, s) char *d, *s; { return (d); }
FILE *	tmpfile() { return (stdin); }
#include <float.h>
/* float.h is included indirectly by stdio.h */
unsigned   int SINFINITY;
unsigned   int DINFINITY[2];
unsigned   int SQNAN;
unsigned   int DQNAN[2];
unsigned   int SSNAN;
unsigned   int DSNAN[2];
unsigned read_rnd() { return (0u); }
unsigned write_rnd(rnd) unsigned rnd; { return (rnd); }

int iswalpha (x) wint_t x; { return (0); }
int iswalnum (x) wint_t x; { return (0); }
int iswcntrl (x) wint_t x; { return (0); }
int iswdigit (x) wint_t x; { return (0); }
int iswgraph (x) wint_t x; { return (0); }
int iswlower (x) wint_t x; { return (0); }
int iswprint (x) wint_t x; { return (0); }
int iswpunct (x) wint_t x; { return (0); }
int iswspace (x) wint_t x; { return (0); }
int iswupper (x) wint_t x; { return (0); }
int iswxdigit (x) wint_t x; { return (0); }
wint_t towupper (x) wint_t x; { return (wint_t) (0); }
wint_t towlower (x) wint_t x; { return (wint_t) (0); }
int iswctype (x,y) wint_t x; wctype_t y; { return (0); }

wctype_t wctype (s) char *s; { return (s); }

wint_t getwc (f) FILE *f; { return (0); }
wint_t putwc (c, f) wint_t c; FILE *f; { return (0); }

wchar_t *fgetws (s, x, f) wchar_t *s; int x; FILE *f; { return (s); }
int fputws (s, f) wchar_t *s; FILE *f; { return (0); }

double wcstod (c,s) wchar_t *c; wchar_t **s; { return (double) (0); }
long wcstol (c, s, x) wchar_t *c; wchar_t **s; int x; { return (long) (0); }
unsigned long wcstoul (c, s, x) wchar_t *c; wchar_t **s; int x;
{ return (unsigned long) (0); }
int wcscoll (c, s) wchar_t c; wchar_t s; { return (0); }
wchar_t *wcstok (c, s) wchar_t c; wchar_t s; { return (c); }
int wcswidth (c, x) wchar_t c; size_t x; { return (0); }
size_t wcsxfrm (c, s, x) wchar_t c; wchar_t s; size_t x; { return (x); }
int wcwidth (c) wchar_t c; { return (0); }
size_t wcsftime (c, x, s, y) wchar_t c;  size_t x; wchar_t s;
     struct tm y; { return (x);  }

#include <fnmatch.h>
int     fnmatch (s1, s2, x) char *s1; char *s2; int x;  { return (0); }


#include <mesg.h>
struct _catset *__cat_get_catset(catd, x) nl_catd catd; int x;
{ return (0); }
struct _msgptr *__cat_get_msgptr(cset, x) struct _catset *cset; int x;
{ return (0); }

#include <glob.h>
int glob (c,x,y,z) char *c; int x; int  *y; glob_t z; { return(x); }
void globfree(x) glob_t x; { }


#include <sys/localedef.h>
_LC_charmap_t  *__lc_charmap;
_LC_collate_t  *__lc_collate;
_LC_ctype_t    *__lc_ctype;
_LC_monetary_t *__lc_monetary;
_LC_numeric_t  *__lc_numeric;
_LC_resp_t     *__lc_resp;
_LC_time_t     *__lc_time;
_LC_locale_t   *__lc_locale;

#include <mbstr.h>
char *mbsinvalid(c) char *c; { return (c); }
size_t mbslen(c) char *c; { return (0); }
mbchar_t mbstomb (c) char * c; { return(0); }
int mbswidth (c,x) char *c; size_t x; { return (0); }
char *mbsadvance (a) const char *a; { return (a); }

#include <sys/method.h>
method_t *std_methods;
int method_class;
int mb_cur_max;

#include <monetary.h>
ssize_t strfmon(c,x,d) char *c; size_t x; char *d; { return(0); }

#include <nl_types.h>
nl_catd NLcatopen (c,x) char *c; int x; { return ((nl_catd)0); }
char *NLcatgets (catd, x, y, c) nl_catd catd; int x; int y; char *c;
{ return (c); }

#include <regex.h>
int regcomp (a,c,x) regex_t *a; char *c; int x; { return (x); }
int regexec (a,c,x,b,y) regex_t *a; char *c; size_t x; regmatch_t *b; int y;
{ return(y); }
size_t  regerror (x,a,c,y) int x; regex_t *a; char *c; size_t y;
{ return(y); }
void regfree(a) regex_t *a; { }


#include <wordexp.h>
int wordexp (c,a,x) char *c; wordexp_t *a; int x; { return (x); }
void wordfree (a) wordexp_t *a; { }


int     sigaltstack(a, b) stack_t a; stack_t b; { return(0); }

