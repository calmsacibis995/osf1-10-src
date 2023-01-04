
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
$ HISTORY
 
$
$ @(#)$RCSfile: man.msg,v $ $Revision: 4.2.3.5 $ (DEC) $Date: 1992/12/11 14:50:36 $
$
$
$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0

$ 	@(#)man.msg\t1.6  com/cmd/man,3.1,9021 3/4/90 17:25:23
$ 
$ 	COMPONENT_NAME: (CMDMAN) commands that allow users to read online
$ 	                         documentation
$ 
$ 	FUNCTIONS: none
$ 
$ 	ORIGINS: 27
$ 
$ 	IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ 	combined with the aggregated modules for this product)
$ 	OBJECT CODE ONLY SOURCE MATERIALS
$ 	(C) COPYRIGHT International Business Machines Corp. 1989
$ 	All Rights Reserved
$ 
$ 	US Government Users Restricted Rights - Use, duplication or
$ 	disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ 	[90/06/19  21:17:17  lehotsky]
$ 

$
$quote "         define quote character.
$len
$set MS_man        Define initial set#
$ $error numbers will be 3xx

$ Do not translate man, %s, etc.

M_MSG_0              "man: %s: The path is missing.\n"
M_MSG_1              "man: The man page name is missing.\n"	/* QAR #7819 */
M_MSG_2              "man: %s: The path is missing.\n"
M_MSG_3              "There is not an entry for %s.\n"
M_MSG_4              "There is not an entry for %1$s in section %2$c.\n"
M_MSG_5              "Reformatting page. Please wait ..."
M_MSG_6              "man: The program has ended abnormally.\n"
M_MSG_7              " completed\n"
M_MSG_8              "man: A parameter is missing.\n"
M_MSG_9              "%s: Cannot find matching entry.\n"
M_MSG_10             "%s: A parameter is missing.\n"		/* QAR #6239 */
M_MSG_11             "%s: %s is not found.\n"			/* QAR #1383 */
M_MSG_12             "Usage: %s [-] [-M Path] [-P path] -t"	/* QAR #7887 */
$ This and the rest of the messages make up the usage statement.
$ Ensure that it matches your publications.
M_MSG_13             "[Section] Name ...\n"
M_MSG_14             "\t%s -k Keyword ... \n"
$ Do not translate -k
M_MSG_15             "\t%s -f File ... \n"
$ Do not translate -f
M_MSG_16             "Usage: %s "
M_MSG_17             "[-M path] keyword ...\n"
M_MSG_18	     "man: Nroff/troff must be installed in order\n\
to use formatted man pages.\n"
M_MSG_19	     "man: cannot read directory %s.\n"
$
$ Do not translate: whatis, catman, -w, -M				
NO_WHATIS_1	     "%s: No whatis database was found in <%s>.\n"
NO_WHATIS_2	     "\tRun:\tcatman -w\n\
\tor:\tcatman -M %s -w\n"
NO_WHATIS_3	     "\tRun:\tcatman -M %s -w\n"
