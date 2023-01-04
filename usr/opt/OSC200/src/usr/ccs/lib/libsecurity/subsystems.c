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
static char *rcsid = "@(#)$RCSfile: subsystems.c,v $ $Revision: 4.2.10.2 $ (DEC) $Date: 1993/04/01 20:26:30 $";
#endif
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1988-1990 SecureWare, Inc.  All Rights Reserved.
 */


/*
 * Based on:

 */

#include <sys/secdefines.h>
#include "libsecurity.h"

/* #if SEC_BASE */ /*{*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <values.h> /* for BITS macro */

#include <sys/security.h>
#include <sys/sec_objects.h>
#include <sys/audit.h>
#include <prot.h>
#include <errno.h>

#define	AUTHORIZATION_DB	"/etc/auth/system/authorize"
#define	SUBSYSNAME_DB		"/etc/auth/system/subsystems"
#define	USER_DB			"/etc/auth/subsystems/users"

#define	BUFFER_SIZE		256
#define	ENTRIES			(BITS(priv_t))
#define	SUBSYS_INCR		20

static int	total_entries = 0;
static int	initialized = 0;
static int	widest = 0;
static int	file_size = 0;
static int	namepair_built = 0;
static int	subsysnames_built = 0;
static int	total_subsystems = 0;
static int	alloc_count = 0;

int		ST_MAX_CPRIV = 0;

/*
 * This array of cmdauth structures maps each authorization name to
 * a bit vector representing the set of authorizations that imply
 * that authorization name.
 */
struct cmdauth {
	char	*auth_name;
	priv_t	*auth_impl;
} *cmdauths = (struct cmdauth *) 0;

/*
 * The userauths bit vector represents the set of authorizations
 * granted to the calling user.
 */
priv_t	*userauths = (priv_t *) 0;

/*
 * The defauths bit vector represents the set of default authorizations.
 */
priv_t	*defauths = (priv_t *) 0;

/*
 * The subsysnames array maps each subsystem group name to its
 * corresponding descriptive name (primarily for audit reduction).
 */
static struct subsysname {
	char	*group_name;
	char	*description;
} *subsysnames = (struct subsysname *) 0;

static int	build_tables();
static int	build_subsysnames();
static char	*read_file();

extern char	*malloc(), *realloc();
extern char	*strdup();

/*
 *
 *	Read up the files and build up the internal data structures
 *
 */

static int
build_tables()

{
	char		*buffer_ptr, *end_ptr, *work_ptr;
	char		*nl_ptr, *comment_ptr;
	int		new_line, found, found_default, default_auths, luid;
	int		index, saved_index, priv_index, temp_index;
	priv_t		*new_ptr;
	char		*login_name;

	alloc_count = 1;

	buffer_ptr = read_file(AUTHORIZATION_DB);
	if (buffer_ptr == (char *) 0)
		return(1);

	end_ptr = &buffer_ptr[file_size];

	cmdauths = (struct cmdauth *) malloc(ENTRIES * sizeof(struct cmdauth));
	if (cmdauths == (struct cmdauth *) 0) {
		free(buffer_ptr);
		return(1);
	}

	total_entries = 0;
	*end_ptr = '\0';
	new_line = 1;

	work_ptr = buffer_ptr;
	while(work_ptr < end_ptr) {
		nl_ptr = strchr(work_ptr, '\n');
		if (nl_ptr == (char *) 0)
			break;
		*nl_ptr = '\0';

		comment_ptr = strchr(work_ptr, '#');
		if (comment_ptr != (char *) 0)
			*comment_ptr = '\0';

		work_ptr = strtok(work_ptr, ":, \t");
		while(work_ptr != (char *) 0) {
			found = 0;
			for (index = 0; index < total_entries; index++) {
				if (strcmp(work_ptr, cmdauths[index].auth_name) == 0) {
					found = 1;
					break;
				}
			}
			if (!found) {
				total_entries++;
				if ((total_entries % ENTRIES) == 0) {
					alloc_count++;
					cmdauths = (struct cmdauth *) realloc(cmdauths, alloc_count * ENTRIES * sizeof(struct cmdauth));
					if (cmdauths == (struct cmdauth *) 0) {
						free(buffer_ptr);
						total_entries = 0;
						return(1);
					}
					memset(&cmdauths[(alloc_count - 1) * ENTRIES], 0,
						ENTRIES * sizeof(struct cmdauth));

					for (temp_index = 0; temp_index < (total_entries - 1); temp_index++) {
						new_ptr = (priv_t *) calloc(alloc_count, sizeof(priv_t));
						if (new_ptr == (priv_t *) 0) {
							free(buffer_ptr);
							free(cmdauths);
							total_entries = 0;
							return(1);
						}
						for (priv_index = 0;
						     priv_index < (alloc_count - 1);
						     priv_index++)
							new_ptr[priv_index] = cmdauths[temp_index].auth_impl[priv_index];
						free(cmdauths[temp_index].auth_impl);
						cmdauths[temp_index].auth_impl = new_ptr;
					}
				}
				cmdauths[index].auth_name = work_ptr;
				cmdauths[index].auth_impl = (priv_t *) calloc(alloc_count, sizeof(priv_t));
				if (cmdauths[index].auth_impl == (priv_t *) 0) {
					free(buffer_ptr);
					free(cmdauths);
					total_entries = 0;
					return(1);
				}
				ADDBIT(cmdauths[index].auth_impl, index);
			}

			if (new_line) {
				/*
				 * This is the first authorization name on the line.
				 * Just remember its index for use in processing other
				 * authorization names on the same line.
				 */
				saved_index = index;
				new_line = 0;
			}
			else {
				/*
				 * This is not the first authorization name on the line,
				 * meaning that it is implied by another authorization
				 * (the one referenced by saved_index).  Thus, we must
				 * add all authorizations that imply the saved_index
				 * entry (represented by its auth_impl mask) to the
				 * auth_impl masks for the current entry and for all
				 * entries implied by the current entry.
				 */
				for (priv_index = 0; priv_index < total_entries; priv_index++) {
					if (ISBITSET(cmdauths[priv_index].auth_impl, index)) {
						for (temp_index = 0; temp_index < alloc_count; temp_index++)
							cmdauths[priv_index].auth_impl[temp_index] |=
								cmdauths[saved_index].auth_impl[temp_index];
					}
				}
			}
			work_ptr = strtok((char *) 0, ": ,\t");
		}

		work_ptr = ++nl_ptr;
		new_line = 1;
	}

	ST_MAX_CPRIV = total_entries;

/*
 *
 *	Now parse the users DB and build up userauths.
 *
 */

	buffer_ptr = read_file(USER_DB);
	if (buffer_ptr == (char *) 0)
		return(1);

	userauths = (priv_t *) calloc(alloc_count, sizeof(priv_t));
	if (userauths == (priv_t *) 0) {
		free(buffer_ptr);
		return(1);
	}

	defauths = (priv_t *) calloc(alloc_count, sizeof(priv_t));
	if (defauths == (priv_t *) 0) {
		free(buffer_ptr);
		return(1);
	}

	if ((luid = getluid()) == -1)
		login_name = (char *) 0;
	else
		login_name = pw_idtoname(luid);

	end_ptr = &buffer_ptr[file_size];
	*end_ptr = '\0';
	new_line = 1;
	found = 0;
	found_default = 0;

	work_ptr = buffer_ptr;
	while ((work_ptr < end_ptr) && (!found || !found_default)) {
		nl_ptr = strchr(work_ptr, '\n');
		if (nl_ptr == (char *) 0)
			break;
		*nl_ptr = '\0';

		comment_ptr = strchr(work_ptr, '#');
		if (comment_ptr != (char *) 0)
			*comment_ptr = '\0';

		while(*work_ptr == ' ' || *work_ptr == '\t')
			work_ptr++;
		if (*work_ptr == ':') {
			default_auths = 1;
			found_default = 1;
		} else
			default_auths = 0;

		for (work_ptr = strtok(work_ptr, ":, \t"); work_ptr != (char *) 0;
				work_ptr = strtok((char *) 0, ":, \t")) {
			/*
			 * Skip this line if it is neither the default entry
			 * nor the one we are looking for.
			 */
			if (!default_auths)
				if (new_line) {
					if (login_name &&
					    strcmp(work_ptr, login_name) == 0) {
						new_line = 0;
						found = 1;
						continue;
					}
					break;
				}

			for (index = 0; index < total_entries; index++) {
				if (strcmp(work_ptr, cmdauths[index].auth_name) == 0) {
					if (default_auths)
						ADDBIT(defauths, index);
					else
						ADDBIT(userauths, index);
					break;
				}
			}
		}
		work_ptr = ++nl_ptr;
		new_line = 1;
	}

	if (!found && found_default && login_name != (char *) 0)
		for (temp_index = 0; temp_index < alloc_count; temp_index++)
			userauths[temp_index] = defauths[temp_index];

	initialized = 1;
	return(0);

}

/*
 *	Build the subsystem name table.
 */

static int
build_subsysnames()
{
	FILE		*fp;
	register char	*cp;
	char		*group_name, *description;
	char		line[BUFFER_SIZE];
	int		allocd = 0;

	if (subsysnames_built)
		return 0;

	fp = fopen(SUBSYSNAME_DB, "r");
	if (fp == (FILE *) 0)
		return 1;
	
	while (fgets(line, sizeof line, fp)) {

		/* Strip comments and newline */
		cp = strpbrk(line, "#\n");
		if (cp)
			*cp = '\0';

		/* Skip leading whitespace */
		for (cp = line; *cp == ' ' || *cp == '\t'; ++cp)
			;
		group_name = cp;

		/* Look for separator */
		if (*cp == '\0' || (cp = strchr(cp, ':')) == NULL)
			continue;
		*cp++ = '\0';

		/* Skip leading whitespace before description */
		while (*cp == ' ' || *cp == '\t')
			++cp;
		description = cp;

		/* Trim trailing whitespace from description */
		cp += strlen(cp);
		while (--cp > description && (*cp == ' ' || *cp == '\t'))
			;
		cp[1] = '\0';

		/* Expand table if necessary */
		if (total_subsystems >= allocd) {
			char *new;

			allocd += SUBSYS_INCR;
			new = (subsysnames == NULL)
				? malloc(allocd * sizeof(struct subsysname))
				: realloc(subsysnames,
					  allocd * sizeof(struct subsysname));
			if (new == NULL) {
				fclose(fp);
				if (subsysnames)
					free(subsysnames);
				subsysnames = NULL;
				total_subsystems = 0;
				return 1;
			}
			subsysnames = (struct subsysname *) new;
		}

		/* Store group name and description in next table entry */
		subsysnames[total_subsystems].group_name = strdup(group_name);
		subsysnames[total_subsystems].description = strdup(description);
		++total_subsystems;
	}

	fclose(fp);

	subsysnames_built = 1;
	return 0;
}

/*
 *
 *	Internal function to read the contents of a file into a buffer.
 *	A pointer to the buffer is returned, along with the global (to this
 *	file only) variable file_size.  A NULL pointer is returned if
 *	we are unable to open or read the file.  The file is closed prior
 *	to returning to the caller.
 *
 */

static char *
read_file(file_name)

char	*file_name;

{
	FILE		*file_ptr;
	struct stat	stat_buffer;
	char		*buffer_ptr;

	file_ptr = fopen(file_name, "r");
	if (file_ptr == (FILE *) 0) {
		if (audgenl(AUTH_EVENT, T_CHARP, 
			 MSGSTR(SUBSYSTEM_1, "unable to open"),
			 T_CHARP, file_name, NULL) == -1)
			if ((errno != EPERM) && (errno != EACCES)) perror("audgenl");
		fprintf(stderr,MSGSTR(SUBSYSTEM_9,"Unable to open %s\n"),file_name);
		return((char *) 0);
	}

	fstat(fileno(file_ptr), &stat_buffer);

	buffer_ptr = (char *) malloc(stat_buffer.st_size + 1);
	if (buffer_ptr == (char *) 0) {
		fclose(file_ptr);
		return((char *) 0);
	}

	if (fread(buffer_ptr, stat_buffer.st_size, 1, file_ptr) != 1) {
		fclose(file_ptr);
		free(buffer_ptr);
		return((char *) 0);
	}

	fclose(file_ptr);

	file_size = stat_buffer.st_size;
	return(buffer_ptr);

}

/*
 *
 *	See if the current user holds the named authorization.
 *	Returns 1 if s/he does, 0 if not or can not build internal
 *	data structures.
 *
 */

int
authorized_user(auth_name)

char	*auth_name;

{
	register int	index, i, luid;
	char	*user;
	char	auditbuf[80];

	/* If security code is not being used then ignore the
	 * authorization checks
	 */
	if (!security_is_on())
		return 1;

	/*
	 * A primordial process (one whose luid has not been set)
	 * is implicitly authorized to do anything.
	 * So is the superuser on systems configured without SEC_PRIV.
	 */
#if SEC_PRIV
	if ((luid = getluid()) == -1)
#else
	if ((luid = getluid()) == -1 || starting_ruid() == 0)
#endif
		return 1;

	if (!initialized && build_tables())
		return(0);

	user = pw_idtoname(luid);
	if (user == (char *) 0) {
		sprintf(auditbuf, MSGSTR(SUBSYSTEM_2, "Check uid %d's authorization for %s"),
			luid, auth_name);
		if (audgenl(AUTH_EVENT, T_CHARP, MSGSTR(SUBSYSTEM_3, "Undefined user id"),
			    T_AUID, (char *)luid, NULL) == -1)
			if ((errno != EPERM) && (errno != EACCES)) perror("audgenl");
		return 0;
	}

	sprintf(auditbuf, MSGSTR(SUBSYSTEM_4, "Check %s's authorization for %s"), user, auth_name);

	for (index = 0; index < total_entries; index++) {
	    if (strcmp(auth_name, cmdauths[index].auth_name) == 0) {
		for (i = 0; i < alloc_count; ++i)
		    if (cmdauths[index].auth_impl[i] & userauths[i]) {
			if (audgenl(AUTH_EVENT, T_CHARP, MSGSTR(SUBSYSTEM_5, "Succeeded"), NULL) == -1)
				if ((errno != EPERM) && (errno != EACCES)) perror("audgenl");
			return(1);
		    }
		if (audgenl(AUTH_EVENT, T_CHARP, MSGSTR(SUBSYSTEM_6, "Failed"), NULL) == -1)
			if ((errno != EPERM) && (errno != EACCES)) perror("audgenl");
		return(0);
	    }
	}

	if (audgenl(AUTH_EVENT, T_CHARP, MSGSTR(SUBSYSTEM_7, "Undefined authorization"),
		    T_CHARP, auth_name, NULL) == -1)
		if ((errno != EPERM) && (errno != EACCES)) perror("audgenl");

	return(0);			/* not found */
}

/*
 *
 *	See if the passed authorization vector implies the named
 *	authorization.  Returns 1 if it does, 0 if not or can not
 *	build internal data structures.
 *
 */

int
hascmdauth(auth_name, auth_vec)

char	*auth_name;
priv_t	*auth_vec;

{
	register int	index, i;

	if (!initialized && build_tables())
		return(0);

	for (index = 0; index < total_entries; index++)
		if (strcmp(auth_name, cmdauths[index].auth_name) == 0) {
			for (i = 0; i < alloc_count; ++i)
				if (cmdauths[index].auth_impl[i] & auth_vec[i])
					return 1;
			return 0;	/* masks don't intersect */
		}

	return(0);			/* not found */
}

/*
 *
 *	Return the total number of authorizations that are configured.
 *	Returns -1 if the internal data structures can not be built.
 *
 */

int
total_auths()

{
	if (!initialized && build_tables())
		return(-1);
	return(total_entries);
}

/*
 *
 *	Returns the length of the longest authorization name configured.
 *	Returns -1 if the internal data structures can not be built.
 *	This is useful for the userif programs in order for them to
 *	properly layout the screens.
 *
 */

int
widest_auth()

{
	int	length, index;

	if (!initialized && build_tables())
		return(-1);

	if (widest == 0) {
		for (index = 0; index < total_entries; index++) {
			length = strlen(cmdauths[index].auth_name);
			if (length > widest)
				widest = length;
		}
	}
	return(widest);
	
}

/*
 *
 *	build the namepair table based on the DB files
 *
 */

int
build_cmd_priv()

{
	struct namepair	*namepair_ptr;
	int		index;

	if (!namepair_built) {
		if (!initialized && build_tables())
			return(-1);
		cmd_priv = (struct namepair *) calloc(total_entries + 1,
						      sizeof(struct namepair));
		if (cmd_priv == (struct namepair *) 0)
			return(-1);

		namepair_ptr = cmd_priv;
		for (index = 0; index < total_entries;
		     index++, namepair_ptr++) {
			namepair_ptr->name  = cmdauths[index].auth_name;
			namepair_ptr->value = index;
		}

		namepair_ptr->name  = (char *) 0;
		namepair_ptr->value = 0;

		namepair_built = 1;
	}
	return(0);
}

/*
 *
 *	Update the user DB (/etc/auth/subsystems/users) file.
 *
 *	If the first auth passed is "default", the named user is being
 *	assigned system default auths and we need to find his entry
 *	(if any) and just remove it.  If the first auth passed is not
 *	"default", the authorizations passed in is the user's new
 *	authorizations.  Either way, we need to delete the user that
 *	is passed in.  Afterwards, if the first auth passed in is not
 *	"default", we build up a line for the user with the new
 *	authorizations and output it to the file.
 *
 */

int
write_authorizations(user, auth_list, list_length)

char	*user;
char	**auth_list;
int	list_length;

{
	int	error = 0;
	int	index;
	char	*delimiter;
	char	*buffer_ptr, *work_ptr, *save_work_ptr;
	char	*temp_path_ptr, *old_path_ptr;
	FILE	*temp_file_ptr, *file_ptr;

	if (!make_transition_files(USER_DB, &temp_path_ptr, &old_path_ptr))
		return(-1);

	buffer_ptr = (char *) malloc(BUFFER_SIZE);
	if (buffer_ptr == (char *) 0) {
		free(temp_path_ptr);
		free(old_path_ptr);
		return(-1);
	}

	save_work_ptr = (char *) malloc(BUFFER_SIZE);
	if (save_work_ptr == (char *) 0) {
		free(buffer_ptr);
		free(temp_path_ptr);
		free(old_path_ptr);
		return(-1);
	}

	if (create_file_securely(temp_path_ptr, AUTH_VERBOSE,
				 MSGSTR(SUBSYSTEM_8, "Rewrite subsystem database")) !=
						CFS_GOOD_RETURN) {
		free(buffer_ptr);
		free(save_work_ptr);
		free(temp_path_ptr);
		free(old_path_ptr);
		return(-1);
	}

	temp_file_ptr = fopen(temp_path_ptr, "w");
	file_ptr      = fopen(USER_DB, "r");
	if ((temp_file_ptr == (FILE *) 0) ||
	    (file_ptr      == (FILE *) 0)) {
		unlink(temp_path_ptr);
		free(buffer_ptr);
		free(save_work_ptr);
		free(temp_path_ptr);
		free(old_path_ptr);
		return(-1);
	}

/*
 *
 *	Scan the file and remove the named user.
 *
 */

	while(!error && fgets(buffer_ptr, BUFFER_SIZE, file_ptr) != (char *) 0) {
		work_ptr = save_work_ptr;
		strcpy(work_ptr, buffer_ptr);

		while(*work_ptr == ' ' || *work_ptr == '\t')
			work_ptr++;

		if (*work_ptr != ':') {
			if (user != (char *) 0) {
				work_ptr = strtok(work_ptr, ": ,\t");
				if (work_ptr == (char *) 0)
					continue;	/* nothing here */
				if (strcmp(work_ptr, user) == 0)
					continue;	/* remove this user */
			}
		}
		else
			if (user == (char *) 0)
				continue;		/* remove default */

		error |= fputs(buffer_ptr, temp_file_ptr) == EOF;
	}

	fclose(file_ptr);

/*
 *
 *	See if we need to add this user in.
 *
 */

	if (!error && (user == (char *) 0)) {
		*buffer_ptr = '\0';
		delimiter = ":";
		if (list_length == 0)
			strcat(buffer_ptr, delimiter);
		for (index = 0; index < list_length; index++) {
			strcat(buffer_ptr, delimiter);
			delimiter = ", ";
			strcat(buffer_ptr, auth_list[index]);
		}
		strcat(buffer_ptr, "\n");
		error |= fputs(buffer_ptr, temp_file_ptr) == EOF;
	}
	else {

		/* include check for zero-length list.
		 * DO NOT UNDO THIS CHANGE WHEN/IF INTEGRATING
		 * OTHER CHANGES FROM SECUREWARE.
		 */
		if (!error && (list_length == 0 ||
			       strcmp(auth_list[0], "default") != 0)) {
			*buffer_ptr = '\0';
			strcat(buffer_ptr, user);
			delimiter = ":\t";
			if (list_length == 0)
				strcat(buffer_ptr, delimiter);
			for (index = 0; index < list_length; index++) {
				strcat(buffer_ptr, delimiter);
				delimiter = ", ";
				strcat(buffer_ptr, auth_list[index]);
			}
			strcat(buffer_ptr, "\n");
			error |= fputs(buffer_ptr, temp_file_ptr) == EOF;
		}
	}

	if (!error) {
		fclose(temp_file_ptr);
		replace_file(temp_path_ptr, USER_DB, old_path_ptr);
	}
	else {
		unlink(temp_path_ptr);
		free(temp_path_ptr);
		free(old_path_ptr);
	}

	free(save_work_ptr);
	free(buffer_ptr);
	return(error);
}

/* return the authorization number of the passed-in name
 * return -1 if unsuccessful db read or invalid name.
 * This function is for backward compatibility.
 */

int
primary_auth (name)
char *name;
{
	register int i;

	if (!initialized && build_tables())
		return -1;

	for (i = 0; i < total_entries; ++i)
		if (strcmp(name, cmdauths[i].auth_name) == 0)
			return i;

	return -1;
}

/* return the authorization number of the passed-in name
 * return -1 if unsuccessful db read or invalid name.
 * This function is for backward compatibility.
 */

int
secondary_auth (name)
char *name;
{
	register int i;

	if (!initialized && build_tables())
		return -1;

	for (i = 0; i < total_entries; ++i)
		if (strcmp(name, cmdauths[i].auth_name) == 0)
			return i;

	return -1;
}

/*
 *
 *	subsys_real_name(name)
 *
 *	Obtain the descriptive name of the subsystem.
 *
 *	The caller passes a string containing the name of the subsystem group.
 *	Returns a static pointer to the character string that describes
 *	the subsystem.  If the subsystems file can't be read or does not
 *	contain the named subsystem, returns the passed argument.
 *
 */

char *
subsys_real_name(subsys)

char	*subsys;

{
	int	index;

	if (!subsysnames_built && build_subsysnames())
		return subsys;

	for (index = 0; index < total_subsystems; index++) 
		if (strcmp(subsysnames[index].group_name, subsys) == 0)
			return subsysnames[index].description;

	return subsys;
}
/* #endif */ /*} SEC_BASE */
