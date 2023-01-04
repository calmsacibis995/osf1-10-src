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
static char	*sccsid = "@(#)$RCSfile: acllib.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:15:10 $";
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
/*  Copyright (c) 1988-90, SecureWare, Inc.
 *    All Rights Reserved
 *
 *  Discretionary access control subroutine library.
 *
 *  This library provides support routines for the creation, deletion,
 *  and modification of Access Control Lists on objects as well as
 *  other functions such as policy configuration parameters.
 *  
 */


/*
 * Based on:

 */

#include <sys/secdefines.h>

#include "libsecurity.h"

#if SEC_ACL_SWARE

/* Discretionary access control library routines:
 *	acl_init()
 *	acl_daemon_init()
 *	acl_er_to_ir()
 *	acl_ir_to_er()
 *	acl_1ir_to_er()
 *	acl_ir_to_tag()
 *	acl_lookup_syn()
 *	acl_alloc_syn()
 *	acl_free_syn()
 *	acl_delete_syn()
 *	acl_insert_syn()
 *	acl_load_syns()
 *	acl_read_syn_db()
 *	acl_store_syns()
 *	acl_write_syn_db()
 */

#include <sys/types.h>
#include <ctype.h>
#include <memory.h>
#ifndef hpux
#include <malloc.h>
#endif
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <search.h>
#include <string.h>
#include <acl.h>
#include <sys/stat.h>
#include <sys/security.h>
#include <sys/secpolicy.h>
#include <sys/secioctl.h>

#define	DAEMON		0
#define	CLIENT		1

/* Miscellaneous definitions */

#define	INIT_ACLSIZE	16	/* MUST be a power of 2 */
#define	INIT_BUFSIZE	128

#ifndef FALSE
#define	FALSE		0
#endif
#ifndef TRUE
#define	TRUE		1
#endif

/* Declarations of global data */

struct acl_config	acl_config;	/* policy configuration parameters */
aclsyn_t		acl_syns;	/* table of ACL synonyms */
char			*acl_dbfile;	/* name of synonym file */

/* Declarations of private data */

static int	aclinit = 0;		/* configuration done flag */
static char	*acl_er = NULL;		/* working ACL er buffer */
static int	acl_ersz = 0;		/* number of bytes in it */
static acle_t	*acl_ir = NULL;		/* working ACL ir buffer */
static int	acl_irsz = 0;		/* number of entries in it */
static char	*acl_synbuf = NULL;	/* space allocated to read database */
static int	acl_synspace = 0;	/* size of acl_synbuf */
static char	*perms[] =		/* permission strings */
			{ "-", "x", "w", "wx", "r", "rx", "rw", "rwx" };
static struct sp_init sp_init;		/* initialization structure */

#define	STREQ(s1,s2)	(strcmp(s1, s2) == 0)
#define	STRGE(s1,s2)	(strcmp(s1,s2)>=0)

/* Stuff for the token scanner */

#define	NUMBER	256
#define	STRING	257

struct lexstate {
	char		*string;	/* string being parsed */
	char		*pos;		/* current position in string */
	unsigned	numval;		/* value of NUMBER token */
	char		strval[32];	/* value of STRING token */
};

/* Forward declarations of internal functions */

static int	check_ir_alloc(), scan_literal(), gettoken(),
		acl_init_ioctl();
static char	*tempdbname();

/* Declarations of external data and functions */

extern int	pw_nametoid(), gr_nametoid();
extern char	*calloc(), *malloc(), *realloc(), *getenv(),
		*pw_idtoname(), *gr_idtoname();
extern void	setpwent(), setgrent();


/*
 * Initialize parameters for discretionary access control
 */
int
acl_init()
{
	FILE	*fp;
	int	ret, fields;
	char	buffer[100];
	long	minor_device;

	if (aclinit)	/* only do this once */
		return 0;

	/*
	 * Retrieve values from kernel
	 */

	if (acl_init_ioctl())
		return -1;

	/* read in the parameters file */
	fp = fopen(ACL_PARAM_FILE, "r");
	if (fp == NULL)
		return -1;

	/* get parameter line */
	do {
		if (fgets(buffer, sizeof buffer, fp) == NULL) {
			fclose(fp);
			return -1;
		}
	} while (buffer[0] == '\n' || buffer[0] == '#');

	/* parse it into fields */
	fields = sscanf(buffer, "%s %ld %d",
			acl_config.dbase, &acl_config.cache_size,
			&acl_config.buffers);

	if (fields != 3) {	/* format error */
		fprintf(stderr, MSGSTR(ACLLIB_1, "Error reading ACL configuration file\n"));
		ret = -1;
	} else			/* success */
		ret = 0;
	fclose(fp);

	aclinit = 1;
	return ret;
}

/*
 * Initialize parameters for discretionary daemon
 * Return cached copy of initialization parameters
 */
int
acldaemon_init(sp)
	struct sp_init	*sp;
{
	if (acl_init() != 0)
		return -1;

	*sp = sp_init;

	return 0;
}

/*
 * Get ACL policy configuration parameters from policy module
 */
static int
acl_init_ioctl()
{
	int		ret, acl_device;

#ifndef SEC_STANDALONE /*{*/

	/* perform the ioctl to get parms */

	if ((acl_device = open(SPD_CONTROL_DEVICE, O_RDWR)) == -1) {
		perror(MSGSTR(ACLLIB_2, "Error opening policy control device"));
		return -1;
	}

	sp_init.magic = SEC_ACL_MAGIC;

	ret = ioctl(acl_device, SPIOC_GETCONF, &sp_init);
	close(acl_device);
	if (ret == -1)
		return ret;

	acl_config.subj_tags = sp_init.subj_tag_count;
	acl_config.obj_tags = sp_init.obj_tag_count;
	acl_config.first_subj_tag = sp_init.first_subj_tag;
	acl_config.first_obj_tag = sp_init.first_obj_tag;
	acl_config.minor_device = sp_init.spminor;
	acl_config.policy = sp_init.policy;

#endif /*} SEC_STANDALONE */

	return 0;
}

#ifndef SEC_STANDALONE /*{*/
/*
 * This routine is given a tag for an ACL and fills in the IR for the ACL.
 * A return value >= 0 indicates the number of entries in the ACL.
 * A return of -1 indicates an error.
 */

int
acl_tag_to_ir(tag, ir)
	tag_t tag;
	char *ir;
{
	register int spdfd;
	register int ret;
	register int acl_size, acl_entries;
	register struct spd_internal_rep *response;
	struct spd_get_attribute query;
	char spd_dev[sizeof(SP_DAEMON_DEVICE) + 32];

	acl_init();

	response = (struct spd_internal_rep *) calloc(1024, 1);
	if (response == (struct spd_internal_rep *) 0)
		return -1;

	sprintf(spd_dev, "%s%d", SP_DAEMON_DEVICE,
		acl_config.minor_device | 1);
	spdfd = open(spd_dev, O_RDWR);
	if (spdfd < 0) {
		free(response);
		return -1;
	}

	query.mhdr.msg_type = SPD_GET_ATTRIBUTE;
	query.tag = tag;
	ret = write(spdfd, &query, sizeof query);

	if (ret == sizeof query) {
		response->mhdr.msg_type = SPD_INTERNAL_REP;
		response->ir.ir_length = 1024 - SPD_INTERNAL_REP_SIZE;
		ret = read(spdfd, response, 1024);

		/* If the internal rep size is 0 it is a Null ACL */

		if(response->mhdr.error_code != 0)
			acl_entries = -1;
		else if (response->ir.ir_length == 0)
			acl_entries = 0;
		else if (ret <= 1024)  {
			memcpy(ir, response + 1, response->ir.ir_length);
			acl_entries = response->ir.ir_length / sizeof(acle_t);
		}
	} else
		acl_entries = -1;

	(void) close(spdfd);
	free(response);

	return(acl_entries);
}
#endif /*} SEC_STANDALONE */

/*
 * Convert ACL internal representation to external representation
 */
char *
acl_ir_to_er(ep, count)
	register acle_t	*ep;	/* pointer to ACL entries */
	register int	count;	/* number of ACL entries */
{
	register int	cc;

	/*
	 * Allocate a buffer to hold the external representation.
	 * We retain this buffer between calls and grow it as
	 * needed.
	 */
	if (acl_ersz == 0) {
		acl_er = malloc(INIT_BUFSIZE);
		if (acl_er == NULL)
			return NULL;
		acl_ersz = INIT_BUFSIZE;
	}

	cc = 0;		/* running character count */

	for (; count-- > 0; ++ep) {
		/*
		 * Expand er buffer if necessary
		 * 24 character limit is broken down as follows:
		 *	< user . group , perm > \0
		 *	1  +8 +1  +8  +1  +3 +1 +1	= 24
		 */
		while (cc + 24 > acl_ersz) {
			char	*nbp = realloc(acl_er, acl_ersz + INIT_BUFSIZE);

			if (nbp == NULL)
				return NULL;
			acl_er = nbp;
			acl_ersz += INIT_BUFSIZE;
		}

		cc += acl_1ir_to_er(ep, &acl_er[cc]);
		acl_er[cc++] = ' ';	/* separate entries */
	}

	if (cc)
		--cc;		/* overwrite trailing space */
	acl_er[cc] = '\0';

	return acl_er;
}

/*
 * Convert one internal representation into a string
 */
int
acl_1ir_to_er(ep, str)
	register acle_t	*ep;
	char		*str;
{
	char		*u, *g;
	static char	pwdbuf[16], grpbuf[16];

	/* Convert user name */
	if (ep->acl_uid == (uid_t) ACL_WILDCARD)
		u = "*";
	else if (ep->acl_uid == (uid_t) ACL_OWNER)
		u = "@";
	else if ((u = pw_idtoname(ep->acl_uid)) == NULL) {
		sprintf(pwdbuf, "%d", ep->acl_uid);
		u = pwdbuf;
	}

	/* Convert group name */
	if (ep->acl_gid == (gid_t) ACL_WILDCARD)
		g = "*";
	else if (ep->acl_gid == (gid_t) ACL_OWNER)
		g = "@";
	else if ((g = gr_idtoname(ep->acl_gid)) == NULL) {
		sprintf(grpbuf, "%d", ep->acl_gid);
		g = grpbuf;
	}

	/* Assemble the components with punctuation */
	sprintf(str, "<%.8s.%.8s,%s>", u, g, perms[ep->acl_perm & 7]);
	return (strlen(str));
}

#ifndef SEC_STANDALONE /*{*/
/*
 * This routine is given an IR and entry count for an ACL and fills in
 * the tag for that IR.  It also returns 1 to note that the operation
 * succeeded or 0 to signify that the operation failed and the tag
 * is left unchanged.
 */

int
acl_ir_to_tag(ir, count, tag)
	acle_t *ir;
	int count;
	tag_t *tag;
{
	register int spdfd;
	register int ret;
	register int msg_size;
	register struct spd_map_tag *query;
	int got_tag = 0;
	int acl_ir_size;
	struct spd_set_tag response;
	char spd_dev[sizeof(SP_DAEMON_DEVICE) + 32 + 2];

	if (acl_init() != 0)
		return got_tag;

	acl_ir_size = count * sizeof(acle_t);
	msg_size = sizeof(*query) + acl_ir_size;
	query = (struct spd_map_tag *) calloc(msg_size, 1);
	if (query != (struct spd_map_tag *) 0)  {
	    sprintf(spd_dev, "%s%d", SP_DAEMON_DEVICE,
		    acl_config.minor_device | CLIENT);
	    spdfd = open(spd_dev, O_RDWR);

	    if (spdfd >= 0)  {
		query->mhdr.msg_type = SPD_MAP_TAG;
		query->ir.ir_length = acl_ir_size;
		memcpy(query + 1, ir, acl_ir_size);
		ret = write(spdfd, query, msg_size);

		if (ret == msg_size)  {
			msg_size = sizeof(response);
			response.mhdr.msg_type = SPD_SET_TAG;
			ret = read(spdfd, &response, msg_size);

			if (ret == msg_size)  {
				*tag = response.tag;
				got_tag = 1;
			}
		}
		(void) close(spdfd);
	    }
	    free(query);
	}

	return got_tag;
}
#endif /*} SEC_STANDALONE */

/*
 * Convert ACL external representation to internal representation
 */
acle_t *
acl_er_to_ir(string, size)
	char	*string;
	int	*size;
{
	struct lexstate	ls;
	register int	count, t;
	aclsyn_t	*sp;

	/* allocate an ir buffer on first call */
	if (acl_irsz == 0) {
		acl_ir = (acle_t *)calloc(INIT_ACLSIZE, sizeof(acle_t));
		if (acl_ir == NULL)
			return NULL;
		acl_irsz = INIT_ACLSIZE;
	}

	ls.string = ls.pos = string;
	count = 0;

	while ((t = gettoken(&ls)) != EOF) {
		if (t == '<') {			/* a literal ACL entry */
			if (check_ir_alloc(count + 1, INIT_ACLSIZE) == ACL_ERR
			||  scan_literal(&ls, &acl_ir[count]) == ACL_ERR)
				return NULL;
			++count;
		} else if (t == STRING) {	/* a synonym */
			sp = acl_lookup_syn(ls.strval);
			if (sp == NULL
			||  check_ir_alloc(count + sp->syn_count,
				    sp->syn_count + INIT_ACLSIZE) == ACL_ERR)
				return NULL;
			if (sp->syn_count > 0) {
				(void)memcpy((char *)(&acl_ir[count]),
					(char *)sp->syn_ents,
					sp->syn_count * sizeof(acle_t));
				count += sp->syn_count;
			}
		} else			/* unrecognizable */
			return NULL;
	}
	*size = count;
	return acl_ir;
}

/*
 * Lookup an ACL synonym by name
 */
aclsyn_t *
acl_lookup_syn(name)
	char	*name;
{
	aclsyn_t	*sp;
	int		cmp;

	acl_load_syns(NULL);	/* make sure synonym file is loaded */

	for (sp = acl_syns.syn_next; sp; sp = sp->syn_next) {
		cmp = strcmp(name, sp->syn_name);
		if (cmp == 0)	/* found it */
			return sp;
		if (cmp < 0)
			return NULL;
	}
	return sp;
}

/*
 * Make a new synonym structure
 */
aclsyn_t *
acl_alloc_syn(ents)
	int	ents;
{
	register aclsyn_t	*sp;

	sp = (aclsyn_t *)calloc(1, sizeof(aclsyn_t));
	if (sp && acl_expand_syn(sp, ents) == ACL_ERR) {
		free(sp);
		return NULL;
	}
	return sp;
}

/*
 * See if pointer lies in contiguous space allocated to read in synonym file
 */
#define	in_contig_range(p) \
	(acl_synbuf <= (char *)(p) && (char *)(p) < &acl_synbuf[acl_synspace])

/*
 * Expand a synonym's ACL buffer (if necessary) to
 * accommodate the specified number of additional entries
 */
int
acl_expand_syn(sp, count)
	register aclsyn_t	*sp;
	register int		count;
{
	register int	size;
	acle_t		*ptr;

	if (count <= 0)		/* don't need any more entries */
		return ACL_OK;

	/*
	 * Set count to the next multiple of INIT_ACLSIZE that is
	 * greater than or equal to the required number of entries.
	 * Set size to the current amount of space allocated.
	 * INIT_ACLSIZE must be a power of 2 for the following
	 * computations to work.
	 */
	count = (sp->syn_count + count + INIT_ACLSIZE - 1) & -INIT_ACLSIZE;
	if (in_contig_range(sp->syn_ents))
		size = sp->syn_count;
	else
		size = (sp->syn_count + INIT_ACLSIZE - 1) & -INIT_ACLSIZE;

	if (count <= size)	/* already big enough */
		return ACL_OK;

	if (sp->syn_ents == NULL || in_contig_range(sp->syn_ents))
		ptr = (acle_t *)malloc(count * sizeof(acle_t));
	else
		ptr = (acle_t *)realloc(sp->syn_ents, count * sizeof(acle_t));

	if (ptr == NULL)
		return ACL_ERR;

	sp->syn_ents = ptr;

	return ACL_OK;
}

/*
 * Deallocate a synonym structure
 */
void
acl_free_syn(sp)
	register aclsyn_t	*sp;
{
	if (sp == NULL)		/* nothing to free */
		return;

	if (sp->syn_name && !in_contig_range(sp->syn_name))
		free(sp->syn_name);
	if (sp->syn_ents && !in_contig_range(sp->syn_ents))
		free(sp->syn_ents);
	if (!in_contig_range(sp))
		free(sp);
}

/*
 * Add a synonym to the list
 */
void
acl_insert_syn(sp)
	aclsyn_t		*sp;
{
	register aclsyn_t	*cur, *nxt;

	if (sp == NULL)			/* nothing to insert */
		return;

	++acl_syns.syn_count;

	if (acl_syns.syn_next == NULL) {	/* empty list, insert at head */
		acl_syns.syn_next = sp;
		return;
	}

	/*
	 * Scan the list to find the appropriate insertion point
	 * in order to maintain ascending lexical order.
	 */
	cur = &acl_syns;
	while ((nxt = cur->syn_next) && STRGE(sp->syn_name, nxt->syn_name))
		cur = nxt;
	sp->syn_next = nxt;
	cur->syn_next = sp;
}

/*
 * Delete a synonym
 */
int
acl_delete_syn(name)
	char	*name;
{
	register aclsyn_t	*sp, *nsp;
	int			cmp;

	for (sp = &acl_syns; (nsp = sp->syn_next) != NULL; sp = nsp) {
		cmp = strcmp(name, nsp->syn_name);
		if (cmp == 0) {
			sp->syn_next = nsp->syn_next;
			acl_free_syn(nsp);
			--acl_syns.syn_count;
			return ACL_OK;
		}
		if (cmp < 0)
			break;
	}
	return ACL_ERR;
}

/*
 * Load the ACL synonym database.  If no file is specified,
 * look first in the file specified by the ACLSYNDB envariable,
 * then in the default file in the current directory, then in
 * the default file in the user's home directory.
 */
int
acl_load_syns(dbfile)
	char	*dbfile;
{
	int	ret, fd = -1;
	char	*home;
	FILE	*fp;

	/*
	 * If file has already been loaded or some synonyms
	 * have been defined, don't try to load again.
	 */
	if (acl_synbuf || acl_syns.syn_count)
		return ACL_OK;

	acl_synbuf = (char *)-1;

	if (dbfile && (fd = open(dbfile, 0)) < 0)
		return ACL_ERR;

	if (fd < 0) {
		dbfile = getenv(ACL_DBENV);
		if (dbfile == NULL || (fd = open(dbfile, 0)) < 0) {
			dbfile = ACL_DBFILE;
			if ((fd = open(dbfile, 0)) < 0) {
				home = getenv("HOME");
				if (home == NULL)
					return ACL_ERR;
				dbfile = malloc(strlen(home)
					+ strlen(ACL_DBFILE) + 2);
				if (dbfile == NULL)
					return ACL_ERR;
				strcpy(dbfile, home);
				strcat(dbfile, "/");
				strcat(dbfile, ACL_DBFILE);
				if ((fd = open(dbfile, 0)) < 0)
					return ACL_ERR;
			}
		}
	}

	fp = fdopen(fd, "r");
	if (fp == NULL) {
		close(fd);
		return ACL_ERR;
	}

	ret = acl_read_syn_db(fp);
	fclose(fp);

	if (ret == ACL_OK) {
		acl_dbfile = dbfile;
#ifdef DEBUG
		printf(MSGSTR(ACLLIB_3, "%d synonyms loaded from %s\n"),
				acl_syns.syn_count, dbfile);
#endif
	}
	return ret;
}

/*
 * Read an ACL synonym file
 * The file consists of a four word header followed
 * by three sections:
 *
 *				     offset
 *	 ---------------------- 
 *	|    header.magic      |	0
 *	|    header.syns       |
 *	|    header.strings    |
 *	|    header.entries    |
 *	|----------------------|
 *	|  synonym structures  |	16
 *	|----------------------|
 *	|    synonym names     | 16 + (syns * 16)
 *	|----------------------|
 *	|      ACL entries     | 16 + (syns * 16) + strings
 *	 ---------------------- 
 *
 * The 'syns' and 'entries' header fields contain the number
 * of structures in the corresponding section of the file.
 * The 'strings' field contains the number of bytes in the
 * synonym names section.  Each synonym name includes a null 
 * terminator.
 * 
 * The pointer fields (syn_name and syn_ents) in the synonym
 * structures contain offsets into the synonym names and ACL
 * entries sections of the file.  The offsets are in units of
 * bytes for the syn_name field and number of ACL entries for
 * the syn_ents field, and are relative to the start of the
 * section.
 */
int
acl_read_syn_db(fp)
	FILE	*fp;
{
	register aclsyn_t	*syns = NULL, *sp;
	register char		*names = NULL;
	register acle_t		*ents = NULL;
	register int		i, prevname, exp_ents;
	struct dbhead		h;

	/* Read header and check magic number */
	if (fread(&h, sizeof h, 1, fp) != 1 || h.magic != ACL_MAGIC)
		return ACL_ERR;

	/* Compute space needed to hold database and allocate it */
	acl_synspace = h.syns * sizeof(aclsyn_t)
			+ h.strings
			+ h.entries * sizeof(acle_t);
	acl_synbuf = malloc(acl_synspace);
	syns = (aclsyn_t *)acl_synbuf;
	ents = (acle_t *)(syns + h.syns);
	names = (char *)(ents + h.entries);

	if (acl_synbuf == NULL
	||  fread((char *)syns, sizeof(aclsyn_t), h.syns, fp) != h.syns
	||  fread(names, 1, h.strings, fp) != h.strings
	||  fread((char *)ents, sizeof(acle_t), h.entries, fp) != h.entries
	||  getc(fp) != EOF)
		goto err;

	/*
	 * Walk the table adjusting the syn_name and syn_ents fields
	 */
	prevname = -1;		/* syn_name field from previous synonym */
	exp_ents = 0;		/* expected value for syn_ents field */
	for (i = h.syns, sp = syns; i > 0; --i, ++sp) {
		/* check consistency of this synonym */
		if ((unsigned)sp->syn_name >= h.strings
				||  (int)sp->syn_name <= prevname
				||  (int)sp->syn_ents != exp_ents
				||  sp->syn_count + exp_ents > h.entries)
			goto err;
		sp->syn_next = (i > 1) ? sp + 1 : NULL;
		prevname = (int)sp->syn_name;
		sp->syn_name = names + prevname;
		sp->syn_ents = ents + exp_ents;
		exp_ents += sp->syn_count;
	}

	acl_syns.syn_count = h.syns;
	if (h.syns)
		acl_syns.syn_next = syns;
	return ACL_OK;

err:
	if (acl_synbuf)
		free(acl_synbuf);
	acl_synbuf = (char *)-1;
	return ACL_ERR;
}

/*
 * Create the synonym database file from the internal list
 * If no file name is specified, use the name stored in the
 * ACLSYNDB envariable, if defined, else the default file name.
 */
acl_store_syns(dbfile)
	char	*dbfile;
{
	struct stat	stb;
	FILE		*fp;
	char		*newdbfile;
	int		ret, fd;

	if (dbfile == NULL) {
		dbfile = getenv(ACL_DBENV);
		if (dbfile == NULL)
			dbfile = ACL_DBFILE;
	}

	newdbfile = tempdbname(dbfile, "acl");
	if (newdbfile == NULL)
		return ACL_ERR;

	fd = open(newdbfile, O_RDWR|O_CREAT|O_EXCL, 0600);
	if (fd < 0) {
		free(newdbfile);
		return ACL_ERR;
	}
	fp = fdopen(fd, "r+");
	if (fp == NULL) {
		close(fd);
		return ACL_ERR;
	}

	ret = acl_write_syn_db(fp);
	fclose(fp);

	if (ret == ACL_OK) {
		if (stat(dbfile, &stb) == 0) {
			chmod(newdbfile, stb.st_mode);
			chown(newdbfile, stb.st_uid, stb.st_gid);
			unlink(dbfile);
		}
		if (link(newdbfile, dbfile) != 0)
			ret = ACL_ERR;
	}

	unlink(newdbfile);
	free(newdbfile);
	if (ret == ACL_OK && acl_dbfile == NULL)
		acl_dbfile = dbfile;
	return ret;
}

/*
 * Write synonym list to a file
 */
acl_write_syn_db(fp)
	FILE			*fp;
{
	struct dbhead		h;
	int			len;
	register aclsyn_t	*sp;
	aclsyn_t		syn;

	/* Initialize header */
	h.magic = ACL_MAGIC;
	h.syns = 0;
	h.strings = 0;
	h.entries = 0;

	/* Leave room for header; it will be filled in later */
	if (fseek(fp, sizeof h, 0) != 0)
		return ACL_ERR;

	/*
	 * Walk the synonym list to write out the synonym
	 * structures.  Compute the values of the three
	 * header fields.
	 */
	syn.syn_next = NULL;
	for (sp = acl_syns.syn_next; sp; sp = sp->syn_next) {
		syn.syn_name = (char *)h.strings;
		syn.syn_count = sp->syn_count;
		syn.syn_ents = (acle_t *)h.entries;
		if (fwrite(&syn, sizeof syn, 1, fp) != 1)
			return ACL_ERR;
		h.syns += 1;
		h.strings += strlen(sp->syn_name) + 1;
		h.entries += sp->syn_count;
	}

	/* Walk the list again to write out the synonym names */
	for (sp = acl_syns.syn_next; sp; sp = sp->syn_next) {
		len = strlen(sp->syn_name) + 1;
		if (fwrite(sp->syn_name, 1, len, fp) != len)
			return ACL_ERR;
	}

	/* Walk the list one last time to write out the ACL entries */
	for (sp = acl_syns.syn_next; sp; sp = sp->syn_next)
		if (fwrite(sp->syn_ents, sizeof(acle_t),
				sp->syn_count, fp) != sp->syn_count)
			return ACL_ERR;
	
	/* Go back and write the header */
	if (fseek(fp, 0L, 0) != 0
			|| fwrite(&h, sizeof h, 1, fp) != 1
			|| fflush(fp) == EOF)
		return ACL_ERR;
	return ACL_OK;
}

/*
 * Check for sufficient room in ACL buffer.
 * Reallocate if necessary.
 */
static int
check_ir_alloc(need, incr)
	int	need, incr;
{
	register acle_t	*new_ir;

	if (need > acl_irsz) {
		if (acl_irsz == 0)	/* nothing allocated yet */
			new_ir = (acle_t *)calloc(incr, sizeof(acle_t));
		else
			new_ir = (acle_t *)realloc((char *)acl_ir,
				(acl_irsz + incr) * sizeof(acle_t));
		if (new_ir == NULL) {
#ifdef DEBUG
			printf(MSGSTR(ACLLIB_4, "check_ir_alloc fails:  out of memory\n"));
#endif
			return ACL_ERR;
		}
#ifdef DEBUG
		printf(MSGSTR(ACLLIB_5, "ir buffer expanded from %d to %d entries\n"),
			acl_irsz, acl_irsz + incr);
#endif
		acl_irsz += incr;
		acl_ir = new_ir;
	}
	return ACL_OK;
}

/*
 * Parse a literal ACL entry
 */
static int
scan_literal(ls, ep)
	register struct lexstate	*ls;
	register acle_t			*ep;
{
	register char	*cp;
	register int	perm, id;
	
	/* Interpret the user ID field */
	switch (gettoken(ls)) {
	case '*':
		ep->acl_uid = (uid_t) ACL_WILDCARD;
		break;
	case '@':
		ep->acl_uid = (uid_t) ACL_OWNER;
		break;
	case STRING:
		if ((id = pw_nametoid(ls->strval)) != -1)
			ep->acl_uid = (uid_t) id;
		else {
#ifdef DEBUG
			printf(MSGSTR(ACLLIB_6, "undefined user name (%s)\n"), ls->strval);
#endif
			return ACL_ERR;
		}
		break;
	case NUMBER:
		if (ls->numval != ACL_WILDCARD && ls->numval != ACL_OWNER)
			ep->acl_uid = (uid_t) ls->numval;
		else {
#ifdef DEBUG
			printf(MSGSTR(ACLLIB_7, "user ID out of range (%u)\n"), ls->numval);
#endif
			return ACL_ERR;
		}
		break;
	default:
#ifdef DEBUG
		printf(MSGSTR(ACLLIB_8, "unrecognizable user ID\n"));
#endif
		return ACL_ERR;
	}

	if (gettoken(ls) != '.') {	/* Skip separator */
#ifdef DEBUG
		printf(MSGSTR(ACLLIB_9, "missing . between uid and gid\n"));
#endif
		return ACL_ERR;
	}

	/* Interpret the group ID field */
	switch (gettoken(ls)) {
	case '*':
		ep->acl_gid = (gid_t) ACL_WILDCARD;
		break;
	case '@':
		ep->acl_gid = (gid_t) ACL_OWNER;
		break;
	case STRING:
		if ((id = gr_nametoid(ls->strval)) != -1)
			ep->acl_gid = (gid_t) id;
		else {
#ifdef DEBUG
			printf(MSGSTR(ACLLIB_10, "undefined group name (%s)\n"), ls->strval);
#endif
			return ACL_ERR;
		}
		break;
	case NUMBER:
		if (ls->numval != ACL_WILDCARD && ls->numval != ACL_OWNER)
			ep->acl_gid = (gid_t) ls->numval;
		else {
#ifdef DEBUG
			printf(MSGSTR(ACLLIB_11, "group ID out of range (%u)\n"), ls->numval);
#endif
			return ACL_ERR;
		}
		break;
	default:
#ifdef DEBUG
		printf(MSGSTR(ACLLIB_12, "unrecognizable group ID\n"));
#endif
		return ACL_ERR;
	}

	if (gettoken(ls) != ',') {	/* Skip separator */
#ifdef DEBUG
		printf(MSGSTR(ACLLIB_13, "missing , between gid and perm\n"));
#endif
		return ACL_ERR;
	}

	switch (gettoken(ls)) {
	case STRING:
		for (cp = ls->strval; *cp; ++cp)
			if (isupper(*cp))
				*cp = tolower(*cp);

		if (STREQ(ls->strval, "all"))
			perm = ACL_READ | ACL_WRITE | ACL_EXEC;
		else if (STREQ(ls->strval, "none") || STREQ(ls->strval, "null"))
			perm = 0;
		else for (perm = 0, cp = ls->strval; *cp; ++cp)
			switch (*cp) {
			case 'r':
				perm |= ACL_READ;
				break;
			case 'w':
				perm |= ACL_WRITE;
				break;
			case 'x':
				perm |= ACL_EXEC;
				break;
			case '-':
				break;
			default:
#ifdef DEBUG
				printf(MSGSTR(ACLLIB_14, "bad character in perm string\n"));
#endif
				return ACL_ERR;
			}
		ep->acl_perm = perm;
		break;
	case NUMBER:
		if (ls->numval <= 7)
			ep->acl_perm = ls->numval;
		else {
#ifdef DEBUG
			printf(MSGSTR(ACLLIB_15, "perm value out of range (%u)\n"), ls->numval);
#endif
			return ACL_ERR;
		}
		break;
	default:
#ifdef DEBUG
		printf(MSGSTR(ACLLIB_16, "unrecognizable perm string\n"));
#endif
		return ACL_ERR;
	}

	if (gettoken(ls) != '>') {	/* Skip terminator */
#ifdef DEBUG
		printf(MSGSTR(ACLLIB_17, "missing > at end of entry\n"));
#endif
		return ACL_ERR;
	}

	return ACL_OK;
}

/*
 * Scan for tokens
 */
static int
gettoken(ls)
	register struct lexstate	*ls;
{
	register char		*cp = ls->pos;
	register unsigned	num, r;
	int			token;

	while (isspace(*cp))		/* skip whitespace */
		++cp;
	if (*cp == '\0') {		/* end of string */
		ls->pos = cp;
#ifdef DEBUG
		printf(MSGSTR(ACLLIB_18, "gettoken returns EOF\n"));
#endif
		return EOF;
	}

	/*
	 * Figure out what kind of token we have.
	 */
	switch (*cp) {
	case '<':	/* single character tokens represent themselves */
	case '>':
	case '.':
	case ',':
	case '@':
	case '*':
		token = *cp++;
#ifdef DEBUG
		printf(MSGSTR(ACLLIB_19, "gettoken returns %c\n"), token);
#endif
		break;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		num = *cp++ - '0';
		/*
		 * Interpret numbers with leading 0 or 0x
		 * according to C conventions.
		 */
		if (num == 0)
			if (*cp == 'x' || *cp == 'X') {
				r = 16;
				++cp;
			} else
				r = 8;
		else
			r = 10;
		while (isdigit(*cp) || r == 16 && isxdigit(*cp)) {
			num *= r;
			num += *cp - (isdigit(*cp)?'0':(islower(*cp)?'a':'A'));
			++cp;
		}
		ls->numval = num;
#ifdef DEBUG
		printf(MSGSTR(ACLLIB_20, "gettoken returns NUMBER (%d)\n"), ls->numval);
#endif
		token = NUMBER;
		break;
	default:
		/*
		 * Anything that's not a number or a single
		 * character token is the start of a STRING
		 * token which is handled below.
		 */
		token = STRING;
		break;
	}

	/*
	 * If token is a STRING, note its starting position.
	 * Otherwise, remember input position to start
	 * scanning on next call and return.
	 */
	ls->pos = cp;
	if (token != STRING)
		return token;
	
	/*
	 * Find end of STRING and copy to caller's buffer
	 */
	cp += strcspn(cp, " \t\n<@*.,>");
	num = cp - ls->pos;
	if (num >= sizeof ls->strval)
		num = sizeof ls->strval - 1;
	strncpy(ls->strval, ls->pos, num);
	ls->strval[num] = '\0';
	ls->pos = cp;
#ifdef DEBUG
	printf(MSGSTR(ACLLIB_21, "gettoken returns STRING \"%s\")\n", ls->strval);
#endif
	return STRING;
}

/*
 * Make a temporary filename from the supplied pathname.
 * The temp file is in the same directory as the original
 * so that it can be renamed to replace the original.
 */
static char *
tempdbname(path, seed)
	char	*path, *seed;
{
	register char	*cp;
	char		*newfile;

	/*
	 * Make a copy of the original pathname.
	 * Get enough extra space to hold the temp
	 * component name (length of seed string +
	 * 5 bytes for pid + 1 byte for $ + 1 byte
	 * for null terminator.
	 */
	newfile = malloc(strlen(path) + strlen(seed) + 7);
	if (newfile == NULL)
		return NULL;
	strcpy(newfile, path);

	cp = strrchr(newfile, '/');
	if (cp && cp[1] == '\0') {	/* allow for / at end of pathname */
		while (cp > newfile && *--cp == '/')
			;
		cp[1] = '\0';
		cp = strrchr(newfile, '/');
	}
	if (cp == NULL)
		cp = newfile;
	else
		++cp;
	sprintf(cp, "%s$%d", seed, getpid());
	return newfile;
}
#endif
