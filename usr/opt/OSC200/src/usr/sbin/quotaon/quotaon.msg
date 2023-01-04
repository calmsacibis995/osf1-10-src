$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0

$ Messages for the quotaon command
$quote "
$set MS_QUOTAON

NAME		"Name must be quotaon or quotaoff not %s\n"
SUSER		"Must be root to execute quotaon or quotaoff\n"
NOFSTAB		"%s not found in fstab\n"
USE1		"Usage:\n\t%s [-g] [-u] [-v] -a\n"
USE2		"\t%s [-g] [-u] [-v] filesys ...\n"
QOFF		"quotaoff: "
QTOFF		"%s: quotas turned off\n"
QON		"quotaon: using %s on"
QTON		"%s: %s quotas turned on\n"
NOMNT		"%s: not mounted\n"
RDONLY		"%s: mounted read-only\n"

$set MS_QUOTAON_SEC

AUTH		"%s: need sysadmin authorization\n"
PRIV		"%s: insufficient privileges\n"
