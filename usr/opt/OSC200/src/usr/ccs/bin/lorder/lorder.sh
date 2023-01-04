#!/bin/sh
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
#	@(#)lorder.sh	3.1	(ULTRIX/OSF)	2/26/91
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
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0


trap "rm -f /tmp/$$sym?ef; exit" 0 1 2 13 15
case $# in
0)      echo usage: lorder file ...
        exit ;;
1)      case $1 in
        *.o)    set $1 $1
        esac
esac


nm -B $* | sed '
        /^$/d
        /:$/{
                /\.o:/!d
                s/://
                h
                s/.*/& &/
                p
                d
        }
        /[TD] /{
                s/.* //
                G
                s/\n/ /
                w '/tmp/$$symdef'
                d
        }
        /C /{
                s/.* //
                G
                s/\n/ /
                w '/tmp/$$symcef'
                d
        }
        s/.* //
        G
        s/\n/ /
        w '/tmp/$$symref'
        d
'
sort /tmp/$$symdef -o /tmp/$$symdef
sort /tmp/$$symcef -o /tmp/$$symcef
sort /tmp/$$symref -o /tmp/$$symref
{
        join /tmp/$$symref /tmp/$$symdef
        join /tmp/$$symref /tmp/$$symcef
        join /tmp/$$symcef /tmp/$$symdef
} | sed 's/[^ ]* *//'
