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
/*  @(#)$RCSfile: argstrings.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/10/13 14:30:23 $  */

/* 
 * argstrings.h header file 
 */

/* Rewritten by John Allen Painter
 *  Based on the work of:
 *      maxwell
 *      thoms
 */

/*
 *  extern char *index(); already declared in string.h 
 */

#define as_data_types	0
#define as_input_trays	1
#define as_output_trays	2
#define as_orientations	3
#define as_page_sizes	4
#define as_messages	5
#define as_sides	6

#define num_opts	7

/* 
 *  The next two are so that the checking functions can
 *  check all different parameters including strings
 *  and numeric (digit string) parameters
 */

#define as_string	998
#define as_numerical	999

/* maximum numerical PostScript arg */
#define MAXARG	10000

struct arg_pair 
{
    char *arg;                    /* valid arg str */
    char *cfentry;                /* entry in control file */
    int  minlen;                  /* len of min unique match */
};

extern void init_args();
extern void get_args();
