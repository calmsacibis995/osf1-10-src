
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
# @(#)$RCSfile: .mrg..Makefile,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/10/29 19:50:23 $
# 
MERGE_ROUTINE=DRI_Merge

#	-DRI_Merge
#		merge routine provided by the DRIs.
#
#	given: 	global variable $_FILE
#	return: 0 if success
#		non-zero if failure
#
#	Note:	1) use MRG_Echo() to output additional messages.
#		2) see also /usr/share/lib/shell/libmrg for other available 
#		   global variables.

DRI_Merge()
{
	RET=0

	#
	# If the merge has not been performed for ALIASES sed fix do it
	#
	grep -q \/:\/ $_FILE ||
	{ 
		# Do the merge for ALIASES sed fix

		MRG_Echo "adding ALIASES sed fix "

		#
		# 	Section to add the ALIASES sed fix
		#
		ed - $_FILE << _Add_ALIASES_sed_ 1>/dev/null
/aliases.time
/MAKEDBM
d
i
		$(SED) -e '/^[ 	]*#/d' -e 's/^[	 ][ 	]*/ /' $(ALIASES) \\
		| $(AWK) '/^[^ 	]*:/ { printf "\n"} {print}  END {print}' \\
		| $(SED) -e '/^[^ 	]*:/s/:/ /' -e '/[^ 	].*[^\\\]\$\$/s/\$\$/\\\\/' \\
		| $(MAKEDBM) - $(YPDBDIR)/$(DOM)/mail.aliases; \\
.
w
q
_Add_ALIASES_sed_
	
		[ "$?" = 0 ] ||
		{
			MRG_Echo "\tfailed to add ALIASES sed fix"
			RET=1
		}
	}	

	return $RET
}
SHELL_LIB=${SHELL_LIB:-/usr/share/lib/shell}
. $SHELL_LIB/libmrg

# define your merge routine here. 

# OPTIONAL: for 3-way merge, make MRG_SetCheck calls here.

[ "$CHECK_SYNTAX" ] || MRG_Merge "$@" 

