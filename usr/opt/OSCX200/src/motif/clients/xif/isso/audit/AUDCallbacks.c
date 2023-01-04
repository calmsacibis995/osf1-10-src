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
static char *rcsid = "@(#)$RCSfile: AUDCallbacks.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/12/20 21:31:37 $";
#endif

#include <Mrm/MrmAppl.h>                /* Motif Toolkit and MRM */
#include "AUD.h"

/* MODIFY SELECTION FILES */
void AUDModSel1ActivateCB();
void AUDModSelCreateWidgetsCB();
void AUDModSel1CreateFormCB();
void AUDModSel1MapFormCB();
void AUDModSelDoItCB();
void AUDModSelDeleteFileCB();
void AUDModSel1ValueChangedCB();
void AUDModSel2Manage();
void AUDModSelSelectAllCB();
void AUDModSelSelectNoCB();
void AUDModSel1OKCB();
void AUDModSelApplyCB();
void AUDModSel1CancelCB();
void AUDModSel2CreateFormCB();
void AUDModSel2MapFormCB();
void AUDModSel2ValueChangedCB();
void AUDModSel2OKCB();
void AUDModSel2CancelCB();
void AUDModSelTextFocusCB();
void AUDModSelTextLosingFocusCB();

/* GENERATE REPORTS */
void AUDGenRepActivateCB();
void AUDGenRepCreateFormCB();
void AUDGenRepCreateWidgetsCB();
void AUDGenRepMapFormCB();
void AUDGenRepApplyCB();
void AUDGenRepCancelCB();
void AUDGenRepChangeDirCB();
void AUDGenRepChangeLogDirMapCB();
void AUDGenRepChangeLogDirOKCB();
void AUDGenRepChangeLogDirCancelCB();
void AUDGenRepActualReportMapCB();
void AUDGenRepARScaleChangedCB();
void AUDGenRepActualReportCancelCB();
void AUDGenRepStatusWindowMapCB();
void AUDGenRepStatusWindowClearCB();
void AUDGenRepStatusWindowDismissCB();
void AUDGenRepAskNewDirMapCB();
void AUDGenRepAskNewDirOKCB();
void AUDGenRepAskNewDirCancelCB();
void AUDGenRepFullRecMapCB();
void AUDGenRepFullRecCancelCB();
void AUDGenRepStatFileCB();
void AUDGenRepDisplayRepNowCB();
void AUDGenRepDisplayCurActCB();
void AUDGenRepSaveToFileCB();
void AUDGenRepCreateRepAllIDsCB();
void AUDGenRepDeselectionFileCB();

/* MODIFY DESELECTION FILES */
void AUDModDeselActivateCB();
void AUDModDeselCreateWidgetsCB();
void AUDModDeselCreateFormCB();
void AUDModDeselMapFormCB();
void AUDModDeselDoItCB();
void AUDModDeselDeleteFileCB();
void AUDModDeselOKCB();
void AUDModDeselApplyCB();
void AUDModDeselCancelCB();
void AUDModDeselSelectItemCB();
void AUDModDeselAddCB();
void AUDModDeselUpdateCB();
void AUDModDeselRemoveCB();

static MrmRegisterArg reglist [] = {
{"AUDModSel1ActivateCB", (caddr_t)AUDModSel1ActivateCB},
{"AUDModSelCreateWidgetsCB", (caddr_t)AUDModSelCreateWidgetsCB},
{"AUDModSel1CreateFormCB", (caddr_t)AUDModSel1CreateFormCB},
{"AUDModSel1MapFormCB", (caddr_t)AUDModSel1MapFormCB},
{"AUDModSelDoItCB", (caddr_t)AUDModSelDoItCB},
{"AUDModSelDeleteFileCB", (caddr_t)AUDModSelDeleteFileCB},
{"AUDModSel1ValueChangedCB", (caddr_t)AUDModSel1ValueChangedCB},
{"AUDModSel2Manage", (caddr_t)AUDModSel2Manage},
{"AUDModSelSelectAllCB", (caddr_t)AUDModSelSelectAllCB},
{"AUDModSelSelectNoCB", (caddr_t)AUDModSelSelectNoCB},
{"AUDModSel1OKCB", (caddr_t)AUDModSel1OKCB},
{"AUDModSelApplyCB", (caddr_t)AUDModSelApplyCB},
{"AUDModSel1CancelCB", (caddr_t)AUDModSel1CancelCB},
{"AUDModSel2CreateFormCB", (caddr_t)AUDModSel2CreateFormCB},
{"AUDModSel2MapFormCB", (caddr_t)AUDModSel2MapFormCB},
{"AUDModSel2ValueChangedCB", (caddr_t)AUDModSel2ValueChangedCB},
{"AUDModSel2OKCB", (caddr_t)AUDModSel2OKCB},
{"AUDModSelTextFocusCB", (caddr_t)AUDModSelTextFocusCB},
{"AUDModSelTextLosingFocusCB", (caddr_t)AUDModSelTextLosingFocusCB},
{"AUDModSel2CancelCB", (caddr_t)AUDModSel2CancelCB},
{"AUDGenRepActivateCB", (caddr_t)AUDGenRepActivateCB},
{"AUDGenRepCreateFormCB", (caddr_t)AUDGenRepCreateFormCB},
{"AUDGenRepCreateWidgetsCB", (caddr_t)AUDGenRepCreateWidgetsCB},
{"AUDGenRepMapFormCB", (caddr_t)AUDGenRepMapFormCB},
{"AUDGenRepApplyCB", (caddr_t)AUDGenRepApplyCB},
{"AUDGenRepCancelCB", (caddr_t)AUDGenRepCancelCB},
{"AUDGenRepChangeDirCB", (caddr_t)AUDGenRepChangeDirCB},
{"AUDGenRepChangeLogDirMapCB", (caddr_t)AUDGenRepChangeLogDirMapCB},
{"AUDGenRepChangeLogDirOKCB", (caddr_t)AUDGenRepChangeLogDirOKCB},
{"AUDGenRepChangeLogDirCancelCB", (caddr_t)AUDGenRepChangeLogDirCancelCB},
{"AUDGenRepActualReportMapCB", (caddr_t)AUDGenRepActualReportMapCB},
{"AUDGenRepARScaleChangedCB", (caddr_t)AUDGenRepARScaleChangedCB},
{"AUDGenRepActualReportCancelCB", (caddr_t)AUDGenRepActualReportCancelCB},
{"AUDGenRepStatusWindowMapCB", (caddr_t)AUDGenRepStatusWindowMapCB},
{"AUDGenRepStatusWindowClearCB", (caddr_t)AUDGenRepStatusWindowClearCB},
{"AUDGenRepStatusWindowDismissCB", (caddr_t)AUDGenRepStatusWindowDismissCB},
{"AUDGenRepAskNewDirMapCB", (caddr_t)AUDGenRepAskNewDirMapCB},
{"AUDGenRepAskNewDirOKCB", (caddr_t)AUDGenRepAskNewDirOKCB},
{"AUDGenRepAskNewDirCancelCB", (caddr_t)AUDGenRepAskNewDirCancelCB},
{"AUDGenRepFullRecMapCB", (caddr_t)AUDGenRepFullRecMapCB},
{"AUDGenRepFullRecCancelCB", (caddr_t)AUDGenRepFullRecCancelCB},
{"AUDGenRepStatFileCB", (caddr_t)AUDGenRepStatFileCB},
{"AUDGenRepDisplayRepNowCB", (caddr_t)AUDGenRepDisplayRepNowCB},
{"AUDGenRepDisplayCurActCB", (caddr_t)AUDGenRepDisplayCurActCB},
{"AUDGenRepSaveToFileCB", (caddr_t)AUDGenRepSaveToFileCB},
{"AUDGenRepCreateRepAllIDsCB", (caddr_t)AUDGenRepCreateRepAllIDsCB},
{"AUDGenRepDeselectionFileCB", (caddr_t)AUDGenRepDeselectionFileCB},
{"AUDModDeselActivateCB", (caddr_t)AUDModDeselActivateCB},
{"AUDModDeselCreateWidgetsCB", (caddr_t)AUDModDeselCreateWidgetsCB},
{"AUDModDeselCreateFormCB", (caddr_t)AUDModDeselCreateFormCB},
{"AUDModDeselMapFormCB", (caddr_t)AUDModDeselMapFormCB},
{"AUDModDeselDoItCB", (caddr_t)AUDModDeselDoItCB},
{"AUDModDeselDeleteFileCB", (caddr_t)AUDModDeselDeleteFileCB},
{"AUDModDeselOKCB", (caddr_t)AUDModDeselOKCB},
{"AUDModDeselApplyCB", (caddr_t)AUDModDeselApplyCB},
{"AUDModDeselCancelCB", (caddr_t)AUDModDeselCancelCB},
{"AUDModDeselSelectItemCB", (caddr_t)AUDModDeselSelectItemCB},
{"AUDModDeselAddCB", (caddr_t)AUDModDeselAddCB},
{"AUDModDeselUpdateCB", (caddr_t)AUDModDeselUpdateCB},
{"AUDModDeselRemoveCB", (caddr_t)AUDModDeselRemoveCB},
};

static int reglist_num = (sizeof reglist / sizeof reglist[0]);

AUDGlobalData	audglobal;

void
AUDInitialize()
{
    MrmRegisterNames (reglist, reglist_num);

    /* Initialize global data */
    audglobal.baseEventList           = (char **) NULL;
    audglobal.baseEventCount          = 0;
    audglobal.baseEventSerialNo       = 0;
    audglobal.siteEventList           = (char **) NULL;
    audglobal.siteEventCount          = 0;
    audglobal.siteEventSerialNo       = 0;
    audglobal.aliasEventList          = (char **) NULL;
    audglobal.aliasEventCount         = 0;
    audglobal.aliasEventSerialNo      = 0;
    audglobal.selectionFiles          = (char **) NULL;
    audglobal.numSelectionFiles       = 0;
    audglobal.selectionFileSerialNo   = 0;
    audglobal.deselectionFiles        = (char **) NULL;
    audglobal.numDeselectionFiles     = 0;
    audglobal.deselectionFileSerialNo = 0;
    audglobal.auditLogDirList         = (char **) NULL;
    audglobal.auditLogDirListCount    = 0;
    audglobal.auditLogDirListSerialNo = 0;
    audglobal.remoteHostList          = (char **) NULL;
    audglobal.remoteHostListCount     = 0;
    audglobal.remoteHostListSerialNo  = 0;

}
void AUDModSel1ActivateCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
VUIT_Manage("AUDModSel1FRM");
VUIT_Create("AUDModSel2FRM");
}

void AUDModSelCreateWidgetsCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
AUDModSelCreateWidgetsCallback(w, tag, reason);
}

void AUDModSel1CreateFormCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
AUDModSel1CreateFormCallback(w, tag, reason);
}

void AUDModSel1MapFormCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
AUDModSel1MapFormCallback (w, tag, reason);
}

void AUDModSelDoItCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
AUDModSelDoItCallback(w, tag, reason);
}

void AUDModSelDeleteFileCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
 AUDModSelDeleteFileCallback(w, tag, reason);
}

void AUDModSel1ValueChangedCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
 AUDModSel1ValueChangedCallback(w, tag, reason);
}

void AUDModSel2Manage (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
VUIT_Manage("AUDModSel2FRM");
}

void AUDModSelSelectAllCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
 AUDModSelSelectAllCallback (w, tag, reason);
}

void AUDModSelSelectNoCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
 AUDModSelSelectNoCallback (w, tag, reason);
}

void AUDModSel1OKCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
   AUDModSel1OKCallback(w, tag, reason);
}

void AUDModSelApplyCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
   AUDModSelApplyCallback(w, tag, reason);
}

void AUDModSel1CancelCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
   AUDModSel1CancelCallback(w, tag, reason);
}

void AUDModSel2CreateFormCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
AUDModSel2CreateFormCallback(w, tag, reason);
}

void AUDModSel2MapFormCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
AUDModSel2MapFormCallback(w, tag, reason);
}

void AUDModSel2ValueChangedCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
AUDModSel2ValueChangedCallback (w, tag, reason);
}

void AUDModSel2OKCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
 AUDModSel2OKCallback (w, tag, reason);
}

void AUDModSel2CancelCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
 AUDModSel2CancelCallback (w, tag, reason);
}

void AUDModSelTextFocusCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
 AUDModSelTextFocusCallback (w, tag, reason);
}

void AUDModSelTextLosingFocusCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
 AUDModSelTextLosingFocusCallback (w, tag, reason);
}

void AUDGenRepActivateCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
    VUIT_Manage("AUDGenRepFRMD");

}

void AUDGenRepCreateWidgetsCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
AUDGenRepCreateWidgetsCallback(w, tag, reason);
}

void AUDGenRepCreateFormCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
AUDGenRepCreateFormCallback(w, tag, reason);
}

void AUDGenRepMapFormCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
AUDGenRepMapFormCallback (w, tag, reason);
}

void AUDGenRepApplyCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
   AUDGenRepApplyCallback(w, tag, reason);
}

void AUDGenRepCancelCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
   AUDGenRepCancelCallback(w, tag, reason);
}

void AUDGenRepChangeDirCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
   AUDGenRepChangeDirCallback(w, tag, reason);
}

void AUDGenRepChangeLogDirMapCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
   AUDGenRepChangeLogDirMapCallback(w, tag, reason);
}

void AUDGenRepChangeLogDirOKCB(w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
   AUDGenRepChangeLogDirOKCallback(w, tag, reason);
}

void AUDGenRepChangeLogDirCancelCB(w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
   AUDGenRepChangeLogDirCancelCallback(w, tag, reason);
}

void AUDGenRepActualReportMapCB(w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
   AUDGenRepActualReportMapCallback(w, tag, reason);
}

void AUDGenRepARScaleChangedCB(w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
   AUDGenRepARScaleChangedCallback(w, tag, reason);
}

void AUDGenRepActualReportCancelCB(w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
   AUDGenRepActualReportCancelCallback(w, tag, reason);
}

void AUDGenRepStatusWindowMapCB(w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
    AUDGenRepStatusWindowMapCallback (w, tag, reason);
}

void AUDGenRepStatusWindowClearCB(w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
    AUDGenRepStatusWindowClearCallback  (w, tag, reason);
}

void AUDGenRepStatusWindowDismissCB(w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
    AUDGenRepStatusWindowDismissCallback   (w, tag, reason);
}

void AUDGenRepAskNewDirMapCB(w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
    AUDGenRepAskNewDirMapCallback    (w, tag, reason);
}

void AUDGenRepAskNewDirOKCB(w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
    AUDGenRepAskNewDirOKCallback     (w, tag, reason);
}

void AUDGenRepAskNewDirCancelCB(w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
    AUDGenRepAskNewDirCancelCallback(w, tag, reason);
}

void AUDGenRepFullRecMapCB(w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
    AUDGenRepFullRecMapCallback(w, tag, reason);
}

void AUDGenRepFullRecCancelCB(w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
    AUDGenRepFullRecCancelCallback(w, tag, reason);
}

void AUDGenRepStatFileCB(w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
    AUDGenRepStatFileCallback(w, tag, reason);
}

void AUDGenRepDisplayRepNowCB(w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
    AUDGenRepDisplayRepNowCallback(w, tag, reason);
}

void AUDGenRepDisplayCurActCB(w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
    AUDGenRepDisplayCurActCallback(w, tag, reason);
}

void AUDGenRepSaveToFileCB(w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
    AUDGenRepSaveToFileCallback(w, tag, reason);
}

void AUDGenRepCreateRepAllIDsCB(w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
    AUDGenRepCreateRepAllIDsCallback(w, tag, reason);
}

void AUDGenRepDeselectionFileCB(w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
    AUDGenRepDeselectionFileCallback(w, tag, reason);
}

void AUDModDeselActivateCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
VUIT_Manage("AUDModDeselFRM");
}

void AUDModDeselCreateWidgetsCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
AUDModDeselCreateWidgetsCallback(w, tag, reason);
}

void AUDModDeselCreateFormCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
AUDModDeselCreateFormCallback(w, tag, reason);
}

void AUDModDeselMapFormCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
AUDModDeselMapFormCallback (w, tag, reason);
}

void AUDModDeselDoItCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
AUDModDeselDoItCallback(w, tag, reason);
}

void AUDModDeselDeleteFileCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
 AUDModDeselDeleteFileCallback(w, tag, reason);
}

void AUDModDeselOKCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
   AUDModDeselOKCallback(w, tag, reason);
}

void AUDModDeselApplyCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
   AUDModDeselApplyCallback(w, tag, reason);
}

void AUDModDeselCancelCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
   AUDModDeselCancelCallback(w, tag, reason);
}

void AUDModDeselSelectItemCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
   AUDModDeselSelectItemCallback(w, tag, reason);
}

void AUDModDeselAddCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
   AUDModDeselAddCallback(w, tag, reason);
}

void AUDModDeselUpdateCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
   AUDModDeselUpdateCallback(w, tag, reason);
}

void AUDModDeselRemoveCB (w, tag, reason)
Widget		w;
int		*tag;
unsigned long	*reason;
{
   AUDModDeselRemoveCallback(w, tag, reason);
}
