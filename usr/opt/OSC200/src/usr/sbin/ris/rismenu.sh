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
# @(#)$RCSfile: rismenu.sh,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/04/17 16:14:20 $
# 
#	ABSTRACT:  This script is the user interface for the RIS utility.

# Check and set menu-control variables:
# 	SwEx	At least one RIS area contains installed software
# 	ClEx	At least one RIS client is registered

[ -d /$RIS/ris*.*/product_* ] && SwEx=y
[ -s $RISDB ] && ClEx=y

#
# Execution starts here
#
while :
do
	MMs=" " MMd=" " MMa=" " MMr=" " MMm=" " MMl=" "
	[ $SwEx ] && MMs=s && MMd=d && MMa=a
	[ $ClEx ] && MMr=r && MMm=m && MMl=l

	echo "\n*** RIS Utility Main Menu ***"
	[ "$SwEx$ClEx" = yy ] || echo "
Choices without key letters are not available."
	Dialog "
    i) Install software products
    $MMs) Show software products in remote installation environments
    $MMd) Delete software products
    $MMa) Add a client
    $MMr) Remove a client
    $MMm) Modify a client
    $MMl) List registered clients
    x) Exit

Enter your choice" ANSWER
	ANSWER=`ToLower $ANSWER`
	case $ANSWER in
	[iarmlds] )
		Cklock_ris || continue
	esac
	case $ANSWER in
	[i] )	trap 'UnlockAndExit' 1 2 3
		Lock_ris
		Instsoft && SwEx=y
		Unlock
		prodnames $RISROOT
		;;

	[a] )	if [ $MMa ]
		then
			Addclient && ClEx=y
		else
			BadChoice $ANSWER
		fi
		;;

	[r] )	if [ $MMr ]
		then
			Rmclient
			ClEx=
			[ -s $RISDB ] && ClEx=y
		else
			BadChoice $ANSWER
		fi
		;;

	[m] )	if [ $MMm ]
		then
			Modclient
		else
			BadChoice $ANSWER
		fi
		;;

	[l] )	if [ $MMl ]
		then
			Listclients
		else
			BadChoice $ANSWER
		fi
		;;

	[s] )	if [ $MMs ]
		then
			Showprod
		else
			BadChoice $ANSWER
		fi
		;;

	[d] )	if [ $MMd ]
		then
			trap 'UnlockAndExit' 1 2 3
			Lock_ris
			Deleteprod
			Unlock
			ClEx= ; SwEx=
			[ -s $RISDB ] && ClEx=y
			[ -d /$RIS/ris*.*/product_* ] && SwEx=y
		else
			BadChoice $ANSWER
		fi
		;;

	[x] )	exit 0 	;;

	* )	BadChoice $ANSWER ;;
	esac
done
