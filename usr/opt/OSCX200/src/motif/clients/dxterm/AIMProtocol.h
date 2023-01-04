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



#ifndef _AIMPROTOCOL_H
#define _AIMPROTOCOL_H
#include <X11/X.h>
#include <X11/Intrinsic.h>
#include "AIMCommon.h"

typedef struct _AIMProtocolCtx *AIMProtocolCtx;

/*
 * AIM Protocol structures that are used by AIM widget.
 */
typedef struct _AIMAnyProtocol {
  int			protocol_id;
} AIMAnyProtocol;

typedef struct {
  int			protocol_id;
  Window		rimp_id;
  Bool		        start_conversion;
  long			start_im_type;
  AIMKey		*start_conversion_key;
  int			start_conversion_key_units;
  AIMKey		*end_conversion_key;
  int			end_conversion_key_units;
} AIMConnectionEstablishProtocol;

typedef struct {
  int			protocol_id;
  unsigned int		get_event_mask;
  unsigned int		peek_event_mask;
  unsigned int		get_non_maskable_mask;
  unsigned int		peek_non_maskable_mask;
} AIMSetSentEventsProtocol;

typedef struct _AIMAnyProtocol AIMPreEditStartProtocol;

typedef struct _AIMAnyProtocol AIMSecPreEditStartProtocol;

typedef struct _AIMAnyProtocol AIMPreEditDoneProtocol;

typedef struct {
  int			protocol_id;
  int			start_offset;
  int			end_offset;
  XmString		new_string;
  AIMTextRendering	*renditions;
  int                   num_rendition;
} AIMPreEditDrawProtocol;

typedef struct {
  int			protocol_id;
  int			start_offset;
} AIMSetCursorPositionProtocol;

typedef struct _AIMAnyProtocol AIMQueryXYPositionProtocol;

typedef struct {
  int			protocol_id;
  int			start_offset;
  int			end_offset;
  XmString		new_string;
} AIMDrawIntermediateCharProtocol;

typedef struct _AIMAnyProtocol AIMGetCurrentCursorCharProtocol;

typedef struct _AIMAnyProtocol AIMDisconnectCompleteProtocol;

typedef union {
  int					protocol_id;
  AIMAnyProtocol			aimany;
  AIMConnectionEstablishProtocol	aimconnectionestablish;
  AIMSetSentEventsProtocol		aimsetsentevents;
  AIMPreEditStartProtocol		aimpreeditstart;
  AIMSecPreEditStartProtocol		aimsecpreeditstart;
  AIMPreEditDoneProtocol		aimpreeditdone;
  AIMPreEditDrawProtocol		aimpreeditdraw;
  AIMSetCursorPositionProtocol		aimsetcursorposition;
  AIMQueryXYPositionProtocol		aimqueryxyposition;
  AIMDrawIntermediateCharProtocol	aimdrawintermediatechar;
  AIMGetCurrentCursorCharProtocol	aimgetcurrentcursorchar;
  AIMDisconnectCompleteProtocol         aimdisconnectcomplete;
} AIMProtocolRec, *AIMProtocol;

/*
 * Forward decralations
 */
#ifdef __STDC__
AIMProtocolCtx AIMProtocolCtxInit(Widget aim_widget,
				  void (*disconnect)(),
				  Opaque *data);
#else
AIMProtocolCtx AIMProtocolCtxInit();
#endif

#ifdef __STDC__
void AIMProtocolCtxGetOffset(AIMProtocolCtx ctx,
			     long *property8_offset,
			     long *property32_offset);
#else
void AIMProtocolCtxOffset();
#endif

#ifdef __STDC__
void AIMProtocolCtxSetOffset(AIMProtocolCtx ctx,
			     long property8_offset,
			     long property32_offset);
#else
void AIMProtocolCtxOffset();
#endif


#ifdef __STDC__
void AIMProtocolCtxFree(AIMProtocolCtx ctx);
#else
void AIMProtocolCtxFree();
#endif

#ifdef __STDC__
AIMProtocol AIMEventToProtocol(AIMProtocolCtx protocol_ctx,
			       XEvent *event,
			       Bool *event_used);
#else
AIMProtocol AIMEventToProtocol();
#endif

#ifdef __STDC__
void AIMProtocolFree(AIMProtocol protocol);
#else
void AIMProtocolFree();
#endif


#ifdef __STDC__
Bool AIMConnect(AIMProtocolCtx ctx,
		Atom   rimp_atom,
		Window aim_win,
		Window app_win,
		Window shell_win);
#else
Bool AIMConnect();
#endif

#ifdef __STDC__
void AIMDisconnect(AIMProtocolCtx ctx,
		   void (*disconnect_complete_cb)(),
		   Opaque *disconnect_complete_data);
#else
void AIMDisconnect();
#endif

#ifdef __STDC__
void AIMStartConversion(AIMProtocolCtx ctx, long im_type);
#else
void AIMStartConversion();
#endif

#ifdef __STDC__
void AIMEndConvertion(AIMProtocolCtx ctx, long im_type);
#else
void AIMEndConvertion();
#endif

#ifdef __STDC__
void AIMEndIState(AIMProtocolCtx ctx);
#else
void AIMEndIState();
#endif

#ifdef __STDC__
void AIMCancelIState(AIMProtocolCtx ctx);
#else
void AIMCancelIState();
#endif

#ifdef __STDC__
void AIMSendCIETable(AIMProtocolCtx ctx, unsigned char *table);
#else
void AIMSendCIETable();
#endif

#ifdef __STDC__
void AIMChangeRIMPAttributes(AIMProtocolCtx ctx,
			     int byte_length,
			     Opaque *rimp_attributes);
#else
void AIMChangeRIMPAttributes();
#endif

#ifdef __STDC__
void AIMAnswerToSecPreEditStart(AIMProtocolCtx ctx,
				int start_offset,
				int end_offset,
				XmString new_string);
#else
void AIMAnswerToSecPreEditStart();
#endif

#ifdef __STDC__
void AIMAnswerToQueryXYPosition(AIMProtocolCtx ctx, int x, int y);
#else
void AIMAnswerToQueryXYPosition();
#endif

#ifdef __STDC__
void AIMAnswerToGetCurrentCursorChar(AIMProtocolCtx ctx, XmString new_string);
#else
void AIMAnswerToGetCurrentCursorChar();
#endif

#ifdef __STDC__
void AIMErrorStatus(AIMProtocolCtx ctx,
		    int error_protocol_id,
		    int error_status_id);
#else
void AIMErrorStatus();
#endif

#ifdef __STDC__
void AIMSendEvent(AIMProtocolCtx ctx,
		  Display *display,
		  Window w,
		  Bool propagate,
		  long mask,
		  XEvent *event);
#else
void AIMSendEvent();
#endif

#ifdef __STDC__
void AIMGetRIMPList(AIMProtocolCtx ctx, Atom **rimp_list, int *num);
#else
void AIMGetRIMPList();
#endif

#ifdef __STDC__
Boolean AIMIsRIMPAvailable(AIMProtocolCtx ctx, Atom rimp_atom);
#else
Boolean AIMIsRIMPAvailable();
#endif

/*
 * RIMP local connection
 */
typedef Boolean (*RIMPLocalConnectionProc)(/* Atom im_name,
					      Window aim_win,
					      Window thw_win,
					      Window closest_shell_win,
					      Databuffer  db */);

#ifdef __STDC__
void _AIMRegisterRIMPLocalConnection(RIMPLocalConnectionProc proc);
#else
void _AIMRegisterRIMPLocalConnection();
#endif

#endif _AIMPROTOCOL_H
