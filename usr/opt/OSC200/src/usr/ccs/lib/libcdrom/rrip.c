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
static char rcsid[] = "@(#)$RCSfile: rrip.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/07/16 13:08:01 $";
#endif

#include <sys/cdrom.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/param.h>			/* MIN, MAX */
#include <fcntl.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

extern int errno;

static int
getfsfd(const char *path)
{
    /* statfs the file, then open the mountpoint read only */
    struct statfs stfs;
    int ret;

    ret = statfs(path, &stfs, sizeof(stfs));
    if (ret == -1)
	return ret;
    return open(stfs.f_mntonname, O_RDONLY);
}

static void
mkdirname(const char *path, char *openpath)
{
    char *cp;
    cp = strrchr(path, '/');
    if (!cp)
	strcpy(openpath, ".");
    else {
	int count;
	if (cp == path) {
	    openpath[0] = '/';
	    openpath[1] = '\0';
	} else {
	    count = MIN(NAME_MAX-1, cp-path);
	    strncpy(openpath, path, count); /* copy slash; we can open foo/bar/ */
	    openpath[count] = '\0';
	}
    }
}

int
cd_setdevmap(char *path,
	     int cmd,
	     int *new_major,
	     int *new_minor) /*  RRIP 5.4.2 */
{
    /*
     * to set the map, we open up path's parent directory for read-only access,
     * then run ioctl's on it.
     */
    int fd;
    int ret, saverr;
    struct rrip_map_arg marg;

    fd = getfsfd(path);
    if (fd == -1)
	return -1;

    switch (cmd) {
    case CD_SETDMAP:
	/* set a device map */
	marg.major = *new_major;
	marg.minor = *new_minor;
	marg.path = path;
	ret = ioctl(fd, CDIOCSETDMAP, &marg);
	if (ret == -1) {
	    if (errno == E2BIG) {
		errno = 0;
		ret = 0;			/* no more space */
	    }
	} else
	    ret = 1;			/* set OK */
	break;

    case CD_UNSETDMAP:
	/* unset a device map */
	marg.path = path;
	ret = ioctl(fd, CDIOCUNSETDMAP, &marg);
	if (ret == -1) {
	    if (errno == ESRCH) {
		errno = 0;		/* not found */
		ret = 0;
	    }
	} else { 
	    *new_major = marg.major;
	    *new_minor = marg.minor;
	    ret = 1;			/* unset OK */
	}
	break;
    default:
	errno = EINVAL;
	ret = -1;
	break;
    }
    saverr = errno;
    (void) close(fd);
    errno = saverr;
    if (ret == -1) {
	switch(errno) {
	case ESRCH:
	    /* no more entries */
	    errno = 0;
	    return 0;
	case ENOTTY:
	    /* not a CDFS file */
	    errno = EINVAL;
	    /* fall through: */
	}
    }
    return ret;
}

int
cd_getdevmap(char *path,
	     int pathlen,
	     int index,
	     int *new_major,
	     int *new_minor) /*  RRIP 5.4.3 */
{
    int fd;
    int ret, saverr;

    fd = getfsfd(path);
    if (fd == -1)
	return -1;

    if (index == 0) {
	struct rrip_map_arg maparg;
	/* get the mappings for file "path" */
	maparg.path = path;
	ret = ioctl(fd, CDIOCGETDMAP, &maparg);
	if (ret != -1) {
	    *new_major = maparg.major;
	    *new_minor = maparg.minor;
	}
    } else {
	struct rrip_map_idx_arg idxarg;
	/* get mappings for nth file */
	idxarg.index = index;
	idxarg.path = path;
	idxarg.pathlen = pathlen;
	ret = ioctl(fd, CDIOCGETDMAPIDX, &idxarg);
	if (ret != -1) {
	    *new_major = idxarg.major;
	    *new_minor = idxarg.minor;
	    ret = 1;			/* found it OK */
	}
    }
    saverr = errno;
    (void) close(fd);
    errno = saverr;
    if (ret == -1) {
	switch(errno) {
	case ESRCH:
	    /* no more entries */
	    errno = 0;
	    return 0;
	case ENOTTY:
	    /* not a CDFS file */
	    errno = EINVAL;
	    /* fall through: */
	}
    }
    return ret;
}

int
cd_suf(char *path,
       int fsec,
       char signature[2],
       int index,
       char *buf, int buflen) /* RRIP 5.4.1 */
{
    int fd, ret, saverr;
    struct rrip_suf_arg sufarg;
    struct stat stb;
    char openpath[NAME_MAX];

    if (lstat(path, &stb) == -1)
        return -1;

    if (S_ISLNK(stb.st_mode)) {
        /* if a symlink, we don't want to open it, but just look at it, so
           we open the parent. */
        mkdirname(path, openpath);

        fd = getfsfd(openpath);
    } else
        fd = getfsfd(path);

    if (fd == -1)
	return fd;

    sufarg.fsec = fsec;
    if (signature) {
	sufarg.sig[0] = signature[0];
	sufarg.sig[1] = signature[1];
	sufarg.sigok = 1;
    } else
	sufarg.sigok = 0;
    sufarg.buf = buf;
    sufarg.buflen = buflen;
    sufarg.sig_index = index;
    sufarg.path = path;
    ret = ioctl(fd, CDIOCGETSUF, &sufarg);

    saverr = errno;
    (void) close(fd);
    errno = saverr;
    if (ret == -1) {
	switch(errno) {
	case ESRCH:
	    /* no more entries */
	    errno = 0;
	    return 0;
	case ENOTTY:
	    /* not a CDFS file */
	    errno = EINVAL;
	    /* fall through: */
	default:
	    return -1;
	}
    }
    return sufarg.buflen;
}
