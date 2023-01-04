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
static char *rcsid = "@(#)$RCSfile: glob.c,v $ $Revision: 1.1.6.3 $ (DEC) $Date: 1993/06/25 21:47:50 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

/*
 * COMPONENT_NAME: (LIBCPAT) Standard C Library Pattern Functions 
 *
 * FUNCTIONS: glob, globfree
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak glob = __glob
#pragma weak globfree = __globfree
#endif
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <fnmatch.h>
#include <glob.h>
#include <limits.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/************************************************************************/
/* Internal definitions							*/
/************************************************************************/

#define	ADRBUF_SIZE   100	/* # of gpat packets/allocation		*/
#define TEXTBUF_SIZE  2000	/* # of gtext bytes/buffer		*/
#define NOSLASH       0		/* do not add trailing '/' to pathname	*/

struct	gadr			/* pathname address buffer		*/
	{
	int	ga_max;		/* maximum # of entries in buffer	*/
	int	ga_used;	/* # of entries used			*/
	char	*ga_adr[1];	/* addresses of pathnames 		*/
	};

struct	gtext			/* pathname text buffer			*/
	{
 struct	gtext	*gt_next;	/* ptr to next text buffer		*/
	int	gt_used;	/* # of bytes used			*/
	char	gt_text[TEXTBUF_SIZE]; /* pathname text			*/
	};

/************************************************************************/
/* Internal function prototypes						*/
/************************************************************************/

static int
add_pname(char *, const char *, glob_t *, int, int *);

static int
check_err(int (*)(), int, char *, char *, int);

static int
globnames(char *, int, int (*)(), glob_t *, char *, int *);

static char *
leading_dir(char *);

static int
path_compare(char **, char **);

static int
wild_check(char *, int);

#ifdef _NAME_SPACE_WEAK_STRONG
#ifdef stat
#undef stat
#endif
#endif
/************************************************************************/
/* glob - generate pathnames matching a pattern				*/
/************************************************************************/
int
glob(const char *pattern, int flags, int (*errfunc)(), glob_t *pglob)
{
	int	first_addr;	/* gl_pathv location of first pathname	*/
	int	init_addr;	/* initial # of pathname addresses	*/
	int	new_count;	/* # of new pathnames added to gl_pathv	*/
 struct gadr	*padr;		/* ptr to pathname address array	*/
	char	*ppat;		/* ptr to read/write pattern		*/
 struct gtext	*ptx;		/* ptr to pathname text			*/
	int	stat;	/* return status			*/
	char	cwd[PATH_MAX+1];/* initial current working directory	*/
	char	dirpath[PATH_MAX+1]; /* traversed pathname		*/

/*
 * if not appending to previous data
 *   initialize glob structure
 *   allocate new pathname text buffer
 *   allocate new pathname address buffer, making sure it is large enough
 *     to hold the reserved addresses
 *   null fill reserved addresses
 */
	if ((flags & GLOB_APPEND) == 0)
		{
		pglob->gl_pathc = 0;
		pglob->gl_pathv = NULL;
		pglob->gl_padr = NULL;
		pglob->gl_ptx = NULL;

		ptx = (struct gtext *)malloc(sizeof(struct gtext));
		if (ptx == NULL)
			return (GLOB_NOSPACE);
		ptx->gt_next = NULL;
		ptx->gt_used = 0;

		if ((flags & GLOB_DOOFFS) != 0)
			first_addr = pglob->gl_offs;
		else
			first_addr = 0;
		init_addr = ADRBUF_SIZE;
		while (init_addr < first_addr)
			init_addr += ADRBUF_SIZE;
		padr = (struct gadr *)malloc(sizeof(int) * 2 + sizeof(char *) * init_addr);
		if (padr == NULL)
			{
			free(ptx);
			return (GLOB_NOSPACE);
			}
		padr->ga_max = init_addr;
		padr->ga_used = first_addr;
		if (first_addr != 0)
			memset(padr->ga_adr, 0, first_addr * sizeof(char *));
		pglob->gl_padr = (void *)padr;
		pglob->gl_ptx = (void *)ptx;
		pglob->gl_pathv = &(padr->ga_adr[0]);
		pglob->gl_pathc = first_addr;
		}
	else if (flags & GLOB_DOOFFS)
		pglob->gl_pathc += pglob->gl_offs;
/*
 * save current working directory if the pattern contains subdirectories
 *   so that current working directory can be restored
 * create writable copy of pattern if it has subdirectories
 */
	if (strchr(pattern, '/') != NULL)
		{
		if (getcwd(cwd, sizeof(cwd)) == NULL)
			{
			free(padr);
			free(ptx);
			stat = check_err(errfunc, _Geterrno(), ".", NULL, flags);
			return (stat);
			}
		ppat = (char *)malloc(strlen(pattern)+1);
		if (ppat == NULL)
                        {
                        free(padr);
                        free(ptx);
                        stat = check_err(errfunc, GLOB_NOSPACE, ".", NULL, flags);
                        return (stat);
			}
		strcpy(ppat, pattern);
		}
	else
		{
		cwd[0] = '\0';
		ppat = (char *)pattern;
		}
	dirpath[0] = '\0';
/*
 * generate list of new pathnames
 * return original pattern as new pathname if no pathnames generated
 *   and caller always wants something returned
 */
	stat = globnames(ppat, flags, errfunc, pglob, dirpath, &new_count);
	if (stat == 0 && new_count == 0) {
		if (flags & GLOB_NOCHECK)
			stat = add_pname(NULL, pattern, pglob, NOSLASH, &new_count);
		else
			stat = GLOB_NOMATCH;
	}
/*
 * update total pathname count
 * null terminate address array
 */
	pglob->gl_pathc += new_count;
	pglob->gl_pathv[pglob->gl_pathc] = NULL;
/*
 * If GLOB_DOOFFS is set, only return the number of elements found, not
 * the total
 */
	if (flags & GLOB_DOOFFS)
		pglob->gl_pathc -= pglob->gl_offs;
/*
 * restore current working directory if it was potentially changed
 */
	if (cwd[0] != '\0')
		if (chdir(cwd) != 0)
			stat = check_err(errfunc, _Geterrno(), cwd, NULL, flags);
	return (stat);
}
#ifdef _NAME_SPACE_WEAK_STRONG
#define stat __stat
#endif

/************************************************************************/
/* globnames - build list of ptrs to new pathnames			*/
/************************************************************************/
static int
globnames(char *pattern, int flags, int (*errfunc)(), glob_t *pglob,
	char *dirpath, int *pcount)
{
	int	adr_off;	/* offset into ga_adr table		*/
	int	exp_count;	/* # of remaining filenames to expand	*/
	int	fatal_err;	/* fatal error status			*/
	int	fn_flags;	/* fnmatch() flags			*/
	int	len;		/* length of disppath			*/
	int	new_count;	/* # of expanded subdirectory pathanmes	*/
 struct gadr	*padr;		/* ptr to address list			*/
	DIR	*pdir;		/* ptr to directory stream		*/
	char	*penddisp;	/* ptr to append of disppath		*/
	char	*pendnew;	/* ptr to append of new			*/
#ifdef _THREAD_SAFE
	struct dirent dirbuf;	/* directory entry */
#endif
 struct	dirent	*pent;		/* ptr to directory entry		*/
	char	**pexpdir;	/* ptr to remaining expansion list	*/
	char	*plast;		/* ptr to last component in pathname	*/
	char	*ppat;		/* ptr to start of filename pattern	*/
	char	*prem;		/* ptr to remaining pattern		*/
	char	**psave;	/* ptr to saved list of address lists	*/
	int	retstat;	/* return status			*/
 struct stat	sbuf;		/* stat() information buffer		*/
	int	wildcard;	/* wildcard used in pathname component	*/
	char	newfile[FILENAME_MAX]; /* new component file name	*/
	char	disppath[PATH_MAX+1]; /* displayable cwd pathname	*/
	char	exppath[PATH_MAX+1]; /* return cwd after expansion	*/
/*
 * add leading directories "/" or "." or ".." to new component name
 *   and make sure pathname ends with "/"
 * sorry but POSIX says not to support ~ <tilde> expansion
 * change to subdirectory . or .. and return on any error
 */
	*pcount = 0;
	strcpy(disppath, dirpath);
	ppat = leading_dir(pattern);
	if (ppat != pattern) 
		{
		len = ppat - pattern;
		strncpy(newfile, pattern, len);
		pendnew = newfile + len - 1;
		if (*pendnew++ != '/')
			*pendnew++ = '/';
		*pendnew = '\0';
		strcat(disppath, newfile);
		if (chdir(newfile) != 0)
			if ((retstat = check_err(errfunc, _Geterrno(), disppath, NULL, flags)) != 0)
				return (retstat);
			else
				return (0);
		}
	else
		{
		newfile[0] = '\0';
		pendnew = newfile;
		}
	penddisp = disppath + strlen(disppath);
/*
 * substitute '*' for remaining pattern if no more pattern left
 * (WARNING:  IBM hints there may be a problem here.  [They commented this out
 *		for their defect #62477, but don't explain why)
 *		VSX4 does fail XPG4.os/genuts/glob/T.glob 1 with this uncommented.
	if (*ppat == '\0')
		{
		strcpy(pattern, "*");
		ppat = pattern;
		}
 */

/*
 * locate beginning <slash> of next subdirectory
 * replace <slash> with null to separate pattern for this pathname component
 *   from remaining pattern used for subdirectories
 */
	prem = strchr(ppat, '/');
	if (prem != NULL)
		*prem++ = '\0';
/*
 * define flags passed to fnmatch() for filename matching
 * do not need FNM_PATHNAME because don't do multiple directories at one time
 */
	fn_flags = FNM_PERIOD;
	if ((flags & GLOB_NOESCAPE) != 0)
		fn_flags |= FNM_NOESCAPE;
/*
 * determine if wildcard operators *, ?, or [] are used in pattern for
 *   any pathname component - read access also required if wildcards used
 */
	if (prem != NULL)
		wildcard = wild_check(prem, flags);
	else
		wildcard = 0;
/*
 * verify directory has search permission - open failure means no
 * stop now on error, even if told to ignore
 */
	pdir = opendir(".");
	if (pdir == NULL)
		{
		if (!wildcard && (stat(ppat, &sbuf) == 0)) {
			if (S_ISDIR(sbuf.st_mode)) {
				if (prem == NULL) {
					retstat = add_pname(disppath, ppat, pglob, NOSLASH, pcount);
					return(retstat);
				}
				else {
					retstat = add_pname(disppath, ppat, pglob, GLOB_MARK, pcount);
					goto process_dirs;
				}
			}
			else {
				retstat = add_pname(disppath, ppat, pglob, NOSLASH, pcount);
				return(retstat);
			}
		}
		if (prem != NULL)
			prem[-1] = '/';
		if ((retstat = check_err(errfunc, _Geterrno(), disppath, NULL, flags)) != 0)
			return (retstat);
		else
			return (0);
		}
/*
 * read each filename and compare just this component of pathname
 * if filename is matched by pattern
 *   reject filename if pattern is not exhausted
 *   add filename to pathname list if pattern is exhausted
 * if directory is matched by pattern
 *   reject pathname component if no read access for wildcard pattern
 *   add directory name without further subdirectories to pathname list
 * call user error function and stop when not allowed to continue
 * close directory when all names have been read
 */
	retstat = 0;

#ifdef _THREAD_SAFE
	pent = &dirbuf;
	while (readdir_r(pdir, pent) != -1)
#else
	while ((pent = readdir(pdir)) != NULL)
#endif
		{
		if (fnmatch(ppat, pent->d_name, fn_flags) == 0)
			{
			if (stat(pent->d_name, &sbuf) == 0)
				{
				if (S_ISDIR(sbuf.st_mode) == 0)
					{
					if (prem == NULL)
						retstat = add_pname(disppath, pent->d_name, pglob, NOSLASH, pcount);
					}
				else
					{
					if (wildcard != 0)
						{
						if (access(pent->d_name, R_OK) != 0)
							retstat = _Geterrno();
						}
					if (retstat == 0)
						{
						if (prem == NULL)
							retstat = add_pname(disppath, pent->d_name, pglob, flags & GLOB_MARK, pcount);
						else
							retstat = add_pname(disppath, pent->d_name, pglob, GLOB_MARK, pcount);
						}
					}
				}
			else
				retstat = _Geterrno();
			if (retstat != 0)
				if ((retstat = check_err(errfunc, retstat, disppath, pent->d_name, flags)) != 0)
					break;
				else
				retstat = 0;
			}
		}
	closedir(pdir);

process_dirs:

	if (prem != NULL)
		prem[-1] = '/';
/*
 * save any fatal error status and return it later on
 * this occurs when a directory error has been detected but
 *   existing directories must still be processed
 */
	fatal_err = retstat;
/*
 * always sort list of names for this directory, even if errors
 * add trailing '/' now that everything is sorted
 * return now if no subdirectories to expand
 */
	exp_count = *pcount;
	pexpdir = (char **)pglob->gl_pathv + ((struct gadr *)pglob->gl_padr)->ga_used - exp_count;
	if (*pcount > 1 && (flags & GLOB_NOSORT) == 0)
		qsort((void *)pexpdir,
		      (size_t)*pcount,
		      sizeof(char *),
		      (int (*)(const void *,const void *))path_compare);
	while (exp_count-- > 0)
		{
		if ((*pexpdir)[-1] == '/')
			strcat(*pexpdir, "/");
		pexpdir++;
		}
	if (prem == NULL)
		return (fatal_err != 0 ? fatal_err : retstat);
/*
 * save cwd so "." can be restored after recursive call to globnames()
 * save address list below directory name being expanded
 */
	if (getcwd(exppath, sizeof(exppath)) == NULL)
		return (fatal_err != 0 ? fatal_err : check_err(errfunc, _Geterrno(), disppath, NULL, flags));
	exp_count = *pcount;
	while (exp_count > 0)
		{
		pexpdir = (char **)pglob->gl_pathv + ((struct gadr *)pglob->gl_padr)->ga_used - exp_count;
		(*pcount)--;
		((struct gadr *)pglob->gl_padr)->ga_used -= exp_count;
		if (--exp_count > 0)
			{
			psave = (char **)malloc(sizeof (char *) * exp_count);
			if (psave == NULL)
				return (fatal_err != 0 ? fatal_err : check_err(errfunc, GLOB_NOSPACE, disppath, NULL, flags));
			memcpy(psave, pexpdir+1, sizeof (char *) * exp_count);
			}
		else
			psave = NULL;
/*
 * determine last directory component name
 * add component name to end of current directory, making new pathname
 * change cwd to subdirectory
 * expand subdirectory filename components with remaining pattern
 * restore cwd
 * stop on error only if instructed to do so
 * increment pathname totals for # of new names from subdirectory
 */
		plast = strrchr(*pexpdir, '/');
		if (plast != NULL)
			{
			if (plast[1] == '\0')
				while (plast > *pexpdir)
					if (*--plast == '/' || *plast == '\0')
						{
						plast++;
						break;
						}
			}
		else
			plast = *pexpdir;
		strcat(penddisp, plast);
		if (chdir(plast) == 0)
			{
			retstat = globnames(prem, flags, errfunc, pglob, disppath, &new_count);
			if (retstat != 0)
				goto rtn_error;
			if (chdir(exppath) != 0)
				{
				retstat = check_err(errfunc, _Geterrno(), exppath, NULL, flags);
				goto rtn_error;
				}
			}
		else if ((retstat = check_err(errfunc, _Geterrno(), disppath, NULL, flags)) != 0)
			goto rtn_error;
		*penddisp = '\0';
/*
 * restore trailing pathnames to bottom of address list
 */
		if (exp_count > 0)
			{
			padr = (struct gadr *)pglob->gl_padr;
			adr_off = padr->ga_used;
			while (adr_off + exp_count >= padr->ga_max)
				{
				padr = (struct gadr *)realloc(padr, sizeof(int) * 2 + sizeof(char *) * (padr->ga_max + ADRBUF_SIZE));
				if (padr == NULL)
					return (fatal_err != 0 ? fatal_err : check_err(errfunc, GLOB_NOSPACE, exppath, NULL, flags));
				padr->ga_max += ADRBUF_SIZE;
				pglob->gl_pathv = &(padr->ga_adr[0]);
				pglob->gl_padr = padr;
				}
			memcpy(&(padr->ga_adr[adr_off]), psave, sizeof (char *) * exp_count);
			free(psave);
			((struct gadr *)pglob->gl_padr)->ga_used += exp_count;
			}
		*pcount += new_count;
		}
	return (fatal_err != 0 ? fatal_err : retstat);
/*
 * error return to free malloc'd memory
 */
rtn_error:
	if (psave != NULL)
		free(psave);
	return (fatal_err != fatal_err ? fatal_err : retstat);
}


/************************************************************************/
/* leading_dir - locate leading directory portion of pattern		*/
/************************************************************************/
static char *
leading_dir(char *p)
{
/*
 * skip past leading "/" or "." or ".." or any contiguous combination
 */
	while (1)
		{
		if (*p == '/')
			{
			p++;
			continue;
			}
		else if (*p == '.')
			{
			if (p[1] == '/')
				{
				p += 2;
				continue;
				}
			else if (p[1] == '\0')
				p++;
			else if (p[1] == '.')
				if (p[2] == '/')
					{
					p += 3;
					continue;
					}
				else if (p[2] == '\0')
					p += 2;
			}
		break;
		}
	return (p);
}


/************************************************************************/
/* add_pname - add new entry to bottom of pathname list			*/
/************************************************************************/
static int
add_pname(char *pdir, const char *path, glob_t *pglob, int sflag, int *pcount)
{
	int	adr_off;	/* offset into address array		*/
	int	len;		/* pathname length			*/
	struct	gadr	*padr;	/* ptr to pathname address structure	*/
	struct gtext	*ptx;	/* ptr to pathname text structure	*/
	int	tx_off;		/* offset into text array		*/

/*
 * add new pathname to bottom of pathname list
 * reallocate pathname address structure if only one entry left
 *   (need to also save room for trailing null address)
 */
	padr = (struct gadr *)pglob->gl_padr;
	adr_off = padr->ga_used;
	if (adr_off + 1 >= padr->ga_max)
		{
		padr = (struct gadr *)realloc(padr, sizeof(int) * 2 + sizeof(char *) * (padr->ga_max + ADRBUF_SIZE));
		if (padr == NULL)
			return (GLOB_NOSPACE);
		padr->ga_max += ADRBUF_SIZE;
		pglob->gl_pathv = &(padr->ga_adr[0]);
		pglob->gl_padr = padr;
		}
/*
 * add new pathname to bottom of pathname text
 * reserve trailing byte for null and possible '/'
 * allocate new buffer at head of chain and link to current one if no room
 * don't bother to try and fill unused portions of previous text buffers
 */
	ptx = (struct gtext *)pglob->gl_ptx;
	tx_off = ptx->gt_used;
	len = strlen(path) + 2;
	if (sflag != NOSLASH)
		{
		sflag = '/';
		len++;
		}
	if (pdir != NULL)
		len += strlen(pdir);
	if (len > TEXTBUF_SIZE - ptx->gt_used)
		{
		ptx = (struct gtext *)malloc(sizeof(struct gtext));
		if (ptx == NULL)
			return (GLOB_NOSPACE);
		ptx->gt_next = (struct gtext *)pglob->gl_ptx;
		pglob->gl_ptx = (void *)ptx;
		ptx->gt_used = 0;
		tx_off = 0;
		}
/*
 * store initial <slash> or <NUL> for restoration to end after sorting
 * move new pathname to text buffer
 */
	ptx->gt_text[tx_off] = sflag;
	if (pdir != NULL)
		{
		strcpy(&(ptx->gt_text[tx_off+1]), pdir);
		strcat(&(ptx->gt_text[tx_off+1]), path);
		}
	else
		strcpy(&(ptx->gt_text[tx_off+1]), path);
/*
 * add new entry to bottom of address list
 * increment use counters in both text and address structues
 */
	padr->ga_adr[adr_off] = &(ptx->gt_text[tx_off+1]);
	ptx->gt_used += len;
	padr->ga_used++;
	(*pcount)++;
	return (0);
}


/************************************************************************/
/* path_compare - qsort() comparison function to compare two pathnames	*/
/************************************************************************/
static int
path_compare(char **p1, char **p2)
{
	return (strcoll(*p1, *p2));
}


/************************************************************************/
/* wild_check - check pattern for wildcard special characters		*/
/************************************************************************/
static int
wild_check(char *pattern, int flags)
{
	wchar_t	wc;		/* process code of next pattern char	*/
	int	wc_len;		/* # bytes for next pattern char	*/

/*
 * if no quoted characters allowed, then just scan for special characters
 */
	if ((flags & GLOB_NOESCAPE) != 0)
		return ((int)strpbrk(pattern, "*?["));
/*
 * since quoting is allowed and characters of interest are above 0x3f,
 * convert each character to process code and look for special
 *   characters not preceeded by <backslash>
 */
	while (*pattern != '\0')
		{
		wc_len = mbtowc(&wc, pattern, MB_CUR_MAX);
		if (wc_len < 0)
			{
			wc_len = 1;
			wc = *pattern & 0xff;
			}
		pattern += wc_len;
		switch(wc)
			{
		case '*':
		case '?':
		case '[':
			return (1);
		case '\\':
			wc_len = mblen(pattern, MB_CUR_MAX);
			if (wc_len < 0)
				wc_len = 1;
			pattern += wc_len;
			}
		}
	return (0);
}
		

/************************************************************************/
/* check_err - report error and decide whether to stop			*/
/************************************************************************/
static int
check_err(int (*errfunc)(), int error, char *disppath, char *component, int flags)
{
	int	retstat;	/* errfunc() return status		*/
	char	ebuf[PATH_MAX+1]; /* pathname to be displayed		*/
	char	*ps;		/* temp pointer to string		*/
/*
 * invoke caller's error function if it is defined
 * append component to pathname, use '.' if nothing else available
 */
	if (errfunc != NULL)
		{
		strcpy(ebuf, disppath);
		if (component != NULL)
			strcat(ebuf, component);
		else
			{
			ps=strrchr(ebuf, '/');
			if (ps != NULL && ps[1]=='\0' && ps != ebuf)
				*ps = '\0';
			}
			
		if (ebuf[0] == '\0')
			strcpy(ebuf, ".");
		retstat = (*errfunc)(ebuf, error == GLOB_NOSPACE ? ENOMEM : error);
		}
	else
		retstat = 0;
/*
 * always terminate if reporting malloc error
 */
	if (error == GLOB_NOSPACE)
		return (GLOB_NOSPACE);
/*
 * terminate if errfunc() says to stop or no errors allowed
 */
	if (retstat != 0 || (flags & GLOB_ERR) != 0)
		return (GLOB_ABORTED);
	return (0);
}		


/************************************************************************/
/* globfree - release memory used in pglob structure			*/
/************************************************************************/
void
globfree(glob_t *pglob)
{
 struct	gtext	*ptx;		/* ptr to pathname text buffer		*/
 struct gtext	*ptx_next;	/* ptr to next pathname text buffer	*/

/*
 * release memory associated with pathname address buffer
 */
	if (pglob->gl_padr != NULL)
		free(pglob->gl_padr);
	pglob->gl_padr = NULL;
/*
 * release memory associated with each pathname text buffer
 */
	ptx = (struct gtext *)pglob->gl_ptx;
	while (ptx != NULL)
		{
		ptx_next = ptx->gt_next;
		free(ptx);
		ptx = ptx_next;
		}
	pglob->gl_ptx = NULL;
	pglob->gl_pathc = 0;
	return;
}
