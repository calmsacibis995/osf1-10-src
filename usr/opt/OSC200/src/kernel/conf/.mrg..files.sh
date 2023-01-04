#!/sbin/sh
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
# @(#)$RCSfile: .mrg..files.sh,v $ $Revision: 1.1.6.4 $ (DEC) $Date: 1993/08/12 21:22:03 $
#
#


MERGE_ROUTINE=DRI_Merge


:       -DeleteFiles
#               remove an entry
#
#
DeleteFiles()
{(
	ENTRYID=$1

	grep -v '^'$ENTRYID $_FILE > $_TMPFILE
	mv $_TMPFILE $_FILE

	return $?
)}


:       -UpdateFiles
#               updates an entry
#
UpdateFiles()
{(
        # remove the entry...
        DeleteFiles "$@"
        echo "$1\t\t\t\c" >> $_FILE
        shift
        echo "$*" >> $_FILE

	return $?
)}


#	-DRI_Merge
#		merge routine provided by the DRIs.
#
#	given: 	global variable $_FILE $_TMPFILE
#	return: 0 if success
#		non-zero if failure
#
#	Note:	1) use MRG_Echo() to output additional messages.
#		2) see also /usr/share/lib/shell/libmrg for other available 
#		   global variables.

DRI_Merge()
{

        COMMENT="#"
        MRGDATA=./.mrg..files.dat

        [ -f $MRGDATA ] ||
        {
                MRG_Echo "no merge data file. can not merge"
                return 1
        }

        LINE=1
        cat $MRGDATA | while read ENTRY
        do
                # if blank line, continue with next line
                [ "$ENTRY" ] || continue

                set xx $ENTRY
                shift
                KEY=$1
                shift

                case "$KEY" in
                U)      # update entry
                        UpdateFiles "$@" ||
				> $_FILE.failmark
                        ;;
                D)      # delete entry
                        DeleteFiles "$@" ||
				> $_FILE.failmark
                        ;;
                ${COMMENT}*)
                        # if a comment, continue with next line
                        ;;
		*)	. $SHELL_LIB/Error
                      	Error "unrecognized key '$KEY', line $LINE"
			> $_FILE.failmark
                        ;;
                esac

                LINE=`expr $LINE + 1`
        done

	[ -f $_FILE.failmark ] &&
	{
		rm -f $_FILE.failmark
		return 1
	}

	return 0 
}


SHELL_LIB=${SHELL_LIB:-/usr/share/lib/shell}
. $SHELL_LIB/libmrg


[ "$CHECK_SYNTAX" ] || MRG_Merge "$@" 

