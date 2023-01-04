
/***************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only
**  pursuant to a valid written license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S.
**  Government is subject to restrictions as set forth in Subparagraph
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
***************************************************************************/

/*****************************************************************************
**
**  FACILITY:
**
**      IDS Examples
**
**  ABSTRACT:
**
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.1, DECwindows V1.0
**
**  AUTHOR(S):
**
**      Subu Garikapati
**
**  CREATION DATE:
**
**	09-FEB-1989
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/
/*
** Include Files
*/
#include <stdio.h>

#ifdef VMS
#include <img/IdsImage.h>
#include <MrmAppl.h>
#else
#include <img/IdsImage.h>
#include <Mrm/MrmAppl.h>
#endif
#ifndef NODAS_PROTO
#include <idsprot.h>		/* IDS prototypes */
#endif

/*
**  Table of contents
*/
    /*
    **  PUBLIC -- low level entry points
    */
#ifdef NODAS_PROTO
#ifdef VMS
int  IDSXM$INITIALIZE_FOR_DRM(); /* VMS: register the image and panned widgets*/
#endif
int  IdsXmInitializeForDRM();	 /* C:   register the image and panned widgets*/
#endif


/*
**  External references
*/
#if ( ( defined(__VAXC) || defined(VAXC) ) && (!defined(__alpha) && !defined(ALPHA) ) ) 
#define external globalref
#else
#define external extern
#endif

external WidgetClass imageWidgetMotifClass;
external WidgetClass pannedImageWidgetMotifClass;

/*****************************************************************************
**  IDSXM$INITIALIZE_FOR_DRM
**
**  FUNCTIONAL DESCRIPTION:
**
**      VMS public entry: register the image and panned widgets
**
**  FORMAL PARAMETERS:
**
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**    returns MrmFAILURE -- 0 or MrmSUCCESS -- 1
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/
#ifdef VMS
int IDSXM$INITIALIZE_FOR_DRM()
{
int status;

#ifdef TRACE
printf( "Entering Routine IDSXM$INITIALIZE_FOR_DRM in module IDS_UIL_SUPPORT_MOTIF \n");
#endif
    /*
    **	Call the C version 
    */
    status = IdsXmInitializeForDRM();

#ifdef TRACE
printf( "Leaving Routine IDSXM$INITIALIZE_FOR_DRM in module IDS_UIL_SUPPORT_MOTIF \n");
#endif
    return( status );
}
#endif

/*****************************************************************************
**  IdsXmInitializeForDRM
**
**  FUNCTIONAL DESCRIPTION:
**
**      C public entry: 
**
**  FORMAL PARAMETERS:
**
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**    returns MrmFAILURE -- 0 or MrmSUCCESS -- 1
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/
int  IdsXmInitializeForDRM()
{

#ifdef TRACE
printf( "Entering Routine IdsXmInitializeForDRM in module IDS_UIL_SUPPORT_MOTIF \n");
#endif
    /*
    ** Register image widgets
    */
    if (MrmRegisterClass(MrmwcUnknown,
			 IdsSMotifClassImage,
			 "IdsXmStaticImageCreate",
			 IdsXmStaticImageCreate,
			 imageWidgetMotifClass) != MrmSUCCESS)
        return(MrmFAILURE);

    if (MrmRegisterClass(MrmwcUnknown,
			 IdsSMotifClassPannedImage,
			 "IdsXmPannedImageCreate",
			 IdsXmPannedImageCreate,
			 pannedImageWidgetMotifClass) != MrmSUCCESS)
        return(MrmFAILURE);

#ifdef TRACE
printf( "Leaving Routine IdsXmInitializeForDRM in module IDS_UIL_SUPPORT_MOTIF \n");
#endif
    return(MrmSUCCESS);
}
