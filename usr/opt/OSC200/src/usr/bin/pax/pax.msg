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
$ @(#)$RCSfile: pax.msg,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/10 20:28:17 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2
$
$ Messages for the pax archiving program
$

$quote "
$set MS_PAX

$
$ Generally when a message has a '%s : ' in the front of it, the
$ name of the program (pax/tar/cpio) will be inserted there.
$

NOMEM		"There is a memory allocation problem."
NOTDIR		"The destination directory is not a directory."
BLOCKS		"%ld Blocks\n"
$
$ 	messages for the file append.c
$
APPEND_INVALID  "The append option is not valid for specified device."
APPEND_CORRUPT	"The archive file appears to be corrupt."
APPEND_BACK "tape backspace error"
APPEND_BACK2	"A backspace error occurred during the append operation."
$
$ 	messages for the file buffer.c
$
BUF_PRESERVE	"An error occurred attempting to preserve the owner and group ids."
BUF_EOF		"An unexpected end-of-file has occurred."
BUF_OVERFLOW	"An overflow condition exists in the write buffer."
BUF_BLKSIZE	"The blocksize value must be greater than zero."
BUF_IOBUF	"Cannot allocate memory for the input/output buffer."
BUF_BUFSKIP	"An overflow condition has occurred within the write buffer."
BUF_BUF_READ	"An overflow condition exists in the read buffer."
BUF_CORRUPT	"The archive file appears to be corrupt."
BUF_WRITE	"A write error has occurred."
BUF_INPUT	"The archive is empty."
BUF_AVAIL	"An overflow condition exists within the write buffer."
$
$ 	messages for the file charmap.c
$
$ NOTICE TO TRANSLATORS:
$ The CM_ messages should not be translated since the charmap stuff with
$ the -e option has been removed from Posix1003.2 Draft 11.2  That means
$ these messages cannot be seen by any user using the pax command.  This
$ may change but as of 06-24-91 this option is out of pax.
$
CM_LONG		"The charmap converted filename has become too long."
CM_NOTFOUND	"Could not find charmap entry"
CM_SYNTAX	"Syntax error in charmap - skipping"
CM_DECL		"Error in charmap declarations - skipping"
CM_RANGE	"Error in charmap range - skipping"
CM_ENCODE	"Error in charmap encoding - skipping"
CM_ADDRANGE	"Error adding charmap range"
CM_ADDENTRY	"Error adding charmap entry"
CM_LINE		"line %d"
$ This message follows the CM_SYNTAX,CM_DECL,CM_RANGE, and CM_ENCODE
$ messages and tells the line number that is being skipped in the 
$ charmap file.
CM_NOMEM	"Unable to allocate enough memory to construct charmap."
$
$ 	messages for the file cpio.c
$
CPIO_USAGE1	"Usage: %s -o[Bacv]\n"
CPIO_USAGE2	"       %s -i[Bcdmrtuvf] [pattern...]\n"
CPIO_USAGE3	"       %s -p[adlmruv] directory\n"
$
$ 	messages for the file create.c
$
CR_LONG		"The name is longer than 255 bytes."
$
$ 	messages for the file extract.c
$
EX_CORRUPT	"The archive file appears to be corrupt."
$
$ The next two messages are printed in sequence
$
EX_NOTAR	"This doesn't look like a tar archive."
EX_SKIP		"         Skipping to the next file..."

EX_UNREG	"The archive format is not recognizable."
EX_BADMAG	"The magic number is not recognizable."
EX_MAY		"The file may be corrupt."
EX_RESYNC	"The extraction appears to have resynchronized."
EX_CONT		"Continuing"
EX_SYM		"A symbolic link is corrupted."
EX_OWN		"An error occurred attempting to restore the owner and group ids."
EX_HDR		"The swapped header is corrupted."
EX_PLEN1	"The swapped pathname length is bad."
EX_PATH		"The swapped pathname is corrupted."
EX_BADPATH	"The swapped pathname is bad."
EX_ASCII	"A cpio ASCII header is corrupted."
EX_BADHDR	"A cpio ASCII header is bad."
EX_PLEN2	"A cpio ASCII pathname length is bad."
EX_APATH	"A cpio ASCII pathname is corrupted."
EX_BADAPATH	"A cpio ASCII pathname is bad."
EX_BHDR		"A cpio binary header is corrupted."
EX_PLEN3	"A cpio binary pathname length is bad."
EX_BPATH	"A cpio binary pathname is corrupted."
EX_BADP		"A cpio binary pathname is bad."
$
$ 	messages for the file fileio.c
$
FIO_EXISTS	"The file exists and will not be overwritten."
FIO_SAME	"The file is the same."
FIO_NEWER	"The file exists and is newer."
FIO_DIR		"Directories are not being created."
FIO_UKNOWN	"The filetype is unknown."
$
$ 	messages for the file link.c
$
LK_UNSEEN	"A link exists on the archive but was not processed."
$
$ 	messages for the file list.c
$
LS_SYM		"Unable to read a symbolic link"
$	This message is followed by the strerror message string for the errno
LS_LINK		" is linked to %s"
$	%s is the name that the file is linked to
LS_SYMLINK	" is a symbolic link to %s"
$	%s is the name that the symbolic link points to 
LS_XSYM		"x %1$s is a symbolic link to %2$s\n"
$	%1$s is replaced by the extracted file name
$	%2$s is replaced by the filename that the extracted file name points to
LS_LINK2	"%1$s is linked to %2$s\n"
$	%1$s is replaced by the extracted file name
$	%2$s is replaced by the filename that the extracted file name is linked to
LS_SUM		"x %s, %ld bytes, %d tape blocks\n"
$	%s is replaced by the file size in bytes
$	%d is replaced by the number of blocks required to store the file 
LS_READ		"Unable to read a symbolic link"
$	This message is followed by the strerror message string for the errno
LS_ASYM		"a %1$s is a symbolic link to %2$s\n"
$	%1$s is replaced by the extracted file name
$	%2$s is replaced by the filename that the extracted file name points to
LS_LINK3	"is a link to %s\n"
$	%s is replaced by the name of the link
LS_SUM2		"? %s %ld blocks\n"
$	%s is replaced by the filename
$	%d is replaced by the number of blocks required to store the file 
$
$ 	messages for the file namelist.c
$
NL_MEM		"Unable to allocate enough memory for the filename list."
NL_DIR		"An error occurred attempting to open the directory."
NL_FOUND	"%1$s: %2$s was not found in the archive.\n"
$	%1$s is replaced by the name of the command(ie, pax,cpio,or tar)
$	%2$s is replaced by the filename which was not found
NL_LONG		"The name is too long."
$
$ 	messages for the file pathname.c
$
PN_TOOMANY	 "The pathname contains too many elements."
$
$ 	messages for the file pax.c
$
PAX_BSIZE	"An invalid block size was specified with the -b option."
PAX_PRIV	"An invalid file characteristic was specified with the -p option."
PAX_CHARMAP	"An error occurred trying to read the charmap file."
PAX_FORMAT	"The archive format specified is different from the existing archive."
PAX_TYPE	"Unable to determine the archive format."
PAX_CPIO	"ASCII CPIO format archive\n"
PAX_BINCPIO	"Binary CPIO format archive\n"
PAX_TAR		"USTAR format archive\n"
PAX_BADVAL	"An invalid block size was specified with the -b option."
PAX_U1 		"Usage: %s -[cdnv] [-f archive] [-s replstr] [pattern...]\n"
PAX_U2 		"       %s -r [-cdiknuvy] [-f archive] [-p string] [-s replstr]\n              [pattern...]\n"
PAX_U3 		"       %s -w [-dituvyX] [-b blocking] [[-a] -f archive] [-s replstr]\n               [-x format] [pathname...]\n"
PAX_U4 		"       %s -r -w [-diklntuvyX] [-p string] [-s replstr] [pathname...] directory\n"
PAX_NOTDIR	"The destination directory is not a directory."

$
$       messages for the file replace.c
$
RP_NOADD	"A replacement string was not added"
$	This message is followed by a colon and then one of
$	the RP_SPACE,RP_BADRE, RP_TRLR, RP_BADDEL or regerror() messages.

$
$	These messages are preceded by the RP_NOADD message.
RP_SPACE	"Unable to allocate enough memory."
RP_MOD		"Invalid RE modifier"
RP_TRLR		"An invalid trailing option was specified with the -s option."
RP_BADDEL	"Bad delimeters"
RP_BADMB	"An invalid multi-byte character was encountered."

RP_ADD		"add %s ? "
$	%s is replaced by the filename to add
RP_EXTRACT	"extract %s ? "
$	%s is replaced by the filename to extract
RP_PASS		"pass %s ? "
$	%s is replaced by the filename to copy
RP_RENAME	"rename %s? "
$	%s is replaced by the filename to rename
RP_BADREF	"An invalid backreference was specified."
RP_STRLONG  "The new filename exceeded the path name limit.\n"
RP_NOSUB    "         There was no substitution done on %s\n"
$	%s is replaced by the filename
RE_DAMAGE2	"regexp: damaged match string"

$
$       messages for the file tar.c
$
TAR_MODE  "%s: Exactly one of the c, t, r, u or x options must be specified.\n"
TAR_BSIZE	"Tar: blocksize = %d\n"
$	%d is the blocksize(an integer)
TAR_OPT		"%s: unknown option %c\n"
$	%s is replaced by "tar"
$	%c is replaced by the invalid option
TAR_ARG		"%s: %c argument missing\n"
$	%s is replaced by "tar"
$	%c is replaced by the invalid option
TAR_USAGE1	"Usage: %s -c[bfvw] device block filename..\n"
TAR_USAGE2	"       %s -r[bvw] device block [filename...]\n"
TAR_USAGE3	"       %s -t[vf] device\n"
TAR_USAGE4	"       %s -u[bvw] device block [filename...]\n"
TAR_USAGE5	"       %s -x[flmovw] device [filename...]\n"
$	%s is replaced by "tar"
$
$       messages for the file ttyio.c
$
TTY_UNVAIL	"Unable to prompt user: /dev/tty is unavailable."
TTY_ABORT	"Aborted"
TTY_CONT	"Continuing"
$
$ Make sure the two strings quoted here are the same as TTY_GO and
$ TTY_QUIT, as the program will use those two messages to do a
$ strcmp on.
$
$ message should not be more than 200 bytes, 
$ answers can't be more than 20 bytes
$
TTY_PROMPT	"%s: Ready for volume %u\n%s: Type \"go\" when ready to proceed (or \"quit\" to abort): \07"
TTY_GO		"go"
TTY_QUIT	"quit"
$
$       messages for the file warn.c
$
WARN_OFF	"%s: [offset "
$	%s is replaced by the command name

