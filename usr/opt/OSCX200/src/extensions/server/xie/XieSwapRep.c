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

/*******************************************************************************
Copyright 1990 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

*******************************************************************************/

/*******************************************************************************
**
**  FACILITY:
**
**      X Image Extension DIX
**
**  ABSTRACT:
**
**	This Xie module consists of wrapper routines for all Xie protocol
**	requests.  These routines perform the byte swapping necessary
**      when the client and server are of different endian-ness.
**	
**  ENVIRONMENT:
**
**	VAX/VMS V5.4
**	ULTRIX  V4.0
**
**  AUTHOR(S):
**
**      Gary L. Grebus
**
**  CREATION DATE:
**
**      Fri Dec  7 11:45:27 1990
**
*******************************************************************************/

/*
**  Include files
*/
#include <stdio.h>

/*
**  Core X Includes
*/
#include <X.h>
#include <Xproto.h>
#include <misc.h>
#include <extnsionst.h>
#include <dixstruct.h>
/*
**  XIE Includes
*/
#include <XieAppl.h>
#include <XieDdx.h>
#include <XieProto.h>
#include <XieServer.h>
#include <XieMacros.h>

/*
**  Table of contents
*/
/* Utility Routines */
void Swap16Write();
#if defined(VMS) && !defined(VXT)
void Swap32Write();
#endif
void SwapFloatWrite();
void SwapTmpWrite();


/* Routines for swapping the Xie replies */
int WriteEventToClient();
int SReplySimple();
int SReplyInitSession();
int SReplyQueryEvents();
int SReplyQueryOpDefaults();
int SReplyQueryResource();
int SReplyGetStream();
int SReplyQueryExport();

/*
**  Equated Symbols
*/

/*
**  MACRO definitions
*/

/*
** External references
*/
void		WriteToClient();

externalref int (*xieDixSwapReply[])();



/*-----------------------------------------------------------------------
------------- Reply swapping utility routines, as lifted from ---------
--------------------- the X11R4 MIT sample server -----------------------
------------------------------------------------------------------------*/
/* Thanks to Jack Palevich for testing and subsequently rewriting all this */

#if defined(VMS) & !defined(VXT)
/*  Write a block of swapped longs */
void
Swap32Write(pClient, size, pbuf)
    ClientPtr	pClient;
    int		size;  /* in bytes */
    register long *pbuf;
{
    int		n, i;

    size >>= 2;
    for(i = 0; i < size; i++)
    /* brackets are mandatory here, because "swapl" macro expands
       to several statements */
    {   
	swapl(&pbuf[i], n);
    }
    (void)WriteToClient(pClient, size << 2, (char *) pbuf);
}
#endif					/* VMS && !VXT */

/*  Write a block of swapped shorts */
void
Swap16Write(pClient, size, pbuf)
    ClientPtr	pClient;
    int		size;  /* in bytes */
    register short *pbuf;
{
    int		n, i;

    size >>= 1;
    for(i = 0; i < size; i++)
    /* brackets are mandatory here, because "swaps" macro expands
       to several statements */
    {   
	swaps(&pbuf[i], n);
    }
    (void)WriteToClient(pClient, size << 1, (char *) pbuf);
}

/*  Write a block of swapped XieFloat records */
void
SwapFloatWrite(pClient, size, pbuf)
    ClientPtr	pClient;
    int		size;  /* in bytes */
    register XieFloatRec *pbuf;
{
    register int n;
    int i;
    int entries;

    entries = size / (sizeof(XieFloatRec));

    for(i = 0; i < entries; i++)
    /* brackets are mandatory here, because "swapfloat" macro expands
       to several statements */
    {   
	swapfloat(&pbuf[i], n);
    }
    (void)WriteToClient(pClient, size, (char *) pbuf);
}

/*  Write a block of swapped Xie Template IDC records */
void
SwapTmpWrite(pClient, size, pbuf)
    ClientPtr	pClient;
    int		size;  /* in bytes */
    XieTmpEntryRec *pbuf;
{
    register int n;
    int i;
    int entries;
    register XieTmpEntryRec *p;

    entries = size / (sizeof(XieTmpEntryRec));
    p = pbuf;

    while (entries > 0)
    {   
	swapl(&p->x, n);
	swapl(&p->y, n);
	swapfloat(&p->value, n);
	entries--; p++;
    }
    (void)WriteToClient(pClient, size, (char *) pbuf);
}

/*-----------------------------------------------------------------------
---------------------- Write Event Reply to Client ----------------------
------------------------------------------------------------------------*/
int WriteEventToClient(pClient, size, pEvent)
    ClientPtr 		pClient;
    int			size;
    xieSendEvent	*pEvent;
{
    register char n;

    if (pClient->swapped) {
	swaps(&pEvent->sequenceNumber, n);
	swapl(&pEvent->resource_id, n);
    }
    WriteToClient(pClient, size, (char *)pEvent);
	
}

/*-----------------------------------------------------------------------
--------------------- Routine for all simple replies --------------------
------------------------------------------------------------------------*/
int SReplySimple(pClient, size, pRep)
    ClientPtr 		pClient;
    int			size;
    xieCalcHistReply    *pRep;
{
    register char n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    WriteToClient(pClient, size, (char *)pRep);
	
}

/*-----------------------------------------------------------------------
------------------------ Init Session Reply -----------------------------
------------------------------------------------------------------------*/
int SReplyInitSession(pClient, size, pRep)
    ClientPtr 		pClient;
    int			size;
    xieInitSessionReply	*pRep;
{
    register char n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    swaps(&pRep->major_version, n);
    swaps(&pRep->minor_version, n);
    swapl(&pRep->service_class, n);
    WriteToClient(pClient, size, (char *)pRep);
	
}

/*-----------------------------------------------------------------------
------------------------ Query Events Reply -----------------------------
------------------------------------------------------------------------*/
int SReplyQueryEvents(pClient, size, pRep)
    ClientPtr 		pClient;
    int			size;
    xieQueryEventsReply	*pRep;
{
    register char n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    swapl(&pRep->event_mask, n);
    WriteToClient(pClient, size, (char *)pRep);
	
}

/*-----------------------------------------------------------------------
------------------ Query Operational Defaults Reply --------------------- 
------------------------------------------------------------------------*/
int SReplyQueryOpDefaults(pClient, size, pRep)
    ClientPtr 		pClient;
    int			size;
    xieQueryOpDefaultsReply	*pRep;
{
    register char n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    SwapLongs(pRep->levels, XieK_MaxComponents);
    WriteToClient(pClient, size, (char *)pRep);
	
}

/*-----------------------------------------------------------------------
------------------------- Query Resource Reply --------------------------
------------------------------------------------------------------------*/
int SReplyQueryResource(client, size, pRep)
    ClientPtr 		client;
    int			size;
    xieQueryPhotofloReply	*pRep;
{
    register char n;

    REQUEST(xieCreateByValueReq);

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);

    /* The remainder of the reply is different depending on the resource.
     * Get the resource type from the original request. */
    switch( stuff->resource_type ) {
      case XieK_IdcCpp :
          {
	  register xieQueryCppReply *rep = (xieQueryCppReply *)pRep;

	  swapl(&rep->x, n);
	  swapl(&rep->y, n);
	  swapl(&rep->photomap_id, n);
          }
	  break;

      case XieK_Photoflo :
          {
	  register xieQueryPhotofloReply *rep = (xieQueryPhotofloReply *)pRep;

	  swapl(&rep->status, n);
          }
	  break;

      case XieK_Photomap :
          {
	  register xieQueryPhotomapReply *rep = (xieQueryPhotomapReply *)pRep;

	  swapl(&rep->width, n);
	  swapl(&rep->height, n);
	  swapfloat(&rep->aspect_ratio, n);
	  SwapLongs(rep->levels, XieK_MaxComponents);
	  swapl(&rep->photo_id, n);
          }
	  break;

	case XieK_IdcRoi :
	   {
	   register xieQueryRoiReply *rep = (xieQueryRoiReply *)pRep; 

	   swapl(&rep->x, n);
	   swapl(&rep->y, n);
	   swapl(&rep->width, n);
	   swapl(&rep->height, n);
           }
	   break;

	case XieK_IdcTmp :
	   {
	   register xieQueryTmpReply *rep = (xieQueryTmpReply *)pRep; 

	   swapl(&rep->center_x, n);
	   swapl(&rep->center_y, n);
	   swapl(&rep->data_count, n);
           }
	   break;
      }

    WriteToClient(client, size, (char *)pRep);
	
}

/*-----------------------------------------------------------------------
---------------------------- Get Stream Reply ---------------------------
------------------------------------------------------------------------*/
int SReplyGetStream(pClient, size, pRep)
    ClientPtr 		pClient;
    int			size;
    xieGetStreamReply	*pRep;
{
    register char n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    swapl(&pRep->byte_count, n);
    WriteToClient(pClient, size, (char *)pRep);
	
}

/*-----------------------------------------------------------------------
---------------------------- Query Export Reply -------------------------
------------------------------------------------------------------------*/
int SReplyQueryExport(pClient, size, pRep)
    ClientPtr 		pClient;
    int			size;
    xieQueryExportReply	*pRep;
{
    register char n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    swapl(&pRep->pixel_count, n);
    swapl(&pRep->lut_id, n);
    WriteToClient(pClient, size, (char *)pRep);
	
}

/* End of MODULE XIESWAPREP.C */

