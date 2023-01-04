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
 *	@(#)$RCSfile: std_defs.h,v $ $Revision: 4.3 $ (DEC) $Date: 1991/09/21 17:12:37 $
 */ 
/*
********************************************************************************
**                                                                            **
**                           Open Software Foundation 			      **
**                             All rights reserved                            **
**    No part of this program may be photocopied, reproduced or translated    **
**    to another programming language or natural language without prior       **
**    written consent of the Open Software Foundation.                        **
**                                                                            **
********************************************************************************
**
**    Description:
**	This header file is for the sandbox programs.  It contains the common
**	includes and defines.
**
**    written by:
**                   Randy J. Barbano
**             HP Lake Stevens Instrument Division
**                   Lake Stevens, WA 
**                   February 19, 1986
**
**    known limitations/defects:
**
**    copyright
**
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
**
**    modification history:
**
 * OSF/1 Release 1.0
**



                                                                              */

/*******************************************************************************

                         INCLUDES					     

*******************************************************************************/

#  include <sys/types.h>
#  include <sys/dir.h>
#  include <sys/file.h>
#  include <string.h>
#  include <stdio.h>


/*******************************************************************************

                         DEFINES					        

*******************************************************************************/

	/* integers */

#ifndef TRUE
#  define  TRUE			1
#endif

#ifndef FALSE
#  define  FALSE		0
#endif

#  define  ERROR		-1
#  define  OK			0
#  define  CHILD		0

#  define  FIRST_FIELD		0
#  define  SEC_FIELD		1
#  define  THIRD_FIELD		2
#  define  FOURTH_FIELD		3
#  define  FIRST_ARG		1
#  define  SEC_ARG		2
#  define  THIRD_ARG		3
#  define  FOURTH_ARG		4
#  define  NO_ARGS		0
#  define  ONE_ARG		1
#  define  TWO_ARGS		2
#  define  THREE_ARGS		3
#  define  FOUR_ARGS		4

#  define  FIRST             	1
#  define  NEXT             	1
#  define  PREV             	1

#  define  MAX_VAR_LEN          14
#  define  MAX_FILES_OPEN       15
#  define  NAME_LEN             50
#  define  SM_SCREEN_LEN        80                           /* narrow screen */
#ifndef PATH_LEN
#  define  PATH_LEN             1024
#endif
#  define  SCREEN_LEN           127                            /* wide screen */
#  define  STRING_LEN           256

	/* chars */

#  define  NUL                  '\0'
#  define  SPACE                ' '                    /* string with a space */
#  define  TAB                  '\t'
#  define  DASH           	'-'
#  define  RETURN_CH      	'\n'
#  define  COLON          	':'
#  define  ESC            	''          /* additional character entries */
#  define  CTRL_D         	''
#  define  BACKSPACE      	'\b'
#  define  SLASH      		'/'
#  define  PERIOD      		'.'
#  define  COMMA      		','
#  define  AT_SIGN     		'@'
#  define  STAR        		'*'
#  define  Y_CH			'y'
#  define  N_CH			'n'


	/* strings */

#  define  EMPTY_STRING         ""               /* empty string with no <cr> */
#  define  WHITESPACE           " \t\n"             /* white space characters */
#  define  STAR_ST		"*"		/* universal matching pattern */
#  define  CR_STRING            "\n"                 /* string with only <cr> */
#  define  YES            	"y"
#  define  NO             	"n"
#  define  SYS			"SYSTEM"	       /* for system error id */

#  define  READ                 "r"				      /* open */
#  define  WRITE                "w"			    /* create or open */
#  define  APPEND               "a" 	 /* create or open for writing at eof */
#  define  O_UPDATE             "r+"          /* open for reading and writing */
#  define  C_UPDATE             "w+"  /* create or open for reading & writing */
#  define  A_UPDATE             "a+"
			     /* create or open for reading and writing at eof */

	/* symbols */

#  define  AND                  &&
#  define  and                  &&
#  define  OR                   ||
#  define  or                   ||
#  define  NOT                  !
#  define  not                  !
#  define  MOD                  %
#  define  mod                  %

	/* macros */

#  define  max(A,B)             ((A) > (B) ? (A) : (B))   /* maximum function */
#  define  min(A,B)             ((A) < (B) ? (A) : (B))   /* minimum function */
#  define  streq(A,B)           (strcmp ((A),(B)) == 0)
						     /* are two strings equal */

	/* for ODE commands */

			/* misc needs */

#  define  BCSSET       "BCSSET"                      /* environment variable */
#  define  BCS_SET_L    8			  /* length of BCS_SET define */
#  define  DEF_BUILD    "latest"                /* default build to submit to */
#  define  DEF_SETINFO  "LATEST"               /* default branch to submit to */
#  define  EDITOR	"EDITOR"		      /* environment variable */
#  define  REPLACE      "replace"
#  define  SANDBOX      "SANDBOX"                     /* environment variable */
#  define  SB_BASE      "sandbox_base"

			/* location and/or names of files */

#  define  BCSCONFIG	".BCSconfig"		     /* file with config info */
#  define  BCSLOCK	".BCSlock"		  /* file with bcs lock in it */
#  define  BCSLOG	".BCSlog-"		   /* file with bcs log in it */
#  define  BCSPATH      ".BCSpath-"               /* file with bcs path in it */
#  define  BCS_SET      ".BCSset-"                /* file with bcs co's in it */
#  define  BUILD_LOC    "/project/osc/build"   /* directory to builds, latest */
#  define  LOCAL_RC	"rc_files/local"         /* location of local rc file */
#  define  LOCAL_T_RC   "rc_files/local.tmpl"          /* files to know about */
#  define  HOLD_FILE	"logs/bsubmit.hold"	     /* files held by bsubmit */
#  define  LOCK_HOLD	"logs/lock_hold"     /* dir to lock bsubmit hold file */
#  define  LOCK_LOGS	"logs/lock_logs"          /* dir to lock bsubmit logs */
#  define  MKCONF_LINK  "src/Makeconf"            /* marker for top of source */
#  define  SHARED_RC	"rc_files/shared"       /* location of shared rc file */
#  define  SHARED_T_RC  "rc_files/shared.tmpl"
#  define  SANDBOXRC    ".sandboxrc"		 /* files to know location of */
#  define  SET_RC       "rc_files/sets"           /* location of sets rc file */
#  define  SUBLOG	"logs/bsubmit.log"	  /* permenent build log file */

			/* directories */

#  define  LINK_DIR     "link"
#  define  EXP_DIR      "export"
#  define  OBJ_DIR      "obj"
#  define  RC_DIR       "rc_files"
#  define  SRC_DIR      "src"                          /* sandbox directories */
#  define  TOOL_DIR     "tools"

			/* key words */

#  define  BASE         "base"
#  define  DEFAULT      "default"
#  define  SB           "sb"
#  define  SET_KEY      "set"                     /* key word in sets rc file */
#  define  SOURCE_COVER	"source_cover"        	       /* key word in rc file */
#  define  SOURCE_LINE  "source_base"         	       /* key word in rc file */
#  define  DEFUNCT_MARK "defunct"	   /* "Rev" to look for in .BCSconfig */


			/* command */

#  define  BCS		"bcs"
#  define  BCI		"bci"
#  define  BCO		"bco"
#  define  BLOG		"blog"
#  define  BMERGE	"bmerge"
#  define  BSTAT	"bstat"

			/* command line options */

#  define  ARGS_OP	""
#  define  AUTO_OP	"-auto"
#  define  AUTO_OUT_OP	"-autooutdate"
#  define  BACK_OP	"-back"
#  define  C_OP		"-c"
#  define  COPY_OP      "-copy"
#  define  DEBUG_OP     "-debug"
#  define  DEF_OP	"-def"
#  define  DEFUNCT_OP   "-defunct"
#  define  DIR_OP	"-dir"
#  define  ECHO_OP	"echo"
#  define  I_OP		"-i"
#  define  INFO_OP	"-info"
#  define  L_OP		"-L"
#  define  LIST_OP	"-list"
#  define  M_OP		"-m"
#  define  NEWC_OP	"-newconfig"
#  define  NEWP_OP	"-newpath"
#  define  NOLOCK_OP	"-nolock"
#  define  NOLOG_OP	"-nolog"
#  define  NOSH_OP      "-nosh"
#  define  NOWRITE_OP   "-nowrite"
#  define  PATH_OP	"-path"
#  define  R_OP		"-R"
#  define  RC_OP	"-rc"
#  define  RCONLY_OP	"-rconly"
#  define  REV_OP	"-rev"
#  define  RM_OP        "-rm"
#  define  Q_OP		"-q"
#  define  QUIET_OP	"-quiet"
#  define  SB_OP        "-sb"
#  define  SBRC_OP	"-sb_rc"
#  define  SET_OP       "-set"
#  define  OUTDATE_OP	"-o"
#  define  SUBDIR_OP    "-subdir"
#  define  FUNLOCK_OP   "-u"
#  define  UNDO_OP	"-undo"
#  define  UNLOCK_OP	"-unlock"
#  define  USAGE_OP	"-usage"
#  define  VC_OP	"-V"
#  define  V_OP         "-v"
#  define  WRITE_OP	"-okwrite"
#  define  XLOG_OP	"-xlog"


/*******************************************************************************

		         TYPEDEFS

*******************************************************************************/

typedef    int   BOOLEAN;                    /* distinguishes type of integer */
                            

/*******************************************************************************

		         RETURN VALUES OF FUNCTIONS

*******************************************************************************/

  FILE        *	fopen ();	 /* standard function returns pointer to file */
  DIR         *	opendir (); /* standard function returns pointer to directory */

  char        *	calloc (),     /* standard functions return pointers to chars */
	      *	malloc (),
	      *	realloc (),
	      *	gets (),
	      *	getenv (),
	      *	mktemp (),
	      * nxtarg (),
	      * concat ();

  void  	perror (),   /* standard functions which do not return values */
		exit ();

  unsigned long	sleep ();
