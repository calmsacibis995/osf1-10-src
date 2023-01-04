#! /bin/sh
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
#	@(#)runacct.sh	3.1	(ULTRIX/OSF)	2/26/91
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

#

#
# COMPONENT_NAME: (CMDACCT) Command Accounting
#
# FUNCTIONS: none
#
# ORIGINS: 3, 9, 27
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# runacct.sh	1.3  com/cmd/acct,3.1,8943 10/24/89 10:59:18
#

#       "nitely accounting shell, should be run from cron (adm) at 4am"
#       "does process, connect, disk, queue accounting and fee accounting"
#       "prepares command and line usage summaries, etc."
#	"shell is restartable and provides reasonable diagnostics"

# *** NOTE: ***
# Uncomment the following line to mail error messages to root and adm
# MAILCOM = "mail root adm" 

MAILCOM=${MAILCOM:-':'}

PATH=/usr/sbin/acct:/bin:/usr/bin:/sbin
export PATH
_MIN_BLKS=500
_wtmp=/var/adm/wtmp
_adm=${ACCTDIR:-/var/adm}
_nite=${_adm}/acct/nite
_sum=${_adm}/acct/sum
_fsdev=`df ${_adm} | awk '/\/dev\// { print $1 }' | sed 's/^\/dev\///'`
_statefile=${_nite}/statefile
_active=${_nite}/active
_lastdate=${_nite}/lastdate
_date="`date +%m%d`"
if _errormsg=`dspmsg acct.cat 23 "\r\n************ ACCT ERRORS : see %s%s********\r\n" ${_active} ${_date}`; then :
else _errormsg="\r\n************ ACCT ERRORS : see ${_active}${_date}********\r\n"; fi

#        "make sure that 2 crons weren't started, or leftover problems"
cd ${_adm}
if [ -f ${_nite}/lock ] ; then
	if _lnkerr="`dspmsg acct.cat 24 '\r\n*********** 2 CRONS or ACCT PROBLEMS***********\r\n'`" 
	then :
	else _lnkerr="\r\n*********** 2 CRONS or ACCT PROBLEMS***********\r\n"
	fi
	echo "$_lnkerr" >&2
	echo "$_lnkerr" | $MAILCOM
	if dspmsg acct.cat 25 "ERROR: locks found, run aborted\n" >> ${_active}
	then :
	else echo "ERROR: locks found, run aborted\n" >> ${_active}
	fi
	exit 1
fi
date  > ${_nite}/lock
chmod 400 ${_nite}/lock

#       "Check for enough space in ${_fsdev} to do nitely accounting"
_blocks=`df ${_adm} | awk '/\/dev\// { print $4 }' `
if [ "$_blocks" -le $_MIN_BLKS ];then
	if MSG1=`dspmsg acct.cat 26 'runacct: Insufficient space on /dev/%1$s (%2$s blks); Terminating procedure\n' ${_fsdev} ${_blocks}`; then : 
	else MSG1="runacct: Insufficient space on /dev/${_fsdev} (${_blocks} blks); Terminating procedure\n" ; fi
	echo $MSG1 >&2 
	echo $MSG1 >> ${_active}
	echo $MSG1 | $MAILCOM
	rm -f ${_nite}/lock
	exit 1
fi


case $# in
0)
#	"as called by the cron each day"
	if test ! -r ${_lastdate} ; then
		echo "0000" > ${_lastdate}
	fi
	if test "${_date}" = "`cat ${_lastdate}`"; then
		echo "${_errormsg}" >&2
		echo "${_errormsg}" | $MAILCOM
		if dspmsg acct.cat 27 'ERROR: acctg already run for %1$s : check %2$s \n' `date` ${_lastdate} >> ${_active}; then :
		else ( echo "ERROR: acctg already run for \c"
		echo "`date` \c"
		echo ": check \c"
		echo ${_lastdate} ) >> ${_active}
		fi
		rm -f ${_nite}/lock
		mv ${_active} ${_active}${_date}
		exit 1
	fi
	echo ${_date} > ${_lastdate}
	echo "SETUP" > ${_statefile}
	nulladm ${_active}
	;;

1)
#	"runacct MMDD  (date)  will restart at current state"
	_date=$1
	if dspmsg acct.cat 29 'restarting acctg for %1$s at %2$s\n' ${_date} `cat ${_statefile}` >> ${_active}; then :
	else (echo "restarting acctg for \c"
		echo "${_date} at `cat ${_statefile}`" ) >> ${_active}; fi
	;;

2)
#	"runacct MMDD STATE  restart at specified state"
	_date=$1
	if dspmsg acct.cat 29 'restarting acctg for %1$s at %2$s\n' ${_date} $2 >> ${_active}; then :
	else (echo "restarting acctg for \c"
		echo ${_date} at $2 ) >> ${_active}; fi
	if dspmsg acct.cat 31 "previous state was %s\n" `cat ${_statefile}`  >> ${_active}; then :
	else ( echo "previous state was \c"
	 cat ${_statefile} ) >> ${_active}; fi
	echo "$2" > ${_statefile}
	;;
*)
	echo "${_errormsg}" >&2
	echo "${_errormsg}" | $MAILCOM
	if dspmsg acct.cat 32 "ERROR: runacct called with invalid arguments\n" > ${_active}; then :
	else echo "ERROR: runacct called with invalid arguments" > ${_active};
	fi
	rm -f ${_nite}/lock
	mv ${_active} ${_active}${_date}
	exit 1
	;;
esac


#	"processing is broken down into seperate, restartable states"
#	"the statefile is updated at the end of each state so that the"
#	"next loop through the while statement switches to the next state"

while [ 1 ]
do
case "`cat ${_statefile}`" in

SETUP)

cd ${_adm}
(date ; ls -l fee pacct* ${_wtmp}* ${_nite}/dacct qacct ) >> ${_active}

#	"switch current pacct file"
turnacct switch
_rc=$?
if test ${_rc} -ne 0; then
	echo "${_errormsg}" >&2
	echo "${_errormsg}" | $MAILCOM
	if dspmsg acct.cat 33 "ERROR: turnacct switch returned rc=%s\n" ${_rc} >> ${_active}; then :
	else ( echo "ERROR: turnacct switch returned rc=\c"
		echo "${_rc}" ) >> ${_active}; fi
	rm -f ${_nite}/lock
	mv ${_active} ${_active}${_date}
	exit 1
fi

#	" give pacct files unique names for easy restart "
for _i in pacct?*
do
	if test -r S${_i}.${_date} ; then
		echo "${_errormsg}" >&2
		echo "${_errormsg}" | $MAILCOM
		if dspmsg acct.cat 34 "ERROR: S%s.%s already exists\n" ${_i} ${_date} >> ${_active}; then :
		else ( echo "ERROR: S${_i}.${_date} \c"
		echo "already exists" ) >> ${_active}; fi
		if dspmsg acct.cat 35 "file setups probably already run\n" >> ${_active}; then :
		else echo "file setups probably already run" >> ${_active}; fi
		rm -f ${_nite}/lock
		mv ${_active} ${_active}${_date}
		exit 1
	fi
	mv ${_i} S${_i}.${_date}
done


#	"add current time on end"
if test -r ${_nite}/wtmp.${_date} ; then
	echo "${_errormsg}" >&2
	echo "${_errormsg}" | $MAILCOM
	if dspmsg acct.cat 36 "ERROR: S%s.%s already exists: run setup manually\n" ${_nite}/wtmp ${_date} > ${_active}; then :
	else ( echo "ERROR: ${_nite}/wtmp.${_date} \c"
	 	echo "already exists: run setup manually" ) > ${_active}; fi
	rm -f ${_nite}/lock
	mv ${_active} ${_active}${_date}
	exit 1
fi
cp ${_wtmp} ${_nite}/wtmp.${_date}
acctwtmp "runacct" >> ${_nite}/wtmp.${_date}
nulladm ${_wtmp}

if dspmsg acct.cat 37 "files setups complete\n" >> ${_active}; then :
else echo "files setups complete" >> ${_active}; fi
echo "WTMPFIX" > ${_statefile}
;;


WTMPFIX)
#	"verify the integrity of the wtmp file"
#	"wtmpfix will automatically fix date changes"
cd ${_nite}
nulladm tmpwtmp wtmperror
wtmpfix < wtmp.${_date} > tmpwtmp 2>wtmperror
if test $? -ne 0 ; then
	echo "${_errormsg}" >&2
	echo "${_errormsg}" | $MAILCOM
	if dspmsg acct.cat 38  "ERROR: wtmpfix errors see %s/wtmperror%s\n" ${_nite} ${_date}  >> ${_active}; then :
	else ( echo  "ERROR: wtmpfix errors see \c"
	echo "${_nite}/wtmperror${_date}" ) >> ${_active}; fi
	rm -f ${_nite}/lock
	mv ${_active} ${_active}${_date}
	mv wtmperror wtmperror${_date}
	exit 1
fi

if dspmsg acct.cat 39 "wtmp processing complete\n" >> ${_active}; then :
else echo "wtmp processing complete" >> ${_active}; fi 
echo "CONNECT1" > ${_statefile}
;;


CONNECT1)
#	"produce connect records in the ctmp.h format"
#	"the lineuse and reboots files are used by prdaily"
cd ${_nite}
nulladm lineuse reboots ctmp log
acctcon1 -t -l lineuse -o reboots < tmpwtmp  2> log |\
sort +1n +2 > ctmp 

# if the following test is true, then pnpsplit complained about
# the year and holidays not being up to date.  This used to be
# a fatal error, but now it will continue to process the accounting.
# 
if test -s log ; then 
	cat ${_nite}/log | $MAILCOM
	cat ${_nite}/log >> ${_active}${_date}
fi

echo "CONNECT2" > ${_statefile}
;;


CONNECT2)
#	"convert ctmp.h records in tacct records"
cd ${_nite}
nulladm ctacct.${_date}
acctcon2 < ctmp | acctmerg > ctacct.${_date}

if dspmsg acct.cat 40 "connect acctg complete\n" >> ${_active}; then :
else echo  "connect acctg complete" >> ${_active}; fi
echo "PROCESS" > ${_statefile}
;;


PROCESS)
#	"correlate Spacct and ptacct files by number"
#	"will not process Spacct file if corresponding ptacct exists"
#	"remove the ptacct file to rurun the Spacct file"
#	"if death occurs here, rerunacct should remove last ptacct file"

cd ${_nite}
for _Spacct in ${_adm}/Spacct*.${_date}
do
	_ptacct=`basename ${_Spacct} | sed 's/Sp/pt/'`
	if test -s ${_ptacct}; then
		if dspmsg acct.cat 41 "WARNING: accounting already run for %s\n" ${_Spacct} >> ${_active}; then :
		else ( echo  "WARNING: accounting already run for \c"
			echo "${_Spacct}" ) >> ${_active}; fi
		if dspmsg acct.cat 42 "WARNING: remove %s/%s to rerun\n" ${_nite} ${_ptacct}  >> ${_active}; then :
		else ( echo "WARNING: remove \c" 
			echo "${_nite}/${_ptacct} \c" 
			echo "to rerun" ) >> ${_active}; fi
	else
		nulladm ${_ptacct}
                acctprc1 ctmp < ${_Spacct} |\
			acctprc2 > ${_ptacct}
		if dspmsg acct.cat 43 "process acctg complete for %s\n" ${_Spacct} >> ${_active}; then :
		else ( echo "process acctg complete for \c" 
			echo "${_Spacct}" ) >> ${_active}; fi
	fi
done
if dspmsg acct.cat 44 "all process acctg complete for %s\n" ${_date} >> ${_active}; then :
else ( echo "all process acctg complete for \c" 
	echo "${_date}" ) >> ${_active}; fi
echo "MERGE" > ${_statefile}
;;


MERGE)
#	"merge ctacct and ptacct files together"
cd ${_nite}
acctmerg ptacct*.${_date} < ctacct.${_date} > daytacct

if dspmsg acct.cat 45 "tacct merge to create daytacct complete\n" >> ${_active}
then :
else echo "tacct merge to create daytacct complete" >> ${_active}; fi
echo "FEES" > ${_statefile}
;;


FEES)
cd ${_nite}
#	"merge in fees"
if test -s ${_adm}/fee; then
	cp daytacct tmpdayt
	sort +0n +2 ${_adm}/fee | acctmerg -i | acctmerg tmpdayt  > daytacct
	if dspmsg acct.cat 46 "merged fees\n" >> ${_active}; then :
	else echo "merged fees" >> ${_active}; fi
	rm -f tmpdayt
else
	if dspmsg acct.cat 47 "no fees\n" >> ${_active}; then :
	else echo "no fees" >> ${_active}; fi
fi
echo "DISK" > ${_statefile}
;;


DISK)
#	"the last act of any disk acct procedure should be to mv its"
#       "entire output file to dacct, where it will be picked up"
cd ${_nite}
if test -s dacct; then
	cp daytacct tmpdayt
	acctmerg dacct  < tmpdayt > daytacct
	if dspmsg acct.cat 48 "merged disk records\n" >> ${_active}; then :
	else echo "merged disk records" >> ${_active}; fi
	rm -f tmpdayt dacct
else
	if dspmsg acct.cat 49 "no disk records\n" >> ${_active}; then :
	else echo "no disk records" >> ${_active}; fi
fi
echo "QUEUEACCT" > ${_statefile}
;;

QUEUEACCT)
cd ${_nite}
if test -s ${_adm}/qacct ; then
	cp daytacct tmpdayt
	sort +0n +2 ${_adm}/qacct | acctmerg -i1,2,14 | \
		acctmerg tmpdayt >daytacct
	if dspmsg acct.cat 50 "merged queueing system records\n" >> ${_active}
	then :
	else echo "merged queueing system records" >> ${_active}; fi
	rm -f tmpdayt
	nulladm ${_adm}/qacct
else
	if dspmsg acct.cat 51 "no queueing system records\n" >> ${_active};
	then :
	else echo "no queueing system records" >> ${_active}; fi
fi
echo "MERGETACCT" > ${_statefile}
;;

MERGETACCT)
#	"save each days tacct file in sum/tacct.${_date}"
#	"if sum/tacct gets corrupted or lost, could recreate easily"
#	"the mounthly acctg procedure should remove all sum/tacct files"
cd ${_adm}/acct
cp nite/daytacct sum/tacct${_date}
if test ! -r sum/tacct; then
	if dspmsg acct.cat 52 "WARNING: recreating %s/sum/tacct \n" ${_adm} >> ${_active}; then :
	else echo "WARNING: recreating " >> ${_active}
		echo "${_adm}/sum/tacct " >> ${_active}
	fi
	nulladm sum/tacct
fi

#	"merge in todays tacct with the summary tacct"
cp sum/tacct sum/tacctprev
acctmerg sum/tacctprev  < sum/tacct${_date} > sum/tacct

if dspmsg acct.cat 53 "updated sum/tacct\n" >> ${_active}; then :
else echo "updated sum/tacct" >> ${_active}; fi
echo "CMS" > ${_statefile}
;;


CMS)
#	"do command summaries"
cd ${_adm}/acct
nulladm sum/daycms
if test ! -r sum/cms; then
	nulladm sum/cms
	if dspmsg acct.cat 54 "WARNING: recreating %s/sum/cms \n" ${_adm} >> ${_active}; then :
	else echo "WARNING: recreating " >> ${_active}
		echo "${_adm}/sum/cms " >> ${_active}
	fi
fi
cp sum/cms sum/cmsprev
acctcms ${_adm}/Spacct*.${_date}  > sum/daycms
acctcms -s sum/daycms sum/cmsprev  > sum/cms
acctcms -a -s sum/daycms | sed -n 1,56p  > nite/daycms
acctcms -a -s sum/cms | sed -n 1,56p  > nite/cms
lastlogin 
if dspmsg acct.cat 55 "command summaries complete\n" >> ${_active}; then :
else echo "command summaries complete" >> ${_active}; fi
echo "USEREXIT" > ${_statefile}
;;


USEREXIT)
#	"any installation dependant accounting programs should be run here"
cd ${_adm}
if [ -r siteacct ] ; then
	sh siteacct
fi
echo "CLEANUP" > ${_statefile}
;;


CLEANUP)
#	" finally clear files; could be done next morning if desired"
cd ${_adm}/acct
nulladm ${_adm}/fee
rm -f ${_adm}/Spacct*.${_date}
#	"put reports onto a file"
prdaily >> sum/rprt${_date};
rm -f nite/lock
rm -f nite/ptacct*.${_date} nite/ctacct.${_date}
rm -f nite/wtmp.${_date} nite/wtmperror${_date} nite/active${_date}
mv nite/tmpwtmp nite/owtmp
if dspmsg acct.cat 56 "system accounting completed at %s\n" `date` >> ${_active}
then :
else echo "system accounting completed at " >> ${_active}
	date  >>${_active}; fi
echo "COMPLETE" > ${_statefile}
exit 0
;;

*)
	echo "${_errormsg}" >&2
	echo "${_errormsg}" | $MAILCOM
	if dspmsg acct.cat 58 "ERROR: invalid state, check " >> ${_active}
	then :
	else echo "ERROR: invalid state, check " >> ${_active}; fi
	echo "${_statefile}" >> ${_active}
	rm -f ${_nite}/lock
	mv ${_active} ${_active}${_date}
	exit 1
	;;
esac
done


#	" runacct is normally called with no arguments from the cron"
#	" it checks its own locks to make sure that 2 crons or previous"
#	" problems have not occured"

#	" runacct uses the statefile to record its progress"
#	" each state updates the statefile upon completion"
#	" then the next loop though the while picks up the new state"

#	" to restart this shell,  check the active file for diagnostics"
#	" fix up any corrupted data (ie. bad pacct or wtmp files)"
#	" if runacct detected the error it removes the locks"
#	" remove the locks if necessary, otherwise runacct will complain"
#	" the lastdate file should be removed or changed"
#	" restart runacct at current state with:  runacct MMDD"
#	" to override the statefile: runacct MMDD STATE"


#	" if runacct has been executed after the latest failure"
#	" ie. it ran ok today but failed yesterday"
#	" the statefile will not be correct"
#	" check the active files and restart properly"

#	" if runacct failed in the PROCESS state, remove the last"
#	" ptacct file because it may not be complete"

#	" if shell has failed several days, do SETUP manually"
#	" then rerun runacct once for each day failed"
#	" could use fwtmp here to split up wtmp file correctly"

#	" normally not a good idea to restart the SETUP state"
#	" should be done manually, or just cleanup first"


#       " FILE USAGE:   $ACCTDIR directory"
#       " diskdiag      diagnostic output during execution of disk accounting"
#       " dtmp          output from the acctdusg program"
#       " fee           output from chargefee program, ASCII tacct records"
#       " pacct         active process accounting file"
#       " qacct         active queueing accounting file"
#       " pacct?        pacct files switched via turnacct"
#	" wtmp		active wtmp file"
#       " Spacct?.MMDD  pacct files for MMDD after SETUP, during runacct"

#       " following files in $ACCTDIR/acct/nite directory"
#	" active	place for all descriptive and error messages"
#       " activeMMDD    same as log after runacct detects an error"
#	" cms		acsii total command summary used by prdaily"
#       " ctacct.MMDD   connect tacct records for MMDD"
#       " ctmp          ctmp records from acctcon1"
#       " daycms        ASCII daily command summary used by prdaily"
#	" daytacct	total tacct records for this days accounting"
#       " dacct         disk tacct records produced by dodisk shell"
#	" fd2log	fd2 output for runacct ( see cron entry ) "
#	" lastdate	last day runacct ran in date +%m%d format"
#       " lineuse       tty line usage report used by prdaily"
#	" lock 		controls serial use of runacct"
#       " log           diagnostic output from acctcon1"
#       " logMMDD       same as log after runacct detects an error"
#       " reboots       list of reboots, start/end dates from wtmp"
#	" statefile	records progess of runacct"
#	" tmpwtmp	yesterdays wtmp corrected by wtmpfix"
#       " wtmp.MMDD     yesterdays wtmp file"
#	" wtmperror	place for wtmpfix error messages"
#       " wtmperror.MMDD same as wtmperror after runacct detects an error"

#       " following files in $ACCTDIR/acct/sum directory"
#	" cms		total cms file for current fiscal"
#	" cmsprev	total cms file without latest update"
#	" daycms	cms files for todays usage"
#	" loginlog	output of lastlogin used in prdaily"
#       " ptacct.MMDD   concatenated version of all pacct files for MMDD"
#       " rprtMMDD      output of prdaily program"
#	" tacct		total tacct file for current fiscal"
#	" tacctprev	total tacct file without latest update"
#       " tacctMMDD     tacct file for day MMDD"
#	" wtmp.MMDD	saved copy of wtmp for MMDD"

#       " following files in $ACCTDIR/acct/fiscal directory"
#       " cms?          total cms for fiscal ?"
#       " fiscrpt?      report similar to prdaily for fiscal ?"
#       " tacct?        total accounting file for fiscal ?"
