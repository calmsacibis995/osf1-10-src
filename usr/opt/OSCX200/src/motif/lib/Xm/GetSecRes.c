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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/lib/dec/Xm/GetSecRes.c,v 1.1.2.4 92/04/15 17:34:48 Dave_Hill Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: @(#)GetSecRes.c	3.16 91/01/10";
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1990, 1991 OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1990, HEWLETT-PACKARD COMPANY
*  ALL RIGHTS RESERVED
*  
*  	THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED
*  AND COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND
*  WITH THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR
*  ANY OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
*  AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
*  SOFTWARE IS HEREBY TRANSFERRED.
*  
*  	THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
*  NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY OPEN SOFTWARE
*  FOUNDATION, INC. OR ITS THIRD PARTY SUPPLIERS  
*  
*  	OPEN SOFTWARE FOUNDATION, INC. AND ITS THIRD PARTY SUPPLIERS,
*  ASSUME NO RESPONSIBILITY FOR THE USE OR INABILITY TO USE ANY OF ITS
*  SOFTWARE .   OSF SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*  KIND, AND OSF EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES, INCLUDING
*  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE.
*  
*  Notice:  Notwithstanding any other lease or license that may pertain to,
*  or accompany the delivery of, this computer software, the rights of the
*  Government regarding its use, reproduction and disclosure are as set
*  forth in Section 52.227-19 of the FARS Computer Software-Restricted
*  Rights clause.
*  
*  (c) Copyright 1989, 1990, 1991 Open Software Foundation, Inc.  Unpublished - all
*  rights reserved under the Copyright laws of the United States.
*  
*  RESTRICTED RIGHTS NOTICE:  Use, duplication, or disclosure by the
*  Government is subject to the restrictions as set forth in subparagraph
*  (c)(1)(ii) of the Rights in Technical Data and Computer Software clause
*  at DFARS 52.227-7013.
*  
*  Open Software Foundation, Inc.
*  11 Cambridge Center
*  Cambridge, MA   02142
*  (617)621-8700
*  
*  RESTRICTED RIGHTS LEGEND:  This computer software is submitted with
*  "restricted rights."  Use, duplication or disclosure is subject to the
*  restrictions as set forth in NASA FAR SUP 18-52.227-79 (April 1985)
*  "Commercial Computer Software- Restricted Rights (April 1985)."  Open
*  Software Foundation, Inc., 11 Cambridge Center, Cambridge, MA  02142.  If
*  the contract contains the Clause at 18-52.227-74 "Rights in Data General"
*  then the "Alternate III" clause applies.
*  
*  (c) Copyright 1989, 1990, 1991 Open Software Foundation, Inc.
*  ALL RIGHTS RESERVED 
*  
*  
* Open Software Foundation is a trademark of The Open Software Foundation, Inc.
* OSF is a trademark of Open Software Foundation, Inc.
* OSF/Motif is a trademark of Open Software Foundation, Inc.
* Motif is a trademark of Open Software Foundation, Inc.
* X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/
#include <Xm/LabelGP.h>
#include <Xm/CascadeBGP.h>
#include <Xm/PushBGP.h>
#include <Xm/ToggleBGP.h>
#include <Xm/SeparatoGP.h>
#include <Xm/BaseClassI.h>
#include <Xm/XmP.h>

#ifdef VAXC
#pragma noinline (GetSecResData)
#endif

#ifdef VMS
externalref XmExtClassRec 	xmExtClassRec;
#endif


static Cardinal GetSecResData ();
static Cardinal _XmFilterExtClassResources();


#ifdef _NO_PROTO
Cardinal XmGetSecondaryResourceData(class, secondaryDataRtn)
    WidgetClass			class;
    XmSecondaryResourceData	**secondaryDataRtn;

#else /* _NO_PROTO */
Cardinal XmGetSecondaryResourceData (WidgetClass class, XmSecondaryResourceData
**secondaryDataRtn)
#endif /* _NO_PROTO */

{   int	done = 0;
    int num = 0;
    XmBaseClassExt  bceptr;
    WidgetClass   lw;

    num =  GetSecResData(class, secondaryDataRtn);


    return (num);
}


/*
 * GetSecResData()
 *  - Called from : XmGetSecondaryResourceData ().
 *
 */
static Cardinal GetSecResData(class, secResDataRtn)
    WidgetClass         class;
    XmSecondaryResourceData **secResDataRtn;
{
    XmBaseClassExt  *bcePtr;   /*  bcePtr is really **XmBaseClassExtRec */
    WidgetClass     secObjClass;
    XmSecondaryResourceData   secResData, *sd;
	Cardinal count = 0;

    bcePtr = _XmGetBaseClassExtPtr(class, XmQmotif); 
    if ((bcePtr) && (*bcePtr) )
	count = ( (*bcePtr)->getSecResData)( class, secResDataRtn);
    return (count);
}

Cardinal
_XmSecondaryResourceData ( bcePtr, secResDataRtn, client_data,
                            name, class, basefunctionpointer)
XmBaseClassExt  bcePtr;
XmSecondaryResourceData **secResDataRtn;
String	name, class;
XtPointer	client_data;
XmResourceBaseProc  basefunctionpointer;
{
    WidgetClass     secObjClass;
    XmSecondaryResourceData   secResData, *sd;
    Cardinal count = 0;
   XtResource      *origResources;
    Cardinal        origNumResources;

    if ( (bcePtr)  )
    { secObjClass = ( (bcePtr)->secondaryObjectClass);
      if (secObjClass)
      {

        if (!(secObjClass->core_class.class_inited))
                 XtInitializeWidgetClass(secObjClass);

        secResData = XtNew(XmSecondaryResourceDataRec);

   /** 1. Get the senondary resource list;
	   2. Filter the resource list to exclude secondary resource of
          object and gadgets; 
	   3. then link the filtered resources to secondary datarecord.
	***/

	    origResources = NULL;
	    origNumResources = 0;

        XtGetResourceList(secObjClass,
                &(origResources),
                &(origNumResources));

     secResData->num_resources =
        _XmFilterExtClassResources(origResources,
                   origNumResources,
                   &(xmExtClassRec) , /* secObjClass, */
                   &(secResData->resources));

	if (origResources)  XtFree ((char *)origResources);

        secResData->name = name;
        secResData->res_class = class;
        secResData->client_data = client_data;
        secResData->base_proc = (XmResourceBaseProc) (basefunctionpointer);
        sd = (XmSecondaryResourceData *)
                XtMalloc ( sizeof (XmSecondaryResourceData )); 
	    *sd = secResData;
        *secResDataRtn = sd;
        count++;
      }
    }
   return (count);
}



static Cardinal _XmFilterExtClassResources(resources, numResources,
		filterClass, filteredResourcesRtn)
    XtResource          *resources;
    Cardinal            numResources;
    WidgetClass         filterClass;
    XtResource          **filteredResourcesRtn;
{
    XtResource      *filteredResources;
    int        copyIndexes[256];
    Cardinal        filterOffset;
    Cardinal        i, j;

    filterOffset = filterClass->core_class.widget_size;

    for (i = 0, j = 0; i < numResources; i++)
      {
      if (resources[i].resource_offset >= filterOffset)
        {
        copyIndexes[i] = i;
        j++;
	}
       else
         copyIndexes[i] = -1;
      }

    filteredResources = (XtResource *) XtMalloc(j * sizeof(XtResource));

    j = 0;
    for (i=0; i < numResources; i++)
      { if (copyIndexes[i] >= 0)
	    { filteredResources[j] = resources[copyIndexes[i]];
          j++;
        }
      }

    if (j > 0) *filteredResourcesRtn = filteredResources;
	  else *filteredResourcesRtn = NULL;

    return (j);
}

