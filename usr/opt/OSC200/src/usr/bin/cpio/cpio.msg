 
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

$ @(#)$RCSfile: cpio.msg,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1992/10/30 16:31:13 $ 8/15/90 SecureWare

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
$ cpio.msg	1.8  com/cmd/arch,3.1,9021 3/21/90 21:24:41
$
$quote "
$set MS_CPIO
$ Do not translate cpio

ATPFTS "Cannot pass a file to itself.\n"

BBS "Invalid buffer size '%s'\n"

BCIRR "Cannot use the -B or -C flags with the -p flag.\n"

BSR "The block size was reduced to %u bytes.\n"

CBS "Check block size.\n"
$ This is preceded by a perror message.

CCD "Cannot create directory for %1$s; errno is %2$d.\n"
$ Do not translate errno.
$ The %1$s will be a file name and the other will be an integer

CCDNW "Cannot move to directory.\n"
$ Don't spend a lot of time on this one -- it is never seen

CCF "Cannot create %1$s; errno is %2$d.\n"
$ Do not translate errno.
$ The %1$s will be a file name and the other will be an integer

CCHD "Cannot move to a specified directory.\n"
$ Don't spend a lot of time on this one -- it is never seen

CCLF "Cannot close %s: %s\n"
$ The 1st %s is a file name
$ The 2nd %s is a perror string 

CCLN "Cannot symlink <%s> and <%s>: "

CCM "Cannot change permissions of %1$s; errno is %2$d.\n"
$ Do not translate errno.
$ The %1$s will be a file name and the other will be an integer

CCO "Cannot change ownership of %1$s; errno is %2$d.\n"
$ Do not translate errno.
$ The %1$s will be a file name and the other will be an integer

CCS "Cannot close %s: %s\n"
$ The 1st %s is a file name
$ The 2nd %s is a perror string 

CFN "A newer or same aged version of %s already exists.\n"
$ The %s will be a file name

CLSTS "Cannot link %1$s and %2$s: "
$ The variables will be file names

CNMN "Cannot create %1$s; errno is %2$d.\n"
$ Do not translate errno.
$ The %1$s will be a file name and the other will be an integer

COBS "The -B flag value overrides the -C flag value.\n"

COCS "The -C flag value overrides the -B flag value.\n"

COPN "Cannot open %s.\n"

CRNDT "Cannot use the -r flag (rename) because %s is missing.\n"
$ %s is replaced with "/dev/tty"

CRS "Cannot read %s: %s.\n"

CRSINC "Cannot complete read of %s.\n"

CRSL "Cannot read from symbolic link <%s>: "

CUL "Cannot unlink %1$s; errno is %2$d.\n"
$ Do not translate errno.
$ The %1$s will be a file name and the other will be an integer

CWIF "Cannot write to the %s directory.\n"

CWF "Cannot write to %s: %s\n"
$ The 1st %s is a file name, the 2nd a perror message

CWFINC "Cannot complete write to %s\n"
$ The %s is a file name

DINPUT "input"

DOUTPUT "output"

ECLOSEIN "Cannot close input"

ECLOSEOUT "Cannot close output"

ECLOSEVOL "\007Encountered an error closing output"

EINPUT "Cannot open <%s> for input: "

ENOINODEV "Cannot archive <%s> (too many files in archive).\n"

ENOLINK "Out of memory for links.\nFile link data may be corrupt.\n"

EOM "\007Reached end of medium on %s\n"
$ Do not translate "\007cpio"
$ %s is replaced with DINPUT or DOUTPUT

EOUTPUT "Cannot open <%s> for output: "
FLTF "%1$s is linked to %2$s.\n"
$ Both will be file names

GOON "Enter the device or file name when ready to continue.\n"

IGNSPECIAL "Special file %s was ignored.\n"

IHDRQS "Cannot get information about %s.\n"
$ The %s will be a file name

IHDRQ "Cannot open %s.\n"
$ The %s will be a file name

IOERR "\007Encountered an error on "
$ This is followed by either DINPUT or DOUTPUT and then a perror message.

LARGEFILEIN "File <%s> ignored: file too large, limit is %u bytes.\n"

LARGEFILEOUT "File %s cannot be archived: file too large, limit is %u bytes.\n"

LOFDM "The file name length does not match the expected value.\n"

MISD "Cannot move to a necessary directory.\n"

NEW "Original file has a later modification date. Cannot write.\n"

NEXTVOL "Change to volume %d and press RETURN key. [q] "

NMFL "There is not enough memory available to continue linking files.\n"

NOBLKCOP "%ld blocks were copied.\n"

NOMLC "There is not enough memory available now.\n"

NOPROMPT "Cannot prompt for input (can't open %s): "
$ %s is replaced with "/dev/tty"; this is followed by a perror message.

NOSPECIAL "Special file %s cannot be archived.\n"

NOTCPIO "Not a cpio archive.\n"

OMIL "Use one of the -o, -i, or -p flags.\n"

OOF "Cannot copy from the diskette.\n"
$ Do not translate cpio

PREX "Cannot use the -r flag with the -p flag.\n"

RPRPT "Rename <%s>: "
$ Do not translate.

SCLN "Cannot create a symbolic link.\n"

SKIP "Skipped\n"
$ Do not translate

TML "There are too many links.\n"

USAGE1 "Usage: %s %s [-Cbufsize] <name-list >collection\n"
USAGE2 "\t%s %s [-Cbufsize] -Ocollection <name-list\n"
USAGE3 "\t%s %s [-Cbufsize] [pattern ...] <collection\n"
USAGE4 "\t%s %s [-Cbufsize] -Icollection [pattern ...]\n"
USAGE5 "\t%s %s directory <name-list\n"

USAGEOPT1 "-%c must be specified before -%c option"
USAGEOPT2 "-%c option not permitted with -%c option"
USAGEOPT3 "No more than one -I or -O flag permitted"

USE_D "Use the -d flag to copy %s.\n"

WINPUT "Cannot read from the specified input: "

WOUTPUT "Cannot write to the specified output: "
