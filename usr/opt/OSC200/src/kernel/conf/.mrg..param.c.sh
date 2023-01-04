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


MERGE_ROUTINE=DRI_Merge


#################################################
# ADDSVR4  FUNCTION                      	#
#################################################
AddSVR4()
{
	grep -q SVR4_RT_MAX $_FILE ||
	{
		MRG_Echo "adding SVR4 enhancements"
	
		echo "
/*
 * This is the size in bytes of the kernel memory area that is available
 * to the SVR4 versions of kmem_alloc() and kmem_zalloc().  See ddi_init()
 * for details.
 */
int ddi_map_size = 0x10000;

#if RT_SCHED_RQ
/*
 * The quantum table, indexed by SVR4 RT priority.  Note that this table must
 * be editted when the number of RT priorities change because of changes in
 * SVR4_RT_MAX and/or SVR4_RT_MIN.  The priorities are given as MACH
 * priorities, and (SVR4_RT_MAX < SVR4_RT_MIN) and (SVR4_RT_MIN <=
 * BASEPRI_HIGHEST-1).
 */

#define SVR4_RT_MAX	(0)
#define SVR4_RT_MIN	(BASEPRI_HIGHEST-1)

int svr4_rt_max	= SVR4_RT_MAX;
int svr4_rt_min	= SVR4_RT_MIN;

struct rt_default_quantum rt_default_quantum[SVR4_RT_MIN - SVR4_RT_MAX + 1] =
{
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
};
#endif	/* RT_SCHED_RQ */" >> $_FILE

		return $?
	}

	return 0 
}


#################################################
# ADDRT_SCHED_RQ  FUNCTION                      #
#################################################
AddRT_SCHED_RQ()
{
	grep -q '#if\(.*\)RT_SCHED_RQ' $_FILE ||
	{
		MRG_Echo "adding RT_SCHED_RQ include files"

		ed - $_FILE << END 1>/dev/null
/^#if RT_PREEMPT
.1
.1
a

#include <rt_sched_rq.h>
#if RT_SCHED_RQ
#include <sys/rtpriocntl.h>
#include <kern/sched.h>
#endif
.
w
q
END
		return $?
	}

	return 0 
}


#################################################
# ADDSEGMENTATION FUNCTION			#
#################################################
AddSegmentation()
{
	grep -q SEGMENTATION $_FILE || 
	{
		MRG_Echo "adding SEGMENTATION"

		ed - $_FILE << END 1>/dev/null
/VPAGEMAX/a
		SEGMENTATION,		/* Segmentation on or off */
.
w
q
END
		return $?
	}

	return 0
}


#################################################
# ADD vm_initial_limit_rss FUNCTION		#
#################################################
AddVm_initial_limit_rss()
{
	grep -q vm_initial_limit_rss $_FILE ||
	{
		MRG_Echo "adding vm_initial_limit_rss"

		ed - $_FILE << END 1>/dev/null
/vm_initial_limit_core/a
struct rlimit vm_initial_limit_rss = { DFLRSS, MAXRSS };
.
w
q
END
		return $?
	}

	return 0
}


#################################################
# ADD vm_initial_limit_vas FUNCTION		#
#################################################
AddVm_initial_limit_vas()
{
	grep -q vm_initial_limit_vas $_FILE ||
	{
		MRG_Echo "adding vm_initial_limit_vas"

		ed - $_FILE << END 1>/dev/null
/vm_initial_limit_rss/a
struct rlimit vm_initial_limit_vas = { MAXVAS, MAXVAS };
.
w
q
END
		return $?
	}

	return 0
}


#################################################
# FIXWRITEIOKLUSTER FUNCTION			#
#################################################
FixWriteioKluster()
{

	grep -q VM_WRITEIO_KLUSTER $_FILE && 
	{
		MRG_Echo "modifying WRITEIO_KLUSTER definition"

		ed - $_FILE << END 1>/dev/null
/VM_WRITEIO_KLUSTER/s/VM_WRITEIO_KLUSTER/WRITEIO_KLUSTER/
w
q
END
		return $?
	}

	return 0
}


#################################################
# MODIFYSWAPBUFFER 				#
#						#
#	This modifies the vm_tune structure	#
#	as follows:				#
#						#
#	change SWAPBUFFER to SYNCSWAPBUFFERS	#
#	add    ASYNCSWAPBUFFERS			#
#	add    INSWAPPEDMIN			#
#						#
#################################################
ModifySwapBuffer()
{
	grep -q SYNCSWAPBUFFER $_FILE ||
	{
		MRG_Echo "modifying SWAPBUFFER"

		ed - $_FILE << END 1>/dev/null
/SWAPBUFFERS/d
i
		SYNCSWAPBUFFERS,	/* Maximum synchronous swap buffers */
.

/SYSWIREDPERCENT/s/SYSWIREDPERCENT	/SYSWIREDPERCENT,/

/SYSWIREDPERCENT/a
		ASYNCSWAPBUFFERS,	/* Maximum asynchronous swap buffers */
		INSWAPPEDMIN		/* minimum inswapped ticks */
.
w
q
END
		return $?
	}

	return 0
}


#########################################################
# This section relocates the #include of vm_tune.h	#
#							#
# The include has to occur BEFORE vm_initial_limit_vas	#
# is defined.						#
#							#
#########################################################
Relocate_vm_tune_header()
{
	MRG_Echo "modifying inclusion of vm_tune.h"
	ed - $_FILE << END 1>/dev/null
/#include <vm\/vm_tune.h>/d
/#include <sys\/vmparam.h>/a
#include <vm/vm_tune.h>
.
w
q
END
	return $?
}


#########################################################
# This section adds the new calculation of nclists      #
#							#
#							#
#########################################################
Add_new_nclist_calc()
{
	grep -q MAXCLISTS $_FILE ||
	{
		MRG_Echo "modifying nclist calc"

		ed - $_FILE << END 1>/dev/null
/rtpriocntl
+2
a
#include <lat.h>
#include <pty.h>
.
/nclist
d
i
#if defined(NCLIST)
#define MAXCLISTS  NCLIST
#else
#if LAT || NPTY
#define MAXCLISTS  60 + (12 * MAXUSERS)
#else
#define MAXCLISTS  120
#endif
#endif
int	nclist = MAXCLISTS;
.
w
q
END
		return $?
	}

	return 0
}
Delete_mbclusters()
{
# Note this is "deleted" for alpha only, by ifdef'ing
	grep -q -e "int[ 	]*nmbclusters.*NMBCLUSTERS" $_FILE &&
	{
	ed - $_FILE <<END 1>/dev/null
/int[ 	]*nmbclusters.*NMBCLUSTERS
i
#ifndef __alpha
#endif	/* ifndef __alpha */
.
.m.+1
w
q
END
		
	}
}

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

	# The merging message and the error message will only be printed once
	# if necessary, even if multiple merges or errors occur respectively.
	# When error(s) occur, the remainder merges will be attempted.

	AddSegmentation ||
	{
		MRG_Echo "\tfailed to add SEGMENTATION"
		RET=1
	}

	FixWriteioKluster ||
	{
		MRG_Echo "\tfailed to modify WRITEIO_KLUSTER definition"
		RET=1
	}

	AddRT_SCHED_RQ ||
	{
		MRG_Echo "\tfailed to add RT_SCHED_RQ include files"
		RET=1
	}

	AddSVR4 ||
	{
		MRG_Echo "\tfailed to add SVR4 enhancements"
		RET=1
	}

	ModifySwapBuffer ||
	{
		MRG_Echo "\tfailed to modify SWAPBUFFER"
		RET=1
	}

	AddVm_initial_limit_rss ||
	{
		MRG_Echo "\tfailed to add vm_initial_limit_rss"
		RET=1
	}

	AddVm_initial_limit_vas ||
	{
		MRG_Echo "\tfailed to add vm_initial_limit_vas"
		RET=1
	}

	Relocate_vm_tune_header ||
	{
		MRG_Echo "\tfailed to relocate vm/vmtune.h include file"
		RET=1
	}
	Add_new_nclist_calc ||
	{
		MRG_Echo "\tfailed to add new nclist calc"
		RET=1
	}

	Delete_mbclusters ||
	{
		MRG_echo "\tfailed to delete nmbclusters definition"
		RET=1
	}

	return $RET
}


SHELL_LIB=${SHELL_LIB:-/usr/share/lib/shell}
. $SHELL_LIB/libmrg


[ "$CHECK_SYNTAX" ] || MRG_Merge "$@" 

