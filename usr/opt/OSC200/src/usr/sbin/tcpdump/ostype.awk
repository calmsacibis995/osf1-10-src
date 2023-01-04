# 
# HISTORY
# $Log:$
# $EndLog$
# 
# @(#)$RCSfile$ $Revision$ (DEC) $Date$
# 
# @(#) $Header: ostype.awk,v 1.4 92/01/27 12:36:42 mccanne Exp $ (LBL)

BEGIN {
	os = "UNKNOWN"
}

$0 ~ /^Sun.* Release 3\./ {
	os = "sunos3"
}

$0 ~ /^SunOS.* Release 4\./ {
	os = "sunos4"
}

$0 ~ /^4.[1-9]\ ?BSD / {
	os = "bsd"
}

# XXX need an example Ultrix motd
$0 ~ /[Uu][Ll][Tt][Rr][Ii][Xx]/ {
	os = "ultrix"
}

END {
	print os
}
