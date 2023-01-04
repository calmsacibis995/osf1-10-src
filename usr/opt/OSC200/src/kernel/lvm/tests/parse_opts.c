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
static char	*sccsid = "@(#)$RCSfile: parse_opts.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:29:11 $";
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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "parse_opts.h"

int optind;

int
parse_opts(argc, argv)
int argc;
char **argv;
{
char *arg, *opt, *nextval;
struct option *o;
struct values *v;
int value, mode, arglen;

    optind = 1;
    while (optind < argc) {
	arg = argv[optind];
	if (*arg != '-') {
		break;
	}
	arg++;
	arglen = strlen(arg);
	for (o = &opts[0]; o->name; o++) {
		if ((arglen >= o->minlength)
			&& (strncmp(o->name, arg, arglen) == 0)) {
			/* got a match */
			optind++;
			switch (o->arg_type) {
			case ARG_BOOL:
			    *(o->value) ^= 1;
			    break;

			case ARG_INT:
			    opt = argv[optind++];
			    if (isdigit(*opt)) {
				    *(o->mask) = ~0;
				    *(o->value) = atoi(opt);
			    } else {
				switch (mode = *opt) {
				case '=':
					*(o->mask) = ~0;
					opt++; break;
				case '+':
					*(o->mask) = 0;
					opt++; break;
				case '-':
					*(o->mask) = 0;
					opt++; break;
				default:
					*(o->mask) = 0;
					mode = '+';
				}
				for (;opt && *opt; opt = nextval) {
				    if (nextval = index(opt,'|')) {
					*nextval = '\0';
					nextval++;
				    }
				    for (v = o->values;
				    	v && v->keyword; v++) {
				    	if (strcmp(v->keyword, opt)==0) {
					    if (mode == '-') {
						*(o->mask) |= v->mask;
						*(o->value) &= 
						    ((~v->value)&v->mask);
					    } else {
						*(o->mask) |= v->mask;
						*(o->value) |= v->value;
					    }
					    break;
					}
				    }
				    if (v && v->keyword) continue;
				    fprintf(stderr,
					"Unrecognized option value %s\n", opt);
				    return(1);
				}
			    }
			    break;
			}
		break;
		}
	}
	if (o->name == NULL)
		return(1);
    }
    return(0);
}
