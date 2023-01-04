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
static char	*sccsid = "@(#)$RCSfile: ldr_sys_int.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/12/07 16:20:28 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 *	loader system interface routines
 *
 * OSF/1 Release 1.0
 */

#include <errno.h>
#include <sys/signal.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <loader.h>

#include "ldr_errno.h"
#include "ldr_types.h"
#include "ldr_malloc.h"
#include "ldr_sys_int.h"
#include "ldr_windows.h"


/*
 *	File I/O routines
 */

ldr_file_t 
ldr_open(const char *path, int flags)
{
	int rc;

	rc = open((char *)path, flags);
	return (ldr_file_t)((rc == -1) ? ldr_errno_to_status(errno) : rc);
}

int 
ldr_close(ldr_file_t fhandle)
{
	int rc, rrc;

	rc = close((int) fhandle);
	rrc = ldr_flush_mappings(fhandle);
	if (rc == LDR_SUCCESS) rc = rrc;

	return ((rc == -1) ? ldr_errno_to_status(errno) : rc);
}

int
ldr_read(ldr_file_t fhandle, char *buf, unsigned nbytes)
{
	int rc;

	rc = read((int) fhandle, buf, nbytes);
	return ((rc == -1) ? ldr_errno_to_status(errno) : rc);
}

int
ldr_write(ldr_file_t fhandle, char *buf, unsigned nbytes)
{
	int rc;

	rc = write((int) fhandle, buf, nbytes);
	return ((rc == -1) ? ldr_errno_to_status(errno) : rc);
}

int
ldr_stat(const char *path, struct stat *buf)
{
	int rc;

	rc = stat((char *)path, buf);
	return ((rc == -1) ? ldr_errno_to_status(errno) : rc);
}

int 
ldr_fstat(ldr_file_t fhandle, struct stat *buf)
{
	int rc;

	rc = fstat((int) fhandle, buf);
	return ((rc == -1) ? ldr_errno_to_status(errno) : rc);
}

int
ldr_lseek(ldr_file_t fhandle, off_t offset, int whence)
{
	int rc;

	rc = lseek((int) fhandle, offset, whence);
	return ((rc == -1) ? ldr_errno_to_status(errno) : rc);
}

int 
ldr_ftruncate(ldr_file_t fhandle, off_t length)
{
	int rc;

	rc = ftruncate((int) fhandle, length);
	return ((rc == -1) ? ldr_errno_to_status(errno) : rc);
}

int
ldr_unlink(const char *path)
{
	int rc;

	rc = unlink(path);
	return ((rc == -1) ? ldr_errno_to_status(errno) : rc);
}

int
ldr_grow_file(ldr_file_t fd, off_t new_size)

/* Grow the specified file, if required, to be at least the specified size.
 */
{
	struct stat	stat_buf;	/* for file size */
	int		rc;

	if ((rc = ldr_fstat(fd, &stat_buf)) != LDR_SUCCESS)
		return(rc);
	if (new_size > stat_buf.st_size) {
#ifdef NOTDEF
		/* Would like to do it this way, but it doesn't work */
		if ((rc = ldr_ftruncate(fd, new_size)) != LDR_SUCCESS)
			return(rc);
#else
		off_t	old_pos;
		char	ch = '\0';

		old_pos = ldr_ltell(fd);
		ldr_lseek(fd, new_size - 1, LDR_L_SET);
		rc = (ldr_write(fd, &ch, 1) == 1 ? LDR_SUCCESS : ldr_errno_to_status(errno));
		ldr_lseek(fd, old_pos, LDR_L_SET);
#endif
	}
	return(rc);
}


/*
 *	Mapping files and anonymous regions
 */

int 
ldr_mmap(univ_t addr, size_t len, int prot, int flags,
		    ldr_file_t fhandle, off_t off, univ_t *mapped_addr)
{
	caddr_t rc;

	rc = mmap((caddr_t) addr, len, prot, flags, (int) fhandle, off);
	if (rc == (caddr_t)(-1)) return ldr_errno_to_status(errno);
	else {
		*mapped_addr = (univ_t) rc;
		return LDR_SUCCESS;
	}
}

int 
ldr_munmap(univ_t addr, size_t len)
{
	int rc;

	rc = munmap((caddr_t) addr, len);
	return ((rc == -1) ? ldr_errno_to_status(errno) : rc);
}

int 
ldr_msync(univ_t addr, size_t len, int flags)
{
#ifdef NOTYET
	int rc;

	rc = msync((caddr_t) addr, len, flags);
	return ((rc == -1) ? ldr_errno_to_status(errno) : rc);
#endif
}

int 
ldr_mprotect(univ_t addr, size_t len, int prot)
{
	int rc;

	rc = mprotect((caddr_t) addr, len, prot);
	return ((rc == -1) ? ldr_errno_to_status(errno) : rc);
}

int 
ldr_mvalid(univ_t addr, size_t len, int prot)
{
	int rc;

	rc = mvalid((caddr_t) addr, len, prot);
	return ((rc == -1) ? ldr_errno_to_status(errno) : rc);
}


char *
ldr_strdup(const char *str)

/* Duplicate the specified string into ldr_malloc'ed storage and return
 * the new storage.  Return NULL on error.
 */
{
	char		*newstr;
	int		rc;

	if ((rc = ldr_malloc(strlen(str)+1, LDR_STRING_T,
			     (univ_t *)&newstr)) != LDR_SUCCESS)
		return(NULL);
	(void)strcpy(newstr, str);
	return(newstr);
}


char *
ldr_heap_strdup(ldr_heap_t heap, const char *str)

/* Duplicate the specified string into ldr_heap_malloc'ed storage allocated
 * from the specified heap, and return the new storage.  Return NULL on error.
 */
{
	char		*newstr;
	int		rc;

	if ((rc = ldr_heap_malloc(heap, strlen(str)+1, LDR_STRING_T,
			     (univ_t *)&newstr)) != LDR_SUCCESS)
		return(NULL);
	(void)strcpy(newstr, str);
	return(newstr);
}


int
ldr_getaddressconf(struct addressconf **conf)

/* Get the address configuration record from the kernel.  Return a pointer to
 * the (static) address configuration record.  Returns LDR_SUCCESS on success
 * or negative error status on error.
 */
{
	int		sz;
	static int initialized = 0;	/* true iff addr conf has been read */
	static struct addressconf addr_conf[AC_N_AREAS]; /* the address conf record */

	if (initialized) {
		*conf = addr_conf;
		return(LDR_SUCCESS);
	}

	if ((sz = getaddressconf(addr_conf, sizeof(addr_conf))) < 0)
		return(ldr_errno_to_status(errno));
	if (sz < sizeof(addr_conf))	/* shouldn't happen */
		return(LDR_EINVAL);
	
	initialized = 1;
	*conf = addr_conf;
	return(LDR_SUCCESS);
}


int
ldr_maketemp(const char *loc, int mode, ldr_file_t *pfd, char **pfname)

/* Make a temporary file name, and create it.  Arguments are: pathname of
 * a file (or directory, must end in '/') in which to create the temp
 * file (may be NULL), and protection mode for new file.  Returns the
 * pathname of the temp file (in ldr_strdup'ed storage), and the open
 * file descriptor on the temp file.
 *
 * Constructed file name is of the form "ldrPPPPPPPP.X", where the
 * PPPPPPPP is the process ID in hex, and the .X is a "uniqueizer".
 */
{
	const char		*defdir = "/tmp/";
	const int		deflen = 5; /* strlen(defdir) */
	const char		*pattern = "ldr%08x.%1x";
	const int		psize = 14; /* size of pattern */
	const int		maxtry = 16;

	const char		*endloc;
	const char		*p;
	char			*newname;
	int			try;
	ldr_file_t		fd;
	int			rc;

	if (loc == NULL) {		/* no location specified */
		loc = (char *)defdir;
		endloc = &defdir[deflen];
	} else {
		for (p = loc, endloc = NULL; *p != '\0'; p++)
			if (*p == '/')
				endloc = p + 1;
		if (endloc == NULL)	/* HUH? */
			return(LDR_EINVAL); /* invalid location */
	}

	if ((rc = ldr_malloc((endloc - loc) + psize + 1, LDR_STRING_T, (univ_t *)&newname)) != LDR_SUCCESS)
		return(rc);

	bcopy(loc, newname, (endloc - loc));

	for (try = 0; try < maxtry; try++) {

		ldr_sprintf(&newname[(endloc - loc)], psize, pattern, getpid(),
			    try);
		if ((fd = open(newname, O_RDWR|O_CREAT|O_TRUNC|O_EXCL,
				   mode)) >= 0)
			break;
	}

	if (fd < 0) {
		(void)ldr_free(newname);
		return(ldr_errno_to_status(errno));
	}

	*pfd = fd;
	*pfname = newname;
	return(LDR_SUCCESS);
}


/* TEMP */
void
abort(void)
{
	ldr_abort();
}


void
ldr_abort(void)

/* Abort the process.  Don't bother with a core dump -- it won't
 * be useful anyway.
 */
{
	(void)kill(getpid(), SIGKILL);
}


void
ldr_bpt(void)

/* Simulate a breakpoint.  Used to return control to the debugger
 * after we complete loading a program, so it can debug it.
 */
{
	(void)kill(getpid(), SIGTRAP);
}
