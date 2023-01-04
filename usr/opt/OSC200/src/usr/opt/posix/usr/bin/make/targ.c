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
static char *rcsid = "@(#)$RCSfile: targ.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/29 22:05:58 $";
#endif

/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * Copyright (c) 1988, 1989 by Adam de  * Copyright (c) 1989 by Berkeley Softworks
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*-
 * targ.c --
 *	Functions for maintaining the Lst allTargets. Target nodes are
 * kept in two structures: a Lst, maintained by the list library, and a
 * hash table, maintained by the hash library.
 *
 * Interface:
 *	Targ_Init 	    	Initialization procedure.
 *
 *	Targ_NewGN	    	Create a new GNode for the passed target
 *	    	  	    	(string). The node is *not* placed in the
 *	    	  	    	hash table, though all its fields are
 *	    	  	    	initialized.
 *
 *	Targ_FindNode	    	Find the node for a given target, creating
 *	    	  	    	and storing it if it doesn't exist and the
 *	    	  	    	flags are right (TARG_CREATE)
 *
 *	Targ_FindList	    	Given a list of names, find nodes for all
 *	    	  	    	of them. If a name doesn't exist and the
 *	    	  	    	TARG_NOCREATE flag was given, an error message
 *	    	  	    	is printed. Else, if a name doesn't exist,
 *	    	  	    	its node is created.
 *
 *	Targ_Ignore	    	Return TRUE if errors should be ignored when
 *	    	  	    	creating the given target.
 *
 *	Targ_Silent	    	Return TRUE if we should be silent when
 *	    	  	    	creating the given target.
 *
 *	Targ_Precious	    	Return TRUE if the target is precious and
 *	    	  	    	should not be removed if we are interrupted.
 *
 * Debugging:
 *	Targ_PrintGraph	    	Print out the entire graph all variables
 *	    	  	    	and statistics for the directory cache. Should
 *	    	  	    	print something for suffixes, too, but...
 */

#include	  <stdio.h>
#include	  <time.h>
#include	  "make.h"
#include	  "hash.h"
#include 	  "pmake_msg.h"

extern nl_catd	  catd;
static Lst        allTargets;	/* the list of all targets found so far */
static Hash_Table targets;	/* a hash table of same */

#define HTSIZE	191		/* initial size of hash table */

/*-
 *-----------------------------------------------------------------------
 * Targ_Init --
 *	Initialize this module
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	The allTargets list and the targets hash table are initialized
 *-----------------------------------------------------------------------
 */
void
Targ_Init (void)
{
    allTargets = Lst_Init (FALSE);
    Hash_InitTable (&targets, HTSIZE);
}

/*-
 *-----------------------------------------------------------------------
 * Targ_NewGN  --
 *	Create and initialize a new graph node
 *
 * Results:
 *	An initialized graph node with the name field filled with a copy
 *	of the passed name
 *
 * Side Effects:
 *	None.
 *-----------------------------------------------------------------------
 */
GNode *
Targ_NewGN (char *name)
{
    /* char           *name;	The name to stick in the new node */

    GNode *gn;

    gn = (GNode *) emalloc (sizeof (GNode));
    gn->name = strdup (name);
    gn->path = (char *) 0;
    gn->type =		0;
    gn->unmade =    	0;
    gn->make = 	    	FALSE;
    gn->made = 	    	UNMADE;
    gn->childMade = 	FALSE;
    gn->mtime = gn->cmtime = 0;
    gn->iParents =  	Lst_Init (FALSE);
    gn->cohorts =   	Lst_Init (FALSE);
    gn->parents =   	Lst_Init (FALSE);
    gn->children =  	Lst_Init (FALSE);
    gn->successors = 	Lst_Init(FALSE);
    gn->preds =     	Lst_Init(FALSE);
    gn->context =   	Lst_Init (FALSE);
    gn->commands =  	Lst_Init (FALSE);

    return (gn);
}

/*-
 *-----------------------------------------------------------------------
 * Targ_FindNode  --
 *	Find a node in the list using the given name for matching
 *
 * Results:
 *	The node in the list if it was. If it wasn't, return NILGNODE of
 *	flags was TARG_NOCREATE or the newly created and initialized node
 *	if it was TARG_CREATE
 *
 * Side Effects:
 *	Sometimes a node is created and added to the list
 *-----------------------------------------------------------------------
 */
GNode *
Targ_FindNode (char *name, int flags)
{
    /* char           *name;	The name to find */
    /* int             flags;	flags governing events when target not found */

    GNode         *gn;	      /* node in that element */
    Hash_Entry	  *he;	      /* New or used hash entry for node */
    Boolean	  isNew;      /* Set TRUE if Hash_CreateEntry had to create */
			      /* an entry for the node */


    if (flags & TARG_CREATE) {
	he = Hash_CreateEntry (&targets, name, &isNew);
	if (isNew) {
	    gn = Targ_NewGN (name);
	    Hash_SetValue (he, gn);
	    (void) Lst_AtEnd (allTargets, (ClientData)gn);
	}
    } else {
	he = Hash_FindEntry (&targets, name);
    }

    if (he == (Hash_Entry *) NULL) {
	return (NILGNODE);
    } else {
	return ((GNode *) Hash_GetValue (he));
    }
}

/*-
 *-----------------------------------------------------------------------
 * Targ_FindList --
 *	Make a complete list of GNodes from the given list of names 
 *
 * Results:
 *	A complete list of graph nodes corresponding to all instances of all
 *	the names in names. 
 *
 * Side Effects:
 *	If flags is TARG_CREATE, nodes will be created for all names in
 *	names which do not yet have graph nodes. If flags is TARG_NOCREATE,
 *	an error message will be printed for each name which can't be found.
 * -----------------------------------------------------------------------
 */
Lst
Targ_FindList (Lst names, int flags)
{
    /* Lst            names;	list of names to find */
    /* int            flags;	flags used if no node is found for a given name */

    Lst            nodes;	/* result list */
    LstNode  ln;		/* name list element */
    GNode *gn;		/* node in tLn */
    char    	  *name;

    nodes = Lst_Init (FALSE);

    if (Lst_Open (names) == FAILURE) {
	return (nodes);
    }
    while ((ln = Lst_Next (names)) != NILLNODE) {
	name = (char *)Lst_Datum(ln);
	gn = Targ_FindNode (name, flags);
	if (gn != NILGNODE) {
	    /*
	     * Note: Lst_AtEnd must come before the Lst_Concat so the nodes
	     * are added to the list in the order in which they were
	     * encountered in the makefile.
	     */
	    (void) Lst_AtEnd (nodes, (ClientData)gn);
	    if (gn->type & OP_DOUBLEDEP) {
		(void)Lst_Concat (nodes, gn->cohorts, LST_CONCNEW);
	    }
	} else if (flags == TARG_NOCREATE) {
		Error (catgets(catd, MS_MAKE, UNKNOWNTARG,
                   "\"%s\" -- target unknown."), name);
	}
    }
    Lst_Close (names);
    return (nodes);
}

/*-
 *-----------------------------------------------------------------------
 * Targ_Ignore  --
 *	Return true if should ignore errors when creating gn
 *
 * Results:
 *	TRUE if should ignore errors
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
Boolean
Targ_Ignore (GNode *gn)
{
    /* GNode          *gn;		node to check for */

    if (ignoreErrors || gn->type & OP_IGNORE) {
	return (TRUE);
    } else {
	return (FALSE);
    }
}

/*-
 *-----------------------------------------------------------------------
 * Targ_Silent  --
 *	Return true if be silent when creating gn
 *
 * Results:
 *	TRUE if should be silent
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
Boolean
Targ_Silent (GNode *gn)
{
    /* GNode          *gn;   node to check for */

    if (beSilent || gn->type & OP_SILENT) {
	return (TRUE);
    } else {
	return (FALSE);
    }
}

/*-
 *-----------------------------------------------------------------------
 * Targ_Precious --
 *	See if the given target is precious
 *
 * Results:
 *	TRUE if it is precious. FALSE otherwise
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
Boolean
Targ_Precious (GNode *gn)
{
    /* GNode          *gn;   node to check */

    if (allPrecious || (gn->type & (OP_PRECIOUS|OP_DOUBLEDEP))) {
	return (TRUE);
    } else {
	return (FALSE);
    }
}

/******************* DEBUG INFO PRINTING ****************/

static GNode	  *mainTarg;	/* the main target, as set by Targ_SetMain */
/*- 
 *-----------------------------------------------------------------------
 * Targ_SetMain --
 *	Set our idea of the main target we'll be creating. Used for
 *	debugging output.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	"mainTarg" is set to the main target's node.
 *-----------------------------------------------------------------------
 */
void
Targ_SetMain (GNode *gn)
{
    /* GNode   *gn;   The main target we'll create */

    mainTarg = gn;
}

int
TargPrintName (GNode *gn, ...)
{
    /* GNode          *gn; */

    printf (catgets(catd, MS_DEBUG, TARG001, "#\t\tTarget Name = \"%s\".\n"),
    	    gn->name);
    return (0);
}


int
Targ_PrintCmd (char *cmd, ...)
{
    /* char           *cmd; */

    printf ("#\t\t%s\n", cmd);
    return (0);
}

/*-
 *-----------------------------------------------------------------------
 * Targ_FmtTime --
 *	Format a modification time in some reasonable way and return it.
 *
 * Results:
 *	The time reformatted.
 *
 * Side Effects:
 *	The time is placed in a static area, so it is overwritten
 *	with each call.
 *
 *-----------------------------------------------------------------------
 */
char *
Targ_FmtTime (time_t time)
{
    /* time_t    time; */

    struct tm	  	*parts;
    static char	  	buf[128];

    parts = localtime(&time);
    strftime(buf, sizeof(buf) - 1, "%c", parts) ;
    return(buf);
}
    
/*-
 *-----------------------------------------------------------------------
 * Targ_PrintType --
 *	Print out a type field giving only those attributes the user can
 *	set.
 *
 * Results:
 *   this code is broke...
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
void
Targ_PrintType (int type, ...)
{
    /* int    type; */


#ifdef DEBUG_FLAG

    int    tbit;
    /* type &= ~OP_OPMASK;		mast off the depenency operator bits */
    if (!type)
      printf("#\No known attributes.");
    else
       printf("#\t");
    while (type) {
      tbit = 1 << (ffs(type) - 1); 
      type &= ~tbit;
      switch(tbit) {
      case OP_DEPENDS  : printf("DEPEND "); break;
      case OP_DOUBLEDEP  : printf("DOUBLEDEP "); break;
      case OP_IGNORE  : printf("IGNORE "); break;
      case OP_PRECIOUS : printf("PRECIOUS "); break;
      case OP_SILENT  : printf("SILENT "); break;
      case OP_MAKE  : printf("MAKE "); break;
      case OP_INVISIBLE  : printf("INVISIBLE "); break;
      case OP_NOTMAIN  : printf("NOTMAIN "); break;	    
      case OP_TRANSFORM  : printf("TRANSFORM "); break;
      case OP_MEMBER  : printf("MEMBER "); break;
      case OP_ARCHV  : printf("ARCHV "); break;
      case OP_HAS_COMMANDS  : printf("HASCOMMANDS "); break;
      case OP_DEPS_FOUND  : printf("DEPSFOUND "); break;
      }
    }
    printf("\n");

#endif				/* DEBUG_FLAG */

  }

/*-
 *-----------------------------------------------------------------------
 * TargPrintNode --
 *	print the contents of a node
 *-----------------------------------------------------------------------
 */
static int
TargPrintNode (GNode *gn, int pass)
{
    /* GNode         *gn; */
    /* int	     pass; */

    if (!OP_NOP(gn->type)) {
	if (gn == mainTarg) {
            printf("\n#***\n");
	    printf(catgets(catd, MS_DEBUG, TARG002,
		"#*** Printing Main Target \"%s\" Information:\n"),gn->name);
            printf("#***\n\n");
	} else {
            printf("\n#***\n");
	    printf(catgets(catd, MS_DEBUG, TARG003,
		"#*** Printing Target \"%s\" Information:\n"),gn->name);
            printf("#***\n\n");
	  }
	if (pass <=  2) {
	    if (gn->unmade) {
		printf(catgets(catd, MS_DEBUG, TARG004,
		  "#\tTarget \"%s\" had \"unmade\" Prerequisites.\n"),gn->name);
	    } else {
		printf(catgets(catd, MS_DEBUG, TARG005,
		  "#\tTarget \"%s\" had no \"unmade\" Prerequisites.\n"), gn->name);
	    }
	    if (gn->mtime != 0) {
		printf(catgets(catd, MS_DEBUG, TARG006, "#\tTarget \"%s\" was last modified on: %s.\n"),gn->name,Targ_FmtTime(gn->mtime));
		printf(catgets(catd, MS_DEBUG, TARG007, "#\tTarget \"%s\" status was considered \"%s\".\n"),gn->name,
			  (gn->made == UNMADE ? "unmade" :
			   (gn->made == MADE ? "made" :
			    (gn->made == UPTODATE ? "up-to-date" :
			     "error when made"))));

	    } else if (gn->made != UNMADE) {
	      printf(catgets(catd, MS_DEBUG, TARG008, "#\tTarget \"%s\" had no modification time.\n"),gn->name);
	      printf(catgets(catd, MS_DEBUG, TARG009, "#\tTarget \"%s\" could be non-existent: Its status was considered \"%s\".\n"),gn->name,
		     (gn->made == MADE ? "made" :
		      (gn->made == UPTODATE ? "up-to-date" :
		       (gn->made == ERROR ? "error when made" :
			"aborted"))));
	    } else {
		printf(catgets(catd, MS_DEBUG, TARG010, "#\tTarget \"%s\" was not modified and not \"made\".\n"),gn->name);
	    }
	    if (!Lst_IsEmpty (gn->iParents)) {
		printf(catgets(catd, MS_DEBUG, TARG011, "#\tTarget \"%s\" was an \"Implied\" prerequisite of the following target(s):\n "),gn->name);
		Lst_ForEach (gn->iParents, (int (*)(void*,void*))TargPrintName, (ClientData)0);
		putc ('\n', stdout);
	    } else printf(catgets(catd, MS_DEBUG, TARG012, "#\tTarget \"%s\" was not an \"Implied\" prerequisite of any other target.\n"),gn->name);
	}
	if (!Lst_IsEmpty (gn->parents)) {
	    printf(catgets(catd, MS_DEBUG, TARG013, "#\tTarget \"%s\" was a prerequisite of the following target(s):\n"),gn->name);
	    Lst_ForEach ((Lst)gn->parents, (int (*)(void*,void*))TargPrintName, (ClientData)0);
	    putc ('\n', stdout);
	} else printf(catgets(catd, MS_DEBUG, TARG014, "#\tTarget \"%s\" was not a prerequisite of any other target.\n"),gn->name);
 
        printf(catgets(catd, MS_DEBUG, TARG015,
		"#\tTarget \"%s\" had the following Operator.   "),gn->name);
	switch (gn->type & OP_OPMASK) {
	    case OP_DEPENDS:
		printf("\":\"\n"); break;
	    case OP_DOUBLEDEP:
		printf("\"::\"\n"); break;
	}

#ifdef DEBUG_FLAG

	printf("#\tTarget \"%s\" had the following Attributes:\n",gn->name);
	Targ_PrintType (gn->type);

#endif				/* DEBUG_FLAG */

	if (!Lst_IsEmpty (gn->children)) {
	  printf(catgets(catd, MS_DEBUG, TARG016, "#\tTarget \"%s\" had the following Prerequisite(s): \n"),gn->name);
	  Lst_ForEach (gn->children,  (int (*)(void*,void*))TargPrintName, (ClientData)0);
	  printf("\n");
	} else printf(catgets(catd, MS_DEBUG, TARG017, "#\tTarget \"%s\" had no known Prerequisites.\n"),gn->name);

	if (!Lst_IsEmpty (gn->commands)) {
	  printf("\n");
	  printf(catgets(catd, MS_DEBUG, TARG018, "#\tTarget \"%s\" had the following Command(s):\n"),gn->name);
	  Lst_ForEach (gn->commands, (int (*)(void*,void*))Targ_PrintCmd, (ClientData)0);
          printf("\n");
	} else printf(catgets(catd, MS_DEBUG, TARG019, "#\tTarget \"%s\" had no known Commands.\n"),gn->name);

	if (gn->type & OP_DOUBLEDEP) {
	  printf(catgets(catd, MS_DEBUG, TARG020, "#\tTarget \"%s\" had the following Cohort(s):\n") ,gn->name);
	    Lst_ForEach (gn->cohorts, (int (*)(void*,void*))TargPrintNode, (ClientData)pass);
	}
    }
    return (0);
}

/*-
 *-----------------------------------------------------------------------
 * TargPrintOnlySrc --
 *	Print only those targets that are just a source.
 *
 * Results:
 *	0.
 *
 * Side Effects:
 *	The name of each file is printed preceeded by #\t
 *
 *-----------------------------------------------------------------------
 */
static int
TargPrintOnlySrc(GNode *gn, ...)
{
    /* GNode   	  *gn; */

    if (OP_NOP(gn->type)) {
	printf(catgets(catd, MS_DEBUG, TARG021,
		"#\t\t\"%s\" is only a Prerequisite.\n"), gn->name);
    }
    return (0);
}

/*-
 *-----------------------------------------------------------------------
 * Targ_PrintGraph --
 *	print the entire graph. heh heh
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	lots o' output
 *-----------------------------------------------------------------------
 */
void
Targ_PrintGraph (int pass)
{
    /* int pass; Which pass this is. 1 => no processing 2 => processing done */

				/* Target(s) Information */
  printf("\n#***\n");
  printf(catgets(catd, MS_DEBUG, TARG022,
    "#*** Printing all known Target(s) Information:\n"));
  printf("#***\n\n");
  Lst_ForEach (allTargets, (int (*)(void*,void*))TargPrintNode, (ClientData)pass);
  if (Lst_IsEmpty (allTargets)) {
    printf(catgets(catd, MS_DEBUG, TARG023, "\n#* No known Target(s).\n\n"));
  }
  else
    printf("\n\n");
				/* (Sources/Dependents/Prerequisites) */
  printf("\n#***\n");
  printf(catgets(catd, MS_DEBUG, TARG024,
    "#*** Printing  all Prerequisites that are not targets:\n"));
  printf("#***\n");
  Lst_ForEach (allTargets, (int (*)(void*,void*))TargPrintOnlySrc,(ClientData)0 );

				/* Global Variables */
  printf("\n#***\n");
  printf(catgets(catd, MS_DEBUG, TARG025,
    "#*** Printing all known Global Variables:\n"));
  printf("#***\n\n");
  Var_Dump (VAR_GLOBAL);
				/* Command-line Variables */
  printf("\n#***\n");
  printf(catgets(catd, MS_DEBUG, TARG026, 
    "#*** Printing all known Command-line Variables:\n"));
  printf("#***\n\n");
  Var_Dump (VAR_CMD);

#ifdef DEBUG_FLAG
				/* Directory Information */
  if (DEBUG(DIR)) {
    printf("\n#***\n");
    printf("#*** Printing all known Directory Information:\n");
    printf("#***\n\n");
    Dir_PrintDirectories();
  }
#endif				/* DEBUG_FLAG */
				/*  Suffix Information */
  printf("\n#***\n");
  printf(catgets(catd, MS_DEBUG, TARG027, 
    "#*** Printing all known Suffix Information:\n"));
  printf("#***\n\n");
  Suff_PrintAll();

}
