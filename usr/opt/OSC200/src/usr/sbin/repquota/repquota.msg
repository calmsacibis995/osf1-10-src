$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0

$ Messages for the edquota command
$quote "
$set MS_REPQUOTA

NOFSTAB		"%s not found or not properly enabled for quotas in fstab\n"
USE1		"repquota [-v] [-g] [-u] -a"
USE2		"repquota [-v] [-g] [-u] filesys ..."
NOQUOTA		"*** Warning: Quotas are not compiled into this kernel\n"
REPORT		"*** Report for %s quotas on %s (%s)\n"
LIMITS		"                        Block limits               File limits\n"
MSGHDU		"User            used    soft    hard  grace    used  soft  hard  grace\n"
MSGHDG		"Group           used    soft    hard  grace    used  soft  hard  grace\n"
DAYS		"%ddays"
SUSER		"Must be root to execute repquota\n"

$set MS_REPQUOTA_SEC

AUTH		"%s: need sysadmin authorization\n"
PRIV		"%s: insufficient privileges\n"
