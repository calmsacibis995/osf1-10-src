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
static char *rcsid = "@(#)$RCSfile: tc_option_data.c,v $ $Revision: 1.2.16.10 $ (DEC) $Date: 1993/11/17 23:13:07 $";
#endif

/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from tc_option_data.c	2.2      (ULTRIX)  3/2/90";
 */

/*
 * Modification History: tc_option_data.c
 *
 * 22-Mar-92	Uttam Shikarpur
 *	Added DEFTA adapter
 *
 * 05-Jun-91	Chran-Ham Chang
 *	Add DEFZA adapter
 *
 * 05-Jun-91	Mark Parenti
 *	Add VME adapter.
 *
 * 05-May-91	Joel Gringorten
 *	Changes for new WS graphics driver.
 *
 * 26-Jan-90	Randall Brown
 *	Changed file to reference tc structures. Moved from maxoption_data.c
 *
 * 29-Dec-89	afd
 *	Added 2 new fields for when to enable the interrupt line for devices.
 *	Added "type" and "adapter config routine" fields to the table.
 *
 * 26-Oct-89	afd
 *	Put in the correct module names for the options that we know about.
 *
 * 12-Sep-89	afd
 *	Created this file to contain a data table that maps the 8 byte
 *	IO module name in the option ROM to the device or controler name
 *	in the config file.
 *
 */

#include <io/dec/tc/tc.h>

extern	int	tscsiconf();

#ifdef mips
extern	int	xviaconf();
extern  int     tcciconf();
#endif /* mips */

/*
 * Dynamically added extern declarations
 * Note: If the struct tc_option line is changed you must modify
 * the mktcdata utility as it keys on that line for placement of
 * dynamically added externs.
 */

struct tc_option tc_option [] =
{
    /*	module		driver	intr_b4	itr_aft		adpt	*/
    /*	name		name	probe	attach	type	config	*/
    /*	------		------	-------	-------	----	------	*/

    {	"PMTNV-AA", 	"nvtc",	  0,	  1,	'C',	0},	/* TCNVRAM */
    {	"PMAP-AA ", 	"nvtc",	  0,	  1,	'C',	0},	/* TCNVRAM temp */
    {	"PMAD-AA ",	"ln",	  0,	  1,	'C',	0},	/* Lance */
    {   "PMAF-AA ",     "fza",    1,      1,    'C',    0},     /* FDDI */
    {   "PMAF-FA ",     "fta",    1,      1,    'C',    0},     /* FDDI */
    {   "PMAF-FS ",     "fta",    1,      1,    'C',    0},     /* FDDI */
    {   "PMAF-FD ",     "fta",    1,      1,    'C',    0},     /* FDDI */
    {   "PMAF-FU ",     "fta",    1,      1,    'C',    0},     /* FDDI */
    {	"PMAZ-AA ",	"asc",	  0,	  1,	'C',	0},	/* SCSI */
    {	"PMAZ-DS ",	"tcds",	  0,	  1,	'A',	0},	/* TCDS */
    {	"PMAZ-FS ",	"tcds",	  0,	  1,	'A',	0},	/* TCDS */
    {	"PMAZB-AA",	"tcds",	  0,	  1,	'A',	0},	/* TCDS */
    {	"PMAZB-AB",	"tcds",	  0,	  1,	'A',	0},	/* TCDS */
    {	"PMAZC-AA",	"tcds",	  0,	  1,	'A',	0},	/* TCDS */
    {   "KZTSA-AA",     "tza",    0,      1,    'C',    0},     /* TZA */
    {   "PMAGB-BA",     "fb",     0,      1,    'C',    0},     /* SFB */
    {	"PMAG-RO ",	"fb",	  0,	  0,	'C',	0},	/* RO*/
    {	"PMAG-JA ",	"fb",	  0,	  0,	'C',	0},	/* RO*/
    {	"PMAG-CA ",	"px",	  0,	  1,	'C',	0},	/* 2DA */
    {	"PMAG-DA ",	"px",	  0,	  1,	'C',	0},	/* LM-3DA */
    {	"PMAG-FA ",	"px",	  0,	  1,	'C',	0},	/* HE-3DA */
    {	"PMAG-FB ",	"px",	  0,	  1,	'C',	0},	/* HE+3DA */
    {	"PMAGB-FA",	"px",	  0,	  1,	'C',	0},	/* HE+3DA */
    {	"PMAGB-FB",	"px",	  0,	  1,	'C',	0},	/* HE+3DA */
    {   "PMAGC   ",	"pv",     0,      1,    'C',    0},     /* Low PV */
    {   "PMAGC-AA",	"pv",     0,      1,    'C',    0},     /* Low PV */
    {   "PMAGC-BA",	"pv",     0,      1,    'C',    0},     /* Mid PV */
    {   "PMAT-AA ",	"tra",    0,      1,    'C',    0},     /* Token Ring */
    {   "PMAGD   ",	"fb",     0,      1,    'C',    0},     /* SFBP-8,-32,-32Z */
    {   "PMAGD-AA",	"fb",     0,      1,    'C',    0},     /* SFBP-8 */
    {   "PMAGD-BA",	"fb",     0,      1,    'C',    0},     /* SFBP-32 */

#ifdef mips
    {	"PMAG-BA ",	"fb",	  0,	  0,	'C',	0},	/* CFB */
    {	"PMAG-AA ",	"fb",	  0,	  0,	'C',	0},	/* MFB */
    {	"PMABV-AA",	"vba",	  1,	  1,	'A',	xviaconf}, /* VME */
    {   "CITCA-AA",     "ci",     0,      1,    'A',    tcciconf},/* CI */
#endif /* mips */

    /*
     * Do not delete any table entries above this line or your system
     * will not configure properly.
     *
     * Add any new controllers or devices here.
     * Remember, the module name must be blank padded to 8 bytes.
     */

    /*
%%% Used by mktcdata as placemarker for automatic installation
    */
    

    /*
     * Do not delete this null entry, which terminates the table or your
     * system will not configure properly.
     */
    {	"",		""	}	/* Null terminator in the table */
};
