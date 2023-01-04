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
#ifdef ultrix
static char     *sccsid = "%W%  ULTRIX  %G%";
#else /* ultrix */
static char     *sccsid = "@(#)$RCSfile: auto_look.c,v $ $Revision: 4.2.6.5 $ (DEC) $Date: 1993/12/15 18:29:28 $";
#endif /* ultrix */
#endif /* lint */

/*
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */

/*
 *      Modification History:
 *
 *      4 Sep 90 -- condylis
 *              Changed getmapent_hosts() to try to query mountd's TCP
 *              port first when getting an export list from a host.
 */

#include <stdio.h>
#include <ctype.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <pwd.h>
#include <netinet/in.h>
#include <netdb.h>
extern	int h_errno;		/* Need for Ultrix */
#include <rpc/types.h>
#include <rpc/auth.h>
#include <rpc/auth_unix.h>
#include <rpc/xdr.h>
#include <rpc/clnt.h>
#include <rpc/svc.h>
#ifdef ultrix				/* JFS */
typedef quad fsid_t;                    /* file system id type */
#include "auto_ultrix.h"
#endif /* ultrix */
#include <rpcsvc/ypclnt.h>
#include <sys/socket.h>
#define NFSCLIENT
#include <sys/mount.h>
#include "nfs_prot.h"
#include <rpcsvc/mount.h>
#include "automount.h"

struct timeval TIMEOUT = { 25, 0 };
#define	MAXHOSTS 20
nfsstat do_mount();
void diag();
void getword();
void unquote();
void macro_expand();
extern int trace;

/*
 * Debugging aid:
 *
 *      If you do the following command:
 *
 *      file /net/=x
 *
 *      where /net is directory being watched by automount
 *      and x is one of the following options:
 *
 *              0       turn trace off (= no -T)
 *              1       turn first level trace on (= -T)
 *              2       turn second level trace on (= -T -T)
 *              v       toggle verbose (flips state of -v)
 *              d       print dir
 *              n       print vNodes
 *              f       print fs
 */
nfsstat
lookup(dir, name, vpp, cred)
	struct autodir *dir;
	char *name;
	struct vnode **vpp;
	struct authunix_parms *cred;
{
	struct mapent *me;
	struct mapfs *mfs;
	struct link *link;
	struct filsys *fs;
	char *linkpath;
	nfsstat status;

	if (name[0] == '.' &&
	    (name[1] == '\0' || (name[1] == '.' && name[2] == '\0'))) {
		*vpp = &dir->dir_vnode;
		return (NFS_OK);
	}


	if ((link = findlink(dir, name)) && link->link_fs) {
		link->link_death = time_now + max_link_time;
		link->link_fs->fs_rootfs->fs_death = time_now + max_link_time;
		*vpp =  &link->link_vnode;
		 return (NFS_OK);
	}

	me = getmapent(dir->dir_map, dir->dir_opts, name, cred);
	if (me == NULL) {
		if (*name == '=' && cred->aup_uid == 0) {
			if (isdigit(*((u_char *)name+1))) {
				trace = atoi(name+1);
				(void) fprintf(stderr, "automount trace = %d\n",
					trace);
			} else {
				diag(name+1);
			}
		}
		return (NFSERR_NOENT);
	}

	if (trace > 1) {
		struct mapent *ms;
	
		fprintf(stderr, "%s/ %s (%s)\n",
			dir->dir_name, name, me->map_root);
		for (ms = me ; ms ; ms = ms->map_next) {
			fprintf(stderr, "   %s \t-%s\t",
				*ms->map_mntpnt ? ms->map_mntpnt : "/",
				ms->map_mntopts);
			for (mfs = ms->map_fs ; mfs ; mfs = mfs->mfs_next)
				fprintf(stderr, "%s:%s%s%s ",
					mfs->mfs_host,
					mfs->mfs_dir,
					*mfs->mfs_subdir ? ":" : "",
					mfs->mfs_subdir);
			fprintf(stderr, "\n");
		}
	}

	fs = NULL;
	status = do_mount(dir, me, &fs, &linkpath);
	if (status != NFS_OK) {
		free_mapent(me);
		return (status);
	}

	link = makelink(dir, name, fs, linkpath);
	free_mapent(me);
	if (link == NULL)
		return (NFSERR_NOSPC);
	*vpp = &link->link_vnode;
	return (NFS_OK);
}

void
diag(s)
	char *s;
{
	register int i;
	struct autodir *dir;
	register struct vnode *vnode;
	register struct link *link;
	extern struct queue fh_q_hash[];
	register struct filsys *fs, *nextfs;
	extern int verbose;
	extern dev_t tmpdev;

	switch (*s) {
	case 'v':	/* toggle verbose */
		verbose = !verbose;
		syslog(LOG_ERR, "verbose %s", verbose ? "on" : "off");
		break;

	case 'd':		/* print dirs */
		for (dir = HEAD(struct autodir, dir_q); dir;
		     dir = NEXT(struct autodir, dir)) {
			fprintf(stderr, "%s: name %s",
				(dir->dir_vnode.vn_type == VN_LINK)? "link": "dir ",
				dir->dir_name);
			if (strcmp(dir->dir_name, dir->dir_truename))
				fprintf(stderr, " (truly %s)", dir->dir_truename);
			fprintf(stderr, ", map %s, opts %s, remove %d\n",
				dir->dir_map, dir->dir_opts,
				dir->dir_remove);
			if (link = HEAD(struct link, dir->dir_head)) {
				fprintf(stderr, " Links:\n");
				do
					diag_link(link, "   ");
				while (link = NEXT(struct link, link));
			}
		}
		break;

	case 'n':	/* print vnodes */
		for (i = 0; i < FH_HASH_SIZE; i++) {
			vnode = HEAD(struct vnode, fh_q_hash[i]);
			for (; vnode; vnode = NEXT(struct vnode, vnode)) {
				if (vnode->vn_type == VN_LINK)
					diag_link((struct link *)vnode->vn_data, "link: ");
				else {
					dir = (struct autodir *)vnode->vn_data;
					(void) fprintf(stderr, "dir : %s %s -%s\n",
						dir->dir_name, dir->dir_map,
						dir->dir_opts);
				}
			}
		} /* end for */
		break;

	case 'f':	/* print fs's */
		for (fs = HEAD(struct filsys, fs_q); fs; fs = nextfs) {
			nextfs = NEXT(struct filsys, fs);
			(void) fprintf(stderr, "%s %s:%s -%s ",
				fs->fs_mntpnt, fs->fs_host, fs->fs_dir,
				fs->fs_opts);
			if (fs->fs_mine || (trace > 1)) {
				(void) fprintf(stderr, "(%x)%x:%x ",
					tmpdev & 0xFFFF,
					fs->fs_mntpntdev & 0xFFFF,
					fs->fs_mountdev & 0xFFFF);
				(void) fprintf(stderr, "%d\n",
					fs->fs_death > time_now ?
					fs->fs_death - time_now : 0);
			}
			else
				(void) fprintf(stderr, "\n");
		} /* end for */
		break;
	}
}

/*
 * Routine to dump the most useful data associated with a struct link.
 */
diag_link(link, prefix)
	struct link *link;
	char *prefix;
{
	(void) fprintf(stderr, "%s%s/ %s ", prefix,
		       link->link_dir->dir_name,
		       link->link_name);
	if (link->link_path)
		(void) fprintf(stderr, "-> \"%s\" ",
			       link->link_path);
	if (link->link_fs)
		(void) fprintf(stderr, "@ %s:%s ",
			       link->link_fs->fs_host,
			       link->link_fs->fs_dir);
	(void) fprintf(stderr, "\t[%d]\n",
		       link->link_death <= 0 ? 0 :
		       link->link_death - time_now);
}

struct mapent *
getmapent(mapname, mapopts, key, cred)
	char *mapname, *mapopts, *key;
	struct authunix_parms *cred;
{
	FILE *fp = NULL;
	char *ypline = NULL;
	char *index();
	struct mapent *me = NULL;
	int len;
	char *get_line();
	char word[64], wordq[64];
	char *p;
	char *lp, *lq, linebuf[2048], linebufq[2048];
	extern int verbose, syntaxok;
	struct mapent *getmapent_hosts();
	struct mapent *getmapent_passwd();
	struct mapent *do_mapent();

	if (strcmp(mapname, "-hosts") == 0)
		return getmapent_hosts(mapopts, key);
	if (strcmp(mapname, "-passwd") == 0)
		return getmapent_passwd(mapopts, key, cred);

	if ((fp = fopen(mapname, "r")) != NULL) {
		/*
		 * The map is in a file, and the file exists; scan it.
		 */
		for (;;) {
			lp = get_line(fp, linebuf, sizeof linebuf);
			if (lp == NULL) {
				(void) fclose(fp);
				return ((struct mapent *)0);
			}

			/* now have complete line */

			if (verbose && syntaxok && isspace(*(u_char *)lp)) {
				syntaxok = 0;
				syslog(LOG_ERR,
				"leading space in map entry \"%s\" in %s",
				lp, mapname);
			}

			lq = linebufq;
			unquote(lp, lq);
			getword(word, wordq, &lp, &lq, ' '); /* Get key */

			p = &word[strlen(word) - 1];
			if (*p == '/')
				*p = '\0';	/* delete trailing / */
			if (word[0] == '*' && word[1] == '\0')
				break;
			if (strcmp(word, key) == 0)
				break;
			if (word[0] == '+') {
				me = getmapent(word+1, mapopts, key, cred);
				if (me != NULL) {
					(void) fclose(fp);
					return me;
				}
			}
			/*
			 * sanity check each map entry key against
			 * the lookup key as the map is searched.
			 */
			if (verbose && syntaxok) { /* sanity check entry */
				if (*key == '/') {
					if (*word != '/') {
						syntaxok = 0;
						syslog(LOG_ERR,
						"bad key \"%s\" in direct map %s\n",
						word, mapname);
					}
				} else {
					if (strchr(word, '/')) {
						syntaxok = 0;
						syslog(LOG_ERR,
						"bad key \"%s\" in indirect map %s\n",
						word, mapname);
					}
				}
			}

		}
		(void) fclose(fp);
	} else {
		/*
		 * The map is a NIS map, or is claimed to be a file but
		 * the file does not exist; just lookup the NIS entry.
		 */
		if (try_yp(mapname, key, &ypline, &len))
			return ((struct mapent *)NULL);

		/* trim the NIS entry - ignore # and beyond */
		if (lp = index(ypline, '#'))
			*lp = '\0';
		len = strlen(ypline);
		if (len <= 0)
			goto done;
		/* trim trailing white space */
		lp = &ypline[len - 1];
		while (lp > ypline && isspace(*(u_char *)lp))
			*lp-- = '\0';
		if (lp == ypline)
			goto done;
		(void) strcpy(linebuf, ypline);
		lp = linebuf;
		lq = linebufq;
		unquote(lp, lq);
	}

	/* now have correct line */

	me = do_mapent(lp, lq, mapname, mapopts, key);

done:
	if (ypline)
		FREE((char *)ypline);
	return (me);
}

struct mapent *
do_mapent(lp, lq, mapname, mapopts, key)
	char *lp, *lq, *mapname, *mapopts, *key;
{
	char w[1024], wq[1024];
	char entryopts[1024];
	struct mapent *me, *mp, *ms;
	int err, implied;
	extern char *opt_check();
	extern int syntaxok;
	char *p;

	macro_expand(key, lp, lq);
	if (trace > 1)
		(void) fprintf(stderr, "\"%s\" \"%s\"\n", key, lp);

	getword(w, wq, &lp, &lq, ' '); /* Get default options or mount point */

	if (w[0] == '-') {	/* default mount options for entry */
		if (syntaxok && (p = opt_check(w+1))) {
			syntaxok = 0;
			syslog(LOG_ERR,
			"WARNING: option \"%s\" ignored for %s in %s",
			p, key, mapname); 
		}
		(void) strcpy(entryopts, w+1);
		mapopts = entryopts;
		getword(w, wq, &lp, &lq, ' '); /* Get mount point */
	}
	implied = *w != '/';

	ms = NULL;
	while (*w == '/' || implied) {
		mp = me;
		me = (struct mapent *)malloc(sizeof *me);
		if (me == NULL)
			goto alloc_failed;
		bzero((char *) me, sizeof *me);
		if (ms == NULL)
			ms = me;
		else
			mp->map_next = me;
		
		if (strcmp(w, "/") == 0 || implied)
			me->map_mntpnt = strdup("");
		else
			me->map_mntpnt = strdup(w);

		if (me->map_mntpnt == NULL)
			goto alloc_failed;

		if (implied)
			implied = 0;
		else
			getword(w, wq, &lp, &lq, ' '); /* Get options or location */

		if (w[0] == '-') {	/* mount options */
			if (syntaxok && (p = opt_check(w+1))) {
				syntaxok = 0;
				syslog(LOG_ERR,
				"WARNING: option \"%s\" ignored for %s in %s",
				p, key, mapname); 
			}
			me->map_mntopts = strdup(w+1);
			getword(w, wq, &lp, &lq, ' '); /* Get location */
		} else
			me->map_mntopts = strdup(mapopts);
		if (me->map_mntopts == NULL)
			goto alloc_failed;
		if (w[0] == '\0') {
			syslog(LOG_ERR, "map %s, key %s: no location for mount point %s",
			       mapname, key, me->map_mntpnt);
			goto bad_entry;
		}
		err = mfs_get(mapname, me, w, wq, &lp, &lq);
		if (err < 0)
			goto alloc_failed;
		if (err > 0)
			goto bad_entry;
		me->map_next = NULL;
	}

	if (*key == '/') {
		*w = '\0';	/* a hack for direct maps */
	} else {
		(void) strcpy(w, "/");
		(void) strcat(w, key);
	}
	ms->map_root = strdup(w);
	if (ms->map_root == NULL)
		goto alloc_failed;

	return (ms);

alloc_failed:
	syslog(LOG_ERR, "Memory allocation failed: %m");
bad_entry:
	free_mapent(ms);
	return ((struct mapent *) NULL);
}

try_yp(map, key, ypline, yplen)
	char *map, *key;
	char **ypline;
	int *yplen;
{
	int reason;
	extern int yp;

	if (!yp)
		return (1);

	reason = yp_match(mydomain, map, key, strlen(key), ypline, yplen);
	if (reason) {
		if (reason == YPERR_KEY) {
			/*
			 * Try the default entry "*"
			 */
			if (yp_match(mydomain, map, "*", 1, ypline, yplen))
				return (1);
		} else {
			syslog(LOG_ERR, "%s: %s", map, yperr_string(reason));
			return (1);
		}
	}
	return (0);
}

char *
get_line(fp, line, linesz)
	FILE *fp;
	char *line;
	int linesz;
{
	register char *p;
	register int len;
	int discard = 0;	/* Set when we see a long line */

retry:
	p = line;

	for (;;) {
		if (fgets(p, linesz - (p-line), fp) == NULL)
			return NULL;
trim:
		len = strlen(line);
		if (len <= 0) {
			p = line;
			continue;
		}
		p = &line[len - 1]; /* Get last character of line */

		/* Line too long or at EOF? */
		if (*p != '\n')
			if (p == &line[linesz - 2]) {
				syslog(LOG_ERR, "Line too long: %.40s...\n", line);
				discard = 1;
				goto retry; /* Get rest of line & junk it */
			} else {
				syslog(LOG_ERR, "Last line has no newline: %s", line);
				return NULL;
			}

		/* trim trailing white space */
		while (p > line && isspace(*(u_char *)p))
			*p-- = '\0';
		if (p == line)
			continue;
		/* if continued, get next line */
		if (*p == '\\')
			continue;
		/* ignore # and beyond */
		if (p = index(line, '#')) {
			*p = '\0';
			goto trim;
		}
		if (discard) {	/* Finally reached real EOL, want it? */
			discard = 0;
			goto retry; /* Nope, return next line */
		}
		return line;
	}
}

/*
 * Routine to parse a list of locations terminated by EOL or the next mount
 * point.  Approximate BNF for what's going on is:
 *
 * location ::= <hostlist>:<dirname>:<subdir>
 *	      | <hostlist>:<dirname>
 * hostlist ::= <host>
 *	      | <hostlist>,<host>
 *
 * w and wq point to a token that holds just the first location, i.e. the
 * result of a getword(..., ' ') call.  lp and lq point to the rest of the
 * line and are used to pickup the rest of the locations for the mount.
 *
 * This returns with w pointing to the next mount point, if any.
 */
mfs_get(mapname, me, w, wq, lp, lq)
	struct mapent *me;
	char *mapname, *w, *wq, **lp, **lq;
{
	struct mapfs *mfs, **mfsp;
	char *wlp, *wlq;
	char *hl, hostlist[1024], *hlq, hostlistq[1024];
	char hostname[MAXHOSTNAMELEN+1];
	char dirname[MAXPATHLEN+1], subdir[MAXPATHLEN+1];
	char qbuff[MAXPATHLEN+1];

	mfsp = &me->map_fs;
	*mfsp = NULL;

	while (*w && *w != '/') {
		wlp = w ; wlq = wq;
		getword(hostlist, hostlistq, &wlp, &wlq, ':');
		if (!*hostlist) {
			syslog(LOG_ERR, "Null location within map %s", mapname);
			goto bad_entry;
		}
		getword(dirname, qbuff, &wlp, &wlq, ':');
		if (!*dirname) {
			syslog(LOG_ERR, "Map %s: No hostname in location \"%s\"", mapname, w);
			goto bad_entry;
		}
		*subdir = '/'; *qbuff = ' ';
		getword(subdir+1, qbuff+1, &wlp, &wlq, ':');

		hl = hostlist ; hlq = hostlistq;
		for (;;) {
			getword(hostname, qbuff, &hl, &hlq, ',');
			if (!*hostname)
				break;
			mfs = (struct mapfs *)malloc(sizeof *mfs);
			if (mfs == NULL)
				return -1;
			bzero(mfs, sizeof *mfs);
			*mfsp = mfs;
			mfsp = &mfs->mfs_next;
	
			mfs->mfs_host = strdup(hostname);
			if (mfs->mfs_host == NULL)
				return -1;
			mfs->mfs_dir = strdup(dirname);
			if (mfs->mfs_dir == NULL)
				return -1;
			mfs->mfs_subdir = strdup( *(subdir+1) ? subdir : "");
			if (mfs->mfs_subdir == NULL)
				return -1;
		}
		getword(w, wq, lp, lq, ' ');
	}
	return 0;

bad_entry:
	return 1;
}

free_mapent(me)
	struct mapent *me;
{
	struct mapfs *mfs;
	struct mapent *m;

	while (me) {
		while (me->map_fs) {
			mfs = me->map_fs;
			if (mfs->mfs_host)
				FREE(mfs->mfs_host);
			if (mfs->mfs_dir)
				FREE(mfs->mfs_dir);
			if (mfs->mfs_subdir)
				FREE(mfs->mfs_subdir);
			me->map_fs = mfs->mfs_next;
			FREE((char *)mfs);
		}
		if (me->map_root)
			FREE(me->map_root);
		if (me->map_mntpnt)
			FREE(me->map_mntpnt);
		if (me->map_mntopts)
			FREE(me->map_mntopts);
		m = me->map_next;
		FREE((char *)me);	/* from all this misery */
		me = m;
	}
}

/*
 * Gets the next token from the string "p" and copies
 * it into "w".  Both "wq" and "w" are quote vectors
 * for "w" and "p".  Delim is the character to be used
 * as a delimiter for the scan.  A space means "whitespace".
 */
void
getword(w, wq, p, pq, delim)
	char *w, *wq, **p, **pq, delim;
{
	while ((delim == ' ' ? isspace(**(u_char **)p) : **p == delim)
			&& **pq == ' ')
		(*p)++, (*pq)++;

	while (**p &&
	     !((delim == ' ' ? isspace(**(u_char **)p) : **p == delim)
	       && **pq == ' ')) {
		*w++  = *(*p)++;
		*wq++ = *(*pq)++;
	}
	*w  = '\0';
	*wq = '\0';
}

/*
 * Performs text expansions in the string "pline".
 * "plineq" is the quote vector for "pline".
 * An identifier prefixed by "$" is replaced by the
 * corresponding environment variable string.  A "&"
 * is replaced by the key string for the map entry.
 */
void
macro_expand(key, pline, plineq)
	char *key, *pline, *plineq;
{
	register char *p,  *q;
	register char *bp, *bq;
	register char *s;
	char buffp[MAXMAPLEN], buffq[MAXMAPLEN];
	char envbuf[64], *pe;
	int expand = 0;
	char *getenv();

	p = pline ; q = plineq;
	bp = buffp ; bq = buffq;
	while (*p) {
		if (*p == '&' && *q == ' ') {	/* insert key */
			for (s = key ; *s ; s++) {
				*bp++ = *s;
				*bq++ = ' ';
			}
			expand++;
			p++; q++;
			continue;
		}

		if (*p == '$' && *q == ' ') {	/* insert env var */
			p++; q++;
			pe = envbuf;
			if (*p == '{') {
				p++ ; q++;
				while (*p && *p != '}') {
					*pe++ = *p++;
					q++;
				}
				if (*p) {
					p++ ; q++;
				}
			} else {
				while (*p && isalnum(*(u_char *)p)) {
					*pe++ = *p++;
					q++;
				}
			}
			*pe = '\0';
			s = getenv(envbuf);
			if (s) {
				while (*s) {
					*bp++ = *s++;
					*bq++ = ' ';
				}
				expand++;
			}
			continue;
		}
		*bp++ = *p++;
		*bq++ = *q++;

	}
	if (!expand)
		return;
	*bp = '\0';
	*bq = '\0';
	(void) strcpy(pline , buffp);
	(void) strcpy(plineq, buffq);
}

/* Removes quotes from the string "str" and returns
 * the quoting information in "qbuf". e.g.
 * original str: 'the "quick brown" f\ox'
 * unquoted str: 'the quick brown fox'
 *         qbuf: '    ^^^^^^^^^^^  ^ '
 */
void
unquote(str, qbuf)
	char *str, *qbuf;
{
	register int escaped, inquote, quoted;
	register char *ip, *bp, *qp;
	char buf[2048];

	escaped = inquote = quoted = 0;

	for (ip = str, bp = buf, qp = qbuf ; *ip ; ip++) {
		if (!escaped) {
			if (*ip == '\\') {
				escaped = 1;
				quoted++;
				continue;
			} else
			if (*ip == '"') {
				inquote = !inquote;
				quoted++;
				continue;
			}
		}

		*bp++ = *ip;
		*qp++ = (inquote || escaped) ? '^' : ' ';
		escaped = 0;
	}
	*bp = '\0';
	*qp = '\0';
	if (quoted)
		(void) strcpy(str, buf);
}

struct mapent *
getmapent_passwd(mapopts, login, cred)
	char *mapopts, *login;
	struct authunix_parms *cred;
{
	struct mapent *me;
	struct mapfs *mfs;
	struct passwd *pw;
	char buf[64];
	char *rindex();
	char *p;
	int c;

	if (login[0] == '~' && login[1] == 0) {
		pw = getpwuid(cred->aup_uid);
		if (pw)
			login = pw->pw_name;
	}
	else
		pw = getpwnam(login);
	if (pw == NULL)
		return ((struct mapent *)NULL);
	for (c = 0, p = pw->pw_dir ; *p ; p++)
		if (*p == '/')
			c++;
	if (c != 3)     /* expect "/dir/host/user" */
		return ((struct mapent *)NULL);

	me = (struct mapent *)malloc(sizeof *me);
	if (me == NULL)
		goto alloc_failed;
	bzero((char *) me, sizeof *me);
	me->map_mntopts = strdup(mapopts);
	if (me->map_mntopts == NULL)
		goto alloc_failed;
	mfs = (struct mapfs *)malloc(sizeof *mfs);
	if (mfs == NULL)
		goto alloc_failed;
	bzero((char *) mfs, sizeof *mfs);
	me->map_fs = mfs;
	(void) strcpy(buf, "/");
	(void) strcat(buf, login);
	mfs->mfs_subdir = strdup(buf);
	p = rindex(pw->pw_dir, '/');
	*p = '\0';
	p = rindex(pw->pw_dir, '/');
	mfs->mfs_host = strdup(p+1);
	if (mfs->mfs_host == NULL)
		goto alloc_failed;
	me->map_root = strdup(p);
	if (me->map_root == NULL)
		goto alloc_failed;
	me->map_mntpnt = strdup("");
	if (me->map_mntpnt == NULL)
		goto alloc_failed;
	mfs->mfs_dir = strdup(pw->pw_dir);
	if (mfs->mfs_dir == NULL)
		goto alloc_failed;
	(void) endpwent();
	return (me);

alloc_failed:
	syslog(LOG_ERR, "Memory allocation failed: %m");
	free_mapent(me);
	return ((struct mapent *)NULL);
}

struct mapent *
getmapent_hosts(mapopts, host)
	char *mapopts, *host;
{
	struct mapent *me, *ms, *mp;
	struct mapfs *mfs;
	struct exports *ex = NULL;
	struct exports *exlist, *texlist, **texp, *exnext;
	struct groups *gr;
	enum clnt_stat clnt_stat, pingmount(), callrpc();
	char name[MAXPATHLEN];
	int elen;
	struct hostent *hp;
	struct sockaddr_in sin;
	CLIENT *cl;
	int s;
	static time_t deadtime;
	static struct in_addr deadhost;
	int cache_time = 60;  /* sec */

	hp = gethostbyname(host);
	if (hp == NULL)
		return ((struct mapent *)NULL);

	/* check for special case: host is me */

	if (local_addr(*(struct in_addr *)hp->h_addr)) {
		ms = (struct mapent *)malloc(sizeof *ms);
		if (ms == NULL)
			goto alloc_failed;
		bzero((char *) ms, sizeof *ms);
		ms->map_root = strdup("");
		if (ms->map_root == NULL)
			goto alloc_failed;
		ms->map_mntpnt = strdup("");
		if (ms->map_mntpnt == NULL)
			goto alloc_failed;
		ms->map_mntopts = strdup("");
		if (ms->map_mntopts == NULL)
			goto alloc_failed;
		mfs = (struct mapfs *)malloc(sizeof *mfs);
		if (mfs == NULL)
			goto alloc_failed;
		bzero((char *) mfs, sizeof *mfs);
		mfs = (struct mapfs *)malloc(sizeof *mfs);
		if (mfs == NULL)
			goto alloc_failed;
		bzero((char *) mfs, sizeof *mfs);
		ms->map_fs = mfs;
		mfs->mfs_host = strdup(self);
		if (mfs->mfs_host == NULL)
			goto alloc_failed;
		mfs->mfs_addr = my_addr;
		mfs->mfs_dir  = strdup("/");
		if (mfs->mfs_dir == NULL)
			goto alloc_failed;
		mfs->mfs_subdir  = strdup("");
		if (mfs->mfs_subdir == NULL)
			goto alloc_failed;
		return (ms);
	}

	/*
	 * If portmap or mountd just timed out on us, additional NFS requests
	 * have piled up on our socket.  Therefore, we cache such failures to
	 * dispense with subsequent calls quickly.  (Pingmount is caching
	 * nfsd timeouts.)
	 */
	if (deadtime > time_now &&
	    ((struct in_addr *)hp->h_addr)->s_addr == deadhost.s_addr)
		return ((struct mapent *)NULL);

	/* ping the null procedure of the server's nfs */
	if (pingmount(*(struct in_addr *)hp->h_addr) != RPC_SUCCESS)
		return ((struct mapent *)NULL);

	(void) bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
	sin.sin_family = AF_INET;
        sin.sin_port=0;
        s = RPC_ANYSOCK;
        /*
         * First try tcp, then drop back to udp if
         * tcp is unavailable (an old version of mountd perhaps)
         * Using tcp is preferred because it can handle
         * arbitrarily long export lists.
         */
        cl = clnttcp_create(&sin, MOUNTPROG, MOUNTVERS, &s, 0, 0);
	/* get export list of host */
	if (cl == NULL) {
        	clnt_stat = callrpc(host, MOUNTPROG, MOUNTVERS, 
		MOUNTPROC_EXPORT, xdr_void, 0, xdr_exports, &ex);
	} else {
		clnt_stat = (enum clnt_stat) clnt_call(cl, MOUNTPROC_EXPORT, xdr_void, 0, xdr_exports, &ex, TIMEOUT);
	}

	if (clnt_stat != RPC_SUCCESS) {
		syslog(LOG_ERR, "%s: exports: %s",host,clnt_sperrno(clnt_stat));
		        if (cl != NULL) {
                		(void) close(s);
                		clnt_destroy(cl);
        		}
		deadhost = *(struct in_addr *)hp->h_addr;
		deadtime = time_now + cache_time;
		return ((struct mapent *)NULL);
	}
	if (ex == NULL)
		return ((struct mapent *)NULL);

	/* now sort by length of names - to get mount order right */
	exlist = ex;
	texlist = NULL;
	for (ex = exlist; ex; ex = exnext) {
		exnext = ex->ex_next;
		ex->ex_next = 0;
		elen = strlen(ex->ex_name);

		/*  check netgroup list  */
		if (ex->ex_groups) {
			for ( gr = ex->ex_groups ; gr ; gr = gr->g_next) {
				if (local_name(gr->g_name) ||
				    in_netgroup(gr->g_name, self, mydomain))
					break;
			}
			if (gr == NULL) {
				freeex(ex);
				continue;
			}
		}

		for (texp = &texlist; *texp; texp = &((*texp)->ex_next))
			if (elen < strlen((*texp)->ex_name))
				break;
		ex->ex_next = *texp;
		*texp = ex;
	}
	exlist = texlist;

	/* Now create a mapent from the export list */
	ms = NULL;
	me = NULL;
	for (ex = exlist; ex; ex = ex->ex_next) {
		mp = me;
		me = (struct mapent *)malloc(sizeof *me);
		if (me == NULL)
			goto alloc_failed;
		bzero((char *) me, sizeof *me);

		if (ms == NULL)
			ms = me;
		else
			mp->map_next = me;

		(void) strcpy(name, "/");
		(void) strcat(name, host);
		me->map_root = strdup(name);
		if (me->map_root == NULL)
			goto alloc_failed;
		if (strcmp(ex->ex_name, "/") == 0)
			me->map_mntpnt = strdup("");
		else
			me->map_mntpnt = strdup(ex->ex_name);
		if (me->map_mntpnt == NULL)
			goto alloc_failed;
		me->map_mntopts = strdup(mapopts);
		if (me->map_mntopts == NULL)
			goto alloc_failed;
		mfs = (struct mapfs *)malloc(sizeof *mfs);
		if (mfs == NULL)
			goto alloc_failed;
		bzero((char *) mfs, sizeof *mfs);
		me->map_fs = mfs;
		mfs->mfs_host = strdup(host);
		if (mfs->mfs_host == NULL)
			goto alloc_failed;
		mfs->mfs_addr = *(struct in_addr *)hp->h_addr;
		mfs->mfs_dir  = strdup(ex->ex_name);
		if (mfs->mfs_dir == NULL)
			goto alloc_failed;
		mfs->mfs_subdir = strdup("");
		if (mfs->mfs_subdir == NULL)
			goto alloc_failed;
	}
	freeex(exlist);
	if (cl != NULL) {
		(void) close(s);
		clnt_destroy(cl);
	}
	return (ms);

alloc_failed:
	syslog(LOG_ERR, "Memory allocation failed: %m");
	free_mapent(ms);
	freeex(exlist);
	if (cl != NULL) {
		(void) close(s);
		clnt_destroy(cl);
	}
	return ((struct mapent *)NULL);
}

#define MAXGRPLIST 512
/*
 * Check cached netgroup info before calling innetgr().
 * Two lists are maintained here:
 * membership groups and non-membership groups.
 * This returns 0 if we are not in the group, 1 if we are.
 *
 * Warning: Bizarre code ahead.
 * 
 * 1) If yp isn't running, one could build a pretty good case for
 *    returning 0.  After all, if you want to use netgroups, you need yp.
 *
 * 2) Why not just believe netgroup.byhost and be done with it?  All
 *    this nonsense of adding to grplist and maintaining nogrplist is
 *    bizarre, especially since nogrplist can grow into a list of all
 *    workstation names on the net!  The number of calls to strncmp becomes
 *    pretty high, escpecially if we're called by touching a big server
 *    that exports home directories to a flock of workstations.
 *
 * I would have done the above if we weren't so close to release.
 */
int
in_netgroup(group, hostname, domain)
	char *group, *hostname, *domain;
{
	static char    grplist[MAXGRPLIST+1];
	static char *nogrplist;
	static int nogrplen;	/* Bytes allocated to nogrplist */
	char key[256];
	char *ypline = NULL;
	int yplen;
	extern int yp;
	register char *gr, *p;
	static time_t last;
	static int cache_time = 300; /* 5 min */

	if (!yp)
		return (1);

	if (time_now > last + cache_time) {
		if (!nogrplen)
			if (nogrplist = (char *) malloc(MAXGRPLIST))
				nogrplen = MAXGRPLIST;
			else
				return 1;
		last = time_now;
		grplist[0]   = '\0';
		nogrplist[0] = '\0';
		(void) strcpy(key, hostname);
		(void) strcat(key, ".");
		(void) strcat(key, domain);
		if (yp_match(domain, "netgroup.byhost", key,
		    strlen(key), &ypline, &yplen) == 0) {
			(void) strncpy(grplist, ypline, MIN(yplen, MAXGRPLIST));
			grplist[MIN(yplen, MAXGRPLIST)] = '\0';
			FREE(ypline);
		}
	}

	for (gr = grplist ; *gr ; gr = p ) {
		for (p = gr ; *p && *p != ',' ; p++)
			;
		if (strncmp(group, gr, p - gr) == 0)
			return (1);
		if (*p == ',')
			p++;
	}
	for (gr = nogrplist ; *gr ; gr = p ) {
		for (p = gr ; *p && *p != ',' ; p++)
			;
		if (strncmp(group, gr, p - gr) == 0)
			return (0);
		if (*p == ',')
			p++;
	}

	if (innetgr(group, hostname, (char *)NULL, domain)) {
		if (strlen(grplist)+1+strlen(group) > MAXGRPLIST)
			return (1);
		if (*grplist)
			(void) strcat(grplist, ",");
		(void) strcat(grplist, group);
		return (1);
	} else {
		if (strlen(nogrplist)+1+strlen(group) > nogrplen) {
			p = (char *) realloc(nogrplist,
					     nogrplen + strlen(group) + MAXGRPLIST);
			if (!p)
				return (0); /* No memory! */
			nogrplist = p;
			nogrplen += strlen(group) + MAXGRPLIST;
		}
		if (*nogrplist)
			(void) strcat(nogrplist, ",");
		(void) strcat(nogrplist, group);
		return (0);
	}
}

/*
 * The following perhaps should migrate to a new file, but for now
 * they're happy here.  For the time being, we'll keep the includes and
 * storage definitions we need here.
 */
#include <sys/ioctl.h>
#include <net/if.h>

#define IFREQCNT 64   		/* need enough room for total number of
				 * possible interfaces, including aliases for
				 * LAT and DECnet */
#define LOCAL_ADDRS 32		/* More than enough (I hope) local IP addrs */
#define LOCAL_ALIASES 64	/* Ditto, local names and aliases */

struct in_addr addr_table[LOCAL_ADDRS],	/* Local IP addresses */
	*addr_end = addr_table;		/* Last entry used + 1 */
struct nameent {
	int name_len;
	char *name_name;
} name_table[LOCAL_ALIASES], *name_end = name_table;

#define ADD_NAME(name) { 			\
	name_end->name_name = strdup(name);	\
	name_end->name_len = strlen(name);	\
	name_end++;				\
}

void local_add_hp();

/*
 * Routine to collect all our IP addresses and names.
 *
 * This looks at all configured network interfaces, records the IP address,
 * then records the name and aliases associated with that IP address.
 */
int local_init() {
        int s;
        struct ifreq *ifr;
        struct ifreq ifreqs[IFREQCNT];
        struct ifconf ifc;
	struct sockaddr_in *sin;
	struct hostent *hp;

	if (gethostname(self, MAXHOSTNAMELEN)) {
		syslog(LOG_ERR, "Can't get my node name");
		exit(1);
	}
	if (!(hp = gethostbyname(self))) {
		syslog(LOG_ERR, "Can't get my address");
		exit(1);
	}
	local_add_hp(hp);
	my_addr = addr_table[0]; /* Save address copied by local_add_hp() */

        if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                syslog(LOG_ERR, "local_init socket: %m");
                return(-1);
        }
	ifc.ifc_req = ifreqs;
	ifc.ifc_len = sizeof(ifreqs);
	if (ioctl(s, SIOCGIFCONF, &ifc) < 0) {
		syslog(LOG_ERR, "local_init SIOCGIFCONF ioctl:");
		close(s);
		return(-1);
	}
	close(s);
	for (ifr = ifreqs;
	     ifr < &ifreqs[ifc.ifc_len / sizeof(struct ifreq)]; ifr++) {
		if (ifr->ifr_addr.sa_family != AF_INET)
			continue;
		sin = (struct sockaddr_in *) &ifr->ifr_addr;
		if (addr_end < &addr_table[LOCAL_ADDRS] &&
		    !local_addr(sin->sin_addr))
			*addr_end++ = sin->sin_addr; /* Save our addresses */
		if (hp = gethostbyaddr((char *) &sin->sin_addr,
					 sizeof(sin->sin_addr), AF_INET))
			local_add_hp(hp);
		else
			syslog(LOG_ERR, "gethostbyaddr failed, h_errno %d\n",
			       h_errno);
	}
#ifdef notdef
	/* Can't test trace, cause we're called before options are parsed! */
	{
		struct nameent *np;
		struct in_addr *iap;

		fprintf(stderr,"local aliases:");
		for (np = name_table; np < name_end; np++)
			fprintf(stderr, " %s", np->name_name);
		fprintf(stderr,"\nlocal addresses:");
		for (iap = addr_table; iap < addr_end; iap++)
			fprintf(stderr, " %s", inet_ntoa(*iap));
		fprintf(stderr, "\n");
	}
#endif
}

void local_add_hp(hp)
	struct hostent *hp;
{
	struct in_addr **iap;
	char **cpp;

	if (hp->h_name && name_end < &name_table[LOCAL_ALIASES] &&
	    !local_name(hp->h_name))
		ADD_NAME(hp->h_name);
	for (cpp = hp->h_aliases;
	     *cpp && name_end < &name_table[LOCAL_ALIASES];
	     cpp++)
		if (!local_name(*cpp))
			ADD_NAME(*cpp);
	for (iap = (struct in_addr **)hp->h_addr_list; *iap; iap++)
		if (addr_end < &addr_table[LOCAL_ADDRS] &&
		    !local_addr(**iap))
			*addr_end++ = **iap; /* Save our addresses */
}

/*
 * Routine that says if name is our system.
 */
int local_name(name)
	char *name;
{
	struct nameent *np;
	int len = strlen(name);

	for (np = name_table; np < name_end; np++)
		if (np->name_len == len && strcasecmp(np->name_name, name) == 0)
			return TRUE;
	return FALSE;
}

/*
 * Routine that says if ia is one of our interfaces.
 */
int local_addr(ia)
	struct in_addr ia;
{
	struct in_addr *iap;

	for (iap = addr_table; iap < addr_end; iap++)
		if (iap->s_addr == ia.s_addr)
			return TRUE;
	return FALSE;
}
