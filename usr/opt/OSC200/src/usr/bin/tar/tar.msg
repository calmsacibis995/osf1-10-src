 
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
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0

$ @(#)$RCSfile: tar.msg,v $ $Revision: 4.2.4.5 $ (DEC) $Date: 1992/12/11 16:00:33 $

$ COMPONENT_NAME: (CMDARCH) archive files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$ OBJECT CODE ONLY SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ tar.msg	1.8  com/cmd/arch,3.1,9021 3/21/90 21:25:32
$
$quote "
$set MS_TAR
$               GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate tar -- this is the command name.
$ Do not translate %o %s  %1$s %2$s etc.
$ These are output formats for file names, directory
$ names, permission codes, command names, and the like,
$ depending on their context within each message.
$ The program replaces %o,%1$o,etc with an octal number.
$ The program replaces %d,%1$d,etc with a decimal number.
$ the program replaces %.2f with a fixed-point number.
$ the program replaces %u with an unsigned integer.
$ The program replaces %.1f%% with a percentage.
$ The program replaces %s,%1$s,etc with a string of
$ characters.

BLKS "a %1$s %2$ld blocks.\n"
$ %1$s is replaced by a file name.
$ %2$ld is replaced by a whole number.
$ do not translate "a" -- it means "archive" by tar convention.

BLKSIZE "tar: blocksize = %d\n"
$ %d is replaced by a whole number.

BLKSON "tar: %1$ld blocks on %2$s\n"
$ %2$s is replaced by a file/device name.
$ %1$ld is replaced by a whole number.

EBLKSIZE "tar: tape blocksize error\n"

EBLKBIG "tar: Not enough memory available for blocksize %d.\n"

EBSIZE "tar: %s is not a valid numeric block size.\n"
$ %s is replaced by a file name.

ECHBACK "tar: cannot change back: \n"
$ The program inserts a standard system message
$ from the "perror" subroutine after the : character.

ECHDIR "tar: cannot change directories to \n"
$ The program inserts a standard system message
$ from the "perror" subroutine after the word to.

ECSUMC "tar: directory checksum error on media (%1$d != %2$d)\n"
$ do not translate "checksum"

EDIRREAD "tar: cannot read directory %s.\n"

EEOT "tar: reached end of tape.\n"
$ The tar command cannot prompt for a new\n\
$ tape when running in background. Run the tar command\n\
$ in the foreground.\n"

EFFLAG "tar: a file name must be specified with the 'f' option.\n"

EFSIZE "tar: %s: file changed size.\n"

EFLINK "tar: cannot link %1$s to %2$s: "
$ The program inserts a standard system message
$ from the "perror" subroutine after the : character.

ELPREFIX "link no longer exists since prefix was stripped.\n"

ELINKS "tar: missing links to %s\n"

ELONGFN "tar: %s: file name is too long.\n"

ELONGFN2 "    (use 'n' option)\n"

ELONGSL "tar: %s: symbolic link name is too long.\n"

EMEM "tar: out of memory.\n"

EMKNOD "%s: mknod failed:"
$ Do not translate mknod. It is a system call.

ENOARCH "tar: %s could not be archived\n"

ENOTSPEC "tar: warning: %s: not a character or block special device.\n"

EOT "\007tar: End of device.  Mount next volume on %s and press return.\n"
$ Do not translate \007tar:
$ %s is replaced by a tape unit identifier.

EREAD "tar: Read error on "
$ The program inserts a standard system message
$ from the "perror" subroutine after the word on.

EREADSL "tar: cannot read symbolic link "
$ The program inserts a standard system message
$ from the "perror" subroutine after the word link.

ERFLAG "tar: a file name must be specified with the 'R' option\n"

ERTXFLAG "tar: -R not valid with -t or -x\n"

ESETTIME "tar: cannot set the time on %s: "
$ %s is replaced by a file name.
$ The program inserts a standard system message
$ from the "perror" subroutine after the : character.

ESTDOUT "tar: can only create standard output archives.\n"

ESYMFAIL "tar: %s: symbolic link failed: "
$ %s is replaced by a file name.
$ The program inserts a standard system message
$ from the "perror" subroutine after the : character.

ETCREATE "tar: cannot create %s: "
$ Do not translate "create" on the line above.  It is
$ the name of a system call.  Translate "create"
$ on the line below -- it means "to make".

ETCRTMP "tar: cannot create temporary file %s.\n"

ETNOTAPESIZE "tar: tape size must be specified with 'S' option.\n"

ETNOBSIZE "tar: block size must be specified with 'b' option.\n"

ETNOPREFIX "tar: strip prefix must be specified with the 'P' option.\n"

ETOPEN "tar: cannot open "

ETREAD "tar: An error occurred while reading from the media.\n"

ETWREOF "tar: write error: reached end-of-file before expected.\n"

ETRDEOF "tar: read error: reached end-of-file before expected.\n"

ETWRITE "tar: An error occurred while writing to the storage media.\n"

ETCLOSE "tar: cannot close storage media.\n"

EVCLOSE "tar: cannot close volume.\n"

EXWRITE "tar: %s: extract error: cannot write data \n"

EXWREOF "tar: %s: extract error: reached end-of-file before expected\n"

EXCLOSE "tar: %s: extract error: cannot close file\n"

LINKTO "a %s linked to %s\n"
$ the letter  a indicates archive.  do not translate.

LINKEDTO "x %s linked to %s\n"
$ the letter  x indicates extract.  do not translate.

LINKED " linked to %.*s\n"
$ the program inserts a file name in front of this message.

PROCEED "tar: Proceeding with device %s.\n"
$ this is an informational message that the user
$ sees after responding to message EOT

SLINKTO "a %s symbolic link to %.*s.\n"
$ the letter  a indicates archive.  do not translate.

SLINKED " symbolic link to %.*s.\n"
$ the program inserts a file name in front of this message.

USAGE1 "Usage: tar [-]{c|r|t|u|x}[bfhilmnopsvwBLFPRSU0-9] [blocking] [tarfile]"

USAGE2 " [prefix]\n\t\t\t\t [listfile] [feet[@density]|blocksb]\n"

USAGE3 "\t\t\t\t\t    [-C directory] [-e except] file ...\n"

ESTAT "tar: Cannot get information about file %s.\n"

XSTAT "x %1$s, %2$ld bytes, %3$ld tape blocks.\n"
$ appears as message when using the -v flag.
$ the letter  x indicates extract.  do not translate.

XSYMLINK "x %1$s symbolic link to %2$s.\n"
$ appears as message when using the -v flag.
$ the letter  x indicates extract.  do not translate.

ELONGLN "tar: %s skipped: linked to a path name that is too long.\n"

EBACKSP "tar: tape backspace error: "

EBADMVOL "tar: %s is not a device capable of %s multi-volume archives.\n"
$ first %s is replaced with device name
$ second %s is replaced with either TARCREATE or TARXTRACT

TARCREATE "creating"

TARXTRACT "exracting"

ESECSTDOUT "tar: cannot write archives to standard output.\n"

ESECSTDIN "tar: cannot read archives from standard input.\n"

ESECUSEF "\tplease use -f tarfile option.\n"
$ do not translate -f 

ESECMKNODAUTH "tar: %s: need mknod kernel authorization.\n"
$ do not translate mknod or kernel

EMAXRETRIES "tar: maximum retries limit exceeded - giving up.\n"

ENOMEMPFLAG "tar: no memory - not restoring modes of directories.\n"

ENOMEMTIME "tar: no memory - not restoring modification times of directories.\n"

ENOMEMORY "tar: no memory.\n"
