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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: scu_global.c,v $ $Revision: 1.1.9.3 $ (DEC) $Date: 1993/12/15 20:57:00 $";
#endif
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
 * File:	scu_global.c
 * Author:	Robin T. Miller
 * Date:	September 21, 1991
 *
 * Description:
 *	This file contains the global data declarations.
 */
#include <sys/types.h>
#include <io/common/iotypes.h>
#include <io/cam/cdrom.h>
#include <io/cam/rzdisk.h>
#include <io/cam/cam.h>
#include <io/cam/cam_special.h>

#undef SUCCESS
#undef FATAL_ERROR

#include "scu.h"
#include "scu_device.h"
#include "scu_pages.h"
#include "scsipages.h"

/*
 * Global Data Declarations:
 */
char *OurName;				/* Pointer to our program name.	*/
char *HelpTopic;			/* Help topic entered by user.	*/
char *InputBufPtr;			/* The input buffer pointer.	*/
char *InputFileName;			/* The input file name.		*/
char *OutputBufPtr;			/* The output buffer pointer.	*/
char *OutputFileName;			/* The output file name.	*/
char *OutputLogName;			/* The output log file name.	*/
char *OutputPager;			/* The output pager string.	*/
char *QarBufPtr;			/* The QAR buffer pointer.	*/
char *SourceFileName;			/* The source input file name.	*/
char *TmpBufPtr;			/* Temporary buffer pointer.	*/
char *WrkBufPtr;			/* The work buffer pointer.	*/
char *Home;				/* The home directory string.	*/

long AutoReassign = FALSE;		/* Auto-reassign block flag.	*/
long BypassFlag = FALSE;		/* Bypass special checks flag.	*/
long CamErrorFlag = TRUE;		/* CAM error report flag.	*/
long CamRetrysFlag = TRUE;		/* CAM special I/O retrys.	*/
long CmdInProgressFlag = FALSE;		/* Command in progress flag.	*/
volatile long CmdInterruptedFlag = FALSE; /* User interrupted command.	*/
long CoreDumpFlag = FALSE;		/* Disable core dump flag.	*/
long DebugFlag = FALSE;			/* Our debug control flag.	*/
long DumpFlag = FALSE;			/* Dump buffer control flag.	*/
long DisplayVerbose = TRUE;		/* Display verbosity flag.	*/
long ExitFlag = TRUE;			/* Program exit flag.		*/
int  ExitStatus = SUCCESS;		/* The program exit status.	*/
long FullErrorReport = FALSE;		/* Full error reporting flag.	*/
long InteractiveFlag = FALSE;		/* Running program interactive.	*/
long OutputRadix = DEC_RADIX;		/* Default to decimal output.	*/
long PageSize;				/* Machine page size (bytes).	*/
long PagingFlag = TRUE;			/* The screen paging flag.	*/
long PerrorFlag = TRUE;			/* Print error messages flag.	*/
long PromptUserFlag = TRUE;		/* Enable prompt user flag.	*/
long ScuFlags;				/* Our program control flags.	*/
long SpecialFlags = SA_NO_ERROR_LOGGING;/* Special I/O control flags.	*/
long StatisticsFlag = FALSE;		/* Performance statistics flag.	*/
long TimingFlag = FALSE;		/* Command timing control flag.	*/
int  UagtFd = -1;			/* User agent file descriptor.	*/
long UagtFileFlags;			/* User agent file open mode.	*/
long WatchProgress = -1L;		/* Watch the I/O progress flag.	*/

/*
 * Test and/or I/O Variables:
 */
u_long AlignOffset;			/* Align buffer at this offset.	*/
u_long BlockLimit;			/* Data transfer block limit.	*/
u_long BlockSize;			/* Data block size (in bytes).	*/
u_long BufferMode;			/* The Read/Write Buffer mode.	*/
u_long BufferId = DEFAULT_BUFFER_ID;	/* The Read/Write Buffer id.	*/
u_long BufferOffset;			/* The Read/Write Buffer offset	*/
u_long CompareFlag;			/* Compare data control flag.	*/
u_long DataLimit;			/* Total data limit per pass.	*/
u_long DataPattern = DEFAULT_PATTERN;	/* The default data pattern.	*/
u_long DelayValue;			/* Delay between I/O's value.	*/
u_long EndingLogicalBlock;		/* Ending logical block number.	*/
u_long ErrorLimit;			/* Number of errors tolerated.	*/
u_long ExpressionValue;			/* Evaluate expression value.	*/
u_long IncrementCount;			/* # of records to increment.	*/
u_long LogicalBlock;			/* Logical block number.	*/
u_long PassLimit;			/* Default number of passes.	*/
u_long RandomFlag;			/* Do random I/O disk tests.	*/
u_long RandomizingKey;			/* The randomizing key to use.	*/
u_long RecordLimit;			/* The # of records to process.	*/
u_long RecoveryFlag = TRUE;		/* The error recovery flag.	*/
u_long RetryLimit;			/* Times to retry the command.	*/
u_long SeekCount;			/* The seek count for disks.	*/
u_long SegmentSize;			/* The download segment size.	*/
u_long SkipCount;			/* The skip count for tapes.	*/
u_long SleepValue;			/* The sleep time (in seconds).	*/
u_long StepValue;			/* The step value for seeks.	*/
u_long StartingLogicalBlock;		/* Starting logical block.	*/
u_long TestRecoveryFlag;		/* Test error recovery flag.	*/
u_long TransferLength;			/* Transfer length (in blocks).	*/
u_long VerifyFlag;			/* Verify data written flag.	*/
u_long VerifyLength = VERIFY_LENGTH;	/* Default verify block length.	*/

/*
 * Test Defaults (if none are specified by user).
 */
u_long DefaultAlignOffset = 0;
u_long DefaultBlockSize = DEFAULT_BLOCK_SIZE;
u_long DefaultBufferMode = DEFAULT_BUFFER_MODE;
u_long DefaultCompareFlag = TRUE;
u_long DefaultDataLimit = 0;
u_long DefaultDataPattern = DEFAULT_PATTERN;
u_long DefaultDelayValue = 0;
u_long DefaultErrorLimit = DEFAULT_ERROR_LIMIT;
u_long DefaultIncrementCount = 0;
u_long DefaultPassLimit = DEFAULT_PASS_LIMIT;
u_long DefaultRandomFlag = FALSE;
u_long DefaultRandomizingKey;
u_long DefaultRetryLimit = DEFAULT_RETRY_LIMIT;
u_long DefaultSeekCount = 0;
u_long DefaultSegmentSize = DEFAULT_SEGMENT_SIZE;
u_long DefaultSkipCount = 0;
u_long DefaultStepValue = 0;
u_long DefaultVerifyFlag = TRUE;
u_long DefaultVerifyLength = -1L;

u_long CamFlags = CAM_SIM_QFRZDIS;	/* The SCSI I/O CCB CAM flags.	*/
long CamTimeout = 0;			/* The SCSI I/O CCB timeout.	*/
long UserBus = -1L;			/* User set bus number.		*/
long UserTarget = -1L;			/* User set target number.	*/
long UserLun = -1L;			/* User set logical unit number	*/
long UserSubLun = -1L;			/* User set sub-lun number.	*/
long UserDeviceType = -1L;		/* User set device type.	*/
long UserSCSIID = -1L;			/* User set SCSI id		*/
char *UserDeviceName;			/* User specified device name.	*/

FILE *InputFp;				/* The input file pointer.	*/
FILE *LogFp;				/* The output log file pointer.	*/
FILE *OutputFp;				/* The output file pointer.	*/
FILE *PipeFp;				/* The pipe file pointer.	*/

struct parser_control ParserControl;	/* The parser control block.	*/
struct scu_device *ScuDevice;		/* Current SCSI device entry.	*/
struct scu_device *ScuDeviceList;	/* List of SCSI device entrys.	*/
struct scu_device *ScuPrevious;		/* The previous device entry.	*/

long Formatting = FALSE;		/* Formatting in-progress flag.	*/
long DensityType = 0;			/* The diskette density type.	*/
long DefectFormat = PHY_FORMAT;		/* The defect list format.	*/
long DefectType = KNOWN_DEFECTS;	/* The defect list type.	*/
long DisplayBlockDesc = TRUE;		/* Display block descriptors.	*/
long DisplayModeParameters = TRUE;	/* Display mode parameters flag	*/
long ProcessAllPages = 0;		/* Processing all pages flag.	*/
long OperationType = SHOW_OPERATION;	/* The operation type selected.	*/
long SetPageParameters = TRUE;		/* Set pages parameters flag.	*/
long CheckPageSavable = FALSE;		/* Check page savable bit.	*/

long ModePageCode = -1L;			/* The mode page code selected.	*/
long PageControlField = PCF_CURRENT;	/* Default page control field.	*/
u_char VerifyPcf = PCF_DEFAULT;		/* Mode page to verify against.	*/
long SaveModeParameters = TRUE;		/* Save mode parameters flag.	*/

/*
 * This table contains function pointers to all supported mode pages.
 */
struct mode_page_funcs *mode_page_table[MAX_MODE_PAGES];

/*
 * CD-ROM Specific Declarations:
 */
long ForwardTracks = 1;			/* Skip forward tracks.		*/
long BackwardTracks = 1;		/* Skip backward tracks.	*/
long StartingTrack = 1;			/* Starting track number.	*/
long StartingIndex = 1;			/* Starting index value.	*/
long EndingTrack = -1L;			/* Ending track number.		*/
long EndingIndex = 1;			/* Ending index value.		*/
long TrackNumber = 0;			/* Specific track number.	*/
long NumberIndexes = 0xFF;		/* Number of indexes to play.	*/
long AddressFormat = -1L;		/* Default address format.	*/
long ISRCTrackNumber = 1;		/* The ISRC track number.	*/

/*
 * The following defaults are for media conforming to the CD-ROM and
 * CD_DA standard.  These fields are NOT changeable with our drive and
 * are obtained from Mode Sense Page 0x0D (CD-ROM Parameters Page).
 */
int S_Units_per_M_Unit = 60;	/* 0x3C	/* S-Units per M-Unit value.	*/
int F_Units_per_S_Unit = 75;	/* 0x4B	/* F-Units per S-Unit value.	*/
long BlocksPerCDBlock = (CD_BLOCK_SIZE / DEC_BLOCK_SIZE);

/*
 * The table of contents is stored in the sub-channel of lead-in area.
 * The lead-in area is coded as track zero.  Track zero and the initial
 * 150 sector pre-gap (or audio pause) are not accessible with logical
 * block addressing.  When converting from MSF format to LBA format, this
 * initial area must be adjusted for or else our calculation doesn't match
 * that returned in the TOC entry when LBA format is requested.
 *
 * NOTE: The blocks to adjust isn't always 600.  When necessary, this
 *	 base must be set from the MSF from the first track.  On my
 *	 Pink Floyd CD "Dark Side Of The Moon" it is 732.
 */
long MSFtoLBA_Adjust = 600;		/* The MSF to LBA adjust size.	*/

long Starting_M_Unit;			/* Starting minute units value.	*/
long Starting_S_Unit;			/* Starting second units value.	*/
long Starting_F_Unit;			/* Starting frame units value.	*/
long Ending_M_Unit;			/* Ending minute units value.	*/
long Ending_S_Unit;			/* Ending second units value.	*/
long Ending_F_Unit;			/* Ending frame units value.	*/
long Channel_0_Select = CDROM_CHANNEL_0; /* Channel 0 selection code.	*/
long Channel_0_Volume = CDROM_MAX_VOLUME;/* Channel 0 volume level.	*/
long Channel_1_Select = CDROM_CHANNEL_1; /* Channel 1 selection code.	*/
long Channel_1_Volume = CDROM_MAX_VOLUME;/* Channel 1 volume level.	*/
long Channel_2_Select;			/* Channel 2 selection code.	*/
long Channel_2_Volume;			/* Channel 2 volume level.	*/
long Channel_3_Select;			/* Channel 3 selection code.	*/
long Channel_3_Volume;			/* Channel 3 volume level.	*/
long VolumeLevel = CDROM_MAX_VOLUME;	/* Volume level (all channels).	*/

/*
 * Magtape Specific Declarations:
 */
u_long MtopCount = -1L;			/* Magtape operation count.	*/
long DensityCode = -1L;			/* The tape density code.	*/
long BlockingSize = DEC_BLOCK_SIZE;	/* The tape blocking size.	*/
long BufferedMode;			/* The tape buffered mode.	*/
