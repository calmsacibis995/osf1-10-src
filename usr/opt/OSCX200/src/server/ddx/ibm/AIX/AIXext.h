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
 * $XConsortium: AIXext.h,v 1.2 91/07/16 12:56:01 jap Exp $
 *
 * Copyright IBM Corporation 1987,1988,1989,1990,1991
 *
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS, AND 
 * NONINFRINGEMENT OF THIRD PARTY RIGHTS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

#ifndef _AIXEXT_H
#define _AIXEXT_H

#include "pixmapstr.h"
#include "miscstruct.h"

typedef struct _AIXInfoRec {
int kbdid;
int dpsid;
int displayid;
int autoloadmode ;
int inputfd ;
int kbdiodn  ;
int lociodn  ;
#ifdef AIXTABLET
int mouseiodn ;
int tabletiodn ;
#endif AIXTABLET
int dialiodn ;
int lpfkiodn ;
int loctype  ;
int whatelse[5]  ;
} AIXInfoRec , *AIXInfoPtr ;

/* copy long from src to dst byteswapping on the way */

#define cpswapl(src, dst) \
                 ((char *)&(dst))[0] = ((char *) &(src))[3];\
                 ((char *)&(dst))[1] = ((char *) &(src))[2];\
                 ((char *)&(dst))[2] = ((char *) &(src))[1];\
                 ((char *)&(dst))[3] = ((char *) &(src))[0];

/* copy short from src to dst byteswapping on the way */

#define cpswaps(src, dst)\
		 ((char *) &(dst))[0] = ((char *) &(src))[1];\
		 ((char *) &(dst))[1] = ((char *) &(src))[0];

#define LengthRestB(stuff) \
    (((unsigned long)stuff->length << 2) - sizeof(*stuff))

#define LengthRestS(stuff) \
    (((unsigned long)stuff->length << 1) - (sizeof(*stuff) >> 1))

#define LengthRestL(stuff) \
    ((unsigned long)stuff->length - (sizeof(*stuff) >> 2))

#define SwapRestS(stuff) \
    SwapShorts((short *)(stuff + 1), LengthRestS(stuff))

#define SwapRestL(stuff) \
    SwapLongs((long *)(stuff + 1), LengthRestL(stuff))

#define PropagateMask ( \
	KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | \
	MotionMask )

#define WriteAIXReplyToClient(pClient, size, pReply) \
   if (pClient->swapped) \
      (*ReplySwapVector[((xReq *)pClient->requestBuffer)->reqType]) \
           (pClient, size, pReply); \
      else WriteToClient(pClient, size, (char *) pReply);

#define WriteAIXSwappedDataToClient(pClient, size, pbuf) \
   if (pClient->swapped) \
      (*pClient->pSwapReplyFunc)(pClient, size, pbuf); \
   else WriteToClient (pClient, size, (char *) pbuf);

typedef struct _PolyMarkerState {
    short int xhot, yhot;
    short int  width, height;
    unsigned char  *marker;
    int  version;
    void  (*DrawPolyMarker)();
    void  (*SetPolyMarker)();
    void  (*PolyMarkerDelete)();
    void  (*DrawPolyMarkers)();
} PolyMarkerStateRec, *PolyMarkerStatePtr;

#ifdef AIXEXTSAMPLE
typedef struct _GeoTextState {
    short int xhot, yhot;
    void (*DrawGeoText)();
} GeoTextStateRec, *GeoTextStatePtr;
#endif

typedef struct _DialState {
    unsigned	int 	devmask ;	/* dev selection */
    unsigned	int 	dialmask ;	/* dial control */
    char 		dialmap[16] ;
    unsigned	int 	eventMask ;
    unsigned	int	dontPropagateMask;
    unsigned	int 	allEventMasks ;
    unsigned	int	deliverableEvents;
} DialStateRec, *DialStatePtr;

typedef struct _LpfkState {
    unsigned	int 	devlpfkmask ;     /* dev selection */
    unsigned	int 	devlightmask ;    /* dev selection */
    unsigned	int	lpfkmask ;        /* input control */
    unsigned	int	lightmask ;       /* output control */
    unsigned	int 	eventMask ;
    unsigned	int	dontPropagateMask;
    unsigned	int 	allEventMasks ;
    unsigned	int	deliverableEvents;
} LpfkStateRec, *LpfkStatePtr;

#ifdef AIXTABLET
typedef struct _TabletState {
    unsigned    int     devmask ;       /* dev selection */
    unsigned    int     mode ;          /* tablet conversion mode */
    unsigned    int     resolution ;    /* tablet resolution */
    unsigned    int     origin ;        /* tablet coordinate origin */
    unsigned    int     rate ;          /* tablet sampling rate */
    unsigned    int     eventMask ;
    unsigned    int     dontPropagateMask;
    unsigned    int     allEventMasks ;
    unsigned    int     deliverableEvents;
} TabletStateRec, *TabletStatePtr;
#endif AIXTABLET

typedef struct _WInterest {
    struct _WInterest	*pNextWInterest;
    struct _WInterest	*pLastWInterest;
    int			length;		/* size */
    unsigned long	owner;		/* extension id of owning extension */
    unsigned long	ValInterestMask;  /* you tell me what this is for */
    unsigned long	ChangeInterestMask;  /* you tell me what this is for */
    Bool		(* ValidateWindow) ();
    Bool		(* ChangeExtWindowAttributes) ();
    Bool		(* MapWindow) (); /* give a chance to people who */
    Bool		(* UnmapWindow) (); /* like to do something when */
    Bool		(* CreateWindow) ();  /*  these routines got */
    Bool		(* DestroyWindow) (); /* called , might be redundant */
    Bool		(* PositionWindow) ();
    void		(* WIProc) ();
    unsigned char       *extPriv; /* aix X server internal private data */
} WInterestRec;

typedef WInterestRec *WInterestPtr ;

#define    InsertWI(pWin, pWI, order, pPrevWI)    \
	   order(pWin,pWI,pPrevWI)

#define WI_FIRST(pWin,pWI,dummy)\
    {				   \
    pWI->pNextWInterest = ((aixPrivWinPtr)(pWin->devPrivate))->pNextWInterest;\
    pWI->pLastWInterest = (WInterestPtr)\
	&((aixPrivWinPtr)(pWin->devPrivate))->pNextWInterest ; \
    ((aixPrivWinPtr)(pWin->devPrivate))->pNextWInterest->pLastWInterest = pWI; \
    ((aixPrivWinPtr)(pWin->devPrivate))->pNextWInterest=pWI;		    \
    }					    

#define WI_MIDDLE(pWin,pWI,pPrevWI)\
    {					    \
    pWI->pNextWInterest = pPrevWI->pNextWInterest;\
    pWI->pLastWInterest = (WInterestPtr)&pPrevWI->pNextWInterest; \
    pPrevWI->pNextWInterest->pLastWInterest = pWI; \
    pPrevWI->pNextWInterest=pWI;		    \
    }

#define WI_LAST(pWin,pWI,dummy)\
    {					    \
    pWI->pNextWInterest = (WInterestPtr)&((aixPrivWinPtr)\
	(pWin->devPrivate))->pNextWInterest;\
    pWI->pLastWInterest = ((aixPrivWinPtr)(pWin->devPrivate))->pLastWInterest;\
    ((aixPrivWinPtr)(pWin->devPrivate))->pLastWInterest->pNextWInterest = pWI;\
    ((aixPrivWinPtr)(pWin->devPrivate))->pLastWInterest = pWI;     \
    }

#define RemoveWI(pWI) \
	pWI->pNextWInterest->pLastWInterest = pWI->pLastWInterest;\
	pWI->pLastWInterest->pNextWInterest = pWI->pNextWInterest;\
	pWI->pNextWInterest = 0;\
	pWI->pLastWInterest = 0;

#define	NUM_GAIDEV	1

typedef struct {
    int                 fastBorder;
    int                 fastBackground;
    DDXPointRec		oldRotate;
    PixmapPtr  		pRotatedBackground;
    PixmapPtr	        pRotatedBorder;
    struct _WInterest	*pNextWInterest;
    struct _WInterest	*pLastWInterest;
    unsigned	int     eventMask ;
    unsigned    int     dontPropagateMask;
    unsigned    int     allEventMasks ;
    unsigned    int     deliverableEvents;
    pointer		pGaiWin 	;	/* gWindowPtr  */
   pointer		pGaiWinGeom 	;	/* gWinGeomPtr */
} aixPrivWin;

typedef aixPrivWin *aixPrivWinPtr ;

extern void  ValidateWindow();
extern void  SetWIMask();
extern WInterestPtr GetWI();
extern void  FreeWI();

#endif /* _AIXEXT_H */
