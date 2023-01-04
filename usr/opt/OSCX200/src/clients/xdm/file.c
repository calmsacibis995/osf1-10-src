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
 * xdm - display manager daemon
 *
 * $XConsortium: file.c,v 1.15 91/02/13 19:13:21 rws Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * file.c
 */

# include	"dm.h"
# include	<ctype.h>

DisplayTypeMatch (d1, d2)
DisplayType	d1, d2;
{
	return d1.location == d2.location &&
	       d1.lifetime == d2.lifetime &&
	       d1.origin == d2.origin;
}

static void
freeArgs (args)
    char    **args;
{
    char    **a;

    for (a = args; *a; a++)
	free (*a);
    free ((char *) args);
}

static char **
splitIntoWords (s)
    char    *s;
{
    char    **args, **newargs;
    char    *wordStart;
    int	    nargs;

    args = 0;
    nargs = 0;
    while (*s)
    {
	while (*s && isspace (*s))
	    ++s;
	if (!*s || *s == '#')
	    break;
	wordStart = s;
	while (*s && *s != '#' && !isspace (*s))
	    ++s;
	if (!args)
	{
    	    args = (char **) malloc (2 * sizeof (char *));
    	    if (!args)
	    	return NULL;
	}
	else
	{
	    newargs = (char **) realloc ((char *) args,
					 (nargs+2)*sizeof (char *));
	    if (!newargs)
	    {
	    	freeArgs (args);
	    	return NULL;
	    }
	    args = newargs;
	}
	args[nargs] = malloc (s - wordStart + 1);
	if (!args[nargs])
	{
	    freeArgs (args);
	    return NULL;
	}
	strncpy (args[nargs], wordStart, s - wordStart);
	args[nargs][s-wordStart] = '\0';
	++nargs;
	args[nargs] = NULL;
    }
    return args;
}

static char **
copyArgs (args)
    char    **args;
{
    char    **a, **new, **n;

    for (a = args; *a; a++)
	/* SUPPRESS 530 */
	;
    new = (char **) malloc ((a - args + 1) * sizeof (char *));
    if (!new)
	return NULL;
    n = new;
    a = args;
    /* SUPPRESS 560 */
    while (*n++ = *a++)
	/* SUPPRESS 530 */
	;
    return new;
}

freeSomeArgs (args, n)
    char    **args;
    int	    n;
{
    char    **a;

    a = args;
    while (n--)
	free (*a++);
    free ((char *) args);
}

ParseDisplay (source, acceptableTypes, numAcceptable)
char		*source;
DisplayType	*acceptableTypes;
int		numAcceptable;
{
    char		**args, **argv, **a;
    char		*name, *class, *type;
    struct display	*d;
    int			usedDefault;
    DisplayType		displayType;

    args = splitIntoWords (source);
    if (!args)
	return;
    if (!args[0])
    {
	LogError ("Missing display name in servers file\n");
	freeArgs (args);
	return;
    }
    name = args[0];
    if (!args[1])
    {
	LogError ("Missing display type for %s\n", args[0]);
	freeArgs (args);
	return;
    }
    displayType = parseDisplayType (args[1], &usedDefault);
    class = NULL;
    type = args[1];
    argv = args + 2;
    /*
     * extended syntax; if the second argument doesn't
     * exactly match a legal display type and the third
     * argument does, use the second argument as the
     * display class string
     */
    if (usedDefault && args[2])
    {
	displayType = parseDisplayType (args[2], &usedDefault);
	if (!usedDefault)
	{
	    class = args[1];
	    type = args[2];
	    argv = args + 3;
	}
    }
    while (numAcceptable)
    {
	if (DisplayTypeMatch (*acceptableTypes, displayType))
	    break;
	--numAcceptable;
	++acceptableTypes;
    }
    if (!numAcceptable)
    {
	LogError ("Unacceptable display type %s for display %s\n",
		  type, name);
    }
    d = FindDisplayByName (name);
    if (d)
    {
	d->state = OldEntry;
	if (class && strcmp (d->class, class))
	{
	    char    *newclass;

	    newclass = malloc ((unsigned) (strlen (class) + 1));
	    if (newclass)
	    {
		free (d->class);
		strcpy (newclass, class);
		d->class = newclass;
	    }
	}
	Debug ("Found existing display:  %s %s %s", d->name, d->class, type);
	freeArgs (d->argv);
    }
    else
    {
	d = NewDisplay (name, class);
	Debug ("Found new display:  %s %s %s", d->name, d->class, type);
    }
    d->displayType = displayType;
    d->argv = copyArgs (argv);
    for (a = d->argv; a && *a; a++)
	Debug (" %s", *a);
    Debug ("\n");
    freeSomeArgs (args, argv - args);
}

static struct displayMatch {
	char		*name;
	DisplayType	type;
} displayTypes[] = {
	"local",		{ Local, Permanent, FromFile },
	"foreign",		{ Foreign, Permanent, FromFile },
	0,			{ Local, Permanent, FromFile },
};

DisplayType
parseDisplayType (string, usedDefault)
	char	*string;
	int	*usedDefault;
{
	struct displayMatch	*d;

	for (d = displayTypes; d->name; d++)
		if (!strcmp (d->name, string))
		{
			*usedDefault = 0;
			return d->type;
		}
	*usedDefault = 1;
	return d->type;
}

int ParseKeymaps(d, lang, default_keymap, lang_keymap)
    struct display	*d;
    unsigned int	lang;
    char		*default_keymap;
    char		*lang_keymap;

    {
    FILE	*keymaps_file;
    char	line[PATH_MAX + NAME_MAX];
    char	keymap_dir[PATH_MAX];
    char	fallback_keymap[NAME_MAX];
    char	**words;

    Debug("ParseKeymaps %s\n", d->keymaps);

    default_keymap[0] = '\0';
    lang_keymap[0] = '\0';
    keymap_dir[0] = '\0';
    fallback_keymap[0] = '\0';

    keymaps_file = fopen(d->keymaps, "r");
    if (keymaps_file == NULL)
	return FALSE;

    while (default_keymap[0] == '\0')
	{
	if (NULL == fgets(line, PATH_MAX + NAME_MAX, keymaps_file))
	    return FALSE;
	words = splitIntoWords(line);
	if (words != NULL)
	    {
	    strcpy(default_keymap, words[0]);
	    freeArgs(words);
	    }
	}
    while (keymap_dir[0] == '\0')
	{
	if (NULL == fgets(line, PATH_MAX + NAME_MAX, keymaps_file))
	    return FALSE;
	words = splitIntoWords(line);
	if (words != NULL)
	    {
	    strcpy(keymap_dir, words[0]);
	    freeArgs(words);
	    }
	}
    while (lang_keymap[0] == '\0')
	{
	if (NULL == fgets(line, PATH_MAX + NAME_MAX, keymaps_file))
	    break;
	words = splitIntoWords(line);
	if ((words != NULL) && (strtol(words[0], NULL, 16) == (long)0)
		&& (words[1] && words[2]))
	    strcpy(fallback_keymap, words[2]);
	/*
	 * A value of -1 for the lang is an override and always matches.
	 * Must come first in the table before the real languages.
	 */
	if ((words != NULL) && ((strtol(words[0], NULL, 16) == (long)lang)
			     || (strtol(words[0], NULL, 16) == (long)-1)))
	    {
	    if (words[1] && words[2])
		{
		strcpy(lang_keymap, keymap_dir);
		strcat(lang_keymap, words[2]);
		}
	    else if (words[1] && !words[2])
		{
		LogInfo("Null keymap specified for lang %x, no keymap default created.\n", lang);
		freeArgs(words);
		return FALSE;
		}
	    }
	if (words != NULL)
	    freeArgs(words);
	}
    if (lang_keymap[0] == '\0')
	{
	if (fallback_keymap[0] == '\0')
	    {
	    LogError("Can't find keymap for lang %x, no fallback specified.\n",
			lang);
	    return FALSE;
	    }
	strcpy(lang_keymap, keymap_dir);
	strcat(lang_keymap, fallback_keymap);
	LogInfo("Can't find keymap for lang %x, using fallback %s\n",
		  lang, fallback_keymap);
	}

    fclose(keymaps_file);

    return TRUE;
    }
