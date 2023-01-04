#ifndef _dwc_db_private_include_h_
#define _dwc_db_private_include_h_
/* #module dwc_db_private_include.h */
/* $Id$ */
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
**	Private include file for database routines. This file contains
**	references to internal structures, macros, etc.
**
**--
*/

/*
**  Include Files
*/
#include "dwc_db_private_structures.h"
#include <errno.h>

#if defined (__osf__)
struct DWC$db_time_ctx;	    /* Forward declaration */
#endif
/*
**  Structure for queue head in doubly linked lists
*/
struct DWC$db_queue_head
{
    struct DWC$db_queue_head *flink;
    struct DWC$db_queue_head *blink;
};

typedef struct DWC$db_vm_record	    VM_record;
typedef struct DWC$db_access_block  Cab_type;
#if 0
/* moved below to alpha specific stuff (SP) */
typedef unsigned long int Rec_ptr;
#endif
/*
**  Define database creator system.  DWC$t_database_creator can't be longer
**  than 39 characters (plus terminator).
*/

#if defined(VMS) && defined(VAX)
#define DWC$k_db_creator DWC$k_db_vax_vms
#define DWC$t_database_creator "Database creator O/S: VAX/VMS."

#elif (defined(ultrix) || defined(__ultrix))
#if (defined(mips) || defined(__mips))
#define DWC$k_db_creator DWC$k_db_mips_ultrix
#define DWC$t_database_creator "Database creator O/S: Risc ULTRIX."
#else
#define DWC$k_db_creator DWC$k_db_vax_ultrix
#define DWC$t_database_creator "Database creator O/S: VAX ULTRIX."
#endif

#elif defined(mips)
#define DWC$k_db_creator DWC$k_db_mips_osf1
#define DWC$t_database_creator "Database creator O/S: Risc OSF/1."

#elif defined(sparc)
#define DWC$k_db_creator DWC$k_db_sun
#define DWC$t_database_creator "Database creator O/S: Sparc SunOS."

#elif defined(VMS) && !defined(VAX)
#define DWC$k_db_creator DWC$k_db_alpha_vms
#define DWC$t_database_creator "Database creator O/S: ARA VMS."

#else
#define DWC$k_db_creator DWC$k_db_unknown
#define DWC$t_database_creator "Database creator O/S: UNKNOWN."

#endif

/*
**  Redefine entry points to memory allocation and deallocation
**  routines if this is being compiled for debug
*/
#ifdef DWC$MEMORY_DEBUG
#define malloc jck_malloc
#define free jck_free
#endif

/*
**  ASCII database identification string. Please note that this string must
**  fit in a field of 50 bytes (including the terminating NULL).
*/
#define DWC$t_database_version "DECwindows Calendar database; V3.0, rev #11."

/*
**  This constant defines the max number of records that can be allocated in
**  one single go.
*/
#define DWC$k_db_max_allocation_chunk 32

/*
**  Max number of additional days that code will inspect when trying to
**  satisfy conditional repeats.
*/
#define DWC$k_db_max_rcond_check 14

/*
**  Define the VMS specific file attributes for the calendar
*/
#ifdef VMS
#define DWC$t_db_file_attributes "dna = SYS$LOGIN:.DWC"
#define DWC$t_db_file_attributes_read "dna = SYS$LOGIN:.DWC", "shr = get"
#else
#define DWC$t_db_file_attributes 0
#define DWC$t_db_file_attributes_read 0
#endif

/*
**  Mode in which file is opened
*/
#define DWC$t_db_file_mode 0600 /* Owner: READ+WRITE */

/*
**  Macro Definitions
*/
#define DWC$ERROR(Errorcode)	unsigned long Errorcode

#define _Set_base(Base)	DWC$$DB_Set_error_base(Cab, Base)
#define _Set_cause(Cause)	DWC$$DB_Set_error_cause(Cab, Cause)
#define _Pop_cause	DWC$$DB_Pop_error_cause(Cab)
#define _Signal(Err)	DWC$$DB_Signal_error(Cab, Err)

#define _Rzero(Ptr) {int i; char *p = &(void)Ptr;\
			for (i=0; i<=sizeof(Ptr)-1; i++)\
			    *p++ = '\0';\
		    }
#define _Mbz(field)	(DWC$$DB_Mbz_validate(Cab, (char *) &field, sizeof(field)))
#define _Mbz_check(field) if (_Mbz(field) == -1) return (-1)
#define DWC_vm_record_len (DWC$K_VM_HEADER + DWC$k_db_record_length)

#if (defined(ultrix)||defined(__ultrix)) && !(defined(mips)||defined(__mips))
/*
** We have to do this for VAX ULTRIX because the "portable" approach doesn't
** work with cc (and vcc doesn't work at all).
*/
#define DWC$m_vm_flag (0x80000000)
#define DWC$m_va_flag (0x80000000)
#else
#define DWC$m_vm_flag (UL_ONE << (CHAR_BIT * sizeof(CARD32) - 1))
#define DWC$m_va_flag (UL_ONE << (CHAR_BIT * sizeof(CARD32) - 1))
#endif

#if (DWC_LONG_BIT == 64)
typedef CARD32 Rec_ptr;
#define DWC_ALPHA_PTR_MASK 0
#define _Fixup(Ptr) DWC$$DB_Fixup_pointer(&((CARD32)Ptr))
#define Make_VM_address(Pointer) (Pointer ? ((VM_record *) (((Pointer) &~ DWC$m_vm_flag) | DWC_ALPHA_PTR_MASK)) : (VM_record *) 0)
#define Make_VM_pointer(Address) (Address ? ((CARD32) ((CARD32)(Address) | DWC$m_vm_flag) &~ DWC_ALPHA_PTR_MASK) : (CARD32) 0)
#define DWC$$DB_Make_real_addr(Addr) ((CARD32) (Addr & ~DWC$m_va_flag) | DWC_ALPHA_PTR_MASK)
#define DWC$$DB_Make_va_addr(Addr) ((CARD32) (Addr | DWC$m_va_flag) &~ DWC_ALPHA_PTR_MASK)
#define _Ptr(foo) ponte_ptr(foo)
#define _UnPtr(foo) ponte_unptr(foo)

static void *ponte_ptr (address)
CARD32	address; 
{
    unsigned long tmp;

    /*
    ** Upgrade the size of storage.
    */
    tmp = address;

    /*
    ** Put in the bit.
    */
    if (tmp != 0)
	tmp = tmp | DWC_ALPHA_PTR_MASK;

    /*
    ** Return the modified value.
    */
    return ((void *)tmp);
}

static CARD32 ponte_unptr (ptr)
void *ptr;
{
    unsigned long	tmp0;
    CARD32		tmp;

    /*
    ** Take out the bit.
    */
    tmp0 = (unsigned long) ptr;
    if (tmp0)
	tmp0 = tmp0 &~ DWC_ALPHA_PTR_MASK;

    /*
    ** Shrink the storage.
    */
    tmp = (CARD32) tmp0;
	
    /*
    ** Return the unpointered value.
    */
    return tmp;
}
#else
typedef unsigned long int Rec_ptr;
#define _Fixup(Ptr) DWC$$DB_Fixup_pointer(&(Ptr))
#define _Ptr(foo) ((void*)foo)
#define _UnPtr(foo) ((CARD32)foo)
#define Make_VM_address(Pointer) ((VM_record *) ((Pointer)&~DWC$m_vm_flag))
#define Make_VM_pointer(Address) ((CARD32) (Address) | DWC$m_vm_flag)
#define DWC$$DB_Make_real_addr(Addr) (CARD32) (Addr & ~DWC$m_va_flag)
#define DWC$$DB_Make_va_addr(Addr) (CARD32) (Addr | DWC$m_va_flag)
#endif
#define Is_FILE_pointer(Pointer) !((CARD32) (Pointer) & DWC$m_vm_flag)
#define Is_VM_pointer(Pointer)   ((CARD32) (Pointer) & DWC$m_vm_flag)
#define DWC$$DB_Is_va_addr(Addr) ((CARD32) Addr & DWC$m_va_flag)
#define DWC$$DB_Is_not_va_addr(Addr) (((CARD32) Addr & DWC$m_va_flag) == 0)
#define _Field_offset(base,field) ((char *)&(base->field) - (char *)base)
#define _Dbgprintf(Msg) printf("Calendar [debug] -- %s\n",Msg)
#define _Follow(Field, Tar) DWC$$DB_Follow_pointer(Cab, Field, Tar)
#define _Follow_NC(Field, Tar) DWC$$DB_Follow_pointer_NC(Cab, Field, Tar)
#define _Bind(Vmp, Rectyp) (struct Rectyp *)&(Vmp->DWC$t_dbvm_data[0])
/*
**  O/S Conditional definitions
*/
#ifdef VMS
#define _Record_error	    { \
			    Cab->DWC$l_dcab_errno = errno; \
			    Cab->DWC$l_dcab_vaxc = vaxc$errno; \
			    }
#else
#define _Record_error	    Cab->DWC$l_dcab_errno = errno
#endif

#if defined(DWC$NOFSYNC)
int DWC$$DB_fsync PROTOTYPE((struct DWC$db_access_block *Cab));
#define DWC_FSYNC(Cab) DWC$$DB_fsync(Cab)
#else
#define DWC_FSYNC(Cab) \
{ \
    if ((fsync(fileno(Cab->DWC$l_dcab_fd))) == -1) \
    { \
	_Record_error; \
	_Signal(DWC$_FILEIOERR); \
    } \
}
#endif

#define _Write_back(Cab) \
{ \
    int	flush_val; \
    flush_val = fflush(Cab->DWC$l_dcab_fd); \
    if (flush_val != 0) \
    { \
	_Record_error; \
	_Signal(DWC$_FILEIOERR); \
    } \
    DWC_FSYNC(Cab); \
}

/*
**  Table of Contents. Please note that none of the routine have parameters
**  in this declaration (pcc restriction). Check associated module source
**  for parameters.
*/
void DWC$$DB_Set_error_base PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	unsigned long			Errcode));

void DWC$$DB_Set_error_cause PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	unsigned long			Errcode));

void DWC$$DB_Pop_error_cause PROTOTYPE ((
	struct DWC$db_access_block	*Cab));

void DWC$$DB_Signal_error PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	unsigned long			Errcode));

int DWC$$DB_Mbz_validate PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	char				*buf,
	int				len));

void DWC$$DB_Delete_all_buffers PROTOTYPE ((
	struct DWC$db_access_block	*Cab));

void DWC$$DB_Cache_buffer PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	struct DWC$db_vm_record		*Buffer));

void DWC$$DB_Touch_buffer PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	struct DWC$db_vm_record		*Buffer));

void DWC$$DB_Freelist_buffer PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	struct DWC$db_vm_record		*Buffer));

void DWC$$DB_Insque PROTOTYPE ((
	struct DWC$db_queue_head	*Entry,
	struct DWC$db_queue_head	*Pred));
int DWC$$DB_Remque PROTOTYPE ((
	struct DWC$db_queue_head	*Head,
	struct DWC$db_queue_head	**Entry));
	
int DWC$$DB_Get_free_buffer PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				Reqlen,
	struct DWC$db_vm_record		**Retbuf));
	
int DWC$$DB_Get_free_buffer_z PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				Reqlen,
	struct DWC$db_vm_record		**Retbuf));

int DWC$$DB_Purge_cache PROTOTYPE ((
	struct DWC$db_access_block	*Cab));

int DWC$$DB_Write_virtual_record PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	VM_record			*Input_record_vm));

VM_record *DWC$$DB_Follow_pointer PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	void				*Pointer,
	int				Expected_type));

VM_record *DWC$$DB_Follow_pointer_NC PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	void				*Ptr,
	int				Typ));

void DWC$$DB_Fixup_pointer PROTOTYPE ((
	void				*Pointer));

int DWC$$DB_Locate_day PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	VM_record			**Returned_vm,
	int				Day,
	int				Alloc));

int DWC$$DB_Flush_day PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	VM_record			*Day_VM,
	int				Day_num));

int DWC$$DB_Is_subrangemap_empty PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	VM_record			*Submap_vm));

int DWC$$DB_Is_daymap_empty PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	VM_record			*Daymap_vm));

int DWC$$DB_Truncate_day PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	VM_record			*Daymap_vm,
	int				Day_num));

void DWC$$DB_Release_day PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	VM_record			*Daymap_vm));

int DWC$$DB_Fry_stub_day PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	VM_record			*Daymap_vm));

int DWC$$DB_Fry_temp_arrays PROTOTYPE ((struct DWC$db_access_block *Cab));

int DWC$$DB_Cleanup_exit PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	unsigned long			Retsts));

int DWC$$DB_Release_alarms PROTOTYPE ((struct DWC$db_access_block *Cab));

void DWC$$DB_Rundown_errors PROTOTYPE ((struct DWC$db_access_block *Cab));

/*
** dwc_db_day_management.c
*/
int DWC$$DB_Get_daytype PROTOTYPE ((struct DWC$db_access_block *Cab, int Day));

int DWC$$DB_Set_centerpoint PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				Day));

/*
** dwc_db_open_close.c
*/
int DWC$$DB_Adjust_daymap PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				Dayptr));


/*
** dwc_db_physical_records.c
*/
VM_record *DWC$$DB_Read_physical_record PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				position,
	int				Count,
	char				Type));

int DWC$$DB_Write_physical_record PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				position,
	int				Count,
	char *				Buffer));

int DWC$$DB_Validate_record_ptr PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				Rec_addr,
	int				Count));

int DWC$$DB_Alloc_records PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				Count));

int DWC$$DB_Deallocate_records PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				Count,
	int				Record_address));
int DWC$$DB_Extend_file PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				Count));

int DWC$$DB_Find_clear_bits PROTOTYPE ((
	char				*Bitmap,
	int				Bitmap_size,
	int				Target,
	int				*Free_bit));

int DWC$$DB_Set_bits PROTOTYPE ((
	char				*Bitmap,
	int				Start_bit,
	int				Count));

int DWC$$DB_Clear_bits PROTOTYPE ((
	char				*Bitmap,
	int				Start_bit,
	int				Count));

int DWC$$DB_Toggle_bits PROTOTYPE ((
	char				*Bitmap,
	int				Start_bit,
	int				Count));

void DWC$$DB_Byte_swap_buffer PROTOTYPE ((dwcaddr_t Buffer, int preswap));
void DWC$$DB_Byte_swap_field PROTOTYPE ((dwcaddr_t Buffer, int size));

/*
** dwc_db_day_mapping.c
*/
int DWC$$DB_Read_day_extension PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	VM_record			**Returned_vm,
#ifndef _dwc_db_private_structures_h_
#define _dwc_db_private_structures_h_
/* $Header$ */
/* DWC_DB_PRIVATE_STRUCTURES.H "V3.0-002" */
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
**	Per Hamnqvist, September 1988
**
**  ABSTRACT:
**	This module includes the other main structure modules used to
**	build the datastructures that are internal to the DECwindows
**	Calendar database.
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
** V3.0-002 Paul Ferwerda					21-Nov-1990
**		Got rid of the sdl file. Maintainers have to edit this directly
**		now. This was done to make is easier to build Ultrix versions
**		while nfsed to a cms reference copy.
**
**	V1-000	Per Hamnqvist					07-Sep-1988
**		Module created.
**--
*/

/*
**  Include Files
*/
#include "dwc_db_record_structures.h"
#include "dwc_db_work_structures.h"

#endif /* end of _dwc_db_private_structures_h_ */
	struct DWCDB_day		*Day_data,
	char				**Returned_data,
	int				*Max_data));

int DWC$$DB_Flush_maps PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	VM_record			*Start_map_vm,
	int				Force_sub));

/*
** dwc_db_virtual_buffers.c
*/
int DWC$$DB_Release_buffer PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	VM_record			*Input_buffer_VM));

/*
** dwc_db_repeat_expressions.c
*/
int DWC$$DB_Load_repeats PROTOTYPE ((
	struct DWC$db_access_block	*Cab));

int DWC$$DB_Expand_interval PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	struct DWCDB_repeat_expr	*Exp,
	struct DWC$db_repeat_control	*Repc));

int DWC$$DB_Compress_params PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				P1,
	int				P2,
	int				P3,
	int				Day));

int DWC$$DB_Unload_repeats PROTOTYPE ((
	struct DWC$db_access_block	*Cab));

int DWC$$DB_Locate_repeat_buf PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	struct DWC$db_repeat_control	**Repc));

int DWC$$DB_Flush_repeat_buf PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	struct DWC$db_repeat_control	*Repc));

int DWC$$DB_Add_repeat PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				*Return_id,
	int				Base_day,
	int				Base_min,
	int				End_day,
	int				End_min,
	int				Duration_min,
	int				Alarm_vec_len,
	unsigned short int		Alarm_vec[],
	int				Flags,
	char				Input_text[],
	int				Input_text_len,
	int				Input_text_class,
	int				P1,
	int				P2,
	int				P3));

int DWCDB__AddRepeat PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				*Return_id,
	int				Base_day,
	int				Base_min,
	int				End_day,
	int				End_min,
	int				Duration_min,
	int				Alarm_vec_len,
	unsigned short int		Alarm_vec[],
	int				Flags,
	unsigned char			*Input_text, /* XmString */
	int				Input_text_class,
	unsigned int			Input_icons_num,
	unsigned char			Input_icon_ptr[],
	int				P1,
	int				P2,
	int				P3));

int DWC$$DB_Modify_repeat PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				Repeat_id,
	int				Curr_day,
	int				Base_min,
	int				End_day,
	int				End_min,
	int				Duration_min,
	int				Alarm_vec_len,
	unsigned short int		Alarm_vec[],
	int				Flags,
	char				*Input_text,
	int				Input_text_len,
	int				Input_text_class,
	int				P1,
	int				P2,
	int				P3));

int DWCDB__ModifyRepeat PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				Repeat_id,
	int				Curr_day,
	int				Base_min,
	int				End_day,
	int				End_min,
	int				Duration_min,
	int				Alarm_vec_len,
	unsigned short int		Alarm_vec[],
	int				Flags,
	unsigned char			*Input_text, /* XmString */
	int				Input_text_class,
	unsigned int			Input_icons_num,
	unsigned char			Input_icon_ptr[],
	int				P1,
	int				P2,
	int				P3));

int DWC$$DB_Delete_repeat PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				Repeat_id));

int DWC$$DB_Find_repeat PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				Repeat_id,
	struct DWC$db_repeat_control	**Repc));

int DWC$$DB_Page_in_repeat PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	struct DWC$db_repeat_control	*Repc));

int DWC$$DB_Next_repeat_after PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	struct DWCDB_repeat_expr	*Rep_exp,
	unsigned int			After_day,
	unsigned int			After_min,
	unsigned int		    	*Next_day,
	unsigned int			*Next_min));

int DWC$$DB_Next_repeat PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	struct DWCDB_repeat_expr	*Rep_exp,
	unsigned int			Base_day,
	unsigned int			Base_min,
	unsigned int			*Next_day,
	unsigned int			*Next_min));

int DWC$$DB_Free_expr PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	struct DWC$db_repeat_control	*Repc));

int DWC$$DB_Merge_repeat PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	VM_record			*Day_vm,
	struct DWC$db_repeat_control	*Repc,
	unsigned int			Day,
	unsigned int			Minute));

int DWC$$DB_Insert_repeats PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	VM_record			*Day_vm,
	int				Day));

int DWC$$DB_Insert_one_repeat PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	VM_record			*Day_vm,
	int				Day,
	struct DWC$db_repeat_control	*Repc,
	struct DWC$db_time_ctx		*Timec));

int DWC$$DB_Determine_instances PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	VM_record			*Day_VM,
	int				Day));

int DWC$$DB_Back_out_repeats PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	VM_record			*Rep_blo_vm));

int DWC$$DB_Check_day PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				Day,
	int				Options));

int DWC$$DB_Load_exceptions PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	struct DWCDB_repeat_expr	*Rep_exp));

int DWC$$DB_Release_exceptions PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	struct DWCDB_repeat_expr	*Rep_exp));

int DWC$$DB_Add_exception PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	struct DWC$db_repeat_control	*Repc,
	unsigned int			Except_day));

int DWC$$DB_Add_exception_day PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				Rep_id,
	unsigned int			Except_day));

int DWC$$DB_Purge_exceptions PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	struct DWC$db_repeat_control	*Repc,
	unsigned int			Before_day));

int DWC$$DB_Check_exception PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	struct DWCDB_repeat_expr	*Rep_exp,
	unsigned int			Check_day));

int DWC$$DB_Remove_specific_expr PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				Rep_id,
	VM_record			*Day_vm));

int DWC$$DB_Triggers_on PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	struct DWC$db_repeat_control	*Repc,
	struct DWC$db_time_ctx		*Timec));

int DWC$$DB_Check_single_expr PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int				Repeat_id,
	int				Day));

int DWC$$DB_First_day PROTOTYPE ((
	struct DWC$db_access_block	*Cab));

int DWC$$DB_Last_day PROTOTYPE ((
	struct DWC$db_access_block	*Cab));


/*
**
*/
void DWC$$DB_Compute_alarm_time PROTOTYPE ((
	struct DWCDB_entry *Entry,
	unsigned int Org_day,
	unsigned short int Alarm_offset,
	unsigned int (*Trigg_day),
	unsigned short int (*Trigg_min)));

int DWC$$DB_Compare_times PROTOTYPE ((
	unsigned int Day_1,
	unsigned short int Min_1,
	unsigned int Day_2,
	unsigned short int Min_2));

int DWC$$DB_Insert_alarm_item PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	VM_record   *Item_VM,
	int	    Org_day));

int DWC$$DB_Remove_alarm_item PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	int Day,
	int Id));

int DWC$$DB_Release_alarms PROTOTYPE ((
	struct DWC$db_access_block	*Cab));

int DWC$$DB_Unload_alarms PROTOTYPE ((
	struct DWC$db_access_block	*Cab));

void DWC$$DB_Adjust_vm_alarms PROTOTYPE ((
	struct DWC$db_access_block	*Cab,
	unsigned int Request_day,
	unsigned short int Request_min));


/*
** Functions in dwc_db_day_mapping.c
*/
int DWC$$DB_Locate_rangemap PROTOTYPE ((
	    struct DWC$db_access_block *Cab,
	    VM_record **Returned_vm,
	    int Day,
	    int Alloc));

int DWC$$DB_Add_rangemap PROTOTYPE ((
	    struct DWC$db_access_block *Cab,
	    VM_record **Returned_vm,
	    int Day));

int DWC$$DB_Locate_submap PROTOTYPE ((
	    struct DWC$db_access_block *Cab,
	    int Day,
	    VM_record **Returned_vm,
	    int Alloc));

int DWC$$DB_Locate_day PROTOTYPE ((
	    struct DWC$db_access_block *Cab,
	    VM_record **Returned_vm,
	    int Day,
	    int Alloc));

int DWC$$DB_Load_day PROTOTYPE ((
	    struct DWC$db_access_block *Cab,
	    VM_record *Day_VM));

int DWC$$DB_Flush_day PROTOTYPE ((
	    struct DWC$db_access_block *Cab,
	    VM_record *Day_VM,
	    int Day_num));

int DWC$$DB_Size_day PROTOTYPE ((
	    struct DWC$db_access_block	*Cab,
	    VM_record *Day_VM,
	    VM_record **Returned_vm,
	    char **Returned_data,
	    int *Returned_max));

int DWC$$DB_Read_day_extension PROTOTYPE ((
	    struct DWC$db_access_block	*Cab,
	    VM_record **Returned_vm,
	    struct DWCDB_day *Day_data,
	    char **Returned_data,
	    int *Max_data));

void DWC$$DB_Release_maps PROTOTYPE ((
	    struct DWC$db_access_block	*Cab,
	    VM_record		    *Start_map_vm,
	    int			    Remove_count));
	
int DWC$$DB_Allocate_maps PROTOTYPE ((
	    struct DWC$db_access_block *Cab,
	    VM_record *Start_map_vm,
	    Rec_ptr *Starting_record,
	    int *Records_allocated));

void DWC$$DB_Clear_day_context PROTOTYPE ((
	    struct DWC$db_access_block	*Cab));

int DWC$$DB_Fry_stub_day PROTOTYPE ((
	    struct DWC$db_access_block	*Cab,
	    VM_record *Daymap_vm));

int DWC$$DB_Fry_temp_arrays PROTOTYPE ((struct DWC$db_access_block *Cab));

int DWCDB__FryTempArrays PROTOTYPE ((struct DWC$db_access_block *Cab));

void DWC$$DB_Release_day PROTOTYPE ((
	    struct DWC$db_access_block *Cab,
	    VM_record *Daymap_vm));

int DWC$$DB_Is_subrangemap_empty PROTOTYPE ((
	    struct DWC$db_access_block	*Cab,
	    VM_record *Submap_vm));

int DWC$$DB_Is_daymap_empty PROTOTYPE ((
	    struct DWC$db_access_block	*Cab,
	    VM_record *Daymap_vm));

int DWC$$DB_Dump_rmaps PROTOTYPE ((
	    struct DWC$db_access_block	*Cab));
/*
** dwc_db_trading.c
*/
int DWC$$DB_Rundown_wi PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	int status));

int DWC$$DB_Find_interchange PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context));

int DWC$$DB_Treat_token PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context));

int DWC$$DB_Treat_text PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	struct DWC$db_itoken *work_item));

int DWC$$DB_Get_inum PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	unsigned int *outnum));

int DWC$$DB_Append_int_item PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context,
	struct DWCDB_entry *Entry,
	int day));

int DWCDB__FindInterchange PROTOTYPE ((
	struct DWC$db_access_block *Cab,
	struct DWC$db_interchange *work_context));

/*
**  Parts of the VMS specific file, stsdef.h, has been included directly into
**  this module since we're likely not going to have access to it on all target
**  systems.
*/

#define STS$K_ERROR      2
#define STS$K_INFO       3
#define STS$K_SEVERE     4
#define STS$K_SUCCESS    1
#define STS$K_WARNING    0

#define STS$M_MSG_NO     0x0000FFF8
#define STS$M_SEVERITY   0x00000007
#define STS$M_COND_ID    0x0FFFFFF8

#define STS$V_FAC_NO     0x10
#define STS$V_MSG_NO     0x03
#define STS$V_SEVERITY   0x00
#define STS$V_COND_ID    0x03

#define _VMS_STATUS_COND_ID(code) 	( ( (code) & STS$M_COND_ID ) 	>> STS$V_COND_ID )
#define _VMS_STATUS_SEVERITY(code) 	( ( (code) & STS$M_SEVERITY ) 	>> STS$V_SEVERITY )
#define _VMS_ECODE(code, level) ((code << STS$V_MSG_NO) | (DWC_FACILITY << STS$V_FAC_NO) | (level))


/*
**  Declare all the error codes (used to be in a message file). The
**  actual text is defined in the module DWC_DB_ERROR_HANDLING.C
**
**	    PLEASE ADD NEW CODES AT THE END, USING THE NEXT FREE
**	    MESSAGE NUMBER!!!!
*/
#define DWC_FACILITY 2049

#define DWC$_NORMAL (_VMS_ECODE(1, STS$K_SUCCESS))
#define DWC$_TESTOK (_VMS_ECODE(2, STS$K_SUCCESS))
#define DWC$_INFO (_VMS_ECODE(3, STS$K_INFO))
#define DWC$_WARNING (_VMS_ECODE(4, STS$K_WARNING))
#define DWC$_NOFIXUP (_VMS_ECODE(5, STS$K_WARNING))
#define DWC$_NOWRITE (_VMS_ECODE(6, STS$K_WARNING))
#define DWC$_RECERR (_VMS_ECODE(7, STS$K_WARNING))
#define DWC$_CRECAL (_VMS_ECODE(8, STS$K_ERROR))
#define DWC$_ALLOCREC (_VMS_ECODE(9, STS$K_ERROR))
#define DWC$_CREFAIL (_VMS_ECODE(10, STS$K_ERROR))
#define DWC$_BADCLOSE (_VMS_ECODE(11, STS$K_ERROR))
#define DWC$_PUTLBUF (_VMS_ECODE(12, STS$K_ERROR))
#define DWC$_WRITEREC (_VMS_ECODE(13, STS$K_ERROR))
#define DWC$_READREC (_VMS_ECODE(14, STS$K_ERROR))
#define DWC$_INVPTR (_VMS_ECODE(15, STS$K_ERROR))
#define DWC$_INSVIRMEM (_VMS_ECODE(16, STS$K_ERROR))
#define DWC$_FILEIOERR (_VMS_ECODE(17, STS$K_ERROR))
#define DWC$_INVRTYPE (_VMS_ECODE(18, STS$K_ERROR))
#define DWC$_UNKDAYIT (_VMS_ECODE(19, STS$K_ERROR))
#define DWC$_DLENMM (_VMS_ECODE(20, STS$K_ERROR))
#define DWC$_UNXDIE (_VMS_ECODE(21, STS$K_ERROR))
#define DWC$_BADDCHK (_VMS_ECODE(22, STS$K_ERROR))
#define DWC$_EXTENDFAIL (_VMS_ECODE(23, STS$K_ERROR))
#define DWC$_CREF (_VMS_ECODE(24, STS$K_ERROR))
#define DWC$_OPENF (_VMS_ECODE(25, STS$K_ERROR))
#define DWC$_NOCURCAL (_VMS_ECODE(26, STS$K_ERROR))
#define DWC$_REPLOAD (_VMS_ECODE(27, STS$K_ERROR))
#define DWC$_UNLREPS (_VMS_ECODE(28, STS$K_ERROR))
#define DWC$_NOSUCHREP (_VMS_ECODE(29, STS$K_ERROR))
#define DWC$_BADRCHK (_VMS_ECODE(30, STS$K_ERROR))
#define DWC$_TESTFAIL (_VMS_ECODE(31, STS$K_ERROR))
#define DWC$_OUTOFPHASE (_VMS_ECODE(32, STS$K_SEVERE))
#define DWC$_OVERFLOW (_VMS_ECODE(33, STS$K_SEVERE))
#define DWC$_UNDERFLOW (_VMS_ECODE(34, STS$K_SEVERE))
#define DWC$_WRITEERR (_VMS_ECODE(35, STS$K_SEVERE))
#define DWC$_CLOSEFAIL (_VMS_ECODE(36, STS$K_SEVERE))
#define DWC$_MBZFAIL (_VMS_ECODE(37, STS$K_SEVERE))
#define DWC$_FREEFAIL (_VMS_ECODE(38, STS$K_SEVERE))
#define DWC$_OPENFAIL (_VMS_ECODE(39, STS$K_SEVERE))
#define DWC$_READONLY (_VMS_ECODE(40, STS$K_SEVERE))
#define DWC$_PURGEFAIL (_VMS_ECODE(41, STS$K_SEVERE))
#define DWC$_MODPROF (_VMS_ECODE(42, STS$K_SEVERE))
#define DWC$_PROFOVER (_VMS_ECODE(43, STS$K_SEVERE))
#define DWC$_GETPROF (_VMS_ECODE(44, STS$K_SEVERE))
#define DWC$_GETIFAIL (_VMS_ECODE(45, STS$K_SEVERE))
#define DWC$_PUTIFAIL (_VMS_ECODE(46, STS$K_SEVERE))
#define DWC$_DELIFAIL (_VMS_ECODE(47, STS$K_SEVERE))
#define DWC$_DAYATAF (_VMS_ECODE(48, STS$K_SEVERE))
#define DWC$_DAYATGF (_VMS_ECODE(49, STS$K_SEVERE))
#define DWC$_ALARMINCON (_VMS_ECODE(50, STS$K_SEVERE))
#define DWC$_OCFAIL (_VMS_ECODE(51, STS$K_SEVERE))
#define DWC$_ADDRF (_VMS_ECODE(52, STS$K_SEVERE))
#define DWC$_MODREPF (_VMS_ECODE(53, STS$K_SEVERE))
#define DWC$_DELREPF (_VMS_ECODE(54, STS$K_SEVERE))
#define DWC$_MODIFAIL (_VMS_ECODE(55, STS$K_SEVERE))
#define DWC$_RENFAIL (_VMS_ECODE(56, STS$K_SEVERE))
#define DWC$_FLUSHF (_VMS_ECODE(57, STS$K_SEVERE))
#define DWC$_GETNAMEF (_VMS_ECODE(58, STS$K_SEVERE))
#define DWC$_UNLFAIL (_VMS_ECODE(59, STS$K_SEVERE))
#define DWC$_RENFAT (_VMS_ECODE(60, STS$K_SEVERE))
#define DWC$_GETNAME (_VMS_ECODE(61, STS$K_SEVERE))
#define DWC$_PURGE (_VMS_ECODE(62, STS$K_SEVERE))
#define DWC$_UNEXPREP (_VMS_ECODE(63, STS$K_SEVERE))
#define DWC$_DTCBAD (_VMS_ECODE(64, STS$K_SEVERE))
#define DWC$_EVALREP (_VMS_ECODE(65, STS$K_SEVERE))
#define DWC$_LOADFIL (_VMS_ECODE(66, STS$K_SEVERE))
#define DWC$_RUNDF (_VMS_ECODE(67, STS$K_SEVERE))
#define DWC$_ICREF (_VMS_ECODE(68, STS$K_SEVERE))
#define DWC$_CSTR (_VMS_ECODE(69, STS$K_SEVERE))
#define DWC$_UNSTR (_VMS_ECODE(70, STS$K_SEVERE))
#define DWC$_CNCHKR (_VMS_ECODE(71, STS$K_SEVERE))
#define DWC$_PUTIF (_VMS_ECODE(72, STS$K_SEVERE))
#define DWC$_FGNALT (_VMS_ECODE(73, STS$K_SEVERE))


#endif /* end of _dwc_db_private_include_h_ */
