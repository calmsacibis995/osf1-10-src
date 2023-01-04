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
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: UilCmdDef.h,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/07 00:32:27 $ */

/*
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */

/*
**++
**  FACILITY:
**
**      User Interface Language Compiler (UIL)
**
**  ABSTRACT:
**
**      This include file defines the interface to the UIL command line
**	processing.
**
**--
**/

#ifndef UilCmdDef_h
#define UilCmdDef_h

/*
**
**  INCLUDE FILES
**
**/


typedef struct  
{
    char	    *ac_source_file;
    char	    *ac_resource_file;
    char	    *ac_listing_file;
    unsigned int    include_dir_count;	    /* number of directories in	    */
					    /* ac_include_dir array	    */
    char	    **ac_include_dir;    /* pointer to array of	    */
					    /* directories to search for    */
					    /* includes in		    */
    unsigned	    v_listing_file: 1;
    unsigned	    v_resource_file: 1;
    unsigned	    v_show_machine_code: 1;
    unsigned	    v_report_info_msg: 1;
    unsigned	    v_report_warn_msg: 1;
    unsigned	    v_parse_tree: 1;	    /* generate parse tree	    */
    unsigned        v_issue_summary: 1;
    
    unsigned int    status_update_delay;    /* Number of times a status	    */
					    /* point is passed before	    */
					    /* calling statusCB routine	    */
					    /* 0 means called every time    */
    Uil_continue_type	(*message_cb)();
    char		*message_data;
    Uil_continue_type	(*status_cb)();
    char		*status_data;
    unsigned	    	v_use_setlocale: 1;    /* Allow localized strings */
    char		*ac_database;
    unsigned 		v_database: 1;
} cmd_command_line_type;

#endif /* UilCmdDef_h */
/* DON'T ADD STUFF AFTER THIS #endif */
