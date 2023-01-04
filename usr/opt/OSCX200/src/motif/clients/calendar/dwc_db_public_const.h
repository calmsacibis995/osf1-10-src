#ifndef _dwc_db_public_const_h_
#define _dwc_db_public_const_h_
/* DWC_DB_PUBLIC_CONST.H "V3.0-04"  */
/* $Header$ */
/*
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**++
**  FACILITY:
**
**	DECwindows Calendar; database access routines
**
**  AUTHOR:
**
**	Per Hamnqvist, 01-May-1989
**
**  ABSTRACT:
**
**	This module defines the public (for UI) data structures of
**	the DECwindows calendar database routines.
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**  MODIFICATION HISTORY:
**
** V3.0-004 Paul Ferwerda					    21-Nov-1990
**		Converted from SDL to make Ultrix builds easier.
**		
**	V2.1-003	Rod Burr				    12-Feb-1990
**		Added constant definitions for new Open_flags field in 
**		DWC$db_Open_calendar.
**
**	V2-002	Per Hamnqvist					    15-May-1989
**		Add constant defining the number of bits to shift the target
**		day in super condtional
**
**	V2-001	Per Hamnqvist					    02-May-1989
**		Convert "public" SDL comments to private comments.
**
**	V2-000	Per Hamnqvist					    01-May-1989
**		Module created
**--
*/

/* THIS HAS TO BE KEPT IN SYNC WITH DWC_DB_PUBLIC_CONST.UIL !!!!!! */
#define DWC$k_db_none 0
#define DWC$k_db_absolute 1
#define DWC$k_db_abscond 2
#define DWC$k_db_nth_day 3
#define DWC$k_db_nth_xday 4
#define DWC$k_db_last_weekday 5
#define DWC$k_db_nth_day_cwd 6
#define DWC$k_db_nth_day_end 7
#define DWC$k_db_undefined 0
#define DWC$k_day_default 0
#define DWC$k_day_workday 5
#define DWC$k_day_nonwork 6
#define DWC$m_day_special 8
#define DWC$m_day_defaulted 128
#define DWC$k_use_empty 0
#define DWC$k_use_normal 1
#define DWC$k_use_significant 2
#define DWC$m_item_insignif 1
#define DWC$m_item_rpos 6
#define DWC$k_item_middle 0
#define DWC$k_item_last 2
#define DWC$k_item_first 4
#define DWC$k_item_text 0
#define DWC$k_item_texti 1
#define DWC$k_item_cstr 2
#define DWC$m_cond_none 0
#define DWC$m_cond_fwd 256
#define DWC$m_cond_bck 512
#define DWC$m_cond_flags 768
#define DWC$v_cond_day 10
#define DWC$m_cond_mask 1023
#define DWC$k_db_max_pretty 75
#define DWC$k_db_calendar_precision 1440
#define DWC$k_db_max_alarm_time 20160
#define DWC$k_db_normal 1
#define DWC$k_db_failure 2
#define DWC$k_db_open_rd 3
#define DWC$k_db_open_wt 4
#define DWC$k_db_upgrade 5
#define DWC$k_db_upgread 6
#define DWC$k_db_locked 7
#define DWC$k_db_badfile 8
#define DWC$k_db_insdisk 9
#define DWC$k_db_notren 10
#define DWC$k_db_nosuchfile 11
#define DWC$k_db_dayfull 12
#define DWC$k_db_nomore 13
#define DWC$k_db_nsitem 14
#define DWC$k_db_overlap 15
#define DWC$k_db_noala 16
#define DWC$k_db_toobig 17
#define DWC$k_db_insmem 18
#define DWC$k_db_unexpend 19
#define DWC$k_db_notoks 20
#define DWC$k_db_badisyn 21
#define DWC$k_db_invdate 22
#define DWC$k_db_invtime 23
#define DWC$k_db_shortnum 24
#define DWC$k_db_loadf 25
#define DWC$k_db_triggers 26
#define DWC$k_db_notrigger 27
#define DWC$m_force_upg 1
#define DWC$m_disable_rpts 2

#endif /* end of _dwc_db_public_const_h_ */
