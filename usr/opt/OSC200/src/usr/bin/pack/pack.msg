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
$ @(#)$RCSfile: pack.msg,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 18:54:13 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2

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
$ "pack.msg	1.7  com/cmd/files,3.1,9013 12/1/89 15:10:38"
$
$quote "
$set MS_PACK
$		GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate pack: -- this is the command name.
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
$ 
READERR ": There is a read error.\n"
$ This message is followed by the message FILUNCHN. 
$ 
READERR1 ": There is a read error.\n"
$ This message is followed by the message FILUNCHN. 
$ 
WRITERR ".z: There is a write error while creating the .z file.\n"
$ This message is followed by the message FILUNCHN. 
$ 
TRIVIAL ": There is no saving in file space.\n"
$ This message is followed by the message FILUNCHN. 
$ 
LVLCNT ": The file is too large or too complex to pack.\n"
$ This message is followed by the message FILUNCHN. 
$ 
NOSAVE ": Packing this file saves no storage space.\n"
$ This message is followed by the message FILUNCHN. 
$ 
PCKDMSG ": The specified file is already packed.\n"
$ 
FILNAMLEN ": The file name is too long.\n"
$ 
CANTOPEN ": Cannot open the specified file.\n"
$ 
PACKDIR ": Cannot pack a directory.\n"
$ 
LINKCNT ": The specified file has links; it cannot be packed.\n"
$ 
ZEXISTS ".z: A file by this name already exists.\n"
$ 
CANTCREAT ".z: Cannot create the specified file.\n"
$ 
CHMODMSG "Cannot change the mode to %o.\n"
$ 
CANTUNLNK "%1$s: Cannot unlink %2$s.\n"
$ 
COMPRESS ": The file is reduced in size %.1f%%.\n"
$ The program replaces %.1f%% with a percentage.
$ 
SIZEMSG "\tThe file changed from %1$ld to %2$ld bytes.\n"
$ 
HUFFLVLS "\tThe Huffman tree has %d levels below the root.\n"
$ Do not translate "Huffman".  It is the name of a file building technigue.
$ 
DISTINCT "\tThere are %d distinct bytes in input.\n"
$ %d is replaced by the number of bytes in the unpacked file. 
DICTOVRHD "\tThe dictionary overhead is %ld bytes.\n"
$ %d is replaced by the number of storage medium bytes occupied by 
$ the current system dictionary. 
EFFENTROPY "\tThe effective entropy is %.2f bits per byte.\n"
$ effective entropy means the average number of bits per byte for this
$ current packed message. 
ASYMENTROPY "\tThe asymptotic entropy is %.2f bits per byte.\n"
$ asymptotic entropy means the average number of bits per byte for this
$ current group of packed messages.
FILUNCHG "\tThe file is unchanged.\n"

USAGE	"Usage: pack [-f] [-] file...\n"
