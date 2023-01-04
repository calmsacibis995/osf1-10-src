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
static char *rcsid = "@(#)$RCSfile: var.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/29 22:06:03 $";
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
 * var.c --
 *	Variable-handling functions
 *
 * Interface:
 *	Var_Set	  	    Set the value of a variable in the given
 *	    	  	    context. The variable is created if it doesn't
 *	    	  	    yet exist. The value and variable name need not
 *	    	  	    be preserved.
 *
 *	Var_Append	    Append more characters to an existing variable
 *	    	  	    in the given context. The variable needn't
 *	    	  	    exist already -- it will be created if it doesn't.
 *	    	  	    A space is placed between the old value and the
 *	    	  	    new one.
 *
 *	Var_Exists	    See if a variable exists.
 *
 *	Var_Value 	    Return the value of a variable in a context or
 *	    	  	    NULL if the variable is undefined.
 *
 *	Var_Subst 	    Substitute for all variables in a string using
 *	    	  	    the given context as the top-most one. If the
 *	    	  	    third argument is non-zero, Parse_Error is
 *	    	  	    called if any variables are undefined.
 *
 *	Var_Parse 	    Parse a variable expansion from a string and
 *	    	  	    return the result and the number of characters
 *	    	  	    consumed.
 *
 *	Var_Init  	    Initialize this module.
 *
 * Debugging:
 *	Var_Dump  	    Print out all variables defined in the given
 *	    	  	    context.
 *
 * XXX: There's a lot of duplication in these functions.
 */

#include    <stdio.h>
#include    <ctype.h>
#include    <stdlib.h>
#include    "make.h"
#include    "buf.h"
#include    "pmake_msg.h"

extern nl_catd	catd;	/* Message catalog descriptor */

/*
 * This is a harmless return value for Var_Parse that can be used by Var_Subst
 * to determine if there was an error in parsing -- easier than returning
 * a flag, as things outside this module don't give a hoot.
 */
char 	var_Error[] = "";

/*
 * Similar to var_Error, but returned when the 'err' flag for Var_Parse is
 * set false. Why not just use a constant? Well, gcc likes to condense
 * identical string instances...
 */
static char	varNoError[] = "";

/*
 * Internally, variables are contained in four different contexts.
 *	1) the environment. They may not be changed. If an environment
 *	    variable is appended-to, the result is placed in the global
 *	    context.
 *	2) the global context. Variables set in the Makefile are located in
 *	    the global context. It is the penultimate context searched when
 *	    substituting.
 *	3) the command-line context. All variables set on the command line
 *	   are placed in this context. They are UNALTERABLE once placed here.
 *	4) the local context. Each target has associated with it a context
 *	   list. On this list are located the structures describing such
 *	   local variables as $(@) and $(*)
 * The four contexts are searched in the reverse order from which they are
 * listed.
 */
GNode          *VAR_GLOBAL;   /* variables from the makefile */
GNode          *VAR_CMD;      /* variables defined on the command-line */

#define FIND_CMD	0x1   /* look in VAR_CMD when searching */
#define FIND_GLOBAL	0x2   /* look in VAR_GLOBAL as well */
#define FIND_ENV  	0x4   /* look in the environment also */

typedef struct Var {
    char          *name;	/* the variable's name */
    Buffer	  val;	    	/* its value */
    int	    	  flags;    	/* miscellaneous status flags */
#define VAR_IN_USE	1   	    /* Variable's value currently being used.
				     * Used to avoid recursion */
#define VAR_FROM_ENV	2   	    /* Variable comes from the environment */
#define VAR_JUNK  	4   	    /* Variable is a junk variable that
				     * should be destroyed when done with
				     * it. Used by Var_Parse for undefined,
				     * modified variables */
}  Var;

/*-
 *-----------------------------------------------------------------------
 * VarCmp  --
 *	See if the given variable matches the named one. Called from
 *	Lst_Find when searching for a variable of a given name.
 *
 * Results:
 *	0 if they match. non-zero otherwise.
 *
 * Side Effects:
 *	none
 *-----------------------------------------------------------------------
 */
static int
VarCmp (Var *v, char *name)
{
    /* Var            *v;	VAR structure to compare */
    /* char           *name;	name to look for */

    return (strcmp (name, v->name));
}

/*-
 *-----------------------------------------------------------------------
 * VarFind --
 *	Find the given variable in the given context and any other contexts
 *	indicated.
 *
 * Results:
 *	A pointer to the structure describing the desired variable or
 *	NIL if the variable does not exist.
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
static Var *
VarFind (char *name, GNode *ctxt, int flags)
{
    /* char           	*name;	name to find */
    /* GNode          	*ctxt;	context in which to find it */
    /* int             	flags;	FIND_GLOBAL set means to look in the
     *                          VAR_GLOBAL context as well.
     *                          FIND_CMD set means to look in the VAR_CMD
     *                          context also.
     *                          FIND_ENV set means to look in the
     *                          environment */

    LstNode         	var;
    Var		  	*v;

    /*
     * First look for the variable in the given context. If it's not there,
     * look for it in VAR_CMD, VAR_GLOBAL and the environment, in that order,
     * depending on the FIND_* flags in 'flags'
     */
    var = Lst_Find ((Lst)ctxt->context, (ClientData)name, (int(*)(void*,void*))VarCmp);

    if ((var == NILLNODE) && (flags & FIND_CMD) && (ctxt != VAR_CMD)) {
	var = Lst_Find (VAR_CMD->context, (ClientData)name, (int(*)(void*,void*))VarCmp);
    }
    if (!checkEnvFirst && (var == NILLNODE) && (flags & FIND_GLOBAL) &&
	(ctxt != VAR_GLOBAL))
    {
	var = Lst_Find (VAR_GLOBAL->context, (ClientData)name, (int(*)(void*,void*))VarCmp);
    }
    if ((var == NILLNODE) && (flags & FIND_ENV)) {
	char *env;

	if ((env = getenv (name)) != NULL) {
	    /*
	     * If the variable is found in the environment, we only duplicate
	     * its value (since eVarVal was allocated on the stack). The name
	     * doesn't need duplication since it's always in the environment
	     */
	    int	  	len;
	    
	    v = (Var *) emalloc(sizeof(Var));
	    v->name = name;

	    len = strlen(env);
	    
	    v->val = Buf_Init(len);
	    Buf_AddBytes(v->val, len, (Byte *)env);
	    
	    v->flags = VAR_FROM_ENV;
	    return (v);
	} else if (checkEnvFirst && (flags & FIND_GLOBAL) &&
		   (ctxt != VAR_GLOBAL))
	{
	    var = Lst_Find (VAR_GLOBAL->context, (ClientData)name, (int(*)(void*,void*))VarCmp);
	    if (var == NILLNODE) {
		return ((Var *) NIL);
	    } else {
		return ((Var *)Lst_Datum(var));
	    }
	} else {
	    return((Var *)NIL);
	}
    } else if (var == NILLNODE) {
	return ((Var *) NIL);
    } else {
	return ((Var *) Lst_Datum (var));
    }
}

/*-
 *-----------------------------------------------------------------------
 * VarAdd  --
 *	Add a new variable of name name and value val to the given context
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	The new variable is placed at the front of the given context
 *	The name and val arguments are duplicated so they may
 *	safely be freed.
 *-----------------------------------------------------------------------
 */
static void
VarAdd (char *name, char *val, GNode *ctxt)
{
    /* char           *name;	name of variable to add */
    /* char           *val;	value to set it to */
    /* GNode          *ctxt;	context in which to set it */

    Var   *v;
    int	    	  len;

    v = (Var *) emalloc (sizeof (Var));

    v->name = strdup (name);

    len = strlen(val);
    v->val = Buf_Init(len+1);
    Buf_AddBytes(v->val, len, (Byte *)val);

    v->flags = 0;

    (void) Lst_AtFront (ctxt->context, (ClientData)v);
    if (DEBUG(VAR)) {
	printf("%s:%s = %s\n", ctxt->name, name, val);
    }
}

/*-
 *-----------------------------------------------------------------------
 * Var_Set --
 *	Set the variable name to the value val in the given context.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	If the variable doesn't yet exist, a new record is created for it.
 *	Else the old value is freed and the new one stuck in its place
 *
 * Notes:
 *	The variable is searched for only in its context before being
 *	created in that context. I.e. if the context is VAR_GLOBAL,
 *	only VAR_GLOBAL->context is searched. Likewise if it is VAR_CMD, only
 *	VAR_CMD->context is searched. This is done to avoid the literally
 *	thousands of unnecessary strcmp's that used to be done to
 *	set, say, $(@) or $(<).
 *-----------------------------------------------------------------------
 */
void
Var_Set (char *name, char *val, GNode *ctxt)
{
    /* char           *name;	name of variable to set */
    /* char           *val;	value to give to the variable */
    /* GNode          *ctxt;	context in which to set it */

    Var   *v;

    /*
     * We only look for a variable in the given context since anything set
     * here will override anything in a lower context, so there's not much
     * point in searching them all just to save a bit of memory...
     */
    v = VarFind (name, ctxt, 0);
    if (v == (Var *) NIL) {
	VarAdd (name, val, ctxt);
    } else {
	Buf_Discard(v->val, Buf_Size(v->val));
	Buf_AddBytes(v->val, strlen(val), (Byte *)val);

	if (DEBUG(VAR)) {
	    printf("%s:%s = %s\n", ctxt->name, name, val);
	}
    }
    /*
     * Any variables given on the command line are automatically exported
     * to the environment (as per POSIX standard)
     */
    if (ctxt == VAR_CMD) {
	setenv(name, val, 1);
    }
}

/*-
 *-----------------------------------------------------------------------
 * Var_Append --
 *	The variable of the given name has the given value appended to it in
 *	the given context.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	If the variable doesn't exist, it is created. Else the strings
 *	are concatenated (with a space in between).
 *
 * Notes:
 *	Only if the variable is being sought in the global context is the
 *	environment searched.
 *	XXX: Knows its calling circumstances in that if called with ctxt
 *	an actual target, it will only search that context since only
 *	a local variable could be being appended to. This is actually
 *	a big win and must be tolerated.
 *-----------------------------------------------------------------------
 */
void
Var_Append (char *name, char *val, GNode *ctxt)
{
    /* char           *name;	Name of variable to modify */
    /* char           *val;	String to append to it */
    /* GNode          *ctxt;	Context in which this should occur */

    Var   *v;

    v = VarFind (name, ctxt, (ctxt == VAR_GLOBAL) ? FIND_ENV : 0);

    if (v == (Var *) NIL) {
	VarAdd (name, val, ctxt);
    } else {
	Buf_AddByte(v->val, (Byte)' ');
	Buf_AddBytes(v->val, strlen(val), (Byte *)val);

	if (DEBUG(VAR)) {
	    printf("%s:%s = %s\n", ctxt->name, name,
		   Buf_GetAll(v->val, (int *)NULL));
	}

	if (v->flags & VAR_FROM_ENV) {
	    /*
	     * If the original variable came from the environment, we
	     * have to install it in the global context (we could place
	     * it in the environment, but then we should provide a way to
	     * export other variables...)
	     */
	    v->flags &= ~VAR_FROM_ENV;
	    Lst_AtFront(ctxt->context, (ClientData)v);
	}
    }
}

/*-
 *-----------------------------------------------------------------------
 * Var_Exists --
 *	See if the given variable exists.
 *
 * Results:
 *	TRUE if it does, FALSE if it doesn't
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
Boolean
Var_Exists(char *name, GNode *ctxt)
{
    /* char	  *name;    	Variable to find */
    /* GNode	  *ctxt;    	Context in which to start search */

    Var	    	  *v;

    v = VarFind(name, ctxt, FIND_CMD|FIND_GLOBAL|FIND_ENV);

    if (v == (Var *)NIL) {
	return(FALSE);
    } else if (v->flags & VAR_FROM_ENV) {
	Buf_Destroy(v->val, TRUE);
	free((char *)v);
    }
    return(TRUE);
}

/*-
 *-----------------------------------------------------------------------
 * Var_Value --
 *	Return the value of the named variable in the given context
 *
 * Results:
 *	The value if the variable exists, NULL if it doesn't
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
char *
Var_Value (char *name, GNode *ctxt)
{
    /* char           *name;	name to find */
    /* GNode          *ctxt;	context in which to search for it */

    Var            *v;

    v = VarFind (name, ctxt, FIND_ENV | FIND_GLOBAL | FIND_CMD);
    if (v != (Var *) NIL) {
      /*  printf("value == %s\n",(char *)Buf_GetAll(v->val, (int *)NULL)); */
	return ((char *)Buf_GetAll(v->val, (int *)NULL));
    } else {
	return ((char *) NULL);
    }
}

/*-
 *-----------------------------------------------------------------------
 * VarHead --
 *	Remove the tail of the given word and place the result in the given
 *	buffer.
 *
 * Results:
 *	TRUE if characters were added to the buffer (a space needs to be
 *	added to the buffer before the next word).
 *
 * Side Effects:
 *	The trimmed word is added to the buffer.
 *
 *-----------------------------------------------------------------------
 */
static Boolean
VarHead (char *word, Boolean addSpace, Buffer buf, ...)
{
    /* char    	  *word;    	Word to trim */
    /* Boolean 	  addSpace; 	True if need to add a space to the buffer
     *                          before sticking in the head */
    /* Buffer  	  buf;	    	Buffer in which to store it */

    char *slash;

    slash = rindex (word, '/');
    if (slash != (char *)NULL) {
	if (addSpace) {
	    Buf_AddByte (buf, (Byte)' ');
	}
	*slash = '\0';
	Buf_AddBytes (buf, strlen (word), (Byte *)word);
	*slash = '/';
	return (TRUE);
    } else {
	/*
	 * If no directory part, give . (q.v. the POSIX standard)
	 */
	if (addSpace) {
	    Buf_AddBytes(buf, 2, (Byte *)" .");
	} else {
	    Buf_AddByte(buf, (Byte)'.');
	}
	return(TRUE);
    }
}

/*-
 *-----------------------------------------------------------------------
 * VarTail --
 *	Remove the head of the given word and place the result in the given
 *	buffer.
 *
 * Results:
 *	TRUE if characters were added to the buffer (a space needs to be
 *	added to the buffer before the next word).
 *
 * Side Effects:
 *	The trimmed word is added to the buffer.
 *
 *-----------------------------------------------------------------------
 */
static Boolean
VarTail (char *word, Boolean addSpace, Buffer buf, ...)
{
    /* char    	  *word;    	Word to trim */
    /* Boolean 	  addSpace; 	TRUE if need to stick a space in the
     *                          buffer before adding the tail */
    /* Buffer  	  buf;	    	Buffer in which to store it */

    char *slash;

    if (addSpace) {
	Buf_AddByte (buf, (Byte)' ');
    }

    slash = rindex (word, '/');
    if (slash != (char *)NULL) {
	*slash++ = '\0';
	Buf_AddBytes (buf, strlen(slash), (Byte *)slash);
	slash[-1] = '/';
    } else {
	Buf_AddBytes (buf, strlen(word), (Byte *)word);
    }
    return (TRUE);
}

typedef struct {
    char    	  *lhs;	    /* String to match */
    int	    	  leftLen;  /* Length of string */
    char    	  *rhs;	    /* Replacement string (w/ &'s removed) */
    int	    	  rightLen; /* Length of replacement */
    int	    	  flags;
#define VAR_SUB_GLOBAL	1   /* Apply substitution globally */
#define VAR_MATCH_START	2   /* Match at start of word */
#define VAR_MATCH_END	4   /* Match at end of word */
#define VAR_NO_SUB	8   /* Substitution is non-global and already done */
} VarPattern;

/*-
 *-----------------------------------------------------------------------
 * VarSubstitute --
 *	Perform a string-substitution on the given word, placing the
 *	result in the passed buffer.
 *
 * Results:
 *	TRUE if a space is needed before more characters are added.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
static Boolean
VarSubstitute (char *word, Boolean addSpace, Buffer buf, VarPattern *pattern)
{
    /* char    	  	*word;	    Word to modify */
    /* Boolean 	  	addSpace;   True if space should be added before
     *                              other characters */
    /* Buffer  	  	buf;	    Buffer for result */
    /* VarPattern	*pattern;   Pattern for substitution */

    int  	wordLen;    /* Length of word */
    char 	*cp;	    /* General pointer */

    wordLen = strlen(word);
    if ((pattern->flags & VAR_NO_SUB) == 0) {
	/*
	 * Still substituting -- break it down into simple anchored cases
	 * and if none of them fits, perform the general substitution case.
	 */
	if ((pattern->flags & VAR_MATCH_START) &&
	    (strncmp(word, pattern->lhs, pattern->leftLen) == 0)) {
		/*
		 * Anchored at start and beginning of word matches pattern
		 */
		if ((pattern->flags & VAR_MATCH_END) &&
		    (wordLen == pattern->leftLen)) {
			/*
			 * Also anchored at end and matches to the end (word
			 * is same length as pattern) add space and rhs only
			 * if rhs is non-null.
			 */
			if (pattern->rightLen != 0) {
			    if (addSpace) {
				Buf_AddByte(buf, (Byte)' ');
			    }
			    addSpace = TRUE;
			    Buf_AddBytes(buf, pattern->rightLen,
					 (Byte *)pattern->rhs);
			}
		} else if (pattern->flags & VAR_MATCH_END) {
		    /*
		     * Doesn't match to end -- copy word wholesale
		     */
		    goto nosub;
		} else {
		    /*
		     * Matches at start but need to copy in trailing characters
		     */
		    if ((pattern->rightLen + wordLen - pattern->leftLen) != 0){
			if (addSpace) {
			    Buf_AddByte(buf, (Byte)' ');
			}
			addSpace = TRUE;
		    }
		    Buf_AddBytes(buf, pattern->rightLen, (Byte *)pattern->rhs);
		    Buf_AddBytes(buf, wordLen - pattern->leftLen,
				 (Byte *)(word + pattern->leftLen));
		}
	} else if (pattern->flags & VAR_MATCH_START) {
	    /*
	     * Had to match at start of word and didn't -- copy whole word.
	     */
	    goto nosub;
	} else if (pattern->flags & VAR_MATCH_END) {
	    /*
	     * Anchored at end, Find only place match could occur (leftLen
	     * characters from the end of the word) and see if it does. Note
	     * that because the $ will be left at the end of the lhs, we have
	     * to use strncmp.
	     */
	    cp = word + (wordLen - pattern->leftLen);
	    if ((cp >= word) &&
		(strncmp(cp, pattern->lhs, pattern->leftLen) == 0)) {
		/*
		 * Match found. If we will place characters in the buffer,
		 * add a space before hand as indicated by addSpace, then
		 * stuff in the initial, unmatched part of the word followed
		 * by the right-hand-side.
		 */
		if (((cp - word) + pattern->rightLen) != 0) {
		    if (addSpace) {
			Buf_AddByte(buf, (Byte)' ');
		    }
		    addSpace = TRUE;
		}
		Buf_AddBytes(buf, (int)(cp-word), (Byte *)word);
		Buf_AddBytes(buf, pattern->rightLen, (Byte *)pattern->rhs);
	    } else {
		/*
		 * Had to match at end and didn't. Copy entire word.
		 */
		goto nosub;
	    }
	} else {
	    /*
	     * Pattern is unanchored: search for the pattern in the word using
	     * String_FindSubstring, copying unmatched portions and the
	     * right-hand-side for each match found, handling non-global
	     * subsititutions correctly, etc. When the loop is done, any
	     * remaining part of the word (word and wordLen are adjusted
	     * accordingly through the loop) is copied straight into the
	     * buffer.
	     * addSpace is set FALSE as soon as a space is added to the
	     * buffer.
	     */
	    Boolean done;
	    int origSize;

	    done = FALSE;
	    origSize = Buf_Size(buf);
	    while (!done) {
		cp = Str_FindSubstring(word, pattern->lhs);
		if (cp != (char *)NULL) {
		    if (addSpace && (((cp - word) + pattern->rightLen) != 0)){
			Buf_AddByte(buf, (Byte)' ');
			addSpace = FALSE;
		    }
		    Buf_AddBytes(buf, (int)(cp-word), (Byte *)word);
		    Buf_AddBytes(buf, pattern->rightLen, (Byte *)pattern->rhs);
		    wordLen -= (cp - word) + pattern->leftLen;
		    word = cp + pattern->leftLen;
		    if (wordLen == 0) {
			done = TRUE;
		    }
		    if ((pattern->flags & VAR_SUB_GLOBAL) == 0) {
			done = TRUE;
			pattern->flags |= VAR_NO_SUB;
		    }
		} else {
		    done = TRUE;
		}
	    }
	    if (wordLen != 0) {
		if (addSpace) {
		    Buf_AddByte(buf, (Byte)' ');
		}
		Buf_AddBytes(buf, wordLen, (Byte *)word);
	    }
	    /*
	     * If added characters to the buffer, need to add a space
	     * before we add any more. If we didn't add any, just return
	     * the previous value of addSpace.
	     */
	    return ((Buf_Size(buf) != origSize) || addSpace);
	}
	/*
	 * Common code for anchored substitutions: if performed a substitution
	 * and it's not supposed to be global, mark the pattern as requiring
	 * no more substitutions. addSpace was set TRUE if characters were
	 * added to the buffer.
	 */
	if ((pattern->flags & VAR_SUB_GLOBAL) == 0) {
	    pattern->flags |= VAR_NO_SUB;
	}
	return (addSpace);
    }
 nosub:
    if (addSpace) {
	Buf_AddByte(buf, (Byte)' ');
    }
    Buf_AddBytes(buf, wordLen, (Byte *)word);
    return(TRUE);
}

/*-
 *-----------------------------------------------------------------------
 * VarModify --
 *	Modify each of the words of the passed string using the given
 *	function. Used to implement all modifiers.
 *
 * Results:
 *	A string of all the words modified appropriately.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
static char *
VarModify (char *str, Boolean (*modProc)(char*,Boolean,Buffer,ClientData), ClientData datum)
{
    /* char    	  *str;	    	  String whose words should be trimmed */
    /* Boolean    (*modProc)();   Function to use to modify them */
    /* ClientData datum;    	  Datum to pass it */

    Buffer  	  buf;	    	    /* Buffer for the new string */
    char *cp;	    	    /* Pointer to end of current word */
    char    	  endc;	    	    /* Character that ended the word */
    Boolean 	  addSpace; 	    /* TRUE if need to add a space to the
				     * buffer before adding the trimmed
				     * word */
    
    buf = Buf_Init (0);
    cp = str;
    addSpace = FALSE;
    
    while (1) {
	/*
	 * Skip to next word and place cp at its end.
	 */
	while (isspace (*str)) {
	    str++;
	}
	for (cp = str; *cp != '\0' && !isspace (*cp); cp++) {
	    /* void */ ;
	}
	if (cp == str) {
	    /*
	     * If we didn't go anywhere, we must be done!
	     */
	    Buf_AddByte (buf, '\0');
	    str = (char *)Buf_GetAll (buf, (int *)NULL);
	    Buf_Destroy (buf, FALSE);
	    return (str);
	}
	/*
	 * Nuke terminating character, but save it in endc b/c if str was
	 * some variable's value, it would not be good to screw it
	 * over...
	 */
	endc = *cp;
	*cp = '\0';

	addSpace = (* modProc) (str, addSpace, buf, datum);

	if (endc) {
	    *cp++ = endc;
	}
	str = cp;
    }
}

/*-
 *-----------------------------------------------------------------------
 * Var_Parse --
 *	Given the start of a variable invocation, extract the variable
 *	name and find its value, then modify it according to the
 *	specification.
 *
 * Results:
 *	The (possibly-modified) value of the variable or var_Error if the
 *	specification is invalid. The length of the specification is
 *	placed in *lengthPtr (for invalid specifications, this is just
 *	2...?).
 *	A Boolean in *freePtr telling whether the returned string should
 *	be freed by the caller.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
char *
Var_Parse (char *str, GNode *ctxt, Boolean err, int *lengthPtr, Boolean *freePtr)
{
    /* char    	  *str;	    	The string to parse */
    /* GNode   	  *ctxt;    	The context for the variable */
    /* Boolean 	  err;    	TRUE if undefined variables are an error */
    /* int	  *lengthPtr;	OUT: The length of the specification */
    /* Boolean 	  *freePtr; 	OUT: TRUE if caller should free result */

    char   *tstr;    	/* Pointer into str */
    Var	   *v;	    	/* Variable in invocation */
    char   *cp;    	/* Secondary pointer into str (place marker
				 * for tstr) */
    Boolean 	    haveModifier;/* TRUE if have modifiers for the variable */
    char   endc;    	/* Ending character when variable in parens
				 * or braces */
    char    	    *start;
    Boolean 	    dynamic;	/* TRUE if the variable is local and we're
				 * expanding it in a non-local context. This
				 * is done to support dynamic sources. The
				 * result is just the invocation, unaltered */
    
    *freePtr = FALSE;
    dynamic = FALSE;
    start = str;
    
    if (str[1] != '(' && str[1] != '{') {
	/*
	 * If it's not bounded by braces of some sort, life is much simpler.
	 * We just need to check for the first character and return the
	 * value if it exists.
	 */
	char	  name[2];

	name[0] = str[1];
	name[1] = '\0';

	v = VarFind (name, ctxt, FIND_ENV | FIND_GLOBAL | FIND_CMD);
	if (v == (Var *)NIL) {
	    /*
	     * Error
	     */
	    *lengthPtr = 2;
	    return (err ? var_Error : varNoError);
	} else {
	    haveModifier = FALSE;
	    tstr = &str[1];
	    endc = str[1];
	}
    } else {
	endc = str[1] == '(' ? ')' : '}';

	/*
	 * Skip to the end character or a colon, whichever comes first.
	 */
	for (tstr = str + 2;
	     *tstr != '\0' && *tstr != endc && *tstr != ':';
	     tstr++)
	{
	    continue;
	}
	if (*tstr == ':') {
	    haveModifier = TRUE;
	} else if (*tstr != '\0') {
	    haveModifier = FALSE;
	} else {
	    /*
	     * If we never did find the end character, return NULL
	     * right now, setting the length to be the distance to
	     * the end of the string, since that's what make does.
	     */
	    *lengthPtr = tstr - str;
	    return (var_Error);
	}
	*tstr = '\0';
 	
	v = VarFind (str + 2, ctxt, FIND_ENV | FIND_GLOBAL | FIND_CMD);
	if ((v == (Var *)NIL) && (ctxt != VAR_CMD) && (ctxt != VAR_GLOBAL) &&
	    ((tstr-str) == 4) && (str[3] == 'F' || str[3] == 'D'))
	{
	    /*
	     * Check for bogus D and F forms of local variables since we're
	     * in a local context and the name is the right length.
	     */
	    switch(str[2]) {
		case '@':
		case '%':
		case '*':
		case '<':
		case '?':
		{
		    char    vname[2];
		    char    *val;

		    /*
		     * Well, it's local -- go look for it.
		     */
		    vname[0] = str[2];
		    vname[1] = '\0';
		    v = VarFind(vname, ctxt, 0);
		    
		    if (v != (Var *)NIL) {
			/*
			 * No need for nested expansion or anything, as we're
			 * the only one who sets these things and we sure don't
			 * but nested invocations in them...
			 */
			val = (char *)Buf_GetAll(v->val, (int *)NULL);
			
			if (str[3] == 'D') {
			    val = VarModify(val, (Boolean (*)(char*,Boolean,Buffer,ClientData))VarHead, (ClientData)0);
			} else {
			    val = VarModify(val, (Boolean (*)(char*,Boolean,Buffer,ClientData))VarTail, (ClientData)0);
			}
			/*
			 * Resulting string is dynamically allocated, so
			 * tell caller to free it.
			 */
			*freePtr = TRUE;
			*lengthPtr = tstr-start+1;
			*tstr = endc;
			return(val);
		    }
		    break;
		}
	    }
	}
			    
	if (v == (Var *)NIL) {
	    if ((((tstr-str) == 3) ||
		 ((((tstr-str) == 4) && (str[3] == 'F' ||
					 str[3] == 'D')))) &&
		((ctxt == VAR_CMD) || (ctxt == VAR_GLOBAL)))
	    {
		/*
		 * If substituting a local variable in a non-local context,
		 * assume it's for dynamic source stuff. We have to handle
		 * this specially and return the longhand for the variable
		 * with the dollar sign escaped so it makes it back to the
		 * caller. Only four of the local variables are treated
		 * specially as they are the only four that will be set
		 * when dynamic sources are expanded.
		 */
		switch (str[2]) {
		    case '@':
		    case '%':
		    case '*':
		    case '!':
			dynamic = TRUE;
			break;
		}
	    } else if (((tstr-str) > 4) && (str[2] == '.') &&
		       isupper(str[3]) &&
		       ((ctxt == VAR_CMD) || (ctxt == VAR_GLOBAL)))
	    {
		int	len;
		
		len = (tstr-str) - 3;
		if ((strncmp(str+2, ".TARGET", len) == 0) ||
		    (strncmp(str+2, ".ARCHIVE", len) == 0) ||
		    (strncmp(str+2, ".PREFIX", len) == 0) ||
		    (strncmp(str+2, ".MEMBER", len) == 0))
		{
		    dynamic = TRUE;
		}
	    }
	    
	    if (!haveModifier) {
		/*
		 * No modifiers -- have specification length so we can return
		 * now.
		 */
		*lengthPtr = tstr - start + 1;
		*tstr = endc;
		if (dynamic) {
		    str = (char *)emalloc(*lengthPtr + 1);
		    strncpy(str, start, *lengthPtr);
		    str[*lengthPtr] = '\0';
		    *freePtr = TRUE;
		    return(str);
		} else {
		    return (err ? var_Error : varNoError);
		}
	    } else {
		/*
		 * Still need to get to the end of the variable specification,
		 * so kludge up a Var structure for the modifications
		 */
		v = (Var *) emalloc(sizeof(Var));
		v->name = &str[1];
		v->val = Buf_Init(1);
		v->flags = VAR_JUNK;
	    }
	}
    }

    if (v->flags & VAR_IN_USE) {
	Fatal(catgets(catd, MS_MAKE, VARRECURSIVE, "Variable %s is recursive."),
	      v->name);
	/*NOTREACHED*/
    } else {
	v->flags |= VAR_IN_USE;
    }
    /*
     * Before doing any modification, we have to make sure the value
     * has been fully expanded. If it looks like recursion might be
     * necessary (there's a dollar sign somewhere in the variable's value)
     * we just call Var_Subst to do any other substitutions that are
     * necessary. Note that the value returned by Var_Subst will have
     * been dynamically-allocated, so it will need freeing when we
     * return.
     */
    str = (char *)Buf_GetAll(v->val, (int *)NULL);
    if (index (str, '$') != (char *)NULL) {
	str = Var_Subst(str, ctxt, err);
	*freePtr = TRUE;
    }
    
    v->flags &= ~VAR_IN_USE;
    
    /*
     * Now we need to apply any modifiers the user wants applied.
     * These are:
     *  	  :M<pattern>	words which match the given <pattern>.
     *  	  	    	<pattern> is of the standard file
     *  	  	    	wildcarding form.
     *  	  :S<d><pat1><d><pat2><d>[g]
     *  	  	    	Substitute <pat2> for <pat1> in the value
     *  	  :H	    	Substitute the head of each word
     *  	  :T	    	Substitute the tail of each word
     *  	  :E	    	Substitute the extension (minus '.') of
     *  	  	    	each word
     *  	  :R	    	Substitute the root of each word
     *  	  	    	(pathname minus the suffix).
     *	    	  :lhs=rhs  	Like :S, but the rhs goes to the end of
     *	    	    	    	the invocation.
     */
    if ((str != (char *)NULL) && haveModifier) {
	/*
	 * Skip initial colon while putting it back.
	 */
	*tstr++ = ':';
	while (*tstr != endc) {
	    char	*newStr;    /* New value to return */
	    char	termc;	    /* Character which terminated scan */
	    VarPattern	pattern;
	    Boolean	eqFound;

	    pattern.flags = 0;
	    eqFound = FALSE;
	    /*
	     * First we make a pass through the string trying
	     * to verify it is a SYSV-make-style translation:
	     * it must be: <string1>=<string2>)
	     */
	    for (cp = tstr; *cp != '\0' && *cp != endc; cp++) {
		if (*cp == '=') {
		    eqFound = TRUE;
		    /* continue looking for endc */
		}
	    }
	    if (*cp == endc && eqFound) {

		/*
		 * Now we break this sucker into the lhs and
		 * rhs. We must null terminate them of course.
		 */
		for (cp = tstr; *cp != '='; cp++) {
		    ;
		}
		pattern.lhs = tstr;
		pattern.leftLen = cp - tstr;
		*cp++ = '\0';

		pattern.rhs = cp;
		while (*cp != endc) {
		    cp++;
		}
		pattern.rightLen = cp - pattern.rhs;
		*cp = '\0';

		/*
		 * SYSV modifications happen through the whole
		 * string. Note the pattern is anchored at the end.
		 */
		pattern.flags |= VAR_SUB_GLOBAL|VAR_MATCH_END;

		newStr = VarModify(str, (Boolean (*)(char*,Boolean,Buffer,ClientData))VarSubstitute,
				   (ClientData)&pattern);

		/*
		 * Restore the nulled characters
		 */
		pattern.lhs[pattern.leftLen] = '=';
		pattern.rhs[pattern.rightLen] = endc;
		termc = endc;
	    } else {
		Error (catgets(catd, MS_MAKE, NOEQUALSIGN,
		       "Equal sign not found in macro substitution\n"));
		for (cp = tstr+1;
		     *cp != ':' && *cp != endc && *cp != '\0';
		     cp++) {
			 ;
		}
		termc = *cp;
		newStr = var_Error;
	    }
	    if (DEBUG(VAR)) {
		printf(catgets(catd, MS_DEBUG, VAR001, "Result is \"%s\")\n"),
		       newStr);
	    }
	    
	    if (*freePtr) {
		free (str);
	    }
	    str = newStr;
	    if (str != var_Error) {
		*freePtr = TRUE;
	    } else {
		*freePtr = FALSE;
	    }
	    if (termc == '\0') {
		Error(catgets(catd, MS_MAKE, VARSPECERR,
		      "Unclosed variable specification for %s"), v->name);
	    } else if (termc == ':') {
		*cp++ = termc;
	    } else {
		*cp = termc;
	    }
	    tstr = cp;
	}
	*lengthPtr = tstr - start + 1;
    } else {
	*lengthPtr = tstr - start + 1;
	*tstr = endc;
    }
    
    if (v->flags & VAR_FROM_ENV) {
	Boolean	  destroy = FALSE;
	
	if (str != (char *)Buf_GetAll(v->val, (int *)NULL)) {
	    destroy = TRUE;
	} else {
	    /*
	     * Returning the value unmodified, so tell the caller to free
	     * the thing.
	     */
	    *freePtr = TRUE;
	}
	Buf_Destroy(v->val, destroy);
	free((Address)v);
    } else if (v->flags & VAR_JUNK) {
	/*
	 * Perform any free'ing needed and set *freePtr to FALSE so the caller
	 * doesn't try to free a static pointer.
	 */
	if (*freePtr) {
	    free(str);
	}
	*freePtr = FALSE;
	free((Address)v);
	if (dynamic) {
	    str = (char *)emalloc(*lengthPtr + 1);
	    strncpy(str, start, *lengthPtr);
	    str[*lengthPtr] = '\0';
	    *freePtr = TRUE;
	} else {
	    str = var_Error;
	}
    }
    return (str);
}

/*-
 *-----------------------------------------------------------------------
 * Var_Subst  --
 *	Substitute for all variables in the given string in the given context
 *	If undefErr is TRUE, Parse_Error will be called when an undefined
 *	variable is encountered.
 *
 * Results:
 *	The resulting string.
 *
 * Side Effects:
 *	None. The old string must be freed by the caller
 *-----------------------------------------------------------------------
 */
char *
Var_Subst (char *str, GNode *ctxt, Boolean undefErr)
{
    /* char       *str;	    	    the string in which to substitute */
    /* GNode      *ctxt;	    the context wherein to find variables */
    /* Boolean 	  undefErr; 	    TRUE if undefineds are an error */

    Buffer  	  buf;	    	    /* Buffer for forming things */
    char    	  *val;		    /* Value to substitute for a variable */
    int	    	  length;   	    /* Length of the variable invocation */
    Boolean 	  doFree;   	    /* Set true if val should be freed */
    static Boolean errorReported;   /* Set true if an error has already
				     * been reported to prevent a plethora
				     * of messages when recursing */

    buf = Buf_Init (BSIZE);
    errorReported = FALSE;

    while (*str) {
	if ((*str == '$') && (str[1] == '$')) {
	    /*
	     * A dollar sign may be escaped either with another dollar sign.
	     * In such a case, we skip over the escape character and store the
	     * dollar sign into the buffer directly.
	     */
	    str++;
	    Buf_AddByte(buf, (Byte)*str);
	    str++;
	} else if (*str != '$') {
	    /*
	     * Skip as many characters as possible -- either to the end of
	     * the string or to the next dollar sign (variable invocation).
	     */
	    char  *cp;

	    for (cp = str++; *str != '$' && *str != '\0'; str++) {
		;
	    }
	    Buf_AddBytes(buf, (int)(str-cp), (Byte *)cp);
	} else {
	    val = Var_Parse (str, ctxt, undefErr, &length, &doFree);

	    /*
	     * When we come down here, val should either point to the
	     * value of this variable, suitably modified, or be NULL.
	     * Length should be the total length of the potential
	     * variable invocation (from $ to end character...)
	     */
	    if (val == var_Error || val == varNoError) {
		str += length;
	    } else {
		/*
		 * We've now got a variable structure to store in. But first,
		 * advance the string pointer.
		 */
		str += length;
		
		/*
		 * Copy all the characters from the variable value straight
		 * into the new string.
		 */
		Buf_AddBytes (buf, strlen (val), (Byte *)val);
		if (doFree) {
		    free ((Address)val);
		}
	    }
	}
    }
	
    Buf_AddByte (buf, '\0');
    str = (char *)Buf_GetAll (buf, (int *)NULL);
    Buf_Destroy (buf, FALSE);
    return (str);
}

/*-
 *-----------------------------------------------------------------------
 * Var_Init --
 *	Initialize the module
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	The VAR_CMD and VAR_GLOBAL contexts are created 
 *-----------------------------------------------------------------------
 */
void
Var_Init (void)
{
    VAR_GLOBAL = Targ_NewGN ("Global");
    VAR_CMD = Targ_NewGN ("Command");

}

/****************** PRINT DEBUGGING INFO *****************/
static int
VarPrintVar (Var *v, ...)
{
    /* Var            *v; */

    printf ("%-16s = %s\n", v->name, Buf_GetAll(v->val, (int *)NULL));
    return (0);
}

/*-
 *-----------------------------------------------------------------------
 * Var_Dump --
 *	print all variables in a context
 *-----------------------------------------------------------------------
 */
void
Var_Dump (GNode *ctxt)
{
    /* GNode          *ctxt; */

    Lst_ForEach (ctxt->context, (int (*)(void*,void*))VarPrintVar, (ClientData)0);
}
