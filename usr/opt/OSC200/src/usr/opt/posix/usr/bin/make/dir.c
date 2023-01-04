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
static char *rcsid = "@(#)$RCSfile: dir.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/29 22:01:54 $";
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
 * dir.c --
 *	Directory searching using wildcards and/or normal names...
 *	Used both for source wildcarding in the Makefile and for finding
 *	implicit sources.
 *
 * The interface for this module is:
 *	Dir_Init  	    Initialize the module.
 *
 *	Dir_FindFile	    Searches for a file on a given search path list.
 *	    	  	    If it exists, the entire path is returned.
 *	    	  	    Otherwise NULL is returned.
 *
 *	Dir_MTime 	    Return the modification time of a node. The file
 *	    	  	    is searched for along the default search path.
 *	    	  	    The path and mtime fields of the node are filled
 *	    	  	    in.
 *
 * For debugging:
 *	Dir_PrintDirectories	Print stats about the directory cache.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include "make.h"
#include "hash.h"
#include "pmake_msg.h"

extern nl_catd catd;

/*
 * DIRTRACE macro used only during development for tracing dir module.
 * Use the compiler option -DDIRTRACE or -DALLTRACE to turn on.
 */

#if defined(DIRTRACE) || defined(ALLTRACE)
#undef DIRTRACE
#define DIRTRACE(message) { \
   printf("DIRTRACE: %s.\n",message);\
   }
#else
#define DIRTRACE(message)
#endif

#define S_IW_ALL_MASK (S_IWUSR|S_IWGRP|S_IWOTH)

/*
 *	A search path consists of a Lst of Path structures. A Path structure
 *	has in it the name of the directory and a hash table of all the files
 *	in the directory. This is used to cut down on the number of system
 *	calls necessary to find implicit dependents and their like. Since
 *	these searches are made before any actions are taken, we need not
 *	worry about the directory changing due to creation commands. If this
 *	hampers the style of some makefiles, they must be changed.
 *
 *	A list of all previously-read directories is kept in the
 *	openDirectories Lst. This list is checked first before a directory
 *	is opened.
 *
 *	The need for the caching of whole directories is brought about by
 *	the multi-level transformation code in suff.c, which tends to search
 *	for far more files than regular make does. In the initial
 *	implementation, the amount of time spent performing "stat" calls was
 *	truly astronomical. The problem with hashing at the start is,
 *	of course, that pmake doesn't then detect changes to these directories
 *	during the course of the make. Three possibilities suggest themselves:
 *
 *	    1) just use stat to test for a file's existence. As mentioned
 *	       above, this is very inefficient due to the number of checks
 *	       engendered by the multi-level transformation code.
 *	    2) use readdir() and company to search the directories, keeping
 *	       them open between checks. I have tried this and while it
 *	       didn't slow down the process too much, it could severely
 *	       affect the amount of parallelism available as each directory
 *	       open would take another file descriptor out of play for
 *	       handling I/O for another job. Given that it is only recently
 *	       that UNIX OS's have taken to allowing more than 20 or 32
 *	       file descriptors for a process, this doesn't seem acceptable
 *	       to me.
 *	    3) record the mtime of the directory in the Path structure and
 *	       verify the directory hasn't changed since the contents were
 *	       hashed. This will catch the creation or deletion of files,
 *	       but not the updating of files. However, since it is the
 *	       creation and deletion that is the problem, this could be
 *	       a good thing to do. Unfortunately, if the directory (say ".")
 *	       were fairly large and changed fairly frequently, the constant
 *	       rehashing could seriously degrade performance. It might be
 *	       good in such cases to keep track of the number of rehashes
 *	       and if the number goes over a (small) limit, resort to using
 *	       stat in its place.
 *
 *	An additional thing to consider is that pmake is used primarily
 *	to create C programs and until recently pcc-based compilers refused
 *	to allow you to specify where the resulting object file should be
 *	placed. This forced all objects to be created in the current
 *	directory. This isn't meant as a full excuse, just an explanation of
 *	some of the reasons for the caching used here.
 *
 *	One more note: the location of a target's file is only performed
 *	on the downward traversal of the graph and then only for terminal
 *	nodes in the graph. This could be construed as wrong in some cases,
 *	but prevents inadvertent modification of files when the "installed"
 *	directory for a file is provided in the search path.
 *
 *	Another data structure maintained by this module is an mtime
 *	cache used when the searching of cached directories fails to find
 *	a file. In the past, Dir_FindFile would simply perform an access()
 *	call in such a case to determine if the file could be found using
 *	just the name given. When this hit, however, all that was gained
 *	was the knowledge that the file existed. Given that an access() is
 *	essentially a stat() without the copyout() call, and that the same
 *	filesystem overhead would have to be incurred in Dir_MTime, it made
 *	sense to replace the access() with a stat() and record the mtime
 *	in a cache for when Dir_MTime was actually called.
 */

Lst   dirSearchPath;	/* main search path */
Lst   dirSccsPath;	/* (jed) under development  sccs search path */

static Lst   openDirectories;	/* the list of all open directories */

/*
 * Variables for gathering statistics on the efficiency of the hashing
 * mechanism.
 */
static int    hits,	      /* Found in directory cache */
misses,	      /* Sad, but not evil misses */
nearmisses,     /* Found under search path */
bigmisses;      /* Sought by itself */

typedef struct Path {
	char         *name;	    	/* Name of directory */
	int	    	  refCount; 	/* Number of paths with this directory */
	int		  hits;	    	/* the number of times a file in this
					 * directory has been found */
	Hash_Table    files;    	/* Hash table of files in directory */
} Path;

static Path    	  *CurrentDirPath;	    /* contents of current directory */
static Hash_Table mtimes;   /* Results of doing a last-resort stat in
			     * Dir_FindFile -- if we have to go to the
			     * system to find the file, we might as well
			     * have its mtime on record. XXX: If this is done
			     * way early, there's a chance other rules will
			     * have already updated the file, in which case
			     * we'll update it again. Generally, there won't
			     * be two rules to update a single file, so this
			     * should be ok, but... */


/*-
 *-----------------------------------------------------------------------
 * Dir_Init --
 *	initialize things for this module
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	some directories may be opened.
 *-----------------------------------------------------------------------
 */
void
Dir_Init (void)
{
	dirSearchPath = Lst_Init (FALSE);
	openDirectories = Lst_Init (FALSE);
	Hash_InitTable(&mtimes, 0);

	/*
     * Since the Path structure is placed on both openDirectories and
     * the path we give DirAddDir (which in this case is openDirectories),
     * we need to remove "." from openDirectories and what better time to
     * do it than when we have to fetch the thing anyway?
     */
	DirAddDir (openDirectories, ".");
	CurrentDirPath = (Path *) Lst_DeQueue (openDirectories);

	/*
     * We always need to have CurrentDirPath around, so we increment its reference count
     * to make sure it's not destroyed.
     */
	CurrentDirPath->refCount += 1;
}

/*-
 *-----------------------------------------------------------------------
 * DirFindName --
 *	See if the Path structure describes the same directory as the
 *	given one by comparing their names. Called from DirAddDir via
 *	Lst_Find when searching the list of open directories.
 *
 * Results:
 *	0 if it is the same. Non-zero otherwise
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
static int
DirFindName (Path *p, char *dname)
{
	/* Path          *p;	      Current name */
	/* char	     *dname;          Desired name */

	return (strcmp (p->name, dname));
}

/*-
 *-----------------------------------------------------------------------
 * Dir_FindFile  --
 *	Find the file with the given name along the static search path.
 *
 * Results:
 *	The path to the file or NULL. This path is guaranteed to be in a
 *	different part of memory than name and so may be safely free'd.
 *
 * Side Effects:
 *	If the file is found in a directory which is not on the path
 *	already (either 'name' is absolute or it is a relative path
 *	[ dir1/.../dirn/file ] which exists below one of the directories
 *	already on the search path), its directory is added to the end
 *	of the path on the assumption that there will be more files in
 *	that directory later on. Sometimes this is true. Sometimes not.
 *-----------------------------------------------------------------------
 */
char *
Dir_FindFile (char *name, Lst InputPathLst)
{
	/* char    	  *name;   the file to find */
	/* Lst               InputPathLst;    the Lst of directories to search */

	char *p1;	    		/* pointer into p->name */
	char *p2;	    		/* pointer into name */
	LstNode     ln;	    	/* a list element */
	char *file;    		/* the current filename to check */
	Path *CurMemberPath;	    	/* current path member */
	char *cp;	    		/* index of first slash, if any */
	Boolean	  hasSlash; /* true if 'name' contains a / */
	struct stat	  stb;	    /* Buffer for stat, if necessary */
	Hash_Entry	  *entry;   /* Entry for mtimes table */

	DIRTRACE("Dir_FindFile Part 0: Find the file with the given name along the static search path");
	/*
     * Find the final component of the name and note whether it has a
     * slash in it (the name, I mean)
     */
	cp = rindex (name, '/');
	if (cp) {
		hasSlash = TRUE;
		cp += 1;
	} else {
		hasSlash = FALSE;
		cp = name;
	}


	/*
     * We look for the file in the current directory
     * before anywhere else and we *do not* add the ./ to it if it exists.
     * This is so there are no conflicts between what the user specifies
     * (fish.c) and what pmake finds (./fish.c).
     */

	if (InputPathLst == dirSearchPath) {
		DIRTRACE("Dir_FindFile Part 1: Find the file in the current directory");

		if (DEBUG(DIR)) {
			printf(catgets(catd, MS_DEBUG, DIR001,
			    "Searching for file: %s/%s ....\n"), CurrentDirPath->name, name);
		}
		if ((!hasSlash || (cp - name == 2 && *name == '.'))) {
			if ((Hash_FindEntry (&CurrentDirPath->files, (Address)cp) != (Hash_Entry *)NULL)) {
				if (DEBUG(DIR)) {
					printf(catgets(catd, MS_DEBUG, DIR002, "Found file: %s/%s\n"),
					    CurrentDirPath->name, name);
				}
				hits += 1;
				CurrentDirPath->hits += 1;
				return (strdup (name));
			}
		}
	}

	DIRTRACE("Dir_FindFile Part 2: Open search path of the given directory");

	/* InputPathLst = dirSearchPath; */

	if (Lst_Open (InputPathLst) == FAILURE) {
		if (DEBUG(DIR)) {
			printf(catgets(catd, MS_DEBUG, DIR003,
			    "Couldn't open search InputPathLst. File not found.\n"));
		}
		misses += 1;
		return ((char *) NULL);
	}

	/*
     * We look through all the directories on the path seeking one which
     * contains the final component of the given name and whose final
     * component(s) match the name's initial component(s). If such a beast
     * is found, we concatenate the directory name and the final component
     * and return the resulting string. If we don't find any such thing,
     * we go on to phase two...
     */

	DIRTRACE("Dir_FindFile Part 3: Find the file looking though given directory");

	while ((ln = Lst_Next (InputPathLst)) != NILLNODE) {
		CurMemberPath = (Path *) Lst_Datum (ln);

		if (DEBUG(DIR)) {
			printf(catgets(catd, MS_DEBUG, DIR001,
			    "Searching for file: ./%s/%s ....\n"), CurMemberPath->name, name);
		}

		if (Hash_FindEntry (&CurMemberPath->files, (Address)cp) != (Hash_Entry *)NULL) {
			if (DEBUG(DIR)) {
				printf(catgets(catd, MS_DEBUG, DIR004,
				    "Found file in hash table: ./%s/%s\n"), CurMemberPath->name, name);
			}

			if (hasSlash) {
				/*
		 * If the name had a slash, its initial components and p's
		 * final components must match. This is false if a mismatch
		 * is encountered before all of the initial components
		 * have been checked (p2 > name at the end of the loop), or
		 * we matched only part of one of the components of p
		 * along with all the rest of them (*p1 != '/').
		 */
				p1 = CurMemberPath->name + strlen (CurMemberPath->name) - 1;
				p2 = cp - 2;
				while (p2 >= name && *p1 == *p2) {
					p1 -= 1; 
					p2 -= 1;
				}
				if (p2 >= name || (p1 >= CurMemberPath->name && *p1 != '/')) {
					if (DEBUG(DIR)) {
						printf(catgets(catd, MS_DEBUG, DIR005,
						    "Component mismatch -- continuing...\n"));
					}
					continue;
				}
			}			/*  if (hasSlash)   */

			if (InputPathLst == dirSccsPath) {
				file =  strdup (name);
			} else {
				file = Str_Concat (CurMemberPath->name, cp, STR_ADDSLASH);
			}

			if (DEBUG(DIR)) {
				printf(catgets(catd, MS_DEBUG, DIR006, "Returning %s\n"), file);
			}

			Lst_Close (InputPathLst);
			CurMemberPath->hits += 1;
			hits += 1;
			return (file);
		} else if (hasSlash) {
			/*
	 * If the file has a leading path component and that component
	 * exactly matches the entire name of the current search
	 * directory, we assume the file doesn't exist and return NULL.
	 */
			for (p1 = CurMemberPath->name, p2 = name; *p1 && *p1 == *p2; p1++, p2++) {
				continue;
			}
			if (*p1 == '\0' && p2 == cp - 1) {
				if (DEBUG(DIR)) {
					printf(catgets(catd, MS_DEBUG, DIR007,
					    "Must be here but isn't -- Returing NULL\n"));
				}
				Lst_Close (InputPathLst);
				return ((char *) NULL);
			}
		}
	}

	DIRTRACE("Dir_FindFile Part 4: Find the file looking though all directories if possible");

	/*
     * We didn't find the file on any existing members of the directory.
     * If the name doesn't contain a slash, that means it doesn't exist.
     * If it *does* contain a slash, however, there is still hope: it
     * could be in a subdirectory of one of the members of the search
     * path. (eg. /usr/include and sys/types.h. The above search would
     * fail to turn up types.h in /usr/include, but it *is* in
     * /usr/include/sys/types.h) If we find such a beast, we assume there
     * will be more (what else can we assume?) and add all but the last
     * component of the resulting name onto the search path (at the
     * end). This phase is only performed if the file is *not* absolute.
     */
	if (!hasSlash) {
		if (DEBUG(DIR)) {
			printf(catgets(catd, MS_DEBUG, DIR008, "Dir_FindFile: No slash. Cannot search. Returning NULL.\n"));
		}
		misses += 1;
		return ((char *) NULL);
	}

	if (*name != '/') {
		Boolean	checkedCurrentDirPath = FALSE;

		if (DEBUG(DIR)) {
			printf(catgets(catd, MS_DEBUG, DIR009,
			    "Dir_FindFile: Trying all Subdirectories...\n"));
		}
		(void) Lst_Open (InputPathLst);
		while ((ln = Lst_Next (InputPathLst)) != NILLNODE) {
			CurMemberPath = (Path *) Lst_Datum (ln);
			if (CurMemberPath != CurrentDirPath) {
				file = Str_Concat (CurMemberPath->name, name, STR_ADDSLASH);
			} else {
				/*
		 * Checking in CurrentDirPath -- DON'T put a leading ./ on the thing.
		 */
				file = strdup(name);
				checkedCurrentDirPath = TRUE;
			}
			if (DEBUG(DIR)) {
				printf(catgets(catd, MS_DEBUG, DIR010, "Checking %s...\n"), file);
			}


			if (stat (file, &stb) == 0) {
				if (DEBUG(DIR)) {
					printf(catgets(catd, MS_DEBUG, DIR011, "Found it.\n"));
				}

				Lst_Close (InputPathLst);

				/*
		 * We've found another directory to search. We know there's
		 * a slash in 'file' because we put one there. We nuke it after
		 * finding it and call DirAddDir to add this new directory
		 * onto the existing search path. Once that's done, we restore
		 * the slash and triumphantly return the file name, knowing
		 * that should a file in this directory every be referenced
		 * again in such a manner, we will find it without having to do
		 * numerous numbers of access calls. Hurrah!
		 */
				cp = rindex (file, '/');
				*cp = '\0';
				DirAddDir (InputPathLst, file);
				*cp = '/';

				/*
		 * Save the modification time so if it's needed, we don't have
		 * to fetch it again.
		 */
				if (DEBUG(DIR)) {
					printf(catgets(catd, MS_DEBUG, DIR012,
					    "Caching %s for %s\n"), Targ_FmtTime(stb.st_mtime), file);
				}
				entry = Hash_CreateEntry(&mtimes, file,
				    (Boolean *)NULL);
				Hash_SetValue(entry, stb.st_mtime);
				nearmisses += 1;
				return (file);
			} else {
				free (file);
			}
		}

		if (DEBUG(DIR)) {
			printf(catgets(catd, MS_DEBUG, DIR013, "Search failed.\n"));
		}
		Lst_Close (InputPathLst);

		if (checkedCurrentDirPath) {
			/*
	     * Already checked by the given name, since . was in the path,
	     * so no point in proceeding...
	     */
			if (DEBUG(DIR)) {
				printf(catgets(catd, MS_DEBUG, DIR014,
				    "Checked . already, returning NULL\n"));
			}
			return(NULL);
		}
	}

	DIRTRACE("Dir_FindFile Part 5: Find the file looking in the hash table");

	/*
     * Didn't find it that way, either. Sigh. Phase 3. Add its directory
     * onto the search path in any case, just in case, then look for the
     * thing in the hash table. If we find it, grand. We return a new
     * copy of the name. Otherwise we sadly return a NULL pointer. Sigh.
     * Note that if the directory holding the file doesn't exist, this will
     * do an extra search of the final directory on the path. Unless something
     * weird happens, this search won't succeed and life will be groovy.
     *
     * Sigh. We cannot add the directory onto the search path because
     * of this amusing case:
     * $(INSTALLDIR)/$(FILE): $(FILE)
     *
     * $(FILE) exists in $(INSTALLDIR) but not in the current one.
     * When searching for $(FILE), we will find it in $(INSTALLDIR)
     * b/c we added it here. This is not good...
     */
	if (DEBUG(DIR)) {
		printf(catgets(catd, MS_DEBUG, DIR015, "Looking for \"%s\"...\n"), name);
	}

	bigmisses += 1;
	entry = Hash_FindEntry(&mtimes, name);
	if (entry != (Hash_Entry *)NULL) {
		if (DEBUG(DIR)) {
			printf(catgets(catd, MS_DEBUG, DIR016,"got it (in mtime cache)\n"));
		}
		return(strdup(name));
	} else if (stat (name, &stb) == 0) {
		entry = Hash_CreateEntry(&mtimes, name, (Boolean *)NULL);
		if (DEBUG(DIR)) {
			printf(catgets(catd, MS_DEBUG, DIR017,
			    "Caching %s for %s\n"), Targ_FmtTime(stb.st_mtime), name);
		}
		Hash_SetValue(entry, stb.st_mtime);
		return (strdup (name));
	} else {
		if (DEBUG(DIR)) {
			printf(catgets(catd, MS_DEBUG, DIR018, "failed. Returning NULL\n"));
		}
		return ((char *)NULL);
	}
}

/*-
 *-----------------------------------------------------------------------
 * Dir_MTime  --
 *	Find the modification time of the file described by gn along the
 *	search path dirSearchPath.
 * 
 * Results:
 *	The modification time or 0 if it doesn't exist
 *
 * Side Effects:
 *	The modification time is placed in the node's mtime slot.
 *	If the node didn't have a path entry before, and Dir_FindFile
 *	found one for it, the full name is placed in the path slot.
 *-----------------------------------------------------------------------
 */
int
Dir_MTime (GNode *gn)
{
	/* GNode         *gn;   The file whose modification time is desired */

	char          *fullName;  /* the full pathname of name */
	char          *sccsgetName;  /* the full sccsget name */
	char          *sccsgetPath;  /* the full sccsget pathname */

	struct stat	  stb;	      /* buffer for finding the mod time */
	struct stat	  sccsget_stb;	      /* buffer for finding the mod time od sccsget*/

	Hash_Entry	  *entry;
	Hash_Entry	  *sccsentry;
	Path          *SPath;
	LstNode       ln;



	DIRTRACE("Dir_MTime: Find the modification time of the file");

	if (gn->type & OP_ARCHV) {
		return Arch_MTime (gn);
	} else if (gn->path == (char *)NULL) {
		fullName = Dir_FindFile (gn->name, dirSearchPath);
	} else {
		fullName = gn->path;
	}

	if (fullName == (char *)NULL) {
		fullName = gn->name;
	}

	{
		if (gn->SccsGetFileExists == TRUE) {

			DIRTRACE("Dir_MTime: Checking modification time of SCCS file");

			sccsgetPath = gn->sccsgetpath;
			sccsgetName = gn->sccsgetname;

			(void) Lst_Open (dirSccsPath);
			ln = Lst_Next (dirSccsPath);
			SPath = (Path *) Lst_Datum (ln);

			sccsentry = Hash_FindEntry (&SPath->files, sccsgetName);
			lstat (sccsgetPath, &sccsget_stb);
			(void) Lst_Close (dirSccsPath);
		}
	}

	entry = Hash_FindEntry(&mtimes, fullName);
	if (entry != (Hash_Entry *)NULL) {
		/*
	 * Only do this once -- the second time folks are checking to
	 * see if the file was actually updated, so we need to actually go
	 * to the file system.
	 */
		if (DEBUG(DIR)) {
			printf(catgets(catd, MS_DEBUG, DIR019,
			    "Using cached time %s for %s\n"),
			    Targ_FmtTime((time_t)Hash_GetValue(entry)), fullName);
		}
		stb.st_mtime = (time_t)Hash_GetValue(entry);
		Hash_DeleteEntry(&mtimes, entry);
	} else if (stat (fullName, &stb) < 0) {
		if (gn->type & OP_MEMBER) {
			return Arch_MemMTime (gn);
		} else {
			stb.st_mtime = 0;
		}
	}


	if (fullName && gn->path == (char *)NULL) {
		gn->path = fullName;
	}

	if (gn->SccsGetFileExists == TRUE) {
		if ((sccsget_stb.st_mtime > stb.st_mtime) | (stb.st_mtime == 0)) 
		{
			int mode;
			DIRTRACE("Dir_MTime: SCCS/s.file is more recent Local file");
			if (stat (fullName, &stb) == 0) {
			  mode = stb.st_mode;
			  mode &=  S_IW_ALL_MASK;

			  if (!((mode & S_IWUSR) || (mode & S_IWGRP) || (mode & S_IWOTH))) {
			    DIRTRACE("Dir_MTime: Local file is NOT write enabled!");
			    gn->mtime = 0;
			    return (gn->mtime);
			  } else {
			    DIRTRACE("Dir_MTime: Local file is write enabled!");
			    fprintf(stderr, "Warning: file is writable! Ignoring SCCS/s.file.\n");
			  }
			} 
		      } else {
			DIRTRACE("Dir_MTime: Local file is more recent than SCCS/s.file");
		      }
	      }
	gn->mtime = stb.st_mtime;
	return (gn->mtime);

}

/*-
 *-----------------------------------------------------------------------
 * DirAddDir --
 *	Add the given directoy name to the end of the given path list. 
 *      If the directoy has been opened and hashed, it is in the openDirectories
 *      list. If it is not opened and hashed do it and also place it on the  
 *      openDirectories list. The order of
 *	the arguments is backwards so ParseDoDependency can do a
 *	Lst_ForEach of its list of paths...
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	A structure is added to the list and the directory is 
 *	read and hashed.
 *-----------------------------------------------------------------------
 */
void
DirAddDir (Lst path, char *name)
{
	/* Lst           path;     The path list to which the directory should be added */
	/* char          *name;    The name of the directory to add to the list */

	LstNode       ln;	      /* node in case Path structure is found */
	register Path *p;	      /* pointer to new Path structure */
	DIR     	  *d;	      /* for reading directory */
	register struct dirent *dp; /* entry in directory */

	/* Check to see if the directory is already opened. */

	DIRTRACE("DirAddDir: Add the given directory name to the end of the given path list");

	ln = Lst_Find (openDirectories, (ClientData)name, (int(*)(void*,void*))DirFindName);
	if (ln != NILLNODE) {	/* The directory has been previously opened */

		DIRTRACE("DirAddDir: directory previously opened");

		p = (Path *)Lst_Datum (ln); /* It does not need to be cached. */
		if (Lst_Member(path, (ClientData)p) == NILLNODE) {
			p->refCount += 1;
			(void)Lst_AtEnd (path, (ClientData)p);
		}
	} else {			/* Create a hastable of the directoy contents */
		if (DEBUG(DIR)) {
			printf(catgets(catd, MS_DEBUG, DIR020, "CASHING %s..."), name);
			fflush(stdout);
		}

		DIRTRACE("DirAddDir: CASH  the given directory");

		if ((d = opendir (name)) != (DIR *) NULL) {
			p = (Path *) emalloc (sizeof (Path));
			p->name = strdup (name);
			p->hits = 0;
			p->refCount = 1;
			Hash_InitTable (&p->files, -1);

			/*
	     * Skip the first two entries -- these will *always* be . and ..
	     */
			(void)readdir(d);
			(void)readdir(d);

			while ((dp = readdir (d)) != (struct dirent *) NULL) {
				(void)Hash_CreateEntry(&p->files, dp->d_name, (Boolean *)NULL);
			}
			(void) closedir (d);
			(void)Lst_AtEnd (openDirectories, (ClientData)p); /* Now stick on on open list. */
			(void)Lst_AtEnd (path, (ClientData)p);
		}
		if (DEBUG(DIR)) {
			printf(catgets(catd, MS_DEBUG, DIR021, "done\n"));
		}
	}
}

/********** DEBUG INFO **********/
void
Dir_PrintDirectories(void)
{
	LstNode	ln;
	Path	*p;

	printf ("#*** Directory Cache:\n");
	printf ("# Stats: %d hits %d misses %d near misses %d losers (%d%%)\n",
	    hits, misses, nearmisses, bigmisses,
	    (hits+bigmisses+nearmisses ?
	    hits * 100 / (hits + bigmisses + nearmisses) : 0));
	printf ("# %-20s referenced\thits\n", "directory");
	if (Lst_Open (openDirectories) == SUCCESS) {
		while ((ln = Lst_Next (openDirectories)) != NILLNODE) {
			p = (Path *) Lst_Datum (ln);
			printf ("# directory name: %-20s count: %10d\t hits: %4d\n", p->name, p->refCount, p->hits);
		}
		Lst_Close (openDirectories);
	}
}
