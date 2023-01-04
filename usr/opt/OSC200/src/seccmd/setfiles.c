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
static char	*sccsid = "@(#)$RCSfile: setfiles.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/10/07 15:17:54 $";
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
 * Copyright (c) 1988-90 SecureWare, Inc.  All rights reserved.
 */


/*
 * Based on:

 */

/*
 * This program uses the files in the File Control database to fix the
 * attributes of files contained therein
 */

#include <sys/secdefines.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "setfiles_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SETFILES,n,s)
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

#if SEC_BASE /*{*/

#include <sys/types.h>
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <dirent.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>

#if SEC_ACL_SWARE
#include <acl.h>
#endif
#if SEC_MAC
#include <mandatory.h>
#endif
#if SEC_NCAV
#include <ncav.h>
#endif
#include <errno.h>

extern int errno;

#define	WILD_CARD_END	"/*"


static char **add_path();
static void scan_db();
static void expand_wc();
static void file_analysis();

/* pathname prefix support */

int prefix_size = 0;
char *prefix = (char *) 0;

/* specific entry/wildcard support */

int fdone_count = 0;
int fdone_size = 1000;	/* initial number of entries */

struct files_done {
	dev_t	device;
	ushort	inode;
};

struct files_done *fdone;

int total_errors = 0;
int path_count = 0;
char *path_check;
int nflag = 0;

extern char *optarg;	/* arg pointer for getopt */
extern int optind;	/* option index for getopt */

extern char *malloc();
extern char *realloc();
extern priv_t *privvec();

int
main(argc, argv)
	int argc;
	char *argv[];
{
	register int option;
	register int verbose = 0, ignore = 0;
	register char **target_paths;
	int error = 0;

#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
        catd = catopen(MF_SETFILES,NL_CAT_LOCALE);
#endif

	set_auth_parameters(argc, argv);
	initprivs();
#if SEC_MAC
	mand_init();
#endif

	if (!authorized_user("isso")) {
		fprintf(stderr, MSGSTR(SETFILES_1,
			"%s: need isso authorization\n"), command_name);
		exit(1);
	}

	target_paths = (char **) malloc(sizeof(*target_paths));
	if (target_paths == (char **) 0)  {
		(void) fprintf(stderr, MSGSTR(SETFILES_2,
			"%s: Cannot initialize space for target paths.\n"),
			command_name);
		exit(2);
	}

	*target_paths = (char *) 0;

	while ((option = getopt(argc, argv, "p:nvi")) != EOF)
		switch (option) {
			/*
			 * Tell about all actions.
			 */
			case 'v':
				verbose = 1;
				break;
			
			/*
			 * Report actions but don't do them.
			 */
			case 'n':
				nflag = 1;
				break;

			/*
			 * Ignore paths beginning with the argument name.
			 */
			case 'i':
				ignore = 1;
				break;

			/*
			 * Pathname prefixing support.
			 */

			case 'p':
				prefix = optarg;
				prefix_size = strlen(prefix);
				break;

			case '?':
				error = 1;
				break;
			}

	if (error || ignore && optind >= argc) {
		fprintf(stderr,
		   MSGSTR(SETFILES_3,
			"usage:  %s [-n] [-v] [-i] [-p prefix] [path] ...\n"),
		   command_name);
		exit(2);
	}

	while (optind < argc) {
		target_paths = add_path(target_paths, argv[optind]);
		++optind;
	}

        /* Allocate array for tracking pathname arguments processed */

        if(path_count && (path_check = (char *) calloc(path_count,1)) == NULL) {
                fprintf(stderr, MSGSTR(SETFILES_26, 
			"setfiles: can't allocate path check table\n"));
                exit(2);
        }

	/*
	 * Allocate the initial table for the files processed entries. This
	 * allows specific and wildcard entries in the database with only
	 * the first match having any effect.
	 */

	if((fdone = (struct files_done *) malloc(sizeof(struct files_done) *
	    fdone_size)) == (struct files_done *) 0) {
		fprintf(stderr,MSGSTR(SETFILES_4,
			"setfiles: can't allocate file done table\n"));
		exit(2);
	}

	scan_db(verbose, ignore, target_paths);
        /*
         * If the ignore option is not set, then output an error message if
         * there are pathnames specified but a pathname was not matched.
         */

        if(!ignore && path_count && target_paths) {
                register int i;
                char **pp = target_paths;

                for(i=0; i < path_count; i++) {
                    if(path_check[i] == 0 && pp[i]) {
                        fprintf(stderr, MSGSTR(SETFILES_27,
                            "setfiles: no file control database entry for %s\n")
                            ,pp[i]);
                        total_errors++;
		    }
                }
        }

	return total_errors != 0;
}

static char **
add_path(old_paths, new_name)
	register char **old_paths;
	register char *new_name;
{
	register int path_size;
	register char *new_path;

	new_path = malloc(strlen(new_name) + 1);
	if (new_path == (char *) 0)  {
		(void) fprintf(stderr, MSGSTR(SETFILES_5,
			"%s: Cannot allocate space for path `%s'.\n"),
			command_name, new_name);
		exit(2);
	}

	(void) strcpy(new_path, new_name);

	path_size = 0;
	while (old_paths[path_size] != (char *) 0)
		path_size++;

	old_paths = (char **) realloc(old_paths,
				      (path_size + 2) * sizeof(*old_paths));
	if (old_paths == (char **) 0)  {
		(void) fprintf(stderr, MSGSTR(SETFILES_6,
			"%s: Cannot expand path vector to hold `%s'.\n"),
			command_name, new_name);
		exit(2);
	}

	old_paths[path_size] = new_path;
	old_paths[path_size+1] = (char *) 0;

        path_count++;
	return old_paths;
}


/*
 * March through the File Control database, checking each entry against
 * the real file on the system.
 */
static void
scan_db(verbose, ignore, paths)
	int verbose, ignore;
	char **paths;
{
	register struct pr_file *pr;
	register int entry_no;
	register int name_len;
	register int wc_len;
	char save_name[256];

	setprfient();
	entry_no = 0;
	wc_len = strlen(WILD_CARD_END);

	activate_privs();

	/*
	 * Each pass through this loop covers one entry in the File
	 * Control database.
	 */
	while ((pr = getprfient()) != (struct pr_file *) 0)  {
		if(prefix) {
			strcpy(save_name,prefix);
			strcat(save_name,pr->ufld.fd_name);
		}
		else
			strcpy(save_name, pr->ufld.fd_name);

		name_len = strlen(pr->ufld.fd_name) + prefix_size;
		entry_no++;
		/*
		 * Check for a wildcard entry.  If so, call expand_wc()
		 * to check all possible file matches.  If not, process
		 * the check against the single file with discr_analysis().
		 */
		if (strcmp(save_name+(name_len-wc_len), WILD_CARD_END) == 0)
			expand_wc(save_name, pr, entry_no, verbose,
					ignore, paths);
		else
			file_analysis(entry_no, pr, save_name, (char *) 0,
					verbose, ignore, paths);
		(void) fflush(stdout);
	}
	deactivate_privs();
}


/*
 * The name is a wild card entry.  Use the name to find all files that
 * match it in the directory meant by the entry name.
 */
static void
expand_wc(name, pr, entry_no, verbose, ignore, paths)
	char *name;
	struct pr_file *pr;
	int entry_no;
	int verbose, ignore;
	char **paths;
{
	DIR *dir;
	int switch_posn;
	struct dirent *dir_buf;
	struct stat stat_buf;
	char ent_name[BUFSIZ];

	/*
	 * Strip off the wild card characters to find the directory
	 * component of the name.
	 */
	switch_posn = strlen(name) - strlen(WILD_CARD_END);
	(void) strncpy(ent_name, name, switch_posn);
	ent_name[switch_posn] = '\0';

	/*
	 * Open the directory and scan through the entries, applying
	 * each to the discretionary test.
	 */
	dir = opendir(ent_name);
	if (dir == (DIR *) 0)  {
		if (verbose)  {
			total_errors++;
			printf(MSGSTR(SETFILES_7,
				"Directory %s (entry %d) is missing.\n"),
				ent_name, entry_no);
		}
		return;
	}

	ent_name[switch_posn] = '/';
	while ((dir_buf = readdir(dir)) != (struct dirent *) 0) {
		/*
		 * Ignore entry entries and the references to the
		 * current and parent directories.
		 */
		if ((dir_buf->d_fileno == 0) ||
		    (strcmp(dir_buf->d_name, ".") == 0) ||
		    (strcmp(dir_buf->d_name, "..") == 0))
			continue;

		(void) strcpy(ent_name+switch_posn+1, dir_buf->d_name);
		file_analysis(entry_no, pr, ent_name, name, verbose,
				ignore, paths);
	}

	(void) closedir(dir);
}


/*
 * file_analysis() is called once per real file to fix the
 * discretionary settings.
 */
static void
file_analysis(entry_no, pr, name, real_name, verbose, ignore, paths)
	int entry_no;
	register struct pr_file *pr;
	char *name;
	char *real_name;
	int verbose, ignore;
	char **paths;
{
	register char **pp;
	int found;
	int good_user, good_group, good_mode, good_type;
	int new_owner, new_group;
	struct stat stat_buf;
	char type;
#if SEC_MAC
	int good_slevel;
	static mand_ir_t *mand_ir = (mand_ir_t *) 0;
#endif
#if SEC_ACL_SWARE
	int good_acl;
	static acle_t *acl = (acle_t *) 0;
	static int alen = 0;
#endif
#if SEC_PRIV
	int good_ppriv, good_gpriv;
	privvec_t pprivs;
	privvec_t gprivs;
#endif
#if SEC_NCAV
	int good_ncav;
	static ncav_ir_t *ncav_ir = (ncav_ir_t *) 0;
#endif
	int ret;

	/*
	 * See if this file matches a path specified on the command line.
	 */
	found = 0;
	for (pp = paths; *pp != (char *) 0; ++pp)
		if (matchname(*pp, name)) {

                        /* Mark the pathname as processed */

                        path_check[pp - paths] = 1;

			found = 1;
			break;
		}

	if (*paths && found == ignore) {
		if (verbose)
			printf(MSGSTR(SETFILES_8,
				"%s (%sentry %d) ignored.\n"), name,
				real_name ? MSGSTR(SETFILES_9, "wildcard ") : "", entry_no);
		return;
	}

#if SEC_MAC
	/*
	 * If this entry is expected to be a directory, turn on the
	 * multileveldir privilege to avoid diversion to a subdirectory
	 * in case it is multilevel.
	 */

	if (pr->uflg.fg_type &&
	    pr->ufld.fd_type[0] == 'm' || pr->ufld.fd_type[0] == 'd')
		forcepriv(SEC_MULTILEVELDIR);
#endif

	/*
	 * First check to see if file exists.
	 */
	if (stat(name, &stat_buf) != 0) {
		if (found ||
		    verbose && strcmp(&name[strlen(name)-strlen(AUTH_TEMP_EXT)],
						AUTH_TEMP_EXT) != 0) {
			total_errors++;
			printf(MSGSTR(SETFILES_10,
				"%s (%sentry %d) is missing.\n"),
				name, real_name ? MSGSTR(SETFILES_9, "wildcard ") : "", entry_no);
		}
#if SEC_MAC
		disablepriv(SEC_MULTILEVELDIR);
#endif
		return;
	}

	/*
	 * Check to see if this file has already been processed. If so,
	 * skip to the next entry.
	 */

	if(file_already_done(&stat_buf)) {
#if SEC_MAC
		disablepriv(SEC_MULTILEVELDIR);
#endif
		return;
	}

	good_user = 1;
	if (pr->uflg.fg_uid)
		good_user = stat_buf.st_uid == pr->ufld.fd_uid;

	good_group = 1;
	if (pr->uflg.fg_gid)
		good_group = stat_buf.st_gid == pr->ufld.fd_gid;

	good_mode = 1;
	if (pr->uflg.fg_mode)
		good_mode = (stat_buf.st_mode & 07777) == pr->ufld.fd_mode;

	if (pr->uflg.fg_type) {
		switch (stat_buf.st_mode & S_IFMT) {
		    case S_IFDIR:
#if SEC_MAC
			if (ismultdir(name))
				type = 'm';
			else
				type = 'd';
#else
			type = 'd';
#endif
			break;
		    case S_IFREG:	type = 'r'; break;
		    case S_IFIFO:	type = 'f'; break;
		    case S_IFCHR:	type = 'c'; break;
		    case S_IFBLK:	type = 'b'; break;
#ifdef S_IFSOCK
		    case S_IFSOCK:	type = 's'; break;
#endif
#ifdef S_IFLNK
		    case S_IFLNK:	type = 'l'; break;
#endif
		    default:		type = '?'; break;
		}
		good_type = type == pr->ufld.fd_type[0];
	} else
		good_type = 1;

#if SEC_MAC
	good_slevel = 1;
	if (pr->uflg.fg_slevel) {
		if (mand_ir == (mand_ir_t *) 0)	
			mand_ir = mand_alloc_ir();
		if (mand_ir) {
			ret = statslabel(name, mand_ir);
			if (ret == -1 && errno == EINVAL) {	/* wildcard */
				mand_free_ir(mand_ir);
				mand_ir = (mand_ir_t *) 0;
			}
			if (pr->ufld.fd_slevel == (mand_ir_t *) 0)
				good_slevel = (mand_ir == (mand_ir_t *) 0);
			else
				good_slevel = (mand_ir && 
						memcmp(pr->ufld.fd_slevel,
							mand_ir,
							mand_bytes()) == 0);
		} else
			good_slevel = 0;
	}
#endif
#if SEC_NCAV
	good_ncav = 1;
	if (pr->uflg.fg_ncav) {
		if (ncav_ir == (ncav_ir_t *) 0)	
			ncav_ir = ncav_alloc_ir();
		if (ncav_ir) {
			ret = statncav(name, ncav_ir);
			if (ret == -1 && errno == EINVAL) {	/* wildcard */
				ncav_free_ir(ncav_ir);
				ncav_ir = (ncav_ir_t *) 0;
			}
			if (pr->ufld.fd_ncav == (ncav_ir_t *) 0)
				good_ncav = (ncav_ir == (ncav_ir_t *) 0);
			else
				good_ncav = (ncav_ir &&
					     *pr->ufld.fd_ncav == *ncav_ir);
		} else
			good_ncav = 0;
	}
#endif
#if SEC_ACL_SWARE
	good_acl = 1;
	if (pr->uflg.fg_acl) {
		if (acl == (acle_t *) 0 || alen != pr->ufld.fd_acllen) {
			if (acl)
				free(acl);
			alen = pr->ufld.fd_acllen;
			acl = (acle_t *) malloc(alen * sizeof(acle_t));
		}
		if (acl) {
			ret = statacl(name, acl, alen);
			if (ret == -1 && errno == EINVAL) {
				free(acl);
				acl = (acle_t *) 0;
			}
			if (pr->ufld.fd_acl == ACL_DELETE)
				good_acl = (acl == (acle_t *) 0);
			else
				good_acl = (acl && alen == ret &&
					    memcmp(pr->ufld.fd_acl, acl,
						alen * sizeof(acle_t)) == 0);
		} else
			good_acl = 0;
	}
#endif

#if SEC_PRIV
	good_ppriv = 1;
	if (pr->uflg.fg_pprivs)
		good_ppriv = statpriv(name, SEC_POTENTIAL_PRIV, pprivs) != -1 &&
				memcmp(pr->ufld.fd_pprivs, pprivs,
					sizeof pprivs) == 0;

	good_gpriv = 1;
	if (pr->uflg.fg_gprivs)
		good_gpriv = statpriv(name, SEC_GRANTED_PRIV, gprivs) != -1 &&
				memcmp(pr->ufld.fd_gprivs, gprivs,
					sizeof gprivs) == 0;
#endif

	/*
	 * Collect the new discretionary controls that will be
	 * given to the file.
	 */
	if (good_user)
		new_owner = stat_buf.st_uid;
	else
		new_owner = pr->ufld.fd_uid;
	if (good_group)
		new_group = stat_buf.st_gid;
	else
		new_group = pr->ufld.fd_gid;

	if (good_user && good_group && good_mode && good_type
#if SEC_MAC
	   && good_slevel
#endif
#if SEC_ACL_SWARE
	   && good_acl
#endif
#if SEC_PRIV
	   && good_ppriv && good_gpriv
#endif
#if SEC_NCAV
	   && good_ncav
#endif
	   ) {
		/*
		 * This file checks out.
		 */
		if (verbose)
			printf(MSGSTR(SETFILES_11,
				"%s (%sentry %d) is fine.\n"), name,
				real_name ? MSGSTR(SETFILES_9, "wildcard ") : "", entry_no);
	} else {
		/*
		 * This file misses at least one test.  Notify the user.
		 */
		printf(MSGSTR(SETFILES_12, "%s (%sentry %d) reset"), name,
			real_name ? MSGSTR(SETFILES_9, "wildcard ") : "", entry_no);
		if (!good_user || !good_group) {
			/*
			 * If changing owner or group and the file has either
			 * of the setuid or setgid bits in its mode and the
			 * mode would not otherwise be changed, arrange to
			 * reset the mode after the owner/group change since
			 * the kernel will automatically turn of the
			 * setuid/setgid bits.
			 */
			if (good_mode && (stat_buf.st_mode & 06000)) {
				good_mode = 0;
				pr->ufld.fd_mode = stat_buf.st_mode & 07777;
			}
			if (!good_user)
				printf(MSGSTR(SETFILES_13, " owner"));
			if (!good_group)
				printf(MSGSTR(SETFILES_14, " group"));
			if (!nflag && chown(name, new_owner, new_group) != 0) {
				total_errors++;
				printf(MSGSTR(SETFILES_15, " (failed)"));
			}
		}
		if (!good_mode) {
			printf(MSGSTR(SETFILES_16, " mode"));
			if (!nflag && chmod(name, pr->ufld.fd_mode) != 0) {
				total_errors++;
				printf(MSGSTR(SETFILES_15, " (failed)"));
			}
		}
		if (!good_type) {
#if SEC_MAC
		/*
		 * If the entry is currently a regular directory and it
		 * should be a multilevel directory, try to effect that
		 * change. It will fail if the directory is not empty.
		 */
			if (type == 'd' && pr->ufld.fd_type[0] == 'm') {
				printf(MSGSTR(SETFILES_17, " type"));
				if (!nflag && mkmultdir(name) != 0) {
					total_errors++;
					printf(MSGSTR(SETFILES_15, " (failed)"));
				}
			} else if (type == 'm' && pr->ufld.fd_type[0] == 'd') {
				printf(MSGSTR(SETFILES_17, " type"));
				if (!nflag && rmmultdir(name) != 0) {
					total_errors++;
					printf(MSGSTR(SETFILES_15, " (failed)"));
				}
			}
#else
			printf(MSGSTR(SETFILES_18, " type (failed)"));
#endif
		}
#if SEC_MAC
		if (!good_slevel) {
			printf(MSGSTR(SETFILES_19, " slevel"));
			if (!nflag && chslabel(name, pr->ufld.fd_slevel) != 0) {
				total_errors++;
				printf(MSGSTR(SETFILES_15, " (failed)"));
			}
		}
#endif
#if SEC_NCAV
		if (!good_ncav) {
			printf(MSGSTR(SETFILES_20, " caveats"));
			if (!nflag && chncav(name, pr->ufld.fd_ncav) != 0) {
				total_errors++;
				printf(MSGSTR(SETFILES_15, " (failed)"));
			}
		}
#endif
#if SEC_ACL_SWARE
		if (!good_acl) {
			printf(MSGSTR(SETFILES_21, " acl"));
			if (!nflag && chacl(name, pr->ufld.fd_acl,
						pr->ufld.fd_acllen)) {
				total_errors++;
				printf(MSGSTR(SETFILES_15, " (failed)"));
			}
		}
#endif
#if SEC_PRIV
		if (!good_ppriv) {
			printf(MSGSTR(SETFILES_22, " pprivs"));
			if (!nflag && chpriv(name, SEC_POTENTIAL_PRIV,
						pr->ufld.fd_pprivs) != 0) {
				total_errors++;
				printf(MSGSTR(SETFILES_15, " (failed)"));
			}
		}
		if (!good_gpriv) {
			printf(MSGSTR(SETFILES_23, " gprivs"));
			if (!nflag && chpriv(name, SEC_GRANTED_PRIV,
						pr->ufld.fd_gprivs) != 0) {
				total_errors++;
				printf(MSGSTR(SETFILES_15, " (failed)"));
			}
		}
#endif
		printf("\n");
	}
#if SEC_MAC
	disablepriv(SEC_MULTILEVELDIR);
#endif
}

/*
 * Check to see if the file identified by the inode/device pair has
 * already been processed. This allows specific and wildcard entries
 * to be present in the database with only the first taking effect.
 */

file_already_done(statbuf)
struct stat *statbuf;
{
	register int i;

	if(!fdone)
		return(0);

	for(i=0; i < fdone_count; i++)
		if((fdone[i].inode == statbuf->st_ino) &&
		   (fdone[i].device == statbuf->st_dev))
			return(1);

	/*
	 * Expand the table if not enough room for another entry.
	 */

	if(fdone_count >= fdone_size) {
		if((fdone = (struct files_done *)
		    realloc(fdone, sizeof(struct files_done) * fdone_size * 2))
		    == (struct files_done *) 0) {
			fprintf(stderr,MSGSTR(SETFILES_24,
				"setfiles: can't expand fdone table\n"));
			exit(2);
		}

		fdone_size *= 2;
	}

	fdone[i].inode = statbuf->st_ino;
	fdone[i].device = statbuf->st_dev;
	fdone_count++;
	return(0);
}

/*
 * Compare a pathname pattern with a pathname
 */
matchname(pattern, path)
	char	*pattern, *path;
{
	register int	len;

	/* Provide name match on a prefix directory */

	if(prefix && strlen(path) > prefix_size)
		path += prefix_size;

	len = strlen(pattern);
	if (len)
		--len;
	if (pattern[len] == '*')
		return strncmp(pattern, path, len) == 0;
	else
		return strcmp(pattern, path) == 0;
}
		
static privvec_t saveprivs;

/*
 * Turn on the privileges needed to access and modify file attributes
 */
activate_privs()
{
	if (forceprivs(privvec(SEC_CHOWN, SEC_CHMODSUGID, SEC_OWNER,
				SEC_ALLOWDACACCESS, SEC_CHPRIV, SEC_LOCK,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), saveprivs)) {
		fprintf(stderr, MSGSTR(SETFILES_25,
			"%s: insufficient privileges\n"), command_name);
		exit(1);
	}
#ifdef SEC_MAC
	disablepriv(SEC_MULTILEVELDIR);	/* raised as needed */
#endif
}

/*
 * Reset the effective privileges to the base set
 */
deactivate_privs()
{
	seteffprivs(saveprivs, (priv_t *) 0);
}
#endif /*} SEC_BASE */
