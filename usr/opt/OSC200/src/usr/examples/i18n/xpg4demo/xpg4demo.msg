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
$ HISTORY
$ 
$ @(#)$RCSfile: xpg4demo.msg,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/25 22:04:11 $
$ 
$ /*
$  * XPG4 demo program message catalogue.
$  *
$  */

$quote "

$set MSGError

E_COM_EXISTBADGE	"Employee entry for badge number %ld already exists"
E_COM_FINDBADGE		"Cannot find badge number %ld"
E_COM_INPUT		"Cannot input"
E_COM_MODIFY		"Data file contains no records to modify"
E_COM_NOENT		"Data file contains no records to display"
E_COM_NOTDEL		"Data file contains no records to delete"
E_COM_UNKNOWN		"Unknown command"
E_DB_NOMEMORY		"employee data"
E_DB_WRITEOPEN		"Cannot open %s for output"
E_MES_ABORT		". Aborting...\n"
E_MES_FATAL2		". Exiting...\n"
E_MES_NOMEMORY		"No memory for %s"
E_SCR_INV_BADGE		"Invalid badge number"
E_SCR_INV_CC		"Invalid Cost Center"
E_SCR_INV_JOD		"Invalid date"
E_SCR_2LONG_1STNM	"First name too long"
E_SCR_NOMEMORY		"screen structure"
E_SCR_2LONG_SURNM	"Surname too long"
E_SCR_UNKOWN_TERM	"Unknown terminal type. Try \"ansi\""
E_SCR_TERMCAP_OPEN	"Cannot open termcap/terminfo database"
E_SCR_TERMNOMEM		"termcap buffer"
E_SCR_UNDEFTERM		"Terminal type %s not defined"
E_UTL_NOMEMORY		"message string"
E_XPG_DBREAD		"Database is readonly"
E_XPG_INIT_COM		"Cannot initialize command environment"
E_XPG_INIT_SCR		"Cannot initialize screen"
E_XPG_NEWFILE		"New data file created"
E_XPG_NOMEMORY		"No memory. Exiting...\n"
E_XPG_READ		"Cannot read employee data file %s"
E_XPG_USAGE		"Usage: %s [database]\n"
E_XPG_WRITE		"Write error!"

$set MSGInfo

I_COM_NEWEMP		"New employee"
I_COM_YN_DELETE		"Do you want to delete this record?"
I_COM_YN_MODIFY		"Do you want to modify this record?"
I_COM_YN_REPLACE	"Are these the changes you want to make?"
I_COM_ASK_MOD_FMT	"Enter the %s for the record to be modified [%ld]: "
I_COM_ASK_DEL_FMT	"Enter the %s for the record to be deleted [%ld]: "
I_COM_DISP_LIST_FMT	"%6ld  %20S %-30S %3S %10s\n"
$ I_COM_MOD_BADGE_FMT	"Do you want to modify the record of "
I_DB_SURNAME1ST		"n"
$ 
$ I_MES_YN has to be consistent with YESSTR and NOSTR of the locale.
$ YESEXPR and NOEXPR should be used if they are available.
$ 
I_MES_YN		" (yes/no) [yes]: "
I_XPG_CONTINUE		"Press Return to continue. "
I_SCR_PROMPT_BADGE_DEF	"%s [%ld]? "
I_SCR_PROMPT_BADGE	"%s? "
I_SCR_PROMPT_NAME_DEF	"%s [%S]? "
I_SCR_PROMPT_NAME	"%s? "
I_SCR_PROMPT_CC_DEF	"%s [%S]? "
I_SCR_PROMPT_CC		"%s? "
I_SCR_PROMPT_DOJ_DEF	"%s [%s]? "
I_SCR_PROMPT_DOJ	"%s? "
I_SCR_IN_DATE_FMT	"%2$d/%3$d/%1$d"

$set MSGString

$ 
$ One-character commands.
$ 
S_COM_CREATE		"c"
S_COM_DELETE		"d"
S_COM_EXIT		"e"
S_COM_LIST		"l"
S_COM_MODIFY		"m"
S_COM_LIST_TITLE	" Badge         Name                  Surname                CC      DOJ\n"
S_COM_LIST_LINE		"-----------------------------------------------------------------------------\n"
S_SCR_BADGE		"Employee Badge Number"
S_SCR_CC		"Cost Center"
S_SCR_DOJ		"Date of Joining"
$
$ This prompt message must be consistent with I_SCR_IN_DATE_FMT.
$ 
S_SCR_DOJ_FMT		"Date of Joining (MM/DD/YY)"
S_SCR_MENU		"1-Modify 2-Create 3-Delete 4-List 5-Exit ?",
S_SCR_NAME		"First (Given) Name"
$ 
$ If surname comes before first name, "y" should be specified.
$ 
S_SCR_SNAME1ST		"n"
S_SCR_SURNAME		"Surname"
S_SCR_PROMPT		""
$ 
$ date format used for List
$ 
S_DATE_FMT		"%x"
$ 
$ date format used for showing an employee in full screen.
$ 
S_SCR_DATE_FMT		"%x"
