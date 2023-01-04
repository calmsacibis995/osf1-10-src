/*
 *  Title:	DT_wv_hdr.h
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1988, 1993                                                 |
 *  | By Digital Equipment Corporation, Maynard, Mass.                       |
 *  | All Rights Reserved.                                                   |
 *  |                                                                        |
 *  | This software is furnished under a license and may be used and  copied |
 *  | only  in  accordance  with  the  terms  of  such  license and with the |
 *  | inclusion of the above copyright notice.  This software or  any  other |
 *  | copies  thereof may not be provided or otherwise made available to any |
 *  | other person.  No title to and ownership of  the  software  is  hereby |
 *  | transfered.                                                            |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 *  Module Abstract:
 *
 *	Extensions to terminal definition for DECterm.
 *
 *  Author:	Mike Leibow
 *
 *  Modification history:
 *
 * Alfred von Campe     30-Sep-1993     BL-E
 *      - Add multi-page support.
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Alfred von Campe     21-May-1993     V1.2
 *      - Add definitions from (now obsolete) dt_source.h and change wrong
 *        usage of "short int" to REND.
 *
 * Alfred von Campe     25-Mar-1993     V1.2/BL2
 *      - Add F1-F5 key support.
 *      - Use symbols instead of numbers in array definitions.
 *      - Delete unnecessary structure definitions.
 *
 *  Aston Chan		12-Mar-1993		V1.2/BL2
 *	- Add Turkish/Greek support.  Also change value of ISO_LATIN_8 to 8 and
 *	  that of HEB_SUPPLEMENTAL to 9 to fix a problem in DT_PRINTER.C where
 *	  these will be used as array indices.
 *
 *  Aston Chan		17-Dec-1991		V3.1
 *	- I18n code merge
 *
 *  Bob Messenger	 9-Sep-1990		X3.0-7
 *	- Add pf1_m_graphics_to_host and pf1_m_prt_from_regis in print flags.
 *
 *  Bob Messenger	17-Jul-1990		X3.0-5
 *	Merge in Toshi Tanimoto's changes to support Asian terminals -
 *	- definitions for Asian character sets and terminal IDs
 *	- extended flag word, renditions and gsets (wvt$*_ext_*)
 *	- multi-byte character stack (wvt$b_char_stack*)
 *
 *  Bob Messenger	28-May-1989		X2.0-13
 *	- dimension wvt$b_tab_stops to MAX_COLUMN + 1
 *
 *  Bob Messenger	11-Apr-1989		X2.0-6
 *	- add fields and defs for color table reports
 *
 *  Bob Messenger	 4-Apr-1989		X2.0-5
 *	- fix up defs for color text (since we're actually supporting it)
 *
 *  Bob Messenger	 1-Apr-1989		X2.0-5
 *	- added fields and macros for OSC parsing
 *	- support variable transcript size
 *
 *  Bob Messenger	19-Mar-1989		X2.0-3
 *	- added definition for VT330_ID
 *	- added field for TARGETS atom
 *
 *  Bob Messenger	24-Jan-1989		X1.1-1
 *	- include locator key definitions in common area
 *
 *  Bob Messenger	16-Jan-1989		X1.1-1
 *	Move a lot of fields (wvt$l_alt_term_id in particular) into the
 *	common structure.  All the flags words are now common, except a
 *	new flags word, wvt$l_vt200_specific_flags, has been created to
 *	hold the auto wrap flag and other specific flags.
 *
 *  Bob Messenger	11-Jan-1989		X1.1-1
 *	Move cursts (cursor status) into common structure, because cursor blink
 *	is common to main and status displays
 *
 *  Eric Osman		2-Sep-1988		BL9.2
 *	Separate renditions and codes into independent arrays
 *
 */

#define VWS_DEVICE 0			/* VWS device type (UIS)	    */
#define DECTERM_DEVICE 1		/* DECterm device type		    */
#define DEVICE_TYPE DECTERM_DEVICE	/* this is DECterm		    */
#define LEVEL1 1                        /* VT100 conformance level          */
#define LEVEL2 2                        /* VT200                            */
#define LEVEL3 3                        /* VT300 (Panda)                    */
#define CONFORMANCE_LEVEL LEVEL3	/* we're a level 2 device	    */
#define MAX_LINE 255                     /* max display length (add extra for later)  */
#define MAX_COLUMN 255                  /* most you can fit                 */
#define STATUS_LINE (MAX_LINE+1)	/* the status line external to source */
#define DEFAULT_PLANE_COUNT 3           /* default number of colors         */
#define MAX_PLANE_COUNT 8               /* max number of colors             */
#define MAXPARMS 16                     /* Max sequence parameters          */
#define MAXINTERS 4                     /* Max intermediates                */
#define IGNORE_DCS 32                   /* Ignore the DCS sequence     " "  */
#define DECRSPS_CIR 1			/* Restore Presentation State  CIR  */
#define DECRSPS_TABSR 2			/* Restore Presentation State TABSR */
#define DECRSTS 3			/* Restore Terminal State           */
#define DECRSTS_CTR 4			/*    "        "     "    CTR       */
#define DECRQSS 36                      /* DECRQSS control string      "$"  */
#define IN_REGIS 112                    /* REGIS control string        "p"  */
#define SIXEL 113                       /* SIXEL inducer               "q"  */
#define DECAUPSS 117                    /* Assign User-Preference Set  "u"  */
#define DECCTPB 120                     /* Copy data to Paste Buffer   "x"  */
#define DRCS 123                        /* DRCS command                "{"  */
#define UDK 124                         /* UDK load                    "|"  */
#define DECATFF 125                     /* Assign Font Family          "}"  */
#define CSTR_LIMIT 8                    /*                                  */
#define UIS$M_KB_AUTORPT (1L)


/* The rendition (REND) and extended rendition (EXT_REND) types need to be
 * big enough to hold all the bits defined by the values below.
 */
typedef unsigned int REND;
typedef unsigned char EXT_REND;

#define SS_2 2                          /* Single Shift 2 (use G2)          */
#define SS_3 3                          /* Single Shift 3 (use G3)          */
#define SSS_3_TCS 10                    /* Super Single Shift 3 (use TCS)   */
#define DISP_SHIFT 98                   /* Display Controls shift           */
#define ERROR_SHIFT 99                  /* Next character is an ERROR       */
#define ASCII 0                         /* 7-bit ASCII set                  */
#define LINE_DRAWING 1                  /* VT100 line drawing characters    */
#define TECHNICAL 2                     /* DEC TCS                          */
#define APL 3                           /* APL set                          */
#define DRCS_FONT 4                     /* DRCS - (not supported)           */
#define USER_SET_1 5                    /* Reserved                         */
#define ISO_LATIN_1 6                   /* ISO 8-Bit latin 1                */
#define SUPPLEMENTAL 7                  /* DEC MCS                          */

/* 1 byte character sets (ONE_BYTE_SET) */
#define JIS_ROMAN 0                     /* JIS Roman (Japan)                */
#define JIS_KATAKANA 1                  /* JIS Katakana (Japan)             */
#define CRM_FONT_L 2			/* Control Representation Mode      */
#define CRM_FONT_R 3			/* Control Representation Mode      */
#define KS_ROMAN 4			/* KS Roman (Korean)		    */
#define ISO_LATIN_8 8			/* ISO 8-Bit latin 8 (Hebrew)	    */
#define HEB_SUPPLEMENTAL 9		/* DEC Hebrew Supplemental (Hebrew) */
/* 2 byte character sets (TWO_BYTE_SET) */
#define DEC_KANJI 0                     /* DEC Kanji (Japan)                */
#define DEC_HANZI 1                     /* DEC Hanzi (PRC)                  */
#define DEC_HANGUL 2                    /* DEC Hangul (Korea)               */
/* 4 byte character sets (FOUR_BYTE_SET) */
#define DEC_HANYU 0                     /* DEC Hanyu (Taiwan)               */
#define DEC_HANYU_4 1                   /* DEC Hanyu (Taiwan)               */

/* HEBREW_SUPPLEMENTAL is 8 and ISO_LATIN_8 is 9, so start
 * Greek and Turkish from 10
 */
#define TURKISH_SUPPLEMENTAL 10		/* Turkish Supplemental		    */
#define ISO_LATIN_5 11			/* ISO Turkish			    */
#define GREEK_SUPPLEMENTAL 12		/* Greek Supplemental		    */
#define ISO_LATIN_7 13			/* ISO Greek			    */

/* need to use 0xf and 0x8000 below because ISO_LATIN_7 is 13 (1101 binary) */
#define csa_M_CHAR_SET		0xf
#define csa_M_SELECTIVE_ERASE	0x8000	/* use NODEFAULT_TEXT_BCK, bug ?    */
#define csa_M_BOLD		0x10
#define csa_M_UNDERLINE		0x20
#define csa_M_BLINK		0x40
#define csa_M_REVERSE		0x80
#define csa_M_TEXT_COLOR	0x700
#define csa_M_TEXT_BCK		0x3800
#define csa_M_NODEFAULT_TEXT		0x4000
#define csa_M_NODEFAULT_TEXT_BCK	0x10000
#define csa_M_DOUBLE_BOTTOM 1
#define csa_M_DOUBLE_WIDTH 2
#define csa_M_DOUBLE_HIGH 4

#define CELL_STRIP 240                  /* Strip mask for cell rendition    */
#define LINE_ATTRIBUTE 6                /* Strip mask (2 bits)              */
#define SINGLE_WIDTH 0                  /* Single width (no attributes)     */
#define FONT_1 1                        /* Font 1 for ASCII-LINE_DRAWING-MCS  */
#define FONT_2 8                        /* Font 2 for TECHNICAL             */
#define FONT_3 32                       /* Font 3 for APL overstrike        */
#define FONT_4 64                       /* Font 4 for DRCS set              */

#define BLACK_TEXT	0               /* Black   ANSI text                */
#define RED_TEXT   	0x100           /* Red     ANSI text                */
#define GREEN_TEXT      0x200           /* Green   ANSI text                */
#define YELLOW_TEXT     0x300           /* Yellow  ANSI text                */
#define BLUE_TEXT       0x400           /* Blue    ANSI text                */
#define MAGENTA_TEXT    0x500           /* Magenta ANSI text                */
#define CYAN_TEXT       0x600           /* Cyan    ANSI text                */
#define WHITE_TEXT      0x700           /* White   ANSI text                */
#define MASK_TEXT       0x700           /* MASK for the text                */
#define MASK_TEXT_SHIFT	8               /* shift needed to normalize        */

#define BLACK_TEXT_BCK  0               /* Black   ANSI text background     */
#define RED_TEXT_BCK    0x800           /* Red     ANSI text background     */
#define GREEN_TEXT_BCK  0x1000          /* Green   ANSI text background     */
#define YELLOW_TEXT_BCK 0x1800          /* Yellow  ANSI text background     */
#define BLUE_TEXT_BCK   0x2000          /* Blue    ANSI text background     */
#define MAGENTA_TEXT_BCK 0x2800         /* Magenta ANSI text background     */
#define CYAN_TEXT_BCK   0x3000          /* Cyan    ANSI text background     */
#define WHITE_TEXT_BCK  0x3800          /* White   ANSI text background     */
#define MASK_TEXT_BCK   0x3800          /* MASK for the text background     */
#define MASK_TEXT_BCK_SHIFT 11          /* shift needed to normalize        */

/* extended rendition short word */
#define csa_M_EXT_CHAR_SET	0x3
#define csa_M_BYTE_POSITION	0xc
#define csa_M_BYTE_OF_CHAR	0xf
#define csa_M_IBM_KEISEN	0x70
#define csa_M_LEADING_CODE_MODE 0x80

#define STANDARD_SET	0x0		/* standard character set           */
#define ONE_BYTE_SET	0x1		/* 1 byte character set             */
#define TWO_BYTE_SET	0x2		/* 2 bytes character set            */
#define FOUR_BYTE_SET	0x3		/* 4 bytes character set            */
#define FIRST_BYTE	0x0		/* first byte   		    */
#define SECOND_BYTE	0x4		/* second byte 			    */
#define THIRD_BYTE	0x8             /* third byte  			    */
#define FOURTH_BYTE	0xc		/* fourth byte 			    */
#define FIRST_OF_TWO	0x2		/* first byte of 2 bytes character  */
#define LAST_OF_TWO	0x6		/* last byte of 2 bytes character   */
#define FIRST_OF_FOUR	0x3		/* 1st byte of CNS11643		    */
#define SECOND_OF_FOUR	0x7		/* 2nd byte of CNS11643		    */
#define THIRD_OF_FOUR	0xb		/* 3rd byte of DTSCS, no LC	    */
#define FOURTH_OF_FOUR	0xf		/* 4th byte of DTSCS, no LC	    */
#define FIRST_OF_FOUR_LC	0x83	/* 1st byte of DTSCS, LC ( 0xc2 )   */
#define SECOND_OF_FOUR_LC	0x87	/* 2nd byte of DTSCS, LC ( 0xcb )   */
#define THIRD_OF_FOUR_LC	0x8b	/* 3rd byte of DTSCS, LC	    */
#define FOURTH_OF_FOUR_LC	0x8f	/* 4th byte of DTSCS, LC	    */

#define MAX_UDK_VALUE 20                /* max number of udk definitions    */
#define UDK_AREA_SIZE 768               /* max storage available for udk's  */
#define MAX_LOC_VALUE 10                /* max number of locator definitions  */
#define LOC_AREA_SIZE 60                /* max storage available (1+4)*2*6  */
#define MAX_WORK_BUFFER_SIZE 300        /* work buffer size                 */
#define MAX_BANNER_SIZE 80              /* banner buffer size               */
#define MAX_ANSWERBACK 40               /* max answerback message size      */
#define MAX_NUMBER_OF_PAGES 6           /* max number of saved pages        */
#define VWS_ID 0                        /* Use VWS   ID in DA string        */
#define DECTERM_ID 0			/* Use DECterm ID		    */
#define VT100_ID 1                      /* Use VT100 ID                     */
#define VT101_ID 2                      /*     VT101 ID                     */
#define VT102_ID 3                      /*     VT102 ID                     */
#define VT125_ID 4                      /*     VT125 ID                     */
#define VT200_ID 5                      /*     VT200 ID                     */
#define VT300_ID 6                      /*     VT300 ID                     */
#define VT220_ID 7			/*     VT220 ID			    */
#define VT240_ID 8			/*     VT240 ID			    */
#define VT320_ID 9			/*     VT320 ID			    */
#define VT330_ID 12			/*     VT330 ID			    */
#define VT340_ID 11			/*     VT340 ID			    */
/* Asian terminal IDs */
#define VT80_ID 20			/*     VT80 ID (Japan)              */
#define VT100J_ID 22			/*     VT100J ID (Japan)            */
#define VT102J_ID 23			/*     VT102J ID (Japan)            */
#define VT220J_ID 24			/*     VT220J ID (Japan)            */
#define VT282_ID 25			/*     VT282 ID (Japan)             */
#define VT284_ID 26			/*     VT284 ID (Japan)             */
#define VT286_ID 27			/*     VT286 ID (Japan)             */
#define VT382_ID 28			/*     VT382 ID (Japan, Tomcat)     */
#define VT382CB_ID 29			/*     VT382CB ID (PRC, Bobcat)     */
#define VT382K_ID 30			/*     VT382K ID (Korea, Dickcat)   */
#define VT382D_ID 31			/*     VT382D ID (Taiwan, Fishcat)  */

/*                                                                          */
#define hlm_M_XPAD 16383
#define hlm_M_HIGH_BACK 16384
#define hlm_M_HIGH_FORE 32768

/* TSI - terminal state report buffer information */
struct tsr_buffer_struct {
#define TSR_BUF_SIZE 256
	unsigned char buffer[TSR_BUF_SIZE];
	unsigned char *cp;
	int bit_index, index;
	int end_bit_index, end_index;
};	

/* VT200 Common Flag bits for main & status display */
#define vtc1_m_actv_status_display (1 << 0)
#define vtc1_m_screen_mode (1 << 1)
#define vtc1_m_insert_mode (1 << 2)
#define vtc1_m_c1_transmission_mode (1 << 3)
#define vtc1_m_cursor_key_mode (1 << 4)
#define vtc1_m_keypad_mode (1 << 5)
#define vtc1_m_auto_answerback (1 << 6)
#define vtc1_m_udk_lock_control (1 << 7)
#define vtc1_m_enable_locator (1 << 8)
#define vtc1_m_echo_mode (1 << 9)
#define vtc1_m_lock_set (1 << 10)
#define vtc1_m_udk_erase_control (1 << 11)
#define vtc1_m_code_pair (1 << 12)
#define vtc1_m_loc_erase_control (1 << 13)
#define vtc1_m_loc_code_pair (1 << 14)
#define vtc1_m_kb_disabled (1 << 15)
#define vtc1_m_locator_report_mode (1 << 16)
#define vtc1_m_locator_one_shot (1 << 17)
#define vtc1_m_locator_down_reports (1 << 18)
#define vtc1_m_locator_up_reports (1 << 19)
#define vtc1_m_locator_cell_position (1 << 20)
#define vtc1_m_locator_filter (1 << 21)
#define vtc1_m_locator_filter_prime (1 << 22)
#define vtc1_m_tablet_connected (1 << 23)
#define vtc1_m_inhibit_UIS (1 << 24)
#define vtc1_m_kbd_action_mode (1 << 25)
#define vtc1_m_feature_lock (1 << 26)

/* VT200 Specific Flag bits for main or status display */
#define vts1_m_auto_wrap_mode (1 << 0)
#define vts1_m_origin_mode (1 << 1)
#define vts1_m_last_column (1 << 2)
#define vts1_m_regis_available (1 << 3)

/* VT200 Flag Bits                                                          */
#define vt1_m_enable_paste (1 << 0)
#define vt1_m_vt52_cursor_seq (1 << 1)
#define vt1_m_display_controls_mode (1 << 2)
#define vt1_m_ansi_mode (1 << 3)
#define vt1_m_nrc_mode (1 << 4)
#define vt1_m_scroll_mode (1 << 5)
#define vt1_m_cursor_blink_mode (1 << 6)
#define vt1_m_new_line_mode (1 << 7)
#define vt1_m_enable_term_mbx (1 << 8)
#define vt1_m_enable_osc_strings (1 << 9)
#define vt1_m_real_time_icon (1 << 10)
#define vt1_m_use_live_icon (1 << 11)

/* Second Flag Longword                                                     */
#define vt2_m_delete_disabled (1 << 0)
#define vt2_m_resize_disabled (1 << 1)
#define vt2_m_shrink_disabled (1 << 2)
#define vt2_m_addopt_disabled (1 << 3)
#define vt2_m_enable_cut (1 << 4)
#define vt2_m_ansi_color (1 << 5)
#define vt2_m_enable_ISO_latin (1 << 6)
#define vt2_m_menu_items_locked (1 << 7)
#define vt2_m_pass_forced_kb_data (1 << 8)
#define vt2_m_icon_moved (1 << 9)
#define vt2_m_noexpand_icon (1 << 10)
#define vt2_m_enable_icon_indication (1 << 11)
#define vt2_m_disable_icon_picture (1 << 12)
#define vt2_m_complement_icon (1 << 13)
#define vt2_m_backarrow_mode (1 << 14)
#define vt2_m_vss_scroll_mode (1 << 15)
#define vt2_m_use_fake_icon (1 << 16)
#define vt2_m_new_banner (1 << 17)
#define vt2_m_defer_nd_mode (1 << 18)
#define vt2_m_private_color_map (1 << 19)

/* Third Flag Longword                                                      */
#define vt3_m_ch_attr_extent_mode (1 << 0)
#define vt3_m_cursor_coupled_mode (1 << 1)
#define vt3_m_new_resize_logic (1 << 2)
#define vt3_m_defer_up_scroll (1 << 3)
#define vt3_m_resize_reports (1 << 4)
#define vt3_m_move_reports (1 << 5)
#define vt3_m_shrink_reports (1 << 6)
#define vt3_m_expand_reports (1 << 7)
#define vt3_m_kb_gain_reports (1 << 8)
#define vt3_m_kb_lose_reports (1 << 9)
#define vt3_m_no_control (1 << 10)

/* extended flag word 1 - terminal types, at most 1 bit will be on... */
#define vte1_m_tomcat (1 << 0)
#define vte1_m_bobcat (1 << 1)
#define vte1_m_dickcat (1 << 2)
#define vte1_m_fishcat (1 << 3)
#define vte1_m_hebrew (1 << 4)

/* for Greek and Turkish */
#define vte1_m_greek (1 << 5)
#define vte1_m_turkish (1 << 6)

/* convenient masks - note that they are not independent */
#define vte1_m_asian_common 0x0f	/* Asian terminal common features   */
#define vte1_m_ext_rends 0xff		/* Extended renditions enabled      */
#define vte1_m_two_byte 0x07		/* 2 bytes handling enabled         */
#define vte1_m_four_byte 0x08		/* 4 bytes handling enabled         */
#define vte1_m_chinese_common 0x0a	/* bobcat & fishcat common features */

/* extended flag word 2 - terminal specific flag */
#define vte2_m_sixel_scroll_mode	(1 << 0)	/* Asian 	*/
#define vte2_m_jisroman_mode		(1 << 1)	/* Kanji	*/
#define vte2_m_kanji_mode		(1 << 2)	/* Kanji	*/
#define vte2_m_kanji_78			(1 << 3)	/* Kanji	*/
#define vte2_m_leading_code_mode	(1 << 4)	/* Hanyu	*/
#define vte2_m_intermediate_char	(1 << 5)	/* Hangul	*/
#define vte2_m_ksroman_mode		(1 << 6)	/* Hangul	*/
#define vte2_m_multi_mode		(1 << 7)	/* Multi	*/
#define vte2_m_rtl			(1 << 8)	/* Hebrew	*/
#define vte2_m_kb_map			(1 << 9)	/* Hebrew	*/
#define vte2_m_copy_dir			(1 << 10)	/* Hebrew	*/
#define vte2_m_kb_map_pend_rqst		(1 << 11)	/* Hebrew	*/
#define vte2_m_kb_map_pend_toheb	(1 << 12)	/* Hebrew	*/
#define vte2_m_not_first_reset		(1 << 13)	/* Hebrew	*/
#define vte2_m_kb_soft_switch		(1 << 14)	/* Hebrew	*/
#define vte2_m_kb_was_heb		(1 << 15)	/* Hebrew	*/

/* PRINT flags                                                              */
#define pf1_m_prt_enabled (1 << 0)
#define pf1_m_prt_ff_mode (1 << 1)
#define pf1_m_prt_extent_mode (1 << 2)
#define pf1_m_prt_transmission_mode (1 << 3)
#define pf1_m_prt_controller_mode (1 << 4)
#define pf1_m_auto_print_mode (1 << 5)
#define pf1_m_printer_started (1 << 6)
#define pf1_m_graphics_to_host (1 << 7)
#define pf1_m_prt_data_to_host (1 << 8)
#define pf1_m_prt_display_mode (1 << 9)
#define pf1_m_prt_esc (1<<10)		/* on if possible esc exit prtcon */

#define U_START 0                       /* UDK states (also used for locator)  */
#define U_CODE 1                        /*                                  */
#define U_KEY_DEF 2                     /*                                  */
#define U_IGNORE 3                      /*                                  */
#define R_PARSE_ERROR 0
#define R_CONTINUE 1
#define R_GRAPHIC 2
#define R_CONTROL 3
#define R_ESC_SEQ 4
#define R_CSI_SEQ 5
#define R_DCS_SEQ 6                     /* Parse events                     */
#define ESEQ 0                          /* escape sequence                  */
#define CSI 1                           /* control sequence                 */
#define DCS 2                           /* control sequence                 */
#define LAST_SEQ 2                      /* "in sequence" marker             */
#define CON 3                           /* control and graphic characters   */
#define PARSE_ERROR 0                   /*                                  */
#define CONTINUE 16                     /*                                  */
#define GRAPHIC 32                      /*                                  */
#define CONTROL 48                      /*                                  */
#define ESC_SEQ 64                      /*                                  */
#define PAR_SEQ 80                      /*                                  */
#define CANCEL 208                      /*                                  */
#define PARAM 224                       /*                                  */
#define P_IGNORE 240                    /*                                  */
#define SEQ_START 0
#define CON_START 1                     /* common parse states              */
#define ES_IN_ESC 2                     /* escape sequence parse states     */
/*  Escape sequence parse table                                             */
/*  ---------------------------                                             */
/*                                                                          */
/*  from state:      input:           action:         to state:             */
/*                                                                          */
/*  CON_START(1)     ES_CONTROL(0) => CONTROL(30),    CON_START(1)          */
/*                   ES_INTER(3)   => GRAPHIC(20),    CON_START(1)          */
/*                   ES_FINAL(6)   => GRAPHIC(20),    CON_START(1)          */
/*                   ES_GRAPHIC(9) => GRAPHIC(20),    CON_START(1)          */
/*                                                                          */
/*  SEQ_START(0)     ES_CONTROL(0) => CONTROL(30),    ES_IN_SEQ(2)          */
/*                   ES_INTER(3)   => CONTINUE(10),   ES_IN_SEQ(2)          */
/*                   ES_FINAL(6)   => ESC_SEQ(40),    ES_IN_SEQ(2)          */
/*                   ES_GRAPHIC(9) => PARSE_ERROR(00),ES_IN_SEQ(2)          */
/*                                                                          */
/*  ES_IN_SEQ(2)     ES_CONTROL(0) => CONTROL(30),    ES_IN_SEQ(2)          */
/*                   ES_INTER(3)   => CONTINUE(10),   ES_IN_SEQ(2)          */
/*                   ES_FINAL(6)   => ESC_SEQ(40),    ES_IN_SEQ(2)          */
/*                   ES_GRAPHIC(9) => PARSE_ERROR(00),ES_IN_SEQ(2)          */
/*                                                                          */
/*          eseq_parse_table[input+from_state]  ==>  action,to_state        */
#define ES_CONTROL 0                    /*                                  */
#define ES_INTER 3                      /*                                  */
#define ES_FINAL 6                      /*                                  */
#define ES_GRAPHIC 9                    /*                                  */
#define CS_PRIVATE 1
#define CS_PARAM 2
#define CS_INTER 3
#define CS_IGNORE 4
#define CS_OMIT 5                       /* control sequence parse states    */
/*  control sequence parse table         (all values in hex)                */
/*  ----------------------------                                            */
/*                                                                          */
/*  from state:      input:            action:         to state:            */
/*                                                                          */
/*  SEQ_START(0)     PI_CONTROL(0)  => CONTROL(30),    CS_IGNORE(4)         */
/*                   PI_PRIVATE(6)  => CONTINUE(10),   CS_PRIVATE(1)        */
/*                   PI_NUMERAL(12) => PARAM(E0),      CS_PARAM(2)          */
/*                   PI_SEMICOL(18) => PARAM(E0),      CS_OMIT(5)           */
/*                   PI_INTER(24)   => CONTINUE(10),   CS_INTER(3)          */
/*                   PI_FINAL(30)   => PAR_SEQ(50),    CS_IGNORE(4)         */
/*                   PI_OTHER(36)   => CONTINUE(10),   CS_IGNORE(4)         */
/*                                                                          */
/*  CS_PRIVATE(1)    PI_CONTROL(0)  => CONTROL(30),    CS_IGNORE(4)         */
/*                   PI_PRIVATE(6)  => CONTINUE(10),   CS_IGNORE(4)         */
/*                   PI_NUMERAL(12) => PARAM(E0),      CS_PARAM(2)          */
/*                   PI_SEMICOL(18) => PARAM(E0),      CS_OMIT(5)           */
/*                   PI_INTER(24)   => CONTINUE(10),   CS_INTER(3)          */
/*                   PI_FINAL(30)   => PAR_SEQ(50),    CS_IGNORE(4)         */
/*                   PI_OTHER(36)   => CONTINUE(10),   CS_IGNORE(4)         */
/*                                                                          */
/*  CS_PARAM(2)      PI_CONTROL(0)  => CONTROL(30),    CS_IGNORE(4)         */
/*                   PI_PRIVATE(6)  => CONTINUE(10),   CS_IGNORE(4)         */
/*                   PI_NUMERAL(12) => PARAM(E0),      CS_PARAM(2)          */
/*                   PI_SEMICOL(18) => PARAM(E0),      CS_OMIT(5)           */
/*                   PI_INTER(24)   => CONTINUE(10),   CS_INTER(3)          */
/*                   PI_FINAL(30)   => PAR_SEQ(50),    CS_IGNORE(4)         */
/*                   PI_OTHER(36)   => CONTINUE(10),   CS_IGNORE(4)         */
/*                                                                          */
/*  CS_INTER(3)      PI_CONTROL(0)  => CONTROL(30),    CS_IGNORE(4)         */
/*                   PI_PRIVATE(6)  => CONTINUE(10),   CS_IGNORE(4)         */
/*                   PI_NUMERAL(12) => CONTINUE(10),   CS_IGNORE(4)         */
/*                   PI_SEMICOL(18) => CONTINUE(10),   CS_IGNORE(4)         */
/*                   PI_INTER(24)   => CONTINUE(10),   CS_INTER(3)          */
/*                   PI_FINAL(30)   => PAR_SEQ(50),    CS_IGNORE(4)         */
/*                   PI_OTHER(36)   => CONTINUE(10),   CS_IGNORE(4)         */
/*                                                                          */
/*  CS_IGNORE(4)     PI_CONTROL(0)  => CONTROL(30),    CS_IGNORE(4)         */
/*                   PI_PRIVATE(6)  => IGNORE(F0),     CS_IGNORE(4)         */
/*                   PI_NUMERAL(12) => IGNORE(F0),     CS_IGNORE(4)         */
/*                   PI_SEMICOL(18) => IGNORE(F0),     CS_IGNORE(4)         */
/*                   PI_INTER(24)   => IGNORE(F0),     CS_IGNORE(4)         */
/*                   PI_FINAL(30)   => CANCEL(D0),     CS_IGNORE(4)         */
/*                   PI_OTHER(36)   => IGNORE(F0),     CS_IGNORE(4)         */
/*                                                                          */
/*  CS_OMIT(5)       PI_CONTROL(0)  => CONTROL(30),    CS_IGNORE(4)         */
/*                   PI_PRIVATE(6)  => CONTINUE(10),   CS_IGNORE(4)         */
/*                   PI_NUMERAL(12) => PARAM(E0),      CS_PARAM(2)          */
/*                   PI_SEMICOL(18) => PARAM(E0),      CS_OMIT(5)           */
/*                   PI_INTER(24)   => CONTINUE(10),   CS_INTER(3)          */
/*                   PI_FINAL(30)   => PAR_SEQ(50),    CS_IGNORE(4)         */
/*                   PI_OTHER(36)   => CONTINUE(10),   CS_IGNORE(4)         */
/*                                                                          */
/*          pseq_parse_table[input+from_state]  ==>  action,to_state                                                                
*/
#define PI_CONTROL 0                    /*                                  */
#define PI_PRIVATE 6                    /*                                  */
#define PI_NUMERAL 12                   /*                                  */
#define PI_SEMICOL 18                   /*                                  */
#define PI_INTER 24                     /*                                  */
#define PI_FINAL 30                     /*                                  */
#define PI_OTHER 36                     /*                                  */
/*	Define C0 control character codes                                   */
#define C0_NUL 0                        /* null                             */
#define C0_SOH 1                        /* start of header                  */
#define C0_STX 2                        /* start of text                    */
#define C0_ETX 3                        /* end of text                      */
#define C0_EOT 4                        /* end of transmission              */
#define C0_ENQ 5                        /* enquire                          */
#define C0_ACK 6                        /* acknowledge                      */
#define C0_BEL 7                        /* bell                             */
#define C0_BS 8                         /* backspace                        */
#define C0_HT 9                         /* horizontal tab                   */
#define C0_LF 10                        /* line feed                        */
#define C0_VT 11                        /* vertical tab                     */
#define C0_FF 12                        /* form feed                        */
#define C0_CR 13                        /* carrage return                   */
#define C0_SO 14                        /* shift out                        */
#define C0_SI 15                        /* shift in                         */
#define C0_DLE 16                       /* data link escape                 */
#define C0_DC1 17                       /* dev control 1 (xon)              */
#define C0_DC2 18                       /* dev control 2                    */
#define C0_DC3 19                       /* dev control 3 (xoff)             */
#define C0_DC4 20                       /* dev control 4                    */
#define C0_NAK 21                       /* negative acknowledge             */
#define C0_SYN 22                       /* synch                            */
#define C0_ETB 23                       /* end transmission block           */
#define C0_CAN 24                       /* cancel                           */
#define C0_EM 25                        /*                                  */
#define C0_SUB 26                       /* sub                              */
#define C0_ESC 27                       /* escape                           */
#define C0_FS 28                        /* field seperator                  */
#define C0_GS 29                        /*                                  */
#define C0_RS 30                        /* record seperator                 */
#define C0_US 31                        /* unit seperator                   */
/*	Define C1 control character codes                                   */
#define C1_IND 132                      /* index                            */
#define C1_NEL 133                      /* new line                         */
#define C1_SSA 134                      /*                                  */
#define C1_ESA 135                      /*                                  */
#define C1_HTS 136                      /* set horizontal tab               */
#define C1_HTJ 137                      /*                                  */
#define C1_VTS 138                      /*                                  */
#define C1_PLD 139                      /*                                  */
#define C1_PLU 140                      /*                                  */
#define C1_RI 141                       /* reverse index                    */
#define C1_SS2 142                      /* single shift 2                   */
#define C1_SS3 143                      /* single shift 3                   */
#define C1_DC 144                       /* device control string            */
#define C1_PU1 145                      /*                                  */
#define C1_PU2 146                      /*                                  */
#define C1_STS 147                      /*                                  */
#define C1_CCH 148                      /*                                  */
#define C1_MW 149                       /*                                  */
#define C1_SPA 150                      /*                                  */
#define C1_EPA 151                      /*                                  */
#define C1_SOS 152                      /* Start of string		    */
#define C1_CSI 155                      /* control string inducer           */
#define C1_ST 156                       /* string terminator                */
#define C1_OSC 157                      /* operating system control string  */
#define C1_PM 158                       /* privacy message                  */
#define C1_APC 159                      /* application control string       */
/*                                                                          */
/*	Structure PARSE_STACK                                               */
/*                                                                          */
#define STACK_SIZE 64
struct parse_stack {
    char index;
    char class [64];
    char value [64];
    unsigned short int data [64];
    } ;
#define PARSE_STACK$DISPLAY_STRUCTURE_S 257
#define vt4_m_bell 1
#define vt4_m_smooth_scroll 2
#define vt4_m_underline_cursor 4
#define vt4_m_conforming_wid 8
#define vt4_m_margin_bell 16
#define vt4_m_new_data 32
#define vt4_m_defer_new_data 64
#define vt4_m_skip_new_data 128
#define vt4_m_regis_debug_override 256
#define vt4_m_rubber_box_visible 512
#define vt4_m_cut_active 1024
#define vt4_m_ibar_cursor 2048
#define vt4_m_ibar_in_insert 4096
#define vt4_m_edit_select 8192
#define vt4_m_select_trim 16384

#define cs1_m_cs_run 1
#define cs1_m_cs_off 2
#define cs1_m_cs_inactive 4
#define cs1_m_cs_dsblack 8
#define cs1_m_cs_enabled 16
#define cs1_m_cs_noblink 32

#define es1_m_es_blink 1
#define es1_m_es_blrev 2

typedef struct {
    unsigned char *a_code_base; /* characters                      */
    REND	  *a_rend_base; /* renditions             	   */
    EXT_REND	  *a_ext_rend_base; /* renditions             	   */
    unsigned short int w_widths; /* line widths                    */
    unsigned char b_rendits;   /* line renditions                  */
    unsigned short int highlight_begin; /* highlight begin column  */
    unsigned short int highlight_end;   /* highlight end column    */
} s_line_structure;


/* state for parsing OSC sequences */

#define OSC_STATE_INIT			0
#define OSC_STATE_PARSING_TYPE		1
#define OSC_STATE_NEED_SEMICOLON	2
#define OSC_STATE_PARSING_STRING 	3
#define OSC_STATE_BAD_COMMAND		4

#define OSC_MAX_STRING	64

/* state for parsing DECRSTS for color table reports */

#define CTR_STATE_INIT			0
#define CTR_STATE_COLOR			1
#define CTR_STATE_COORDINATE		2
#define CTR_STATE_CX			3
#define CTR_STATE_CY			4
#define CTR_STATE_CZ			5
#define CTR_STATE_NUMBER		6
#define CTR_STATE_NUMBER_1		7
#define CTR_STATE_IGNORE		8

#define CTR_PARSE_NUMBER( state )		\
    {						\
    _cld wvt$b_ctr_return_state = state;	\
    _cld wvt$b_ctr_state = CTR_STATE_NUMBER;	\
    }

#define TRIM_RGB( value ) ( ( ( ( (int) (value) ) + 1 ) * 100 ) / 65536 )

/*                                                                          */
/*	Structure WVT$  This contains all of the protocol emulator state    */
/*                                                                          */
typedef struct { /* SpecificSourceData */
/*									    */
/* These fields are used to make sure the output has the correct width and  */
/* height; they contain the last width and height the source has told the   */
/* output about.							    */
/*									    */
    int /* display width, (moved to common below), */ display_height;
/*                                                                          */
/* The transcript comes right before the display list for each of the four  */
/* arrays (character codes, character renditions, line widths and line      */
/* renditions).  We can reference the transcript via negative indices       */
/* (line numbers).                                                          */
/*                                                                          */
    int wvt$l_transcript_size;
    int wvt$l_transcript_top;

    unsigned char **wvt$a_trans_code_base;
    unsigned char **wvt$a_code_base;	/* characters                       */
    REND **wvt$a_trans_rend_base;
    EXT_REND **wvt$a_trans_ext_rend_base;
    REND **wvt$a_rend_base;		/* renditions                       */
    EXT_REND **wvt$a_ext_rend_base;	/* renditions                       */
    unsigned short int *wvt$w_trans_widths;
    unsigned short int *wvt$w_widths;	/* line widths                      */
    unsigned char *wvt$b_trans_rendits;
    unsigned char *wvt$b_rendits;	/* line renditions                  */
    unsigned char *wvt$a_cur_cod_ptr;   /* fast char pointer                */
    REND *wvt$a_cur_rnd_ptr;            /* fast rend pointer                */
    EXT_REND *wvt$a_cur_ext_rnd_ptr;	/* fast rend pointer                */
    /* long int wvt$l_actv_width  -- moved to common area below             */
    long int wvt$l_actv_column;         /* current column                   */
    long int wvt$l_actv_line;           /* current line                     */
    REND wvt$w_actv_rendition;          /* current rendition                */
    EXT_REND wvt$w_actv_ext_rendition;	/* current rendition                */
    unsigned char wvt$b_disp_eol;       /* display to EOL                   */
    long int wvt$l_disp_pos;            /* starting display pos             */
    long int wvt$l_top_margin;          /* top margin                       */
    long int wvt$l_bottom_margin;       /* bottom margin                    */
    long int wvt$l_left_margin;         /* left margin (Panda)              */
    long int wvt$l_right_margin;        /* right margin (Panda)             */
    long int wvt$l_vt200_specific_flags;  /* flags specific to main/status  */
    unsigned char wvt$b_single_shift;   /* single shift                     */
    unsigned char wvt$b_gl;             /* current GL set                   */
    unsigned char wvt$b_gr;             /* current GR set                   */
    unsigned char wvt$b_g_sets [4];     /* gset mapping                     */
    unsigned char wvt$b_ext_g_sets [4]; /* gset mapping                     */
    unsigned char wvt$b_ups;            /* which G-sets is UPSS ?	    */
    unsigned char wvt$b_nrc_set;        /* NRC set in nat mode              */
    long int wvt$l_save_vt200_flags;    /* saved for cursor s/r             */
    long int wvt$l_save_line;           /* save block for                   */
    long int wvt$l_save_column;         /* cursor save/restore              */
    long int wvt$l_defer_count;         /* Up scroll defer                  */
    long int wvt$l_defer_max;           /* Up scroll defer max              */
/*    float wvt$f_defer_max_factor;        Up scroll defer max              */
    long int wvt$l_defer_limit;		/* Up scroll defer (from resource)  */
    REND wvt$w_save_rendition;          /*                                  */
    EXT_REND wvt$w_save_ext_rendition;	/*                                  */
    unsigned short int wvt$w_save_pad;  /*                                  */
    unsigned char wvt$b_save_user_preference_set; /*                        */
    unsigned char wvt$b_sav_gl;         /*                                  */
    unsigned char wvt$b_sav_gr;         /*                                  */
    unsigned char wvt$b_sav_pad;        /*                                  */
    unsigned char wvt$b_save_g_sets [4]; /*                                 */
    unsigned char wvt$b_save_ext_g_sets [4]; /*                             */
    unsigned char wvt$b_save_ups;       /* which G-sets is UPSS ?	    */
    unsigned char wvt$b_save_g_sets_crm [2] ; /* save gset in crm	    */
    unsigned char wvt$b_save_ups_crm ;  /* save UPS flag in crm		    */
    unsigned char wvt$b_sav_gl_crm;
    unsigned char wvt$b_sav_gr_crm;
    unsigned char wvt$b_save_ext_g_sets_crm [2]; /* save ext gset in crm    */
    long int wvt$l_page_length;         /* logical page length              */
    long int wvt$l_column_width;        /* logical terminal width           */
/*                                                                          */
/* Pointers to the base of the character cell and rendition arrays          */
/*                                                                          */
    unsigned char *wvt$a_code_cells;    /* base pool pointer                */
    REND *wvt$a_rend_cells;   		/* pointer to renditions buffer     */
    EXT_REND *wvt$a_ext_rend_cells;   	/* pointer to renditions buffer     */
    unsigned long int wvt$l_attributes; /* term window attributes           */
} SpecificSourceData;

/*                                                                          */
/* common display struct contains the common fields of the main and status  */
/* displays.                                                                */
/*                                                                          */
typedef struct { /* SourceData -- common to source and status display*/
    unsigned char wvt$b_conforming_resize; /* resize flag                   */
    long int wvt$l_vt200_common_flags;  /* flag word                        */
    long int wvt$l_vt200_flags;         /* flag word                        */
    long int wvt$l_vt200_flags_1;       /* flag word                        */
    long int wvt$l_vt200_flags_2;       /* flag word                        */
    long int wvt$l_vt200_flags_3;       /* flag word                        */
    unsigned long int wvt$l_flags;      /* flag word                        */
    long int wvt$l_ext_flags;		/* flag word                        */
    long int wvt$l_ext_specific_flags;	/* flag word                        */
    unsigned char wvt$b_cursts;         /* cursor status                    */
    struct parse_stack wvt$r_com_state;
/*                                                                          */
/*	Start of Sub-Structure COM_STATE -- output parser stack             */
/*                                                                          */
    unsigned char wvt$b_privparm;       /* private param "?"                */
    unsigned char wvt$b_parmcnt;        /* parameter count                  */
    unsigned long int wvt$l_parms [MAXPARMS]; /* parameters                 */
    unsigned char wvt$b_intercnt;       /* intermediate count               */
    unsigned char wvt$b_inters [MAXINTERS]; /* intermediates                */
    unsigned char wvt$b_finalchar;      /* final character                  */
    unsigned char wvt$b_dscs [8];       /* dscs string                      */
    unsigned char wvt$b_dscs_intercnt [2]; /* dcsc inter count              */
    unsigned char wvt$dcs_final[8];	/* dcs final chars                  */
    unsigned int  wvt$dcs_final_index;
    unsigned int  wvt$dcs_count;
    struct tsr_buffer_struct wvt$dcs_tsr_buff;/* terminal state report block*/
    unsigned char wvt$b_osc_type;       /* OSC state block                  */
    unsigned char wvt$b_osc_state;         
    unsigned char wvt$b_osc_string[OSC_MAX_STRING+1];
    int wvt$l_osc_length;
    unsigned char wvt$b_in_dcs;         /* DCS processing flag              */
    unsigned char wvt$b_can_sub_detected;
    long int wvt$l_actv_width;          /* width of active line             */
    int display_width;                  /* see description above            */
    unsigned char wvt$b_user_preference_set; /* ISO latin-1 or MCS          */
    unsigned char wvt$b_tab_stops[MAX_COLUMN+1]; /* stop array              */
    unsigned char wvt$b_conformance_level; /* 1, 2 or 3                     */
    unsigned char wvt$b_last_event;     /* saved last parse event           */
    unsigned char wvt$b_national_kb;    /* KB language                      */
    unsigned char wvt$b_alt_term_id;    /* alternate terminal ID            */
    unsigned char wvt$b_graphics_cursor_enabled; /* ReGIS flags needed      */
    unsigned char wvt$b_regis_mode;     /* in the VT300 code                */
    unsigned char wvt$b_regis_graphics;
    unsigned short int wvt$w_print_flags; /* print flags                    */
    unsigned short int wvt$w_save_print_flags; /*                           */
    unsigned char n_pr_exit_chars;	/* number of prtcon exit chars seen */
    unsigned char wvt$b_cursor_type;    /* Cursor type                      */
    long int wvt$l_sixel_lines;
/*                                                                          */
/* work variables                                                           */
/*                                                                          */
    struct
        {
        long int offset;
        unsigned char *address;
        }         wvt$l_work_string;     /* Work buffer                     */
    unsigned char wvt$b_work_buffer [300];
    short int wvt$w_udk_state;          /* UDK state block                  */
    short int wvt$w_udk_code;
    short int wvt$w_udk_size;
    short int wvt$w_udk_space_used;
    short int wvt$w_code_value;
    short int wvt$w_udk_pad;
    short int wvt$w_udk_data [MAX_UDK_VALUE];
    short int wvt$w_udk_length [MAX_UDK_VALUE];
    unsigned char wvt$b_udk_area [UDK_AREA_SIZE];
    unsigned long int wvt$l_kb_attributes; /* KB attributes                 */
    unsigned long int wvt$l_charadr;    /* KB data input buffer             */
    unsigned long int wvt$l_button_data; /* button data input buffer        */
    DECwFunctionKeyMode wvt$f1_key_mode;
    DECwFunctionKeyMode wvt$f2_key_mode;
    DECwFunctionKeyMode wvt$f3_key_mode;
    DECwFunctionKeyMode wvt$f4_key_mode;
    DECwFunctionKeyMode wvt$f5_key_mode;

    long int wvt$l_locator_x;           /* locator (cell coordinates)       */
    long int wvt$l_locator_y;
    long int wvt$l_locator_x_dc;        /* locator (pixel coordinates)      */
    long int wvt$l_locator_y_dc;
    unsigned char wvt$b_loc_cursor_type; /* LKD block                       */

    long int wvt$l_filter_ly;           /* filter rectangle block           */
    long int wvt$l_filter_uy;
    long int wvt$l_filter_lx;
    long int wvt$l_filter_ux;
    short int wvt$w_loc_length [12];
    unsigned char wvt$b_loc_area [72];
    short int wvt$w_loc_state;
    short int wvt$w_loc_code;
    short int wvt$w_loc_half;
    short int wvt$w_loc_code_value;

    unsigned char wvt$b_ctr_state;	/* parsing state for DECCTR */
    unsigned char wvt$b_ctr_return_state;
					/* state to return to once
					   number has been parsed */
    unsigned short wvt$w_ctr_color;	/* color index */
    unsigned short wvt$w_ctr_coordinate;/* color coordinate system, 1 or 2 */
    unsigned short wvt$w_ctr_cx;	/* hue or red */
    unsigned short wvt$w_ctr_cy;	/* lightness or green */
    unsigned short wvt$w_ctr_cz;	/* saturation or green */
    unsigned short wvt$w_ctr_number;	/* number being parsed */

    unsigned char wvt$b_answerback [MAX_ANSWERBACK+1]; /* answerback */

    unsigned char wvt$b_char_stack [4];	/* character stack */
    unsigned char wvt$b_char_stack_top; /* top of the character stack */
    unsigned char wvt$b_char_set;
    unsigned char wvt$b_ext_char_set;
/*
 * source data not part of wv_mumble...
 */
    s_line_structure	line_structure[1];
    struct {
	    Boolean 		has_selection;
	    DECtermPosition	selection_begin;
	    DECtermPosition	selection_end;
    }select[2];
    unsigned int	stop_output;
    char		*put_data;
    int			put_count;
    Boolean		work_proc_registered;
    XtWorkProcId	work_proc_id;
    Atom		targets_atom;

    SpecificSourceData *SpecificSourceData;
    SpecificSourceData MainSourceData, StatusSourceData;

/* Structure to store off-screen pages */
    struct {
        Boolean allocated;
        int page_length;
        int column_width;
        unsigned char *code_cells;
        REND *rend_cells;
        EXT_REND *ext_rend_cells;
        unsigned short int *widths;
        unsigned char *rendits;
    } page[MAX_NUMBER_OF_PAGES];
    int current_page;

} SourceData;

#define Source_has_selection(w,t)	((w)->source.select[t].has_selection)
#define Source_select_begin(w, t)	((w)->source.select[t].selection_begin)
#define Source_select_end(w, t)		((w)->source.select[t].selection_end)
