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
static char *sccsid  =  "@(#)$RCSfile: argstrings.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/10/13 14:28:55 $";
#endif

/*
 * argstrings.c -- table of arguments strings 
 * for PostScript (tm) parameters 
 */

/* 
 * Rewritten by John Allen Painter
 *  Based on the previous work of:
 *      maxwell
 *      Adrian Thoms
 */

#include <stdio.h>
#include "argstrings.h"

/* 
 *  global data structures for new arguments
 *  note strings must be in alphabetic order
 */
static struct arg_pair data_type_args[] = 
{
    {"ansi","ansi",0},
    {"ascii","ascii",0},
    {"postscript","postscript",0},
    {"regis","regis",0},
    {"tek4014","tek4014",0},
    { NULL,0,0}
};

static struct arg_pair input_tray_args[] = 
{
    {"bottom","bottom",0},
    {"lcit","bottom",0},
    {"middle","middle",0},
    {"top","top",0},
    { NULL,0,0}
};

static struct arg_pair output_tray_args[] = 
{
    {"face-up","face_up",0},
    {"lcos","lcos",0},
    {"lower","lower",0},
    {"side","side",0},
    {"top","top",0},
    {"upper","upper",0},
    { NULL,0,0}
};

static struct arg_pair orientation_args[] = 
{
    {"landscape","landscape",0},
    {"portrait","portrait",0},
    { NULL,0,0}
};

static struct arg_pair page_size_args[] = 
{
    /* warning, if you put in #10, put the # in octal
     * as daemon takes leading # to mean "numerical parameter"
     * (i.e. numerical parameters don't acquire () quotes)
     */
    {"a","a",0},
    {"a3","a3",0},
    {"a4","a4",0},
    {"a5","a5",0},
    {"b","b",0},
    {"b4","b4",0},
    {"b5","b5",0},
    {"executive","executive",0},
    {"ledger","b",0},
    {"legal","legal",0},
    {"letter","a",0},
    { NULL,0,0}
};

static struct arg_pair message_args[] = 
{
    {"ignore","ignore",0},
    {"keep","keep",0},
    { NULL,0,0}
};

/* 
 *  note the second string is passed directly to the 
 *  LPS_SETSIDES dcm by lpd and should not be changed 
 */
static struct arg_pair  sides_args[] = 
{
    {"1","one",0},
    {"2","two",0},
    {"one_sided_duplex","one_sided_duplex",0},
    {"one_sided_simplex","one",0},
    {"one_sided_tumble","one_sided_tumble",0},
    {"tumble","tumble",0},
    {"two_sided_simplex","two_sided_simplex",0},
    {"two_sided_duplex","two",0},
    {"two_sided_tumble","tumble",0},
    { NULL,0,0}
};

static struct arg_pair * valid_args[] = 
{
    data_type_args,
    input_tray_args,
    output_tray_args,
    orientation_args,
    page_size_args,
    message_args,
    sides_args
};

/*
 * calculate min unique stems for each string in list 
 * and store result in minlen field of list
 */

void init_args()
{
    struct arg_pair *cur_ptr;
    int i, prev, next;

    for (i = 0; i < num_opts; i++) 
    {
	cur_ptr = valid_args[i];
	prev = 0;
	while ((cur_ptr+1)->arg) 
        {
	    next = strstemlen(cur_ptr->arg,(cur_ptr + 1)->arg);
	    (cur_ptr++)->minlen = prev > next ? prev : next;
	    prev = next;
	}
	cur_ptr->minlen = prev;
    }
}

/*
 * returns pointer to argstrings array (returns in arglist)
 */
void get_args(int opt_num,struct arg_pair **arg_list)
{
    *arg_list = valid_args[opt_num];
}


/*
 * check validity of arguments
 * returns 0 and a pointer to cf str in canstr if match found
 * or -1 if no unique match (was used for datatype matching - now defunct)
 * or -2 if no match at all
 */
int check_arg(char *arg,int opt_num,char **canstr)
{
    struct arg_pair * arg_list = valid_args[opt_num];
    int len;
    register char *ptr;
    char lc_copy[128];

    len = strlen(arg);
    ptr=lc_copy;          /* make lowercase copy of arg */
    while (*ptr++ = tolower(*arg++));

    while (arg_list->arg) 
    {
        if (strncmp(lc_copy,arg_list->arg,len) == 0) 
        {
            if (len <  arg_list->minlen) 
            {
                return(-1);
	    }
	    else 
            {
		*canstr=arg_list->cfentry;
		return(0);
            }
	}
	arg_list++;
    };
    return(-2);
}

/*
 * returns length of common stem 
 */
static int strstemlen(char *s1, char *s2)
{
    int c = 1;
    while (*s1++ == *s2++) 
    {
	if (!*s1) 
        { /* reached end of string */
	    return(c);
	}
	else 
        {
	    c++;
	}
    };
    return(c);
}
