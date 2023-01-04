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
static char *rcsid = "@(#)$RCSfile: regexp_jacket.c,v $ $Revision: 1.1.6.5 $ (DEC) $Date: 1993/09/03 18:48:10 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <regex.h>
#include <stdlib.h>

#define _REGEXP_DEFS_ONLY
#include <regexp.h>

int
__Regcomp( void **handle, const char *str, int cflags, int *nsub)
{
    regex_t	*p = malloc(sizeof(regex_t));
    int		status;

    *handle = p;		/* Update handle */

    if ( !p )			/* Malloc failed? */
	return (_BIGREGEXP);

    status =  regcomp( p, str, cflags );

    if (nsub)
	*nsub = p->re_nsub;

    switch (status) {
      case 0:
	break;

      case REG_ECOLLATE:
      case REG_ECTYPE:
      case REG_BADRPT:
	status = _ABNORMAL; /* no meaningful regexp() equivalent */
	break;

      case REG_BADPAT:
      case REG_NOMATCH:
      case REG_ECHAR:
	status = _ABNORMAL; /* these things shouldn't come from regcomp */
	break;

      case REG_BADBR:
      case REG_EBRACE:
	status = _ABNORMAL; /* these are subdivided by code in compile() */
	break;

      case REG_EPAREN:
	status = _BADPAREN; /* so is this */
	break;

      case REG_EESCAPE:
	status = _NODELIM; /* and so is this */
	break;

      case REG_ERANGE:
	status = _BIGRANGE;
	break;

      case REG_ESUBREG:
	status = _BIGDIGIT;
	break;

      case REG_EBRACK:
	status = _BADBRACKET;
	break;

      case REG_ESPACE:
	status = _BIGREGEXP;
	break;

      default:
	status = -1;
    }

    return (status);
}



int
__Regexec( void *reg, const char *str, int *lb, int *ub, int eflags)
{
    regmatch_t	match;
    int status;

    status = regexec(reg, str, 1, &match, eflags? REG_NOTBOL: 0);

    if (!status) {
	*lb = match.rm_so;
	*ub = match.rm_eo;
    }

    return status;
}


void
__Regfree( void *preg)
{
    if ( !preg )
	return;

    regfree(preg);
    free(preg);
}
