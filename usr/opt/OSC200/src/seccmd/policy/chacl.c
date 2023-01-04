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
static char *rcsid = "@(#)$RCSfile: chacl.c,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/10/07 14:29:48 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	chacl.c,v $
 * Revision 1.1.1.2  92/06/23  01:44:00  devrcs
 *  *** OSF1_1B30 version ***
 * 
 * Revision 1.1.2.4  1992/04/29  17:20:21  valin
 * 	Add set_auth_parameters back in.
 * 	[1992/04/29  17:14:34  valin]
 *
 * Revision 1.1.2.3  1992/04/24  21:49:33  valin
 * 	Set acl_or_label to indciate acls for file_to_buf.
 * 	[1992/04/24  18:27:06  valin]
 * 
 * Revision 1.1.2.2  1992/04/05  12:41:36  marquard
 * 	POSIX chacl program
 * 	[1992/04/05  12:40:41  marquard]
 * 
 * $OSF_EndLog$
 */

/*
 * Copyright (c) 1988-1991 SecureWare, Inc.  All rights reserved.
 *
 *	@(#)chacl.c	1.9 07:30:44 10/15/91  SecureWare
 *
 * This is unpublished proprietary source code of SecureWare, Inc. and
 * should be treated as confidential.
 */

#include <sys/secdefines.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "chacl_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_CHACL,n,s)
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

#if SEC_ACL_POSIX

#include <stdio.h>
#include <sys/errno.h>
#include <sys/security.h>
#include <sys/stat.h>
#include <sys/mode.h>
#include <acl.h>

int	aflag,bflag,cflag,dflag,pflag,fflag,rflag,Rflag,uflag,Uflag,xflag;
char	**args;

int	file1, rentry, file2, uentry, file3, ffile;
int	lfile;

#if SEC_ACL_POSIX

/* 
 * Set acl_or_label to be 1 indicating to filetobuf that it is dealing with
 * acls not labels.
 */

int acl_or_label = 1;

#endif

int	acl_type = ACL_TYPE_ACCESS;

extern acl_entry_t	malloc();

acl_t		acl_rd,	acl_wr, acl_new, acl_tx;
acl_entry_t	fent = (acl_entry_t)NULL;
acl_entry_t	uent = (acl_entry_t)NULL;
acl_entry_t	Uent = (acl_entry_t)NULL;
acl_entry_t	rent = (acl_entry_t)NULL;
acl_entry_t	Rent = (acl_entry_t)NULL;

static void	usage ();
static int	check_flags ();
static void	replace_from_file ();
static void	delete_default ();
static void	delete_extended ();
static int	remove_extended_entries();
static int	update_entries ();
static int	update_acl_entries ();
static void	purge_perms ();
static void	add_perms ();
static int	calculate_mask ();
static void	get_write_acl ();
static int	more_permissive ();
static int	remove_acl ();
static int	update_new_entries ();
static int	update_acl ();
static int	right_flags();
static void	copy_entries ();
static int	alloc_entries ();
static acl_entry_t	get_command_line_entries ();
static acl_entry_t	get_file_entries ();

/*
 *
 * chacl command, changes ACL of files.
 *	Usage:  chacl [-a] directoryname(s)
 *		chacl [-b] [-c] [-d] [-p] [-f file1] [-r entries] 
 *			[-R file2] [-u entries] [-U file3] [-x] filename(s)
 *
 *	-a	Delete a default ACL.
 *	-b	Remove all entries except the base entries:
 *			USER_OBJ, GROUP_OBJ, OTHER_OBJ.  If the
 *			ACL had a mask, then GROUP_OBJ is intersected
 *			with the old value of the mask.
 *	-c	Calculate the MASK_OBJ permissions and update the mask.
 *	-d	The operation applies to the default ACL instead of
 *			the access ACL.
 *	-p	Purge the ineffective permissions from the file group
 *			class entries that were NOT specified with the
 *			-u or -f options by intersecting them with the
 *			old value of the mask.
 *	-f	Replace the existing ACL with the contents of file1.
 *	-r	Remove the specified ACL entries.
 *	-R	Remove the ACL entries specified in file2.
 *	-u	Update the specified ACL entries.  The entries are added
 *			if they do not already exist.
 *	-U	Update the ACL entries specified in file3.  They are added
 *			if they do not already exist.
 *	-x	Do not perform mask calculation.
 *
 * Basically how this command works:
 *
 *	1. Allocate three working acl storage buffers: acl_rd, acl_wr, acl_new.
 *	2. Read the file's original acl into acl_rd.
 *	3. If the -f flag is specified, read the file specified into to acl_wr
 *		buffer, else copy the acl_rd buffer into the acl_wr buffer.
 *	4. Parse all the command line options and make update and deletions
 *		on the acl_wr buffer as well as saving all the new or
 *		updated entries in the acl_new buffer.
 *	5. Calculate the mask based on the three working buffers.
 *	6. Save the mask in the acl_wr buffer and write it out to the file.
 *
 *	Example:
 *		chacl -f file1 -r user:tomg:r-x -u group:adm:r-- testfile
 *
 *		The chacl command will read the testfile's current acl
 *			into the acl_rd buffer.  It will then read
 *			file1 into the acl_wr buffer.  It will then store
 *			the files entries, the user:tomg:r-x entry, and
 *			the group:adm:r-- entry in the 	acl_new buffer
 *			and make the appropriate updates and deletions
 *			from the acl_wr buffer.  Perform the mask
 *			calculations based on the entries stored
 *			in the three buffers. Write the acl_wr buffer
 *			out to the file.
 *	
 *	The order in which the flags are evaluated is the following:
 *		1. -f - replace entries from a file.
 *		2. -p - purge all ineffective permissions.
 *		3. -U - update entries from a file.
 *		4. -u - update entries from the command line.
 *		5. -R - remove entries from a file.
 *		6. -r - remove entries from the command line.
 *		7. mask calculation:
 *			-x - do not perform mask calculation.
 *			-c - perform union mask calculation.
 *		8. -b - only keep the base entries.
 *	
 *		The -d flag marks the operation as a default operation.
 *		The -a removes the default acl and must be specified alone.
 *
 */

main(argc,argv)
int	argc;
char	*argv[];
{
	int	c;
	extern	int optind, opterr;
	extern	char *optarg;
	acl_t	acl;
	int	errflag = 0;

#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif
#ifdef MSG
        catd = catopen(MF_CHACL,NL_CAT_LOCALE);
#endif

	/* check arguments */
	if (argc < 3 || *argv[1] != '-')
		usage();

	args = &argv[0];
	set_auth_parameters(argc,argv);
	opterr = 0;
	while ((c = getopt (argc, argv, "abcdpf:r:R:u:U:x")) != -1) {
	   switch (c) {

	      case 'a':
			aflag++;
			break;
	      case 'b':
			bflag++;
			break;
	      case 'c':
			cflag++;
			break;
	      case 'd':
			dflag++;
			acl_type = ACL_TYPE_DEFAULT;
			break;
	      case 'p':
			pflag++;
			break;
	      case 'f':
			fflag++;
			file1 = optind - 1;
			break;
	      case 'r':
			rflag++;
			rentry = optind - 1;
			break;
	      case 'R':
			Rflag++;
			file2 = optind - 1;
			break;
	      case 'u':
			uflag++;
			uentry = optind - 1;
			break;
	      case 'U':
			Uflag++;
			file3 = optind - 1;
			break;
	      case 'x':
			xflag++;
			break;
	      case '?':
	      default :
			usage();
	      }
	}

	/* mark the first and last file */
	ffile = optind;
	lfile = argc - 1;

	if (check_flags ())
		usage();

	if (aflag)
		delete_default ();

	get_write_acl();

	exit (0);
}

/*
 * FUNCTION:
 *	get_write_acl ()
 *
 * ARGUMENTS:
 *	acl_entry_t	nent;    - The new acl entries.
 *	int		numents; - The number of entries.
 *	char		*file;   - The file we are updating.
 *
 * DESCRIPTION:
 *	The get_write_acl () function decides which operations
 *	to perform based on the flags passed.  It reads the
 *	current acl into the read acl buffer, it performs
 *	the operation (which will fill the write and new
 *	acl buffers), calls the mask calculation function,
 *	and then calls acl_write() to write the acl_wr
 *	buffer out to the data base.
 *
 * RETURNS:
 *	Exits with number of errors.
 */

static void
get_write_acl ()
{
acl_entry_t	ent;
register int	i,errflag = 0;
register int	f;
int		usize = 0,Usize = 0,rsize = 0,Rsize = 0;

	/* allocate the read, write and new acl buffers */
	if (acl_alloc(&acl_rd) || acl_alloc(&acl_wr) || acl_alloc(&acl_new)) {
		fprintf (stderr, MSGSTR (MALLOCERR,
			"%s: Memory allocation error.\n"),args[0]);
		exit (1);
	}

	for (f=ffile; f <= lfile; f++) {

		/* clear read and write buffer */
		acl_wr->acl_num = 0;
		acl_rd->acl_num = 0;

		/* read original ACL */
		raise_aclpriv();
		if (acl_read (args[f], acl_type, acl_rd) == -1) {
			fprintf (stderr, MSGSTR (UPDTACLERR,
		  		"%s: Error updating ACL for \"%s\""),
				args[0],args[f]);
			perror (" ");
			errflag++;
			continue;
		}
		lower_aclpriv();

		/*
		 * If setting or getting a default acl and
		 * the directory currently does not have a
		 * default acl, initialize the base entries.
		 */
		if (acl_rd->acl_num == 0  && acl_type == ACL_TYPE_DEFAULT) { 

			if(get_wildcard_acl(acl_wr, args[f])) {
				fprintf (stderr, MSGSTR (UPDTACLERR,
		  		"%s: Error updating ACL for \"%s\""),
					args[0],args[f]);
				perror (" ");
				errflag++;
				continue;
			}
		}

		/* fflag must be first */
		if (fflag)
			replace_from_file(args[f]);

		/*
		 * If p flag is specified, clear all ineffective
		 * permissions from the original ACL.
		 */
		if (pflag)
			purge_perms();

		/* update from file */
		if (Uflag) {

			/* read given file entries into acl structure */
			if (!Uent) {
				ent = get_file_entries (file3, &Usize);
				if (ent == (acl_entry_t)NULL)
					exit (1);

				/* Transfer global buffer to our own. */
				if (alloc_entries (ent, &Uent, Usize)) {
					fprintf (stderr, MSGSTR (MALLOCERR,
					    "%s: Memory allocation error.\n"),
					    args[0]);
					exit (1);
				}

				/* put all new entries into the new acl buf */
				if (update_new_entries (Uent, Usize, args[f])) {
					errflag++;
					continue;
				}
			}

			/* update all new entries into the write acl buf */
			if (update_acl_entries (Uent, Usize, args[f])) {
				errflag++;
				continue;
			}
		}

		/* then update from command line */
		if (uflag) {

			/* Translate command line string into acl structure */
			if (!uent) {
				ent = get_command_line_entries (uentry,&usize);
				if (ent == (acl_entry_t)NULL)
					exit (1);

				/* Transfer global buffer to our own. */
				if (alloc_entries (ent, &uent, usize)) {
					fprintf (stderr, MSGSTR (MALLOCERR,
					    "%s: Memory allocation error.\n"),
					    args[0]);
					exit (1);
				}

				/* put all new entries into the new acl buf */
				if (update_new_entries (uent, usize, args[f])) {
					errflag++;
					continue;
				}
			}

			/* collect all new entries into the write acl buf */
			if (update_acl_entries (uent, usize, args[f])) {
				errflag++;
				continue;
			}
		}

		/* then remove from file */
		if (Rflag) {

			/* read given file entries into acl structure */
			if (!Rent) {

				ent = get_file_entries (file2,&Rsize);
				if (ent == (acl_entry_t)NULL)
					exit (1);

				/* Transfer global buffer to our own. */
				if (alloc_entries (ent, &Rent, Rsize)) {
					fprintf (stderr, MSGSTR (MALLOCERR,
					    "%s: Memory allocation error.\n"),
					    args[0]);
					exit (1);
				}
			}

			/* Remove from acl_wr and acl_new. */
			if (remove_acl_entries (Rent,Rsize,args[f])) {
				errflag++;
				continue;
			}
		}

		/* then remove from command line */
		if (rflag) {

			/* Translate command line string into acl structure */
			if (!rent) {

				ent = get_command_line_entries(rentry,&rsize);
				if (ent == (acl_entry_t)NULL)
					exit (1);

				/* Transfer global buffer to our own. */
				if (alloc_entries (ent, &rent, rsize)) {
					fprintf (stderr, MSGSTR (MALLOCERR,
					    "%s: Memory allocation error.\n"),
					    args[0]);
					exit (1);
				}
			}

			/* Remove from acl_wr and acl_new. */
			if (remove_acl_entries (rent,rsize,args[f])) {
				errflag++;
				continue;
			}
		}

		/* set mask */
		if (calculate_mask (args[f])) {
			errflag++;
			continue;
		}

		/* If we only want the base */
		if (bflag) {
			if (remove_extended_entries (args[f])) {
				errflag++;
				continue;
			}
		}

		/* set acl on file */
		raise_aclpriv();
		if (acl_write (args[f], acl_type, acl_wr)) {

			fprintf (stderr, MSGSTR (UPDTACLERR,
		  		"%s: Error updating ACL for \"%s\""),
   		  		args[0],args[f]);
			perror (" ");
			errflag++;
		}
		lower_aclpriv();
	}

	exit (errflag);
}

/*
 * FUNCTION:
 *	replace_from_file ()
 *
 * ARGUMENTS:
 *	char	*file;	- the file to read entries from.
 *
 * DESCRIPTION:
 *	The replace_from_file () function calls get_file_entries()
 *	to read and convert the file, checks for relative
 *	permissions, initializes the write and new acl buffers.
 *
 * RETURNS:
 *	Exits on failure.
 */

static void
replace_from_file (file)
char		*file;
{
	static int		size;
	register int		i;
	register acl_entry_t	ent, entp;

	if (!fent) {
		/* read given file entries into acl structure */
		if((ent = get_file_entries (file1,&size)) == (acl_entry_t)NULL)
			exit (1);

		/*
		 * If -f is used, we cannot accept relative
		 * permissions because we cannot replace a permission
		 * with a relative permission.
		 */
		entp = ent;
		for (i=0; i < size; i++) {
			if (entp->acl_perm & ACL_PADD ||
				entp->acl_perm & ACL_PREMOVE) {
			  fprintf (stderr, MSGSTR (RELATIVEERR,
			  "%s: Cannot accept relative permissions.\n"),args[0]);
			  exit (1);
			}
			entp = entp->acl_next;
		}

		if (alloc_entries (ent, &fent, size)) {
			fprintf (stderr, MSGSTR (MALLOCERR,
			    "%s: Memory allocation error.\n"),
			    args[0]);
			exit (1);
		}

		/* Copy new entries into acl_new buffer. */
		if (check_wk_space (acl_new, size)) {
			fprintf (stderr, MSGSTR (MALLOCERR,
				"%s: Memory allocation error.\n"),args[0]);
			exit (1);
		}

		copy_entries (fent, acl_new->acl_first, size);

		acl_new->acl_num = size;
	}

	/* write new entries into acl_wr buffer */
	if (check_wk_space (acl_wr, size)) {
		fprintf (stderr, MSGSTR (MALLOCERR,
			"%s: Memory allocation error.\n"),args[0]);
		exit (1);
	}

	copy_entries (fent, acl_wr->acl_first, size);

	acl_wr->acl_num = size;

	return;
}

/*
 * FUNCTION:
 *	update_new_entries ()
 *
 * ARGUMENTS:
 *	acl_entry_t	nents;   - the new acl entries to be updated.
 *	int		numents; - number of acl entries.
 *	char		*file;   - the file name for messages.
 *
 * DESCRIPTION:
 *	The update_new_entries() function checks the size of the
 *	new entry acl.  If it is not yet initialized, the acl_new
 *	acl is created and the new entries are copied in.  If it
 *	has already been initialized, the new entries are added
 *	by calling update_acl().
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1).
 *		If the update fails.
 *	Exits if memory allocation error.
 */

static int
update_new_entries (nents, numents, file)
acl_entry_t	nents;
int		numents;
char		*file;
{
	register acl_entry_t	ent;
	register int 		i;

	/*
	 * If the new acl entry buffer is not initialized,
	 * copy the new entries in.
	 */

	if (acl_new->acl_num == 0) {

		if (check_wk_space (acl_new, numents)) {
			fprintf (stderr, MSGSTR (MALLOCERR,
				"%s: Memory allocation error.\n"),args[0]);
			exit (1);
		}

		copy_entries (nents, acl_new->acl_first, numents);

		acl_new->acl_num = numents;

		/* Clear relative permissions. */
		ent = acl_new->acl_first;
		for (i=0; i < numents; i++) {
			ent->acl_perm &= ~(ACL_PADD | ACL_PREMOVE);
			ent = ent->acl_next;
		}

		return 0;
	}

	/* Add the new entries to the new acl buffer. */
	if (update_acl (acl_new,numents,nents,file,0))
		return -1;

	return 0;
}

/*
 * FUNCTION:
 *	update_acl_entries ()
 *
 * ARGUMENTS:
 *	acl_entry_t	nents;   - the new acl entries to be updated.
 *	int		numents; - number of acl entries.
 *	char		*file;   - the file name for messages.
 *
 * DESCRIPTION:
 *	The update_acl_entries() function checks the size of the
 *	write entry acl.  If it is not yet initialized, the acl_wr
 *	acl buffer is created and the new entries are copied in.
 *	If it has already been initialized, the new entries are added
 *	by calling update_acl().
 *
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1).
 *		If the update fails.
 *	Exits if memory allocation error.
 */

static int
update_acl_entries (nents, numents, file)
acl_entry_t	nents;
int		numents;
char		*file;
{
	register int i;

	/*
	 * If acl_wr is NULL allocate it here.
	 */
	if (acl_wr->acl_num == 0) {

		if (check_wk_space (acl_wr, acl_rd->acl_num + numents)) {
			fprintf (stderr, MSGSTR (MALLOCERR,
				"%s: Memory allocation error.\n"),args[0]);
			exit (1);
		}

		copy_entries (acl_rd->acl_first, acl_wr->acl_first,
				acl_rd->acl_num);

		acl_wr->acl_num = acl_rd->acl_num;
	}

	/*
	 * If acl_wr is not NULL we have already
	 * stuffed the write acl buffer so just check size.
	 */
	else {
		if (check_wk_space (acl_wr, numents)) {
			fprintf (stderr, MSGSTR (MALLOCERR,
				"%s: Memory allocation error.\n"),args[0]);
			exit (1);
		}
	}

	/* Add the new entries to the acl_wr buffer. */
	if (update_acl (acl_wr,numents,nents,file,1))
		return -1;

	return 0;
}

/*
 * FUNCTION:
 *	update_acl ()
 *
 * ARGUMENTS:
 *	acl_t		acl;	  - the acl to update.
 *	int		numents;  - number of acl entries.
 *	acl_entry_t	nents;    - the acl entries to update with.
 *	char		*file;    - the file name for messages.
 *	int		maskcalc; - indicates whether to add mask entry.
 *
 * DESCRIPTION:
 *	The update_acl() function updates the current
 *	acl (acl_wr or acl_new) with the new entries
 *	specified in nents.  The maskcalc flag is passed
 *	to indicate a mask entry generation is not necessary,
 *	which is true in the case of acl_new.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1).
 *		If new entry cannot be created.
 *		If entry contains relative permissions and
 *			the entry does not currently exist.
 */

static int
update_acl (acl,numents,nents,file,maskcalc)
acl_t		acl;
int		numents;
acl_entry_t	nents;
char		*file;
int		maskcalc;
{
acl_entry_t		newent;
register int		i,j,addflag = 0;
register acl_entry_t	cent;

	/*
	 * For each entry in the new entries (nents)
	 * find the entry in the current acl (acl and cent)
	 * and update it with the new entry.
	 */
	for (i=0; i < numents; i++) {

	   cent = acl->acl_first;

	   for (j=0; (j < acl->acl_num && !addflag); j++) {
	
		if (nents->acl_tag == cent->acl_tag) {

			switch (nents->acl_tag) {

			    case USER_OBJ:
			    case GROUP_OBJ:
			    case OTHER_OBJ:
			    case MASK_OBJ:

				add_perms (nents,cent);
				addflag++;
				break;

			    case USER:
				if (nents->acl_uid != cent->acl_uid)
					break;
				add_perms (nents,cent);
				addflag++;
				break;

			    case GROUP:
				if (nents->acl_gid != cent->acl_gid)
					break;
				add_perms (nents,cent);
				addflag++;
				break;
			}
		}
		cent = cent->acl_next;
	   }

	   /* If the entry isn't in the original ACL, add it. */
	   if (!addflag) {

		/* cannot add a relative permission */
		if (nents->acl_perm & ACL_PADD ||
		    nents->acl_perm & ACL_PREMOVE) {
			fprintf (stderr, MSGSTR (RELATIVEERR,
	  		 "%s: Cannot accept relative permissions.\n"),args[0]);
			return -1;
		}

		/*
		 * If original ACL did not have extended
		 * entries and we are adding one, add a
		 * mask entry.
		 */
		if ((acl->acl_num == 3) && maskcalc &&
		    (nents->acl_tag == USER || nents->acl_tag == GROUP)) {

			if (acl_create_entry (acl,&newent)) {
				fprintf (stderr, MSGSTR (UPDTACLERR,
				  "%s: Error updating ACL for \"%s\""),
					args[0],file);
				perror (" ");

				return -1;
			}
			newent->acl_perm = nents->acl_perm;
			newent->acl_tag = MASK_OBJ;
		}

		/* Add new entry to the acl structure. */
		if (acl_create_entry (acl,&newent)) {
			fprintf (stderr, MSGSTR (UPDTACLERR,
				"%s: Error updating ACL for \"%s\""),
				args[0],file);
			perror (" ");

			return -1;
		}
		(void) acl_copy_entry (nents,newent);
	   }

	   addflag = 0;

	   nents = nents->acl_next;

	}

	return 0;
}

/*
 * FUNCTION:
 *	remove_acl_entries ()
 *
 * ARGUMENTS:
 *	acl_entry_t	nents;   - the acl entries to be updated.
 *	int		numents; - number of acl entries.
 *	char		*file;   - the file name for messages.
 *
 * DESCRIPTION:
 *	The remove_acl_entries() function calls purge_perms() if
 *	if the pflag is set, initializes the acl_wr and acl_new
 *	acl buffers, and calls remove_acl() to remove the
 *	specified entries from the acl.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1).
 *		If remove_acl() fails.
 *	Exits on memory allocation error.
 */

int
remove_acl_entries (nents,numents,file)
acl_entry_t	nents;
int		numents;
char		*file;
{
register int	i;

	/*
	 * Initialize the write ACL buffer.
	 */

	if (acl_wr->acl_num == 0) {

		if (check_wk_space (acl_wr, acl_rd->acl_num)) {
			fprintf (stderr, MSGSTR (MALLOCERR,
				"%s: Memory allocation error.\n"),args[0]);
			exit (1);
		}

		copy_entries (acl_rd->acl_first, acl_wr->acl_first,
				acl_rd->acl_num);

		acl_wr->acl_num = acl_rd->acl_num;
	}

	/*
	 * If acl_new is not initialized,
	 * initialize it and copy in the new entries.
	 */
	if (acl_new->acl_num == 0) {

		if (check_wk_space (acl_new, numents)) {
			fprintf (stderr, MSGSTR (MALLOCERR,
				"%s: Memory allocation error.\n"),args[0]);
			exit (1);
		}

		copy_entries (nents, acl_new->acl_first, numents);

		acl_new->acl_num = numents;
	}

	/*
	 * If acl_new is initialized, remove the
	 * entries from the acl_new buffer.
	 */
	else {
		if (remove_acl (acl_new, numents, nents, file, 0))
			return -1;
	}

	if (remove_acl (acl_wr, numents, nents, file, 1))
		return -1;

	return 0;
}

/*
 * FUNCTION:
 *	remove_acl ()
 *
 * ARGUMENTS:
 *	acl_t		acl;	 - the acl structure to be removed.
 *	int		numents; - number of acl entries.
 *	acl_entry_t	nents;   - the acl entries to be updated.
 *	char		*file;   - the file name for messages.
 *	int		fail;	 - indicates whether to fail or not.
 *
 * DESCRIPTION:
 *	The remove_acl () function finds the entry to
 *	be removed in the current acl of the file and deletes
 *	it from the entry list.  The fail flag is only set when
 *	deleting from the acl_new buffer.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1).
 *		If we are trying to delete a base entry.
 *		If the entry is not found and fail is set.
 *		If the entry is a mask and there are extended entries.
 */


static int
remove_acl (acl, numents, nents, file, fail)
acl_t		acl;
int		numents;
acl_entry_t	nents;
char		*file;
int		fail;
{
register int		i,j,delflag = 0,dmask = 0;
register acl_entry_t	ent = nents;
register acl_entry_t	cent;

	/*
	 * For each entry in the remove entries
	 * find the entry in the current acl.
	 * Check to make sure this entry is the one we want
	 * and then call acl_delete_entry () to remove it
	 * from the ACL.
	 */
	for (i=0; i < numents; i++) {

	   cent = acl->acl_first;

	   for (j=0; (j < acl->acl_num ); j++) {
	
		if (ent->acl_tag == cent->acl_tag) {

		   switch (ent->acl_tag) {

		    /*
		     * Cannot delete the base entries.
		     */
		    case USER_OBJ:
		    case GROUP_OBJ:
		    case OTHER_OBJ:

		     if (fail && !bflag) {
			     fprintf (stderr, MSGSTR (DELBASE,
		 	     "%s: Cannot remove base entry for \"%s\".\n"),
					args[0],file);
				return -1;
		     }

		    case MASK_OBJ:

			/*
			 * Mark a mask deletion to see
			 * if we can do this later.
			 */
			if (fail && !bflag)
				dmask++;

			if (acl_delete_entry (cent))
				goto out;
			delflag++;
			break;

		    case USER:

			if (ent->acl_uid != cent->acl_uid )
				break;

			if (acl_delete_entry (cent))
				goto out;
			delflag++;
			break;

		    case GROUP:

			if (ent->acl_gid != cent->acl_gid) 
				break;

			if (acl_delete_entry (cent))
				goto out;
			delflag++;
			break;
		   }
		}
	        cent = cent->acl_next;
	   }
	   ent = ent->acl_next;
	}

	/*
	 * Make sure that all the entries we
	 * wanted to delete have been found.
	 */
	if (fail) {
		if (delflag != numents) {
			errno = EINVAL;
			goto out;
		}
	}

	/*
	 * We cannot delete a mask entry
	 * when extended entries exist.
	 */
	if (fail && dmask && (acl->acl_num > 3)) {
		fprintf (stderr, MSGSTR (DELMASKERR,
			"%s: Cannot remove mask entry for \"%s\".\n"),
			args[0],file);
		return -1;
	}

	return 0;

out:
	fprintf (stderr, MSGSTR (DELENTERR,
		"%s: Error deleting entry for \"%s\""),
		args[0],file);
	perror (" ");

	return -1;
}

/*
 * FUNCTION:
 *	calculate_mask ()
 *
 * ARGUMENTS:
 *	char	*file; - the file to be changed.
 *
 * DESCRIPTION:
 *	The calculate_mask () function calculates the mask
 *		for the updated acl.
 *
 *	It works off of three acl buffers:
 *		1. acl_rd - the original acl before any changes.
 *		2. acl_wr - the candidate acl to be written out to the
 *			file.  This contains the original acl and any
 *			additions and deletions that may have taken place.
 *			In the case of -f it contains the new entries instead
 *			of the contents of acl_rd.
 *		3. acl_new - all the new additions and deletions.  This is
 *			used to calculate the default mask.
 *
 *  MASK CALCULATION:
 *
 *	- If user explicitly specifies a new mask value,
 *		then chacl will use it.
 *
 *	-x option:
 *		The ACL will be modified exactly as specified by the
 *		user.  No mask calculation or purging of permissions
 *		will occur.
 *
 *	-c option:
 *		The mask will be set to the union of the permissions
 *		granted to the file group class entries on the ACL.
 *		This option could result in inadvertently granting 
 *		extra permissions and an error message will be printed.
 *
 *	-p option:
 *		The command intersects the old value of the mask with
 *		all of the existing, unmodified entries in the file
 *		group class.  Then it will modify or add the new
 *		entries and compute a new value for the mask.
 *		If this flag is set, the purging takes place before 
 *		acl_wr is built.
 *
 *	- if no flags, chacl will use the following algorithm to
 *		calculate the mask:
 *
 *		- For all file group class entries, determine a
 *			candidate new mask value by calculating
 *			the union of all newly granted permissions
 *			and the original effective permissions that have
 *			not explicitly been removed.  
 *
 *		- If there are any permissions in the candidate mask
 *			that are among the original effective permissions
 *			not removed, then applying the candidate new
 *			mask would unexpectedly grant some new right that
 *			the user did not intend.
 *			This condition shall generate an error and the
 *			ACL will not be modified.
 *			If this condition does not hold, then apply the
 *			candidate new mask as the new mask.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1).
 *		If granting additional privileges unexpectedly.
 */

static int
calculate_mask (file)
char		*file;
{
register acl_entry_t	ent;
register acl_entry_t	nent;
register int 		i,j;
acl_permset_t		nmask = (acl_permset_t)NULL;
acl_permset_t		omask = (acl_permset_t)NULL;
acl_permset_t		candidate = (acl_permset_t)NULL;
acl_entry_t		maskptr = (acl_entry_t)NULL;
int			pwarn = 0;

	if (bflag && acl_wr->acl_num == 0) {
		/*
		 * Initialize the write ACL buffer.
		 */
		if (check_wk_space (acl_wr, acl_rd->acl_num)) {
			fprintf (stderr, MSGSTR (MALLOCERR,
				"%s: Memory allocation error.\n"),args[0]);
			exit (1);
		}
		copy_entries (acl_rd->acl_first, acl_wr->acl_first,
				acl_rd->acl_num);

		acl_wr->acl_num = acl_rd->acl_num;
	}

	/*
	 * If user specified a mask, use the
	 * write ACL as final.
	 * If user has specified the -c or -x flags,
	 * exit with error.
	 */
	if (right_flags ()) {
		nent = acl_new->acl_first;
		for (i=0; i < acl_new->acl_num; i++) {
			if (nent->acl_tag == MASK_OBJ) {
			  if (cflag || xflag) {
		    	     fprintf (stderr, MSGSTR (ADDMASKERR,
		    	     "%s: Cannot specify mask and -c or -x flags.\n"),
					args[0]);
				exit (1);
			  }
				
			  return 0;
			}
			nent = nent->acl_next;
		}
	}

	/*
	 * x option:
	 * 	No mask calculation, write out acl as is.
	 */
	if (xflag)
		return 0;

	/* Get the original effective permissions. */
	ent = acl_rd->acl_first;
	for (i=0; i < acl_rd->acl_num; i++) {

		/*
		 * If there are only three entries,
		 * the effective file group class permissions
		 * are the GROUP_OBJ permissions.
		 */
		if (acl_rd->acl_num == 3 && ent->acl_tag == GROUP_OBJ) {
			omask = ent->acl_perm;
			continue;
		}

		if (ent->acl_tag == MASK_OBJ) {
			omask = ent->acl_perm;
			continue;
		}

		ent = ent->acl_next;
	}

	/*
	 * Get pointer to the mask
	 * entry in the write acl buffer.
	 */
	ent = acl_wr->acl_first;
	for (i=0; i < acl_wr->acl_num; i++) {

		if (ent->acl_tag == MASK_OBJ) {
			maskptr = ent;
			break;
		}

		ent = ent->acl_next;
	}

	/*
 	 * c option:
	 * 	Set the mask to the union of the permissions
	 * 	granted to the file group class entries on the ACL.
	 *	Print an error message when granting extra permissions.
	 */
	if (cflag) {

		/*
		 * Calculate the new mask from
		 * the acl_wr acl buffer which contains
		 * the acl with all the updated entries.
		 */
		ent = acl_wr->acl_first;
		for (i=0; i < acl_wr->acl_num; i++) {

			switch (ent->acl_tag) {

				case USER: case GROUP: case GROUP_OBJ:
					nmask |= ent->acl_perm;
			}
			ent = ent->acl_next;
		}

		/*
		 * Issue a warning if granting more
		 * permissions to any previous entry.
		 */
		if (more_permissive(nmask,omask)) {
		    fprintf (stderr, MSGSTR (ADDWARN,
		    "%s: Warning: changing \"%s\" to be more permissive.\n"),
			args[0],file);
		}

		/* Create mask entry if there isn't one. */
		if (!maskptr) {
			if (acl_create_entry (acl_wr, &maskptr))
				return -1;
			maskptr->acl_tag = MASK_OBJ;
		}

		/* Set new mask permissions */
		maskptr->acl_perm = nmask;

		return 0;
	}

	/*
	 * If there are only three entries in
	 * the acl we don't need a mask.
	 */
	if (acl_wr->acl_num == 3)
		return 0;

	/* Create mask entry if there isn't one. */
	if (!maskptr) {
		if (acl_create_entry (acl_wr, &maskptr))
			return -1;
		maskptr->acl_tag = MASK_OBJ;
	}

	/*
	 * Calculate the union of all newly granted permissions ...
	 */

	if (right_flags ()) {
		ent = acl_new->acl_first;
		for (i=0; i < acl_new->acl_num; i++) {
			switch (ent->acl_tag) {
				case USER: case GROUP: case GROUP_OBJ:
					candidate |= ent->acl_perm;
			}
			ent = ent->acl_next;
		}
	}

	/*
	 * ... and the original "effective" permissions that have
	 * not been removed.
	 */

	ent = acl_wr->acl_first;
	for (i=0; i < acl_wr->acl_num; i++) {

		switch (ent->acl_tag) {

		   case USER: case GROUP: case GROUP_OBJ:

			/*
			 * If this entry has been already
			 * counted above, skip it.
			 * This may seem a bit inefficient,
			 * but the new entries have already been
			 * copied into the acl_wr acl buffer and
			 * it is a way to differentiate the
			 * new entries from the old.
			 */
			if (right_flags ()) {
				nent = acl_new->acl_first;
				for (j=0; j < acl_new->acl_num; j++) {
					if ((ent->acl_tag == nent->acl_tag) &&
				   	((ent->acl_uid == nent->acl_uid) ||
				    	(ent->acl_gid == nent->acl_gid)) &&
				    	(ent->acl_perm == nent->acl_perm))
						break;
				}
				nent = nent->acl_next;
			}

			/*
			 * Set only the "effective" permissions.
			 * ie. masked by the old mask.
			 */
			candidate |= (ent->acl_perm & omask);
		}
		ent = ent->acl_next;
	}

	/*
	 * - If there are any permissions in the candidate mask
	 *	that are among the original effective permissions
	 *	not removed, then applying the candidate new
	 *	mask would unexpectedly grant some new right that
	 *	the user did not intend.
	 */

	if (more_permissive(candidate,omask)) {
		    fprintf (stderr, MSGSTR (ADDPERMERR,
		    "%s: Cannot change \"%s\" to be more permissive.\n"),
			args[0],file);
		return -1;
	}

	/* Set new mask permissions */
	maskptr->acl_perm = candidate;

	return 0;
}

/*
 * If rflag, Rflag, or bflag are
 * specified without uflag, Uflag, or fflag
 * it is not necessary to include acl_new in mask calculations.
 */
static int
right_flags()
{
	/* If we are only removing or doing base calculation */
	if ((rflag || Rflag || bflag) && !uflag && !Uflag && !fflag)
		return 0;
	return 1;
}

/*
 * FUNCTION:
 *	more_permissive ()
 *
 * ARGUMENTS:
 *	acl_permset_t	nmask; - the new mask value.
 *	acl_permset_t	omask; - the old mask value.
 *
 * DESCRIPTION:
 *	The more_permissive () function checks to see
 *	whether the new mask will be more permissive
 *	than the previous mask in that it will grant
 *	additional access to a particular entry that
 *	wasn't previously available.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1).
 *		If granting additional permissions to an entry.
 */

static int
more_permissive (nmask,omask)
acl_permset_t	nmask;
acl_permset_t	omask;
{
register int 		i;
register acl_entry_t	ent;
acl_permset_t		odiff = (acl_permset_t)NULL;
acl_permset_t		ndiff = (acl_permset_t)NULL;

	/* See if new mask is more permissive */
	if ((omask & nmask) != nmask) {

		/*
		 * Check to see if we are granting any
		 * new permissions to extended entries.
		 */
		ent = acl_rd->acl_first;
		for (i=0; i < acl_rd->acl_num; i++) {

		   switch (ent->acl_tag) {

		      case USER: case GROUP: case GROUP_OBJ:

			/*
			 * If entry permissions aren't more permissive
			 * than the original mask, then continue.
			 */
			odiff = (omask ^ ent->acl_perm) & ent->acl_perm;
			if (odiff == 0)
				break;

			/*
			 * If entry permissions are more permissive
			 * then old mask, see if we are adding
			 * permissions with the new mask.
			 */
			ndiff = (nmask ^ ent->acl_perm) & ent->acl_perm;

			if (odiff != ndiff)
				return -1;
		   }
		   ent = ent->acl_next;
		}

	}
	return 0;
}

/*
 * FUNCTION:
 *	delete_default ()
 *
 * ARGUMENTS:
 *	none.
 *
 * DESCRIPTION:
 *	The delete_default() function deletes the default
 *	ACL's associated with the passed files.
 *
 * RETURNS:
 *	none.
 */

static void
delete_default ()
{
acl_t		acl;
register int errflag = 0;
register int	f;

	if (acl_alloc (&acl)) {
		fprintf (stderr, MSGSTR (MALLOCERR,
			"%s: Memory allocation error.\n"),args[ffile]);
		exit (1);
	}

	for (f=ffile; f <= lfile; f++) {

		/* write a NULL acl out to the file */
		raise_aclpriv();
		if (acl_write (args[f],ACL_TYPE_DEFAULT,acl)) {
			fprintf (stderr, MSGSTR (DELDEFERR,
			 "%s: Error deleting default acl for \"%s\".\n"),
   			  args[0],args[f]);
		   errflag++;
		}
		lower_aclpriv();
	}
	exit (errflag);
}

/*
 * FUNCTION:
 *	remove_extended_entries()
 *
 * ARGUMENTS:
 *	char	*file;	- the file to remove entries from.
 *
 * DESCRIPTION:
 *	The remove_extended_entries() function removes all
 *	but the USER_OBJ, GROUP_OBJ, OTHER_OBJ entries from
 *	the indicated file.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1).
 *		If the read fails.
 *		If unable to allocate space.
 *		If the write fails.
 *		If format of ACL is bad.
 */

static int
remove_extended_entries(file)
char	*file;
{
register acl_entry_t	ent;
acl_entry_t	user_entry = (acl_entry_t)0;
acl_entry_t	other_entry = (acl_entry_t)0;
acl_entry_t	group_entry = (acl_entry_t)0;
acl_entry_t	mask_entry = (acl_entry_t)0;
register int	i;

	if (acl_wr->acl_num <= 3)
		return 0;

	/* Sort the entries into order.  */
	sort_acl (acl_wr);

	/*
	 * Increment through entries and
	 * re-store only the USER_OBJ, GROUP_OBJ,
	 * and OTHER_OBJ entries.
	 */

	ent = acl_wr->acl_first;

	for (i=0; (i < acl_wr->acl_num && !other_entry); i++) {

		switch (ent->acl_tag) {
		
			case USER_OBJ :
				user_entry = ent;
				break;

			case MASK_OBJ :
				mask_entry = ent;
				break;

			case GROUP_OBJ :
				/*
				 * If mask is present,
				 * intersect group mode with perms of
				 * mask entry.
				 */
				if (mask_entry)
					ent->acl_perm &= mask_entry->acl_perm;
					
				group_entry = ent;
				break;

			case OTHER_OBJ :
				other_entry = ent;
				break;
		}
		ent = (acl_entry_t) ent->acl_next;
	}

	/* Stuff the acl_wr acl buffer */
	ent = acl_wr->acl_first;
	copy_entries (user_entry, ent, 1);
	ent = ent->acl_next;
	copy_entries (group_entry, ent, 1);
	ent = ent->acl_next;
	copy_entries (other_entry, ent, 1);
	acl_wr->acl_num = 3;

	return 0;
}

/*
 * FUNCTION:
 *	get_command_line_entries ()
 *
 * ARGUMENTS:
 *	int	offset; - offset into the argv array.
 *	int	*size;  - how many entries.
 *
 * DESCRIPTION:
 *	The get_command_line_entries() function replaces
 *	all the ',' with '\n' and calls acl_unpack()
 *	to parse the entries.
 *
 * RETURNS:
 *	NOERROR - The entries.
 *	ERROR   - NULL.
 *		If the contents of command line are malformed.
 */

static acl_entry_t
get_command_line_entries (offset,size)
int	offset;
int	*size;
{
char		*ptr = args[offset];
int		numents = 1;

	/*
	 * user::rwx,group::rwx,mask::rwx
	 * Replace the ',' with '\n' and
	 * call the library routine.
	 */
	while (ptr && *ptr) {
		if (*ptr == ',') {
			*ptr = '\n';
			numents++;
		}
		ptr++;
	}

	if (!acl_tx) {
		if (acl_alloc (&acl_tx)) {
			fprintf (stderr, MSGSTR (MALLOCERR,
				"%s: Memory allocation error.\n"),args[0]);
			exit (1);
		}
	}

	/*
	 * Parse the external representation
	 * of the ACL into an internal representation.
	 */
	if (acl_unpack (args[offset], ACL_TEXT_PACKAGE, acl_tx) < 0) {
		fprintf (stderr, MSGSTR (UNUSENTRY,
			"%s: Entries are unusable.\n"),args[0]);
		return ((acl_entry_t)NULL);
	}

	*size = acl_tx->acl_num;

	if (acl_tx->acl_first == (acl_entry_t)NULL) 
		fprintf (stderr, "The impossible has happened, I grovel in awe of your superior intellect (part 2).\n");
	return acl_tx->acl_first;
}

/*
 * FUNCTION:
 *	get_file_entries ()
 *
 * ARGUMENTS:
 *	int	offset; - offset into the argv array.
 *	int	*size;  - how many entries.
 *
 * DESCRIPTION:
 *	The get_file_entries() function reads the file
 *	into a contiguous string and then calls 
 *	acl_unpack() to parse it into acl entries.
 *
 * RETURNS:
 *	NOERROR - The entries.
 *	ERROR   - NULL.
 *		If the contents of file are malformed.
 */

static acl_entry_t
get_file_entries (offset,size)
int	offset;
int	*size;
{
acl_entry_t	ent;
acl_data_t	dent;
char		*buf;

	/* read file into a contiguous string */
	if ((buf = (char *)file_to_buf (args[offset])) == (char *)NULL) {
		fprintf (stderr, MSGSTR (READERR, "%s: Cannot read \"%s\""),
			args[0],args[offset]);
		perror (" ");
		return ((acl_entry_t)NULL);
	}

	if (!acl_tx) {
		if (acl_alloc (&acl_tx)) {
			fprintf (stderr, MSGSTR (MALLOCERR,
				"%s: Memory allocation error.\n"),args[0]);
			exit (1);
		}
	}

	/*
	 * Parse the external representation
	 * of the ACL into working storage.
	 */
	if (acl_unpack (buf, ACL_TEXT_PACKAGE, acl_tx) < 0) {
		fprintf (stderr, MSGSTR (UNUSERR,
			"%s: Contents of file \"%s\" unusable.\n"),
			args[0],args[offset]);
		return ((acl_entry_t)NULL);
	}

	*size = acl_tx->acl_num;
	acl_tx->acl_num = 0;
	if (acl_tx->acl_first == (acl_entry_t)NULL) 
		fprintf (stderr, "The impossible has happened, I grovel in awe of your superior intellect.\n");
			
	return acl_tx->acl_first;
}

/*
 * FUNCTION:
 *	add_perms ()
 *
 * ARGUMENTS:
 *	acl_entr_t	ent;  - The entry containing the new permissions.
 *	acl_entr_t	nent; - The write buffer entry pointer.
 *
 * DESCRIPTION:
 *	The add_perms () function checks the permissions set by the
 *	subroutine acl_unpack() indicating the operation on
 *	the permission flags.
 *
 * RETURNS:
 *	none.
 */

static void
add_perms (ent,nent)
acl_entry_t	ent;
acl_entry_t	nent;
{
	/* add the permissions */
	if (ent->acl_perm & ACL_PADD)
		nent->acl_perm |= ent->acl_perm;

	/* remove the permissions */
	else if (ent->acl_perm & ACL_PREMOVE)
		nent->acl_perm &= ~ent->acl_perm;

	/* set permissions */
	else
		nent->acl_perm = ent->acl_perm;

	/* clear extended perms */
	ent->acl_perm &= ~(ACL_PADD | ACL_PREMOVE);
	nent->acl_perm &= ~(ACL_PADD | ACL_PREMOVE);

	return;
}

/*
 * FUNCTION:
 *	purge_perms ()
 *
 * ARGUMENTS:
 *	none.
 *
 * DESCRIPTION:
 *	The purge_perms() function purges the ineffective
 *	permissions from the file group class entries that
 *	were not specified with the -u or -f options by
 *	intersecting them with the old value of the mask.
 *
 *	The entries that were specified with the -u or -f
 *	flag have not yet been added.
 *
 * RETURNS:
 *	none.
 */

static void
purge_perms ()
{
register acl_entry_t	ent;
register int	i;
acl_permset_t	maskperms = 0, gotmask = 0;

	/* First find the mask permissions */
	ent = acl_rd->acl_first;

	for (i=0; i < acl_rd->acl_num; i++) {
		if (ent->acl_tag == MASK_OBJ) {
			maskperms = ent->acl_perm;
			gotmask++;
			break;
		}

		ent = ent->acl_next;
	}

	/* purge the permissions with the mask */
	if (gotmask) {
		ent = acl_rd->acl_first;
	
		for (i=0; i < acl_rd->acl_num; i++) {
	
			switch (ent->acl_tag) {
	
				case USER : case GROUP : case GROUP_OBJ :
					ent->acl_perm &= maskperms;
			}
	
			ent = ent->acl_next;
		}
	}

	/*
	 * If acl_wr is NULL allocate it here.
	 */
	if (acl_wr->acl_num == 0) {

		if (check_wk_space (acl_wr, acl_rd->acl_num)) {
			fprintf (stderr, MSGSTR (MALLOCERR,
				"%s: Memory allocation error.\n"),args[0]);
			exit (1);
		}

		copy_entries (acl_rd->acl_first, acl_wr->acl_first,
				acl_rd->acl_num);

		acl_wr->acl_num = acl_rd->acl_num;
	}

	return;
}

static int
alloc_entries (from, to, numents)
acl_entry_t	from;
acl_entry_t	*to;
int		numents;
{
register acl_entry_t	ent;
register int		i;

	ent = (acl_entry_t) calloc (numents, ACL_ENT_SIZE);
	if (!ent)
		return -1;

	*to = ent;

	for (i=0; i < numents; i++, ent++) {

		ent->acl_magic = ACL_MAGIC;
		ent->acl_tag   = from->acl_tag;
		ent->acl_perm  = from->acl_perm;
		ent->acl_id    = from->acl_id;

		from = from->acl_next;

		if ( i != numents - 1)
			ent->acl_next = (void *)(ent + 1);
	} 

	return 0;
}

/*
 * FUNCTION:
 *	copy_entries  ()
 *
 * ARGUMENTS:
 *	register acl_entry_t	from;    - the source entries.
 *	register acl_entry_t	to;	 - the destination.
 *	int			numents; - the number of entries to copy.
 *
 * DESCRIPTION:
 *	The copy_entries () function copies
 *	the acl entries from from to to.
 *
 * RETURNS:
 *	None.
 */

static void
copy_entries (from, to, numents)
register acl_entry_t	from;
register acl_entry_t	to;
int			numents;
{
acl_entry_t	prev_ent;
register int	i;

	for (i=0; i < numents; i++) {

		to->acl_tag  = from->acl_tag;
		to->acl_perm = from->acl_perm;
		to->acl_id   = from->acl_id;
		to->acl_magic = ACL_MAGIC;

		to = to->acl_next;
		from = from->acl_next;
	}

	return;
}

/*
 * FUNCTION:
 *	check_flags ()
 *
 * ARGUMENTS:
 *	none.
 *
 * DESCRIPTION:
 *	The check_flags() function checks the 
 *	command line flags for validity.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1).
 *		If flags are not consistent with function.
 */

static int
check_flags ()
{
	/* aflag has to be alone.  */
	if (aflag && (bflag || cflag || dflag || pflag ||
		fflag || rflag || Rflag || uflag || Uflag || xflag))
		return -1;

	/* xflag cannot have flags to do with mask calculation */
	if (xflag && (cflag || pflag))
		return -1;

	/* xflag must be accompanied by another flag */
	if (xflag && (!rflag && !Rflag && !fflag && !uflag && !Uflag))
		return -1;

	/* dflag must be accompanied by another flag */
	if (dflag && (!rflag && !Rflag && !fflag && !uflag && 
	    !Uflag && !pflag && !bflag && !cflag))
		return -1;

	/* cannot purge and replace */
	if (fflag && pflag)
		return -1;

	/* check for correct number of args */
	if (fflag && (lfile == file1))
		return -1;

	if ((rflag && (lfile == rentry)) || (Rflag && (lfile == file2)))
		return -1;

	if ((uflag && (lfile == uentry)) || (Uflag && (lfile == file3)))
		return -1;

	return 0;
}

/*
 * FUNCTION:
 *	usage ()
 *
 * ARGUMENTS:
 *
 * DESCRIPTION:
 *	Prints usage message.
 *
 * RETURNS:
 *	none.
 */

static void
usage ()
{
	(void) fprintf (stderr, MSGSTR(CHUSAGE,
		"Usage:  chacl [-a] directoryname(s)\n\tchacl [ -x | -c ] [-p] [-b] [-d] [-f file1] [-r entries]\n\t\t[-R file2] [-u entries] [-U file3] [-x] filename(s).\n"));
	exit (1);
}

static privvec_t save_privs;
/*
 * FUNCTION:
 *	raise_aclpriv()
 *
 * ARGUMENTS:
 *
 * DESCRIPTION:
 *	Raise access control override privileges for which
 *	the user is authorized.
 *
 * RETURNS:
 *	none.
 */

raise_aclpriv()
{
	enableprivs(privvec(SEC_OWNER, SEC_ALLOWDACACCESS,
#if SEC_MAC
                                SEC_ALLOWMACACCESS,
                                SEC_WRITEUPCLEARANCE,
                                SEC_WRITEUPSYSHI,
#endif
#if SEC_NCAV
                                SEC_ALLOWNCAVACCESS,
#endif
                                -1), save_privs);

}
/*
 * FUNCTION:
 *	lower_aclpriv()
 *
 * ARGUMENTS:
 *
 * DESCRIPTION:
 *	Lowers access control override privileges 
 *	
 *
 * RETURNS:
 *	none.
 */

lower_aclpriv()
{
	seteffprivs(save_privs, (priv_t *) 0);
}

#endif /* SEC_ACL_POSIX */
