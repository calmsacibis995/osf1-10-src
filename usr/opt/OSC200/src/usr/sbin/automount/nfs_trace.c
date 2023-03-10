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
static char     *sccsid = "@(#)$RCSfile: nfs_trace.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/10/11 17:32:50 $";
#endif /* ultrix */
#endif /* lint */

/*
 * Copyright (c) 1987 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <sys/types.h>
#include <rpc/types.h>
#include "nfs_prot.h"

p_void()
{
}

struct statent {
	nfsstat val;
	char *str;
} stattab[] = {
#define doit(XXX)	{ XXX, "XXX" },
	{ NFS_OK, "OK" },
	{ NFSERR_PERM, "PERM" },
	{ NFSERR_NOENT, "NOENT" },
	{ NFSERR_IO, "IO" },
	{ NFSERR_NXIO, "NXIO" },
	{ NFSERR_ACCES, "ACCES" },
	{ NFSERR_EXIST, "EXIST" },
	{ NFSERR_NODEV, "NODEV" },
	{ NFSERR_NOTDIR, "NOTDIR" },
	{ NFSERR_ISDIR, "ISDIR" },
	{ NFSERR_FBIG, "FBIG" },
	{ NFSERR_NOSPC, "NOSPC" },
	{ NFSERR_ROFS, "ROFS" },
	{ NFSERR_NAMETOOLONG, "NAMETOOLONG" },
	{ NFSERR_NOTEMPTY, "NOTEMPTY" },
	{ NFSERR_DQUOT, "DQUOT" },
	{ NFSERR_STALE, "STALE" },
	{ NFSERR_WFLUSH, "WFLUSH" }
#undef doit
};
#define STATTABSIZE (sizeof(stattab)/sizeof(struct statent))


p_nfsstat(status)
	nfsstat *status;
{
	int i;

	for (i = 0; i < STATTABSIZE; i++) {
		if (stattab[i].val == *status) {
			fprintf(stderr, "stat=%s", stattab[i].str);
			return;
		}
	}
	fprintf(stderr, "stat=%d", *status);
}

p_fhandle(fh)
	nfs_fh *fh;
{
	register int *ip = (int *)fh;

	fprintf(stderr, "fh=[%d, %d, %d](%s)", ip[0], ip[1], ip[2], fhtocp(fh));
}

p_diropargs(d)
	diropargs *d;
{
	p_fhandle(&d->dir);
	fprintf(stderr, ", name=%s", d->name);
}

p_nfstime(t)
	nfstime *t;
{
	char *s;
	char *ctime();
	
	s = ctime((int *)&t->seconds);
	s[strlen(s) - 1] = 0;
	fprintf(stderr, "%s", s);
}

p_fattr(f)
	fattr *f;
{
	fprintf(stderr, "type=%u, mode=%o, nlink=%u, uid=%u, gid=%u,\
size=%u, blocksize=%u, rdev=%u, blocks=%u, fsid=%u, fileid=%u",
		f->type, f->mode, f->nlink, f->uid, f->gid, f->size, 
		f->blocksize, f->rdev, f->blocks, f->fsid, f->fileid);
	fprintf(stderr, ", atime=");
	p_nfstime(&f->atime);
	fprintf(stderr, ", mtime=");
	p_nfstime(&f->mtime);
	fprintf(stderr, ", ctime=");
	p_nfstime(&f->ctime);
}

p_sattr(s)
	sattr *s;
{
	fprintf(stderr, "mode=%o, uid=%u, gid=%u, size=%u", 
		s->mode, s->uid, s->gid, s->size);
	fprintf(stderr, ", atime=");
	p_nfstime(&s->atime);
	fprintf(stderr, ", mtime=");
	p_nfstime(&s->mtime);
}

p_diropres(d)
	diropres *d;
{
	p_nfsstat(&d->status);
	if (d->status == NFS_OK) {
		fprintf(stderr, ", ");
		p_fhandle(&d->diropres_u.diropres.file);
		fprintf(stderr, ", ");
		p_fattr(&d->diropres_u.diropres.attributes);
	}
}

p_sattrargs(sa)
	sattrargs *sa;
{
	p_fhandle(&sa->file);
	fprintf(stderr, ", ");
	p_sattr(&sa->attributes);
}

p_attrstat(as)
	attrstat *as;
{
	p_nfsstat(&as->status);
	if (as->status == NFS_OK) {
		fprintf(stderr, ", ");
		p_fattr(&as->attrstat_u.attributes);
	}
}

p_readlinkres(r)
	readlinkres *r;
{
	p_nfsstat(&r->status);
	if (r->status == NFS_OK) {
		fprintf(stderr, ", data=%s", r->readlinkres_u.data);
	}
}

p_readargs(r)
	readargs *r;
{
	p_fhandle(&r->file);
	fprintf(stderr, ", offset=%u, count=%u", r->offset, r->count);
}

p_readres(r)
	readres *r;
{
	p_nfsstat(&r->status);
	if (r->status == NFS_OK) {
		fprintf(stderr, ", ");
		p_fattr(&r->readres_u.reply.attributes);
		fprintf(stderr, ", len=%u, data=(data)",
			r->readres_u.reply.data.data_len);
	}
}

p_writeargs(w)
	writeargs *w;
{
	p_fhandle(&w->file);
	fprintf(stderr, ", offset=%u, len=%u, data=(data)", 
		w->offset, w->data.data_len);
}

p_createargs(c)
	createargs *c;
{
	p_diropargs(&c->where);
	fprintf(stderr, ", ");
	p_sattr(&c->attributes);
}

p_renameargs(r)
	renameargs *r;
{
	p_diropargs(&r->from); 
	fprintf(stderr, ", ");
	p_diropargs(&r->to);
}

p_linkargs(args)
	linkargs *args;
{
	p_fhandle(&args->from);
	fprintf(stderr, ", ");
	p_diropargs(&args->to);
}

p_symlinkargs(args)
	symlinkargs *args;
{
	p_diropargs(&args->from);
	fprintf(stderr, ", to=%s, ", args->to);
	p_sattr(&args->attributes);
}

p_statfsres(res)
	statfsres *res;
{
	p_nfsstat(&res->status);
	if (res->status == NFS_OK) {
		fprintf(stderr, ", tsize=%d, bsize=%d, blocks=%d, bfree=%d, bavail=%d",
			res->statfsres_u.reply.tsize, 
			res->statfsres_u.reply.bsize, 
			res->statfsres_u.reply.blocks,
			res->statfsres_u.reply.bfree, 
			res->statfsres_u.reply.bavail);
	}
}

p_readdirargs(args)
	readdirargs *args;
{
	p_fhandle(&args->dir);
	fprintf(stderr, ", cookie=%d, count=%d",
		*(int *)args->cookie, args->count);
}

p_entryp(p)
	entry *p;
{
	while (p != NULL) {
		fprintf(stderr, "(fileid=%u, name=%s, cookie=%u), ", 
			p->fileid, p->name, *(int*)p->cookie);
		p = p->nextentry;
	}
}

p_readdirres(res)
	readdirres *res;
{
	p_nfsstat(&res->status);
	if (res->status == NFS_OK) {
		p_entryp(res->readdirres_u.reply.entries);
		fprintf(stderr, ", eof=%d", res->readdirres_u.reply.eof);
	}
}
	
struct procinfo {
	char *name;
	int (*pargs)();
	int (*pres)();
} procs[] = {
    { "NULL",		p_void,		p_void		},
    { "GETATTR",	p_fhandle,	p_attrstat	},
    { "SETATTR",	p_sattrargs, 	p_attrstat	},
    { "ROOT",		p_void,		p_void		},
    { "LOOKUP",		p_diropargs,	p_diropres	},
    { "READLINK",	p_fhandle,	p_readlinkres	},
    { "READ",		p_readargs,	p_readres	},
    { "WRITECACHE",	p_void,		p_void		},
    { "WRITE",		p_writeargs,	p_attrstat	},
    { "CREATE",		p_createargs,	p_diropres	},
    { "REMOVE",		p_diropargs,	p_nfsstat	},
    { "RENAME",		p_renameargs,	p_nfsstat	},
    { "LINK",		p_linkargs,	p_nfsstat	},
    { "SYMLINK",	p_symlinkargs,	p_nfsstat	},
    { "MKDIR",		p_createargs,	p_nfsstat	},
    { "RMDIR",		p_diropargs,	p_nfsstat	},
    { "READDIR",	p_readdirargs,	p_readdirres	},
    { "STATFS",		p_fhandle,	p_statfsres	},
};


trace_call(procnum, args)
	int procnum;
	char *args;
{
	fprintf(stderr, "%s call(", procs[procnum].name);
	(*procs[procnum].pargs)(args);
	fprintf(stderr, ")\n");
}

trace_return(procnum, res)
	int procnum;
	char *res;
{
	fprintf(stderr, "%s return(", procs[procnum].name);
	(*procs[procnum].pres)(res);
	fprintf(stderr, ")\n");
}
