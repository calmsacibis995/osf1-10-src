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
#ifdef MODE_SWITCH
/************************************************************************
Copyright 1990 by Digital Equipment Corporation, Maynard, Massachusetts.
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

************************************************************************/

#define KMEMAJORVERSION 1
#define KMEMINORVERSION 0

#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "extnsionst.h"
#include "dixstruct.h"
#include "os.h"
#include "xkmeproto_include.h"

static int  KMEReqBase;		/* major opcode base for this extension */

/*
 * macro definition
 */
#define WriteKMEReplyToClient(pclient,size,reply,opcode) \
    { \
        WriteToClient(pclient,size,(char *)(reply)); \
   }

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      KMEInit - Keyboard Management Extension Initialization routine.
**      This routine is called at the server system initialization 
**      time and server reset time.
**
**  FORMAL PARAMETERS:
**
**      none
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      The request opcode base of the extension is returned by 
**      AddExtension call. 
**
**  FUNCTION VALUE:
**
**      none
**
**  SIDE EFFECTS:
**
**      If this extension is not loaded, it causes the server to 
**	fail.
**
**--
**/
int
KMEInit()
{
   ExtensionEntry *extensionEntry, *AddExtension();
   int	KMEProcDispatch();
   int  sKMEProcDispatch();
   void KMEProcReset();

   extensionEntry = AddExtension(KMEXTENSIONNAME, 0, 0, KMEProcDispatch,
		    sKMEProcDispatch, KMEProcReset, StandardMinorOpcode);

   if( extensionEntry ) {

       KMEReqBase = extensionEntry->base;
       MakeAtom(KMEXTENSIONNAME, strlen(KMEXTENSIONNAME), TRUE);

   } else {

     FatalError("KME/KMEInit: Add extension failure\n");

   }

}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      KMEProcReset
**
**  FORMAL PARAMETERS:
**
**	none
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
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
void
KMEProcReset()
{
   
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      KMEProcDispatch - Server Management Extension Dispatch routine
**
**  FORMAL PARAMETERS:
** 
**      stuff - pointer to request stream
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
**      Minor opcode process status is returned
**
**  SIDE EFFECTS:
**
**      The state of the running server may be effected.
**	Some server control parameters may be changed
**      Client name may be added
**
**--
**/
int
KMEProcDispatch(client)
ClientPtr client;
{
    REQUEST(xReq);

    switch( stuff->data)
      {
	case X_KMEDoKBModeSwitch :
	  return(KMEProcDoKBModeSwitch(client));

        default:
          {
            return(BadRequest);
          }
      }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      sKMEProcDispatch - Keyboard Management Extension Dispatcher
**                         (byteswapped)
**
**  FORMAL PARAMETERS:
**
**	client - pointer to the client structure
**
**  IMPLICIT INPUTS:
**
**      [@description_or_none@]
**
**  IMPLICIT OUTPUTS:
**
**      [@description_or_none@]
**
**  {@function_value_or_completion_codes@}
**
**      [@description_or_none@]
**
**  SIDE EFFECTS:
**
**      [@description_or_none@]
**
**--
**/
int
sKMEProcDispatch(client)
register ClientPtr client;
{
    return(BadRequest);
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      KMEProcDoKBModeSwitch 
**
**	This function allows a smart X client to instruct
**	the server to cause the keyboard switch mode from 
**	group 1 to group 2 or vice versa.  This is basically
**	emulate the function of having the user keys in
**	the proper key combination(s) that is bound to the 
**	mode-switch modifier, XK_MODE_SWITCH.
**	For V3, the function would only allow the smart X client
**	to lock down the keyboard mode-switching function or unlock it.
**
**  FORMAL PARAMETERS:
**
**      client record structure 
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	One of the following status will be returned in the reply to
**	the client:
**	   KBModeSwitchSuccess, 
**         KBModeSwitchFailure,
**	   KBModeSwitchInvalidCmd,
**         KBModeSwitchNoop,
**
**  FUNCTION VALUE:
**
**	always return success status
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
int
KMEProcDoKBModeSwitch(client)
register ClientPtr client;
{
    REQUEST(xKMEDoKBModeSwitchReq);
    xKMEDoKBModeSwitchRep Reply;

    REQUEST_AT_LEAST_SIZE(xKMEDoKBModeSwitchReq);

    /* Non-zero status means keyboard mode-switch command failed*/

    Reply.status = DoKeyboardModeSwitch(stuff->mode);

    Reply.type = X_Reply;
    Reply.sequenceNumber = client->sequence;
    Reply.length = 0;

    WriteKMEReplyToClient(client,(sizeof(xKMEDoKBModeSwitchRep)),&Reply,stuff->minor_opcode);

    return(client->noClientException);
}
#endif MODE_SWITCH
