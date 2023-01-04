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
/*	
 *	@(#)$RCSfile: stdio.h,v $ $Revision: 4.2.10.7 $ (DEC) $Date: 1993/12/15 22:14:21 $
 */ 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * COMPONENT_NAME: stdio.h
 *                                                                    
 * ORIGIN: IBM
 *
 * Copyright International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * stdio.h  1.40  com/inc,3.1,9021 5/12/90 06:54:44
 */                                                                   

#ifndef _STDIO_H_
#define _STDIO_H_

#include <standards.h>

/*
 *
 *      The ANSI and POSIX standards require that certain values be in stdio.h.
 *      It also requires that if _ANSI_C_SOURCE or _POSIX_SOURCE is defined
 *	then ONLY those values are present. This header includes all the ANSI 
 *	and POSIX required entries.
 *
 */

#ifdef _ANSI_C_SOURCE

/*
 *      The following definitions are included in <sys/types.h>.  They
 *      are also included here to comply with ANSI standards.
 */

#ifndef NULL
#define NULL    0L
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long   size_t;
#endif

#ifndef _FPOS_T
#define _FPOS_T
typedef long    fpos_t;
#endif

/*
 *      The definition of TMP_MAX is included in <sys/limits.h>.  It is
 *      also defined here to comply with ANSI standards.
 */

#ifndef TMP_MAX
#define TMP_MAX         16384
#endif

/*
 * Maximum fopens per process - OBSOLETE, sysconf() interface should be
 * used.
 */
#define FOPEN_MAX 	64

#define FILENAME_MAX 	255
#define BUFSIZ		8192
#define _P_tmpdir       "/tmp/"
#define L_tmpnam	(sizeof(_P_tmpdir) + 15)

/*
 * _IOLBF means that a file's output will be buffered line by line
 * In addition to being flags, _IONBF, _IOLBF and _IOFBF are possible
 * values for "type" in setvbuf.
 */
#define _IOFBF		0000
#define _IOLBF		0200
#define _IONBF		0004


#ifndef EOF
#define EOF		(-1)
#endif

#include <sys/seek.h>

typedef struct {
	int	_cnt;
	unsigned char	*_ptr;
	unsigned char	*_base;
	int	_bufsiz;
	short	_flag;
	short	_file;

#ifndef	__alpha
	void	*_unused;
#endif	/* __alpha */
	char	*__newbase;
	void	*_lock;			/* lock for thread safe library (or unused2) */
	unsigned char	*_bufendp;
} FILE;

extern FILE	_iob[];

#define _IOEOF		0020
#define _IOERR		0040

#define stdin		(&_iob[0])
#define stdout		(&_iob[1])
#define stderr		(&_iob[2])

#ifdef _NONSTD_TYPES
extern int     fread();
extern int     fwrite();
#else
_BEGIN_CPLUSPLUS
extern size_t	fread __((void *, size_t, size_t, FILE *));
extern size_t	fwrite __((const void *, size_t, size_t, FILE *));
_END_CPLUSPLUS
#endif  /* _NONSTD_TYPES */

_BEGIN_CPLUSPLUS
extern int	_flsbuf __((int, FILE *));
extern int	_filbuf __((FILE *));
extern int 	ferror __((FILE *));
extern int 	feof __((FILE *));
extern void 	clearerr __((FILE *));
extern int 	putchar __((int));
extern int 	getchar __((void));
extern int 	putc __((int, FILE *));
extern int 	getc __((FILE *));
extern int	remove __((const char *));
extern int	rename __((const char *, const char *));
extern FILE 	*tmpfile __((void));
extern char 	*tmpnam __((char *));
extern int 	fclose __((FILE *));
extern int 	fflush __((FILE *));
extern FILE	*fopen __((const char *, const char *));
extern FILE 	*freopen __((const char *, const char *, FILE *));
extern void 	setbuf __((FILE *, char *));
extern int 	setvbuf __((FILE *, char *, int, size_t));
extern int	fprintf __((FILE *, const char *, ...));
extern int	fscanf __((FILE *, const char *, ...));
extern int	printf __((const char *, ...));
extern int	scanf __((const char *, ...));
extern int	sprintf __((char *, const char *, ...));
extern int	sscanf __((const char *, const char *, ...));

#ifdef _VA_LIST
extern int  vfprintf __((FILE *, const char *, va_list));
extern int  vprintf __((const char *, va_list));
extern int  vsprintf __((char *, const char *, va_list));
#else /* _VA_LIST */

#ifdef _XOPEN_SOURCE
#include <va_list.h>
extern int  vfprintf __((FILE *, const char *, va_list));
extern int  vprintf __((const char *, va_list));
extern int  vsprintf __((char *, const char *, va_list));
#else	/* _XOPEN_SOURCE */

#define _HIDDEN_VA_LIST		/* define a type not in the namespace */
#include <va_list.h>
extern int  vfprintf __((FILE *, const char *, __va_list));
extern int  vprintf __((const char *, __va_list));
extern int  vsprintf __((char *, const char *, __va_list));

#endif	/* _XOPEN_SOURCE */
#endif	/* _VA_LIST */

extern int 	fgetc __((FILE *));
extern char 	*fgets __((char *, int, FILE *));
extern int 	fputc __((int, FILE *));
extern int 	fputs __((const char *, FILE *));
extern char 	*gets __((char *));
extern int 	puts __((const char *));
extern int	ungetc __((int, FILE *));
extern int	fgetpos __((FILE *, fpos_t *));
extern int 	fseek __((FILE *, long, int));
extern int	fsetpos __((FILE *, const fpos_t *));
extern long	ftell __((FILE *));
extern void	rewind __((FILE *));
extern void 	perror __((const char *));
#if defined(_REENTRANT) || defined(_THREAD_SAFE)
extern void	flockfile __((FILE *));
extern void	funlockfile __((FILE *));
extern int	fclose_unlocked __((FILE *));
extern int	fflush_unlocked __((FILE *));
extern size_t	fread_unlocked __((void *, size_t, size_t, FILE *));
extern size_t	fwrite_unlocked __((const void *, size_t, size_t, FILE *));
#endif	/* _REENTRANT || _THREAD_SAFE */


/* Define commonly used macros here.
 */
#define _GETC_UNLOCKED(p) \
		(--(p)->_cnt < 0 ? _filbuf(p) : (int) *(p)->_ptr++)

#define _PUTC_UNLOCKED(x, p) \
		(--(p)->_cnt < 0 ? \
			_flsbuf((int) (x), (p)) : \
			(int) (*(p)->_ptr++ = (unsigned char) (x)))

#define _CLEARERR_UNLOCKED(p)	((void) ((p)->_flag &= ~(_IOERR | _IOEOF)))
#define _FEOF_UNLOCKED(p)	((p)->_flag & _IOEOF)
#define _FERROR_UNLOCKED(p)	((p)->_flag & _IOERR)
#define _FILENO_UNLOCKED(p)	((p)->_file)
_END_CPLUSPLUS

#if defined(_REENTRANT) || defined(_THREAD_SAFE)

#ifdef _NAME_SPACE_WEAK_STRONG
#undef getc_unlocked
#undef getchar_unlocked
#undef getc_locked
#undef getchar_locked
#undef putc_unlocked
#undef putchar_unlocked
#undef putc_locked
#undef putchar_locked
#endif
#define getc_unlocked(p)	_GETC_UNLOCKED(p)
#define getchar_unlocked()	getc_unlocked(stdin)
#define getc_locked(p)		fgetc(p)
#define getchar_locked()	getc_locked(stdin)
#define putc_unlocked(x, p)	_PUTC_UNLOCKED(x, p)
#define putchar_unlocked(x)	putc_unlocked(x,stdout)
#define putc_locked(x, p)	fputc(x, p)
#define putchar_locked(x)	putc_locked(x,stdout)

/*
 * The default for getc and putc are locked for compatibility with 
 * Posix P1003.4a
 * By defining _STDIO_UNLOCK_CHAR_IO before including this
 * file, the default action is changed to unlocked putc and getc.
 * A file lock can still be placed around a block of putc's or getc's
 * regardless of the locking mode, and invoking the locked or
 * unlocked version directly always overrides the default action.
 */
#ifdef _STDIO_UNLOCK_CHAR_IO
#define getc(p)			getc_unlocked(p)
#define putc(x, p)		putc_unlocked(x, p)
/*
 * if _STDIO_UNLOCK_CHAR_IO is not defined, these macros will not be defined
 * and become functions.
 */
#define clearerr(p)		clearerr_unlocked(p)
#define feof(p)			feof_unlocked(p)
#define ferror(p)		ferror_unlocked(p)

#else
#define getc(p)			getc_locked(p)
#define putc(x, p)		putc_locked(x, p)
#endif /* _STDIO_UNLOCK_CHAR_IO */

#define clearerr_unlocked(p)	_CLEARERR_UNLOCKED(p)
#define feof_unlocked(p)	_FEOF_UNLOCKED(p)
#define ferror_unlocked(p)	_FERROR_UNLOCKED(p)
#define fileno_unlocked(p)	_FILENO_UNLOCKED(p)

#else /* _REENTRANT || _THREAD_SAFE */

#define getc(p)			_GETC_UNLOCKED(p)
#define putc(x, p)		_PUTC_UNLOCKED(x, p)
#define clearerr(p)		_CLEARERR_UNLOCKED(p)
#define feof(p)			_FEOF_UNLOCKED(p)
#define ferror(p)		_FERROR_UNLOCKED(p)

#endif /* _REENTRANT || _THREAD_SAFE */

#define getchar()		getc(stdin)
#define putchar(x)		putc((x), stdout)
#endif /*_ANSI_C_SOURCE */


/*
 *	The following are values that have historically been in stdio.h.
 *
 *	They are a part of the POSIX defined stdio.h and therefore are
 *	included when _POSIX_SOURCE and _XOPEN_SOURCE are defined.
 *
 */

#ifdef _POSIX_SOURCE
#include <sys/types.h>

#define L_ctermid	9
#define L_cuserid	9

_BEGIN_CPLUSPLUS
extern int 	fileno __((FILE *));
extern FILE 	*fdopen __((int, const char *));

#ifndef _XOPEN_SOURCE
extern char *ctermid __((char *));
#endif
extern char *cuserid __((char *));
_END_CPLUSPLUS

#if !defined(_REENTRANT) || defined(_STDIO_UNLOCK_CHAR_IO)
#ifdef _NAME_SPACE_WEAK_STRONG
#undef fileno
#endif
#define fileno(p)	_FILENO_UNLOCKED(p)
#endif	/* !_REENTRANT || _STDIO_UNLOCK_CHAR_IO */

#endif /* _POSIX_SOURCE */


#ifdef _XOPEN_SOURCE

/*** To be withdrawn ***/
#include <getopt.h>			/* Also in unistd.h */
/***/

#define P_tmpdir	_P_tmpdir

_BEGIN_CPLUSPLUS
extern char	*ctermid __((char *));
extern int 	getw __((FILE *));
extern int 	pclose __((FILE *));
extern int 	putw __((int, FILE*));
extern FILE 	*popen __((const char *, const char *));
extern char 	*tempnam __((const char*, const char*));
_END_CPLUSPLUS

#endif /* _XOPEN_SOURCE */


#ifdef _OSF_SOURCE

#include <sys/limits.h>	/* limits.h not allowed by Posix.1a.  Must be here */

/*
 * Maximum open files per process - OBSOLETE, sysconf() interface should 
 * be used
 */
#ifdef OPEN_MAX
#define _NFILE		OPEN_MAX
#else
#define _NFILE		64	/* should be a multiple of _NIOBRW */
#endif

/* buffer size for multi-character output to unbuffered files */
#define _SBFSIZ 8

#define _IOREAD		0001
#define _IOWRT		0002
#define _IOMYBUF	0010
#define _IOSTRG		0100
#define _IONOFD		_IOSTRG		/* strange or no file descriptor */
#define _IORW		0400
#define _IOUNGETC	01000
#define _IOINUSE	02000		/* new flag for _THREAD_SAFE */
#define _IONONSTD	04000
#define _IOCLOSE	010000

#define _bufend(p)	((p)->_bufendp)
#define _bufsiz(p)	(_bufend(p) - (p)->_base)

_BEGIN_CPLUSPLUS
extern void 	setbuffer __((FILE *, char*, int));
extern void 	setlinebuf __((FILE *));
_END_CPLUSPLUS

#endif /* _OSF_SOURCE */

#ifdef _INTRINSICS
#pragma intrinsic(printf, fprintf)

#ifdef _INLINE_INTRINSICS
static __inline int __CFE_print_puts( char *x )
{
 return( fputs( x, stdout  ));
}

static __inline int __CFE_print_putc( int x )
{
 putc( x, stdout );
 return( 1 );
}

static __inline int __CFE_print_putc_nl( int x )
{
 putc(x, stdout ); putc( '\n',stdout );
 return( 2 );
}

static __inline int __CFE_fprint_puts_nl( char *x, FILE *f )
{ 
 register int i;
 i = fputs( x, f ); 
 putc( '\n', f  ); 
 return( i+1 );
}

static __inline int __CFE_fprint_putc( int x, FILE *f )
{
 putc( x, f );
 return( 1 );
}

static __inline int __CFE_fprint_putc_nl( int x, FILE *f )
{
 putc( x,f ); putc( '\n', f );
 return(2);
}
#endif
#endif

#endif /* _STDIO_H_ */


