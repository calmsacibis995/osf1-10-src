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
/***********************************************************
Copyright 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* $XConsortium: mkfontdir.c,v 1.8 93/06/14 15:31:46 dpw Exp $ */

#include <X11/Xos.h>
#include <X11/Xfuncs.h>
#include <stdio.h>

#ifndef X_NOT_POSIX
#ifdef _POSIX_SOURCE
#include <limits.h>
#else
#define _POSIX_SOURCE
#include <limits.h>
#undef _POSIX_SOURCE
#endif
#endif
#ifndef PATH_MAX
#include <sys/param.h>
#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX 1024
#endif
#endif
#endif

#ifndef X_NOT_POSIX
#include <dirent.h>
#else
#ifdef SYSV
#include <dirent.h>
#else
#ifdef USG
#include <dirent.h>
#else
#include <sys/dir.h>
#ifndef dirent
#define dirent direct
#endif
#endif
#endif
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "fontfilest.h"

#include <errno.h>
extern int errno;

#define  XK_LATIN1
#include <X11/keysymdef.h>

extern void * FontFileSaveString();

char *progName;

static Bool
WriteFontTable(dirName, table)
    char	    *dirName;
    FontTablePtr    table;
{
    int		    i;
    FILE	    *file;
    char	    full_name[PATH_MAX];
    FontEntryPtr    entry;

    sprintf (full_name, "%s/%s", dirName, FontDirFile);

    /* remove old fonts.dir, in case it is a link */

    if (unlink(full_name) < 0 && errno != ENOENT)
    {
	fprintf(stderr, "%s: cannot unlink %s\n", progName, full_name);
	return FALSE;
    }

    file = fopen (full_name, "w");
    if (!file)
    {
	fprintf (stderr, "%s: can't create directory %s\n", progName, full_name);
	return FALSE;
    }
    fprintf(file, "%d\n", table->used);
    for (i = 0; i < table->used; i++) {
	entry = &table->entries[i];
	fprintf (file, "%s %s\n", entry->u.bitmap.fileName, entry->name.name);
    }
    fclose (file);
    return TRUE;
}

static char *
NameForAtomOrNone (a)
    Atom    a;
{
    char    *name;

    name = NameForAtom (a);
    if (!name)
	return "";
    return name;
}

static Bool
GetFontName(file_name, font_name)
    char    *file_name;
    char    *font_name;
{
    FontInfoRec	info;
    int		i;
    char	*atom_name;
    char	*atom_value;

    if (BitmapGetInfoBitmap ((FontPathElementPtr) 0, &info, (FontEntryPtr) 0, file_name) != Successful)
	return FALSE;

    for (i = 0; i < info.nprops; i++) 
    {
	atom_name = (char *) NameForAtomOrNone (info.props[i].name);
	if (atom_name && strcmp (atom_name, "FONT") == 0 && info.isStringProp[i])
	{
	    atom_value = NameForAtomOrNone (info.props[i].value);
	    if (strlen (atom_value) == 0)
		return FALSE;
	    strcpy (font_name, atom_value);
	    return TRUE;
	}
    }
    return FALSE;
}

static char *
FontNameExists (table, font_name)
    FontTablePtr    table;
    char	    *font_name;
{
    FontNameRec	    name;
    FontEntryPtr    entry;

    name.name = font_name;
    name.length = strlen (font_name);
    name.ndashes = FontFileCountDashes (name.name, name.length);
    entry = FontFileFindNameInDir (table, &name);
    if (entry)
	return entry->u.bitmap.fileName;
    return 0;
}

AddEntry (table, fontName, fileName)
    FontTablePtr    table;
    char	    *fontName, *fileName;
{
    FontEntryRec    prototype;

    prototype.name.name = fontName;
    prototype.name.length = strlen (fontName);
    prototype.name.ndashes = FontFileCountDashes (fontName, prototype.name.length);
    prototype.type = FONT_ENTRY_BITMAP;
    prototype.u.bitmap.fileName = FontFileSaveString (fileName);
    return FontFileAddEntry (table, &prototype) != 0;
}

static Bool
ProcessFile (dirName, fileName, table)
    char		*dirName;
    char		*fileName;
    FontTablePtr	table;
{
    char	    font_name[PATH_MAX];
    char	    full_name[PATH_MAX];
    char	    *existing;

    strcpy (full_name, dirName);
    if (dirName[strlen(dirName) - 1] != '/')
	strcat (full_name, "/");
    strcat (full_name, fileName);

    if (!GetFontName (full_name, font_name))
	return FALSE;

    CopyISOLatin1Lowered (font_name, font_name, strlen(font_name));

    if (existing = FontNameExists (table, font_name))
    {
	fprintf (stderr, "%s: Duplicate font names %s\n", progName, font_name);
	fprintf (stderr, "\t%s %s\n", existing, fileName);
	return FALSE;
    }
    return AddEntry (table, font_name, fileName);
}

static
Estrip(ext,name)
    char	*ext;
    char	*name;
{
    name[strlen(name) - strlen(ext)] = '\0';
}

/***====================================================================***/

typedef struct _nameBucket {
    struct _nameBucket	*next;
    char		*name;
    FontRendererPtr	renderer;
} NameBucketRec, *NameBucketPtr;
    
#define New(type,count)	((type *) malloc (count * sizeof (type)))

#define HASH_SIZE   1024

char *
MakeName(name)
    char	*name;
{
    char    *new;

    new = New(char, strlen(name) + 1);
    strcpy (new,name);
    return new;
}

int
Hash(name)
    char	*name;
{
    int	    i;
    char    c;

    i = 0;
    while (c = *name++)
	i = (i << 1) ^ c;
    return i & (HASH_SIZE - 1);
}

static Bool
LoadDirectory (dirName, table)
    char	    *dirName;
    FontTablePtr    table;
{
    DIR			*dirp;
    struct dirent	*file;
    FontRendererPtr	renderer;
    char		fileName[PATH_MAX];
    int			hash;
    char		*extension;
    NameBucketPtr	*hashTable, bucket, *prev, next;
    Bool		status;
    
    if ((dirp = opendir (dirName)) == NULL)
	return FALSE;
    hashTable = New (NameBucketPtr, HASH_SIZE);
    bzero((char *)hashTable, HASH_SIZE * sizeof(NameBucketPtr));
    while ((file = readdir (dirp)) != NULL) {
	renderer = FontFileMatchRenderer (file->d_name);
	if (renderer)
	{
	    extension = renderer->fileSuffix;
	    Estrip (extension, file->d_name);
	    hash = Hash (file->d_name);
	    prev = &hashTable[hash];
	    bucket = *prev;
	    while (bucket && strcmp (bucket->name, file->d_name))
	    {
		prev = &bucket->next;
		bucket = *prev;
	    }
	    if (bucket)
	    {
		if (bucket->renderer->number > renderer->number)
		    bucket->renderer = renderer;
	    }
	    else
	    {
		bucket = New (NameBucketRec, 1);
		if (!bucket)
		    return FALSE;
		if (!(bucket->name = MakeName (file->d_name)))
		    return FALSE;
		bucket->next = 0;
		bucket->renderer = renderer;
		*prev = bucket;
	    }
	}
    }
    for (hash = 0; hash < HASH_SIZE; hash++)
    {
	for (bucket = hashTable[hash]; bucket; bucket = next)
	{
	    next = bucket->next;
	    strcpy (fileName, bucket->name);
	    strcat (fileName, bucket->renderer->fileSuffix);
	    if (!ProcessFile (dirName, fileName, table))
	    {
		fprintf(stderr, "%s: unable to process font %s/%s, skipping\n",
			progName, dirName, fileName);
	    }
	    free (bucket->name);
	    free (bucket);
	}
    }
    free (hashTable);
    return TRUE;
}

LoadScalable (dirName, table)
    char	    *dirName;
    FontTablePtr    table;
{
    char    file_name[MAXFONTFILENAMELEN];
    char    font_name[MAXFONTNAMELEN];
    char    dir_file[MAXFONTFILENAMELEN];
    FILE    *file;
    int	    count;
    int	    i;

    strcpy(dir_file, dirName);
    if (dirName[strlen(dirName) - 1] != '/')
	strcat(dir_file, "/");
    strcat(dir_file, FontScalableFile);
    file = fopen(dir_file, "r");
    if (file) {
	count = fscanf(file, "%d\n", &i);
	if ((count == EOF) || (count != 1)) {
	    fclose(file);
	    return BadFontPath;
	}
	while ((count = fscanf(file, "%s %[^\n]\n", file_name, font_name)) != EOF) {
	    if (count != 2) {
		fclose(file);
		fprintf (stderr, "%s: bad format for %s file\n",
			 progName, dir_file);
		return FALSE;
	    }
	    if (!AddEntry (table, font_name, file_name))
	    {
		fclose (file);
		fprintf (stderr, "%s: out of memory\n", progName);
		return FALSE;
	    }
	}
	fclose(file);
    } else if (errno != ENOENT) {
	perror (dir_file);
	return FALSE;
    }
    return TRUE;
}

static Bool
DoDirectory(dirName)
    char	*dirName;
{
    FontTableRec	table;
    Bool		status;

    if (!FontFileInitTable (&table, 100))
	return FALSE;
    if (!LoadDirectory (dirName, &table))
    {
	FontFileFreeTable (&table);
	return FALSE;
    }
    if (!LoadScalable (dirName, &table))
    {
	FontFileFreeTable (&table);
	return FALSE;
    }
    status = TRUE;
    if (table.used >= 0)
	status = WriteFontTable (dirName, &table);
    FontFileFreeTable (&table);
    return status;
}

GetDefaultPointSize ()
{
    return 120;
}

GetClientResolutions ()
{
    return 0;
}

RegisterFPEFunctions ()
{
}

ErrorF ()
{
}

/***====================================================================***/

main (argc, argv)
    int argc;
    char **argv;
{
    int i;

    BitmapRegisterFontFileFunctions ();
    progName = argv[0];
    if (argc == 1)
    {
	if (!DoDirectory("."))
	{
	    fprintf (stderr, "%s: failed to create directory in %s\n",
		     progName, ".");
	    exit (1);
	}
    }
    else
	for (i = 1; i < argc; i++) {
	    if (!DoDirectory(argv[i]))
	    {
		fprintf (stderr, "%s: failed to create directory in %s\n",
			 progName, argv[i]);
		exit (1);
	    }
 	}
    exit(0);	
}
