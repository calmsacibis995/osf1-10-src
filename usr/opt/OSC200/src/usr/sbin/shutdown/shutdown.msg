$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0
$quote "
$set MS_SHUTDOWN

UNKNOWN		"shutdown: '%c' - unknown flag\n"
USAGE		"Usage: shutdown [ -krhfn ] shutdowntime [ message ]\n"
INCOMP		"shutdown: Incompatible switches 'fast' & 'nosync'\n"
NO_ROOT		"NOT super-user\n"
MSG_AT		"Shutdown at %5.5s (in "
MSG_TIME_1	"%d hour%s "
MSG_TIME_2	"s"
MSG_TIME_3	"%d minute%s) "
MSG_PID		"[pid %d]\n"
MSG_LOG		"shutdown: /var/adm/utmp"
MSG_FINAL	"FINAL "
ARRIVED		"\n\007\007System shutdown time has arrived\007\007\n"
MSG_BY		"%s by %s: %s"
YOURSELF	"but you'll have to do it yourself\n"
MSG_REBOOT	"REBOOT"
MSG_HALT	" HALT"
NO_FSCK		" -l %s (without fsck's)\n"
KILL_HUP	"kill -HUP 1\n"
MSG_WAIT	"That must be tomorrow\nCan't you wait till then?\n"
BAD_TIME	"Bad time format\n"
SYS_MSG		 "\r\n\007\007\t*** %sSystem shutdown message from %s@%s ***\r\n\n"
DOWN_1		"System going down at %5.5s\r\n"
DOWN_2		"System going down in %d minute%s\r\n"
DOWN_3		"System going down in %d second%s\r\n"
DOWN_4		"System going down IMMEDIATELY\r\n"
NO_LOGINS	"\n\nNO LOGINS: System going down at %5.5s\n\n"

$set MS_SHUTDOWN_SEC

SYS_AUTH	"shutdown: need sysadmin authorization\n"
INSUFF_PRIV	"shutdown: insufficient privileges\n"
