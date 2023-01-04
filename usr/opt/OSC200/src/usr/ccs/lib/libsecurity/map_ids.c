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
static char	*sccsid = "@(#)$RCSfile: map_ids.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/04/01 20:24:07 $";
#endif 
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	map_ids.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.8.2.2  1992/06/11  14:28:54  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  14:26:34  hosking]
 *
 * Revision 1.8  1991/03/23  17:57:34  devrcs
 * 	<<<replace with log message for ./usr/ccs/lib/libsecurity/map_ids.c>>>
 * 	[91/03/12  11:30:48  devrcs]
 * 
 * 	Merge fixes up from 1.0.1
 * 	[91/03/11  15:25:48  seiden]
 * 
 * 	In make_file(), if the line can't be parsed ignore it rather than failing.
 * 	[91/03/10  09:38:09  seiden]
 * 
 * Revision 1.5  90/10/07  20:07:52  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:15:55  gm]
 * 
 * Revision 1.4  90/08/24  13:48:28  devrcs
 * 	Changes for type-widening.
 * 	[90/08/20  14:17:35  seiden]
 * 
 * Revision 1.3  90/07/17  12:20:32  devrcs
 * 	osc.14 SecureWare merge
 * 	[90/07/06  10:01:45  staffan]
 * 
 * 	Internationalized
 * 	[90/07/05  07:27:34  staffan]
 * 
 * Revision 1.2  90/06/22  21:47:30  devrcs
 * 	Initial version from SecureWare
 * 	[90/05/31  11:31:27  staffan]
 * 
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1988-90, SecureWare, Inc.
 *   All rights reserved.
 *
 * File containing routines that map between user and group names and IDs.
 */

/* #ident "@(#)map_ids.c	6.3 06:18:53 2/19/91 SecureWare" */
/*
 * Based on:
 *   "@(#)map_ids.c	2.4.1.1 11:44:49 2/3/90 SecureWare"
 */

#include <sys/secdefines.h>

/* #if SEC_BASE */ /*{*/

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <values.h>
#include <sys/security.h>
#include <siad.h>

/*
 * The data types are a table of pointers to an entry-mapping structure.
 * The pointers in memory are real pointers but in the mapping file are
 * file offsets.  The file structure is as follows:
 *	int #entries in the name-to-id map
 *	int #entries in the id-to-name map
 *	int number of characters before the tables
 *	for each mapping, an integer ID and a null-terminated char string.
 *	A sorted list of offsets of entries by name
 *	A sorted list of offsets of entries by ID
 */

typedef union {
	uid_t	uid;
	gid_t	gid;
}	ID;

struct pw_entry {
	ID	id;
	char	name[1];
};

union pw_map {
	long	file_offset;
	struct pw_entry *pw;
};

/*
 * The following global variables hold the state of the internal database.
 */

struct pw_info {
	char *base_file;
	char *map_file;
	int (*name_comp)();
	int (*id_comp)();
	void (*id_parse)();
	union pw_map *map_by_name;
	union pw_map *map_by_id;
	char *map_buf;
	int map_buf_size;
	int nentries;
	int nidentries;
	time_t last_check;
};

/* internal routine declarations for this module */
static int uid_comp();
static int gid_comp();
static void uid_parse();
static void gid_parse();
static int name_comp();
static int make_file();
static int read_file();
static int check_file();
static int parse_line();
static int lock_file();
static int unlock_file();

static struct pw_info passwd_info = {
	"/etc/passwd", "/etc/auth/system/pw_id_map",
	name_comp, uid_comp, uid_parse,
	(union pw_map *) 0, (union pw_map *) 0, (char *) 0, 0, 0, 0, (time_t) 0
};
static struct pw_info group_info = {
	"/etc/group", "/etc/auth/system/gr_id_map",
	name_comp, gid_comp, gid_parse,
	(union pw_map *) 0, (union pw_map *) 0, (char *) 0, 0, 0, 0, (time_t) 0
};
/* Line buffer */

#define LINE_BUFSIZ	1024	/* SPR 101 MR 589 (S001) */
static char line_buf[LINE_BUFSIZ];

/* Alignment fwrite buffer */

char nulls[sizeof(ID)] = { 0 };

#ifdef SUCCESS
#undef SUCCESS
#endif
#ifdef FAIL
#undef FAIL
#endif
#define SUCCESS 0
#define FAIL -1
#define ID_MAX_SIZE 8
#define MAP_FILE_OWNER "auth"
#define MAP_FILE_GROUP "auth"
#define MAP_FILE_MODE 0664

/* The externally visible routines are the ones which do the mapping.
 * They all need a base structure with which to do a binary search.
 */

static struct pw_entry *pw = (struct pw_entry *) 0;

static int
check_pw()
{
	if (pw == (struct pw_entry *) 0) {
		pw = (struct pw_entry *) malloc(sizeof *pw + ID_MAX_SIZE);
		if (pw == (struct pw_entry *) 0)
			return (FAIL);
	}
	return (SUCCESS);
}

/* map a user name to a user ID.
 * returns a valid user ID or -1.
 */

uid_t
pw_nametoid (name)
char *name;
{
	union pw_map u, *up;

	if (check_file (&passwd_info) == FAIL)
		return (uid_t) -1;
	if (check_pw() == FAIL)
		return (uid_t) -1;
	u.pw = pw;
	strcpy (pw->name, name);
	up = (union pw_map *) bsearch ((char *) &u,
				       (char *) passwd_info.map_by_name,
				       passwd_info.nentries, sizeof (u),
				       name_comp);
	if (! up) {
		struct passwd pwp;
		char pwbuf[SIABUFSIZ+1];
		union sia_get_params uspu;
#define usp uspu.passwd
		extern int sia_getpasswd();

		bzero(pwbuf, sizeof pwbuf);
		bzero((caddr_t)&usp, sizeof usp);
		usp.buffer = pwbuf;
		usp.len = sizeof pwbuf;
		usp.name = name;
		usp.result = &pwp;
		if (sia_getpasswd(P_NAM, REENTRANT, &uspu)&1) {
			return pwp.pw_uid;
		}
	}
	return up ? up->pw->id.uid : (uid_t) -1;
}

/* map a user ID to a user name.
 * returns a valid user name or NULL.
 */

char *
pw_idtoname (id)
uid_t id;
{
	union pw_map u, *up;

	if (check_file (&passwd_info) == FAIL)
		return (NULL);
	if (check_pw() == FAIL)
		return (NULL);
	u.pw = pw;
	pw->id.uid = id;
	up = (union pw_map *) bsearch ((char *) &u,
				       (char *) passwd_info.map_by_id,
				       passwd_info.nidentries, sizeof (u),
				       uid_comp);
	if (! up) {
		struct passwd pwp;
		char pwbuf[SIABUFSIZ+1];
		union sia_get_params uspu;
#undef usp
#define usp uspu.passwd
		extern int sia_getpasswd();
		static char *last;

		bzero(pwbuf, sizeof pwbuf);
		bzero((caddr_t)&usp, sizeof usp);
		usp.buffer = pwbuf;
		usp.len = sizeof pwbuf;
		usp.uid = id;
		usp.result = &pwp;
		if (sia_getpasswd(P_UID, REENTRANT, &uspu)&1) {
			if (last) free(last);
			last = strdup(pwp.pw_name);
			if (last) return last;
		}
	}
	return up ? up->pw->name : NULL;
}

/* map a group name to a group ID.
 * returns a valid group ID or -1.
 */

gid_t
gr_nametoid (name)
char *name;
{
	union pw_map u, *up;

	if (check_file (&group_info) == FAIL)
		return (-1);
	if (check_pw() == FAIL)
		return (-1);
	u.pw = pw;
	strcpy (pw->name, name);
	up = (union pw_map *) bsearch ((char *) &u,
				       (char *) group_info.map_by_name,
				       group_info.nentries, sizeof (u),
				       name_comp);
	if (! up) {
		struct group pwp;
		char pwbuf[SIABUFSIZ+1];
		union sia_get_params uspu;
#undef usp
#define usp uspu.group
		extern int sia_getgroup();

		bzero(pwbuf, sizeof pwbuf);
		bzero((caddr_t)&usp, sizeof usp);
		usp.buffer = pwbuf;
		usp.len = sizeof pwbuf;
		usp.name = name;
		usp.result = &pwp;
		if (sia_getgroup(G_NAM, REENTRANT, &uspu)&1) {
			return pwp.gr_gid;
		}
	}
	return up ? up->pw->id.gid : (gid_t) -1;
}

/* map a group ID to a group name.
 * returns a valid group name or NULL.
 */

char *
gr_idtoname (id)
gid_t id;
{
	union pw_map u, *up;

	if (check_file (&group_info) == FAIL)
		return (NULL);
	if (check_pw() == FAIL)
		return (NULL);
	u.pw = pw;
	pw->id.gid = id;
	up = (union pw_map *) bsearch ((char *) &u,
				       (char *) group_info.map_by_id,
				       group_info.nidentries, sizeof (u),
				       gid_comp);
	if (! up) {
		struct group pwp;
		char pwbuf[SIABUFSIZ+1];
		union sia_get_params uspu;
#undef usp
#define usp uspu.group
		extern int sia_getgroup();
		static char *last;

		bzero(pwbuf, sizeof pwbuf);
		bzero((caddr_t)&usp, sizeof usp);
		usp.buffer = pwbuf;
		usp.len = sizeof pwbuf;
		usp.gid = id;
		usp.result = &pwp;
		if (sia_getgroup(G_GID, REENTRANT, &uspu)&1) {
			if (last) free(last);
			last = strdup(pwp.gr_name);
			if (last) return last;
		}
	}
	return up ? up->pw->name : NULL;
}

/* Following are the routines which are private to this module: */

/* check if the mapping file must be rebuilt, whether the internal version
 * of the mapping file must be read in, or both.
 * returns SUCCESS if it's OK to continue, FAIL on error.
 * The algorithm is to read in the mapping file if it exists and it's newer
 * than the base file.  If the base file is newer or the mapping file
 * doesn't exist and the program has permission to rewrite the mapping file,
 * then create a new one. To avoid constant stats, check every INTERVAL
 * seconds.
 */
#define INTERVAL	30
static int
check_file (pwi)
register struct pw_info *pwi;
{
	time_t now = time ((time_t *) 0);
	struct stat base_sb, map_sb;
	time_t last_check = pwi->last_check;
	FILE *fp;
	int status;

	/* if have checked, then only check conditions if INTERVAL has passed */
	if (last_check != (time_t) 0 && now - last_check < INTERVAL)
		return (SUCCESS);

	/* if can't get at base file, there's real trouble */
	if (stat (pwi->base_file, &base_sb) < 0)
		return (FAIL);
	
	/* if base file hasn't changed since we last checked, return */
	if (last_check > base_sb.st_mtime) {
		last_check = now;
		return (SUCCESS);
	}

	/* The base file has changed since we last checked.
	 * if there's no map file, read in directly from the database.
	 */
	if (stat (pwi->map_file, &map_sb) < 0)
		return (make_file (pwi));

	/* There is a map file.
	 * Check if we have to rewrite it (base file is newer).
	 */
	if (map_sb.st_mtime < base_sb.st_mtime)
		return (make_file(pwi));
	
	/* if haven't checked and there is a map file, read it in */
	if (last_check == (time_t) 0 || map_sb.st_mtime > last_check) {
		fp = fopen (pwi->map_file, "r");
		if (fp == (FILE *) 0 || lock_file (fp, F_RDLCK) == FAIL) {
			fclose (fp);
			return (make_file(pwi));
		}
		status = read_file (pwi, fp);
		unlock_file (fp);
		fclose (fp);
		/* if format was bad, try to re-create the file */
		if (status == FAIL)
			return (make_file (pwi));
		return (status);
	} else { /* hasn't changed and have checked previously */
		pwi->last_check = now;
		return (SUCCESS);
	}
}

/* make the mapping file, if possible.  If not possible, read in the
 * database directly from the base file.
 */

static int
make_file (pwi)
register struct pw_info *pwi;
{
	FILE *map_fp, *base_fp;
	union pw_map u;
	char *name;
	ID id;
	int count, nentries, nidentries;
	char *map_buf;
	int map_buf_size;
	register char *cp;
	register struct pw_entry *pw;
	register union pw_map *pmp, *map_by_name, *map_by_id;
	struct stat sb;
	register int i, j;
	int fd;
	int round = 0;
	char nowrite = 0;

	/* initialize buffer pointer */
	map_buf = (char *)0;	/* SPR 101 MR 589 (S001) */

	/* if file doesn't exist, try to create it, o.w., open for update */
	if (stat (pwi->map_file, &sb) < 0) {
		int oumask;

		oumask = umask (0);
		fd = open (pwi->map_file, O_RDWR|O_CREAT, MAP_FILE_MODE);
		umask (oumask);
		if (fd >= 0) {
			chown(pwi->map_file, 0, 0);
			map_fp = fdopen (fd, "r+");
		} else
			map_fp = NULL;
	}
	else	map_fp = fopen (pwi->map_file, "r+");

	if (map_fp != (FILE *) 0  && lock_file (map_fp, F_WRLCK) == FAIL) {
		fclose (map_fp);
		unlink(pwi->map_file);
		map_fp = (FILE *) 0;
	}

	if (map_fp == (FILE *) 0)
		nowrite = 1;

	/*
	 * If base file cannot be opened, remove any existing map file
	 */

	base_fp = fopen (pwi->base_file, "r");
	if (base_fp == (FILE *) 0) {
		if (map_fp != (FILE *) 0)
			fclose (map_fp);
		unlink(pwi->map_file);
		return (FAIL);
	}

	/* count up the total space for IDs and names and count entries
	 * If saving to map file, write the IDs and names to the file.
	 */
	count = 0;
	nentries = 0;
	if (!nowrite) {
		if (fwrite((char *)&nentries, sizeof(nentries), 1, map_fp) != 1)
			nowrite = 1;
		else if (fwrite((char *)&nentries, sizeof(nentries), 1, map_fp) != 1)
			nowrite = 1;
		else if (fwrite((char *)&count, sizeof(count), 1, map_fp) != 1)
			nowrite = 1;
	}
	while (fgets (line_buf, sizeof(line_buf), base_fp) != NULL) {
	/*
 	 * Deal with over-long lines
	 */
		if ((i = strlen(line_buf)) > 0 && line_buf[i-1] != '\n') {
			while ((i = getc(base_fp)) != EOF && i != '\n')
				continue;
		}

		/* If the line can't be parsed, ignore it */

		if (parse_line (line_buf, &name, &id, pwi->id_parse) == FAIL)
			continue;

		/* save entries to map file if we are building it */
		if (!nowrite) {
			if (fwrite((char *) &id, sizeof (id), 1, map_fp) != 1)
				nowrite = 1;
			else if (fputs (name, map_fp) == EOF)
				nowrite = 1;
			else if (putc ((int) '\0', map_fp) == EOF)
				nowrite = 1;
		}
		count += sizeof (id) + strlen (name) + 1;
		if (round = (count % sizeof(ID))) {
			round = sizeof(ID) - round;
			count += round;
			if (!nowrite &&
			    fwrite(nulls, 1, round, map_fp) != round)
				nowrite = 1;
		}
		nentries++;
	}

	/*
	 * Round up to an appropriate boundary for the map entries
	 */
	if (round = (count % sizeof(u))) {
		round = sizeof(u) - round;
		count += round;
		if (!nowrite && fwrite(nulls, 1, round, map_fp) != round)
			nowrite = 1;
	}

	if (!nowrite)
		fflush(map_fp);

	/* allocate internal space for the database.  Use the old space if
	 * it's the same size.
	 */
	map_buf_size = count + (2 * nentries * sizeof(u));
	if (pwi->map_buf == (char *) 0)
		map_buf = malloc (map_buf_size);
	else if (pwi->map_buf_size != map_buf_size)
		map_buf = realloc (pwi->map_buf, map_buf_size);
	else
		map_buf = pwi->map_buf;
	if (map_buf == (char *) 0)
		goto fail;
	
	memset((char *) map_buf, 0, map_buf_size);
	/* scan the base file again for the mappings (map file is locked) */
	cp = map_buf;
	fseek (base_fp, 0L, 0);
	while (fgets (line_buf, sizeof (line_buf), base_fp) != NULL) {
		if (parse_line (line_buf, &name, &id, pwi->id_parse) == FAIL)
			continue;
		pw = (struct pw_entry *) cp;
		pw->id = id;
		strcpy (pw->name, name);
		cp += sizeof (pw->id) + strlen (name) + 1;
		if(round = ((int) cp % sizeof(ID))) {
			round = sizeof(ID) - round;
			cp += round;
		}
	}
	fclose (base_fp);
	base_fp = (FILE *) 0;

	/* build the pointer tables and sort them */
	map_by_name = (union pw_map *) (map_buf + count);
	map_by_id = map_by_name + nentries;
	cp = map_buf;
	pmp = map_by_name;
	for (i = 0; i < nentries; i++, pmp++) {
		/*
		 * Store the pw_entry pointer in both maps.
		 * pmp points into the name map while
		 * &pmp[nentries] points into the id map.
		 */
		pmp[nentries].pw = pmp->pw = (struct pw_entry *) cp;
		cp += sizeof (pmp->pw->id) + strlen (pmp->pw->name) + 1;
		if(round = ((int) cp % sizeof(ID))) {
			round = sizeof(ID) - round;
			cp += round;
		}
	}
	qsort ((char *) map_by_name, nentries, sizeof(*pmp), pwi->name_comp);
	qsort ((char *) map_by_id,   nentries, sizeof(*pmp), pwi->id_comp);

	/* Collapse duplicates in the id map */
	for (j = 0, i = 1; i < nentries; ++i)
		/*
		 * If there are duplicate IDs, keep the one that
		 * appeared first in the base file.
		 */
		if ((*pwi->id_comp)(&map_by_id[j], &map_by_id[i]) == 0) {
			if (map_by_id[i].pw < map_by_id[j].pw)
				map_by_id[j].pw = map_by_id[i].pw;
		} else if (i != ++j)
			map_by_id[j].pw = map_by_id[i].pw;
	nidentries = j + 1;
			

 	/* save the tables to the map file */
	if (!nowrite) {
		pmp = map_by_name;
		for (i = 0; i < nentries && !nowrite; i++, pmp++) {
			u.file_offset = ((char *) pmp->pw) - map_buf;
			if (fwrite ((char *)&u, sizeof(u), 1, map_fp) != 1)
				nowrite = 1;
		}
		pmp = map_by_id;
		for (i = 0; i < nidentries && !nowrite; i++, pmp++) {
			u.file_offset = ((char *) pmp->pw) - map_buf;
			if (fwrite ((char *)&u, sizeof(u), 1, map_fp) != 1)
				nowrite = 1;
		}
		/* add the counters after seeking to beginning of file */
		if (!nowrite) {
			fflush(map_fp);
			fseek (map_fp, 0L, 0);
			if (fwrite((char *)&nentries, sizeof nentries,
			      1, map_fp) != 1)
				nowrite = 1;
			else if (fwrite((char *)&nidentries, sizeof nidentries,
			      1, map_fp) != 1)
				nowrite = 1;
			else if (fwrite((char *)&count, sizeof count,
			      1, map_fp) != 1)
				nowrite = 1;
			unlock_file (map_fp);
			fclose (map_fp);
		}
	}
	pwi->last_check = time ((time_t *) 0);
	pwi->map_by_name = map_by_name;
	pwi->map_by_id = map_by_id;
	pwi->map_buf = map_buf;
	pwi->map_buf_size = map_buf_size;
	pwi->nentries = nentries;
	pwi->nidentries = nidentries;

	/*
	 * If nowrite is set and we fell to here, the internal tables
	 * were built but the output file could not be written.
	 * Unlock and remove the output file before returning SUCCESS.
	 */

	if (nowrite) {
		if (map_fp != (FILE *) 0) {
			unlock_file (map_fp);
			fclose (map_fp);
		}
		unlink (pwi->map_file);
	}
	return (SUCCESS);

	/*
	 * The fail label is reached if the underlying file was corrupted.
	 * All memory must be reclaimed.
	 */
fail:
	if (base_fp != (FILE *) 0)
		fclose (base_fp);
	if (map_fp != (FILE *) 0) {
		unlock_file (map_fp);
		fclose (map_fp);
	}
	unlink (pwi->map_file);
	if (map_buf != (char *) 0)
		free (map_buf);
	if (pwi->map_buf != (char *) 0 && pwi->map_buf != map_buf)
		free (pwi->map_buf);
	pwi->last_check = 0;
	pwi->map_by_name = (union pw_map *) 0;
	pwi->map_by_id = (union pw_map *) 0;
	pwi->map_buf = (char *) 0;
	pwi->map_buf_size = 0;
	pwi->nentries = 0;
	pwi->nidentries = 0;
	return (FAIL);
}

/* Comparison routines for qsort and bsearch */

static int
uid_comp (pmp1, pmp2)
register union pw_map *pmp1, *pmp2;
{
	/*
	 * Make explicit comparisons rather than returning id1 - id2
	 * to avoid reliance on 2's complement arithmetic and the
	 * order of implicit type conversions.
	 */
	if (pmp1->pw->id.uid == pmp2->pw->id.uid)
		return 0;
	if (pmp1->pw->id.uid < pmp2->pw->id.uid)
		return -1;
	return 1;
}

static int
gid_comp (pmp1, pmp2)
register union pw_map *pmp1, *pmp2;
{
	/*
	 * Make explicit comparisons rather than returning id1 - id2
	 * to avoid reliance on 2's complement arithmetic and the
	 * order of implicit type conversions.
	 */
	if (pmp1->pw->id.gid == pmp2->pw->id.gid)
		return 0;
	if (pmp1->pw->id.gid < pmp2->pw->id.gid)
		return -1;
	return 1;
}

static int
name_comp (pmp1, pmp2)
register union pw_map *pmp1, *pmp2;
{
	return (strcmp (pmp1->pw->name, pmp2->pw->name));
}

/* read in the mapping file.
 * returns SUCCESS or FAIL.
 */

static int
read_file (pwi, fp)
struct pw_info *pwi;
FILE *fp;
{
	int count;
	int nentries;
	int nidentries;
	int i;
	char *map_buf = (char *) 0;
	int map_buf_size;
	union pw_map *idp, *namep;
	struct stat stb;

	if (fread ((char *) &nentries, sizeof (nentries), 1, fp) != 1)
		return (FAIL);
	if (fread ((char *) &nidentries, sizeof (nidentries), 1, fp) != 1)
		return (FAIL);
	if (fread ((char *) &count, sizeof (count), 1, fp) != 1)
		return (FAIL);
	map_buf_size = count + ((nentries+nidentries) * sizeof (union pw_map));

	/*
	 * Verify that the file is exactly as large as the header fields
	 * indicate it sould be.
	 */
	if (fstat(fileno(fp), &stb) < 0)
		return (FAIL);
	stb.st_size -= sizeof nentries + sizeof nidentries + sizeof count;
	if (stb.st_size != map_buf_size)
		return (FAIL);

	map_buf = malloc (map_buf_size);
	if (map_buf == (char *) 0)
		return (FAIL);
	if (fread (map_buf, map_buf_size, 1, fp) != 1) {
		free (map_buf);
		return (FAIL);
	}
	pwi->map_by_name = (union pw_map *) (map_buf + count);
	pwi->map_by_id = pwi->map_by_name + nentries;
	idp = pwi->map_by_id;
	namep = pwi->map_by_name;
	for (i = 0; i < nentries; i++, idp++, namep++) {
		/*
		 * Verify that the map pointers are properly aligned and point
		 * into the section of the file that contins the pw_entry
		 * structures.
		 */
		if ((u_long) namep->file_offset % sizeof(ID) ||
			(u_long) namep->file_offset >= (u_long) map_buf_size ||
		    i < nidentries && ((u_long) idp->file_offset % sizeof(ID) ||
			(u_long) idp->file_offset >= (u_long) map_buf_size))
		{
			free (map_buf);
			return (FAIL);
		}
		namep->pw = (struct pw_entry *) (map_buf + namep->file_offset);
		if (i < nidentries)
		    idp->pw = (struct pw_entry *) (map_buf + idp->file_offset);
	}
	pwi->map_buf = map_buf;
	pwi->map_buf_size = map_buf_size;
	pwi->last_check = time ((time_t *) 0);
	pwi->nentries = nentries;
	pwi->nidentries = nidentries;
	return (SUCCESS);
}

/* parse a line of the base file.  We take advantage of the fact that
 * /etc/passwd and /etc/group have same format wrt names and IDs.
 * returns SUCCESS or failure.
 */

static int
parse_line (line, namep, idp, id_parse)
char *line;
char **namep;
ID *idp;
void (*id_parse)();
{
	register char *cp;

	*namep = line;
	cp = strchr (line, ':');
	if (cp == (char *) 0)
		return (FAIL);
	*cp++ = '\0';
	cp = strchr (cp, ':');
	if (cp == (char *) 0)
		return (FAIL);
	cp++;
	if (*cp == '\0' || !isdigit (*cp) && (*cp != '-' || !isdigit(cp[1])))
		return (FAIL);
	(*id_parse)(idp, cp);
	return (SUCCESS);
}

static void
uid_parse(idp, cp)
	ID	*idp;
	char	*cp;
{
	idp->uid = atoi(cp);
}

static void
gid_parse(idp, cp)
	ID	*idp;
	char	*cp;
{
	idp->gid = atoi(cp);
}

/* file locking and unlocking routines */

#define TRIES 3
#define WAIT 1

static int
lock_file (fp, type)
FILE *fp;
int type;
{
	struct flock f;
	int i;

	f.l_type = type;
	f.l_whence = 0;
	f.l_start = 0;
	f.l_len = 0;
	for (i = 0; i < TRIES; i++)
		if (fcntl (fileno (fp), F_SETLK, &f) == 0)
			return (SUCCESS);
		else
			sleep (WAIT);
	return (FAIL);
}

static int
unlock_file (fp)
FILE *fp;
{
	struct flock f;

	f.l_type = F_UNLCK;
	f.l_whence = 0;
	f.l_start = 0;
	f.l_len = 0;
	fcntl (fileno (fp), F_SETLK, &f);
	return (SUCCESS);
}
/* #endif */ /*} SEC_BASE */
