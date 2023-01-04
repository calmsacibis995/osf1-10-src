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
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: WmWinConf.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 21:46:25 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO

extern void    AdjustPos ();
extern unsigned int ButtonStateBit ();
extern void    CancelFrameConfig();
extern void	CheckEatButtonRelease ();
extern Boolean CheckVisualPlace ();
extern void    CompleteFrameConfig ();
extern Cursor  ConfigCursor();
extern void    DoFeedback();
extern Boolean DoGrabs();
extern void    DrawOutline();
extern void	EatButtonRelease ();
extern void    FixFrameValues();
extern void    FlashOutline();
extern void    ForceOnScreen();
extern void	GetClipDimensions ();
extern void    GetConfigEvent ();
extern Window  GrabWin();
extern void    HandleClientFrameMove ();
extern void    HandleClientFrameResize ();
extern Boolean HandleResizeKeyPress();
extern void    MoveOpaque ();
extern void    MoveOutline ();
extern void    ProcessNewConfiguration ();
extern void    ReGrabPointer();
extern void	SetFrameInfo ();
extern Boolean SetKeyboardResizePart ();
extern void	SetOutline();
extern void    SetPointerPosition ();
extern Boolean SetPointerResizePart ();
extern Boolean StartClientMove ();
extern void    StartClientResize();
extern Boolean StartResizeConfig();
extern int     ResizeType();
extern void    UndoGrabs();

#else /* _NO_PROTO */

extern void AdjustPos (int *pX, int *pY, unsigned int oWidth, 
		       unsigned int oHeight, unsigned int nWidth, 
		       unsigned int nHeight);
extern unsigned int ButtonStateBit (unsigned int button);
extern void CancelFrameConfig (ClientData *pcd);
extern void	CheckEatButtonRelease (ClientData *pcd, XEvent *pev);
extern Boolean CheckVisualPlace (ClientData *pCD, int tmpX, int tmpY);
extern void CompleteFrameConfig (ClientData *pcd, XEvent *pev);
extern Cursor ConfigCursor (int frame_part);
extern void DoFeedback (ClientData *pcd, int x, int y, unsigned int width, 
			unsigned int height, unsigned long newStyle, 
			Boolean resizing);
extern Boolean DoGrabs (Window grab_win, Cursor cursor, unsigned int pmask, 
			Time grabTime, ClientData *pCD, Boolean alwaysGrab);
extern void DrawOutline (int x, int y, unsigned int width, 
			 unsigned int height);
extern void	EatButtonRelease (unsigned int releaseButtons);
extern void FixFrameValues (ClientData *pcd, int *pfX, int *pfY, 
			    unsigned int *pfWidth, unsigned int *pfHeight, 
			    Boolean resizing);
extern void FlashOutline (int x, int y, unsigned int width, 
			  unsigned int height);
extern void ForceOnScreen (int screen, int *pX, int *pY);
extern void GetClipDimensions (ClientData *pCD, Boolean fromRoot);
extern void GetConfigEvent (Display *display, Window window, 
			    unsigned long mask, int curX, int curY, 
			    int oX, int oY, unsigned oWidth, 
			    unsigned oHeight, XEvent *pev);
extern Window GrabWin (ClientData *pcd, XEvent *pev);
extern void HandleClientFrameMove (ClientData *pcd, XEvent *pev);
extern void HandleClientFrameResize (ClientData *pcd, XEvent *pev);
extern Boolean HandleResizeKeyPress (ClientData *pcd, XEvent *pev);
extern void    MoveOpaque (ClientData *pcd, int x, int y,
		 unsigned int width, unsigned int height);
extern void MoveOutline (int x, int y, unsigned int width, 
			 unsigned int height);
extern void ProcessNewConfiguration (ClientData *pCD, int x, int y, 
				     unsigned int width, unsigned int height, 
				     Boolean clientRequest);
extern void ReGrabPointer (Window grab_win, Time grabTime);
extern void SetOutline (XSegment *pOutline, int x, int y, unsigned int width, 
			unsigned int height, int fatness);
extern void SetPointerPosition (int newX, int newY, int *actualX, 
				int *actualY);
extern Boolean SetPointerResizePart (ClientData *pcd, XEvent *pev);
extern Boolean StartClientMove (ClientData *pcd, XEvent *pev);
extern void StartClientResize (ClientData *pcd, XEvent *pev);
extern Boolean StartResizeConfig (ClientData *pcd, XEvent *pev);
extern int ResizeType (ClientData *pcd, XEvent *pev);
extern void UndoGrabs (void);

#endif /* _NO_PROTO */
