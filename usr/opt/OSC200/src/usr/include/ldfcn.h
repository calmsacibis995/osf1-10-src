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
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991, 1990 MIPS Computer Systems, Inc.      |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 252.227-7013.  |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Avenue                               |
 * |         Sunnyvale, California 94088-3650, USA             |
 * |-----------------------------------------------------------|
 */
/* $Header: /usr/sde/alpha/rcs/alpha/src/./usr/include/ldfcn.h,v 4.2.4.5 1992/12/08 18:13:38 Don_Anderson Exp $ */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *	The following two declarations appear in the IH versions of
 *	"stdio.h" but do not appear in the normal 1.2 versions.
 */
#if defined(__mips) || defined(__alpha)
long ftell();
char *fgets();
#endif /* __mips */

#ifndef _LDFCN_H
#define _LDFCN_H

#include <obj.h>
#include <ar.h>


struct	ldfile {
	int		_fnum_;	/* so each instance of an LDFILE is unique */
	FILE		*ioptr;	/* system I/O pointer value */
	long		offset;	/* absolute offset to the start of the file */
	FILHDR		header;	/* the file header of the opened file */
#if defined(__mips) || defined(__alpha)
	pCHDRR		pchdr;  /* pointer to the symbol table */
	int		lastindex; /* index of last symbol accessed */
	unsigned short	type;	/* indicator of the type of the file */
	unsigned	fswap : 1;	/* if set, we must swap */
	unsigned	fBigendian : 1;	/* if set, we must swap aux for the
					 * last retrieved symbol
					 */
#else
	unsigned short	type;		/* indicator of the type of the file */
#endif /* __mips */
};

/*
	define a structure "type" for an archive header
*/

typedef struct
{
	char ar_name[16];
	int ar_date;
	int ar_uid;
	int ar_gid;
	int ar_mode;
	int ar_size;
} archdr;

#define	ARCHDR	archdr
#define ARCHSZ	sizeof(ARCHDR)


typedef struct {
	struct ldfile	ldfile;
	pOBJ		obj;
	int		arfd;
	off_t		aroffset; /* offset to next archive header */
	ARCHDR		archdr;
} elf32_LDFILE;

#define LDFILE elf32_LDFILE

/*
	provide a structure "type" definition, and the associated
	"attributes"
*/

#define IOPTR(x)        (x)->ldfile.ioptr
#define OFFSET(x)	(x)->ldfile.offset
#define TYPE(x)         (x)->ldfile.type
#define	HEADER(x)	(x)->ldfile.header
#define LDFSZ		sizeof(LDFILE)
#if defined(__mips) || defined(__alpha)
#define PSYMTAB(x)	(x)->ldfile.pchdr
#define SYMTAB(x)	(ldreadst((x), -1), (x)->ldfile.pchdr)
#define SYMHEADER(x)    (x)->ldfile.pchdr->hdr
#define PFD(x)          (x)->ldfile.pchdr->pfd
#define LDSWAP(x)       (x)->ldfile.fswap
#define LDAUXSWAP(x,ifd) (PFD(x)[ifd].fBigendian != x->ldfile.fBigendian)
#define LDERROR(x,y,z)  fprintf(stderr,"libmld: x:"); fprintf (stderr, y, z);
#endif /* __mips */

/*
	define various values of TYPE(ldptr)
*/

#define ARTYPE 	0177545
#define ISARCHIVE(x) ((x) == ARTYPE)

/*
	define symbolic positioning information for FSEEK (and fseek)
*/

#define BEGINNING	0
#define CURRENT		1
#define END		2

/*
	define some useful symbolic constants
*/

#define SYMTBL	0	/* section nnumber and/or section name of the Symbol Table */

#define	SUCCESS	 1
#define	CLOSED	 1
#define	FAILURE	 0
#define	NOCLOSE	 0
#define	BADINDEX	-1L

#define	OKFSEEK	0

/*
	define macros to permit the direct use of LDFILE pointers with the
	standard I/O library procedures
*/

#if defined(_NO_PROTO) && !defined(__mips64) && !defined(__alpha)
elf32_LDFILE *ldopen();
elf32_LDFILE *ldaopen();
#endif

#ifdef __mips
/*LDFILE *ldinitheaders();*/
#endif   /* __mips */

#ifdef __ultrix
# ifdef FREADM
#  undef FREADM
# endif /* FREADM */
# define FREADM LDFREAD
#define LDFREAD(p,s,n,ldptr)    ldfread(p,s,n,ldptr)
#else  /* ! __ultrix */
#define FREADM(p,s,n,ldptr)  ldfread(p,s,n,ldptr)
#endif /* __ultrix */
#define FSEEK(ldptr,o,p)    ldfseek(ldptr,o,p)
/*fseek(IOPTR(ldptr),(p==BEGINNING)?(OFFSET(ldptr)+o):o,p)*/

#if !defined(__mips) && !defined(__alpha)
#define STROFFSET(ldptr)	(HEADER(ldptr).f_symptr + HEADER(ldptr).f_nsyms * 18) /* 18 == SYMESZ */
#else /* __mips */
#define STROFFSET(ldptr)	(SYMHEADER(ldptr).cbSsOffset)
#endif /* __mips */


#ifndef _LIBMLD
#ifdef  _NO_PROTO

extern	int 	ldaclose();
extern	LDFILE 	*ldaopen();
extern  int 	ldclose();
extern	int 	ldfhread();
extern	int 	ldfread();
extern	int 	ldfseek();
extern  pAUXU 	ldgetaux();
extern  long 	ldgetpd();
extern	char 	*ldgetname();
extern	int  	ldgetsymstr();
extern  int 	ldlread();
extern	int 	ldlinit();
extern  int 	ldlitem();
extern	int 	ldlseek();
extern	int 	ldnlseek();
extern	int 	ldnrseek();
extern	int 	ldnreloc();
extern	int 	ldnsseek();
extern	int 	ldnshread();
extern	int 	ldohseek();
extern	int 	ldohread();
extern	int 	ldrseek();
extern	int 	ldshread();
extern	int 	ldsseek();
extern  LDFILE *ldopen();
extern  int	ldreadst();
extern	int	ldtbindex();
extern	int 	ldtbread();
extern	int 	ldtbseek();
extern	int	ranhashinit();
extern	int	ranhash();
extern	struct ranlib	ranlookup();
extern	int	disassembler();

#else /* _NO_PROTO */

extern	int 	ldaclose(LDFILE *);
extern	LDFILE 	*ldaopen(char *, LDFILE *);
extern  int 	ldclose(LDFILE *);
extern	int 	ldfhread(LDFILE *, FILHDR*);
extern	int 	ldfread(void *, int, int, LDFILE *);
extern	int 	ldfseek(LDFILE *, int, int);
extern  pAUXU 	ldgetaux(LDFILE *, int);
extern  long 	ldgetpd(LDFILE *, int, pPDR);
extern	char 	*ldgetname(LDFILE *, pSYMR);
extern	int  	ldgetsymstr(LDFILE *, pSYMR, int, char *);
extern  int 	ldlread(LDFILE *, long, unsigned short, LINER *);
extern	int 	ldlinit(LDFILE *, long);
extern  int 	ldlitem(LDFILE *, unsigned short, LINER *);
extern	int 	ldlseek(LDFILE *, unsigned short);
extern	int 	ldnlseek(LDFILE *, char *);
extern	int 	ldnrseek(LDFILE *, char *);
extern	int 	ldnreloc(LDFILE *, SCNHDR *);
extern	int 	ldnsseek(LDFILE *, char *);
extern	int 	ldnshread(LDFILE *, char *, SCNHDR *);
extern	int 	ldohseek(LDFILE *);
extern	int 	ldohread(LDFILE *, AOUTHDR *);
extern	int 	ldrseek(LDFILE *, unsigned short);
extern	int 	ldshread(LDFILE *, int, SCNHDR *);
extern	int 	ldsseek(LDFILE *, unsigned short);
extern  LDFILE *ldopen(char *, LDFILE *);
extern  int	ldreadst(LDFILE *, int);
extern	int	ldtbindex(LDFILE *);
extern	int 	ldtbread(LDFILE *, int, pSYMR);
extern	int 	ldtbseek(LDFILE *);
extern	int	ranhashinit(struct ranlib *, char *, int);
extern	int	ranhash(char *);
extern	struct ranlib	ranlookup(char *);
extern	int	disassembler(unsigned long, int, char *(*)(),
			     unsigned long (*)(), unsigned int (*)(),
			     void (*)());
#endif  /* _NO_PROTO */
#endif  /* _LIBMLD */
#endif  /* _LDFILE_H */
