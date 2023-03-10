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
static char	*sccsid = "@(#)$RCSfile: errorsubr.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/11/20 17:01:34 $";
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint

#endif /* not lint */

#include <stdio.h>
#include <ctype.h>
#include "error.h"
/*
 *	Arrayify a list of rules
 */
arrayify(e_length, e_array, header)
	int	*e_length;
	Eptr	**e_array;
	Eptr	header;
{
	reg	Eptr	errorp;
	reg	Eptr	*array;
	reg	int	listlength;
	reg	int	listindex;

	for (errorp = header, listlength = 0;
	     errorp; errorp = errorp->error_next, listlength++)
		continue;
	array = (Eptr*)Calloc(listlength+1, sizeof (Eptr));
	for(listindex = 0, errorp = header;
	    listindex < listlength;
	    listindex++, errorp = errorp->error_next){
		array[listindex] = errorp;
		errorp->error_position = listindex;
	}
	array[listindex] = (Eptr)0;
	*e_length = listlength;
	*e_array = array;
}

/*VARARGS1*/
error(msg, a1, a2, a3)
	char	*msg;
{
	fprintf(stderr, "Error: ");
	fprintf(stderr, msg, a1, a2, a3);
	fprintf(stderr, "\n");
	fflush(stdout);
	fflush(stderr);
	exit(6);
}
/*ARGSUSED*/
char *Calloc(nelements, size)
	int	nelements;
	int	size;
{
	char	*back;
	if ( (back = (char *)calloc(nelements, size)) == (char *)NULL){
		error("Ran out of memory.\n");
		exit(1);
	}
	return(back);
}

char *strsave(instring)
	char	*instring;
{
	char	*outstring;
	(void)strcpy(outstring = (char *)Calloc(1, strlen(instring) + 1),
		instring);
	return(outstring);
}
/*
 *	find the position of a given character in a string
 *		(one based)
 */
int position(string, ch)
	reg	char	*string;
	reg	char	ch;
{
	reg	int	i;
	if (string)
	for (i=1; *string; string++, i++){
		if (*string == ch)
			return(i);
	}
	return(-1);
}
/*
 *	clobber the first occurance of ch in string by the new character
 */
char *substitute(string, chold, chnew)
	char	*string;
	char	chold, chnew;
{
	reg	char	*cp = string;

	if (cp)
	while (*cp){
		if (*cp == chold){
			*cp = chnew;
			break;
		}
		cp++;
	}
	return(string);
}

char lastchar(string)
	char	*string;
{
	int	length;
	if (string == 0) return('\0');
	length = strlen(string);
	if (length >= 1)
		return(string[length-1]);
	else
		return('\0');
}

char firstchar(string)
	char	*string;
{
	if (string)
		return(string[0]);
	else
		return('\0');
}

char	next_lastchar(string)
	char	*string;
{
	int	length;
	if (string == 0) return('\0');
	length = strlen(string);
	if (length >= 2)
		return(string[length - 2]);
	else
		return('\0');
}

clob_last(string, newstuff)
	char	*string, newstuff;
{
	int	length = 0;
	if (string)
		length = strlen(string);
	if (length >= 1)
		string[length - 1] = newstuff;
}

/*
 *	parse a string that is the result of a format %s(%d)
 *	return TRUE if this is of the proper format
 */
boolean persperdexplode(string, r_perd, r_pers)
	char	*string;
	char	**r_perd, **r_pers;
{
	reg	char	*cp;
		int	length = 0;

	if (string)
		length = strlen(string);
	if (   (length >= 4)
	    && (string[length - 1] == ')' ) ){
		for (cp = &string[length - 2];
		     (isdigit(*cp)) && (*cp != '(');
		     --cp)
			continue;
		if (*cp == '('){
			string[length - 1] = '\0';	/* clobber the ) */
			*r_perd = strsave(cp+1);
			string[length - 1] = ')';
			*cp = '\0';			/* clobber the ( */
			*r_pers = strsave(string);
			*cp = '(';
			return(TRUE);
		} 
	}
	return(FALSE);
}
/*
 *	parse a quoted string that is the result of a format \"%s\"(%d)
 *	return TRUE if this is of the proper format
 */
boolean qpersperdexplode(string, r_perd, r_pers)
	char	*string;
	char	**r_perd, **r_pers;
{
	reg	char	*cp;
		int	length = 0;

	if (string)
		length = strlen(string);
	if (   (length >= 4)
	    && (string[length - 1] == ')' ) ){
		for (cp = &string[length - 2];
		     (isdigit(*cp)) && (*cp != '(');
		     --cp)
			continue;
		if (*cp == '(' && *(cp - 1) == '"'){
			string[length - 1] = '\0';
			*r_perd = strsave(cp+1);
			string[length - 1] = ')';
			*(cp - 1) = '\0';		/* clobber the " */
			*r_pers = strsave(string + 1);
			*(cp - 1) = '"';
			return(TRUE);
		} 
	}
	return(FALSE);
}

static	char	cincomment[] = CINCOMMENT;
static	char	coutcomment[] = COUTCOMMENT;
static	char	fincomment[] = FINCOMMENT;
static	char	foutcomment[] = FOUTCOMMENT;
static	char	newline[] = NEWLINE;
static	char	piincomment[] = PIINCOMMENT;
static	char	pioutcomment[] = PIOUTCOMMENT;
static	char	lispincomment[] = LISPINCOMMENT;
static	char	riincomment[] = RIINCOMMENT;
static	char	rioutcomment[] = RIOUTCOMMENT;
static	char	troffincomment[] = TROFFINCOMMENT;
static	char	troffoutcomment[] = TROFFOUTCOMMENT;
static	char	mod2incomment[] = MOD2INCOMMENT;
static	char	mod2outcomment[] = MOD2OUTCOMMENT;

struct	lang_desc lang_table[] = {
	/*INUNKNOWN	0*/	"unknown", cincomment,	coutcomment,
	/*INCPP		1*/	"cpp",	cincomment,    coutcomment,
	/*INCC		2*/	"cc",	cincomment,    coutcomment,
	/*INAS		3*/	"as",	ASINCOMMENT,   newline,
	/*INLD		4*/	"ld",	cincomment,    coutcomment,
	/*INLINT	5*/	"lint",	cincomment,    coutcomment,
	/*INF77		6*/	"f77",	fincomment,    foutcomment,
	/*INPI		7*/	"pi",	piincomment,   pioutcomment,
	/*INPC		8*/	"pc",	piincomment,   pioutcomment,
	/*INFRANZ	9*/	"franz",lispincomment, newline,
	/*INLISP	10*/	"lisp",	lispincomment, newline,
	/*INVAXIMA	11*/	"vaxima",lispincomment,newline,
	/*INRATFOR	12*/	"ratfor",fincomment,   foutcomment,
	/*INLEX		13*/	"lex",	cincomment,    coutcomment,
	/*INYACC	14*/	"yacc",	cincomment,    coutcomment,
	/*INAPL		15*/	"apl",	".lm",	       newline,
	/*INMAKE	16*/	"make",	ASINCOMMENT,   newline,
	/*INRI		17*/	"ri",	riincomment,   rioutcomment,
	/*INTROFF	18*/	"troff",troffincomment,troffoutcomment,
	/*INMOD2	19*/	"mod2",	mod2incomment, mod2outcomment,
				0,	0,	     0
};

printerrors(look_at_subclass, errorc, errorv)
	boolean	look_at_subclass;
	int	errorc;
	Eptr	errorv[];
{
	reg	int	i;
	reg	Eptr	errorp;

	for (errorp = errorv[i = 0]; i < errorc; errorp = errorv[++i]){
		if (errorp->error_e_class == C_IGNORE)
			continue;
		if (look_at_subclass && errorp->error_s_class == C_DUPL)
			continue;
		printf("Error %d, (%s error) [%s], text = \"",
			i,
			class_table[errorp->error_e_class],
			lang_table[errorp->error_language].lang_name);
		wordvprint(stdout,errorp->error_lgtext,errorp->error_text);
		printf("\"\n");
	}
}

wordvprint(fyle, wordc, wordv)
	FILE	*fyle;
	int	wordc;
	char	*wordv[];
{
	int	i;
	char *sep = "";

	for(i = 0; i < wordc; i++)
		if (wordv[i]) {
			fprintf(fyle, "%s%s",sep,wordv[i]);
			sep = " ";
		}
}

/*
 *	Given a string, parse it into a number of words, and build
 *	a wordc wordv combination pointing into it.
 */
wordvbuild(string, r_wordc, r_wordv)
	char	*string;
	int	*r_wordc;
	char	***r_wordv;
{
	reg	char 	*cp;
		char	*saltedbuffer;
		char	**wordv;
		int	wordcount;
		int	wordindex;

	saltedbuffer = strsave(string);
	for (wordcount = 0, cp = saltedbuffer; *cp; wordcount++){
		while (*cp  && isspace(*cp))
			cp++;
		if (*cp == 0)
			break;
		while (*cp && !isspace(*cp))  	/* 0001 */
			cp++;
	}
	wordv = (char **)Calloc(wordcount + 1, sizeof (char *));
	for (cp=saltedbuffer,wordindex=0; wordcount; wordindex++,--wordcount){
		while (*cp && isspace(*cp))
			cp++;
		if (*cp == 0)
			break;
		wordv[wordindex] = cp;
		while(*cp && !isspace(*cp))	/* 0001 */
			cp++;
		*cp++ = '\0';
	}
	if (wordcount != 0)
		error("Initial miscount of the number of words in a line\n");
	wordv[wordindex] = (char *)0;
#ifdef FULLDEBUG
	for (wordcount = 0; wordcount < wordindex; wordcount++)
		printf("Word %d = \"%s\"\n", wordcount, wordv[wordcount]);
	printf("\n");
#endif
	*r_wordc = wordindex;
	*r_wordv = wordv;
}
/*
 *	Compare two 0 based wordvectors
 */
int wordvcmp(wordv1, wordc, wordv2)
	char	**wordv1;
	int	wordc;
	char	**wordv2;
{
	reg	int i;
		int	back;
	for (i = 0; i < wordc; i++){
		if (wordv1[i] == 0 || wordv2[i] == 0)
				return(-1);
		if (back = strcmp(wordv1[i], wordv2[i])){
			return(back);
		}
	}
	return(0);	/* they are equal */
}
		
/*
 *	splice a 0 basedword vector onto the tail of a
 *	new wordv, allowing the first emptyhead slots to be empty
 */
char	**wordvsplice(emptyhead, wordc, wordv)
	int	emptyhead;
	int	wordc;
	char	**wordv;
{
	reg	char	**nwordv;
		int	nwordc = emptyhead + wordc;
	reg	int	i;

	nwordv = (char **)Calloc(nwordc, sizeof (char *));
	for (i = 0; i < emptyhead; i++)
		nwordv[i] = 0;
	for(i = emptyhead; i < nwordc; i++){
		nwordv[i] = wordv[i-emptyhead];
	}
	return(nwordv);
}
/*
 *	plural'ize and verb forms
 */
static	char	*S = "s";
static	char	*N = "";
char *plural(n)
	int	n;
{
	return( n > 1 ? S : N);
}
char *verbform(n)
	int	n;
{
	return( n > 1 ? N : S);
}

