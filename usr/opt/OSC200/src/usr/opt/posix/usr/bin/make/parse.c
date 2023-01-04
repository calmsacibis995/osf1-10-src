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
static char *rcsid = "@(#)$RCSfile: parse.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/29 22:05:31 $";
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
 * parse.c --
 *	Functions to parse a makefile.
 *
 *	One function, Parse_Init, must be called before any functions
 *	in this module are used. After that, the function Parse_File is the
 *	main entry point and controls most of the other functions in this
 *	module.
 *
 *	Most important structures are kept in Lsts. Directories for
 *	the #include "..." function are kept in the 'parseIncPath' Lst, while
 *	those for the #include <...> are kept in the 'sysIncPath' Lst. The
 *	targets currently being defined are kept in the 'targets' Lst.
 *
 *	The variables 'fname' and 'lineno' are used to track the name
 *	of the current file and the line number in that file so that error
 *	messages can be more meaningful.
 *
 * Interface:
 *	Parse_Init	    	    Initialization function which must be
 *	    	  	    	    called before anything else in this module
 *	    	  	    	    is used.
 *
 *	Parse_File	    	    Function used to parse a makefile. It must
 *	    	  	    	    be given the name of the file, which should
 *	    	  	    	    already have been opened, and a function
 *	    	  	    	    to call to read a character from the file.
 *
 *	Parse_IsVar	    	    Returns TRUE if the given line is a
 *	    	  	    	    variable assignment. Used by MainParseArgs
 *	    	  	    	    to determine if an argument is a target
 *	    	  	    	    or a variable assignment. Used internally
 *	    	  	    	    for pretty much the same thing...
 *
 *	Parse_Error	    	    Function called when an error occurs in
 *	    	  	    	    parsing. Used by the variable and
 *	    	  	    	    conditional modules.
 *	Parse_MainName	    	    Returns a Lst of the main target to create.
 */

#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "make.h"
#include "buf.h"
#include "pmake_msg.h"

extern nl_catd	catd;

/*
 * These values are returned by ParseEOF to tell Parse_File whether to
 * KEEP_PARSING parsing, i.e. it had only reached the end of an include file,
 * or if it's DONE.
 */
#define	KEEP_PARSING	1
#define	DONE		0
static int 	    ParseEOF(int);

static Lst     	    targets;	/* A single dependency lines list of targets we're working on. 
                                 * Reinitialized after each dependency specification. */

static Boolean	    inDependSpec;	/* true if currently in a dependency specification. 
				         * ie (line or its commands) */

static char    	    *fname;	/* name of current file (for errors) */
static int          lineno;	/* line number in current file */
static FILE   	    *curFILE; 	/* current makefile */

static int	    fatals = 0;

static GNode	    *mainNode;	/* The main target to create. This is the
				 * first target on the first dependency
				 * line in the first makefile */
/*
 * Definitions for handling #include specifications
 */
typedef struct IFile {
    char           *fname;	    /* name of previous file */
    int             lineno;	    /* saved line number */
    FILE *       F;		    /* the open stream */
}              	  IFile;

static Lst      includes;  	/* stack of IFiles generated by
				 * #includes */
Lst dirSccsPath;                /* sccs search path */

/*-
 * specType contains the SPECial TYPE of the current target. It is
 * Not if the target is unspecial. If it *is* special, however, the children
 * are linked as children of the parent but not vice versa. This variable is
 * set in ParseDoDepenencyLine
 */
typedef enum {
    Default,	    /* .DEFAULT */
    Ignore,	    /* .IGNORE */
    Not,	    /* Not special */
    Posix,	    /* .POSIX */
    Precious,	    /* .PRECIOUS */
    Sccsget,	    /* .SCCS_GET */
    Silent,	    /* .SILENT */
    Suffixes	    /* .SUFFIXES */
} ParseSpecial;

static ParseSpecial specType;

/*
 * The parseKeywords table is searched using binary search when deciding
 * if a target or source is special. The 'spec' field is the ParseSpecial
 * type of the keyword ("Not" if the keyword isn't special as a target) while
 * the 'op' field is the operator to apply to the list of targets if the
 * keyword is used as a source ("0" if the keyword isn't special as a source)
 */
static struct {
    char    	  *name;    	/* Name of keyword */
    ParseSpecial  spec;	    	/* Type when used as a target */
    int	    	  op;	    	/* Operator when used as a source */
} parseKeywords[] = {
{ ".DEFAULT",	  Default,  	0 },
{ ".IGNORE",	  Ignore,   	OP_IGNORE },
{ ".POSIX",	  Posix, 	0 },
{ ".PRECIOUS",	  Precious, 	OP_PRECIOUS },
{ ".SCCS_GET",	  Sccsget, 	0 },
{ ".SILENT",	  Silent,   	OP_SILENT },
{ ".SUFFIXES",	  Suffixes, 	0 },
};

static Boolean maybePosix = TRUE;
static Boolean isPosix = FALSE;


static int
ParsePrintTrans (GNode *t, ...)
{
    /* GNode   *t; */

    extern int Targ_PrintCmd (char *, ...);
    printf ("# %-16s:\n", t->name);
    Lst_ForEach (t->commands, (int (*)(void*,void*))Targ_PrintCmd, (ClientData)0);
    if (Lst_IsEmpty(t->commands)) {
      printf ("#* The targets->commands list is empty .\n\n");
    }
    else
	printf("\n\n");
    return;
}


static void
Parse_Print_All(void)
{

    printf ("\n#***\n");
    printf ("#*** Printing all Targets Transformation Rules:\n");
    printf ("#***\n\n");

    Lst_ForEach (targets, (int (*)(void*,void*))ParsePrintTrans, (ClientData)0);
    if (Lst_IsEmpty(targets)) {
      printf ("#* Lst is empty targets.\n\n");
    }
    else
	printf("\n\n");

    return;
}


/*-
 *----------------------------------------------------------------------
 * ParseFindKeyword --
 *	Look in the table of keywords for one matching the given string.
 *
 * Results:
 *	The index of the keyword, or -1 if it isn't there.
 *
 * Side Effects:
 *	None
 *----------------------------------------------------------------------
 */
static int
ParseFindKeyword (char *str)
{
    /* char	    *str;		String to find */

    int    start=0;
    int    end=0;
    int    cur=0;
    int    diff=0;
    
    end = (sizeof(parseKeywords)/sizeof(parseKeywords[0])) - 1;

    do {
	cur = start + ((end - start) / 2);
	diff = strcmp (str, parseKeywords[cur].name);

	if (diff == 0) {
	    return (cur);
	} else if (diff < 0) {
	    end = cur - 1;
	} else {
	    start = cur + 1;
	}
    } while (start <= end);
    return (-1);
}

/*-
 * Parse_Error  --
 *	Error message abort function for parsing. Prints out the context
 *	of the error (line number and file) as well as the message with
 *	two optional arguments.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	"fatals" is incremented if the level is PARSE_FATAL.
 */
/* VARARGS */
void
Parse_Error(int type, const char *fmt, ...)
{
	va_list ap;

	(void)fprintf(stderr, catgets(catd, MS_MAKE, LINENUM, 
		      "\"%s\", line %d: "), fname, lineno);
	if (type == PARSE_WARNING)
		(void)fprintf(stderr, catgets(catd, MS_MAKE, WARNING, "warning: "));
	va_start(ap, fmt);
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fprintf(stderr, "\n");
	(void)fflush(stderr);
	if (type == PARSE_FATAL)
		fatals += 1;
}

/*-
 *---------------------------------------------------------------------
 * ParseLinkSrc  --
 *	Link the parent node to its new child. Used in a Lst_ForEach by
 *	ParseDoDepenencyLine. If the specType isn't 'Not', the parent
 *	isn't linked as a parent of the child.
 *
 * Results:
 *	Always = 0
 *
 * Side Effects:
 *	New elements are added to the parents list of cgn and the
 *	children list of cgn. the unmade field of pgn is updated
 *	to reflect the additional child.
 *---------------------------------------------------------------------
 */
static int
ParseLinkSrc (GNode *pgn, GNode *cgn)
{
    /* GNode          *pgn;	The parent node */
    /* GNode          *cgn;	The child node */

    if (Lst_Member (pgn->children, (ClientData)cgn) == NILLNODE) {
	(void)Lst_AtEnd (pgn->children, (ClientData)cgn);
	if (specType == Not) {
	    (void)Lst_AtEnd (cgn->parents, (ClientData)pgn);
	}
	pgn->unmade += 1;
    }
    return (0);
}

/*-
 *---------------------------------------------------------------------
 * ParseDoOp  --
 *	Apply the parsed operator to the given target node. Used in a
 *	Lst_ForEach call by ParseDoDepenencyLine once all targets have
 *	been found and their operator parsed. If the previous and new
 *	operators are incompatible, a major error is taken.
 *
 * Results:
 *	Always 0
 *
 * Side Effects:
 *	The type field of the node is altered to reflect any new bits in
 *	the op.
 *---------------------------------------------------------------------
 */
static int
ParseDoOp (GNode *gn, int op, ...)
{
    /* GNode          *gn;	  The node to which the operator is to be applied */
    /* int             op;	  The operator to apply */

    /*
     * If the dependency mask of the operator and the node don't match and
     * the node has actually had an operator applied to it before, and
     * the operator actually has some dependency information in it, complain. 
     */
    if (((op & OP_OPMASK) != (gn->type & OP_OPMASK)) &&
	!OP_NOP(gn->type) && !OP_NOP(op))
    {
	Parse_Error (PARSE_FATAL, catgets(catd, MS_MAKE, IOPERATOR,
		     "Inconsistent operator for %s"), gn->name);
	return (1);
    }

    if ((op == OP_DOUBLEDEP) && ((gn->type & OP_OPMASK) == OP_DOUBLEDEP)) {
	/*
	 * If the node was the object of a :: operator, we need to create a
	 * new instance of it for the children and commands on this dependency
	 * line. The new instance is placed on the 'cohorts' list of the
	 * initial one (note the initial one is not on its own cohorts list)
	 * and the new instance is linked to all parents of the initial
	 * instance.
	 */
	 GNode	*cohort;
	LstNode	    	ln;
			
	cohort = Targ_NewGN(gn->name);
	/*
	 * Duplicate links to parents so graph traversal is simple. Perhaps
	 * some type bits should be duplicated?
	 *
	 * Make the cohort invisible as well to avoid duplicating it into
	 * other variables. True, parents of this target won't tend to do
	 * anything with their local variables, but better safe than
	 * sorry.
	 */
	Lst_ForEach(gn->parents, (int (*)(void*,void*))ParseLinkSrc, (ClientData)cohort);
	cohort->type = OP_DOUBLEDEP|OP_INVISIBLE;
	(void)Lst_AtEnd(gn->cohorts, (ClientData)cohort);

	/*
	 * Replace the node in the targets list with the new copy
	 */
	ln = Lst_Member(targets, (ClientData)gn);
	Lst_Replace(ln, (ClientData)cohort);
	gn = cohort;
    }
    /*
     * We don't want to nuke any previous flags (whatever they were) so we
     * just OR the new operator into the old 
     */
    gn->type |= op;

    return (0);
}

/*-
 *---------------------------------------------------------------------
 * ParseDoSrc  --
 *	Given the name of a source, figure out if it is an attribute
 *	and apply it to the targets if it is. Else decide if there is
 *	some attribute which should be applied *to* the source because
 *	of some special target and apply it if so. Otherwise, make the
 *	source be a child of the targets in the list 'targets'
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Operator bits may be added to the list of targets or to the source.
 *	The targets may have a new source added to their lists of children.
 *---------------------------------------------------------------------
 */
static void
ParseDoSrc (int tOp, char *src)
{
    /* int	tOp;	operator (if any) from special targets */
    /* char	*src;	name of the source to handle */

    GNode	*gn;

    /*
     * We need to find/create a node for the source.  After
     * that we can apply any operator to it
     * from a special target or link it to its parents, as
     * appropriate.
     *
     * In the case of a source that was the object of a :: operator,
     * the attribute is applied to all of its instances (as kept in
     * the 'cohorts' list of the node) or all the cohorts are linked
     * to all the targets.
     */
    gn = Targ_FindNode (src, TARG_CREATE);
    if (tOp) {
	gn->type |= tOp;
    } else {
	Lst_ForEach (targets, (int (*)(void*,void*))ParseLinkSrc, (ClientData)gn);
    }
    if ((gn->type & OP_OPMASK) == OP_DOUBLEDEP) {
	 GNode  	*cohort;
	 LstNode	ln;

	for (ln=Lst_First(gn->cohorts); ln != NILLNODE; ln = Lst_Succ(ln)){
	    cohort = (GNode *)Lst_Datum(ln);
	    if (tOp) {
		cohort->type |= tOp;
	    } else {
		Lst_ForEach(targets, (int (*)(void*,void*))ParseLinkSrc, (ClientData)cohort);
	    }
	}
    }
}

/*-
 *-----------------------------------------------------------------------
 * ParseFindMain --
 *	Find a real target in the list and set it to be the main one.
 *	Called by ParseDoDepenencyLine when a main target hasn't been found
 *	yet.
 *
 * Results:
 *	0 if main not found yet, 1 if it is.
 *
 * Side Effects:
 *	mainNode is changed and Targ_SetMain is called.
 *
 *-----------------------------------------------------------------------
 */
static int
ParseFindMain(GNode *gn,...)
{
    /* GNode   	  *gn;	    Node to examine */

    if ((gn->type & (OP_NOTMAIN|OP_TRANSFORM)) == 0) {
	mainNode = gn;
	Targ_SetMain(gn);
	return (1);
    } else {
	return (0);
    }
}

/*-
 *---------------------------------------------------------------------
 * ParseDoDepenencyLine  --
 *	Parse the dependency line in line.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	The nodes of the sources are linked as children to the nodes of the
 *	targets. Some nodes may be created.
 *
 * PART 1 
 * We parse a dependency line by first extracting words from the line.
 * We start by looking on the left of the operator. We are looking for targets. 
 * This is done until a character is encountered which is an operator character. 
 * Currently these are only ':' and '::'.
 *
 * Part 2 
 * After the word is found, check to see if the word is a special target.
 * 
 * Part 2.1
 * Determine what the special target is, and perform actions necessay.
 *
 * When special targets are found, they are converted into graph-nodes "gn's".
 * The special target gn's are placed on a static list of targets.
 * 
 * Part 2.2 
 * The word was not a special target. 
 * When non-special targets are found, they are converted into graph-nodes "gn's"
 * if it does not all ready exist.
 * The special target gn's are placed on a static list of targets.
 * However special processing is necessay to determine if the target is a suffix
 * rule. If it is it must also be added to the suffix graph. 
 * 
 * Part 3
 * If the target was a special target, it's the only word (target) that 
 * should be left of the operator.
 *  
 * Part 4  
 * target parsing is done. Get rid of the local target list.
 * 
 * Part 5
 *
 * At this point the dependency line hase been parsed up till the operator.
 * All targets are parsed. Now parse the operator.
 * The parsed operator is applied to each node in the static 'targets' list,
 * which is where the nodes found for the targets are kept, by means of
 * the ParseDoOp function.
 *
 * Part 6
 * The pointer is now advanced until the first source is encountered.
 *
 * PART 7
 *
 * Several special targets take different actions if present with no
 * sources.
 *
 * PART 8	
 * The sources are read in much the same way as the targets were except
 * that now they are expanded using the wildcarding scheme of the C-Shell
 * and all instances of the resulting words in the list of all targets
 * are found. Each of the resulting nodes is then linked to each of the
 * targets as one of its children.
 *	Certain targets are handled specially. These are the ones detailed
 * by the specType variable.
 *	The storing of transformation rules is also taken care of here.
 * A target is recognized as a transformation rule by calling
 * Suff_IsTransform. If it is a transformation rule, its node is gotten
 * from the suffix module via Suff_AddTransform rather than the standard
 * Targ_FindNode in the target module.
 *---------------------------------------------------------------------
 */
static void
ParseDoDepenencyLine (char *line)
{
    /* char           *line;	The line to parse */

    char  *cp;		/* our current position */
     GNode *gn;		/* a general purpose temporary node */
     int    op;		/* the operator on the line */
    char            savec;	/* a place to save a character */
    int	    	    tOp;    	/* operator from special target */
    Lst	    	    sources;	/* list of source names after expansion */
    Lst 	    curTargs;	/* list of target names to be found and added
				 * to the targets list */

    tOp = 0;

    specType = Not;

    curTargs = Lst_Init(FALSE);
    
/* PART 1 */

    do {			/* main parse do while loop */
	for (cp = line;		/* locate archive if it exists */
	     *cp && !isspace(*cp) && (*cp != ':') && (*cp != '(');
	     cp++)
	  {
	    continue;
	  }
	if (*cp == '(') {	/* found and archive */
	    /*
	     * Archives must be handled specially to make sure the OP_ARCHV
	     * flag is set in their 'type' field, for one thing, and because
	     * things like "archive(file1.o file2.o file3.o)" are permissible.
	     * Arch_ParseArchive will set 'line' to be the first non-blank
	     * after the archive-spec. It creates/finds nodes for the members
	     * and places them on the given list, returning SUCCESS if all
	     * went well and FAILURE if there was an error in the
	     * specification. On error, line should remain untouched.
	     */
	    if (Arch_ParseArchive (&line, targets, VAR_CMD) != SUCCESS) {
		Parse_Error (PARSE_FATAL,
			     catgets(catd, MS_MAKE, ARERR,
			     "Error in archive specification: \"%s\""), line);
		return;
	    } else {
		cp = line;
		continue;
	      }
	}			/* end  "if (*cp == '(')" */

	savec = *cp;
	
	if (!*cp) {
	    /*
	     * Ending a dependency line without an operator is a Bozo
	     * no-no 
	     */
	    Parse_Error (PARSE_FATAL, catgets(catd, MS_MAKE, NEEDOPERATOR,
			 "Need an operator"));
	    return;
	}

	*cp = '\0';

/* PART 2 */

	/*
	 * Have a word in line. See if it's a special target and set
	 * specType to match it.
	 */

	if (*line == '.' && isupper (line[1])) {
	    /*
	     * See if the target is a special target that must have it
	     * or its sources handled specially. 
	     */
	    int keywd = ParseFindKeyword(line);
	    if (keywd != -1) {
		specType = parseKeywords[keywd].spec;
		tOp = parseKeywords[keywd].op; /* used later in the source processing */

		/*
		 * Certain special targets have special semantics:
		 *	.DEFAULT    	Need to create a node to hang
		 *			commands on, but we don't want
		 *			it in the graph, nor do we want
		 *			it to be the Main Target, so we
		 *			create it, set OP_NOTMAIN and
		 *			add it to the list, setting
		 *			DEFAULT to the new node for
		 *			later use. We claim the node is
		 *	    	    	A transformation rule to make
		 *	    	    	life easier later, when we'll
		 *	    	    	use Make_HandleTransform to actually
		 *	    	    	apply the .DEFAULT commands.
		 */
/* PART 2.1  */
		switch (specType) {
		    case Default:
			gn = Targ_NewGN(".DEFAULT");
			gn->type |= (OP_NOTMAIN|OP_TRANSFORM);
			(void)Lst_AtEnd(targets, (ClientData)gn);
			DEFAULT = gn;
			break;
		/*
		 * Certain special targets have special semantics:
		 *	.SCCS_GET    	Need to create a node to hang
		 *			commands on, but we don't want
		 *			it in the graph, nor do we want
		 *			it to be the Main Target, so we
		 *			create it, set OP_NOTMAIN and
		 *			add it to the list, setting
		 *			DEFAULT to the new node for
		 *			later use. We claim the node is
		 *	    	    	A transformation rule to make
		 *	    	    	life easier later, when we'll
		 *	    	    	use "??????" to actually
		 *	    	    	apply the .SCCS_GET commands.
		 */
		    case Sccsget:
			gn = Targ_NewGN(".SCCS_GET");
			gn->type |= (OP_NOTMAIN|OP_TRANSFORM);
			(void)Lst_AtEnd(targets, (ClientData)gn);
			SCCS_GET = gn;
			break;
		    case Posix:
			if (maybePosix)
			    isPosix = TRUE;
			else
			    Parse_Error(PARSE_WARNING, catgets(catd, MS_MAKE, NOTPOSIX, ".POSIX directive is not first non-comment line"));
		    case  Suffixes:	/* Setup for .SUFFIXES  was performed in main. */
			break;
		    default:
		    
			break;
		}		/* end "switch (specType)" */
	      }			/* end  "if (keywd != -1)"*/
	}			/* end "if (*line == '.' && isupper (line[1]))" */
	
/*  PART 2.2 */

	/*
	 * Have 'non special' word (target) in line. It could be that the target exists on the static list 
         * of targets. Get or create its node and stick it at the end of the static target list.
         * The target can also be a suffix rule in the form:  "c.o :" If it is, it also needs to added to 
         * Suffix graph.    
	 */

	if ((specType == Not) && (*line != '\0')) {
	    (void)Lst_AtEnd(curTargs, (ClientData)line); /* Stick the line data at the end of the local list. */
	    
	    while(!Lst_IsEmpty(curTargs)) {
		char	*targName = (char *)Lst_DeQueue(curTargs); /* Pull the first target off the local list/w its data */
		
		if (!Suff_IsTransform (targName)) { /* Determine if the target requires transformation. */
		    gn = Targ_FindNode (targName, TARG_CREATE);	/* it was not so create if it needs to created. */
		} else {
		    gn = Suff_AddTransform (targName);
		}
		
		(void)Lst_AtEnd (targets, (ClientData)gn);
	    }			/* end " while(!Lst_IsEmpty(curTargs))" */
	  }			/* end "if ((specType == Not) && (*line != '\0'))" */
	
	*cp = savec;

/* PART 3 */

	/*
	 * If the target was a special target, it's the only word (target) that should be left of the operator.
	 */

	if (specType != Not) {
	    Boolean warn = FALSE;
	    
	    while ((*cp != '!') && (*cp != ':') && *cp) {
		if (*cp != ' ' && *cp != '\t') {
		    warn = TRUE;
		}
		cp++;
	    }
	    if (warn) {
		Parse_Error(PARSE_WARNING, catgets(catd, MS_MAKE, IGNORETARGET,
			    "Extra target ignored"));
	    }
	} else {
	    while (*cp && isspace (*cp)) {
		cp++;
	    }
	}
	line = cp;

				/* end the main line parse loop */

      } while ((*line != ':') && *line);   /* end main parse do while loop */

 
/* PART 4 */

    /*
     * Don't need the local list of local current target names anymore...
     */


    Lst_Destroy(curTargs, NOFREE);

    if (!Lst_IsEmpty(targets)) { 
	switch(specType) {	/* The list is NOT empty if here */
	    default:
		Parse_Error(PARSE_WARNING, catgets(catd, MS_MAKE, MIXTARGET, "Special and mundane targets don't mix. Mundane ones ignored"));
		break;
	    case Default:
	    case Sccsget:
				/* These two  create nodes which to hang commands, so */
				/* targets should not be empty... */
	    case Not:
		/*
		 * Nothing special here -- targets can be empty if it wants.
		 */
		break;
	}
    }

/* PART 5 */

    /*
     * Have now parsed all the target names. Must parse the operator next. The
     * result is left in  op .
     * Apply the operator to all targets.
     */


    if (*cp == ':') {
	if (cp[1] == ':') {
	    op = OP_DOUBLEDEP;
	    cp++;
	} else {
	    op = OP_DEPENDS;
	}
    } else {
	Parse_Error (PARSE_FATAL, catgets(catd, MS_MAKE, MISSDEPOP,
		     "Missing dependency operator!"));
	return;
    }

    cp++;			/* Advance beyond operator */

    Lst_ForEach (targets, (int (*)(void*,void*))ParseDoOp, (ClientData)op);

/* PART 6 */


    /*
     * Get to the first source 
     */


    while (*cp && isspace (*cp)) {
	cp++;
    }
    line = cp;

/* PART 7 */

    /*
     * Several special targets take different actions if present with no
     * sources:
     *	a .SUFFIXES line with no sources clears out all old suffixes
     *	a .PRECIOUS line makes all targets precious
     *	a .IGNORE line ignores errors for all targets
     *	a .SILENT line creates silence when making all targets
     *  a .SCCS_GET This is a must!.
     */
    if (!*line) {
	switch (specType) {
	    case Suffixes:
		Suff_ClearSuffixes ();
		break;
	    case Precious:
		allPrecious = TRUE;
		break;
	    case Ignore:
		ignoreErrors = TRUE;
		break;
	    case Silent:
		beSilent = TRUE;
		break;
	    case Sccsget:		         
		break;  /* .SCCS_GET:  No sources allowed on this line. No action. It must be empty */
				/* Good so far. */
	    default:
		break;
	}
    }
/* PART 7.1 */

    /*
     * Two special targets cannot take sources:
     *  .SCCS_GET: Fatal Error.
     *  .DEFAULT: Fatal Error
     */

    if (*line) {
      switch (specType) {
      case Default:
	Parse_Error (PARSE_FATAL, ".DEFAULT:  special target must be specified without Prerequisites!");
        return;
      case Sccsget:
	Parse_Error (PARSE_FATAL, ".SCCS_GET: special target must be specified without Prerequisites!");
	return;
      default:
        break;
      }
    }


 /* PART 8 */   

    /*
     * START GO FOR ALL THE SOURCES 
     */

 /* PART 8.1 */
   
    /*
     * START WORK ON ".SUFFIX" SPECIAL SOURCES 
     */

    if (specType == Suffixes) {
	while (*line) {
	    /*
	     * If the target was one that doesn't take files as its sources
	     * but takes something like suffixes, we take each
	     * space-separated word on the line as a something and deal
	     * with it accordingly.
	     *
	     * If the target was .SUFFIXES, we take each source as a
	     * suffix and add it to the list of suffixes maintained by the
	     * Suff module.
	     */
	    char  savec;
	    while (*cp && !isspace (*cp)) {
		cp++;
	    }
	    savec = *cp;
	    *cp = '\0';
            Suff_AddSuffix (line);
	    *cp = savec;
	    if (savec != '\0') {
		cp++;
	    }
	    while (*cp && isspace (*cp)) {
		cp++;
	    }
	    line = cp;
	}			/*  "while (*line)" */

    /*
     * END WORK ON THE ".SUFFIX" SOURCES 
     */

 /* PART 8.2 */   

    } else {			/*  "if (specType == Suffixes)" */

    /*
     * START WORK ON ANY OTHER SOURCES 
     */

	while (*line) {
	    /*
	     * The targets take real sources, so we must beware of archive
	     * specifications (i.e. things with left parentheses in them)
	     * and handle them accordingly.
	     */
	    while (*cp && !isspace (*cp)) {
		if ((*cp == '(') && (cp > line) && (cp[-1] != '$')) {
		    /*
		     * Only stop for a left parenthesis if it is not at the
		     * start of a word (that'll be for variable changes
		     * later) and is not preceded by a dollar sign (a dynamic
		     * source).
		     */
		    break;
		} else {	/* end if */
		    cp++;
		}		/* end else */
	      }			/* end while */

/* The pointer has advanced to a point in a word*/

	    if (*cp == '(') {	/* It must be library archive. */
		GNode	  *gn;

		sources = Lst_Init (FALSE);
		if (Arch_ParseArchive (&line, sources, VAR_CMD) != SUCCESS) {
		    Parse_Error (PARSE_FATAL,
				 catgets(catd, MS_MAKE, SRCARERR,
				 "Error in source archive spec \"%s\""), line);
		    return;
		  }

		while (!Lst_IsEmpty (sources)) {
		    gn = (GNode *) Lst_DeQueue (sources);
		    ParseDoSrc (tOp, gn->name);
		  }
		Lst_Destroy (sources, NOFREE);
		cp = line;

	    } else {		/* It was not a library archive */
		if (*cp) {
		    *cp = '\0'; /* terminate the word */
		    cp += 1;	/* advance the pointer */
		  }

		ParseDoSrc (tOp, line);
	      }
	    while (*cp && isspace (*cp)) {
		cp++;
	    }
	    line = cp;
	}			/* end "while (*line)" */
    }				/* end else  */

    /*
     * END WORK ON ANY OTHER SOURCES 
     */

    /*
     * END GO FOR THE SOURCES 
     */

    
    if (mainNode == NILGNODE) {
	/*
	 * If we have yet to decide on a main target to make, in the
	 * absence of any user input, we want the first target on
	 * the first dependency line that is actually a real target.
	 */
	Lst_ForEach (targets, (int (*)(void*,void*))ParseFindMain, (ClientData)0);
    }

}				/* end of function */

/*-
 *---------------------------------------------------------------------
 * Parse_IsVar  --
 *	Return TRUE if the passed line is a variable assignment. A variable
 *	assignment consists of a single word followed by optional whitespace
 *	followed by either a += or an = operator.
 *	This function is used both by the Parse_File function and main when
 *	parsing the command-line arguments.
 *
 * Results:
 *	TRUE if it is. FALSE if it ain't
 *
 * Side Effects:
 *	none
 *---------------------------------------------------------------------
 */
Boolean
Parse_IsVar (char *line)
{
     /* char  *line;	The line to check */

     Boolean wasSpace = FALSE;	/* set TRUE if found a space */
     Boolean haveName = FALSE;	/* Set TRUE if have a variable name */

    /*
     * Skip to variable name
     */
    while ((*line == ' ') || (*line == '\t')) {
	line++;
    }

    while (*line != '=') {
	if (*line == '\0') {
	    /*
	     * end-of-line -- can't be a variable assignment.
	     */
	    return (FALSE);
	} else if ((*line == ' ') || (*line == '\t')) {
	    /*
	     * there can be as much white space as desired so long as there is
	     * only one word before the operator 
	     */
	    wasSpace = TRUE;
	} else if (wasSpace && haveName) {
	    /*
	     * This is the start of another word, so not assignment.
	     */
	    return (FALSE);
	} else {
	    haveName = TRUE; 
	    wasSpace = FALSE;
	}
	line++;
    }

    return (haveName);
}

/*-
 *---------------------------------------------------------------------
 * Parse_DoVar  --
 *	Take the variable assignment in the passed line and do it in the
 *	global context.
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	the variable structure of the given variable name is altered in the
 *	global context.
 *---------------------------------------------------------------------
 */
void
Parse_DoVar (char *line, GNode *ctxt)
{
    /* char            *line;	Line guaranteed to be a variable */
    /*                          assignment. This reduces error checks */
    /* GNode   	    *ctxt;    	Context in which to do the assignment */

     char   *cp;	/* pointer into line */

    /*
     * Skip to variable name
     */
    while ((*line == ' ') || (*line == '\t')) {
	line++;
    }

    /*
     * Skip to equal sign, nulling out whitespace as we go
     */
    for (cp = line + 1; *cp != '='; cp++) {
	if (isspace (*cp)) {
	    *cp = '\0';
	}
    }
    *cp++ = '\0';	/* nuke the = */

    while (isspace (*cp)) {
	cp++;
    }

    Var_Set (line, cp, ctxt);
}

/*-
 * ParseAddCmd  --
 *	Lst_ForEach function to add a command line to all targets
 *
 * Results:
 *	Always 0
 *
 * Side Effects:
 *	A new element is added to the commands list of the node.
 */
static int
ParseAddCmd(GNode *gn, char *cmd)
{
	/* GNode *gn;	The node to which the command is to be added */
	/* char *cmd;	The command to add */

	/* if target already supplied, ignore commands */
	if (!(gn->type & OP_HAS_COMMANDS))
		(void)Lst_AtEnd(gn->commands, (ClientData)cmd);
	return(0);
}

/*-
 *-----------------------------------------------------------------------
 * ParseHasCommands --
 *	Callback procedure for Parse_File when destroying the list of
 *	targets on the last dependency line. Marks a target as already
 *	having commands if it does, to keep from having shell commands
 *	on multiple dependency lines.
 *
 * Results:
 *	Always 0.
 *
 * Side Effects:
 *	OP_HAS_COMMANDS may be set for the target.
 *
 *-----------------------------------------------------------------------
 */
static void
ParseHasCommands(GNode *gn)
{
    /* GNode   	  *gn;	    Node to examine */

    if (!Lst_IsEmpty(gn->commands)) {
	gn->type |= OP_HAS_COMMANDS;
    }
    return;
}

/*-
 *---------------------------------------------------------------------
 * ParseDoInclude  --
 *	Push to another file.
 *	
 *	The input is the line minus the include.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	A structure is added to the includes Lst and readProc, lineno,
 *	fname and curFILE are altered for the new file
 *---------------------------------------------------------------------
 */
static void
ParseDoInclude (char *file)
{
    /* char          *file;	file specification */

    IFile         *oldFile;	/* state associated with current file */
    char	  *cp;

    /*
     * Skip whitespace
     */
    while ((*file == ' ') || (*file == '\t')) {
	file++;
    }

    /*
     * Substitute for any variables in the file name before trying to
     * find the thing.
     */
    if (strchr(file, '$') != (char *) NULL) {
	char *nfile = Var_Subst (file, VAR_CMD, FALSE);
	free (file);
	file = nfile;
    } else
	file = strdup(file);

    cp = file;
    while (*cp && *cp != ' ' && *cp != '\t')
	cp++;
    *cp = '\0';

    oldFile = (IFile *) emalloc (sizeof (IFile));
    oldFile->fname = fname;

    oldFile->F = curFILE;
    oldFile->lineno = lineno;

    (void) Lst_AtFront (includes, (ClientData)oldFile);

    /*
     * Once the previous state has been saved, we can get down to reading
     * the new file. We set up the name of the file to be the absolute
     * name of the include file so error messages refer to the right
     * place. Naturally enough, we start reading at line number 0.
     */
    fname = file;
    lineno = 0;

    curFILE = fopen (file, "r");
    if (curFILE == (FILE * ) NULL) {
	Parse_Error (PARSE_FATAL, catgets(catd, MS_MAKE, OPENERR,
		     "Cannot open %s"), file);
	/*
	 * Pop to previous file
	 */
	(void) ParseEOF(0);
    }
}

/*-
 *---------------------------------------------------------------------
 * ParseEOF  --
 *	Called when EOF is reached in the current file. If we were reading
 *	an include file, the includes stack is popped and things set up
 *	to go back to reading the previous file at the previous location.
 *
 * Results:
 *	KEEP_PARSING if there's more to do. DONE if not.
 *
 * Side Effects:
 *	The old curFILE, is closed. The includes list is shortened.
 *	lineno, curFILE, and fname are changed if KEEP_PARSING is returned.
 *---------------------------------------------------------------------
 */
static int
ParseEOF (int opened)
{
    IFile     *ifile;	/* the state on the top of the includes stack */

    if (Lst_IsEmpty (includes)) {
	return (DONE);
    }

    ifile = (IFile *) Lst_DeQueue (includes);
    free (fname);
    fname = ifile->fname;
    lineno = ifile->lineno;
    if (opened)
	(void) fclose (curFILE);
    curFILE = ifile->F;
    free ((Address)ifile);
    return (KEEP_PARSING);
}

/*-
 *---------------------------------------------------------------------
 * ParseReadc  --
 *	Read a character from the current file and update the line number
 *	counter as necessary
 *
 * Results:
 *	The character that was read
 *
 * Side Effects:
 *	The lineno counter is incremented if the character is a newline
 *---------------------------------------------------------------------
 */
#define ParseReadc() (getc(curFILE))


/*-
 *---------------------------------------------------------------------
 * ParseReadLine --
 *	Read an entire line from the input file. Called only by Parse_File.
 *	To facilitate escaped newlines and what have you, a character is
 *	buffered in 'lastc', which is '\0' when no characters have been
 *	read. When we break out of the loop, c holds the terminating
 *	character and lastc holds a character that should be added to
 *	the line (unless we don't read anything but a terminator).
 *
 * Results:
 *	A line w/o its newline
 *
 * Side Effects:
 *	Only those associated with reading a character
 *---------------------------------------------------------------------
 */
static char *
ParseReadLine (void)
{
    Buffer  	  buf;	    	/* Buffer for current line */
     int  c;	      	/* the current character */
     int  lastc;    	/* The most-recent character */
    Boolean	  semiNL;     	/* treat semi-colons as newlines */
    Boolean	  ignDepOp;   	/* TRUE if should ignore dependency operators
				 * for the purposes of setting semiNL */
    Boolean 	  ignComment;	/* TRUE if should ignore comments (in a
				 * shell command */
    char    	  *line;    	/* Result */
    int	    	  lineLength;	/* Length of result */

    semiNL = FALSE;
    ignDepOp = FALSE;
    ignComment = FALSE;

    /*
     * Handle special-characters at the beginning of the line. Either a
     * leading tab (shell command) or pound-sign (possible conditional)
     * forces us to ignore comments and dependency operators and treat
     * semi-colons as semi-colons (by leaving semiNL FALSE). This also
     * discards completely blank lines.
     */
    while(1) {
	c = ParseReadc();

	if (c == '\t') {
	    ignComment = ignDepOp = TRUE;
	    break;
	} else if (c == '\n') {
	    lineno++;
	} else if (c == '#') {
		ungetc(c, curFILE); 
		break;
	} else {
	    /*
	     * Anything else breaks out without doing anything
	     */
	    break;
	}
    }
	
    if (c != EOF) {
	lastc = c;
	buf = Buf_Init(BSIZE);
	
	while (((c = ParseReadc ()) != '\n' || (lastc == '\\')) &&
	       (c != EOF))
	{
test_char:
	    switch(c) {
	    case '\n':
		/*
		 * Escaped newline: read characters until a non-space or an
		 * unescaped newline and replace them all by a single space.
		 * This is done by storing the space over the backslash and
		 * dropping through with the next nonspace. If it is a
		 * semi-colon and semiNL is TRUE, it will be recognized as a
		 * newline in the code below this...
		 */
		lineno++;
		lastc = ' ';
		while ((c = ParseReadc ()) == ' ' || c == '\t') {
		    continue;
		}
		if (c == EOF || c == '\n') {
		    goto line_read;
		} else {
		    /*
		     * Check for comments, semiNL's, etc. -- easier than
		     * ungetc(c, curFILE); continue;
		     */
		    goto test_char;
		}
		break;
	    case ';':
		/*
		 * Semi-colon: Need to see if it should be interpreted as a
		 * newline
		 */
		if (semiNL) {
		    /*
		     * To make sure the command that may be following this
		     * semi-colon begins with a tab, we push one back into the
		     * input stream. This will overwrite the semi-colon in the
		     * buffer. If there is no command following, this does no
		     * harm, since the newline remains in the buffer and the
		     * whole line is ignored.
		     */
		    ungetc('\t', curFILE);
		    goto line_read;
		} 
		break;
	    case '=':
		if (!semiNL) {
		    /*
		     * Haven't seen a dependency operator before this, so this
		     * must be a variable assignment -- don't pay attention to
		     * dependency operators after this.
		     */
		    ignDepOp = TRUE;
		}
		break;
	    case '#':
		if (!ignComment) {
			/*
			 * If the character is a hash mark and it isn't escaped
			 * (or we're being compatible), the thing is a comment.
			 * Skip to the end of the line.
			 */
			do {
			    c = ParseReadc();
			} while ((c != '\n') && (c != EOF));
			goto line_read;
		}
		break;
	    case ':':
		if (!ignDepOp) {
		    /*
		     * A semi-colon is recognized as a newline only on
		     * dependency lines. Dependency lines are lines with a
		     * colon or an exclamation point. Ergo...
		     */
		    semiNL = TRUE;
		}
		break;
	    }
	    /*
	     * Copy in the previous character and save this one in lastc.
	     */
	    Buf_AddByte (buf, (Byte)lastc);
	    lastc = c;
	    
	}
    line_read:
	lineno++;
	
	if (lastc != '\0') {
	    Buf_AddByte (buf, (Byte)lastc);
	}
	Buf_AddByte (buf, (Byte)'\0');
	line = (char *)Buf_GetAll (buf, &lineLength);
	Buf_Destroy (buf, FALSE);
	
	return (line);
    } else {
	/*
	 * Hit end-of-file, so return a NULL line to indicate this.
	 */
	return((char *)NULL);
    }
}

/*-
 *-----------------------------------------------------------------------
 * ParseFinishDependencySpec --
 *	Handle the end of a dependency specification.
 *
 * Results:
 *	Nothing.
 *
 * Side Effects:
 *	inDependSpec set FALSE. 'targets' list destroyed.
 *
 *-----------------------------------------------------------------------
 */
static void
ParseFinishDependencySpec(void)
{
    extern int Suff_EndTransform(GNode *, ...);

    if (inDependSpec) {
#ifdef DEBUG_FLAG
      if (DEBUG(SUFF))
	  Parse_Print_All();
#endif
      if (!Lst_IsEmpty(targets)) {
	Lst_ForEach(targets, (int (*)(void*,void*))Suff_EndTransform, (ClientData)NULL);
	Lst_Destroy (targets, (void (*)(void*))ParseHasCommands);
	inDependSpec = FALSE;
      }
    }
  }
		    

/*-
 *---------------------------------------------------------------------
 * Parse_File --
 *	Parse a file into its component parts, incorporating it into the
 *	current dependency graph. This is the main function and controls
 *	almost every other function in this module
 *
 * PART 1 Look for comment lines, process them then continue.
 * PART 2 Look for command lines, process them then continue.
 * PART 3 Look for include lines, process them then continue.
 * PART 4 Look for variable lines, process them then continue.
 * PART 5 Look for depenceny lines, process them then continue.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Loads. Nodes are added to the list of all targets, nodes and links
 *	are added to the dependency graph. etc. etc. etc.
 *---------------------------------------------------------------------
 */
void
Parse_File(char *name, FILE * stream)
{
    /* char          *name;	The name of the file being read */
    /* FILE *	  stream;   	Stream open to makefile to parse */

    char *cp,		/* pointer into the line */
                  *line='\0';	/* the line we're working on */

    inDependSpec = FALSE;		/* are we in a dependency spec? */
    fname = name;
    curFILE = stream;
    lineno = 0;
    fatals = 0;
    maybePosix = TRUE;
    isPosix = FALSE;

    do {
	while (line = ParseReadLine ()) { /* read a line */
				/* skip comments */

 /* PART 1 Look for comment lines, process them then continue. */

	    if (*line == '#') {
		/* If we're this far, the line must be a comment. */
		free (line);
		continue;
	    }
				/* Is the line  a command line?  */

 /* PART 2 Look for command lines, process them then continue. */

 /* PART 2.1 Look for Special Targets that change if there are no commands. */

				/* .DEFAULT is an error */

				/* .SCCS_GET turn off SCCS_GET functionality. */
			       
	    if (inDependSpec) {
	      if ((specType == Sccsget) && (*line != '\t') && (Lst_IsEmpty(SCCS_GET->commands))) {
		TrySccsGet = FALSE;
	      }
	      else if   ((specType == Default) && (*line != '\t') && (Lst_IsEmpty(DEFAULT->commands))){ 
		Parse_Error (PARSE_FATAL, " .DEFAULT: special target must be specified with Commands!");
	      }
	    }
			     			 
	    if (*line == '\t')
	    {
		maybePosix = FALSE;
		/*
		 * If a line starts with a tab, it
		 * can only hope to be a creation command.
		 */
		
		for (cp = line + 1; isspace (*cp); cp++) {
		    continue;
		}
		if (*cp) {
		    if (inDependSpec) { /* If it is a command line, it must be part of a depenency spec. */
			/*
			 * So long as it's not a blank line and we're actually
			 * in a dependency spec, add the command to the list of
			 * commands of all targets in the dependency spec 
			 */
			Lst_ForEach (targets, (int (*)(void*,void*))ParseAddCmd, (ClientData)cp);
			continue;
		    } else {
			Parse_Error (PARSE_FATAL,
				     catgets(catd, MS_MAKE, SHCOMMAND,
				     "Unassociated shell command \"%.20s\""),
				     cp);
		    }
		}
		free (line);
		continue;	/* go to the next line  */
	    }			/* first char was a tab */

/* PART 3 Look for include lines, process them then continue. */

	    if (!isPosix && *line == 'i' && strncmp(line, "include", 7) == 0 &&
		(line[7] == ' ' || line[7] == '\t')) {
		ParseDoInclude (line + 7);
		free (line);
		continue;
	    }

/* PART 4 Look for variable lines, process them then continue. */

	    if (Parse_IsVar (line)) { /* Is this line a varable line "VARIABLE=value" */
		maybePosix = FALSE;
		ParseFinishDependencySpec();
		Parse_DoVar (line, VAR_GLOBAL);

/* PART 5 Look for depenceny lines, process them then continue. */

	    } else {		/* It could only be a depenency line if here */
		/*
		 * We now know it's a dependency line so it needs to have all
		 * variables expanded before being parsed. Tell the variable
		 * module to complain if some variable is undefined...
		 * To make life easier on novices, if the line is indented we
		 * first make sure the line has a dependency operator in it.
		 * If it doesn't have an operator and we're in a dependency
		 * line's script, we assume it's actually a shell command
		 * and add it to the current list of targets.
		 */
		Boolean	nonSpace = FALSE;
		
		cp = line;
		if (line[0] == ' ') {
		    while ((*cp != ':') && (*cp != '\0')) {
			if (!isspace(*cp)) {
			    nonSpace = TRUE;
			}
			cp++;
		    }
		}
		    
		if (*cp == '\0') {
		    if (inDependSpec) {
			Parse_Error (PARSE_FATAL,
				     catgets(catd, MS_MAKE, NEEDTAB,
				     "Shell command needs a leading tab"));
		    } else if (nonSpace) {
			Parse_Error (PARSE_FATAL, catgets(catd, MS_MAKE,
				     MISSOPERATOR, "Missing operator"));
		    }
		    		    
		} else {

		    ParseFinishDependencySpec();

		    cp = Var_Subst (line, VAR_CMD, TRUE);
		    free (line);
		    line = cp;
		    
		    /*
		     * Need a non-circular list for the target nodes 
		     */
		    targets = Lst_Init (FALSE);
		    inDependSpec = TRUE; /* A depenency Spec can have a command line */

				/* If here we know its is a dependency line. */
		    
		    ParseDoDepenencyLine (line); /* Do more Depenency Line's (targets and sources). */
		    maybePosix = FALSE;
		}
	    }
	}
	/*
	 * Reached EOF, but it may be just EOF of an include file... 
	 */
    } while (ParseEOF(1) == KEEP_PARSING);

    if (fatals) {
	fprintf (stderr, catgets(catd, MS_MAKE, FATALERR,
		"Fatal errors encountered -- cannot continue\n"));
	exit (2);
    }
}

/*-
 *---------------------------------------------------------------------
 * Parse_Init --
 *	initialize the parsing module
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	none
 *---------------------------------------------------------------------
 */
void
Parse_Init (void)
{
    mainNode = NILGNODE;
    includes = Lst_Init (FALSE);
}

/*-
 *-----------------------------------------------------------------------
 * Parse_MainName --
 *	Return a Lst of the main target to create for main()'s sake. If
 *	no such target exists, we Punt with an obnoxious error message.
 *
 * Results:
 *	A Lst of the single node to create.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
Lst
Parse_MainName(void)
{
    Lst           main;	/* result list */

    main = Lst_Init (FALSE);

    if (mainNode == NILGNODE) {
	Punt (catgets(catd, MS_MAKE, NOTARGET, "No target to make.\n"));
    	/*NOTREACHED*/
    } else if (mainNode->type & OP_DOUBLEDEP) {
	Lst_Concat(main, mainNode->cohorts, LST_CONCNEW);
    }
    (void) Lst_AtEnd (main, (ClientData)mainNode);
    return (main);
}
