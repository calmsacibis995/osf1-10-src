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
static char *rcsid = "@(#)$RCSfile: eisa_option_data.c,v $ $Revision: 1.1.6.6 $ (DEC) $Date: 1993/11/02 15:28:13 $";
#endif

/***************************************************************************/
/*                                                                         */
/* MODULE NAME: eisa_option_data.c					   */
/* 									   */ 
/* LOCATION:	.../src/kernel/data					   */
/* 									   */ 
/* DESCRIPTION:								   */
/*		Contains data table for EISA bu options. This table maps   */
/*		option board product id name with device or controller     */
/*		name in the config file. The eisa_option structure is	   */
/*		defined in io/dec/eisa/eisa.h.				   */
/* 									   */ 
/***************************************************************************/

#include	<io/dec/eisa/eisa.h>


struct	eisa_option	eisa_option[] =
{
    /* board	  function    driver  intr_b4 itr_aft	     adpt	*/
    /* id	    name       name    probe   attach type  config	*/
    /* ------	 ------------ ------  ------- ------- ----  ------	*/

    { "CPQ3011", "",  	       "vga",    0,	 1,   'C',    0},    /* QVISION */
    { "CPQ3111", "",  	       "vga",    0,	 1,   'C',    0},    /* QVISION "fir" */
    { "DEC4220", "NET,ETH",    "ln",     0,	 1,   'C',    0},    /* DEC_LANCE */
    { "ADP0001", "AHA1740",    "aha",    0,	 1,   'C',    0},    /* ADP1740A SCSI */
    { "ADP0002", "AHA1740",    "aha",    0,	 1,   'C',    0},    /* ADP1742A SCSI */
    { "ADP0002", "MSD,FPYCTL", "fdi",    0,	 1,   'C',    0},    /* ADP1742A FLPY */
    { "DEC3001", "",	       "fta",    0,	 1,   'C',    0},    /* DEC_FDDI */
    { "DEC3002", "",	       "fta",    0,	 1,   'C',    0},    /* DEC_FDDI */
    { "DEC2500", "", 	       "envram", 0,	 0,   'C',    0},    /* EISA NVRAM */
    { "ISA1010", "COM,1",      "ace",    0,	 1,   'C',    0},    /* COMM PORTS */
    { "ISA1010", "COM,2",      "ace",    0,	 1,   'C',    0},    /* COMM PORTS */
    { "ISA1010", "PAR",        "ace",    0,	 1,   'C',    0},    /* COMM PORTS */

    /*
     * Do not delete any table entries above this line or your system
     * will not configure properly.
     *
     * Add any new controllers or devices here.
     * Remember, the module name must be blank padded to 7 bytes.
     */

    /*
%%% Used by mkeisadata as placemarker for automatic installation
    */
    

    /*
     * Do not delete this null entry, which terminates the table or your
     * system will not configure properly.
     */
    {	"",		""	}	/* Null terminator in the table */

};
