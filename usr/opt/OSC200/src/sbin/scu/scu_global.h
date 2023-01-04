/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/*
 * @(#)$RCSfile: scu_global.h,v $ $Revision: 1.1.8.3 $ (DEC) $Date: 1993/12/15 20:57:01 $
 */
/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived	from  software	received  from	the	*
 *   University    of	California,   Berkeley,   and	from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is	subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * File:	scu_global.h
 * Author:	Robin T. Miller
 * Date:	September 21, 1991
 *
 * Description:
 *	Common include file use by SCSI Utility Program.
 *
 * Modification History:
 *
 */

/*
 * External Declarations:
 */
extern char *OurName, *HelpTopic;
extern char *InputBufPtr, *InputFileName;
extern char *OutputBufPtr, *OutputFileName, *OutputLogName, *OutputPager;
extern char *SourceFileName, *TmpBufPtr, *WrkBufPtr;
extern char *Home;

extern volatile long CmdInterruptedFlag;

extern long AutoReassign, BypassFlag, CamErrorFlag, CamRetrysFlag;
extern long CmdInProgressFlag, CoreDumpFlag;
extern long ExitFlag, DebugFlag, DisplayVerbose, DumpFlag;
extern long FullErrorReport, InteractiveFlag, OutputRadix, PagingFlag;
extern long PageSize, PerrorFlag, PromptUserFlag, ScuFlags;
extern long SpecialFlags, StatisticsFlag, TimingFlag;
extern long UagtFileFlags, WatchProgress;
extern int  ExitStatus, UagtFd;

u_long AlignOffset, BlockLimit, BlockSize, BufferMode, BufferId, BufferOffset;
u_long CompareFlag, DataLimit, DataPattern, DelayValue;
u_long EndingLogicalBlock, ErrorLimit;
u_long ExpressionValue, IncrementCount, LogicalBlock, PassLimit;
u_long RandomFlag, RandomizingKey, RecordLimit, RecoveryFlag, RetryLimit;
u_long SeekCount, SegmentSize, SkipCount, StartingLogicalBlock, SleepValue;
u_long StepValue, TestRecoveryFlag, TransferLength, VerifyFlag, VerifyLength;

u_long DefaultAlignOffset, DefaultBlockSize, DefaultBufferMode;
u_long DefaultCompareFlag;
u_long DefaultDataLimit, DefaultDataPattern, DefaultDelayValue;
u_long DefaultErrorLimit, DefaultIncrementCount, DefaultPassLimit;
u_long DefaultRandomFlag, DefaultRandomizingKey, DefaultRetryLimit;
u_long DefaultSeekCount, DefaultSegmentSize, DefaultSkipCount;
u_long DefaultStepValue;
u_long DefaultVerifyFlag, DefaultVerifyLength;

extern u_long CamFlags;
extern long CamTimeout;
extern long UserBus, UserTarget, UserLun, UserSubLun, UserDeviceType;
extern long UserSCSIID;
extern char *UserDeviceName;

extern FILE *InputFp, *LogFp, *OutputFp, *PipeFp;

extern long Formatting, DensityType, DefectFormat, DefectType;
extern long DisplayBlockDesc, DisplayModeParameters, ProcessAllPages;
extern long OperationType, SetPageParameters, CheckPageSavable;

extern long ModePageCode;
extern long PageControlField;
extern u_char VerifyPcf;
extern long SaveModeParameters;

/*
 * CD-ROM Specific Declarations:
 */
extern long ForwardTracks, BackwardTracks;
extern long StartingTrack, StartingIndex, EndingTrack, EndingIndex;
extern long TrackNumber, NumberIndexes, AddressFormat, ISRCTrackNumber;
extern long MSFtoLBA_Adjust;
extern int S_Units_per_M_Unit, F_Units_per_S_Unit;
extern long BlocksPerCDBlock;
extern long Starting_M_Unit, Starting_S_Unit, Starting_F_Unit;
extern long Ending_M_Unit, Ending_S_Unit, Ending_F_Unit;
extern long Channel_0_Select, Channel_0_Volume;
extern long Channel_1_Select, Channel_1_Volume;
extern long Channel_2_Select, Channel_2_Volume;
extern long Channel_3_Select, Channel_3_Volume;
extern long VolumeLevel;

extern u_long MtopCount;
extern long DensityCode, BlockingSize, BufferedMode;

extern struct cmd_entry CmdTable[];

extern struct parser_control ParserControl;
