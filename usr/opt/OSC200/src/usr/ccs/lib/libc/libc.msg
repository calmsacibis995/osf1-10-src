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
 
$ @(#)$RCSfile: libc.msg,v $ $Revision: 4.2.13.7 $ (DEC) $Date: 1993/12/16 23:55:44 $
$
$ (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
$ ALL RIGHTS RESERVED 
$
$
$ OSF/1 1.2

$
$ Copyright International Business Machines Corp. 1989
$ Unpublished Work
$ All Rights Reserved
$ Licensed Material - Property of IBM
$
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ 
$ libc.msg	1.24  com/lib/c/msghdr,3.1,9013 1/24/90 12:13:18
$
$	Revision History:
$
$ 14-May-91	Peter H. Smith
$	Don't piggyback on EINPROGRESS.
$
$ 03-May-91	Peter H. Smith
$	Add POSIX 1003.4 Draft 10 errno definitions.

$quote "

$set MS_LIBC

$ Must begin a message 99 for compatibility
99 "Function not implemented"

$ The following message is for perror
M_PERROR "Error %d occurred."

$ The following message is for NLinit
M_NLINIT "Cannot obtain standard C locale\n"

$ The following messages are for assert
M_ASSERT1 "Assertion failed:\040"
M_ASSERT2 ", file\040"
$ Do not add message number for previous string.
M_ASSERT3 ", line NNNNN\n"
$ Do not add message number for previous string.
$ The previous string must contain NNNNN.
$ _assert will replace NNNNN with the line number (__LINE__).

$ The following message is for initgroups
M_INITG	"initgroups: %s is in too many groups\n"

$ The following messages are for getgrent
M_SETGR	"setgrent: getdomainname system call missing\n"
M_GETGR	"getgrent: getdomainname system call missing\n"

$ The following messages are for getpwent
M_SETPW "setpwent: getdomainname system call missing\n"
M_GETPW "getpwent: getdomainname system call missing\n"

$ The following messages are for getopt
$	There is a trailing space after each.
$  These message will be follow by the offending option:
$	<progname> : illegal option -- <opt>\n

M_OPTILL ": illegal option -- "
M_OPTARG ": option requires an argument -- "

$ The following messages are for getwd
M_RSTAT "getwd: can't fullstat /"
M_HSTAT "getwd: can't fullstat ."
M_OPENPAR "getwd: can't open .."
M_READPAR "getwd: read error in .."

$ The following messages are for psignal()
$ THIS MUST BE KEPT IN_SYNC WITH siglist.c
M_PSIGNAL       "Signal 0"
M_SIGHUP        "Hangup"
M_SIGINT        "Interrupt"
M_SIGQUIT       "Quit"
M_SIGILL        "Illegal instruction"
M_SIGTRAP       "Trace/BPT trap"
M_SIGABRT       "IOT/Abort trap"
M_SIGEMT        "EMT trap"
M_SIGFPE        "Floating point exception"
M_SIGKILL       "Killed"
M_SIGBUS        "Bus error"
M_SIGSEGV       "Segmentation fault"
M_SIGSYS        "Bad system call"
M_SIGPIPE       "Broken pipe"
M_SIGALRM       "Alarm clock"
M_SIGTERM       "Terminated"
M_SIGURG        "Urgent I/O condition"
M_SIGSTOP       "Stopped (signal)"
M_SIGSTP        "Stopped"
M_SIGCONT       "Continued"
M_SIGCHLD       "Child exited"
M_SIGTTIN       "Stopped (tty input)"
M_SIGTTOU       "Stopped (tty output)"
M_SIGIO         "I/O possible/complete"
M_SIGXCPU       "Cputime limit exceeded"
M_SIGXFSZ       "Filesize limit exceeded"
M_SIGVTALRM     "Virtual timer expired"
M_SIGPROF       "Profiling timer expired"
M_SIGWINCH      "Window size changes"
M_SIGINFO       "Information request"
M_SIGUSR1       "User defined signal 1"
M_SIGUSR2       "User defined signal 2"
M_NO_SIGNAL     "Unknown signal"

$ The following messages are for initstate
M_INITST "initstate: not enough state (%d bytes) with which to do operation; ignored.\n"
M_SETST	"setstate: state info has been damaged; not changed.\n"

$ The following messages are for mcount
M_MONCNT "%s: Number of %s (%u) exceeds profiling limit (%u)\n"

$ The following messages are for monitor
M_MON1 "%s: No space for profiling buffer(s) (%u bytes)\n"
M_MON2 "%s: Error processing output data file: %s\n"
M_MON3 "%s: Null address range specified Lo: %8.8x  Hi: %8.8x\n"
M_MON4 "%s: _mondata.prof_type(%d) must be _PROF_TYPE_IS_P or _PROF_TYPE_IS_PG\n"
M_MON5 "%s: No data area found. (Stopped without starting?)\n"
M_MON6 "%s: Buffer range not contiguous. p[%d]: %8.8x size[%d]: %8.8x p[%d]: %8.8x\n"
M_MON7 "%s: Ranges not in ascending order. hi[%d]: %8.8x lo[%d]: %8.8x\n"

$ The following messages are for crt0/mcrt0/gcrt0
M_CRT0 "%s: Profiling setup call to %s failed. rc: %d\n"

$ The following message is for re_comp
M_RETOOLONG "Regular expression too long"
M_NOPREV "No previous regular expression"
M_UNMATCH "unmatched \\("
M_MISSING "missing ]"
M_TOOMANY "too many \\(\\) pairs"
M_UNMATCHLEFT "unmatched \\)"

$ The following message is for longjmperr
M_SETJMPER "longjmp or siglongjmp function used outside of saved context\n"
$	Cause:	This is a programming error with the setjmp and longjmp or
$		sigsetjmp and siglongjmp functions.  Either the program does
$		not use the setjmp or sigsetjmp function to save the program
$		context, or the function containing the setjmp or sigsetjmp	$		invocation has terminated in the interim.
$

$ The following messages are for getwd
M_LSTAT "getwd: can't stat .."
M_STATFS "getwd: can't statfs ."
M_PATHTOOLONG "getwd: path too long"
M_NOMEM "getwd: out of memory"
M_BADSIZE "getwd: buffer size too small"

$quote "
$set MS_LIBSIA

SIA_MSG_LOGEVENT     	"EVENT"         
SIA_MSG_LOGERROR     	"ERROR"     
SIA_MSG_LOGALERT     	"ALERT" 
SIA_MSG_LDRERR		"Error accessing sia package %s \n"
SIA_MSG_CONFERR		"SIA configuration error pertaining to %s\n"
SIA_MSG_ENTINCON	"SIA configuration entity inconsistency\n"
SIA_MSG_NOPASSPORTS	"SIA out of passports "
SIA_MSG_INITLDRERR	"SIA initialization loader error"
SIA_MSG_INITFAIL 	"SIA initialization failure"
SIA_MSG_NOCHGFINGER	"No selection configured for changing finger "
SIA_MSG_MULTMECHMENU 	"You are registered with the following security mechanisms\n"
SIA_MSG_BADCOLLERR	"Error in parameter collection"
SIA_MSG_CHGFINGERR	"Failure to change finger operation for %s"
SIA_MSG_CHGFINGOOD	"Successful change finger operation for %s"
SIA_MSG_NOCHGPASSWD  	"No selection configured for changing password"
SIA_MSG_CHGPASSERR	"Failure to change password for %s"
SIA_MSG_CHGPASSGOOD	"Successful password change for %s"
SIA_MSG_NOCHGSHELL	"No selection configured for changing shell"
SIA_MSG_CHGSHELLERR  	"Failure to change shell for %s"
SIA_MSG_CHGSHELLGOOD 	"Successful shell change for %s"
SIA_MSG_MATFAIL		"Failure to initialize SIA matrix"
SIA_MSG_MATGOOD		"Successful SIA matrix initialization"
SIA_MSG_MATBADFORMAT	"Bad matrix file format in %s\n"
SIA_MSG_MATOPENERR	"Can't open matrix file %s\n"
SIA_MSG_MATCLOSERR 	"Can't close matrix file %s\n"
SIA_MSG_SEMAERR		"SIA semaphore failure on %s"
SIA_MSG_INITKEY		"SIA initialization semaphore"
SIA_MSG_STATKEY 	"SIA wait for initialization semaphore"
SIA_MSG_NOSYSPASSERR	"No selection configured for changing password"
SIA_MSG_BADINITERR	"SIA initialization failure"
SIA_MSG_NOFILERR	"File %s not found"
SIA_MSG_INITGOOD	"Successful SIA initialization"
SIA_MSG_SESAUTHGOOD	"Successful session authentication for %s on %s"
SIA_MSG_SESAUTHBAD	"Failure to authenticate session for %s on %s"
SIA_MSG_SESESTABAD	"Failure to establish session"
SIA_MSG_SESESTABGOOD 	"Successful establishment of session"
SIA_MSG_GOODSESINIT	"Successful session initialization"
SIA_MSG_SESINITERR	"Failure to initialize  session"
SIA_MSG_SESLAUNCHBAD 	"Failure to launch  session"
SIA_MSG_SESLAUNCHGOOD 	"Successful launching of session"
SIA_MSG_SUAUTHGOOD	"Successful authentication for su from %s to %s"
SIA_MSG_SUAUTHBAD	"Failure on authentication for su from %s to %s"
SIA_MSG_ENTALLOCBAD	"Failure to allocate an entity"
SIA_MSG_REAUTHGOOD	"Successful reauthentication "
SIA_MSG_REAUTHBAD	"Failure on reauthentication "
SIA_MSG_GOODRELEASE	"Successful session release"
SIA_MSG_BADRELEASE 	"Failure in session release"
SIA_MSG_MISMATCH	"Hashed passwd database does not match the %s file.  Rerun 'mkpasswd %s'.\n"
SIA_MSG_NIS		"\nUser %s not found in the local %s file.  Use 'yppasswd' to change an NIS passwd entry.\n"   

SIAMULTMECHMENU	"You are registered with the following security mechanisms\n"

SIA_MSG_CHGPASSWD	"change password"
SIA_MSG_SUTRY		"su"
SIA_MSG_REAUTHTRY	"reauthentication"

SIA_LAST_LOG		"Last login: %.*s "
SIA_LAST_FROM		"from %.*s"
SIA_LAST_ON		"on %.*s"

SIA_MSG_DIALUP		"DIALUP: %s %s"
SIA_MSG_ROOTLOGIN	"ROOT login on %s"

$set MS_BSD

ENTRY_LOGIN		"login: "
ENTRY_PWD		"Password:"
ENTRY_LOGIN_TITLE	"System Login"
INCORRECT		"Login incorrect"
TOO_MANY_USR		"Too many users logged on already.\nTry again later."
TOO_MANY_PROC		"You have too many processes running."
QUOTA			"quota (Q_SETUID)"
NO_DIR			"No directory!"
LOG_IN			"Logging in with home = \"/\"."
REFUSEDON		"root login refused on this terminal.\n"
WHO_ARE_YOU 		"Who are you?"
UNKNOWN 		"Unknown login: %s"
NO_PERM 		"You do not have permission to su %s\n"
SORRY 			"Sorry\n"
CANTOPEN		"Can't open %s"
REFUSEDLOG		"ROOT LOGIN REFUSED %s"

$set MS_BSDPASSWD

CHFNU		"usage: chfn [username]\n"
CHSHU		"usage: chsh [username]\n"
USAGE		"usage: passwd [-fs] [username]\n"
UNKUID		"%s: %u: unknown user uid.\n"
OUTOFSPACE	"%s: out of space.\n"
UNKUSER		"%s: %s: unknown user.\n"
PERMDEN1	"Permission denied.\n"
CHANG		"Changing %s for %s.\n"
FINGINF		"finger information"
LOGINSHELL	"login shell"
PASSWORD	"password"
PWDFILEBUSY	"%s: password file busy - try again.\n"
FDOPNFAIL	"%s: fdopen failed.\n"
WARN1		"Warning: dbm_open failed: %s: "
WARN2		"Warning: lock failed"
PERMDEN2	"%s: permission denied.\n"
WARN3		"Warning: dbm_store failed.\n"
WARN4		"Warning: %s write error, %s not updated.\n"
OLDPWD		"Old password:"
NEWPWD		"New password:"
PWDUNCH		"Password unchanged."
LONGER		"Please enter a longer password."
SUGGEST		"Please don't use an all-lower case password.\nUnusual \
		 capitalization, control characters or digits are suggested."
RETYPR		"Retype new password:"
MISMATCH	"Mismatch - password unchanged."
RESTRICT	"Cannot change from restricted shell %s\n"
OLDSH		"Old shell: %s\nNew shell: "
LOGINSH		"Login shell unchanged."
UNACCSH		"%s is unacceptable as a new shell.\n"
UNAVAILB	"%s is unavailable.\n"
DEF_VALUE	"Default values are printed inside of '[]'."
ACCEPT		"To accept the default, type <return>."
ENTRY1		"To have a blank entry, type the word 'none'."
NAME		"\nName [%s]: "
ROOM		"Room number (Exs: 597E or 197C) [%s]: "
PHONE1		"Office Phone (Ex: 6426000) [%s]: "
PHONE2		"Home Phone (Ex: 9875432) [%s]: "
FINGERINFO	"Finger information unchanged."
BADCHARS	"':' and ',' are not allowed."
MAX_CHAR_NUM	"Maximum number of characters allowed is %d\n"
CTRCHARS	"Control characters are not allowed."
PHONEMSG	"Phone numbers may only contain digits."
ALLOCATE	"\nUnable to allocate storage in get_defaults!\n"
NONE		"none\n"
BADTTY		"No controlling tty."
NODB		"Hashed database not in use, only %s text file updated.\n"

$set MS_BSDSU

BAD_SU2         "BADSU %s on %s"
SU_ON           "SU %s on %s"
SET_GID         "su: setgid"
INITGROUPS      "su: initgroups failed\n"
SETUID          "su: setuid"
NO_SHELL        "No shell\n"

$ The following are Regular Expression errors
$ The default error text is found in regerror.c

$set MS_REG

$ Do not translate: [] () {} *?+ ^ $

1   "Pattern not found."
2   "Invalid pattern."
3   "Invalid character within [] expression."
4   "Invalid character class within [] expression."
5   "Last character cannot be \\."
6   "Invalid subexpression backreference number"
7   "[] imbalance."
8   "() imbalance."
9   "{} imbalance."
10  "Invalid {} repetition."
11  "Invalid range within [] expression."
12  "Out of memory."
13  "?*+ not preceded by valid expression."
14  "Invalid multibyte character."

$set LIBCNET
NET12 "Unknown Error"
NET13 "Error 0"
NET14 "Unknown host"
NET15 "Host name lookup failure"
NET16 "Unknown server error"
NET17 "No address associated with name"
NET18 "innetgr: getdomainname system call missing\r\n"
NET19 "innetgr: recursive overflow\r\n"
NET20 "innetgr: netgroup %s called recursively\r\n"
NET21 "innetgr: syntax error in /etc/netgroup\r\n"
NET22 "%s: unknown host\n"
NET23 "socket: All ports in use\n"
NET24 "connect to address %s: "
NET25 "Trying %s...\n"
NET26 "write: setting up stderr"
NET27 "socket: protocol failure in circuit setup.\n"
NET28 "rcmd: getdomainname system call missing\n"
NET29 "Error - .netrc file not correct mode.\n"
NET30 "Remove password or correct mode.\n"
NET31 "Unknown .netrc option %s\n"
NET32 "Can't make key\n"
NET33 "Name (%s:%s): "
NET34 "Password (%s:%s): "
NET35 "setprotoent: getdomainname system call missing\n"
NET36 "getprotoent: getdomainname system call missing\n"

$set MS_ERRNO

$ The following are from libc/gen/errlst.c, for perror
1 "Not owner"
2 "No such file or directory"
3 "No such process"
4 "Interrupted system call"
5 "I/O error"
6 "No such device or address"
7 "Argument list too long"
8 "Exec format error"
9 "Bad file number"
10 "No child processes"
11 "Resource temporarily unavailable"
12 "Not enough space"
13 "Permission denied"
14 "Bad address"
15 "Block device required"
16 "Device busy"
17 "File exists"
18 "Cross-device link"
19 "No such device"
20 "Not a directory"
21 "Is a directory"
22 "Invalid argument"
23 "File table overflow"
24 "Too many open files"
25 "Not a typewriter"
26 "Text file busy"
27 "File too large"
28 "No space left on device"
29 "Illegal seek"
30 "Read-only file system"
31 "Too many links"
32 "Broken pipe"
33 "Argument out of domain"
34 "Result too large"
35 "Operation would block"
36 "Operation now in progress"
37 "Operation already in progress"
38 "Socket operation on non-socket"
39 "Destination address required"
40 "Message too long"
41 "Protocol wrong type for socket"
42 "Option not supported by protocol"
43 "Protocol not supported"
44 "Socket type not supported"
45 "Operation not supported on socket"
46 "Protocol family not supported"
47 "Address family not supported by protocol family"
48 "Address already in use"
49 "Can't assign requested address"
50 "Network is down"
51 "Network is unreachable"
52 "Network dropped connection on reset"
53 "Software caused connection abort"
54 "Connection reset by peer"
55 "No buffer space available"
56 "Socket is already connected"
57 "Socket is not connected"
58 "Can't send after socket shutdown"
59 "Too many references: can't splice"
60 "Connection timed out"
61 "Connection refused"
62 "Too many levels of symbolic links"
63 "File name too long"
64 "Host is down"
65 "Host is unreachable"
66 "Directory not empty"
67 "Too many processes"
68 "Too many users"
69 "Disc quota exceeded"
70 "Missing file or filesystem"
71 "Item is not local to host"
72 "RPC structure is bad"
73 "RPC version is wrong"
74 "RPC program not available"
75 "Program version wrong"
76 "Bad procedure for program"
77 "No locks available"
78 "Function not implemented"
79 "Inappropriate file type or format"
80 "No message of desired type"
81 "Identifier removed"
82 "Out of STREAMS resources"
83 "System call timed out"
84 "Next message has wrong type"
85 "Error in protocol"
86 "No message on stream head read queue"
87 "File descriptor not associated with a stream"
88 "Tells open to clone the device"
89 "Mounting a dirty file-system without force"
90 "Duplicate package name"
91 "Version mismatch"
92 "Unresolved package name"
93 "Unresolved symbol name"
94 "Operation canceled"
95 "Cannot start operation"
96 "Inappropriate operation for file type"
97 "Operation in progress"
98 "Too many timers"
99 "Function not implemented"
100 "Internal AIO operation complete"
116 "Invalid byte sequence"
