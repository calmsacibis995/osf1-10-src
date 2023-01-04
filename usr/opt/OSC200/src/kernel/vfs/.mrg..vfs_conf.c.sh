#!/sbin/sh
#
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
# @(#)$RCSfile: .mrg..vfs_conf.c.sh,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/08/12 21:22:13 $
# 


MERGE_ROUTINE=DRI_Merge


#	-DRI_Merge
#		merge routine provided by the DRIs.
#
#	given: 	global variables $_FILE and $_NEWFILE
#	return: 0 if success
#		non-zero if failure
#
#	Note:	1) use MRG_Echo() to output additional messages.
#		2) see also /usr/share/lib/shell/libmrg for other available 
#		   global variables.

DRI_Merge()
{

	RET=0

        grep -q -i sysv_fs $_FILE &&
	{
		MRG_Echo "removing sysv_fs information"

        	diff -e $_FILE $_NEWFILE | ed - $_FILE ||
		{
                	MRG_Echo "\tfailed to remove sysv_fs information"
			RET=1
		}
	}

	return $RET
}


SHELL_LIB=${SHELL_LIB:-/usr/share/lib/shell}
. $SHELL_LIB/libmrg


[ "$CHECK_SYNTAX" ] || MRG_Merge "$@" 

