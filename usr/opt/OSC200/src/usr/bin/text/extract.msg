 
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
 
$	@(#)$RCSfile: extract.msg,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/01/18 13:53:08 $

$
$ Modification history
$ ~~~~~~~~~~~~~~~~~~~~
$ 000	Tom Woodburn, 26 Mar 1991
$	- Created.
$

$quote "

$
$ re.c messages
$
$ (common file used by extract.c, strextract.c, and strmerge.c)
$

$set MS_RE

$ The first %s in these messages is the program name.

M_RE_1	"%s: regexp error %d in %s\n"
$ Printed by the ERROR macro used in /usr/include/regexp.h's compile()
$ function.  The %d is the error number, the second %s is the instring
$ parameter to compile().

M_RE_2	"%s: cannot open pattern file\n"
$ Printed if the pattern file can't be opened.

M_RE_3	"%s: malloc failed\n"
$ Printed if malloc() returns null.

M_RE_4	"%s: missing reserved word in %s\n"
$ Printed if a $ in the pattern file isn't followed by a reserved
$ word or space.  The second %s is the name of the pattern file.

M_RE_5	"%s: rewrite too long\n"
M_RE_6	"%s: CATHEAD too long\n"
M_RE_7	"%s: SRCHEAD1 too long\n"
M_RE_8	"%s: SRCHEAD2 too long\n"
$ Printed when the values following these keywords (REWRITE, CATHEAD,
$ SRCHEAD1, and SRCHEAD2) are too long.

$
$ extract.c messages
$

$set MS_EXTRACT

M_EXTRACT_1	"%s: bad option %c\n"
$ Printed when a command-line option isn't recognized.  The %c is
$ the option.

M_EXTRACT_2	"usage: %s\n[-p patterns] [-i ignore] [-u] [-n] [-m prefix] [-s string] filelist\n"
$ Usage, %s is the program name.

M_EXTRACT_3	"Error: %s %s\n"
$ Prints two error strings.  The first %s is either M_EXTRACT_6, 7,
$ 17, 18, or 19.  For 6, 7, 17, and 18, the second %s is a file name;
$ for 19, the second %s is the rewrite string.

M_EXTRACT_4	"Source: "
$ Title of the window displaying the source program.  Followed by
$ the pathname of that program; e.g., "Source: example.c".

M_EXTRACT_5	"Catalogue: "
$ Title of the window displaying the message catalog being created.
$ Followed by the pathname of that catalog; e.g., "Catalogue:
$ example.msg".

M_EXTRACT_6	"cannot open"
M_EXTRACT_7	"cannot popen "
$ Printed in the first %s of M_EXTRACT_3 when a file can't be opened
$ or popened.

M_EXTRACT_8	"Errors were found. Continue (y/n)? "
$ If the -u option is specified, strextract is run and its output
$ is used to drive extract.  Print this message if strextract outputs
$ any errors.

M_EXTRACT_9	"yn"
$ Used to check for valid input to M_EXTRACT_8 and to M_EXTRACT_15.

M_EXTRACT_10	"No string to duplicate. Press <RETURN> "
$ Printed if you choose the DUPLICATE command to duplicate a string
$ but the string isn't a duplicate of anything.

M_EXTRACT_11	"Cannot write to ignore file. Press <RETURN> "
$ Printed if a string can't be written to the ignore file.

M_EXTRACT_12	"Input text for message file and press <RETURN>\n"
$ Printed when you select the COMMENT command to enter a comment
$ in the message catalog.

M_EXTRACT_13	"Text OK (y/n/q) ? "
$ After you enter a comment (in response to M_EXTRACT_12), this
$ message is printed.

M_EXTRACT_14	"ynq"
$ Used to check for valid input to M_EXTRACT_13.

M_EXTRACT_15	"Really quit (y/n) ? "
$ Printed when you enter the quit command.

M_EXTRACT_16	"Select option and press <RETURN> "
$ The command window lists the valid commands.  Below them is this
$ message.

M_EXTRACT_17	"Bad offset in"
M_EXTRACT_18	"Unexpected EOF in "
M_EXTRACT_19	"Bad rewrite string in pattern file"
$ Printed in the first %s of M_EXTRACT_3.  See the explanations for
$ M_STRMERGE_3, 4, and 5.

M_EXTRACT_20	"Press <RETURN> to continue "
$ If you choose the HELP command, help is displayed with this message
$ at the bottom.

M_EXTRACT_21	"DEC OSF/1 String Extraction Tool V1.1"
$ Banner at the very top of the window.

M_EXTRACT_22	"n"
M_EXTRACT_23	"y"
M_EXTRACT_24	"q"
$ Valid no, yes, and quit responses.  Only the first character is used.

$ Valid commands.

M_EXTRACT_25	"EXTRACT"
$ Extract a string into the message catalog.

M_EXTRACT_26	"DUPLICATE"
$ If a string is a duplicate of a previous string, use the message
$ created for the previous string.

M_EXTRACT_27	"IGNORE"
$ Ignore a string and all subsequent occurences of it.

M_EXTRACT_28	"PASS"
$ Just ignore the current instance of a string.

M_EXTRACT_29	"ADD"
$ Like IGNORE, but add a string to the ignore file so it'll be ignored
$ if extract is run again.

M_EXTRACT_30	"COMMENT"
$ Add a comment to the message file.

M_EXTRACT_31	"QUIT"
$ Quit the program.

M_EXTRACT_32	"HELP"
$ Get help on the program.

M_EXTRACT_33	"%s: malloc failed in loadopts"
$ Printed in loadopts() if malloc() returns null.

$
$ strextract.c messages
$

$set MS_STREXTRACT

$ The first %s in these STREXTRACT messages is the program name.

M_STREXTRACT_1	"%s: bad argument %c\n"
$ Printed when a command-line option isn't recognized.  The %c is
$ the option.

M_STREXTRACT_2	"STDIN"
$ Used as the second %s in M_STREXTRACT_4-5 if the source program
$ comes from stdin.

M_STREXTRACT_3	"%s: cannot open %s\n"
$ Printed if an input file or output file can't be opened.  The
$ second %s is the file name.

M_STREXTRACT_4	"%s: %s, line %d: %s\n"
$ Used to print error messages returned by re.c's matchre().  The
$ second %s is the name of the input file, the %d is the line number
$ where the error occurred, and the last %s is the error message.

M_STREXTRACT_5	"%s: %s. Duplicate string line %d %.*s\n"
$ If the -d(uplicate) option isn't specified and a duplicate line
$ is found, prints a warning.  The second %s is the name of the input
$ file, the %d is the line number of the duplicate, and the last %s
$ is the duplicate string.

$
$ strmerge.c messages
$

$set MS_STRMERGE

$ The first %s in these STRMERGE messages is the program name.

M_STRMERGE_1	"%s: bad argument %c\n"
$ Printed when a command-line option isn't recognized.  The %c is
$ the option.

M_STRMERGE_2	"%s: cannot open %s\n"
$ Printed if an input file or output file can't be opened.  The
$ second %s is the file name.

M_STRMERGE_3	"%s: Bad offset %d in %s\n"
$ The offsets in the message file should be in increasing order.
$ Print this message if we find one that's not.  The %d is the
$ offset, the second %s is the name of the message file.

M_STRMERGE_4	"%s: Unexpected EOF in %s\n"
$ Printed if we hit the end of the source file before we find the
$ string we're looking for.  The second %s is the name of the
$ source file.

M_STRMERGE_5	"%s: Bad rewrite string in pattern file at %%%c\n"
$ In the REWRITE string, %s, %n, and %t are special.  If a % is
$ found without an s, n, or t after it, print this message.  The
$ %c is the bogus character following the %.
