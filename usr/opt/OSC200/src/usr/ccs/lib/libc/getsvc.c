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
static	char	*sccsid = "@(#)$RCSfile: getsvc.c,v $ $Revision: 4.3.6.3 $ (DEC) $Date: 1993/07/20 22:47:59 $";
#endif lint
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
 * Modification History:
 *
 * 08-Nov-90	jsd
 *	Replace fscanf with fgets and cleaner logic to save time and space.
 *
 * 13-Nov-89	sue
 *	Added an fclose to an error leg in init_svc().
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak getsvc_r = __getsvc_r
#pragma weak init_svc_r = __Init_svc_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak getsvc = __getsvc
#pragma weak init_svc = __Init_svc
#endif
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/svcinfo.h>
#include <strings.h>
#include <sys/stat.h>
#ifdef	_THREAD_SAFE
#include "ts_supp.h"
#endif	/* _THREAD_SAFE */

#define STUFFSIZE 1000

/* strings in file svc.conf */

static char * databases[] = {
	"aliases",
	"auth",
	"group",
	"hosts",
	"netgroup",
	"networks",
	"passwd",
	"protocols",
	"rpc",
	"services",
	""
};

#ifdef	_THREAD_SAFE
#ifdef _NAME_SPACE_WEAK_STRONG
#ifdef svc_stat
#undef svc_stat
#endif
#endif

#define	INITSVC(x,y)		init_svc_r(x,y)
#define	SVCFAIL		TS_FAILURE

#else

int svc_lastlookup; /* svc_lastlookup is used externally */
struct svcinfo svc_info;
struct stat svc_stat;

#define	INITSVC(x,y)		init_svc()
#define	SVCFAIL		NULL

#endif	/* _THREAD_SAFE */

#ifdef	_THREAD_SAFE
int getsvc_r(struct svcinfo *svi)
#else
struct svcinfo *getsvc(void)
#endif	/* _THREAD_SAFE */
{
#ifdef	_THREAD_SAFE
	struct stat svc_stat;
#else
	struct svcinfo *svi = &svc_info;
#endif	/* !_THREAD_SAFE */

	if (stat(SVC_CONF, &svc_stat) < 0) {
		(void) fprintf(stderr, "getsvc: stat of %s failed\n", SVC_CONF);
		perror("getsvc: stat failed");
		return SVCFAIL;
	}
	if (svc_stat.st_mtime != (time_t)svi->svcdate) {
#ifdef	_THREAD_SAFE
		memset(svi, 0, sizeof (struct svcinfo));
#endif	/* _THREAD_SAFE */
		if(INITSVC(svi, &svc_stat) == SVCFAIL) {
			(void) fprintf(stderr, "getsvc: cannot initialize from %s\n", SVC_CONF);
			return SVCFAIL;
		}
	}
#ifdef	_THREAD_SAFE
	return TS_SUCCESS;
#else
	return &svc_info;
#endif	/* _THREAD_SAFE */
}

#ifdef	_THREAD_SAFE
init_svc_r(struct svcinfo *svi, struct stat *svs)
#else
init_svc(void)
#endif	/* _THREAD_SAFE */
{
#ifndef	_THREAD_SAFE
	struct svcinfo *svi = &svc_info;
	struct stat *svs = &svc_stat;
#endif	/* !_THREAD_SAFE */
	FILE *stream;
	char stuff[STUFFSIZE];

	if ((stream = fopen (SVC_CONF, "r")) == 0) {
	        fprintf(stderr, "init_svc: cannot open %s, errno = %d\n", SVC_CONF, errno);
		return SVCFAIL;
	}
	else {
		while (getsvcstring(stream,stuff) > 0)
			if (map_string_to_data (stuff, svi) == -1) {
				(void) fprintf(stderr, "init_svc: bad format in file %s\n", SVC_CONF);
        			if (fclose(stream) == EOF)
					(void) fprintf(stderr, "init_svc: cannot close %s, errno = %d\n", SVC_CONF, errno);
				return SVCFAIL;
			}
	}
        if (fclose(stream) == EOF)
		(void) fprintf(stderr, "init_svc: cannot close %s, errno = %d\n", SVC_CONF, errno);
#ifdef	_THREAD_SAFE
	svi->svcdate = svs->st_mtime;
	return TS_SUCCESS;
#else
	(time_t)svc_info.svcdate=svc_stat.st_mtime;
	return 1;
#endif	/* THREAD_SAFE */
}

/* This function reads lines from /etc/svc.conf.  It returns each
 * line seperately, stripped of comments and white space.
 * It returns NULL at end of file.
 */
static int getsvcstring(FILE *stream, char *buff)
{
	char *cp;
	int i;

top:
	if (fgets(buff, STUFFSIZE-1, stream) == NULL)
		return(NULL);

	/* zap the newline */
	buff[strlen(buff)-1] = NULL;

	if ((buff[0] == NULL) || (buff[0] == '#'))
		goto top;	/* skip blank lines or comment lines */

	/* zap trailing comment and/or white space */
	cp=buff;
	i=0;
	while (*cp) {
		if ((*cp == ' ') || (*cp == '\t')) {
			cp++;	/* skip over space or tab */
		} else {
			if (*cp == '#')	/* stop if we reach a comment */
				break;
			buff[i++] = *cp++;
		}
	}
	buff[i] = NULL;
	if (i == 0)	/* blank line (white space only) */
		goto top;
	/* printf("buff=|%s|\n",buff); */
	return(strlen(buff));
}

#ifdef	_THREAD_SAFE
static int map_string_to_data(char *p, struct svcinfo *svi)
#else
static int map_string_to_data (char *p)
#endif	/* _THREAD_SAFE */
{
#ifndef	_THREAD_SAFE
	struct svcinfo *svi = &svc_info;
#endif	/* !_THREAD_SAFE */
	char *rhs, *s, *eol;
	int i, j;
        int lastone;

	if (*p == '#')
	      return(NULL);
	if ((rhs = strchr( p, '=')) == (char *)0)
		return(-1);
	*rhs = '\0';

	i = 0;
	s = databases[0];
	while (*s != '\0') {
		if (strcmp(s,p) == 0)
			break;
		i++;
		s = databases[i];
	}
	rhs++;
	for (eol = rhs; eol && *eol && (*eol != ' ' || *eol != '\t' || *eol != '\n'); eol++)
		;
	*eol = '\0';
	svi->svcauth.seclevel = SEC_BSD;
	if (*s == '\0') {
	  if ((strcmp("PASSLENMIN",p) != 0) &&
	      (strcmp("PASSLENMAX",p) != 0) &&
	      (strcmp("SOFTEXP",p) != 0) &&
	      (strcmp("SECLEVEL",p) != 0)) 
	    {
	      return(-1);
	    }
	  return(NULL);
	}

        lastone = 0;
	for (j=0; ; j++) {
		s = rhs;
		while (1) {
			if (*s == ',')
				break;
			if (*s == '\0') {
				lastone = 1;
				break;
			}
			s++;
        	}
		*s = '\0';
 		if (strcmp("local", rhs) == 0)
			svi->svcpath[i][j] = SVC_LOCAL;
		else if (strcmp("yp", rhs) == 0)
			svi->svcpath[i][j] = SVC_YP;
		else if 
		  ((strcmp(databases[i],"hosts") == 0) && (strcmp("bind", rhs) == 0))
			svi->svcpath[i][j] = SVC_BIND;
		/* ignore "bind" entries for non-hosts databases */
		else if (strcmp("bind", rhs) == 0)
		        j--;   
		else
			return(-1);
		if (lastone) {
			svi->svcpath[i][j+1] = SVC_LAST;
			break;
		}
		rhs = ++s;
	}
	return(NULL);
}
