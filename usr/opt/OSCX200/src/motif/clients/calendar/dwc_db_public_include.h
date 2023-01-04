#ifndef _dwc_db_public_include_h_
#define _dwc_db_public_include_h_ 1
/* $Header$ */
/* dwc_db_public_include.h */
/*
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**++
**  FACILITY:
**
**	DECwindows Calendar; database access routines
**
**  AUTHOR:
**
**	Per Hamnqvist, November 1987
**
**  ABSTRACT:
**
**	This module contains the "public" interface to the database routines.
**	Public means -- for the DECwindows UI code. This module contains things
**	such as routine prototype definitions, etc, such that calls can be
**	checked by LINT.
**
**--
*/

/*
**  Include Files
*/
#include "dwc_db_public_structures.h"
#include "dwc_db_public_const.h"
#include "dwc_compat.h"

/*
**  Macro Definitions
*/
#define DWC$CAB(Cabname)	struct DWC$db_access_block *Cabname

#define _Failure -1
#define _Success 0

#ifndef TRUE
#define TRUE (1)
#define FALSE (0)
#endif

#ifndef NULL
#define NULL (0)
#endif

/*
**  Different codes used for P1 with repeat expressions
*/
#define _REPEAT_NONE 0
#define _REPEAT_ABSOLUTE 1
/*
**  Different codes used for P2 with repeat expressions
*/
#define _REPEAT_UNDEFINED 0

/*
**  Function prototypes for public interface to the database. Routines not
**  listed here should not be considered as part of the routines that can
**  be used by anyone except the DB routines internally.
**
**  PLEASE NOTE: THE PARAMETERS ASSOCIATED WITH A ROUTINE ARE LISTED BELOW
**  THE ROUTINE NAME.
*/
int	DWC$DB_Create_calendar PROTOTYPE ((
	char *Calname,
	int Delta_now,
	int *C_errno,
	int *Vaxc_errno));

int	DWC$DB_Open_calendar PROTOTYPE ((
	char *Name,
	struct DWC$db_access_block **Cab,
	int Force_upg,
	int *Callback,
	int User_param,
	int *C_errno,
	int *Vaxc_errno
    ));

int	DWC$DB_Close_calendar PROTOTYPE ((  DWC$CAB (Cab)  ));

int	DWC$DB_Put_r_item PROTOTYPE ((
	DWC$CAB			(Cab),
	int			*Item_id,
	int			Start_day,
	int			Start_min,
	int			Duration_days,
	int			Duration_min,
	int			Alarm_vec_len,
	unsigned short int	Alarm_vec[],
	int			Flags,
	int			Input_text_len,
	char			Input_text_ptr[],
	int			Input_text_class,
	int			P1,
	int			P2,
	int			P3,
	int			End_day,
	int			End_min));

int	DWC$DB_Get_r_item PROTOTYPE ((
	DWC$CAB			(Cab),
	int			From_day,
	int			*Next_item_id,
	int			*Rep_start_day,
	int			*Rep_start_min,
	int			*Start_min,
	int			*Duration_days,
	int			*Duration_min,
	int			*Alarm_vec_len,
	unsigned short int	**Alarm_vec,
	int			*Flags,
	int			*Output_text_len,
	char			**Output_text_ptr,
	int			*Output_text_class,
	int			*P1,
	int			*P2,
	int			*P3,
	int			*End_day,
	int			*End_min));

int	DWC$DB_Modify_r_item PROTOTYPE ((
	DWC$CAB			(Cab),
	int			Item_id,
	int			Start_day,
	int			Start_min,
	int			Not_used_1,
	int			Not_used_2,
	int			Duration_days,
	int			Duration_min,
	int			Alarm_vec_len,
	unsigned short int	Alarm_vec[],
	int			Flags,
	int			Input_text_len,
	char			Input_text_ptr[],
	int			Input_text_class,
	int			P1,
	int			P2,
	int			P3,
	int			End_day,
	int			End_min));

int	DWC$DB_Delete_r_item PROTOTYPE ((
	DWC$CAB			(Cab),
	int			Item_id,
	int			Start_day,
	int			Delete_all));

int	DWC$DB_Get_specific_r_item PROTOTYPE ((
	DWC$CAB			(Cab),
	int			From_day,
	int			Item_id,
	int			*Rep_start_day,
	int			*Rep_start_min,
	int			*Start_min,
	int			*Duration_days,
	int			*Duration_min,
	int			*Alarm_vec_len,
	unsigned short int	**Alarm_vec,
	int			*Flags,
	int			*Output_text_len,
	char			**Output_text_ptr,
	int			*Output_text_class,
	int			*P1,
	int			*P2,
	int			*P3,
	int			*End_day,
	int			*End_min));

int DWCDB_PutRItem PROTOTYPE ((
	DWC$CAB			(Cab),
	int			*Item_id,
	int			Start_day,
	int			Start_min,
	int			Duration_days,
	int			Duration_min,
	int			Alarm_vec_len,
	unsigned short int	Alarm_vec[],
	int			Flags,
	unsigned char		*Input_text_ptr,    /* XmString */
	int			Input_text_class,
	unsigned int		Input_icon_num,
	unsigned char		Input_icon_ptr[],
	int			P1,
	int			P2,
	int			P3,
	int			End_day,
	int			End_min));

int DWCDB_GetRItem PROTOTYPE ((
	DWC$CAB			(Cab),
	int			From_day,
	int			*Next_item_id,
	int			*Rep_start_day,
	int			*Rep_start_min,
	int			*Start_min,
	int			*Duration_days,
	int			*Duration_min,
	int			*Alarm_vec_len,
	unsigned short int	**Alarm_vec,
	int			*Flags,
	unsigned char		**Output_text_ptr,  /* XmString * */
	int			*Output_text_class,
	unsigned int		*Output_icon_num,
	unsigned char		**Output_icon_ptr,
	int			*P1,
	int			*P2,
	int			*P3,
	int			*End_day,
	int			*End_min));

int	DWCDB_ModifyRItem PROTOTYPE ((
	DWC$CAB			(Cab),
	int			Item_id,
	int			Start_day,
	int			Start_min,
	int			Not_used_1,
	int			Not_used_2,
	int			Duration_days,
	int			Duration_min,
	int			Alarm_vec_len,
	unsigned short int	Alarm_vec[],
	int			Flags,
	unsigned char		*Input_text_ptr,    /* XmString */
	int			Input_text_class,
	unsigned int		Input_icon_num,
	unsigned char		Input_icon_ptr[],
	int			P1,
	int			P2,
	int			P3,
	int			End_day,
	int			End_min
	));

int	DWCDB_GetSpecificRItem PROTOTYPE ((
	DWC$CAB			(Cab),
	int			From_day,
	int			Item_id,
	int			*Rep_start_day,
	int			*Rep_start_min,
	int			*Start_min,
	int			*Duration_days,
	int			*Duration_min,
	int			*Alarm_vec_len,
	unsigned short int	**Alarm_vec,
	int			*Flags,
	unsigned char		**Output_text_ptr,  /* XmString * */
	int			*Output_text_class,
	unsigned int		*Output_icon_num,
	unsigned char		**Output_icon_ptr,
	int			*P1,
	int			*P2,
	int			*P3,
	int			*End_day,
	int			*End_min));


int DWC$DB_Get_profile PROTOTYPE ((
	DWC$CAB			(Cab),
	int			Request_len,
	void			*Profile_ptr));

int
DWC$DB_Modify_profile PROTOTYPE ((
	DWC$CAB			(Cab),
	void			*Param_ptr,
	int			Param_len));

int
DWC$DB_Put_day_attr PROTOTYPE ((
	DWC$CAB			(Cab),
	unsigned long		Day,
	unsigned char		Daymark,
	unsigned char		Dayflags));

int
DWC$DB_Get_day_attr PROTOTYPE ((
	DWC$CAB			(Cab),
	unsigned long		Day,
	unsigned char		*Daymark,
	unsigned char		*Dayflags,
	unsigned char		*Dayused));

int
DWC$DB_Get_day_attr_rng PROTOTYPE ((
	DWC$CAB			(Cab),
	unsigned long		Start_day,
	unsigned long		End_day,
	unsigned char		*Daymark_vec,
	unsigned char		*Dayflag_vec,
	unsigned char		*Dayused_vec));

int
DWC$DB_Get_next_alarm_time PROTOTYPE ((
	DWC$CAB			(Cab),
	unsigned int		Start_day,
	unsigned short int	Start_min,
	unsigned int		*Alarm_day,
	unsigned short int	(*Alarm_min)));

int
DWC$DB_Get_next_alarm_id PROTOTYPE ((
	DWC$CAB			(Cab),
	unsigned int		*Day,
	int			*Id));

int
DWC$DB_Check_overlap PROTOTYPE ((
	DWC$CAB			(Cab),
	unsigned long		Day,
	int			Start_time,
	int			Duration_minutes,
	int			*Over_id));

int
DWC$DB_Rename_calendar PROTOTYPE ((
	DWC$CAB			(Cab),
	char			New_name[]));

int
DWC$DB_Get_calendar_name PROTOTYPE ((
	DWC$CAB			(Cab),
	char			*Calname[]));

int DWC$DB_Get_next_repeat_expr PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	int *Next_item_id,
	int *Rep_start_day,
	int *Rep_start_min,
	int *Duration_days,
	int *Duration_min,
	int *Alarm_vec_len,
	unsigned short int **Alarm_vec,
	int *Flags,
	int *Output_text_len,
	char **Output_text_ptr,
	int *Output_text_class,
	int *P1,
	int *P2,
	int *P3,
	int *P4,
	int *P5,
	int *P6,
	int *End_day,
	int *End_min,
	struct DWC$db_exception_head **Exception_list_head_ptr));

int DWCDB_GetNextRepeatExpr PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	int *Next_item_id,
	int *Rep_start_day,
	int *Rep_start_min,
	int *Duration_days,
	int *Duration_min,
	int *Alarm_vec_len,
	unsigned short int **Alarm_vec,
	int *Flags,
	unsigned char **Output_text_ptr,
	int *Output_text_class,
	unsigned int *Output_icons_num,
	unsigned char **Output_icon_ptr,
	int *P1,
	int *P2,
	int *P3,
	int *P4,
	int *P5,
	int *P6,
	int *End_day,
	int *End_min,
	struct DWC$db_exception_head **Exception_list_head_ptr));

int DWC$DB_Build_time_ctx PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int Day,
	int Minute,
	struct DWC$db_time_ctx *Timec));

/*
** dwc_db_open_close.c
*/
int DWC$DB_Update_statistics PROTOTYPE ((
	struct DWC$db_access_block  *Cab));

/*
** functions from dwc_db_trading.c
*/
#ifdef vaxc
/*
** The function pointers seem to get really confused between what is allowed
** in VAXC and what is needed in DECC.
*/
int DWC$DB_Write_interchange PROTOTYPE ((
	struct DWC$db_access_block  *Cab,
	struct DWC$db_interchange   *work_context,
	void			    (*date_fmt) (),
	void			    (*time_fmt) (),
	void			    *Uparam,
	char			    **Int_arr));
#else
int DWC$DB_Write_interchange ();
#endif

int
DWC$DB_Create_i_handle PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange **work_context));

int
DWC$DB_Put_i_item PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	int Day,
	int Start,
	int Duration,
	unsigned char *Data_ptr,
	int Data_len,
	int Data_class));

int DWC$DB_Load_interchange PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	char *Work_file,
	char **Int_arr,
	int *c_err,
	int *vaxc_err));

int DWCDB_LoadInterchange PROTOTYPE ((
    struct DWC$db_access_block *Cab,
    char *Work_file,
    unsigned char **Int_arr,
    int *c_err,
    int *vaxc_err));

int
DWC$DB_Rundown_interchange PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context));

int
DWC$DB_Get_next_i_item PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	int *Day,
	int *Start,
	int *Duration,
	unsigned char **Data_ptr,
	int *Data_len,
	int *Data_class));

int
DWC$DB_Get_all_i_texts PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	char **Int_arr));

int
DWC$DB_Parse_interchange PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	char *tokens,
	struct DWC$db_interchange **work_context,
	int *err_line));

#ifdef vaxc
/*
** The function pointers seem to get really confused between what is allowed
** in VAXC and what is needed in DECC.
*/
int DWC$DB_Create_interchange PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	int Start_day,
	int End_day,
	int Item_id,
	void (*date_fmt)(),
	void (*time_fmt)(),
	void *Uparam,
	char **Int_arr));
#else
int DWC$DB_Create_interchange ();
#endif

#ifdef vaxc
/*
** The function pointers seem to get really confused between what is allowed
** in VAXC and what is needed in DECC.
*/
int DWCDB_WriteInterchange PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	char *(*date_fmt)(),
	char *(*time_fmt)(),
	void *Uparam,
	unsigned char **Int_arr));  /* XmString * */
#else
int DWCDB_WriteInterchange ();
#endif

int DWCDB_CreateIHandle PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange **work_context));

int DWCDB_PutIItem PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	int Day,
	int Start,
	int Duration,
	unsigned char *Data_ptr,
	int Data_class));

int DWCDB_LoadInterchange PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	char *Work_file,
	unsigned char **Int_arr,    /* XmString * */
	int *c_err,
	int *vaxc_err));

int DWCDB_RundownInterchange PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context));

int DWCDB_GetNextIItem PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	int *Day,
	int *Start,
	int *Duration,
	unsigned char **Data_ptr,   /* XmString * */
	int *Data_len,
	int *Data_class));

int DWCDB_GetAllITexts PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	unsigned char **Int_arr));  /* XmString * */

int DWCDB_ParseInterchange PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	unsigned char *tokens,	    /* XmString */
	struct DWC$db_interchange **work_context,
	int *err_line));

/*
** from dwc_db_day_management.c
*/
int DWC$DB_Set_workweek PROTOTYPE ((
        struct DWC$db_access_block  *Cab,
        int Work_mask));

/*
** from dwc_db_day_items.c
*/
int DWC$DB_Check_trigger PROTOTYPE ((
       struct DWC$db_access_block  *Cab,
        int Repeat_id,
        unsigned long Day));

int DWC$DB_Examine_r_params PROTOTYPE ((
        struct DWC$db_access_block  *Cab,
        int Item_id,
        int *P4,
        int *P5,
        int *P6));

int DWC$DB_Evaluate_r_params PROTOTYPE ((
        struct DWC$db_access_block  *Cab,
        int P1,
        int Day,
        int *P4,
        int *P5,
        int *P6));

/*
** from dwc_db_error_handling.
*/
int DWC$DB_Get_error_codes PROTOTYPE ((
        struct DWC$db_access_block *Cab,
        int *c_errno,
        int *vaxc_errno));

#endif /* _dwc_db_public_include_h_ */
/*
**  End of Module
*/
