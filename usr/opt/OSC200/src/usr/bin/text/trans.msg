 
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
 
$	@(#)$RCSfile: trans.msg,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/01/18 13:53:58 $

$
$ Modification history
$ ~~~~~~~~~~~~~~~~~~~~
$ 000	Tom Woodburn, 26 Mar 1991
$	- Created.
$

$quote "

$
$ trans.c messages
$

$set MS_TRANS

M_TRANS_1	"\nusage: %s [-c] [-o name] file.msg\n"
$ Usage; %s is the program name.

M_TRANS_2	"\nwarning: output name ignored, multiple files\n"
M_TRANS_3	"\nAcknowledged? (return)"
$ If you invoke trans like this:
$
$     trans -o files.msg file1.msg file2.msg file3.msg
$
$ M_TRANS_2 is printed, and a separate output file is used for each
$ message file (file1_trans.msg, file2_trans.msg, file3_trans.msg).
$ Then M_TRANS_3 is printed and trans waits for you to enter a character.

M_TRANS_4	"Save? (y/n) "
$ When you're done translating, trans makes sure you really want to
$ save your output.

M_TRANS_6	"y"
$ Valid positive reply.  Only the first character is used.

M_TRANS_8	"n"
$ Valid negative reply.  Only the first character is used.

M_TRANS_9	"error: %s: %s\n"
M_TRANS_10	"error: %s\n"
$ Print either two or one error string(s).

M_TRANS_11	"warning: %s\n"
$ Print a warning string.

M_TRANS_12	"no quote char set"
$ Error string:  $quote isn't followed by a quote character.

M_TRANS_13	"Original"
M_TRANS_14	"Translated"
M_TRANS_15	"Type '<ctrl>k' for help"
$ Titles for original, translated, and command windows.

M_TRANS_16	"cannot open"
$ Error string:  can't open file.

M_TRANS_17	"file has been modified"
$ Error string:  translated file doesn't sync with original file.

M_TRANS_18	"unmatched quote, reinput line"
$ Error string:  message doesn't end with a quote character.

M_TRANS_19	"missing quote, reinput line"
$ Error string:  message doesn't begin with a quote character.

M_TRANS_20	"line starts with a blank, reinput line"
$ Warning string:  line starts with a blank.

M_TRANS_21	"Translating file: %s\n"
$ In the command window, show which file is being translated.
$ The %s is the name of the file.

M_TRANS_22	"DEC OSF/1 Translation Tool"
$ Window banner.

$
$ getline.c messages
$

$set MS_GETLINE

M_GETLINE_1	"INSERT"
M_GETLINE_2	"REPLACE"
$ Input modes.

M_GETLINE_3	"^h  back space    ^l  forward space   ^w  back word    ^f forward word\n"
M_GETLINE_4	"^e  end of input  ^b  start of input  ^n  next line    ^p previous line\n"
M_GETLINE_5	"DEL delete char   ^u  delete input    ^i  insert mode  ^r replace mode\n"
M_GETLINE_6	"^d  exit          ^c  quit"
$ Help on commands.
