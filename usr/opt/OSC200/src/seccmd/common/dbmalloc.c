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
static char	*sccsid = "@(#)$RCSfile: dbmalloc.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:02:06 $";
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
 * The following code is for the debug of allocated memory.
 * It maintains a circular buffer of malloc records that allow you
 * to walk back through malloc history and match allocations with release.
 */

#ifdef DEBUG_MALLOC

#include <stdio.h>

static FILE *dmfp;
FILE *fopen();

#define PR_DM1(F,B) {if (dmfp || (dmfp = fopen (".malloc_log", "w"))) {\
	fprintf (dmfp, "%04d: %s %08x", nmalloc_circ, F, B); fflush (dmfp);}}
#define PR_DM2(P) {if (dmfp) {fprintf (dmfp, " = %8x\n", P); fflush (dmfp);}}

#undef malloc
#undef realloc
#undef calloc
#undef free
#undef strdup

extern char *malloc(), *realloc(), *calloc(), *strdup();
extern void free();

#define MALLOC_CIRC_SIZE 1000

struct malloc_circ {
	int size;
	char *ptr1;
	char *ptr2;
	char func[4];
} malloc_circ[MALLOC_CIRC_SIZE];
int nmalloc_circ = 0;

char *
dbmalloc(bytes)
int bytes;
{
	struct malloc_circ *mp = &malloc_circ[nmalloc_circ];
	char *cp;

	if (++nmalloc_circ == MALLOC_CIRC_SIZE)
		nmalloc_circ = 0;
	PR_DM1 ("mal", bytes);
	cp = malloc (bytes);
	PR_DM2 (cp);
	mp->size = bytes;
	mp->ptr1 = cp;
	mp->ptr2 = (char *) 0;
	strcpy (mp->func, "mal");
	return(cp);
}

char *
dbrealloc(pointer, newsize)
char *pointer;
int newsize;
{
	struct malloc_circ *mp = &malloc_circ[nmalloc_circ];
	char *cp;

	if (++nmalloc_circ == MALLOC_CIRC_SIZE)
		nmalloc_circ = 0;
	PR_DM1 ("rea", newsize);
	cp = realloc (pointer, newsize);
	PR_DM2 (cp);
	mp->size = newsize;
	mp->ptr1 = pointer;
	mp->ptr2 = cp;
	strcpy (mp->func, "rea");
	return(cp);
}

char *
dbcalloc(nelem, elsize)
int nelem;
int elsize;
{
	struct malloc_circ *mp = &malloc_circ[nmalloc_circ];
	char *cp;

	if (++nmalloc_circ == MALLOC_CIRC_SIZE)
		nmalloc_circ = 0;
	PR_DM1 ("cal", nelem*elsize);
	cp = calloc (nelem, elsize);
	PR_DM2 (cp);
	mp->size = nelem*elsize;
	mp->ptr1 = cp;
	mp->ptr2 = (char *) 0;
	strcpy (mp->func, "cal");
	return(cp);
}

void
dbfree(pointer)
char *pointer;
{
	struct malloc_circ *mp = &malloc_circ[nmalloc_circ];

	if (++nmalloc_circ == MALLOC_CIRC_SIZE)
		nmalloc_circ = 0;
	PR_DM1 ("fre", pointer);
	free(pointer);
	PR_DM2 (pointer);
	mp->size = 0;
	mp->ptr1 = pointer;
	mp->ptr2 = (char *) 0;
	strcpy(mp->func, "fre");
}

char *
dbstrdup(pointer)
char *pointer;
{
	struct malloc_circ *mp = &malloc_circ[nmalloc_circ];
	char *cp;

	if (++nmalloc_circ == MALLOC_CIRC_SIZE)
		nmalloc_circ = 0;
	PR_DM1 ("dup", strlen(pointer) + 1);
	/* note: some systems do not have strdup */
	cp = malloc(strlen(pointer) + 1);
	PR_DM2 (cp);
	if (cp) {
		strcpy(cp, pointer);
		mp->size = strlen(pointer);
		mp->ptr1 = cp;
		mp->ptr2 = (char *) 0;
		strcpy(mp->func, "dup");
	}
	return(cp);
}
#endif

#ifndef SYSV_3
extern char *malloc();
char *
strdup(p)
	char    *p;
{
	char    *np = malloc(strlen(p) + 1);

	if (np)
		strcpy(np, p);

	return np;
}
#endif
