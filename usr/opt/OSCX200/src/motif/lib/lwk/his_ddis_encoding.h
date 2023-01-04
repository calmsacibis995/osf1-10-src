/*
** COPYRIGHT (c) 1988 BY
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
**	{@description@}
**
**  Version: {@version-number@}
**
**  Abstract:
**	Common include file for all modules which do DDIS encoding/decoding.
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
**  Creation Date: 15-Nov-1988
**
**  Modification History:
**--
*/


/*
**  Include HIS-dependent DDIS symbol definitions.  These are created from the
**  SDL source files produced by DDIS compiler while processing the Object
**  encoding descriptions.
*/

#ifdef MSDOS
#include "perddis.h"
#include "oidddis.h"
#include "prpddis.h"
#include "perddisu.h"
#include "oidddisu.h"
#include "prpddisu.h"
#else /* !MSDOS */
#include "his_pers_ddis.h"
#include "his_oid_ddis.h"
#include "his_prop_ddis.h"
#include "his_pers_ddis_unique.h"
#include "his_oid_ddis_unique.h"
#include "his_prop_ddis_unique.h"
#endif /* MSDOS */

/*
**  Include generic DDIS symbol definitions.
*/

#if defined(VMS)

#include <ddis$def.h>
#include <ddis$msg.h>

#define CDA_CALLBACK

#elif MSDOS

#include <ddis$def.h>
#include <ddis$msg.h>

#ifndef msdos
#define msdos
#endif /* !msdos */

#include <ddis$ptp.h>

#elif __osf__

#include <ddisdef.h>
#include <ddismsg.h>
#include <ddisprt.h>
#include <ddisptp.h>
#define CDA_CALLBACK

#else /* not VMS, not MSDOS, not OSF */

#include "ddisdef.h"
#include "ddismsg.h"

#define CDA_CALLBACK

#endif


/*
** Definitions
*/

#define _EncodingBufferInitialLength 1000
#define _EncodingBufferIncrement 500

/*
**  DDIS Toolkit access -- Types and Definitions
*/

#if defined(__osf__)
typedef CDAstatus    _DDISstatus,   _PtrTo _DDISstatusPtr;
typedef CDAuserparam _DDISuserparam,_PtrTo _DDISuserparamPtr;
typedef CDAconstant  _DDISconstant, _PtrTo _DDISconstantPtr;
typedef CDAsize      _DDISsize,     _PtrTo _DDISsizePtr;
typedef CDAbufaddr   _DDISdata,     _PtrTo _DDISdataPtr;
typedef DDIStype     _DDIStype,     _PtrTo _DDIStypePtr;
typedef DDISlength   _DDISlength,   _PtrTo _DDISlengthPtr;
typedef DDISentry    _DDISentry,    _PtrTo _DDISentryPtr;
typedef CDAaddress   _DDIStable,    _PtrTo _DDIStablePtr;
#else
typedef unsigned long int _DDISInteger, _PtrTo _DDISIntegerPtr;
typedef unsigned long int _DDISstatus, _PtrTo _DDISstatusPtr;
typedef unsigned _Char _PtrTo _DDISData, _PtrTo _PtrTo _DDISDataPtr;
typedef _DDISInteger _DDISType,     _PtrTo _DDISTypePtr;
typedef _DDISInteger _DDISLength,   _PtrTo _DDISLengthPtr;
typedef _DDISInteger _DDISuserparam,_PtrTo _DDISuserparamPtr;
typedef _DDISInteger _DDISconstant, _PtrTo _DDISconstantPtr;
typedef _DDISInteger _DDISsize,     _PtrTo _DDISsizePtr;
typedef _DDISInteger _DDISdata,     _PtrTo _DDISdataPtr;
typedef _DDISInteger _DDIStype,     _PtrTo _DDIStypePtr;
typedef _DDISInteger _DDISlength,   _PtrTo _DDISlengthPtr;
typedef _DDISInteger _DDISentry,    _PtrTo _DDISentryPtr;
typedef struct ddis_r_parse_table   _PtrTo _DDIStable, _PtrTo _PtrTo _DDIStablePtr;
#endif /* __osf__ */

#define _CreateDDISStream(VString, ParseTableList) \
    LwkCreateDDISStream((VString), (ParseTableList))
#define _OpenDDISStream(VString, ParseTableList) \
    LwkOpenDDISStream((VString), (ParseTableList))
#define _CloseDDISStream(Handle) LwkCloseDDISStream((Handle))

_DeclareFunction(_DDIShandle LwkCreateDDISStream,
    (_VaryingStringPtr encoding, _AnyPtr parse_tables));
_DeclareFunction(_DDIShandle LwkOpenDDISStream,
    (_VaryingString encoding, _AnyPtr parse_tables));
_DeclareFunction(void LwkCloseDDISStream, (_DDIShandle handle));

/*
**  Macro definition for the ddis routines with '$' signs.
**
**  ** Should be removed once all the platforms will have CDA V1.7
*/

#if defined(VMS) || defined(ultrix)

#define ddis_create_stream ddis$create_stream
#define ddis_open_stream   ddis$open_stream
#define ddis_close_stream  ddis$close_stream

#define ddis_put           ddis$put
#define ddis_get_tag       ddis$get_tag
#define ddis_get_value     ddis$get_value

#endif /* VMS || ULTRIX */

/*
** Macro definition for the DDIS constant with $ signs
**
**  ** This should be removed once we use CDA V1.7 on all platforms.
**     The definitions below are the only constants used from the DDIS
**     definition files.
*/

#if defined(VMS) || defined(unix)

#define DDIS_K_T_EOC		DDIS$K_T_EOC
#define DDIS_K_T_OCTET_STRING	DDIS$K_T_OCTET_STRING
#define DDIS_K_T_SEQUENCE	DDIS$K_T_SEQUENCE
#define DDIS_K_F_FLOAT		DDIS$K_F_FLOAT

#define DDIS_NORMAL		DDIS$_NORMAL

#endif /* VMS || ULTRIX || SUN */

