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
static char	*sccsid = "@(#)$RCSfile: timezone.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/10/13 13:26:04 $";
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
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/*
 * COMPONENT_NAME: (LIBBSD)  Berkeley Compatibility Library
 *
 * FUNCTIONS: timezone
 *
 * ORIGINS: 26 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * The arguments are the number of minutes of time
 * you are westward from Greenwich and whether DST is in effect.
 * It returns a string
 * giving the name of the local timezone.
 *
 */

/*
DESIGN NOTE:
Default BSD action passes back timezone name from the environment
variable TZNAME.  If TZNAME is not set, then the arguments are used
to index the array zonetab below.

For more complete compatibility with AIX, the environment variable
TZ is checked as well before going on to use the default name table.
The code for extracting the timezone names is lifted from the AIX-JLS
version of ctime.c.

The functions used, gettzname() and atosec() are identical to those
in ctime.c.  They really should be busted out of this file and the
ctime.c file and shared so that the format of the TZ environment variable
can change without having to change both of these files.
*/

static struct zone {
	int	offset;
	char	*stdzone;
	char	*dlzone;
} zonetab[] = {
	-1*60, "MET", "MET DST",	/* Middle European */
	-2*60, "EET", "EET DST",	/* Eastern European */
	4*60, "AST", "ADT",		/* Atlantic */
	5*60, "EST", "EDT",		/* Eastern */
	6*60, "CST", "CDT",		/* Central */
	7*60, "MST", "MDT",		/* Mountain */
	8*60, "PST", "PDT",		/* Pacific */
#ifdef notdef
	/* there's no way to distinguish this from WET */
	0, "GMT", 0,			/* Greenwich */
#endif
	0*60, "WET", "WET DST",		/* Western European */
	-10*60, "EST", "EST",		/* Aust: Eastern */
	-10*60+30, "CST", "CST",	/* Aust: Central */
	-8*60, "WST", 0,		/* Aust: Western */
	-1
};

#if defined (NLS) || defined (KJI)
	/* this code borrowed from ctime.c */
	/*  Pad tzname fields to allow for name up to length NLTZSIZE-1.
	 *  This prevents strcpy() in tzset from overflowing
	 */
#define NLTZSIZE 10
static char    *tzname[] = {"EST\0.....", "EDT\0.....",};
#endif


char *timezone(zone, dst)
{
	register struct zone *zp;
	static char czone[10];
	char *sign;
	register char *p, *q;
	char *getenv(), *index();

	if (p = getenv("TZNAME")) {
		if (q = index(p, ',')) {
			if (dst)
				return(++q);
			else {
				*q = '\0';
				strncpy(czone, p, sizeof(czone)-1);
				czone[sizeof(czone)-1] = '\0';
				*q = ',';
				return (czone);
			}
		}
		return(p);
	}
#if defined (NLS) || defined (KJI)
	/* this code borrowed from ctime.c */
	if((p = getenv ("TZ")) && *p) {
		char *gettzname(), *atosec();
		int timezoff;
		p = gettzname(p,0);
		if (!dst)
			return(tzname[0]);
		p = atosec(p, &timezoff);
		p = gettzname(p,1);
			return(tzname[1]);
	}
#endif
	for (zp=zonetab; zp->offset!=-1; zp++)
		if (zp->offset==zone) {
			if (dst && zp->dlzone)
				return(zp->dlzone);
			if (!dst && zp->stdzone)
				return(zp->stdzone);
		}
	if (zone<0) {
		zone = -zone;
		sign = "+";
	} else
		sign = "-";
	sprintf(czone, "GMT%s%d:%02d", sign, zone/60, zone%60);
	return(czone);
}


#if defined (NLS) || defined (KJI)
/* this code borrowed from ctime.c */
static
char *atosec(p, result)
char *p;
long *result;
{
	register int n, sign;
	register long v;
	/*
	 *  Convert string of form [-]hh.mm to seconds
	 */
	if(sign = *p == '-')
		p++;
	n = 0;
	while(*p >= '0' && *p <= '9')
		n = (n * 10) + *p++ - '0';
	v = ((long)(n * 60)) * 60;
	if(*p == '.') {
		p++;
		n = 0;
		while(*p >= '0' && *p <= '9')
			n = (n * 10) + *p++ - '0';
		v += n * 60;
	}
	if(sign)
		v = -v;
	*result = v;
	return p;
}

static
char *
gettzname(p,index)
char *p;
{
	register char *q;
	register int n, i;
	q = tzname[index];
	n = NLTZSIZE-1;
	while (*p) {
		if ( (*p>='0' && *p<='9') || *p == '-' || *p==':')
		    break;
		if (n--)
		    *q++ = *p++;
	}
	*q = 0;
	/* for compatibility with standard tzname, pad with blanks to  */
	/* at least three bytes */
	if ( (n = strlen(tzname[index])) < 3 ) {
	    tzname[index][0] = 0;  /* zero = no daylight savings time */
	    n++;
	    while(n<3) tzname[index][n++] = ' ';
	    tzname[index][3] = 0;
	}
	return p;
}

static
char *
getfield(p,s)
char *p, *s;
{
	while( *p && *p != ':' )
	    *s++ = *p++;
	if (*p == ':')
	    p++;
	*s = 0;
	return p;
}
#endif

