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
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**	GEUI.H         	  	Widget include file
**
**  ABSTRACT:
**
**	Main include file for Graphics Editor Interface
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	DAA 03/22/89 Created
**
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  C A U T I O N !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!
! The order of the constants in this file is VITAL to the functioning 
! of the Graphics Editor.
!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!  C A U T I O N !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
**/

/* Application shell and class names
 */
#define GE_APPLICATION_NAME	"Graphics Editor"
#define GE_APPLICATION_CLASS	"gra"

/* Titles and Popups
 */
#define GEGRAMAIN_WIN		"gramain_win"
#define GEGRA_PANEL	    	"gra_panel"
#define GEHINTS_POPUP		"hints_popup"
#define GEATTRIBUTES_POPUP	"attributes_popup"
#define GEERROR_MESSAGE		"error_message"
#define GEFILEOPS_POPUP		"fileops_popup"
#define GESHOW_CROP_BOX		"show_crop_box"
#define GECROPSAVE_POPUP	"cropsave_box"
#define GEGRID_POPUP		"grid_popup"
#ifdef VMS
#define GEGRA_HELP		"sys$help:gra_help.decw$book"
#else
#define GEGRA_HELP		"/usr/lib/X11/app-defaults/rags/gra_help.decw_book"
#endif
#define GECONFIRM_SAVE_POPUP	"confirm_save_popup"
#define GECONFIRM_CLOSE_POPUP	"confirm_close_popup"
#define GECONFIRM_CLEAR_POPUP	"confirm_clear_popup"
#define GECONFIRM_PURGE_POPUP	"confirm_purge_popup"
#define GECONFIRM_PURGE_HIDDEN_POPUP	"confirm_purge_hidden_popup"
#define GECONFIRM_EXIT_POPUP	"confirm_exit_popup"
#define GEUNITS_POPUP		"units_popup"
#define GEZOOM_POPUP		"zoom_popup"
#define GEMOPS_INTERFACE	"mops_interface"
#define GESETTINGS_POPUP	"option_settings_popup"
#define GEDIGITIZE_AS_BOX	"digitize_as_box"
#define GEDPY_BOX		"screen_info_box"
#define GEDESCRIPTION_POPUP	"description_popup"
#define GECREATE_LIN_POPUP	"create_lin_box"
#define GECREATE_ARC_POPUP	"create_arc_box"
#define GECREATE_CIR_POPUP	"create_cir_box"
#define GECREATE_ELP_POPUP	"create_elp_box"
#define GECREATE_REC_POPUP	"create_rec_box"
#define GEANIMATE_POPUP		"animation_popup"

/* Constants
 */
#define GEBUTTCAP     1	
#define GEROUNDCAP    2	
#define GEPROJECTING  3	

#define GEMITERJOIN   0	
#define GEROUNDJOIN   1	
#define GEBEVELJOIN   2	

#define GESTYLETRANSP  0
#define GESTYLETRANSL  1
#define GESTYLEOPAQUE  2

#define	GEDIG_THRESHOLD	     0
#define	GEDIG_HALFTONE_HIGH  1
#define	GEDIG_HALFTONE_LOW   2
#define	GEDIG_CONVERT_COL    3

#define	GERGBMODEL	     0
#define	GECMYKMODEL	     1
#define	GECOLORLISTMODEL     2

#define GEHELP_SENSITIVE     0
#define GEHELP_OVERVIEW      1
#define GEHELP_GLOSSARY      2
#define GEHELP_INDEX         3
#define GEHELP_FEATURES      4
#define GEHELP_USING         5
#define GEHELP_PRODUCT       6
#define GEHELP_KEYBOARD      7
#define GEHELP_TUTORIAL      8

/*  Objects
 */
#define GEOBJ_LIN     1	
#define GEOBJ_BOX     2	
#define GEOBJ_BXR     3	
#define GEOBJ_ARC     4	
#define GEOBJ_CIR     5	
#define GEOBJ_ELP     6	
#define GEOBJ_PIE     7	
#define GEOBJ_POL     8	
#define GEOBJ_IMG     9	

/*  Callback Reasons
 */
#define GENO            500
#define GEYES           501
#define GECANCEL        502
#define GEOK 	        503
#define GEAPPLY	        504
#define	GERESET         505
#define	GEVALUPDATE     506
#define GEVALUE	        507
#define GEACTIVATE      508
#define GECREATEWIDG    509
#define GEON	        510
#define GEOFF	        511
#define GEOPEN	        512
#define	GEIMPORT        513
#define GECLIPART       514
#define	GESAVEAS        515	
#define GECLOSEPAGE	516
#define	GEGET	        517	
#define	GEPUT           518	
#define	GEGETALL        519
#define	GEPUTALL        520
#define	GECROPONSAVE    521
#define	GEWINDOW_CREATE 522
#define GEDIRECTION     523
#define GERSCROLL       524	
#define GELSCROLL       525	
#define GEUSCROLL       526	
#define GEDSCROLL       527	
#define	GERPAGE 	528	
#define	GELPAGE 	529	
#define	GEUPAGE         530	
#define	GEDPAGE         531	
#define	GEINTERACTIVE	532 
#define	GECLEAR 	533 
#define	GEPREVIEWAS 	534
#define GEADD		535
#define GERETRIEVE	536
#define GEDELETE 	537


/* The following is a list of widget id's for the global GE_UI structure.
 * The order is IMPERATIVE and each widget id must be unique.
 * 
 *  Panel, Scrollbars, Popup Dialog Boxes
 */
#define	GEWMENUBAR         0
#define	GEWPANEL           1
#define GEWWINDOW    	   2 
#define GEWHSCROLL    	   3 
#define GEWVSCROLL    	   4
#define GEWERROR     	   5
/*#define GEWHELP      	   6	 unused */
#define GEWFILEOPS   	   7 
#define GEWOPEN       	   8
#define GEWIMPORT    	   9
#define GEWEXPORT     	  10
#define GEWSAVE      	  11 
#define	GEWCLOSE 	  12
#define	GEWEXIT 	  13
#define GEWCLEAR 	  14
#define	GEWPURGE	  15
#define GEWZOOM      	  16 
#define GEWCROPBOX        17
#define GEWCROPSAVEBOX    18
#define GEWATTRIBUTES	  19
#define GEWTEXTFONTS      20
#define GEWOBJEXTATTR     21
#define GEWOBJINTATTR	  22
#define GEWTEXTFGATTR 	  23
#define GEWTEXTBGATTR	  24
#define GEWPAGEATTR 	  25
#define GEWHINTS      	  26
#define GEWUNITS     	  27 
#define GEWGRID     	  28
#define GEWSETTINGS	  29
#define GEWDIGITIZEBOX	  30
#define GEWSAVEAS	  31
#define GEWDESCRIPTION	  32
#define	GEWDESCRIP_TEXT	  33
#define	GEWPREVIEWAS 	  34

/* Various Window Widgets 
 */
#define GEWCROPWINDOW     35
#define GEWCROPSAVEWINDOW 36
#define	GEWHINTSMSG       37 
#define	GEWUNITSMSG       38 
#define	GEWUNITSLIST      39
#define GEWSTATWINDOW     40
#define	GEWDPYBOX	  41
#define	GEWDPY_WIN	  42
#define	GEWPURGEHIDDEN	  43

/* Buttons on Menubar pulldowns, used for greying out 
 */
#define GEWCMAP_ENTRY     45
#define GEWEXPORTPB	  46
#define GEWSAVEPB	  47
#define GEWSAVEASPB	  48
#define	GEWCLOSEPB	  49
#define GEWSELECTALLPB	  50
#define GEWGROUPALLPB	  51
#define GEWGROUPTXTPB	  52
#define GEWREVEALPB	  53
#define GEWUNDELETEPB	  54
#define GEWPURGEPB	  55
#define GEWROUNDPB    	  56
#define GEWCLEARPB	  57
#define GEWREFRESHPB	  58
#define GEWSHOWCROPPB	  59
#define	GEWREDIGITIZEPB   60
#define GEWUNZOOMPB	  61
#define GEWPANVIEWPB 	  62
#define GEWSETVIEWPB 	  63
#define GEWCUTPB     	  64
#define GEWPASTEPB   	  65
#define GEWPURGEHIDDENPB  66

/* Cascade Pushbuttons	
 */
#define	GEWDIGTHRESHOLD	    70
#define	GEWDIGHALFTONEHIGH  71  
#define	GEWDIGHALFTONELOW   72  
#define	GEWDIGCONVCOL 	    73        
#define	GEWZOOM1_4X         74
#define	GEWZOOM1_2X         75
#define	GEWZOOM2X           76
#define	GEWZOOM3X           77
#define	GEWZOOM4X           78
#define GEWZOOMTOBOX        79
#define GEWZOOMUSER         80
#define	GEWZOOMFACTOR  	    81  

/* Settings Popup buttons
 */
#define	GEWSCRAUTO        90 
#define	GEWHILIGHTMODE	  91
#define	GEWLINEPOINTS  	  92
#define	GEWAUTOAPPLY	  93 
#define	GEWDAMAGEREPAIR   94  
#define	GEWPRINTPGBG      95 
#define	GEWCONFIRM	  96
#define	GEWTEXTJUSTMENU	  97 
#define GEWJUSTTXTC       98 
#define GEWJUSTTXTL       99 
#define GEWJUSTTXTR       100
#define	GEWTEXTMODEMENU	  101
#define	GEWINSERT         102
#define	GEWOVERSTRIKE  	  103
#define	GEWTAB 		  104
#define	GEWCROSSWINDOW    105
#define GEWANIMCURSOR     106   
#define	GEWZOOMLINETHICK  107
#define	GEWOBJACTION      108
#define	GEWROUNDCOORDS    109
#define	GEWIMGSTORAGE     110
#define	GEWAUTOMOVE       111	
#define	GEWANIMDISPMENU	  112         
#define	GEWANIM_LIN  	  113 
#define	GEWANIM_TRUE 	  114 
#define	GEWANIM_BOX  	  115 
#define	GEWANIM_NONE 	  116 
#define	GEWCROPMENU	  117
#define	GEWAUTOCROP	  118
#define	GEWMANUALCROP	  119
#define	GEWWINDOWCROP	  120
#define	GEWCHECKPTMENU	  121
#define	GEWCHKPT_OFF 	  122 
#define	GEWCHKPT_5   	  123 
#define	GEWCHKPT_15  	  124 
#define	GEWCHKPT_50  	  125 
#define	GEWCHKPT_100 	  126 
#define	GEWCHKPT_200 	  127 
#define	GEWSTARTICON	  128
#define	GEWDISPLAYEPS 	  129

/* FileOps Popup buttons
 */
#define GEWVERSIONMENU	   134
#define	GEWGROUPIMPORT	   135 
#define	GEWGROUPOPEN   	   136
#define GEWMANUALPLACEMENT 137
#define GEWVERSIONCURRENT  138
#define GEWVERSION5	   139

/* Grid Popup buttons
 */
#define	GEWGRIDON         140 
#define	GEWGRIDTOP        141 
#define	GEWGRIDXALIGN     142 
#define	GEWGRIDYALIGN     143 
#define	GEWGRIDMAJORX     144  
#define	GEWGRIDMAJORY     145  
#define	GEWGRIDDIVX       146 
#define	GEWGRIDDIVY       147 

/* Attributes Popup widgets
 */
#define	GEWATTR1FORM  	  150
#define	GEWATTR2FORM  	  151
#define GEWATTR3FORM  	  152
#define GEWSAMPLEFORM 	  153
#define GEWGETENTRY	  154
#define GEWGETALL         155 
#define GEWGETMARKED      156 
#define GEWSETENTRY	  157
#define GEWSETALL         158 
#define GEWSETMARKED      159 
#define	GEWATTRTYPEMENU   160 
#define GEWFONTSLIST      161 
#define	GEWMODEMENU 	  162
#define	GEWTRANSPPB       163
#define	GEWTRANSLPB       164
#define	GEWOPAQUEPB       165
#define GEWCOLORPICKER    166 
#define GEWRGBMODEL	  167
#define GEWCMYKMODEL	  168
#define GEWHSBMODEL	  169
#define GEWCOLORLISTMODEL 170
#define	GEWCOLORRED	  171
#define	GEWCOLORGREEN	  172
#define	GEWCOLORBLUE	  173
#define	GEWCOLORGRAY	  174
#define	GEWCOLORCYAN	  175
#define	GEWCOLORMAGENTA	  176
#define	GEWCOLORYELLOW	  177
#define	GEWCOLORBLACK	  178
#define	GEWCOLORHUE	  179
#define	GEWCOLORSATURATION 180
#define	GEWCOLORBRIGHTNESS 181
#define	GEWCOLORLIST	  182
#define	GEWFILLPATTERN	  183
#define	GEWCAPMENU 	  184
#define GEWCAPBUTT     	  185 
#define GEWCAPROUND  	  186 
#define GEWCAPPROJ   	  187 
#define	GEWJOINMENU 	  188
#define GEWJOINMITER 	  189 
#define GEWJOINROUND 	  190 
#define GEWJOINBEVEL 	  191 
#define	GEWLWLABEL     	  192
#define	GEWLWTEXT     	  193
#define	GEWLSMENU 	  194
#define	GEWLINESOLID	  195    
#define	GEWLINEDASH 	  196    
#define	GEWLINEDOTTED	  197    
#define	GEWLINEDASHDOT	  198    
#define	GEWLINEUSERDEF	  199    
#define	GEWLSON1LABEL 	  200
#define	GEWLSOFF1LABEL 	  201
#define	GEWLSON2LABEL 	  202
#define	GEWLSOFF2LABEL 	  203
#define	GEWLSTEXT1     	  204
#define	GEWLSTEXT2     	  205
#define	GEWLSTEXT3     	  206
#define	GEWLSTEXT4     	  207
#define	GEWMODETOFROM     208 
#define GEWCOLORTOFROM    209 
#define	GEWFILLTOFROM     210 
#define	GEWCAPTOFROM	  211
#define	GEWJOINTOFROM     212
#define	GEWLWTOFROM   	  213
#define	GEWLSTOFROM    	  214
#define GEWATTRSAMPLEWIN  215
#define GEWAPPLYBUTTON    216
#define	GEWRESETBUTTON	  217
#define	GEWSEPTOFROM 	  218
#define	GEWSEPMENU   	  219
#define	GEWKNOCKOUT 	  220
#define	GEWOVERPRINT 	  221

/* Buttons on the Control Panel
*/
#define	GEWARCOBJECT      225
#define	GEWCIROBJECT 	  226
#define	GEWELPOBJECT 	  227
#define	GEWBXROBJECT 	  228
#define	GEWLINOBJECT 	  229
#define	GEWPOLYOBJECT     230
#define	GEWBOXOBJECT 	  231
#define	GEWPIEOBJECT 	  232
#define	GEWCREATETOSIZE   233
#define	GEWSTACKONTOP     234
#define	GEWEDIT_OBJ  	  235
#define	GEWMOVE_OBJ  	  236
#define	GEWGROUP_OBJ 	  237
#define	GEWUNGROUP_OBJ	  238
#define	GEWTOP_OBJ   	  239
#define	GEWBOTTOM_OBJ	  240
#define	GEWHIDE_OBJ	  241
#define	GEWROTATE_OBJ	  242
#define	GEWALIGN_OBJ	  243
#define	GEWDELETE_OBJ	  244
#define	GEWCOPY_OBJ	  245
#define	GEWMIRROR_OBJ	  246
#define	GEWCOPYMIRROR_OBJ 247
#define	GEWABOVE_OBJ	  248
#define	GEWBELOW_OBJ	  249
#define	GEWCOMPLEMENT_OBJ 250
#define	GEWSCALE_OBJ	  251
#define	GEWANIMATE_OBJ	  252
#define	GEWHORZCONSTR	  253 
#define	GEWVERTCONSTR	  254 
#define	GEWEDITPOINTS     255
#define	GEWGRPEDIT        256
#define	GEWGRAVON         257
#define	GEWGRAVVAL        258
#define	GEWCLOCKWISE 	  259
#define	GEWROTATEANGLE 	  260
#define	GEWINTERACTIVE	  261
#define	GEWSCALEPERCENT	  262
#define	GEWALNFROMMENU    263
#define	GEWALNTOMENU	  264
#define	GEWALNFROMTOP  	  265
#define	GEWALNFROMBOT 	  266
#define	GEWALNFROMCTR 	  267
#define	GEWALNFROMLFT 	  268
#define	GEWALNFROMRT 	  269
#define	GEWALNTOTOP 	  270
#define	GEWALNTOBOT 	  271
#define	GEWALNTOCTR	  272
#define	GEWALNTOLFT	  273
#define	GEWALNTORT 	  274
#define	GEWALNTOHORZ	  275
#define	GEWALNTOVERT	  276
#define	GEWWHITEOUT	  277

/* MOPS (Export Popup) Widgets
 */
#define GEWMOPS_FILENAME    300
#define GEWMOPS_FSE         301
#define GEWMOPS_DDIF        302
#define GEWMOPS_PS          303
#define	GEWMOPS_SIX 	    304
#define GEWMOPS_MONOSIX     305
#define GEWMOPS_COLORSIX    306
#define	GEWMOPS_X 	    307
#define GEWMOPS_SDML        308
#define GEWMOPS_FIGURE      309
#define GEWMOPS_FIGUREW     310
#define GEWMOPS_ICON        311
#define GEWMOPS_ICONR       312
#define GEWMOPS_MARGICON    313
#define GEWMOPS_LIST        314 
#define GEWMOPS_SCALE       315
#define GEWMOPS_XSCALE      316
#define GEWMOPS_YSCALE      317
#define GEWMOPS_SIZE        318
#define GEWMOPS_WIDTH       319
#define GEWMOPS_HEIGHT      320
#define GEWMOPS_ASPECT      321
#define GEWMOPS_UNITS       322
#define GEWMOPS_INCHES      323
#define GEWMOPS_PICAS       324
#define GEWMOPS_MM          325
#define	GEWMOPS_EXCLSCALE   326
#define GEWMOPS_ROTATE      327
#define GEWMOPS_ANGLE       328
#define GEWMOPS_LABELTEXT   329
#define GEWMOPS_CLOCKWISE   330
#define	GEWMOPS_EXCLROT     331
#define GEWMOPS_ORIENTATION 332
#define GEWMOPS_PORTRAIT    333
#define GEWMOPS_LANDSCAPE   334
#define GEWMOPS_PAPERSIZE   335
#define GEWMOPS_LETTER      336
#define GEWMOPS_LEDGER      337
#define GEWMOPS_LEGAL       338
#define GEWMOPS_EXECUTIVE   339
#define GEWMOPS_A3          340
#define GEWMOPS_A4          341
#define GEWMOPS_A5          342
#define GEWMOPS_B4          343
#define GEWMOPS_B5          344
#define GEWMOPS_PLACEMENT   345
#define GEWMOPS_CENTER      346
#define GEWMOPS_LL          347
#define GEWMOPS_LR          348
#define GEWMOPS_UL          349
#define GEWMOPS_UR          350
#define GEWMOPS_BACKGROUND  351
#define GEWMOPS_COMPLEMENT  352
#define GEWMOPS_CROPMARK    353
#define GEWMOPS_LABEL       354
#define	GEWMOPS_CAPTION     355
#define	GEWMOPS_CAPTIONTEXT 356
#define GEWMOPS_MARGIN      357 
#define GEWMOPS_MIRROR      358
#define GEWMOPS_HMIRROR     359  
#define GEWMOPS_VMIRROR     360
#define GEWMOPS_HVMIRROR    361
#define GEWMOPS_OUTLINE     362 
#define GEWMOPS_TEXTWIDTH   363
                             
/* Create to Size Popups
 */
#define	GEWCREATE_ARC 	  400
#define	GEWCREATE_CIR 	  401
#define	GEWCREATE_ELP 	  402
#define	GEWCREATE_REC 	  403
#define	GEWCREATE_LIN 	  404
#define	GEWCREATE_LINW	  405
#define	GEWCREATE_LEN	  406
#define	GEWCREATE_DEGROT  407
#define	GEWCREATE_RAD     408
#define	GEWCREATE_STARTA  409
#define	GEWCREATE_ENDA    410
#define	GEWCREATE_WIDTH	  411
#define	GEWCREATE_HEIGHT  412   	
#define GEWCREATE_ROTATE  413

#define	GEWANIMATION 	  420
#define GEWNUMFRAMES	  421
#define GEWFIRSTFRAME	  422
#define GEWLASTFRAME	  423
#define GEWNUMPACKETS	  424
#define GEWREVERSEEND	  425
#define GEWSTARTFORWARD	  426
#define GEWCLIPOVERRIDE	  427
#define GEWCZUPDISP	  428
#define GEWREPEAT	  429
#define GEWONTIME	  430
#define GEWOFFTIME	  431
#define GEWDELTAX	  432
#define GEWDELTAY	  433
#define GEWHSCALE	  434
#define GEWVSCALE	  435
#define GEWANIMROT	  436
#define GEWANIMCLOCKWISE  437
#define GEWERASE	  438
#define GEWREDRAW	  439
#define GEWINTERRUPT	  440
#define GEWBLOCK	  441
#define GEWANIMFLAG	  442
#define GEWPACKETNUM	  443
#define GEWDELETEPB   	  444
#define GEWRETRIEVEPB 	  445
#define GEWPREVPB         446
#define GEWNEXTPB         447
