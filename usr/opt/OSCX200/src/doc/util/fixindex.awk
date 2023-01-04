# 
# *****************************************************************
# *                                                               *
# *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
# *                                                               *
# *   All Rights Reserved.  Unpublished rights  reserved  under   *
# *   the copyright laws of the United States.                    *
# *                                                               *
# *   The software contained on this media  is  proprietary  to   *
# *   and  embodies  the  confidential  technology  of  Digital   *
# *   Equipment Corporation.  Possession, use,  duplication  or   *
# *   dissemination of the software and media is authorized only  *
# *   pursuant to a valid written license from Digital Equipment  *
# *   Corporation.                                                *
# *                                                               *
# *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
# *   by the U.S. Government is subject to restrictions  as  set  *
# *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
# *   or  in  FAR 52.227-19, as applicable.                       *
# *                                                               *
# *****************************************************************
#
# HISTORY
# 
BEGIN {
	FS = ":";
	BD = "\\s+1\\fB";
	ED = "\\fP\\s-1";
}

NR == 1 {
	if ($3 != "")
		printf(".Ib \"%s\"\n", $2);
	major = $2;
	minor = $3;
	if ($4 == "@DEF@") {
		pagelist = BD $1 ED;
	}
	else {
		pagelist = $1;
	}
	pageno = $1;
	oldpageno = $1;
	oldpagelist = "";
}

NR != 1 {
	if ($2 == major && $3 == minor)		# neither has changed
	{
		if ($1 != pageno) {		# new page number, append
			oldpageno = $1;
			oldpagelist = pagelist;
			if ($4 == "@DEF@") {
				pagelist = pagelist ", " BD $1 ED;
			}
			else {
				pagelist = pagelist ", " $1;
			}
		}
		else {				# old page, but check for def
               	        if ($4 == "@DEF@") {
                            if (pageno == oldpageno) {
                                if (oldpagelist != "")
                                        oldpagelist = oldpagelist ", "
			    }
                            pagelist = oldpagelist BD $1 ED;
                        }
                }
	}
	else					# one has changed
	{
		if (minor != "")		# dump full record
			printf(".I< \"%s\" \"%s\" \"%s\"\n", major, minor, pagelist);
		else
			printf(".I> \"%s\" \"%s\"\n", major, pagelist);
		if ($4 == "@DEF@") {		# restart pagelist
			pagelist = BD $1 ED;
		}
		else {
			pagelist = $1;
		}
		oldpagelist = "";
		oldpageno = $1;
		if ($2 != major && $3 != "")	# major has changed, minor not null
		printf(".Ib \"%s\"\n", $2);
	}
	major = $2;
	minor = $3;
	pageno = $1;
}

END {
	if (minor != "")			# dump full record
		printf(".I< \"%s\" \"%s\" \"%s\"\n", major, minor, pagelist);
	else
		printf(".I> \"%s\" \"%s\"\n", major, pagelist);
}
