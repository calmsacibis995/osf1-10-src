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
static char	*sccsid = "@(#)$RCSfile: macro.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:13:31 $";
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
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1990
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
#include "i386/rdb/ctype.h"
#include "i386/rdb/debug.h"

#define MAX_MACRO 32
#define AVERAGE_MACRO_SPACE 50

#define MAX_MACRO_SPACE (MAX_MACRO * AVERAGE_MACRO_SPACE)

/*
 * the following code includes "macros.h" multiple times after
 * redefining DEFINE so that it builds up part of the data
 * structure that we need
 */
struct symtab macrocmd[MAX_MACRO] = {
#define DEFINE(name,value) SYM( name, sizeof value - 1 )
#include "i386/rdb/macros.h"
#undef DEFINE
{0, 0 }
};

char macro_space[MAX_MACRO_SPACE] = { 
#define DEFINE(name,value) value
#include "i386/rdb/macros.h"
#undef DEFINE
"" };

char *macro_space_ptr = macro_space
#define DEFINE(name,value) + sizeof(value) - 1
#include "i386/rdb/macros.h"
#undef DEFINE
	;
char *macro_space_end = macro_space+MAX_MACRO_SPACE;
int macro_count = 0		/* the count of defined macros */
#define DEFINE(name,value) + 1
#include "i386/rdb/macros.h"
#undef DEFINE
	;

struct symtab *macro_free;	/* points to first free macro symtab slot */

#define LBRACE '{'	/* use macro so that vi's % command works inside code */
#define RBRACE '}'	/* use macro so that vi's % command works inside code */

#define MAX_MACRO_DEPTH 4

void macro_delete();


/*
 * initialize built-in macros. We can do just about everyting except
 * build the pointer in s->value with the compiler.
 * also look up "init" in the macro table and execute it if its there.
 */
static macro_init()
{
	struct symtab *s = macrocmd;
	int i;
	char *text = macro_space;
	int err_flag;
	static char *init[2] = { "init", 0 };	/* an init command */

	for (i=0; i<macro_count; ++i, ++s) /* ++s is ok here */
		{
		int len = s->value;
		s->value = (int) text;
		text += len;
		}
	if (text != macro_space_ptr)
		printf("macro_init: botch! %x %x\n", text, macro_space_ptr);
	if (lookup(init[0], &err_flag, macrocmd))
		macro_cmd(1, init, 1);	/* do debugger initializations */
	macro_free = s;			/* first free slot */
}
/*
 * execute the given command. First test to see if its a macro.
 * if not, then just call dump_cmd to execute it. If it is a macro
 * then we take each of the component strings, copy into our 
 * line buffer and expand the macro arguments. Then we parse it
 * and recursively call ourselves.
 * We preserve the state of lastdebugcmd over macro invocations.
 */
macro_cmd(argc,argv,depth)
int argc;
char **argv;
int depth;
{
	char line[MAX_LINE];
	char *nargv[MAX_ARG];
	int nargc;
	char *text;
	int len;
	extern int lastdebugcmd;	       /* default is to give help */
	extern int lastcmdargs;			/* no args specified */
	int lastcmd = lastdebugcmd, lastargs = lastcmdargs;
	static int inited = 0;
	int result = 0;

	if (!inited)
		++inited, macro_init();		/* do first time stuff */

	if (argc && *argv[0] == '#')
		return(0);		/* nothing to do */

	if (argc == 0 || macro_count == 0 ||
	    (argv[0][0] == '\\' && ++argv[0]) ||
	    (text = (char *) lookup(argv[0], &err_flag, macrocmd)) == 0)
		return(dump_cmd(argc,argv,depth));

	for (;*text;text += strlen(text) + 1)
	{
		if (debug_option&DEBUG_DEBUG)
			printf("macro %s generates %s\n",argv[0],text);
		macro_expand(text, line, argc, argv);
		if (debug_option&OPTION_EXPAND)
			printf("+%s\n",line);
		nargc = _parse(line, line, nargv, (char **) 0);
		if (debug_option&DEBUG_DEBUG)
			printf("macro %s parses into %d arguments\n",argv[0],nargc);
		if (depth >= MAX_MACRO_DEPTH)
			{
			printf("macros nested too deep\n");
			return(1);;
			}
		if (nargc == 1 && strcmp(nargv[0],"shift") == 0)
			{
			int i;
			if (argc <= 1)
				{
				printf("shift: no arguments left\n");
				++err_flag;
				continue;
				}
			for (i=1; i<argc; ++i)
				argv[i] = argv[i+1];	/* move down */
			--argc;
			}
		else
			if ((result = macro_cmd(nargc,nargv,depth+1)) != 0)
				break;	/* allow nested commands */
	}
	lastdebugcmd = lastcmd;
	lastcmdargs = lastargs;
	return(result);
}

/*
 * expand arguments in macro text
 *	$$	expands to $
 *	$n	expands to argument n 
 *	${n-def}	expands to argument n or to def
 *	$#	expands to number of arguments
 */
macro_expand(source, dest, argc, argv)
char *source, *dest, **argv;
int argc;
{
	char *arg;
	int n;
	int c;

	for (; *source; )
		{
		if (*source != '$')
			*dest++ = *source++;
		else if (*++source == '$')
			*dest++ = *source++;
		else if (*source == '#')
			{ /* we "know" that we can't have > 99 args */
			n = argc-1;
			if (n > 9)
				*dest++ = n/10 + '0';
			*dest++ = n%10 + '0';
			++source;
			}
		else if (*source == LBRACE)
			{
			char *temp;

			n = atoi(++source);
			while (isdigit(*source))
				++source;
			for (temp=source; *temp; ++temp)
				if (*temp == RBRACE)
					break;
			if (*temp == 0)
				{
				printf("%c missing\n",RBRACE);
				continue;
				}
			if (temp != source && *source != '-')
				printf("badly formed macro expansion\n");
			if (n < argc && (arg = argv[n]) != 0)
				while (*arg)
					*dest++ = *arg++;
			else
				if (*source++ == '-')
					while (source < temp)
						*dest++ = *source++;
			source = temp+1;
			}
		else if (isdigit(*source))
			{
			n = atoi(source);
			while (isdigit(*source))
				++source;
			if (n < argc)
				arg = argv[n];
			else
				arg = "";
			while (*arg)
				*dest++ = *arg++;
			}
		else if (*source == '*')
			{
			++source;
			for (n=1; n<argc; ++n)
				{
				arg = argv[n];
				while (*arg)
					*dest++ = *arg++;
				*dest++ = ' ';
				}
			}
		else
			*dest++ = '$';
		}
	*dest=0;
}


/*
 * define "name" as the specified macro. If it already exists we delete
 * it first. 
 * We always store the text at the end of the macro space area,
 * and the name at the end of the macro symbol table.
 */

void macro_define(name)
char *name;
{
	struct symtab *s;
	char *text;
	int len;
	char line[MAX_LINE];
	struct symtab *locate();

	if ((s = locate(name, &err_flag, macrocmd)) != 0)
		macro_delete(name);

	if (macro_count >= MAX_MACRO || macro_free >= macrocmd+MAX_MACRO)
		{
		printf("%s: more than %d macros\n",name,MAX_MACRO);
		return;
		}

	s = macro_free;

	for (text = macro_space_ptr;;)
		{
		printf("+ ");
		if (db_gets(line) == 0 || line[0]==0)
			break;
		len = strlen(line);
		if (text+len > macro_space_end)
			{
			printf("%s: more than %d bytes of macros\n",name,MAX_MACRO_SPACE);
			return;
			}
		strcpy(text, line);
		text += len + 1;
		}
	*text++ = 0;
	strcpy(s->symbol, name);
	s->value = (int) macro_space_ptr;
	macro_space_ptr = text;
	macro_count++;		/* now committed */
	macro_free = next_sym(s);
}


void macro_delete(name)
char *name;
{
	struct symtab *s, *p, *q;
	struct symtab *locate();
	int len;

	if ((s = locate(name, (int *) 0, macrocmd)) == 0)
		return;
	--macro_count;
	p = next_sym(s);	/* the next macro */
	macro_free -= p - s;	/* adjust free by size of s */
	len = macro_space_cleanup(s);
	for (; s < macro_free; )
		{
		q = next_sym(p);
		s->value = p->value-len;	/* adjust the data pointer */
		strcpy(s->symbol, p->symbol);	/* copy the symbol */
		s = next_sym(s);
		p = q;
		}
	bzero(macro_free, sizeof (struct symtab));	/* get rid of it for good! */
}

/*
 * when deleting a macro we move the following text up so that
 * new definitions can always be added after the current ones.
 * we zero the released space for ease of debugging.
 *	s	pointer to macro to be deleted
 *	len	length of the space for the given macro
 *	length	amount of macro text to move
 */
static macro_space_cleanup(s)
	struct symtab *s;
{
	int len = macro_length((char *)s->value);
	char *text;
	int length;		/* length to moved */

	text = (char *)s->value + len;		/* start of next macro text */
	length = macro_space_ptr - text;
	macro_space_ptr -= len;
	if (length)
		{
		bcopy(text, (char *)s->value, length);
		bzero(macro_space_ptr,len);	/* for completeness */
		}
	return(len);
}

/*
 * compute how many bytes are in a macro definition
 */
static int macro_length(text)
char *text;
{
	int len, length;
	for (length=0;*text;length += len)
		{
		len = strlen(text) + 1;
		text += len;
		}
	return(length+1);
}
		

void macro_list(name)
char *name;
{
	struct symtab *s;
	int i;

	if (name == 0)
		for (i=0, s = macrocmd; i<macro_count; ++i, s = next_sym(s))
			macro_display(s);
	else if ((s = locate(name, (int *) 0, macrocmd)) != 0)
		macro_display(s);
}

macro_display(s)
	struct symtab *s;
{
	char *text;

	printf("macro %s\n", s->symbol);
	for (text=(char *)s->value; text && *text; text += strlen(text) + 1)
		printf("%s\n",text);
	printf("\n");
}

macro_help(name)
char *name;
{
	struct symtab *s = macrocmd;
	int i;
	char *text;

	if (macro_count==0)
		return(0);
	if (name == 0)
		printf("Macros (must be exactly has given)\n");
	for (i=0; i<macro_count; ++i, s = next_sym(s))
		{
		if (name == 0)
			printf("%s%s", s->symbol, ((i+1)&07) ? "	" : "\n");
		else if (strcmp(name,s->symbol) == 0)
			{
			char *p = (char *) s->value;
			if (*p == '#')
				++p;
			printf("%s %s\n", s->symbol, p);
			break;
			}
		}
	if (name == 0)
		printf("\n");
	return(i < macro_count);
}

/*
 * push a line into the stack at the end of the macro string 
 * region. we copy and preceeding lines up, leaving space at 
 * the end for the new line.
 */
char *put_line(line)
char *line;
{
	int len = strlen(line)+1;
	char *end = macro_space_end-len;

	if (macro_space_ptr >= end)
		return(0);	/* failed */
	bcopy(macro_space_end, end,
		macro_space + MAX_MACRO_SPACE - macro_space_end);
	bcopy(line, macro_space + MAX_MACRO_SPACE - len, len);
	macro_space_end -= len;
	return(macro_space + MAX_MACRO_SPACE - len);
}

char *db_gets(line)
char *line;
{
	int len;
	char *gets();

	if (macro_space_end >= macro_space + MAX_MACRO_SPACE)
		return(gets(line));

	len = strlen(macro_space_end)+1;
	bcopy(macro_space_end, line, len);
	bzero(macro_space_end, len);
	macro_space_end += len;
	printf("%s\n",line);
	return(line);
}

edit_macro(name)
char *name;
{
	char *text;
	char line[MAX_LINE];

	if (name == 0)
		{
		printf("macro name not specified\n");
		return;
		}
	if ((text = (char *) lookup(name, (int *) 0, macrocmd)) == 0)
		return;
	printf("Use up-arrow to insert a new line\n");
	sprintf(line,"macro %s",name);
	db_edit(line);
	if (line[0] == 0)
		return;		/* cancel edit */
	put_line(line);
	for (;*text;)
		{
		int i;
		strcpy(line, text);
		i = db_edit(line);
		if (i&EDIT_UP)
			{
			if (i&EDIT_CHANGE)
				printf("up arrow - changes discarded\n");
			line[0] = 0;
			db_edit(line);
			put_line(line);
			continue;
			}
		put_line(line);
		text += strlen(text) + 1;
		if (line[0] == 0)
			break;
		}
	while (line[0])
		{
		line[0] = 0;
		db_edit(line);
		put_line(line);
		}
}
