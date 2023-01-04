/*
*****************************************************************************

	      Copyright (c) Digital Equipment Corporation, 1990, 1991
	      All Rights Reserved.  Unpublished rights reserved
	      under the copyright laws of the United States.
	      
	      The software contained on this media is proprietary
	      to and embodies the confidential technology of 
	      Digital Equipment Corporation.  Possession, use,
	      duplication or dissemination of the software and
	      media is authorized only pursuant to a valid written
	      license from Digital Equipment Corporation.

	      RESTRICTED RIGHTS LEGEND   Use, duplication, or 
	      disclosure by the U.S. Government is subject to
	      restrictions as set forth in Subparagraph (c)(1)(ii)
	      of DFARS 252.227-7013, or in FAR 52.227-19, as
	      applicable.

*****************************************************************************
**++
**  FACILITY:
**
**	DECwindows Toolkit
**
**  ABSTRACT:
**
**	Asian Input Method
**
**
**  MODIFICATION HISTORY:
**
**
**--
**/


#ifndef AIMCommon_h
#define AIMCommon_h

/*
 * Protocol ID
 */
#define Connect				1
#define Disconnect			2
#define StartConversion			3
#define EndConversion			4
#define EndIState			5
#define CancelIState			6
#define SendCIETable			7
#define ChangeRIMPAttributes		8
#define AnswerToSecPreEditStart		9
#define AnswerToQueryXYPosition		10
#define AnswerToGetCurrentCursorChar	11
#define ErrorStatus			12
#define ConnectionEstablish		13
#define SetSentEvents			14
#define PreEditStart			15
#define SecPreEditStart			16
#define PreEditDone			17
#define PreEditDraw			18
#define SetCursorPosition		19
#define QueryXYPosition			20
#define DrawIntermediateChar		21
#define GetCurrentCursorChar		22
#define DisconnectComplete              23

/*
 * Masks for {get|peek}_non_maskable_mask in AIMSetSentEventsProtocol
 */
#define AIMGraphicsExposeMask	1L
#define AIMNoExposeMask		(1L << 1)
#define AIMClientMessageMask	(1L << 2)
#define AIMMappingNotifyMask	(1L << 3)
#define AIMSelectionClearMask	(1L << 4)
#define AIMSelectionNotifyMask	(1L << 5)
#define AIMSelectionRequestMask	(1L << 6)

/*
 * AIMTextRendering
 */
#define	AIMReverse	1L
#define AIMUnderLine	(1L << 1)
#define AIMHighlight	(1L << 2)
#define AIMPrimary	(1L << 6)
#define AIMSecondary	(1L << 7)
#define AIMTertiary	(1L << 8)

typedef unsigned long   AIMTextRendering;

/*
 * Error Status
 */
#define CannotPerform		1

typedef struct {
  unsigned long modifier_mask;
  KeySym	keysym;
  long		im_type;
  Boolean	stack_allow;
} AIMKey;

/*
 * Properties used for the protocol
 */

#define AIM_PROTOCOL      "AIM_PROTOCOL"
#define AIM_RIMP_PROPERTY32 "AIM_RIMP_PROPERTY32"
#define RIMP_AIM_PROPERTY32 "RIMP_AIM_PROPERTY32"
#define AIM_RIMP_PROPERTY8  "AIM_RIMP_PROPERTY8"
#define RIMP_AIM_PROPERTY8  "RIMP_AIM_PROPERTY8"
#define ANSWER_PROPERTY8    "ANSWER_PROPERTY8"

#define AIM_RIM_LIST        "_AIM_RIM_LIST"
#endif
