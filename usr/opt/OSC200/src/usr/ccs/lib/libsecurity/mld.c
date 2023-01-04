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
static char	*sccsid = "@(#)$RCSfile: mld.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:16:29 $";
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
 * Copyright (c) 1988-1990 SecureWare, Inc.
 * All Rights Reserved.
 */



#include <sys/secdefines.h>

#if SEC_MAC

#include "libsecurity.h"

#include	<sys/types.h>
#include	<stdio.h>
#include	<sys/stat.h>
#ifdef AUX
#include	<sys/dir.h>
#define	dirent	direct
#else
#include	<dirent.h>
#endif

#include	<sys/security.h>
#include	<sys/secpolicy.h>
#include	<sys/audit.h>
#include	<mandatory.h>
#include	<prot.h>

#ifdef MLDDEBUG
#define	LOOPS	2
#endif



static struct dirent *nextdir();
static char *fulldirname();


extern char *malloc();
extern char *calloc();

#ifdef MLDDEBUG
main(argc, argv)
	int argc;
	char *argv[];
{
	char *mld_name;
	MDIR *mymld;
	struct dirent *mentry;
	int count = 0;
	int ret;
	mand_ir_t *spec;
	int technique;
	char subdir[NAME_MAX+1];

	set_auth_parameters(argc, argv);

	switch (argc)  {
		case 1:
			mld_name = ".";
			technique = MAND_MLD_ALLDIRS;
			break;
		case 2:
			mld_name = argv[1];
			technique = MAND_MLD_ALLDIRS;
			break;
		case 3:
			mld_name = argv[1];
			technique = MAND_MLD_MANDLEVEL;
			spec = mand_er_to_ir(argv[2]);
			break;
		default:
			printf(MSGSTR(MLD_1, "Usage: %s [multileveldir]\n"), command_name);
			break;
	}

	mymld = openmultdir(mld_name, technique, spec);

	if (mymld == (MDIR *) 0)  {
		ret = ismultdir(mld_name);
		switch (ret)  {
			case 0:
				printf(MSGSTR(MLD_2, "%s isn't a multilevel directory\n"),
					mld_name);
				break;
			case 1:
				printf(MSGSTR(MLD_3, "memory or DB problems\n"));
				break;
			default:
				printf(MSGSTR(MLD_4, "Cannot access %s\n"), mld_name);
				break;
		}
	}
	else  {
		do  {
			readmultdir(mymld, subdir, &mentry);

			if (mentry == (struct dirent *) 0)  {
				count++;
				if (count < LOOPS)  {
					printf(MSGSTR(MLD_5, "rewinding to try again\n"));
					rewindmultdir(mymld);
				}
			}
			else
				printf(MSGSTR(MLD_6, "ino %5d subdir %14s name %14s\n"),
					mentry->d_ino, subdir, mentry->d_name);
		}
		while (count < LOOPS);

		closemultdir(mymld);
	}
}
#endif


/*
 * Like opendir(), openmultdir() is called to open a MLD (multi-level
 * directory).  A scanning technique is specified, where the scan can
 * encompass all subdirectories or only the once with the given security
 * level of spec.
 */
MDIR *
openmultdir(dirname, technique, spec)
	char *dirname;
	int technique;
	mand_ir_t *spec;
{
	struct multdir *mld;
	struct dirent *subdir;
	char *full_name;

	/*
	 * This routine is only successful on MLDs.  Even regular
	 * directories with the same structure will not be legal.
	 */
	if (ismultdir(dirname) == 1)  {
	    mld = (MDIR *) calloc(sizeof(*mld), 1);
	    if (mld != (MDIR *) 0)  {
		mld->name = malloc(strlen(dirname) + 1);
		mld->technique = technique;

		/*
		 * Determine scanning technique.
		 */
		switch (mld->technique)  {
			case MAND_MLD_ALLDIRS:
				break;
			case MAND_MLD_MANDLEVEL:
				mld->ir = mand_alloc_ir();
				memcpy(mld->ir, spec, mand_bytes());
				break;
		}

		/*
		 * The `real' file entries are 2 levels down.  The
		 * opendir() routines handle the management of the
		 * MLD itself and each of the subdirectories.
		 */
		strcpy(mld->name, dirname);
		mld->mdir = opendir(dirname);
		if (mld->mdir != (DIR *) 0)  {
			subdir = nextdir(mld);
			if (subdir)  {
				full_name = fulldirname(mld->name,
							subdir->d_name);
				mld->sdir = opendir(full_name);
				free(full_name);
			}
		} else
			mld = (MDIR *) 0;
	    }
	}
	else
		mld = (MDIR *) 0;

	return mld;
}


/*
 * Close a MLD and release the open files and space used for it.
 */
void
closemultdir(mld)
	MDIR *mld;
{
	if (mld != (MDIR *) 0)  {
		if (mld->sdir != (DIR *) 0)
			closedir(mld->sdir);
		if (mld->mdir != (DIR *) 0)
			closedir(mld->mdir);
		if (mld->name != (char *) 0)
			free(mld->name);
		switch (mld->technique)  {
			case MAND_MLD_ALLDIRS:
				break;
			case MAND_MLD_MANDLEVEL:
				if (mld->ir != (mand_ir_t *) 0)
					mand_free_ir(mld->ir);
				break;
		}

		free(mld);
	}
}


/*
 * Reset the MLD to the beginning, so the scan can be redone.
 */
void
rewindmultdir(mld)
	MDIR *mld;
{
	struct dirent *subdir;
	char *full_name;

	if (mld->sdir != (DIR *) 0)  {
		closedir(mld->sdir);
		mld->sdir = (DIR *) 0;
	}
	if (mld->mdir != (DIR *) 0)  {
		rewinddir(mld->mdir);
		subdir = nextdir(mld);
		if (subdir)  {
			full_name = fulldirname(mld->name, subdir->d_name);
			mld->sdir = opendir(full_name);
			free(full_name);
		}
	}
}


/*
 * Read an entry from a MLD.  Both the subdirname and actual subdirent
 * entry is filled in.
 */
void
readmultdir(mld, subdirname, subdirent)
	MDIR *mld;
	char *subdirname;
	struct dirent **subdirent;
{
	struct dirent *subdir;
	char *full_name;

	if (mld->sdir != (DIR *) 0)  {
		*subdirent = readdir(mld->sdir);

		/*
		 * Finished with one subdirectory, close it off and
		 * proceed to the next one.
		 */
		if (*subdirent == (struct dirent *) 0) {
			closedir(mld->sdir);
			mld->sdir = (DIR *) 0;
			subdir = nextdir(mld);

			if (subdir)  {
				full_name =
					fulldirname(mld->name, subdir->d_name);
				mld->sdir = opendir(full_name);
				free(full_name);
				if (mld->sdir != (DIR *) 0)
					*subdirent = readdir(mld->sdir);
			}
		}
	}
	else
		*subdirent = (struct dirent *) 0;

	if ((mld->sdir != (DIR *) 0) && (subdirname != (char *) 0))
		strcpy(subdirname, mld->sdirname);
}


/*
 * Return the subdirectory at the given security level.  If none is found,
 * or the directory is not MLD, return NULL.  This could be done by
 * openmultdir() with contortions and overhead;  this routine cuts out
 * some of the unused muck.
 */
char *
ir_to_subdir(mld_name, ir)
	char *mld_name;
	register mand_ir_t *ir;
{
	register DIR *mld = (DIR *) 0;
	register struct dirent *subdir;
	register mand_ir_t *file_sec_level = (mand_ir_t *) 0;
	register char *subdirname = (char *) 0;
	char *full_name = (char *) 0;
	char *psubname;
	static char subdirbuf[NAME_MAX + 1];

	mld = opendir(mld_name);
	full_name = malloc(strlen(mld_name) + NAME_MAX + 2);
	file_sec_level = mand_alloc_ir();

	if ((mld != (DIR *) 0) && (full_name != (char *) 0) &&
	    (file_sec_level != (mand_ir_t *) 0) &&
	    (ismultdir(mld_name) == 1))  {
		strcpy(full_name, mld_name);
		strcat(full_name, "/");
		psubname = full_name + strlen(full_name);

		/*
		 * Search each subdirectory to find the one at the given
		 * security level.
		 */
		while ((subdirname == (char *) 0) && (subdir = readdir(mld)))  {
			if ((strcmp(subdir->d_name, ".") != 0) &&
			    (strcmp(subdir->d_name, "..") != 0))  {
				strcpy(psubname, subdir->d_name);
				statslabel(full_name, file_sec_level);
				if (memcmp(file_sec_level, ir,
					   mand_bytes()) == 0)  {
					strcpy(subdirbuf, subdir->d_name);
					subdirname = subdirbuf;
				}
			}
		}
	}

	if (file_sec_level != (mand_ir_t *) 0)
		mand_free_ir(file_sec_level);

	if (mld != (DIR *) 0)
		closedir(mld);

	if (full_name != (char *) 0)
		free(full_name);

	return subdirname;
}


/*
 * Find the next subdirectory in the scan.  Ignore `.' and `..' entries.
 * Use the scanning technique to skip over useless directories.
 */
static struct dirent *
nextdir(mld)
	MDIR *mld;
{
	register struct dirent *subdir;
	register int look_more;
	struct stat stat_buf;
	char *full_name;
	mand_ir_t *file_sec_level;

	do  {
		subdir = readdir(mld->mdir);
		if (subdir == (struct dirent *) 0)
			look_more = 0;
		else  {
			full_name = fulldirname(mld->name, subdir->d_name);

			look_more = ((strcmp(subdir->d_name, ".") == 0) ||
				     (strcmp(subdir->d_name, "..") == 0) ||
				     (stat(full_name, &stat_buf) != 0) ||
				     ((stat_buf.st_mode & S_IFMT) != S_IFDIR));

			if (!look_more)
			    switch (mld->technique)  {
				case MAND_MLD_ALLDIRS:
					break;
				case MAND_MLD_MANDLEVEL:
					file_sec_level = mand_alloc_ir();
					statslabel(full_name, file_sec_level);
					look_more = (memcmp(file_sec_level,
						mld->ir, mand_bytes()) != 0);
					mand_free_ir(file_sec_level);
					break;
			    }
			free(full_name);

			if (!look_more)
				strcpy(mld->sdirname, subdir->d_name);
		}
	} while (look_more);

	return subdir;
}

/*
 * Form the full pathname of the directory.  This is to be able to
 * reference the directory and files while the MLD is open.
 */
static char *
fulldirname(parent, ent)
	char *parent;
	char *ent;
{
	register char *full_name;

	full_name = malloc(strlen(parent) + strlen(ent) + 2);
	strcpy(full_name, parent);
	strcat(full_name, "/");
	strcat(full_name, ent);

	return full_name;
}
#endif
