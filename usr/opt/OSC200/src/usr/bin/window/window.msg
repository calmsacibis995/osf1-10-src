 
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

$
$
$ @(#)$RCSfile: window.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:12:42 $
$
$ Messages for the window command.
$
$quote "
$
$set MS_WINDOW
COMMAND		"Command: "
NOWINDOW	"No window."
NOSUCHWIN	"%c: No such window."
NOPREVWIN	"No previous window."
TYPE4HELP	"Type ? for help."
WHICHWIN	"Which window? "
NOPROCINWIN	"No process in window."
NEWWIN1		"New window (upper left corner): "
NEWWIN2		"\nNew window (lower right corner): "
CANCELLED	"\nCancelled.  "
LOCINFO		"\nType [hjklHJKL] to move, return to enter position, escape to cancel."
$
$ keep these messages together, the last message in the group must
$ have the name SHELP_LAST.
$ Do not translate the actual command characters (first field)
$
SHRTCMDS	"Short commands"
SHELP01	"#       Select window # and return to conversation mode"
SHELP03	"%#      Select window # but stay in command mode"
SHELP04 "escape  Return to conversation mode without changing window"
SHELP05 "^^      Return to conversation mode and change to previous window"
SHELP06 "c#      Close window #"
SHELP07 "w       Open a new window"
SHELP08 "m#      Move window #"
SHELP09 "M#      Move window # to its previous position"
SHELP10 "s#      Change the size of window #"
SHELP11 "S#      Change window # to its previous size"
SHELP12 "^Y      Scroll up one line"
SHELP13 "^E      Scroll down one line"
SHELP14 "^U      Scroll up half a window"
SHELP15 "^D      Scroll down half a window"
SHELP16 "^B      Scroll up a full window"
SHELP17 "^F      Scroll down a full window"
SHELP18 "h       Move cursor left"
SHELP19 "j       Move cursor down"
SHELP20 "k       Move cursor up"
SHELP21 "l       Move cursor right"
SHELP22 "^S      Stop output in current window"
SHELP23 "^Q      Restart output in current window"
SHELP24 "^L      Redraw screen"
SHELP25 "^Z      Suspend"
SHELP26 "q       Quit"
SHELP_LAST ":       Enter a long command"

$
$ keep these help messages together, the last message in the group
$ must have the name LHELP_LAST.
$ Make sure commands are the same as in the command table
$
LONGCMDS	"Long commands"
LHELP01 ":alias name string ...  Make `name' an alias for `string ...'"
LHELP02 ":alias                  Show all aliases"
LHELP03 ":close # ...            Close windows"
LHELP04 ":close all              Close all windows"
LHELP05 ":cursor modes           Set the cursor modes"
LHELP06 ":echo # string ...      Print `string ...' in window #"
LHELP07 ":escape c               Set escape character to `c'"
LHELP08 ":foreground # flag      Make # a foreground window, if `flag' is true"
LHELP09 ":label # string         Set label of window # to `string'"
LHELP10 ":list                   List all open windows"
LHELP11 ":nline lines            Set default window buffer size to `lines'"
LHELP12 ":select #               Select window #"
LHELP13 ":shell string ...       Set default shell program to `string ...'"
LHELP14 ":smooth # flag          Set window # to smooth scroll mode"
LHELP15 ":source filename        Execute commands in `filename'"
LHELP16 ":terse flag             Set terse mode"
LHELP17 ":unalias name           Undefine `name' as an alias"
LHELP18 ":unset variable         Deallocate `variable'"
LHELP19 ":variable               List all variables"
LHELP20 ":window [row col nrow ncol nline label pty frame mapnl keepopen smooth shell]"
LHELP21 "                        Open a window at `row', `col' of size `nrow', `ncol',"
LHELP22 "                        with `nline' lines in the buffer, and `label'"
LHELP_LAST ":write # string ...     Write `string ...' to window # as input"

CONT		"%s: (continued)\n\n"
REALLYQ		"Really quit [yn]? "
YES		"Yes"
HELP		"Help"
CANTOPENHELP	"Can't open help window: %s."
ESCCHAR		"The escape character is %c.\n"
NUMEXP		"(# represents one of the digits from 1 to 9.)\n\n"));
CANTOPEN	"Can't open %s."
OUTOFMEM	"Out of memory."
NEWPOS		"New window position: "
DEBUG		"[m(smap) n(ns) o(os) s(string) v(nvis) w(win)]? "
DEBUG2		"Allocated Strings"
DEBUG3		"Can't open string window: %s."
DEBUG4		"No string debugging."
NEWWINSZ	"New window size (lower right corner): "
ERRFROM		"Errors from %s"
CANTOPENERR	"Can't open error window.  "
LINE		"line %d: "
NOWINSPEC	"No window specified."
ONLYONEC	"Only one -c allowed.\n"
USAGE		"Usage: window [-e escape-char] [-c command] [-t] [-f] [-d]\n"
IOSTAT		"IO Statistics"
CANTOPENSTAT	"Can't open statistics window: %s."
TRUSAGE		"Timing and Resource Usage"
COTIME		"Can't open time window: %s."
WINDOWS		"Windows"
COWINDOWS	"Can't open listing window: %s."
NOPROC		"(No process)"
NOLABEL		"(No label)"
VARS		"Variables"
COVARS		"Can't open variable window: %s."
ERROR		"ERROR\n"
ALIASES		"Aliases"
COALIASES	"Can't open alias window: %s."
IFERR		"if: Numeric value required."
SYNTAXERR	"Syntax error."
UNKNOWN		"%s: No such command or alias."
TOOMANY		"%s: Too many arguments."
BADSYN		"%s: Bad alias syntax."
UNKNOWNARG	"%s:Unknown argument \"%s\"."
DUPARG		"%s: Argument %d(%s) duplicated."
DUPARG2		"%s: Argument %d duplicated."
TYPEER		"%s: Argument %d (%s) type mismatch."
TYPEERR2	"%s: Argument %d type mismatch."
RECURALIAS	"%s: Recursive alias."
NOTVAR		"%d: Not a variable."
NUMLEFT		"?: Numeric left operand required."
NUMREQ		"||: Numeric operands required."
NUMREQ2		"&&: Numeric operands required."
NUMREQ3		"%s: Numeric operands required."
NOARG		"%d: No such argument."
UNDEFVAR	"%s: Undefined variable."
ECSCHAR2	"Escape character is %s.\r\n"
POSERR		"Illegal window position."
CANTOPENWIN	"Can't open window: %s."
NOMEM		"No memory for label."
CANTEXE		"Can't execute %s: %s."
TOOMANYWIN	"Too many windows."
CANTRESIZE	"Can't resize window: %s."
HITAKEY		"[Type any key to continue]"
HITAKEY2	"[Type escape to abort, any other key to continue]"
NOERROR		"No error"
NOPTTYS		"Out of pseudo-terminals"
BADSIZE		"Bad window size"
BADTYPE		"Unknown terminal type"
NODICE		"Can't run window on this terminal"
UNKNOWNE	"Unknown error"
PROCDIED	"Process died."
