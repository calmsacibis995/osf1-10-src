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
static char	*sccsid = "@(#)$RCSfile: reader.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/10/11 15:50:14 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: CloseFile, GetName, InHeader, InMembers, InSymbol, InType,
	      InUsage, OpenFile
 *
 * ORIGINS: 00 03 10 27 32
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * reader.c	1.5  com/cmd/prog/lint/pass2,3.1,9013 9/12/89 11:52:04"; 
 */

#include "lint_msg.h"
#define         MSGSTR(Num, Str) catgets(catd, MS_LINT, Num, Str)
nl_catd catd;

#include <stdio.h>
#include "mfile1.h"
#include "lint2.h"
static size_t last_read_debug;
static size_t total_bytes_read_debug = 0;
/*#define FRED_DBG		/* Define this for debug output */
FILE *fp;			/* current file pointer */
/* #ifdef 0 */
#define BIO 1 			/* binary i/o selected */
/* #endif */
/*
** Header distinguishes file partitions from symbol data.
*/
extern void printtyinfo( TPTR typtr, int flag);
InHeader()
{
  
  union {
    long iocode_long;
    int  iocode_int;
    char iocode
    } I;

	/* Read file delimiter record. */
#ifdef	BIO
	if (last_read_debug = fread((char *) &I.iocode, sizeof(char), 1, fp) < 1) {
	total_bytes_read_debug += last_read_debug;    
#else
	/* We scanf for an int */
	if (fscanf(fp, "%d", &I.iocode_int) == EOF) {
#endif
#ifdef FRED_DBG
	  if (fdebug)
	    printf("InHeader: read EOF %d\n",I.iocode);
#endif
		if (markerEOF)
			return ((int) LINTEOF);
		cerror(MSGSTR(M_MSG_265, "unexpected EOF for file %s"),
			curPFname);
 	}
	
	/* Handle case where delimiter indicates a new file. */
	if (I.iocode == LINTBOF) {
#ifdef FRED_DBG
	  if (fdebug)
	    printf("InHeader: read BOF %d\n",I.iocode);
#endif
		curPFname = StoreSName(GetName(GETMISC));
		/* Read for next record (assumed symbol). */
#ifdef	BIO
		last_read_debug = fread((char *) &I.iocode, sizeof(char), 1, fp);
		if (last_read_debug)  total_bytes_read_debug += last_read_debug; 
#else
		fscanf(fp, "%d", &I.iocode_int);
#endif
#ifdef FRED_DBG
	  if (fdebug)
	    printf("Inheader: iocode: %d\n",I.iocode);
#endif
		markerEOF = 0;
	}
	return ((int) I.iocode);
}

/*
** Read characer string from intermediate file.
** Do name truncation if portability option enabled.
*/
char *
GetName(what)
	int what;
{
	static char buf[BUFSIZ];
	register char *cp = (char *) buf;
	register int c;

#ifdef	BIO
	while ((c = fgetc(fp)) && c != EOF) {
		*cp++ = c;
	}
	*cp = '\0'; /* Null terminate */
#else
	fscanf(fp, "%s", cp);
#endif
#ifdef FRED_DBG
	if (fdebug) {
	  printf("GetName: Name: %s\n",buf);
	}
#endif

	/* Six character name truncation to upper case. */
	if (pflag && (what == GETNAME)) {
		register char *ep = (char *) &buf[6];
		for (cp = (char *) buf; *cp && cp < ep; cp++)
			*cp = tolower(*cp);
		*cp = '\0';
	}
	return (buf);
}

/*
** Read function usage symbol record.
*/
InUsage()
{
	long temp;
	curSym->sname = (char *) sbuf;
	strcpy(curSym->sname, GetName(GETNAME));
#ifdef	BIO
	last_read_debug = fread((char *) &curSym->usage, sizeof(short), 1, fp);
	if (last_read_debug)  total_bytes_read_debug += last_read_debug; 
#else
	fscanf(fp, "%d", &temp);
        curSym->nmbrs = (short) temp;

#endif
#ifdef FRED_DBG
	if (fdebug)
	  printf("InUsage: Usage: 0%o\n",curSym->usage);
#endif

}

/*
** Read symbol record.
*/
InSymbol()
{
	char *s;
	long temp;

	/* Clear out any left over data from previous symbol */
	memset(curSym, '\0', sizeof (SMTAB));
	curSym->sname = (char *) sbuf;
	curIFname = (char *) ibuf;

#ifdef FRED_DBG
	if (fdebug)
	  printf("InSymbol: ");
#endif
	strcpy(curSym->sname, GetName(GETNAME));

	/* Minimize storing new include filenames. */
	s = GetName(GETMISC);
	if ((prevIFname != NULL) && !strcmp(prevIFname, s))
		curIFname = prevIFname;
	else
		prevIFname = curIFname = StoreSName(s);

#ifdef	BIO
	last_read_debug = fread((char *) &curDLine, sizeof(short), 1, fp);
	if (last_read_debug)  total_bytes_read_debug += last_read_debug; 
	last_read_debug = fread((char *) &curRLine, sizeof(short), 1, fp);
	if (last_read_debug)  total_bytes_read_debug += last_read_debug; 
	last_read_debug = fread((char *) &curSym->usage, sizeof(short), 1, fp);
	if (last_read_debug)  total_bytes_read_debug += last_read_debug; 
#else
	fscanf(fp, "%ld", &temp);
	curDLine = (short) temp;
	fscanf(fp, "%ld", &temp);
	curRLine = (short) temp;
	fscanf(fp, "%ld", &temp);
	curSym->usage = (short) temp;
#endif
#ifdef FRED_DBG
	if (fdebug)
	  printf("\t\t\t line: %d rline %d Usage: 0%o\n",
		 curDLine,curRLine,curSym->usage);
#endif

	curSym->type = InType();

	/* Read symbol members, if any. */
	curSym->nmbrs = 0;
	curSym->mbrs = 0;
	if (curSym->usage & LINTMBR) {
	  long temp;
#ifdef	BIO
		last_read_debug = fread((char *) &curSym->nmbrs, sizeof(short), 1, fp);
		if (last_read_debug)  total_bytes_read_debug += last_read_debug; 
#else
		fscanf(fp, "%ld", &temp);
	        curSym->nmbrs = (short) temp;
#endif
#ifdef FRED_DBG
	  if (fdebug)
	    printf("\t Number of members: %d\n",curSym->nmbrs);
#endif
		InMembers();
	}
}

/*
** Get each member of the current symbol.
*/
InMembers()
{
	register MBTAB *m;
	register int i;
	
	TWORD bt;
#ifdef FRED_DBG
	if (fdebug)
	  printf("InMembers:\n");
#endif

	/* Read and link each member. */
	for (i = 0; i < curSym->nmbrs; i++) {
		if (i == 0) {
			curSym->mbrs = m = MBMalloc();
		}
		else {
			m->next = MBMalloc();
			m = m->next;
		}

		m->mname = StoreMName(GetName(GETNAME));
		m->type = InType();
		m->tagname = 0;
		if ((bt = BTYPE(m->type)) == STRTY || bt == UNIONTY ||
			bt == ENUMTY) {
		        union {
		        int usage_i;
			short usage
			}U;
#ifdef	BIO
			U.usage_i = 0; 
			last_read_debug = fread((char *) &U.usage, sizeof(short), 1, fp);
			if (last_read_debug)  total_bytes_read_debug += last_read_debug; 
#else
			fscanf(fp, "%d", &U.usage_i);
#endif
#ifdef FRED_DBG
			if (fdebug)
			  printf("\t\t Usage: 0%o\n",U.usage_i);
#endif
			if (U.usage && LINTTAG){

#ifdef FRED_DBG
			  if (fdebug)
			    printf("\t\t Tag! tagnme:\t");
#endif
				m->tagname = StoreMName(GetName(GETNAME));
			      }
		}
		m->next = 0;
	}
}

/*
** Get the type of the current symbol.
*/
TPTR
InType()
{
	register TPTR t;
	register PPTR p;
	TPTR ot;
#ifdef	BIO
	struct tyinfo ty;
#else
	unsigned long tnext_t, info_t, type_t, pnext_t;
	TWORD type, info;
	void * tnext;
	void * pnext;
#endif
#ifdef FRED_DBG
	if (fdebug)
	  printf("InType:\n");
#endif


#ifdef	BIO
	last_read_debug = fread((char *) &ty, sizeof(struct tyinfo), 1, fp);
	if (last_read_debug)  total_bytes_read_debug += last_read_debug; 
#ifdef FRED_DBG
	if (fdebug)
	  printtyinfo(&ty,0);
#endif
/*	if (ty.tword == TNULL) return(TNIL); */
	if ((t = FindBType(ty.tword)) == TNIL)
		t = tynalloc(ty.tword);
#else
	fscanf(fp, "%lo %lo %lo", &type_t, &tnext_t, &info_t);
#ifdef FRED_DBG
	if (fdebug)
	  printf("InType:: type_t = 0%lo, tnext_t = 0%lo, info_t= 0%lo\n",type_t, tnext_t, info_t);
#endif
	type = (TWORD) type_t;
	if ((t = FindBType(type)) == TNIL)
		t = tynalloc(type);
#endif
	ot = t;

#ifdef	BIO
	while (ty.next != TNIL) {
 		if (ISFTN(t)) {
			if (ty.ftn_parm != PNIL) {	/* is a PPTR */
#else

	while (tnext = (void *) tnext_t) {
#ifdef FRED_DBG
	  if (fdebug)
	    printf ("Processing tnext_t = 0%lo", tnext_t);
#endif
 		if (ISFTN(t)) {
#ifdef FRED_DBG
		  if (fdebug)
		    printf("\n\t Its a function!");
#endif
			if (info_t) {	/* is a PPTR */
#ifdef FRED_DBG
			  if (fdebug)
			    printf("\n\t\t info (a pointer) = %lo",info_t);  
#endif
#endif
				t->ftn_parm = p = parmalloc();
				do {
#ifdef	BIO
					last_read_debug = fread((char *) p,
						sizeof(struct parminfo),1,fp);
					if (last_read_debug)  
					  total_bytes_read_debug += last_read_debug; 
					p->type = InType();
					if (p->next == PNIL)
#else
					fscanf(fp, "%lo %lo", &type_t, &pnext_t);
#ifdef FRED_DBG
					if (fdebug)
					  printf(" parminfo.. type_t = %lo, pnext_t = %lo\n",type_t,pnext_t);
#endif
					p->type = InType();
					if (!pnext_t)
#endif
						break;
					p->next = parmalloc();
				} while (p = p->next);
			}
 		}
		else if (ISARY(t))
#ifndef BIO
#ifdef FRED_DBG
		  if (fdebug)
		    printf("\n\t Its an ARRAY! size = %o ",((unsigned) info_t));  
#endif
#endif
#ifdef	BIO
			t->ary_size = ty.ary_size;
#else
			t->ary_size = (info = (unsigned) info_t);
#endif
#ifdef	BIO
		last_read_debug = fread((char *) &ty, sizeof(struct tyinfo), 1, fp);
		if (last_read_debug)  total_bytes_read_debug += last_read_debug; 
		if ((t->next = FindBType(ty.tword)) == TNIL)
			t->next = tynalloc(ty.tword);
#else
		fscanf(fp, "%lo %lo %lo", &type_t, &tnext_t, &info_t);
#ifdef FRED_DBG
	  if (fdebug)
	    printf(" array info type_t = %lo, tnext_t = %lo, info_t = %lo\n",type_t,tnext_t,pnext_t);
#endif

		if ((t->next = FindBType(type= (TWORD) type_t )) == TNIL)
			t->next = tynalloc(type);
#endif
		t = t->next;
	}
	return (ot);
}

/*
** Simple file open/close functions.
*/
OpenFile()
{
	if ((fp = fopen(fname, "r")) == NULL) {
		cerror(MSGSTR(M_MSG_266, "can't open file %s\n"), fname);
		exit(1);
	}
	markerEOF = 0;
#ifdef FRED_DBG
	  if (fdebug)
	    printf("\n%s opened\n", fname);
#endif
}

CloseFile()
{
	fclose(fp);
}
