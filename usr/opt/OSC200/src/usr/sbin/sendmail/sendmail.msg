 
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
$ @(#)$RCSfile: sendmail.msg,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/10/07 22:37:57 $
$
$ Message for the sendmail command
$
$ Please don't translate "sendmail".
$ 
$ translators should note that there is a help file, sendmail.hf,
$ which gets installed in /usr/share/lib.
$
$set MS_SENDMAIL
$quote "

$ messages for alias.c
$
AL_CANTOPEN	"Cannot open %s"
AL_REBUILD	"rebuilding alias database"
AL_OFD		"alias database out of date"
AL_AUTO		"alias database rebuilt by %s"
AL_LOCK		"Alias file is already being rebuilt"
AL_CANTMAKE	"cannot make %s"
AL_NONC		"Non-continuation line starts with space"
AL_MC		"missing colon"
AL_IN		"illegal alias name"
AL_BAD		"%s... bad address"
AL_NONL		"cannot alias non-local names"
AL_SUM		"%d aliases, longest (%s) %d bytes, %d bytes total"
AL_NOHOME	"forward: no home directory"

$ messages for collect.c
$
CO_CANTCR	"Cannot create %s"
CO_ENTER	"Enter mail, end with \".\" on a line by itself"
CO_CLOSE	"collect: unexpected close on connection from %s: %m\n"
CO_CL2		"collect: unexpected close, from=%s"
CO_REOP		"Cannot reopen %s"
CO_MAILDEL	"\nMAIL DELETED BECAUSE OF LACK OF DISK SPACE\n\n"
CO_CANTWR	"collect: Cannot write %s"
CO_SPACE	"452 Out of disk space for temp file"
$ do not remove the 452 at start of this string

$ message for conf.c
$
CF_WHO		"Can't find your login name."

$ messages for daemon.c
$
DM_UKN		"server \"smtp\" unknown"
DM_SOCK		"getrequests: can't create socket"
DM_NOCONN	"cannot get connection"
DM_BIND		"getrequests: cannot bind"
DM_LSTN		"getrequests: cannot listen"
DM_REJCON	"rejecting connections: load average: %.2f"
DM_ACCCON	"accepting connections"
DM_NOFORK	"daemon: cannot fork"
DM_INV		"Invalid numeric domain spec \"%s\""
DM_SRVUKN	"makeconnection: server \"smtp\" unknown"

$ messages for deliver.c
$ do not translate $u, and the 554 at the beginning of DL_UKNMAILER
$
DL_TOMANY	"Too many parameters to %s before $u"
DL_SMTP		"SMTP style mailer, and this sendmail doesn't support it"
DL_TOLRG	"Message is too large; %ld bytes max"
DL_OVF		"deliver: pv overflow after $u for %s"
DL_CON1		"Connecting to %s (%s)..."
DL_CON2		"Connecting to %s (%s)..."
DL_CNTSND	"Cannot send message for %s"
DL_DIED		"mailer %s died with signal %o"
DL_NOFORK	"openmailer: cannot fork"
DL_NODUP	"Cannot dup to zero!"
DL_EXEC		"Cannot exec %s"
DL_UKNMAILER	"554 unknown mailer error %d"
DL_NOBANG	"No ! in UUCP! (%s)"
DL_NOPEN	"putbody: Cannot open %s for %s from %s"
DL_NOMSG	"<<< No Message Collected >>>"
DL_RDERR	"putbody: read error"
DL_WRERR	"putbody: write error"
DL_NOPEN2	"mailfile: Cannot open %s for %s from %s"
DL_HOPS		"sendall: too many hops %d (%d max): from %s, to %s"
DL_DEL		"deliverable"

$ message for envelope.c
$
EN_UNPARSE	"from=%s unparseable, received from %s"
EN_UPARSE2	"Unparseable username %s wants from=%s"
EN_PPOST	"setsender: can't even parse postmaster!"
EN_PRESCAN	"cannot prescan from (%s)"

$ messages for err.c
$
$ leave spaces before and after " during "  and " with "
ER_DURING	" during "
ER_WITH		" with "
ER_DOWN		"Host %s is down"
ER_CONREF	"Connection refused by %s"
ER_LUP		"Host Name Lookup Failure"
ER_ERROR	"Error %d"

$ messages for headers.c
$
HD_SNTX		"chompheader: syntax error, line \"%s\""

$ messages for main.c
$ additional message added at end
$
MN_ENTER	"main: reentered!"
MN_PERM		"Permission denied"
MN_NODM		"Daemon mode not implemented"
MN_NOSMTP	"I don't speak SMTP"
MN_MODE		"Invalid operation mode %c"
MN_NOFRM	"No \"from\" person"
MN_FRM2		"More than one \"from\" person"
MN_BADF		"Bad -F flag"
MN_BADHOP	"Bad hop count (%s)"
MN_NOQ		"I don't know about queues"
MN_NOLOCAL	"No local mailer defined"
MN_NOPROG	"No prog mailer defined"
MN_CANTCD	"Can't change to the queue dorectory (%s)"
MN_NOQ2		"No queue to print"
MN_TEST		"ADDRESS TEST MODE\nEnter <ruleset> <address>\n"
MN_NOR3		"[Note: No initial ruleset 3 call]\n"
MN_NOFORK	"daemon: cannot fork"
MN_NOTO		"Recipient names must be specified"
MN_NOFZ		"Cannot create freeze file: %s"
MN_FZWR		"Cannot write freeze file: %s"
MN_FZOPEN	"Cannot open frozen config file %s: %m"
MN_FZRD		"Cannot read frozen config file"
MN_FZREV	"Wrong version of frozen config file"
MN_MEM		"Cannot allocate memory to %x"
MN_RDFZ		"Cannot read frozen config file"
MN_VERSION	"Version %s\n"
MN_CNAME	"canonical name: %s\n"
MN_FPERS	"From person = \"%s\"\n"

$ messages for parseaddr.c
$
PA_RESLV	"cannot resolve name"
PA_TOLNG	"Address too long"
PA_ADLOOP	"Address causes rewrite loop: <%s>"
PA_RSLOOP	"Looping on ruleset %d, rule %d"
PA_RSLOOP2	"Loop in ruleset %d, rule %d (too many matches)"
PA_PAREN	"Unbalanced ')'"
PA_GR		"Unbalanced '>'"
PA_SCAN		"prescan: too many tokens"
PA_PAREN2	"Unbalanced '('"
PA_LT		"Unbalanced '<'"
PS_SLASH	"Unbalanced '\"'"
PA_REWRITE	"rewrite: ruleset %d: replacement out of bounds"
PA_TOLNG2	"rewrite: expansion too long"
PA_SCAN2	"rewrite: cannot prescan canonical hostname: %s"
PA_NONET	"buildaddr: no net"
PA_USR		"buildaddr: error: no user"
PA_MAILER	"buildaddr: unknown mailer %s"
PA_NHOST	"buildaddr: no host"
PA_NUSER	"buildaddr: no user"

$ messages for queue.c
$
QU_TMP		"queueup: cannot create temp file %s"
QU_FLOCK	"cannot flock(%s)"
QU_LOCKF	"cannot lockf(%s)"
QU_CR		"queueup: cannot create %s"
QU_FDOPEN   "queueup: cannot fdopen %s"
QU_SKIP		"Skipping queue run -- load average too high\n"
QU_RUN		"running queue: %s"
QU_OPEN		"orderq: cannot open \"%s\" as \".\""
QU_OP2		"orderq: cannot open %s"
QU_SKIP2	"\nSkipping %s\n"
QU_FORK		"dowork: cannot fork"
QU_NOCTRL	"readqf: no control file %s"
QU_LK		"%s: locked\n"
QU_RUN2		"\nRunning %s\n"
QU_OP3		"readqf: cannot open %s"
QU_LINE		"readqf(%s:%d): bad line \"%s\""
QU_EMPTY	"Mail queue is empty\n"
$
$ QU_REQ should be sigular, QU_REQ2 should be plural.
$ one or the other goes in the %s below
QU_NREQ		"\t\tMail Queue (%d %s"
QU_REQ		"request"
QU_REQ2		"requests"
QU_PRNT		", only %d printed"
QU_HDRV	")\n--QID-- --Size-- -Priority- ---Q-Time--- -----------Sender/Recipient-----------\n"
QU_HDR	")\n--QID-- --Size-- -----Q-Time----- ------------Sender/Recipient------------\n"
QU_NOCTRL2	" (no control file)"
QU_CANTCR	"queuename: Cannot create \"%s\" in \"%s\""

$ messages for readcf.c
$
RC_OPEN		"cannot open %s"
RC_RWLINE	"invalid rewrite line \"%s\""
RC_BADRS	"bad ruleset %d (%d max)"
RC_ELINE	"unknown control line \"%s\""
RC_TOOMANY	"too many %c lines, %d max"
RC_EQUAL	"`=' expected"
RC_RWSET	"invalid rewrite set, %d max"
RC_MAILERS	"too many mailers defined (%d max)"
RC_RESET	"(Resetting uid)\n"
RC_NEEDQ	"need QUEUE to set -odqueue"
RC_MODE		"Unknown delivery mode %c"

$ messages for recipient.c
$
RE_LOOP		"aliasing/forwarding loop broken"
RE_NOPROG	"Cannot mail directly to programs"
RE_DUP		"duplicate suppressed"
RE_INC		"Cannot mail directly to :include:s"
$ do not translate :include:s
RE_INC2		"including file %s"
RE_FILES	"Cannot mail directly to files"
RE_LOGIN	"sending to login name %s"
RE_STAT		"Cannot fstat %s!"
RE_OVFL		"address overflow"

$ messages for savemail.c
$
SM_JUNK		"Dumping junk mail"
SM_ROOT		"Cannot parse root!"
SM_MES		"\r\nMessage from %s...\r\n"
SM_MES2		"Errors occurred while sending mail.\r\n"
SM_OPEN		"Cannot open %s"
SM_TRANS	"Transcript of session is unavailable.\r\n"
SM_TRANS2	"Transcript follows:\r\n"
SM_DEAD		"Original message will be saved in dead.letter.\r\n"
SM_PPOST	"cannot parse postmaster!"
SM_DLVR		"Unable to deliver mail"
SM_RETURN	"Can't return mail to %s"
SM_SAVE		"Saving message in %s"
SM_STATE	"savemail: unknown state %d"
SM_HELP		"savemail: HELP!!!!"
SM_RECUR	"returntosender: infinite recursion on %s"
SM_TR		"  ----- Transcript of session is unavailable -----\n"
SMTR2		"   ----- Transcript of session follows -----\n"
SM_RETURN2	"\n   ----- Return message suppressed -----\n\n"
SM_UNSENT	"   ----- Unsent message follows -----\n"
SM_MH		"  ----- Message header follows -----\n"
SM_NOMSG	"  ----- No message was collected -----\n"
SM_ERR		"errbody: I/O error"

$ messaes for srvrsmtp.c
$ Do not translate "MAIL", "HELP", or "RCPT"
$
SV_LOST		"%s Lost input channel from %s"
SV_CONFIG	"Local configuration error, hostname not recognized as local"
SV_GREET	"%s Hello %s, pleased to meet you"
SV_SNDR		"Sender already specified"
SV_NEST		"Nested MAIL command"
SV_SOK		"Sender ok"
SV_ROK		"Recipient ok"
SV_UKN		"Addressee unknown"
SV_NEEDM	"Need MAIL command"
SV_NEEDR	"Need RCPT (recipient)"
SV_OK		"Ok"
SV_RESET	"Reset state"
SV_ALIAS	"Self destructive alias loop"
SV_OK2		"OK"
SV_CLOSE	"%s closing connection"
SV_VERBOSE	"Verbose mode"
SV_ONLY1	"Only one transaction"
SV_TRACK	"\"%s\" command from %s (%s)\n"
SV_UKNCMD	"Command unrecognized"
SV_ERR		"smtp: unknown code %d"
SV_SYNTX	"Syntax error"
SV_NOHLP	"HELP not implemented"
SV_NOTOP	"HELP topic unknown"
SV_ENDHLP	"End of HELP info"
SV_FORK		"%s: cannot fork"
SV_LOST2	"%s: lost child"
SV_CALLU	"Hello %s. why did you call yourself %s"
SV_USERU	"User Unknown"
SV_SUSERU	"<%s> User Unknown"
SV_ISALIAS	"<%s> is an alias"
SV_NOTALIAS	"<%s> is not an alias"



$ mesages for util.c
$
UT_MEM		"Out of memory!!"
UT_TIME		"timeout waiting for input from %s\n"

$ added message for main.c
$
MN_FZUNSUP	"Frozen configurations unsupported"
