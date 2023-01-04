$ 
$ *****************************************************************
$ *                                                               *
$ *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
$ *                                                               *
$ *   All Rights Reserved.  Unpublished rights  reserved  under   *
$ *   the copyright laws of the United States.                    *
$ *                                                               *
$ *   The software contained on this media  is  proprietary  to   *
$ *   and  embodies  the  confidential  technology  of  Digital   *
$ *   Equipment Corporation.  Possession, use,  duplication  or   *
$ *   dissemination of the software and media is authorized only  *
$ *   pursuant to a valid written license from Digital Equipment  *
$ *   Corporation.                                                *
$ *                                                               *
$ *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
$ *   by the U.S. Government is subject to restrictions  as  set  *
$ *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
$ *   or  in  FAR 52.227-19, as applicable.                       *
$ *                                                               *
$ *****************************************************************
$ 
$ 
$ HISTORY
$ 
$ 
$ @(#)$RCSfile: pacl.msg,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:12:27 $
$ 
$ @OSF_COPYRIGHT@
$ 
$ HISTORY
$ $OSF_Log:	pacl.msg,v $
$ Revision 1.1.1.1  92/05/12  01:45:00  devrcs
$  *** OSF1_1B29 version ***
$ 
$ Revision 1.1.2.3  1992/04/05  18:19:58  marquard
$ 	paclif POSIX ACL interface program.
$ 	[1992/04/05  11:52:50  marquard]
$
$ $OSF_EndLog$
$
$
$ Copyright (c) 1988-91 SecureWare, Inc.
$   All rights reserved.
$
$ @(#)pacl_msg.h
$
$quote "
$set MS_PACL

CURSCR_1	"Exiting due to find_terminal failure.\n"
CURSCR_2	"terminal type \'%s\'\n"
CURSCR_3	"Exiting due to inability to get terminal state\n"
CURSCR_4	"Exiting due to inability to set terminal state\n"
CURSCR_5	"Could not initialize window on screen.\n"
CURSCR_6	"Terminal reports eight bits but doesn't have km terminfo capability\n"
CURSCR_7	 "TERM=%s"
CURSCR_8	 "Your terminal type is not specified.\n"
CURSCR_9	 "Cannot find terminal type \'%s\' in the terminal database \'%s\'.\n"
CURSCR_10	 "Please enter your terminal type now (q to exit): "
GETSCR_1	 "ascii,help.keys"
INITST_1	 "Field %d had prompt \'%s\' but was not FLD_PROMPT.\n"
MSGSCR_1	 "Internal error, no messages for index %d"
MSGSCR_2	 "NULL"
MSGSCR_3	 "first message is '%s'"
AIF_MAIN_1	 "Setting up privileges"
AIF_MAIN_2	 "%s: insufficient privileges\n"
AIF_MAIN_3	 "Privileges set up"
AIF_MAIN_6	 "Help screens print"
AIF_MAIN_7	 "logging level = <%d>"
AIF_MAIN_8	 "Screen printing only"
AIF_MAIN_9	 "Normal signal handling"
AIF_MAIN_10	 "Bad argument switch: <%c>"
AIF_MAIN_11	 "%s does not take any arguments.\n"
PRNTDSPLY_1	 "Unable to fork pager process, errno %d\n"
PRNTDSPLY_2	 "File %s could not be opened for reading.\n"
PRNTDSPLY_3	 "Unable to create pipe between program and pager\n"
PRNTDSPLY_4	 "Unable to create pager process\n"
PRNTDSPLY_5	 "Unable to create program process\n"
PRNTDSPLY_6	 "Unable to open file %s\n"
PRNTDSPLY_7	 "Unable to create process for print command\n"
PRNTDSPLY_8	 "Unable to create pipe between program and print command\n"
PRNTDSPLY_10	 "ps_child parent: executing pager %s\n"
PRNTDSPLY_11	 "Cannot execute pager %s\n"
PRNTDSPLY_12	 "do_command, executing %s\n"
PRNTDSPLY_13	 "\targument %d: %s\n"
PRNTDSPLY_14	 "do_command: could not execute \'%s\'.\n"
SIGNALS_1	 "Program terminated due to user request\n"
SIGNALS_2	 "Premature program termination: code %d\n"
SIGNALS_3	 "Premature program termination: file size exceeded.\n"
SIGNALS_4	 "Unexpected program error: signal code %d\n"
TRAVERSE_1	 "Returned from getscreen without choosing\n"
TRAVERSE_2	 "item %d, changed %s\n"
TRAVERSE_3	 "yes"
TRAVERSE_4	 "no"
VARS_1	 	"am"
VARS_2	 	"pm"
VARS_3	 	"Group: %s"
VARS_4	 	"GID: %d"
VARS_5	 	"User: %s"
VARS_6	 	"UID: %d"
VARS_7	 	"Dev: %s"
