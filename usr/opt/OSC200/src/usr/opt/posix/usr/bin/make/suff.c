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
static char *rcsid = "@(#)$RCSfile: suff.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/29 22:05:54 $";
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
 * suff.c --
 *	Functions to maintain suffix lists and find implicit dependents
 *	prerequsits, sources using suffix transformation rules
 *
 * Interface:
 *	Suff_Init 	    	Initialize all things to do with suffixes.
 *
 *	Suff_DoPaths	    	This function is used to make life easier
 *	    	  	    	when searching for a file according to its
 *	    	  	    	suffix. It takes the global search path,
 *	    	  	    	as defined using the .PATH: target, and uses
 *	    	  	    	its directories as the path of each of the
 *	    	  	    	defined suffixes.
 *
 *	Suff_ClearSuffixes  	Clear out all the suffixes and defined
 *	    	  	    	transformations.
 *
 *	Suff_IsTransform    	Return TRUE if the passed string is the lhs
 *	    	  	    	of a transformation rule.
 *
 *	Suff_AddSuffix	    	Add the passed string as another known suffix.
 *
 *	Suff_AddTransform   	Add another transformation to the suffix
 *	    	  	    	graph. Returns  GNode suitable for framing, I
 *	    	  	    	mean, tacking commands, attributes, etc. on.
 *
 *	Suff_FindDeps	    	Find implicit sources for and the location of
 *	    	  	    	a target based on its suffix. Returns the
 *	    	  	    	bottom-most node added to the graph or NILGNODE
 *	    	  	    	if the target had no implicit sources.
 */

#include    	  <stdio.h>
#include    	  <stdlib.h>
#include	  "make.h"
#include    	  "bit.h"
#include	  "pmake_msg.h"


static Lst       sufflist;	/* Lst of suffixes */
static Lst       transforms;	/* Lst of transformation rules */

static int       sNum = 0;	/* Counter for assigning suffix numbers */
extern nl_catd	 catd;


/*
 * Trace macro used only during development for tracing suffix module.
 * Use the compiler option -DSUFFTRACE or -DALLTRACE to turn on.
 */

#if defined(SUFFTRACE) || defined(ALLTRACE)
#undef SUFFTRACE
#define SUFFTRACE(message) { \
   printf("SUFFTRACE: %s.\n",message);\
   }
#else
#define SUFFTRACE(message)
#endif


/*
 * Structure describing an individual suffix.
 */
typedef struct _Suff {
    char         *name;	    	/* The suffix itself (single or double) */
    int		 nameLen;	/* Length of the suffix */
    short	 flags;      	/* Type of suffix */
#define SUFF_NULL 	  0x01	    /* The empty suffix */
#define SUFF_SCCS 	  0x02	    /* This is an SCCS suffix */
    int          sNum;	      	/* The suffix number */
    Lst          parents;	/* Suffixes we have a transformation to */
    Lst          children;	/* Suffixes we have a transformation from */
} Suff;

/*
 * Structure used in the search for Implied Sources.
 */

typedef struct _ISrc {
    char            *file;	/* The file to look for */
    char            *sccsgetname;	/* (jed) under development The sccs file if found */
    char    	    *pref;  	/* Prefix from which file was formed */
    Suff            *suff;	/* The suffix on the file */
    struct _ISrc     *parent;	/* The ISrc for which this is a source */
    GNode           *node;	/* The node describing the file */
    int	    	    children;	/* Count of existing children (so we don't free
				 * this thing too early or never nuke it) */
} ISrc;

static Suff 	    *suffNull;	/* The NULL suffix for this run */
static Suff 	    *emptySuff;	/* The empty suffix required for POSIX
				 * single-suffix transformation rules */

	/*************** Lst Predicates ****************/
/*-
 *-----------------------------------------------------------------------
 * SuffStrIsPrefix  --
 *	See if pref is a prefix of str.
 *
 * Results:
 *	NULL if it ain't, pointer to character in str after prefix if so
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
static char    *
SuffStrIsPrefix (char *pref, char *str)
{
    /* char  *pref;	possible prefix */
    /* char  *str;	string to check */

/*    SUFFTRACE("SuffStrIsPrefix: See if pref is a prefix of str" ); */

    while (*str && *pref == *str) {
	pref++;
	str++;
    }

    return (*pref ? NULL : str);
}

/*-
 *-----------------------------------------------------------------------
 * SuffSuffIsSuffix  --
 *	See if suff->name is a suffix of str. Str should point to THE END of the
 *	string to check. (THE END == the null byte)
 *
 * Results:
 *	NULL if it ain't, pointer to character in str before suffix if
 *	it is.
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
static char *
SuffSuffIsSuffix (Suff *s, char *str)
{
    /* Suff           *s;	possible suffix */
    /* char           *str;	string to examine */

    char  *p1;	    		/* Pointer into suffix name */
    char  *p2;	    		/* Pointer into string being examined */


 /*   SUFFTRACE("SuffSuffIsSuffix: See if suff->name is a suffix of str" ); */

    p1 = s->name + s->nameLen;
    if (s->flags & SUFF_SCCS)
	p1--;
    p2 = str;

    while (p1 >= s->name && *p1 == *p2) {
	p1--;
	p2--;
    }

    return (p1 == s->name - 1 ? p2 : NULL);
}

/*-
 *-----------------------------------------------------------------------
 * SuffSuffIsSuffixP --
 *	Predicate (form of SuffSuffIsSuffix. Passed as the callback function
 *	to Lst_Find.
 *
 * Results:
 *	0 if the suffix is the one desired, non-zero if not.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
static int
SuffSuffIsSuffixP(Suff *s, char *str)
{
    /* Suff           *s;	possible suffix */
    /* char           *str;	string to examine */    

  /*  SUFFTRACE("SuffSuffIsSuffixP: See if suff->name is NOT a suffix of str" ); */
    return(!SuffSuffIsSuffix(s, str));
}

/*-
 *-----------------------------------------------------------------------
 * SuffSuffHasNameP --
 *	Callback procedure for finding a suffix based on its name.
 *
 * Results:
 *	0 if the suffix is of the given name. non-zero otherwise.
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
static int
SuffSuffHasNameP (Suff *s, char *sname)
{
    /* Suff    *s;	    Suffix to check */
    /* char    *sname; 	    Desired name */

/*    SUFFTRACE("SuffSuffHasName: Callback for finding a suffix based on its name"); */

    return (strcmp (sname, s->name));
}

/*-
 *-----------------------------------------------------------------------
 * SuffSuffIsPrefix  --
 *	See if the suffix described by s is a prefix of the string. Care
 *	must be taken when using this to search for transformations and
 *	what-not, since there could well be two suffixes, one of which
 *	is a prefix of the other...
 *
 * Results:
 *	0 if s is a prefix of str. non-zero otherwise
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
static int
SuffSuffIsPrefix (Suff *s, char *str)
{
    /* Suff           *s;      suffix to compare */
    /* char           *str;       string to examine */

  /*  SUFFTRACE("SuffSuffIsPrefix: See if the suffix described by s is a prefix of the string"); */

    return (SuffStrIsPrefix (s->name, str) == NULL ? 1 : 0);
}

/*-
 *-----------------------------------------------------------------------
 * SuffGNHasNameP  --
 *	See if the graph node has the desired name
 *
 * Results:
 *	0 if it does. non-zero if it doesn't
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
static int
SuffGNHasNameP (GNode *gn, char *name)
{
    /* GNode          *gn;	current node we're looking at */
    /* char           *name;	name we're looking for */


 /*   SUFFTRACE("SuffGNHasNameP: See if the graph node has the desired name"); */

    return (strcmp (name, gn->name));
}

 	    /*********** Maintenance Functions ************/
/*-
 *-----------------------------------------------------------------------
 * SuffFree  --
 *	Free up all memory associated with the given suffix structure.
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	the suffix entry is detroyed
 *-----------------------------------------------------------------------
 */
static void
SuffFree (Suff *s)
{
    /* Suff           *s; */


    SUFFTRACE("SuffFree: Free up all memory associated with the given suffix structure");

    Lst_Destroy (s->children, NOFREE);
    Lst_Destroy (s->parents, NOFREE);
    free ((Address)s->name);
    free ((Address)s);
    return;
}

/*-
 *-----------------------------------------------------------------------
 * SuffInsert  --
 *	Insert the suffix into the list keeping the list ordered by suffix
 *	numbers.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Not really
 *-----------------------------------------------------------------------
 */

static void
SuffInsert (Lst l, Suff *s)
{
    /* Lst           l;		The list where in s should be inserted */
    /* Suff          *s;	The suffix to insert */

    LstNode 	  ln;		/* current element in l we're examining */
    Suff          *s2;		/* the suffix descriptor in this element */

    SUFFTRACE("SuffInsert: Insert the suffix into the list keeping the list ordered");
    if (Lst_Open (l) == FAILURE) {
	return;
    }
    s2 = NULL;
    while ((ln = Lst_Next (l)) != NILLNODE) {
	s2 = (Suff *) Lst_Datum (ln);
	if (s2->sNum >= s->sNum) {
	    break;
	}
    }

    Lst_Close (l);
    if (DEBUG(SUFF)) {
	printf(catgets(catd, MS_DEBUG, SUFF001, "inserting %s(%d)..."),
	       s->name, s->sNum);
    }
    if (ln == NILLNODE) {
	if (DEBUG(SUFF)) {
	    printf(catgets(catd, MS_DEBUG, SUFF002, "at end of list\n"));
	}
	(void)Lst_AtEnd (l, (ClientData)s);
    } else if (s2->sNum != s->sNum) {
	if (DEBUG(SUFF)) {
	    printf(catgets(catd, MS_DEBUG, SUFF003, "before %s(%d)\n"),
	           s2->name, s2->sNum);
	}
	(void)Lst_Insert (l, ln, (ClientData)s);
    } else if (DEBUG(SUFF)) {
	printf(catgets(catd, MS_DEBUG, SUFF004, "already there\n"));
    }
    return;
}

/*-
 *-----------------------------------------------------------------------
 * Suff_ClearSuffixes --
 *	This is gross. Nuke the list of suffixes but keep all transformation
 *	rules around. The transformation graph is destroyed in this process,
 *	but we leave the list of rules so when a new graph is formed the rules
 *	will remain.
 *	This function is called from the parse module when a
 *	.SUFFIXES:\n line is encountered.
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	the sufflist and its graph nodes are destroyed
 *-----------------------------------------------------------------------
 */
void
Suff_ClearSuffixes (void)
{

    SUFFTRACE("Suff_ClearSuffixes: Clear the list of suffixes but keep all transformation rules");

    Lst_Destroy (sufflist, (void (*)(void*))SuffFree);

    sufflist = Lst_Init(FALSE);
    sNum = 1;
    suffNull = emptySuff;
    suffNull->children =    Lst_Init (FALSE);
    suffNull->parents =	    Lst_Init (FALSE);
    return;
}

/*-
 *-----------------------------------------------------------------------
 * SuffParseTransform --
 *	Parse a transformation string to find its two component suffixes.
 *      example: .c.o     .f.o 
 *
 * DOUBLE SUFFIXES RULES
 *
 * Results:
 *	TRUE if the string is a valid transformation and FALSE otherwise.
 *
 * Side Effects:
 *	The passed pointers are overwritten.
 *
 *-----------------------------------------------------------------------
 */
static Boolean
SuffParseTransform(char *str, Suff **srcSuffPtr, Suff **targSuffPtr)
{
    /* char    	  	*str;	    	        String being parsed */
    /* Suff    	  	**srcSuffPtr;   	Place to store source of trans. */
    /* Suff    	  	**targSuffPtr;  	Place to store target of trans. */

    LstNode	srcLn;	                        /* element in suffix list of trans source*/
    Suff    	*srcSuff;                       /* Source of transformation */
    LstNode     targLn;	                        /* element in suffix list of trans target*/
    char    	*str2;	                        /* Extra pointer (maybe target suffix) */
    LstNode     singleLn;                       /* element in suffix list of any suffix
				                 * that exactly matches str */
    Suff        *singleSuff;                    /* Source of possible transformation to
				                 * null suffix */
    srcLn = NILLNODE;
    singleLn = NILLNODE;
    singleSuff = NULL;

    SUFFTRACE("SuffParseTransform: Parse a transformation string to find its two component suffixes");
    
    /*
     * Loop looking first for a suffix that matches the start of the
     * string and then for one that exactly matches the rest of it. If
     * we can find two that meet these criteria, we've successfully
     * parsed the string.
     */
    while (1) {
	if (srcLn == NILLNODE) {
	    srcLn = Lst_Find(sufflist, (ClientData)str, (int(*)(void*,void*))SuffSuffIsPrefix);
	} else {
	    srcLn = Lst_FindFrom (sufflist, Lst_Succ(srcLn), (ClientData)str,
				  (int(*)(void*,void*))SuffSuffIsPrefix);
	}
	if (srcLn == NILLNODE) {
	    /*
	     * Ran out of source suffixes -- no such rule
	     */
	    if (singleLn != NILLNODE) {
		/*
		 * Not so fast Mr. Smith! There was a suffix that encompassed
		 * the entire string, so we assume it was a transformation
		 * to the null suffix (thank you POSIX). We still prefer to
		 * find a double rule over a singleton, hence we leave this
		 * check until the end.
		 *
		 * XXX: Use emptySuff over suffNull?
		 */
		*srcSuffPtr = singleSuff;
		*targSuffPtr = suffNull;
		return(TRUE);
	    }
	    return (FALSE);
	}
	srcSuff = (Suff *) Lst_Datum (srcLn);
	str2 = str + srcSuff->nameLen;
	if (*str2 == '\0') {
	    singleSuff = srcSuff;
	    singleLn = srcLn;
	} else {
	    targLn = Lst_Find(sufflist, (ClientData)str2, (int(*)(void*,void*))SuffSuffHasNameP);
	    if (targLn != NILLNODE) {
		*srcSuffPtr = srcSuff;
		*targSuffPtr = (Suff *)Lst_Datum(targLn);
		return (TRUE);
	    }
	}
    }
}

/*-
 *-----------------------------------------------------------------------
 * Suff_IsTransform  --
 *	Return TRUE if the given string is a transformation rule
 *
 *
 * Results:
 *	TRUE if the string is a concatenation of two known suffixes.
 *	FALSE otherwise
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
Boolean
Suff_IsTransform (char *str)
{
    /* char          *str;	    	string to check */
    Suff    	  *srcSuff, *targSuff;

    SUFFTRACE("Suff_IsTransform: Return TRUE if the given string is a transformation rule");

    return (SuffParseTransform(str, &srcSuff, &targSuff));
}

/*-
 *-----------------------------------------------------------------------
 * Suff_AddTransform --
 *	Add the transformation rule described by the line to the
 *	list of rules and place the transformation itself in the transformation 
 *      graph. 
 *
 * Results:
 *	The node created for the transformation in the transforms list (graph)
 *
 * Side Effects:
 *	The node is placed on the end of the transforms Lst and links are
 *	made between the two suffixes mentioned in the target name
 *-----------------------------------------------------------------------
 */
GNode *
Suff_AddTransform (char *line)
{
    /* char          *line;	name of transformation to add */

    GNode         *gn;		/* GNode of transformation rule */
    Suff          *sSuff,       /* source suffix (left) side of double suffix rule. Example: '.c.o'  = '.c'   */				
                  *tSuff;	/* target suffix (right) side of double suffix rule. Example: '.c.o' = '.o'  */
    LstNode 	  ln;	    	/* Node for existing transformation */

				/* Does the transformation rule exist? */
    SUFFTRACE("Suff_AddTransform: Add the transformation rule described by the line to the transformation list");

    ln = Lst_Find (transforms, (ClientData)line, (int(*)(void*,void*))SuffGNHasNameP);

    if (ln == NILLNODE) {
	/*
	 * Make a new graph node for the transformation. It will be filled in
	 * by the Parse module. 
	 */
	gn = Targ_NewGN (line);
	(void)Lst_AtEnd (transforms, (ClientData)gn);
    } else {
	/*
	 * New specification for transformation rule. Just nuke the old list
	 * of commands so they can be filled in again... We don't actually
	 * free the commands themselves, because a given command can be
	 * attached to several different transformations.
	 */
	gn = (GNode *) Lst_Datum (ln);
	Lst_Destroy (gn->commands, NOFREE);
	Lst_Destroy (gn->children, NOFREE);
	gn->commands = Lst_Init (FALSE);
	gn->children = Lst_Init (FALSE);
    }

    gn->type = OP_TRANSFORM;

    (void)SuffParseTransform(line, &sSuff, &tSuff);

    /*
     * link the two together in the proper relationship and order 
     */
    if (DEBUG(SUFF)) {
	printf(catgets(catd, MS_DEBUG, SUFF005, 
	  "defining transformation from `%s' to `%s'\n"), sSuff->name, tSuff->name);
    }
    SuffInsert (tSuff->children, sSuff);
    SuffInsert (sSuff->parents, tSuff);
    return (gn);
}

/*-
 *-----------------------------------------------------------------------
 * Suff_EndTransform --
 *	Handle the finish of a transformation definition, removing the
 *	transformation from the graph if it has neither commands nor
 *	sources. This is a callback procedure for the Parse module via
 *	Lst_ForEach
 *
 * Results:
 *	=== 0
 *
 * Side Effects:
 *	If the node has no commands or children, the children and parents
 *	lists of the affected suffices are altered.
 *
 *-----------------------------------------------------------------------
 */
int
Suff_EndTransform(GNode *gn, ...)
{
    /* GNode   *gn;    	Node for transformation */

    SUFFTRACE("Suff_EndTransform: Remove the transformation definition from the graph");

    if ((gn->type & OP_TRANSFORM) && (Lst_IsEmpty(gn->commands)) && 
	(strcmp(gn->name,".SCCS_GET")) && (strcmp(gn->name,".DEFAULT")) && (Lst_IsEmpty(gn->children)))
    {
	Suff	*sSuff, *tSuff;
	LstNode	ln;

	(void)SuffParseTransform(gn->name, &sSuff, &tSuff);

	if (DEBUG(SUFF)) {
	    printf(catgets(catd, MS_DEBUG, SUFF006,
	      "deleting transformation from %s to %s\n"), sSuff->name, tSuff->name);
	}

	/*
	 * Remove the source from the target's children list. We check for a
	 * nil return to handle a beanhead saying something like
	 *  .c.o .c.o:
	 *
	 * We'll be called twice when the next target is seen, but .c and .o
	 * are only linked once...
	 */

	ln = Lst_Member(tSuff->children, (ClientData)sSuff);
        

	if (ln != NILLNODE) {
	    (void)Lst_Remove(tSuff->children, ln);
	}

	/*
	 * Remove the target from the source's parents list
	 */
	ln = Lst_Member(sSuff->parents, (ClientData)tSuff);
	if (ln != NILLNODE) {
	    (void)Lst_Remove(sSuff->parents, ln);
	}
    } else if ((gn->type & OP_TRANSFORM) && DEBUG(SUFF)) {
	printf(catgets(catd, MS_DEBUG, SUFF007,
	  "Transformation %s Complete\n"), gn->name);
    }

    return(0);
}

/*-
 *-----------------------------------------------------------------------
 * SuffRebuildGraph --
 *	Called from Suff_AddSuffix via Lst_ForEach to search through the
 *	list of existing transformation rules and rebuild the transformation
 *	graph when it has been destroyed by Suff_ClearSuffixes. If the
 *	given rule is a transformation involving this suffix and another,
 *	existing suffix, the proper relationship is established between
 *	the two.
 *
 * Results:
 *	Always 0.
 *
 * Side Effects:
 *	The appropriate links will be made between this suffix and
 *	others if transformation rules exist for it.
 *
 *-----------------------------------------------------------------------
 */
static int
SuffRebuildGraph(GNode *transformGn, Suff *inSuff)
{
    /* GNode   	  	*transformGn; Transformation to test */
    /* Suff    	  	*inSuff;    Suffix to rebuild */

    char 	*cp;
    LstNode	ln;
    Suff  	*tmpSuff;

    SUFFTRACE("SuffRebuildGraph: Search the list of existing transform rules and rebuild the transform graph");

    /*
     * First see if it is a transformation from this suffix.
     */
    cp = SuffStrIsPrefix(inSuff->name, transformGn->name);
    if (cp != (char *)NULL) {
	ln = Lst_Find(sufflist, (ClientData)cp, (int(*)(void*,void*))SuffSuffHasNameP);
	if (ln != NILLNODE) {
	    /*
	     * Found target. Link in and return, since it can't be anything
	     * else.
	     */
	    tmpSuff = (Suff *)Lst_Datum(ln);
	    SuffInsert(tmpSuff->children, inSuff);
	    SuffInsert(inSuff->parents, tmpSuff);
	    return(0);
	}
    }

    /*
     * Not from, maybe to?
     */
    cp = SuffSuffIsSuffix(inSuff, transformGn->name + strlen(transformGn->name));
    if (cp != (char *)NULL) {
	/*
	 * Null-terminate the source suffix in order to find it.
	 */
	cp[1] = '\0';
	ln = Lst_Find(sufflist, (ClientData)transformGn->name, (int(*)(void*,void*))SuffSuffHasNameP);
	/*
	 * Replace the start of the target suffix
	 */
	cp[1] = inSuff->name[0];
	if (ln != NILLNODE) {
	    /*
	     * Found it -- establish the proper relationship
	     */
	    tmpSuff = (Suff *)Lst_Datum(ln);
	    SuffInsert(inSuff->children, tmpSuff);
	    SuffInsert(tmpSuff->parents, inSuff);
	}
    }
    return(0);
}

/*-
 *-----------------------------------------------------------------------
 * Suff_AddSuffix --
 *	Add the suffix in string to the end of the list of known suffixes.
 *	Should we restructure the suffix graph? Make doesn't...
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	A GNode is created for the suffix and a Suff structure is created and
 *	added to the suffixes list unless the suffix was already known.
 *-----------------------------------------------------------------------
 */
void
Suff_AddSuffix (char *str)
{
    /* char          *str;   The name of the suffix to add */

    Suff          *newSuff;	    /* new suffix descriptor */
    LstNode 	  ln;

    SUFFTRACE("Suff_AddSuffix: Add suffix name to list of known suffixes");

    ln = Lst_Find (sufflist, (ClientData)str, (int(*)(void*,void*))SuffSuffHasNameP);
    if (ln == NILLNODE) {
	newSuff = (Suff *) emalloc (sizeof (Suff));

	newSuff->name =   	strdup (str);
	newSuff->nameLen = 	strlen (newSuff->name);
	newSuff->children = 	Lst_Init (FALSE);
	newSuff->parents = 	Lst_Init (FALSE);
	newSuff->sNum =   	sNum++;
	newSuff->flags =  	0;
	if (newSuff->nameLen && *(newSuff->name + newSuff->nameLen - 1) == '~')
	    newSuff->flags |= SUFF_SCCS;

	(void)Lst_AtEnd (sufflist, (ClientData)newSuff);
	/*
	 * Look for any existing transformations from or to this suffix.
	 * XXX: Only do this after a Suff_ClearSuffixes?
	 */
	Lst_ForEach (transforms, (int(*)(void*,void*))SuffRebuildGraph, (ClientData)newSuff);
    } 
    return;
}

 	  /********** Implicit Source Search Functions *********/
/*
 * A structure for passing more than one argument to the Lst-library-invoked
 * function...
 */
typedef struct {
    Lst            l;
    ISrc            *is;
} LstISrc;

/*-
 *-----------------------------------------------------------------------
 * SuffAddISrc  --
 *	Add a suffix as a ISrc structure to the given list with its parent
 *	being the given ISrc structure. If the suffix is the null suffix,
 *	the prefix is used unaltered as the file name in the ISrc structure.
 *
 * Results:
 *	always returns 0
 *
 * Side Effects:
 *	A ISrc structure is created and tacked onto the end of the list
 *-----------------------------------------------------------------------
 */
static int
SuffAddISrc (Suff *inSuff, LstISrc *lsLstISrc)
{
 /* Suff	*inSuff;	    suffix for which to create a ISrc structure */
 /* LstISrc     *lsLstISrc;	    list and parent for the new ISrc */

    ISrc        *newISrc;	    /* new ISrc structure */
    ISrc    	*targISrc; 	    /* Target structure */

    targISrc = lsLstISrc->is;

    SUFFTRACE("SuffAddISrc: Add a suffix as a ISrc structure");
    
    if ((inSuff->flags & SUFF_NULL) && (*inSuff->name != '\0')) {
	/*
	 * If the suffix has been marked as the NULL suffix, also create a ISrc
	 * structure for a file with no suffix attached. Two birds, and all
	 * that...
	 */
	newISrc = (ISrc *) emalloc (sizeof (ISrc));
	newISrc->file =  	strdup(targISrc->pref);
	newISrc->sccsgetname =  strdup(""); /* jed under development  */
	newISrc->pref =  	targISrc->pref;
	newISrc->parent = 	targISrc;
	newISrc->node =  	NILGNODE;
	newISrc->suff =  	inSuff;
	newISrc->children =	0;
	targISrc->children += 1;
	(void)Lst_AtEnd (lsLstISrc->l, (ClientData)newISrc);
      }
    newISrc = (ISrc *) emalloc (sizeof (ISrc));
    if (inSuff->flags & SUFF_SCCS) {
      char *p;
      
      newISrc->file = p = (char *)emalloc (2 + strlen(targISrc->pref) + inSuff->nameLen - 1 + 1);
      *p++ = 's';
      *p++ = '.';
      strcpy(p, targISrc->pref);
      p += strlen(p);
      strncpy(p, inSuff->name, inSuff->nameLen - 1);
      p += inSuff->nameLen - 1;
      *p = 0;
      newISrc->sccsgetname =  strdup(""); /* jed under development */
    } else
      newISrc->file =  Str_Concat (targISrc->pref, inSuff->name, 0);
    {
      char *p;
      
      newISrc->sccsgetname = p = (char *)emalloc (2 + strlen(targISrc->pref) + inSuff->nameLen + 1);
      *p++ = 's';
      *p++ = '.';
      strcpy(p, targISrc->pref);
      p += strlen(p);
      strncpy(p, inSuff->name, inSuff->nameLen);
      p += inSuff->nameLen;
      *p = 0;
    }
      newISrc->pref =	 targISrc->pref;
      newISrc->parent =    targISrc;
      newISrc->node = 	 NILGNODE;
      newISrc->suff = 	 inSuff;
      newISrc->children =  0;
      targISrc->children += 1;
    (void)Lst_AtEnd (lsLstISrc->l, (ClientData)newISrc);
    
    return(0);
}

/*-
 *-----------------------------------------------------------------------
 * SuffCouldBeMadeFrom  --
 *	Add all the children (sources) of the target (parent) as 
 *      ISrc structures to the given list
 *
 * Results:
 *	None
 *
 * Side Effects:
 * 	Lots of structures are created and added to the list
 *-----------------------------------------------------------------------
 */
static void
SuffCouldBeMadeFrom (Lst l, ISrc *targ)
{
    /* Lst            l;		list to which to add the new level */
    /* ISrc            *targ;	        ISrc structure to use as the parent */

    LstISrc         ls;
    ls.is = targ;
    ls.l = l;

    SUFFTRACE("SuffCouldBeMadeFrom: Add all child (sources) of the target (parent)");

    Lst_ForEach (targ->suff->children, (int(*)(void*,void*))SuffAddISrc, (ClientData)&ls);
    return;
}

/*-
 *----------------------------------------------------------------------
 * SuffFreeISrc --
 *	Free all memory associated with a ISrc structure
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	The memory is free'd.
 *----------------------------------------------------------------------
 */
static void
SuffFreeISrc (ISrc *s)
{
    /* ISrc            *s; */

    SUFFTRACE("SuffFreeISrc: Free all memory associated with a ISrc structure");

    free ((Address)s->file);
    if (!s->parent) {
	free((Address)s->pref);
    } else if (--s->parent->children == 0 && s->parent->parent) {
	/*
	 * Parent has no more children, now we're gone, and it's not
	 * at the top of the tree, so blow it away too.
	 */
	SuffFreeISrc(s->parent);
    }
    free ((Address)s);
    return;
}

/*-
 *-----------------------------------------------------------------------
 * SuffFindThem --
 *	Find the first existing file/target in the list of ISrcs
 *
 * Results:
 *	The lowest structure in the chain of transformations
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 *
 */

static ISrc *
SuffFindThem (Lst srcsLst)
{
    /* Lst            srcsLst;	list of ISrc structures to search through */
    ISrc            *curISrc;	/* current ISrc */
    ISrc	    *retISrc;	/* returned ISrc */

    retISrc = (ISrc *) NULL;

    SUFFTRACE("SuffFindThem: Find the first existing file/target in the list of ISrcs");

    while (!Lst_IsEmpty (srcsLst)) {
	curISrc = (ISrc *) Lst_DeQueue (srcsLst);

	if (DEBUG(SUFF)) {
	    printf ("#Look file \"%s\" with implied suffix.\n", curISrc->file);
	}
	/*
	 * A file is considered to exist if either a node exists in the
	 * graph for it or the file actually exists.
	 */

	    /* If true The SCCS directory exists and the file is not an sccsfile */
	    /* Do not look for ./SCCS/s.file.c  when ./s.file exists */

	if (!(curISrc->suff->flags & SUFF_SCCS) && (TrySccsGet)) {

	if ((Targ_FindNode(curISrc->file, TARG_NOCREATE) != NILGNODE) ||
	    (Dir_FindFile (curISrc->file, dirSearchPath) != (char *) NULL) || 
	    (Dir_FindFile (curISrc->sccsgetname, dirSccsPath) != (char *) NULL))

	  {			/* found something  */
	    if (Dir_FindFile (curISrc->sccsgetname, dirSccsPath) != (char *) NULL) {
	        curISrc->node->SccsGetFileExists = TRUE;
                curISrc->node->sccsgetpath = Str_Concat ("SCCS", curISrc->sccsgetname,  STR_ADDSLASH);
	      }
	    retISrc = curISrc;
	    break;
	  } else {
	    if (DEBUG(SUFF)) {
	      printf (catgets(catd, MS_DEBUG, SUFF010, "File node, sccs, or local not found.\n"));
	    }
	    SuffCouldBeMadeFrom (srcsLst, curISrc);
	  }
      }	 else	
	{	

	  if ((Targ_FindNode(curISrc->file, TARG_NOCREATE) != NILGNODE) ||
	      (Dir_FindFile (curISrc->file, dirSearchPath) != (char *) NULL))
	    {
	      if (DEBUG(SUFF)) {
		printf (catgets(catd, MS_DEBUG, SUFF009, "Found  node or local file.\n"));
	      }
	      retISrc = curISrc;
	      break;
	    } else {
	      if (DEBUG(SUFF)) {
		printf (catgets(catd, MS_DEBUG, SUFF010, "File  node or local not found.\n"));
	      }
	      SuffCouldBeMadeFrom (srcsLst, curISrc);
	    }
	}				/* end (TrySccsGet */

      }				/* end while */
    return (retISrc);
}

/*-
 *-----------------------------------------------------------------------
 * SuffFindCmds --
 *	See if any children of the target in the ISrc structure is
 *	one from which the target can be transformed. If there is one,
 *	a ISrc structure is put together for it and returned.
 *
 *      Do any children of the target hold a transformation rule for the target? 
 *
 * Results:
 *	The ISrc structure of the "winning" child, or NIL if no such beast.
 *
 * Side Effects:
 *	A ISrc structure may be allocated.
 *
 *-----------------------------------------------------------------------
 */
static ISrc *
SuffFindCmds (ISrc *targISrc)
{
 /* ISrc	*targISrc;	ISrc structure to play with */

    LstNode 	ln; 	        /* General-purpose list node */
    GNode	*tGn;	 	/* Target GNode */
    GNode  	*sGn; 		/* Source GNode */
    int	    	prefLen;        /* The length of the defined prefix */
    Suff    	*suffSuff;	/* Suffix on matching beastie */
    ISrc	*retISrc;	/* Return value */
    char    	*cp;

    SUFFTRACE("SuffFindCmds: Does any child of target hold a transformation rule for the target?");

    tGn = targISrc->node;
    (void) Lst_Open (tGn->children);
    prefLen = strlen (targISrc->pref);

    while ((ln = Lst_Next (tGn->children)) != NILLNODE) {
	sGn = (GNode *)Lst_Datum (ln);

	cp = rindex (sGn->name, '/');
	if (cp == (char *)NULL) {
	    cp = sGn->name;
	} else {
	    cp++;
	}
	if (strncmp (cp, targISrc->pref, prefLen) == 0) {
	    /*
	     * The node matches the prefix ok, see if it has a known
	     * suffix.
	     */
	    ln = Lst_Find (sufflist, (ClientData)&cp[prefLen],
			   (int(*)(void*,void*))SuffSuffHasNameP);
	    if (ln != NILLNODE) {
		/*
		 * It even has a known suffix, see if there's a transformation
		 * defined between the node's suffix and the target's suffix.
		 *
		 * XXX: Handle multi-stage transformations here, too.
		 */
		suffSuff = (Suff *)Lst_Datum (ln);

		if (Lst_Member (suffSuff->parents,
				(ClientData)targISrc->suff) != NILLNODE)
		{
		    /*
		     * Hot Damn! Create a new ISrc structure to describe
		     * this transformation (making sure to duplicate the
		     * source node's name so Suff_FindDeps can free it
		     * again (ick)), and return the new structure.
		     */
		    retISrc = (ISrc *)emalloc (sizeof(ISrc));
		    retISrc->file = strdup(sGn->name);
		    retISrc->pref = targISrc->pref;
		    retISrc->suff = suffSuff;
		    retISrc->parent = targISrc;
		    retISrc->node = sGn;
		    retISrc->children = 0;
		    targISrc->children += 1;
		    if (DEBUG(SUFF)) {
			printf ("\tusing existing source %s\n", sGn->name);
		    }
		    return (retISrc);
		}
	    }
	}
    }
    Lst_Close (tGn->children);
    return ((ISrc *)NULL);
}

/*-
 *-----------------------------------------------------------------------
 * SuffExpandChildren --
 *	Expand the names of any children of a given node that contain
 *	variable invocations or file wildcards into actual targets.
 *
 * Results:
 *	=== 0 (continue)
 *
 * Side Effects:
 *	The expanded node is removed from the parent's list of children,
 *	and the parent's unmade counter is decremented, but other nodes
 * 	may be added.
 *
 *-----------------------------------------------------------------------
 */
static int
SuffExpandChildren(GNode *cGn, GNode *pGn)
{
    /* GNode   	*cGn;	    Child to examine */
    /* GNode   	*pGn;	    Parent node being processed */

    GNode	*gn;	    /* New source 8) */
    LstNode   	prevLN;    /* Node after which new source should be put */
    LstNode	ln; 	    /* List element for old source */
    char	*cp;	    /* Expanded value */

    SUFFTRACE("SuffExpandChildren: Expand the names of any children of a given node that contain wildcards");

    /*
     * New nodes effectively take the place of the child, so place them
     * after the child
     */
    prevLN = Lst_Member(pGn->children, (ClientData)cGn);
    
    /*
     * First do variable expansion -- this takes precedence over
     * wildcard expansion. If the result contains wildcards, they'll be gotten
     * to later since the resulting words are tacked on to the end of
     * the children list.
     */
    if (index(cGn->name, '$') != (char *)NULL) {
	if (DEBUG(SUFF)) {
	    printf("Expanding \"%s\"...", cGn->name);
	}
	cp = Var_Subst(cGn->name, pGn, TRUE);

	if (cp != (char *)NULL) {
	    Lst	    members = Lst_Init(FALSE);
	    
	    if (cGn->type & OP_ARCHV) {
		/*
		 * Node was an archive(member) target, so we want to call
		 * on the Arch module to find the nodes for us, expanding
		 * variables in the parent's context.
		 */
		char	*sacrifice = cp;

		(void)Arch_ParseArchive(&sacrifice, members, pGn);
	    } else {
		/*
		 * Break the result into a vector of strings whose nodes
		 * we can find, then add those nodes to the members list.
		 * Unfortunately, we can't use Str_Break b/c it
		 * doesn't understand about variable specifications with
		 * spaces in them...
		 */
		char	    *start;
		char	    *initcp = cp;   /* For freeing... */

		for (start = cp; *start == ' ' || *start == '\t'; start++) {
		    ;
		}
		for (cp = start; *cp != '\0'; cp++) {
		    if (*cp == ' ' || *cp == '\t') {
			/*
			 * White-space -- terminate element, find the node,
			 * add it, skip any further spaces.
			 */
			*cp++ = '\0';
			gn = Targ_FindNode(start, TARG_CREATE);
			(void)Lst_AtEnd(members, (ClientData)gn);
			while (*cp == ' ' || *cp == '\t') {
			    cp++;
			}
			/*
			 * Adjust cp for increment at start of loop, but
			 * set start to first non-space.
			 */
			start = cp--;
		    } else if (*cp == '$') {
			/*
			 * Start of a variable spec -- contact variable module
			 * to find the end so we can skip over it.
			 */
			char	*junk;
			int 	len;
			Boolean	doFree;

			junk = Var_Parse(cp, pGn, TRUE, &len, &doFree);
			if (junk != var_Error) {
			    cp += len - 1;
			}

			if (doFree) {
			    free(junk);
			}
		    } else if (*cp == '\\' && *cp != '\0') {
			/*
			 * Escaped something -- skip over it
			 */
			cp++;
		    }
		}

		if (cp != start) {
		    /*
		     * Stuff left over -- add it to the list too
		     */
		    gn = Targ_FindNode(start, TARG_CREATE);
		    (void)Lst_AtEnd(members, (ClientData)gn);
		}
		/*
		 * Point cp back at the beginning again so the variable value
		 * can be freed.
		 */
		cp = initcp;
	    }
	    /*
	     * Add all elements of the members list to the parent node.
	     */
	    while(!Lst_IsEmpty(members)) {
		gn = (GNode *)Lst_DeQueue(members);

		if (DEBUG(SUFF)) {
		    printf("%s...", gn->name);
		}
		if (Lst_Member(pGn->children, (ClientData)gn) == NILLNODE) {
		    (void)Lst_Append(pGn->children, prevLN, (ClientData)gn);
		    prevLN = Lst_Succ(prevLN);
		    (void)Lst_AtEnd(gn->parents, (ClientData)pGn);
		    pGn->unmade++;
		}
	    }
	    Lst_Destroy(members, NOFREE);
	    /*
	     * Free the result
	     */
	    free((char *)cp);
	}
	/*
	 * Now the source is expanded, remove it from the list of children to
	 * keep it from being processed.
	 */
	ln = Lst_Member(pGn->children, (ClientData)cGn);
	pGn->unmade--;
	Lst_Remove(pGn->children, ln);
	if (DEBUG(SUFF)) {
	    printf("\n");
	}
    }

    return(0);
}

/*-
 *-----------------------------------------------------------------------
 * SuffApplyTransform --
 *	Apply a transformation rule, given the source and target nodes
 *	and suffixes.
 *
 * Results:
 *	TRUE if successful, FALSE if not.
 *
 * Side Effects:
 *	The source and target are linked and the commands from the
 *	transformation are added to the target node's commands list.
 *	All attributes but OP_DEPMASK and OP_TRANSFORM are applied
 *	to the target. The target also inherits all the sources for
 *	the transformation rule.
 *
 *-----------------------------------------------------------------------
 */
static Boolean
SuffApplyTransform(GNode *tGn, GNode *sGn, Suff *tSuff, Suff *sSuff)
{
    /* GNode   	*tGn;	    Target node */
    /* GNode   	*sGn;	    Source node */
    /* Suff    	*tSuff;     Target suffix */
    /* Suff    	*sSuff;     Source suffix */

    LstNode 	ln; 	    /* General node */
    char    	*tname;	    /* Name of transformation rule */
    GNode   	*gn;	    /* Node for same */

SUFFTRACE("SuffApplyTransform: Apply a transformation rule, given the source and target nodes");

#ifdef DEBUG_FLAG    
    if (DEBUG(SUFF)) {
      printf("#\tApplying source node \"%s\" and source suffix \"%s\" transformation\n#\tto target node \"%s\" and target suffix \"%s\".\n", sGn->name, sSuff->name, tGn->name, tSuff->name);
    }
#endif

    if (Lst_Member(tGn->children, (ClientData)sGn) == NILLNODE) {
	/*
	 * Not already linked, so form the proper links between the
	 * target and source.
	 */
	(void)Lst_AtEnd(tGn->children, (ClientData)sGn);
	(void)Lst_AtEnd(sGn->parents, (ClientData)tGn);
	tGn->unmade += 1;
    }

    if ((sGn->type & OP_OPMASK) == OP_DOUBLEDEP) {
	/*
	 * When a :: node is used as the implied source of a node, we have
	 * to link all its cohorts in as sources as well. Only the initial
	 * sGn gets the target in its iParents list, however, as that
	 * will be sufficient to get the .IMPSRC variable set for tGn
	 */
	for (ln=Lst_First(sGn->cohorts); ln != NILLNODE; ln=Lst_Succ(ln)) {
	    gn = (GNode *)Lst_Datum(ln);

	    if (Lst_Member(tGn->children, (ClientData)gn) == NILLNODE) {
		/*
		 * Not already linked, so form the proper links between the
		 * target and source.
		 */
		(void)Lst_AtEnd(tGn->children, (ClientData)gn);
		(void)Lst_AtEnd(gn->parents, (ClientData)tGn);
		tGn->unmade += 1;
	    }
	}
    }

    /*
     * Locate the transformation rule itself
     */
    tname = Str_Concat(sSuff->name, tSuff->name, 0);
    ln = Lst_Find(transforms, (ClientData)tname, (int(*)(void*,void*))SuffGNHasNameP);
    free(tname);

    if (ln == NILLNODE) {
	/*
	 * Not really such a transformation rule (can happen when we're
	 * called to link an OP_MEMBER and OP_ARCHV node), so return
	 * FALSE.
	 */
	return(FALSE);
    }

    gn = (GNode *)Lst_Datum(ln);

#ifdef DEBUG_FLAG    
    if (DEBUG(SUFF)) {
      printf("#\tApplying \"%s\" transformation \"%s\" to target \"%s\"\n", sSuff->name, tSuff->name, tGn->name);
    }
#endif

    /*
     * Record last child for expansion purposes
     */
    ln = Lst_Last(tGn->children);
    
    /*
     * Pass the buck to Make_HandleTransform to apply the rule
     */
    (void)Make_HandleTransform(gn, tGn);

    /*
     * Deal with wildcards and variables in any acquired sources
     */
    ln = Lst_Succ(ln);
    if (ln != NILLNODE) {
	Lst_ForEachFrom(tGn->children, ln,
			(int(*)(void*,void*))SuffExpandChildren, (ClientData)tGn);
    }

    /*
     * Keep track of another parent to which this beast is transformed so
     * the .IMPSRC variable can be set correctly for the parent.
     */
    (void)Lst_AtEnd(sGn->iParents, (ClientData)tGn);

    return(TRUE);
}


/*-
 *-----------------------------------------------------------------------
 * SuffFindArchiveDeps --
 *	Locate dependencies for an OP_ARCHV node.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Same as Suff_FindDeps
 *
 *-----------------------------------------------------------------------
 */
static void
SuffFindArchiveDeps(GNode *gn)
{
    /* GNode   	*gn;	    Node for which to locate dependencies */

    char    	*eoarch;    /* End of archive portion */
    char    	*eoname;    /* End of member portion */
    GNode   	*mem;	    /* Node for member */
    static char	*copy[] = { /* Variables to be copied from the member node */
	TARGET,	    	    /* Must be first */
	PREFIX,	    	    /* Must be second */
    };
    char  	*vals[sizeof(copy)/sizeof(copy[0])];
    int	    	i;  	    /* Index into copy and vals */
    Suff    	*ms;	    /* Suffix descriptor for member */
    char    	*name;	    /* Start of member's name */
    
    /*
     * The node is an archive(member) pair. so we must find a
     * suffix for both of them.
     */
    eoarch = index (gn->name, '(');
    eoname = index (eoarch, ')');

    *eoname = '\0';	  /* Nuke parentheses during suffix search */
    *eoarch = '\0';	  /* So a suffix can be found */

    name = eoarch + 1;
    
    /*
     * To simplify things, call Suff_FindDeps recursively on the member now,
     * so we can simply compare the member's .PREFIX and .TARGET variables
     * to locate its suffix. This allows us to figure out the suffix to
     * use for the archive without having to do a quadratic search over the
     * suffix list, backtracking for each one...
     */
    mem = Targ_FindNode(name, TARG_CREATE);
    Suff_FindDeps(mem);

    /*
     * Create the link between the two nodes right off
     */
    if (Lst_Member(gn->children, (ClientData)mem) == NILLNODE) {
	(void)Lst_AtEnd(gn->children, (ClientData)mem);
	(void)Lst_AtEnd(mem->parents, (ClientData)gn);
	gn->unmade += 1;
    }
    
    /*
     * Copy in the variables from the member node to this one.
     */
    for (i = (sizeof(copy)/sizeof(copy[0]))-1; i >= 0; i--) {
	vals[i] = Var_Value(copy[i], mem);
	Var_Set(copy[i], vals[i], gn);
    }

    ms = mem->suffix;
    if (ms == NULL) {
	/*
	 * Didn't know what it was -- use .NULL suffix if not in make mode
	 */
	if (DEBUG(SUFF)) {
	    printf(catgets(catd, MS_DEBUG, SUFF011, "using null suffix\n"));
	}
	ms = suffNull;
    }


    /*
     * Set the other two local variables required for this target.
     */
	if (DEBUG(SUFF)) {
	    printf(catgets(catd, MS_DEBUG, SUFF012, "Local variables for which this node is a member of archive: \n"));
	}
    Var_Set (MEMBER, name, gn);

	if (DEBUG(SUFF)) {
	    printf(catgets(catd, MS_DEBUG, SUFF013, "Local variables for which this node is a archive of archive: \n"));
	}
    Var_Set (ARCHIVE, gn->name, gn);

	if (DEBUG(SUFF)) {
	    printf(catgets(catd, MS_DEBUG, SUFF014, "Local variables for which this node is a target:\n"));
	}
    Var_Set (TARGET, gn->name, gn);

    if (ms != NULL) {
	/*
	 * Member has a known suffix, so look for a transformation rule from
	 * it to a possible suffix of the archive. Rather than searching
	 * through the entire list, we just look at suffixes to which the
	 * member's suffix may be transformed...
	 */
	LstNode	    ln;

	/*
	 * Use first matching suffix...
	 */
	ln = Lst_Find((Lst)ms->parents, (ClientData)eoarch, (int(*)(void*,void*))SuffSuffIsSuffixP);

	if (ln != NILLNODE) {
	    /*
	     * Got one -- apply it
	     */
	    if (!SuffApplyTransform(gn, mem, (Suff *)Lst_Datum(ln), ms) &&
		DEBUG(SUFF))
	    {
		printf(catgets(catd, MS_DEBUG, SUFF015, "\tNo transformation from %s -> %s\n"),
		       ms->name, ((Suff *)Lst_Datum(ln))->name);
	    }
	}
    }

    /*
     * Replace the opening and closing parens now we've no need of the separate
     * pieces.
     */
    *eoarch = '('; *eoname = ')';

    /*
     * Pretend gn appeared to the left of a dependency operator so
     * the user needn't provide a transformation from the member to the
     * archive.
     */
    if (OP_NOP(gn->type)) {
	gn->type |= OP_DEPENDS;
    }

    /*
     * Flag the member as such so we remember to look in the archive for
     * its modification time.
     */
    mem->type |= OP_MEMBER;
    return;
}

/*-
 *-----------------------------------------------------------------------
 * SuffFindNormalDeps --
 *	Locate implicit dependencies for regular targets.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Same as Suff_FindDeps...
 *
 *-----------------------------------------------------------------------
 */
static void
SuffFindNormalDeps(GNode *gn)
{
    /* GNode   	*gn;	    Node for which to find sources */

    char    	*eoname;    /* End of name */
    char    	*sopref;    /* Start of prefix */
    LstNode 	ln; 	    /* Next suffix node to check */
    Lst	    	srcsLst;    /* List of sources at which to look */
    Lst	    	targsLst;   /* List of targets to which things can be
			     * transformed. They all have the same file,
			     * but different suff and pref fields */
    ISrc	*bottom;    /* Start of found transformation path */
    ISrc 	*srcISrc;   /* General ISrc pointer */
    char    	*pref;	    /* Prefix to use */
    ISrc	*targISrc;  /* General ISrc target pointer */

    SUFFTRACE("SuffFindNormalDeps: Locate implicit dependencies for regular targets");

    eoname = gn->name + strlen(gn->name);  /* End of name */

    sopref = gn->name;            /* Start of prefix */
    
    /*
     * Begin at the beginning...
     */
    ln = Lst_First(sufflist);	/* static suffix list */
    srcsLst = Lst_Init(FALSE);	/* local source list */
    targsLst = Lst_Init(FALSE);	/* local target list */

    /*
     * We're caught in a catch-22 here. On the one hand, we want to use any
     * transformation implied by the target's sources, but we can't examine
     * the sources until we've expanded any variables/wildcards they may hold,
     * and we can't do that until we've set up the target's local variables
     * and we can't do that until we know what the proper suffix for the
     * target is (in case there are two suffixes one of which is a suffix of
     * the other) and we can't know that until we've found its implied
     * source, which we may not want to use if there's an existing source
     * that implies a different transformation.
     *
     * In an attempt to get around this, which may not work all the time,
     * but should work most of the time, we look for implied sources first,
     * checking transformations to all possible suffixes of the target,
     * use what we find to set the target's local variables, expand the
     * children, then look for any overriding transformations they imply.
     * Should we find one, we discard the one we found before.
     */
/**********************************************/
/* PART 1 look for all implied sources first! */
/**********************************************/

    SUFFTRACE("SuffFindNormalDeps: Part 1 ");
 
   while(ln != NILLNODE) {
	/*
	 * Look for next possible suffix...
	 */

      /* find a node on the suffix list from a given starting point ln */
      /* using the end of the suffix name */

	ln = Lst_FindFrom(sufflist, ln, (ClientData)eoname, (int(*)(void*,void*))SuffSuffIsSuffixP); 
       
	if (ln != NILLNODE) {
	    int	    prefLen;	    /* Length of the prefix */
	    ISrc	    *targISrc;

	    
	    /*
	     * Allocate a ISrc structure to which things can be transformed
	     */
	    targISrc = (ISrc *)emalloc(sizeof(ISrc));
	    targISrc->file = strdup(gn->name);
	    targISrc->sccsgetname =  strdup("");
	    targISrc->suff = (Suff *)Lst_Datum(ln);
	    targISrc->node = gn;
	    targISrc->parent = (ISrc *)NULL;
	    
	    /*
	     * Allocate room for the prefix, whose end is found by subtracting
	     * the length of the suffix from the end of the name.
	     */
	    prefLen = (eoname - targISrc->suff->nameLen) - sopref;
	    targISrc->pref = (char *)emalloc(prefLen + 1);
	    bcopy(sopref, targISrc->pref, prefLen);
	    targISrc->pref[prefLen] = '\0';

	    /*
	     * Add nodes from which the targISrcet could be made
	     */

	    SUFFTRACE("SuffFindNormalDeps: Part Add nodes from which the targISrcet could be made");

	    SuffCouldBeMadeFrom(srcsLst, targISrc);

	    /*
	     * Record the target so we can nuke it
	     */
	    (void)Lst_AtEnd(targsLst, (ClientData)targISrc);

	    /*
	     * Search from this suffix's successor...
	     */
	    ln = Lst_Succ(ln);
	}
      }				/* while(ln != NILLNODE)  */

    if (ln == NILLNODE) {
      if (DEBUG(SUFF)) {
	    printf("\n# \"%s\" Did not have a known suffix.\n",gn->name);
	  }
    }
    /*
     * Handle target of unknown suffix...
     */

    SUFFTRACE("SuffFindNormalDeps: Part Handle target of unknown suffix...");

  if (Lst_IsEmpty(targsLst) && suffNull != NULL) {
	if (DEBUG(SUFF)) {
	    printf(catgets(catd, MS_DEBUG, SUFF016, "# No known suffix on %s. Using .NULL suffix\n"), gn->name);
	}
	
	targISrc = (ISrc *)emalloc(sizeof(ISrc));
	targISrc->file = strdup(gn->name);
	targISrc->suff = suffNull;
	targISrc->node = gn;
	targISrc->parent = (ISrc *)NULL;
	targISrc->pref = strdup(sopref);
	targISrc->sccsgetname =  strdup("");

	SuffCouldBeMadeFrom(srcsLst, targISrc);
	(void)Lst_AtEnd(targsLst, (ClientData)targISrc);
    }
    
    /*
     * Using the list of possible sources built up from the target suffix(es),
     * try and find an existing file/target that matches.
     */

    SUFFTRACE("SuffFindNormalDeps: Part Try and find an existing file/target that matches");

    bottom = SuffFindThem(srcsLst);

    SUFFTRACE("SuffFindNormalDeps: Part Identify the transformation");

   if (bottom == (ISrc *)NULL) {
	/*
	 * No known transformations -- use the first suffix found for setting
	 * the local variables.
	 */
	if (!Lst_IsEmpty(targsLst)) {
	    targISrc = (ISrc *)Lst_Datum(Lst_First(targsLst));
	} else {
	    targISrc = (ISrc *)NULL;
	}
    } else {
	/*
	 * Work up the transformation path to find the suffix of the
	 * target to which the transformation was made.
	 */
	for (targISrc = bottom; targISrc->parent != NULL; targISrc = targISrc->parent) {
	    ;
	}
    }

    SUFFTRACE("SuffFindNormalDeps: Part Expand variables");

    /*
     * The TARGET local variable we always set to be the name at this point,
     * since it's only set to the path if the thing is only a source and
     * if it's only a source, it doesn't matter what we put here as far
     * as expanding sources is concerned, since it has none...
     */

	if (DEBUG(SUFF)) {
	    printf(catgets(catd, MS_DEBUG, SUFF017, "Local variable for which this node is a target:\n"));
	}

    SUFFTRACE("SuffFindNormalDeps: Part ");

    Var_Set(TARGET, gn->name, gn);

    pref = (targISrc != NULL) ? targISrc->pref : gn->name;

	if (DEBUG(SUFF)) {
	    printf(catgets(catd, MS_DEBUG, SUFF018, "Local variables for which this node is a prefix:\n"));
	}

    Var_Set(PREFIX, pref, gn);

    /*
     * Now we've got the important local variables set, expand any sources
     * that still contain variables or wildcards in their names.
     */

    SUFFTRACE("SuffFindNormalDeps: Part Continuing Expanding....");

    Lst_ForEach(gn->children, (int(*)(void*,void*))SuffExpandChildren, (ClientData)gn);
    

    if (targISrc == NULL) {

	if (DEBUG(SUFF)) {
	    printf(catgets(catd, MS_DEBUG, SUFF019, "\tNo valid suffix on %s\n"), gn->name);
	}

sfnd_abort:
	/*
	 * Deal with finding the thing on the default search path if the
	 * node is only a source (not on the lhs of a dependency operator
	 * or [XXX] it has neither children or commands).
	 */

	SUFFTRACE("SuffFindNormalDeps: Part Deal with finding the thing on the default search path");


	if (OP_NOP(gn->type) ||
	    (Lst_IsEmpty(gn->children) && Lst_IsEmpty(gn->commands)))
	{
	    gn->path = Dir_FindFile(gn->name,dirSearchPath);

	    {
	      if (TrySccsGet) 
		{
		  char *p;
		  char *tmp;
		  gn->sccsgetname = p = (char *)emalloc (3 + strlen(gn->name));
		  *p++ = 's';
		  *p++ = '.';
		  strncpy(p, gn->name, strlen(gn->name));
                  tmp = Dir_FindFile(gn->sccsgetname,dirSccsPath);
		  if (tmp != NULL)
		  {
		    gn->SccsGetFileExists = TRUE;
	            if (gn->path == NULL)
		      gn->path = gn->name;
                    gn->sccsgetpath = Str_Concat ("SCCS", gn->sccsgetname, STR_ADDSLASH);
		  }
		}
	    }

	    if (gn->path != NULL) {
		Var_Set(TARGET, gn->path, gn);

		if (targISrc != NULL) {
		  /*
		   * Suffix known for the thing -- trim the suffix off
		   * the path to form the proper .PREFIX variable.
		   */
		    int		len = strlen(gn->path);
		    char	savec;

		    gn->suffix = targISrc->suff;

		    savec = gn->path[len-targISrc->suff->nameLen];
		    gn->path[len-targISrc->suff->nameLen] = '\0';

		    Var_Set(PREFIX, gn->path, gn);

		    gn->path[len-targISrc->suff->nameLen] = savec;
		} else {
		    /*
		     * The .PREFIX gets the full path if the target has
		     * no known suffix.
		     */
		    gn->suffix = NULL;

		    Var_Set(PREFIX, gn->path, gn);
		}
	      }
	  } else {
	    /*
	     * Not appropriate to search for the thing -- set the
	     * path to be the name so Dir_MTime won't go grovelling for
	     * it.
	     */

	  SUFFTRACE("SuffFindNormalDeps: Part Not appropriate to search for the thing");

	    gn->suffix = (targISrc == NULL) ? NULL : targISrc->suff;
	    gn->path = gn->name;
	}
	
	goto sfnd_return;
      }				/* if (targISrc == NULL) */


    /*
     * Check for overriding transformation rule implied by sources
     */

    SUFFTRACE("SuffFindNormalDeps: Part Check for overriding transformation rule implied by sources");

    if (!Lst_IsEmpty(gn->children)) {
	srcISrc = SuffFindCmds(targISrc);

	if (srcISrc != (ISrc *)NULL) {
	    /*
	     * Free up all the ISrc structures in the transformation path
	     * up to, but not including, the parent node.
	     */
	    while (bottom && bottom->parent != NULL) {
		ISrc *p = bottom->parent;

		SuffFreeISrc(bottom);
		bottom = p;
	    }
	    bottom = srcISrc;
	}
    }

    SUFFTRACE("SuffFindNormalDeps: Part If no idea from where it can come -- return now");

    if (bottom == NULL) {
	/*
	 * No idea from where it can come -- return now.
	 */
	goto sfnd_abort;
    }



    /*
     * We now have a list of ISrc structures headed by 'bottom' and linked via
     * their 'parent' pointers. What we do next is create links between
     * source and target nodes (which may or may not have been created)
     * and set the necessary local variables in each target. The
     * commands for each target are set from the commands of the
     * transformation rule used to get from the srcISrc suffix to the targ
     * suffix. Note that this causes the commands list of the original
     * node, gn, to be replaced by the commands of the final
     * transformation rule. Also, the unmade field of gn is incremented.
     * Etc. 
     */
    if (bottom->node == NILGNODE) {
	bottom->node = Targ_FindNode(bottom->file, TARG_CREATE);
    }
    
    SUFFTRACE("SuffFindNormalDeps: Part create links between source and target nodes");

    for (srcISrc = bottom; srcISrc->parent != (ISrc *)NULL; srcISrc = srcISrc->parent) {
	targISrc = srcISrc->parent;

	srcISrc->node->suffix = srcISrc->suff;

	if (targISrc->node == NILGNODE) {
	    targISrc->node = Targ_FindNode(targISrc->file, TARG_CREATE);
	}

    SUFFTRACE("SuffFindNormalDeps: Part Apply the transformation");

	SuffApplyTransform(targISrc->node, srcISrc->node,
			   targISrc->suff, srcISrc->suff);

	if (targISrc->node != gn) {
	    /*
	     * Finish off the dependency-search process for any nodes
	     * between bottom and gn (no point in questing around the
	     * filesystem for their implicit source when it's already
	     * known). Note that the node can't have any sources that
	     * need expanding, since SuffFindThem will stop on an existing
	     * node, so all we need to do is set the standard and System V
	     * variables.
	     */
	    targISrc->node->type |= OP_DEPS_FOUND;

	    Var_Set(PREFIX, targISrc->pref, targISrc->node);
	
	    Var_Set(TARGET, targISrc->node->name, targISrc->node);
	}
      }

    gn->suffix = srcISrc->suff;

    /*
     * So Dir_MTime doesn't go questing for it...
     */
    gn->path = gn->name;

    /*
     * Nuke the transformation path and the ISrc structures left over in the
     * two lists.
     */
    SuffFreeISrc(bottom);

sfnd_return:
    Lst_Destroy(srcsLst, (void (*)(void*))SuffFreeISrc);
    Lst_Destroy(targsLst, (void (*)(void*))SuffFreeISrc);
    return;

}
	
    


/*-
 *-----------------------------------------------------------------------
 * Suff_FindDeps  --
 *	Find implicit sources for the target described by the graph node
 *	gn
 * This includes sccs files
 * Results:
 *	Nothing.
 *
 * Side Effects:
 *	Nodes are added to the graph below the passed-in node. The nodes
 *	are marked to have their IMPSRC variable filled in. The
 *	PREFIX variable is set for the given node and all its
 *	implied children.
 *
 * Notes:
 *	The path found by this target is the shortest path in the
 *	transformation graph, which may pass through non-existent targets,
 *	to an existing target. The search continues on all paths from the
 *	root suffix until a file is found. I.e. if there's a path
 *	.o -> .c -> .l -> .l,v from the root and the .l,v file exists but
 *	the .c and .l files don't, the search will branch out in
 *	all directions from .o and again from all the nodes on the
 *	next level until the .l,v node is encountered.
 *
 *-----------------------------------------------------------------------
 */
void
Suff_FindDeps (GNode *gn)
{
    /* GNode         *gn;    node we're dealing with */

    SUFFTRACE("Suff_FindDeps: Find implicit sources for the target described by the graph node");

    if (gn->type & OP_DEPS_FOUND) {
	/*
	 * If dependencies already found, no need to do it again...
	 */
	return;
    } else {
	gn->type |= OP_DEPS_FOUND;
    }
    
    if (DEBUG(SUFF)) {
	printf ("# Find all dependencies for \"%s\".\n", gn->name);
    }
    
    if (gn->type & OP_ARCHV) {
	SuffFindArchiveDeps(gn);
    } else {
	SuffFindNormalDeps(gn);
    }
    return;
}

/*-
 *-----------------------------------------------------------------------
 * Suff_Init --
 *	Initialize suffixes module
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Many
 *-----------------------------------------------------------------------
 */
void
Suff_Init (void)
{

  SUFFTRACE("Suff_Init: Initialize suffixes module");

    sufflist = Lst_Init (FALSE);
    transforms = Lst_Init (FALSE);

    sNum = 0;
    /*
     * Create null suffix for single-suffix rules (POSIX). The thing doesn't
     * actually go on the suffix list or everyone will think that's its
     * suffix.
     */
    emptySuff = suffNull = (Suff *) emalloc (sizeof (Suff));

    suffNull->name =   	    strdup ("");
    suffNull->nameLen =     0;
    suffNull->children =    Lst_Init (FALSE);
    suffNull->parents =	    Lst_Init (FALSE);
    suffNull->sNum =   	    sNum++;
    suffNull->flags =  	    SUFF_NULL;
    return;

}

/********************* DEBUGGING FUNCTIONS **********************/

int 
SuffPrintName(Suff *s, ...) 
{
	printf ("%s ", s->name); return (0);
}

static int
SuffPrintSuff (Suff *s, ...)
{
    /* Suff    *s; */

    int	    flags;
    int	    flag;

    printf ("# `%s'", s->name);
    
    flags = s->flags;
    if (flags) {
	fputs (" (", stdout);
	while (flags) {
	    flag = 1 << (ffs(flags) - 1);
	    flags &= ~flag;
	    switch (flag) {
		case SUFF_NULL:
		    printf ("NULL");
		    break;
		case SUFF_SCCS:
		    printf ("SCCS");
		    break;
	    }
	    putc(flags ? '|' : ')', stdout);
	}
    }
    putc ('\n', stdout);
    printf (catgets(catd, MS_DEBUG, SUFF020, "#\tTo: "));
    Lst_ForEach (s->parents, (int(*)(void*,void*))SuffPrintName, (ClientData)0);
    putc ('\n', stdout);
    printf (catgets(catd, MS_DEBUG, SUFF021, "#\tFrom: "));
    Lst_ForEach (s->children, (int(*)(void*,void*))SuffPrintName, (ClientData)0);
    putc ('\n', stdout);
    return (0);
}

static int
SuffPrintTrans (GNode *t, ...)
{
    /* GNode   *t; */

    extern int Targ_PrintCmd (char *, ...);

    printf ("# %-16s:\n", t->name);
    Lst_ForEach (t->commands, (int(*)(void*,void*))Targ_PrintCmd, (ClientData)0);
    putc ('\n', stdout);
    return(0);
}

static int
SuffPrintSpecial (GNode *gn)
{

    extern int Targ_PrintCmd (char *, ...);

    if (gn  == NILGNODE)
      return(0);
    printf ("SPECIAL TARGET: %s\n", gn->name);
    Lst_ForEach (gn->commands, (int(*)(void*,void*))Targ_PrintCmd, (ClientData)0);
    putc ('\n', stdout);
    return(0);
}

void
Suff_PrintAll(void)
{
    Lst_ForEach (sufflist, (int(*)(void*,void*))SuffPrintSuff, (ClientData)0);
    if (Lst_IsEmpty(sufflist)) {
      printf (catgets(catd, MS_DEBUG, SUFF022, "\n#* No known Suffixes.\n\n"));
    }
    else
	printf("\n\n");    

    printf ("\n#***\n");
    printf (catgets(catd, MS_DEBUG, SUFF023, "#*** Printing all known Suffix Transformation Rules:\n"));
    printf ("#***\n\n");
    Lst_ForEach (transforms, (int(*)(void*,void*))SuffPrintTrans, (ClientData)0);
    if (Lst_IsEmpty(transforms)) {
      printf (catgets(catd, MS_DEBUG, SUFF024, "\n#* No known Suffix Transformation Rules.\n\n"));
    }
    else
	printf("\n\n");

    if (DEFAULT  != NILGNODE) {
      printf ("\n#***\n");
      printf (catgets(catd, MS_DEBUG, SUFF023, "#*** Printing special target .DEFAULT Rule:\n"));
      printf ("#***\n\n");
      SuffPrintSpecial (DEFAULT);
    } else
      printf("\n\n");
    if (SCCS_GET  != NILGNODE) {
      printf ("\n#***\n");
      printf (catgets(catd, MS_DEBUG, SUFF023, "#*** Printing special target .SCCS_GET Rule:\n"));
      printf ("#***\n\n");
      SuffPrintSpecial (SCCS_GET);
    } else
      printf("\n\n");

    return;
  }
