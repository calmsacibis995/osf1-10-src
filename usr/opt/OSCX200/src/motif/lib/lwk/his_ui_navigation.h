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
** COPYRIGHT (c) 1988, 1991 BY
** DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
** ALL RIGHTS RESERVED.
**
** THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
** ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
** INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
** COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
** OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
** TRANSFERRED.
**
** THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
** AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
** CORPORATION.
**
** DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
** SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
*/

/*
**++
**  Subsystem:
**	LinkWorks Services User Interface
**
**  Version: X0.1
**
**  Abstract:     
**	Ui Object/Navigation modules shared definitions.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Doug Rayner
**
**  Creation Date: 10-Apr-89
**
**  Modification History:
**  X0.16   WWC  20-Feb-91  facility name HIS --> LWK
**			    new Environment Change property names
**			    _MessageConnect --> _MessageCompleteLink
**			    _MessageUndo --> _MessageGoBack
**			    _ConnectMessage --> _CompleteLinkMessage
**			    _UndoMessage --> _GoBackMessage
**--
*/


/*
**  Include Files
*/

/*
**  Macro Definitions
*/
                                         
#define _CurrencyPropertyPrefix "_DEC_LWK_"
#define _CurrencyPropertyDelimiter "_"

#define _P_EnvironmentManagerAddress "_DEC_LWK_ENVIRONMENT_MANAGER"
                               
#define _P_ApplyDestination  "_DEC_LWK_APPLY_DESTINATION"
#define _P_ApplyOperation "_DEC_LWK_APPLY_OPERATION"

#define _P_ApplyConfirmation  "_DEC_LWK_APPLY_CONFIRMATION"

#define _P_HistoryOrigin "_DEC_LWK_HISTORY_ORIGIN"
#define _P_HistoryDestination  "_DEC_LWK_HISTORY_DESTINATION"
#define _P_HistoryOperation "_DEC_LWK_HISTORY_OPERATION"

#define _P_WIPProperty "_DEC_LWK_WIP"

#define _ThisClientAddress 0
#define _PendingClientAddress -1
#define _UnknownOriginClientAddress -2
#define _UnknownDestinationClientAddress -3
#define _ValidClientAddress(Address) (Address > 0)

#define _PropertyIndicesToNames \
    _String _Constant PropertyNames[lwk_c_env_change_max] = { \
	"_DEC_LWK_CURRENT_HIGHLIGHTING", \
	"_DEC_LWK_CURRENT_COMPOSITENET", \
	"_DEC_LWK_CURRENT_NETWORK", \
	"_DEC_LWK_CURRENT_PATH", \
	"_DEC_LWK_CURRENT_PATH_INDEX", \
	"_DEC_LWK_CURRENT_COMPOSITE_PATH", \
	"_DEC_LWK_CURRENT_TRAIL", \
	"_DEC_LWK_CURRENT_STEP", \
	"_DEC_LWK_CURRENT_CONNECTION", \
	"_DEC_LWK_CURRENT_SOURCE", \
	"_DEC_LWK_CURRENT_TARGET", \
	"_DEC_LWK_CURRENT_FOLLOW", \
	"_DEC_LWK_CURRENT_DESTINATION", \
	"_DEC_LWK_DEFAULT_OPERATION", \
	"_DEC_LWK_DEFAULT_HIGHLIGHTING", \
	"_DEC_LWK_DEFAULT_CONNECTION_TYPE", \
	"_DEC_LWK_DEFAULT_RETAIN_SOURCE", \
	"_DEC_LWK_DEFAULT_RETAIN_TARGET", \
	"_DEC_LWK_RETAIN_SOURCE", \
	"_DEC_LWK_RETAIN_TARGET" \
    }

/*
**  Type Definitions
*/

/*
** LWK Client/EnvironmentManager Message Types
*/

typedef enum __MessageType {
	_MessageApply,
	_MessageApplyConfirmation,
	_MessageCloseView,
	_MessageCompleteLink,
	_MessageShowHistory,
	_MessageUpdateHistory,
	_MessageGoBack,
	_MessageStepForward,
	_MessageClientExit
    } _MessageType;

/*
** The structures below (_*Message) are used for sending Client messages 
** between applications. See LwkOpDXmStateSendMessage in HIS_DWCURRENCY_M.C 
** to see how this is used and passed to XSendEvent. 
** The length of any message instance should not exceed 20 8-bit. We are using 
** a format of 32 in order to have the rigth bit swapping between big-endian
** and little-endian machines. So each field in a message structure has to be
** a 32 bit type.
** On Alpha/OSF1, however, the data in the ClientMessageEvent structure are
** declared as long (64 bits), but before sending the data thru the wire
** (cf. XEvToWire.c in the Xlib) all the long are truncated to 32 bits.
** So, we need to be sure that each of our field will be interpreted as a long
** on Alpha/OSF1, by either declaring it as a long, or as an int but beeing sure
** that it will have an alignment padding.
*/
typedef struct __MessageHeader {
	_MessageType type;
	_Integer checksum;
    } _MessageHeaderInstance, *_MessageHeader;

typedef struct __ApplyMessage {
	_MessageHeaderInstance header;
	_FollowType follow_type;
    } _ApplyMessageInstance, *_ApplyMessage;

typedef struct __ApplyConfirmMessage {
	_MessageHeaderInstance header;
	_Integer client_address;
    } _ApplyConfirmMessageInstance, *_ApplyConfirmMessage;

typedef struct __CloseViewMessage {
	_MessageHeaderInstance header;
    } _CloseViewMessageInstance, *_CloseViewMessage;

typedef struct __ShowHistoryMessage {
	_MessageHeaderInstance header;
    } _ShowHistoryMessageInstance, *_ShowHistoryMessage;

typedef struct __CompleteLinkMessage {
	_MessageHeaderInstance header;
	_Boolean dialog;
    } _CompleteLinkMessageInstance, *_CompleteLinkMessage;

typedef struct __UpdateHistoryMessage {
	_MessageHeaderInstance header;
	_Integer origin_address;
	_FollowType follow_type;
    } _UpdateHistoryMessageInstance, *_UpdateHistoryMessage;

typedef struct __GoBackMessage {
	_MessageHeaderInstance header;
    } _GoBackMessageInstance, *_GoBackMessage;

typedef struct __StepForwardMessage {
	_MessageHeaderInstance header;
    } _StepForwardMessageInstance, *_StepForwardMessage;

typedef struct __ClientExitMessage {
	_MessageHeaderInstance header;
	_Integer client_address;
    } _ClientExitMessageInstance, *_ClientExitMessage;

/*
** Environment Manager Client Support
*/

typedef struct __ClientInstance {
	struct __ClientInstance *next;
	_Surrogate recording_path;
	_Surrogate origin;
	_Surrogate destination;
	_Integer address;
    } _ClientInstance, *_ClientEntry;

/*
**  External Routine Declarations
*/

_DeclareFunction(void LwkUiNavEstablishCurrency, (_Ui ui));
_DeclareFunction(void LwkUiNavEstablishEnvManager, (_Ui ui));
_DeclareFunction(void LwkUiNavRegisterApply, (_Ui ui));
_DeclareFunction(void LwkUiNavUnregisterUi, (_Ui ui));
