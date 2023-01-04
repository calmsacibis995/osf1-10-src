$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0
$quote "
$set MS_GETTY

MSG1	"Amnesiac"
TIMEOUT	"getty: timeout argument invalid. \"%s\"\n"
CHECK_USG	"Check Mode Usage: getty -c gettydefs-like-file\n"
CANTOPEN	"Cannot open %s\n"
NO_TERMINAL	"getty: no terminal line specified.\n"
UNABLE	"getty: unable to find %s in \"%s\".\n"
UNKNOWN	"unknown"
LD_UNKNOWN	"getty: %s is an unknown line discipline.\n"
CANT_SET_LD	"Cannot set discipline %d\n"
BAD_ENTRY	"getty: pointer to next speed in entry %s is bad.\n"
UNABLE_AGAIN	"getty: unable to find %s again.\n"
CANT_OPN1	"getty: cannot open \"%s\". errno: %d\n"
CANT_ASSIGN	"getty: cannot assign controlling tty: errno = %d\n"
CANT_OPN2	 "getty: can't open \"%s\".\n"
TOO_LONG	"Entry too long.\n"
NEXT_ENTRY	"\n**** Next Entry ****\n%s\n"
G_ID	"id: %s\n"
INITIAL_FLG	"initial flags:\niflag- %o oflag- %o cflag- %o lflag- %o\n"
FINAL_FLG	"final flags:\niflag- %o oflag- %o cflag- %o lflag- %o\n"
MSG2	"message: %s\n"
NEXT_ID	"next id: %s\n"
PARSING_FAIL	"Parsing failure in the \"%s\" field\n\%s<--error detected here\n"
UNDEFINED	"Undefined: %s\n"
USAGE		"Usage: getty [-h] [-t time] line speed-label terminal-type [line-discipline]\n"
$set MS_GETTY_SEC
S_CANT_FORK1	"getty: can not fork a subprocess"
S_CANT_FORK2	"getty: cannot fork a subprocess\r\n"
S_CANT_EXEC1	"getty: cannot execute %s: %m"
S_CANT_EXEC2	"getty: cannot execute %s\r\n"
S_EXIT_CODE1	"getty: %s returned exit code 0x%x"
S_EXIT_CODE2	"getty: %s returned exit code 0x%x\r\n"
