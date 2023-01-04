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
static char *rcsid = "@(#)$RCSfile: DXmMess.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/05/07 01:31:40 $";
#endif
/*
*****************************************************************************

              Copyright (c) Digital Equipment Corporation, 1990,1991,1992,1993
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
**      DECwindows Toolkit
**
**  ABSTRACT:
**
**	DXm Message facilty for characters arrays to replace statically
**	defined messages      
**
**
**  MODIFICATION HISTORY:
**
**      1 Feb 1993  Begin work from Messages.c in Motif 1.2.1
**
**
**--
**/



#include <Xm/XmP.h>

/* Define _XmConst before including MessagesI.h, so that the
 * declarations will be in agreement with the definitions.
 */
#ifndef _XmConst
#define _XmConst XmConst
#endif /* _XmConst */

#include "DXmMessI.h"

/*
 * This is to allow for the messages in DXm to be converted to
 * characters arrays, in the same fashion as they are in Xm.
 */

/*************** ColorMix.c *****************/

_XmConst char _DXmMsgColorMix_0000[] =
	"Error allocating color cells - color map probably full";

/*************** FAO_Main.c *****************/

_XmConst char _DXmMsgFAO_0000[] =
	"DDIS CS functions have returned an undefined status";

_XmConst char _DXmMsgFAOName_0000[] =
	"fao_main";

/*************** Help_Shell.c *****************/

_XmConst char _DXmMsgHlpShl_0000[] =
	"DXmHelpShell widget only supports one child";

_XmConst char _DXmMsgHlpShlName_0000[] =
	"DXmHelpShellOneChild";

/*************** Help_Widget.c *****************/

_XmConst char _DXmMsgHlpWid_0000[] =
	"Incorrect dialog style.";

/*************** Pane.c *********************/

_XmConst char _DXmMsgPane_0000[] =
	"Never found the subwidget in Pane.";

_XmConst char _DXmMsgPane_0001[] =
	"PaneGetMinMax called with bad widget.";

/*************** PringWgt.c *****************/

_XmConst char _DXmMsgPrintWgt_0000[] =
	"Required parameters to DXmCreatePrintBox are missing";

_XmConst char _DXmMsgPrintWgt_0001[] =
	"Required parameters to DXmCreatePrintDialog are missing";

_XmConst char _DXmMsgPrintWgt_0002[] =
	"Error converting filename from compound string";

_XmConst char _DXmMsgPrintWgt_0003[] =
	"Could not fetch %s from Mrm";

_XmConst char _DXmMsgPrintWgt_0004[] =
	"Error converting printer from compound string";

_XmConst char _DXmMsgPrintWgt_0005[] =
	"Error converting printer form from compound string";

_XmConst char _DXmMsgPrintWgt_0006[] =
	"Error converting time from compound string";

_XmConst char _DXmMsgPrintWgt_0007[] =
	"Could not open print widget Mrm hierarchy";

_XmConst char _DXmMsgPWgtName_0000[] =
	"incompleteParameterList";

_XmConst char _DXmMsgPWgtName_0001[] =
	"badFilename";

_XmConst char _DXmMsgPWgtName_0002[] =
	"noFetch";

_XmConst char _DXmMsgPWgtName_0003[] =
	"badPrinter";

_XmConst char _DXmMsgPWgtName_0004[] =
	"badPrinterForm";

_XmConst char _DXmMsgPWgtName_0005[] =
	"badTime";

_XmConst char _DXmMsgPWgtName_0006[] =
	"noHierarchy";

/*************** PWSendJob.c *****************/

_XmConst char _DXmMsgPWSend_0000[] =
	"Could not open %s";

_XmConst char _DXmMsgPWSend_0001[] =
	"Could not fork a subprocess";

_XmConst char _DXmMsgPWSend_0002[] =
	"Could not execute '%s': %s";

_XmConst char _DXmMsgPWSendName_0000[] =
	"noFileOpen";

_XmConst char _DXmMsgPWSendName_0001[] =
	"noFork";

_XmConst char _DXmMsgPWSendName_0002[] =
	"noExecvp";

