$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0
$quote "
$set MS_EDQUOTA

PERDEN		"edquota: permission denied\n"
USE1		"Usage: edquota [-u] [-p username] username ...\n"
USE2		"\tedquota -g [-p groupname] groupname ...\n"
USE3		"\tedquota [-u] -t\n", "\tedquota -g -t\n"
NOUSER		"%s: no such user\n"
NOGRP		"%s: no such group\n"
UNKTYPE		"%d: unknown quota type\n"
NOMEM		"edquota: out of memory\n"
WARN		"Warning: %s\n"
NOQUOTAS	"Quotas are not compiled into this kernel"
CREATEQ		"Creating quota file %s\n"
RDERR		"edquota: read error in "
EDQ		"edquota: "
PROC		"You have too many processes\n"
FORK		"fork"
QUOTAS		"Quotas for %s %s:\n"
QLIM1		 "%s: %s %d, limits (soft = %d, hard = %d)\n"
BUSE		"blocks in use:"
QLIM2		"%s %d, limits (soft = %d, hard = %d)\n"
IUSE		"\tinodes in use:"
TMP		"Can't re-read temp file!!\n"
FORM1		"%s: bad format\n"
FORM2		"%s: %s: bad format\n"
BLKUSE		" blocks in use: %d, limits (soft = %d, hard = %d)"
INOUSE		"\tinodes in use: %d, limits (soft = %d, hard = %d)"
ALLOC		"%s: cannot change current allocation\n"
TIME		"Time units may be: days, hours, minutes, or seconds\n"
GRACE		"Grace period before enforcing soft limits for %ss:\n"
BGRACE		"%s: block grace period: %s, "
FGRACE		"file grace period: %s\n"
BFGRACE		" block grace period: %d %s file grace period: %d %s"
DAYS		"days"
HOURS		"hours"
MINUTES		"minutes"
SECONDS		"seconds"
SEC		"second"
MINS		"minute"
HR		"hour"
DAY		"day"
BADUNIT		"%s: bad units, specify %s\n"
DHMS		"days, hours, minutes, or seconds"

$set MS_EDQUOTA_SEC

AUTH		"%s: need sysadmin authorization\n"
PRIV		"%s: insufficient privileges\n"
