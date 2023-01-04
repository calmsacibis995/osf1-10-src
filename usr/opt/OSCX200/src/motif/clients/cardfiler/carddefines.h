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
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
/* $Id: carddefines.h,v 1.1.4.2 1993/09/09 17:04:44 Susan_Ng Exp $ */
/*
**++

  Copyright (c) Digital Equipment Corporation,
  1987, 1988, 1989, 1990, 1991, 1992
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

**--
**/

#ifndef NO_ISL
#include <ddifdef.h>
#include <cdadef.h>
#include <cdamsg.h>
#include <cdaptp.h>
#endif

#define MAX_FILE_LEN	256

/* Text errors output	*/
#define BadLanguage	"Can't get language from toolkit, using US English \n"
#define BadLocale	"Can't open locale \n"
#define NoCardMain	"Can't fetch main widget \n"
#define NoCardMessage	"Can't fetch message widget \n"
#define NoCardWidget	"Can't fetch wigdet \n"
#define NoCardReference	"No reference widget \n"
#define NoCardFetch	"Can't fetch referenced widget \n"
#define NoCardHelp	"Can't fetch Help Widget \n"
#define NoCardError	"Can't fetch error widget \n"

#define APPL_NAME	"Cardfiler"
#ifdef VMS
#define CLASS_NAME	"DECW$CARDFILER"
#else
#define CLASS_NAME	"DXcardfiler"
#endif

#ifndef NO_HYPERHELP
#ifdef VMS
#define CARD_HELP	"DECW$CARDFILER"
#else
#define CARD_HELP       "DXcardfiler"
#endif
#endif

#define xrm_index_x		"Cardfiler.x"
#define xrc_index_x		"Cardfiler.X"
#define xrm_index_y		"Cardfiler.y"
#define xrc_index_y		"Cardfiler.Y"
#define xrm_index_width		"Cardfiler.width"
#define xrc_index_width		"Cardfiler.Width"
#define xrm_index_height	"Cardfiler.height"
#define xrc_index_height	"Cardfiler.Height"

#define xrm_card_x		"CardfilerCard.x"
#define xrc_card_x		"CardfilerCard.X"
#define xrm_card_y		"CardfilerCard.y"
#define xrc_card_y		"CardfilerCard.Y"
#define xrm_card_width		"CardfilerCard.width"
#define xrc_card_width		"CardfilerCard.Width"
#define xrm_card_height		"CardfilerCard.height"
#define xrc_card_height		"CardfilerCard.Height"

#define xrm_image_height		"CardfilerCard*card_drawing.height"
#define xrc_image_height		"CardfilerCard*card_drawing.Height"

#define xrm_language		"Cardfiler.language"
#define xrc_language		"Cardfiler.Language"

#define xrm_c_sort		"Cardfiler.cSort"
#define xrc_c_sort		"Cardfiler.CSort"

#define xrm_full_path_name	"Cardfiler.fullPathnames"
#define xrc_full_path_name	"Cardfiler.FullPathnames"

#define xrm_show_timings	"Cardfiler.showTimings"
#define xrc_show_timings	"Cardfiler.ShowTimings"

/* Cardfiler ISL Error codes */
#define BMP_TOOLARGE	4
#define NOT_VALID_TYPE	6
#define DDIF_NOGRAPHIC	8
#define NOT_DDIF_FILE	10

/*
** Several OS specific defines
** OK_STATUS - status to return for success
** ERROR_STATUS - status to return for failure
** READ_BINARY - access mode string for binary read access
** WRITE_BINARY - access mode string for binary write access
*/

#ifdef VMS
#define OK_STATUS	1
#define ERROR_STATUS	0
#define READ_BINARY	"rb"
#define WRITE_BINARY	"wb+"
#else
#define OK_STATUS	0
#define ERROR_STATUS	1
#define READ_BINARY	"r"
#define WRITE_BINARY	"w+"
#endif

#define INFORMATION	0
#define WARNING		1
#define ERROR		2
#define FATAL		3

#if defined (VAX) || defined (ALPHA) || defined (__alpha)
#define NATIVE_BYTE_ORDER LSBFirst
#define NATIVE_BIT_ORDER LSBFirst
#endif
/* following for MIPS (little endian) or MIPEL */
#ifdef MIPSEL
#define NATIVE_BYTE_ORDER LSBFirst
#define NATIVE_BIT_ORDER LSBFirst
#endif

/* Maximum lengths for elements of the card */
#define INDEX_LENGTH	40
#define FIND_LENGTH	256
#define TEXT_LENGTH	4000
#define BITMAP_SIZE	65536

/* Defines for editing functions to record for the UNDO */
#define CARDFILER_NO_UNDO		0
#define CARDFILER_CUT			1
#define CARDFILER_PASTE			2
#define CARDFILER_CUT_GRAPHIC		3
#define CARDFILER_PASTE_GRAPHIC		4
#define CARDFILER_TEXT_CHANGED		5
#define CARDFILER_GRAPHIC_CHANGED	6

#define k_indexworkarea		1
#define k_cardworkarea		2
#define k_buttonbox		3
#define k_bb_button1		4
#define k_bb_button2		5
#define k_valuewindow		6
#define k_svnlist		7
#define k_cardimagearea		8
#define k_bb_close		9
#define k_card_scroll_window	60

#define k_goto_text			10
#define k_card_goto_text		11
#define k_find_text			12
#define k_card_find_text		13
#define k_index_dialog_text		14
#define k_card_index_dialog_text	15
#define k_open_caution			16
#define k_exit_dialog			17

#define k_undo		20
#define k_cut 		21
#define k_copy		22
#define k_paste		23
#define k_select_graphic	24
#define k_deselect_graphic	25
#define k_readbitmapfile	26

#define k_print_all	31
#define k_print_all_now	32
#define k_print_one	33
#define k_print_one_now	34

#define k_printfile	41
#define k_printfileas	42
#define k_indexgoto	43
#define k_indexfind	44
#define k_indexfindnext	45
#define k_undelete	46
#define k_restore	47

#define k_cardgoto	50
#define k_cardfind	51
#define k_cardfindnext	52

/* file selection types */
#define CLEARSTORE	1
#define EXITSAVE	2
#define MERGE		3
#define READGRAPHIC	4
#define RETRIEVE	5
#define SAVE		6
#define ENTERFNAME	7


/* Text widget callback options */
#define TEXT_FOCUS		1
#define TEXT_VALUE_CHANGED	2

/* ISL macros	*/
#ifndef NO_ISL
#include <img/ImgDef.h>

#define start_get_itemlist(item_list, item_index)	\
   {							\
    item_index = 0;					\
    item_list[item_index].ItmL_Code = 0; 		\
    item_list[item_index].ItmL_Length = 0; 		\
    item_list[item_index].ItmA_Buffer = 0; 		\
    item_list[item_index].ItmA_Retlen = 0;		\
    item_list[item_index].ItmL_Index = 0; 		\
   }

#define put_get_item(item_list, item_index, item_code_value, component)	\
   {									\
    item_list[item_index].ItmL_Code = item_code_value; 			\
    item_list[item_index].ItmL_Length = sizeof(component); 		\
    item_list[item_index].ItmA_Buffer = (char *) &component; 		\
    item_list[item_index].ItmA_Retlen = 0;				\
    item_list[item_index].ItmL_Index = 0; 				\
    item_index++; 							\
   }

#define end_get_itemlist(item_list, item_index)		\
   {							\
    item_list[item_index].ItmL_Code = 0; 		\
    item_list[item_index].ItmL_Length = 0; 		\
    item_list[item_index].ItmA_Buffer = 0; 		\
    item_list[item_index].ItmA_Retlen = 0;		\
    item_list[item_index].ItmL_Index = 0; 		\
    item_index++;					\
   }

#define start_set_itemlist(item_list, item_index)	\
   {							\
    item_index = 0;					\
    item_list[item_index].PutL_Code = 0; 		\
    item_list[item_index].PutL_Length = 0; 		\
    item_list[item_index].PutA_Buffer = 0; 		\
    item_list[item_index].PutL_Index = 0; 		\
   }

#define put_set_item(item_list, item_index, item_code_value, component)	\
   {									\
    item_list[item_index].PutL_Code = item_code_value; 			\
    item_list[item_index].PutL_Length = sizeof(component); 		\
    item_list[item_index].PutA_Buffer = (char *) &component;		\
    item_list[item_index].PutL_Index = 0; 				\
    item_index++;							\
   }

#define end_set_itemlist(item_list, item_index)		\
   {							\
    item_list[item_index].PutL_Code = 0; 		\
    item_list[item_index].PutL_Length = 0; 		\
    item_list[item_index].PutA_Buffer = 0; 		\
    item_list[item_index].PutL_Index = 0; 		\
    item_index++;					\
   }

#if 0
/* Itemlist handling for setting ISL attributes. */
typedef struct sie {
    CDAconstant		item_code;
    CDAsize 		component_length;
    CDAaddress 		component_address;
    CDAindex 		index;
} set_itemlist_entry;

/* Itemlist handling for retrieving ISL attributes. */
typedef struct gie {
    CDAconstant		item_code;
    CDAsize 		component_length;
    CDAaddress 		component_address;
    CDAsize		return_length_address;
    CDAindex 		index;
} get_itemlist_entry;
#endif

/* vms string descriptor type */
typedef struct v_d_t {
    short string_length;
    unsigned char string_type;
    unsigned char string_class;
    unsigned char *string_address;
} vms_descriptor_type;

typedef struct li {
    CDAsize 	len;		/* component length */
    CDAconstant code;		/* the item code */
    CDAaddress	str;		/* address of string */
} item_desc;

typedef struct item_list_struct {
    CDAsize	item_length;
    CDAconstant	item_code;
    CDAaddress	item_address;
} ITEM_LIST_TYPE;

/* macro for setting up item list for
 * calls to cda$convert */
#define MAKE_ITEM(item, i, code, length, address)	\
{item[i].item_code    = code;				\
 item[i].item_length  = length;				\
 item[i].item_address = address;}
#endif

#ifdef VMS
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *                      VMSDSC (VMS descriptors)
 *                      ========================
 *
 *  These macros require the following header file:
 *
 *              descrip.h
 *
 * VMSDSC (typedef)
 * ------
 *
 * $INIT_VMSDSC (intialize a VMS static descriptor)
 * ------------
 *      dsc             : VMSDSC        : write : value
 *      len             : int           : read  : value
 *      addr            : char          : read  : ref
 *
 *    RETURNS: address of the descriptor (dsc)
 */

#define VMSDSC          struct dsc$descriptor

#define $INIT_VMSDSC(dsc, len, addr)            \
        ((dsc .dsc$b_class) = DSC$K_CLASS_S,    \
         (dsc .dsc$b_dtype) = DSC$K_DTYPE_T,    \
         (dsc .dsc$w_length) = (len),           \
         (dsc .dsc$a_pointer) = (addr),         \
         (& (dsc) )                             \
        )

#define INIT_IMAGING if (!imagingInited) InitImaging();
#define IMG$COPY		    (*ImagingCopy)
#define IMG$CREATE_DDIF_STREAM	    (*ImagingCreateDDIFStream)
#define IMG$CREATE_FRAME	    (*ImagingCreateFrame)
#define IMG$IMPORT_BITMAP	    (*ImagingImportBitmap)
#define IMG$EXPORT_DDIF_FRAME	    (*ImagingExportDDIFFrame)
#define IMG$DELETE_DDIF_STREAM	    (*ImagingDeleteDDIFStream)
#define IMG$IMPORT_DDIF_FRAME	    (*ImagingImportDDIFFrame)
#define IMG$GET_FRAME_ATTRIBUTES    (*ImagingGetFrameAttributes)
#define IMG$DECOMPRESS		    (*ImagingDecompress)
#define IMG$EXPORT_BITMAP	    (*ImagingExportBitmap)
#define IMG$DELETE_FRAME	    (*ImagingDeleteFrame)
#define IMG$OPEN_DDIF_FILE	    (*ImagingOpenDDIFFile)
#define IMG$CLOSE_DDIF_FILE	    (*ImagingCloseDDIFFile)

#else
#define INIT_IMAGING ;
#endif
