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
# @(#)$RCSfile: BitTest.sh,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/15 18:49:11 $
# 

:       BitTest
#
#       Given:  $1 = decimally-expressed integer
#               $2 = bit to test, where bit 0 is 2^0
#
#       Does:   Examines set/reset state of the specified bit
#
#       Return: Status = 0 if bit is SET
#                        1 if bit is RESET

BitTest ()
{
        return `echo | awk "{ print (int($1/exp(log(2)*$2))%2+1)%2 }"`
}
