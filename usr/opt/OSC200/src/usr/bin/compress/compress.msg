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
$ (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ @(#)$RCSfile: compress.msg,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/06/10 16:08:09 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2
$

$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS:
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ compress.msg	1.4  com/cmd/files,3.1,9013 3/12/90 15:36:00
$
$ messages for compress
$
$set MS_COMPRESS
$		GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate compress -- this is the command name.
$ Do not translate %o %s %1$s %2$s etc.
$ These are output formats for file names, directory
$ names, permission codes, command names, and the like,
$ depending on their context within each message. 
$ The program replaces %o,%1$o,etc with an octal number.
$ The program replaces %d,%1$d,etc with a decimal number.
$ the program replaces %.2f with a fixed-point number.
$ The program replaces %.1f%% with a percentage.
$ The program replaces %s,%1$s,etc with a string of 
$ characters.  

$quote "

CPUSAGE "Usage: compress [-CVcdfnqv] [-b Maxbits] [File ...]\n"
UNCPUSAGE "Usage: %s [-cdfnqv] [File ...]\n"
ZCATUSAGE "Usage: %s [-n] [File ...]\n"
$
MISSMAXBT "compress: Invalid number of bits specified.\n"
$
UNKFLG "The %c flag is not recognized.\n"
$
NOTCOMP "The %s file is not compressed.\n"
$ Do not translate uncompress.
$
BITSWRONG "File %1$s was compressed with %2$d bits. Only %3$d bits are allowed.\n"
$
ALREADYZ "compress: %s already has a .Z suffix. It is not changed.\n"
$ Do not translate .Z
$ %s is replaced with a file name.
$
NAME2LONG "compress: File name %s is too long to add the .Z suffix.\n"
$ Do not translate .Z
$ %s is replaced with a file name.
$
ALRDYEXST ""
$ This is a null line. Do not translate.
$
WSHOVRWRIT "Do you wish to write over the existing file %s? "
$ Do not put a new line at the end. Leave a blank at the end.
$
NOTOVRWRIT "\tThe file is not written over.\n"
$
INNOTCOMP "The standard input is not compressed.\n"
$
STDINBITS "The standard input was compressed with %2$d bits. Only %3$d bits are allowed.\n"
$
COMPRESSION "Compression: "
$  the program inserts a calculated percentage after the (:) colon.
$ Leave a space after the colon. Do not put a new line at the end.
$
NOTREG "%s: This is not a regular file. It is not changed.\n"
$ This message is preceded by the filename on the same line.
$ Do not put a new line at the end.
$
HASLINKS " This file already has %d links. It is not changed."
$ This message is preceded by the filename on the same line.
$ Do not put a new line at the end.
$
UNCHANGE " This file is not changed; compression does not save space."
$ This message is preceded by the filename on the same line.
$ Do not put a new line at the end.
$
REPLACEW " This file is replaced with %s."
$ This message is preceded by the filename on the same line.
$ Do not put a new line at the end.
$
CORRUPTIN "uncompress: The input file contains bad data; SIGSEGV signal received.\n"
$ Do not translate SIGSEGV
$
OPTS "Options: "
$ Leave a space after the colon.
