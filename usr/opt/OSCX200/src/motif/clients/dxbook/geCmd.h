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
**	GECMD.H         	  	RAGS commands
**
**  ABSTRACT:
**
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 02/06/90 Created
**
**--
**/

/* static char GESid[] = "@(#)geCmd.h	1.2 2/7/90";                                          */

/*
 * Commands
 */     
#define GEDISP				1	/* Display - full dress	     */
#define GECDISP				2	/* Display - full dress-clipd*/
#define GEXDISP				3	/* Display - Xord - 1 pix wid*/
#define GEHDISP				4	/* Highlight display	     */
#define GEERASE				5	/* Erase area occupied by seg*/
#define GEGDISP			        6	/* Display - using GKS       */
#define GEWEVENT			7	/* Process window event	     */
#define GEADDSEG			8	/* Add A seg to the group    */
#define GECREATE			9	/* Create a new segment	     */
#define GEDEL                           10      /* Delete (just hide really) */
#define GEANIMATE                       11      /* Animate operation         */
#define GEBOUNDS			12	/* Set Obj Bounding Box      */
#define GEEXTENT			13	/* Set Clip=Obj Bounding Box */
#define GEWARP1                         14      /* Priv(xy1) = MouseXY       */
#define GEWARP2                         15      /* Priv(xy2) = MouseXY       */
#define GEWARPX                         16      /* Priv(xy?) = MouseXY       */
#define GEMOVE                          17      /* Move a segment            */
#define GESELPT                         18      /* Point Selection           */
#define GESELBX                         19      /* Box   Selection           */
#define GECLR                           20      /* Wipe clean                */
#define GECOPY                          21      /* Duplicate a segment       */
#define GEEDIT                          25      /* Edit a text string        */
#define GEHXDISP			26	/* Erase highlight	     */
#define GEXBLOCKDISP			27	/* Resume anim disp frm blkd */

#define GEWRITE                         30      /* Write output Meta         */
#define GEREAD                          31      /* Read  input  Meta         */
#define GETXT                           32      /* Add(delete) txt char      */
#define GESCALE                         33      /* Scale a segment           */
#define GEEQUATE                        34      /* State.ASegP = seg         */
#define GEJOIN                          35      /* Join another seg to group */
#define GEJOINALL                       36      /* Join ALL grf elements     */
#define GESEP                           37      /* Break up the group        */
#define GEEXTRACT                       38      /* Extract 1 seg from group  */

#define GEINQLIVE                       41      /* How many LIVE segs in grf */
#define GEINQLINCOL                     42      /* Get seg's skin color attr.*/
#define GEINQFILCOL                     43      /* Get seg's fill color attr.*/
#define GEINQCOL                        44      /* Get color - usually LINCOL*/
#define GEINQVISIBLE                    45      /* How manyVISIBLEsegs in grf*/
#define GEINQLVTXT                      46      /* How many LIVE&VISIBL TXTs?*/
#define GEFLIP                          47      /* Flip                      */
#define GECOPYFLP                       48      /* Copy flip                 */

#define GEOBJCOL                        50      /* Disp color from object    */
#define GEZT                            51      /* Move obj to top of chain  */
#define GEZB                            52      /* Move obj to bot of chain  */
#define GEZU                            53      /* Move obj UP 1 level       */
#define GEZD                            54      /* Move obj DOWN 1 level     */
#define GEADDVERT                       55      /* Obj adds self to fill list*/
#define GEXDEL                          57      /* Undelete - only for Grf   */
#define GESCRR                          58      /* Scroll the window Right   */
#define GESCRL                          59      /* Scroll the window Left    */
#define GESCRU                          60      /* Scroll the window Up      */
#define GESCRD                          61      /* Scroll the window Down    */
#define GEGRAV                          62      /* Given pt close enuf to obj*/
#define GEGRAVTST                       63      /* Tests an obj for grav lock*/

#define GEQDISP				66	/* Display - abreviated      */
#define GEVISIBLE			67	/* Make a segment visible    */
#define GEXVISIBLE			68	/* Make a segment INvisible  */

#define GEROTFIXED                      69      /* Rotate fixed angle        */
#define GEROTX                          70      /* Rotate continuously       */
#define GECONTROL			71	/* Obj corner furtst from x,y*/
#define GEMAG				72	/* % Magnify of object	     */
#define GEALNREF			73	/* Set alignment reference   */
#define GEALN				74	/* Alignment of object	     */
#define GEALNTOP			75	/* Align TOP edges	     */
#define GEALNLFT			76	/* Align LEFT edges	     */
#define GEALNCHV			77	/* Align CENTERS - coincident*/
#define GEALNRT				78	/* Align RIGHT edges	     */
#define GEALNBOT			79	/* Align BOTTOM edges	     */
#define GEALNCH				80	/* Align CENTERS - horizntly */
#define GEALNCV				81	/* Align CENTERS - vertically*/
#define	GEZOOM				82	/* Enlarge-Reduce the drawing*/
#define	GEXZOOM				83	/* Return to Normal magnif.  */
#define GEOBJEDIT                       84      /* Keyboard edit of object   */
#define GEOBJCREATE                     85      /* Keyboard create of object */
#define GECOMP                          86      /* Complement outline color  */
#define GECONSTRAIN                     87      /* Impose constraint on obj  */
#define GECROP                          88      /* Crop an object            */
#define GEGPDELEMPTY                    89      /* Delete empty grps & polys */
#define GEGPLIVE                        90      /* Set all grps & polys=live */
#define GERELPRIV                       91      /* Release private resources */
#define GEKILL                          92      /* Destroy seg & release mem */
#define GEINQIMGREF                     93      /* Num Refs on an ImgPtr     */
#define GEINIT                          94      /* Init FillApplication stats*/
#define GEKILTXT_GRP                    95      /* Kill All TXT segs in GRP  */
#define GEKILNONTXT_GRP                 96      /* Kill All NonTXTsegs in GRP*/
#define GEISOTXT_GRP                    97      /* Isolate all txt in groups */
#define GEADDTXT_GRP                    98      /* Add seg to master txt grp */
#define GEJOINALLTXT                    99      /* Join ALL TXT elements     */
#define GETERM                         100      /* Complete clean up         */
#define GEGENBX                        101      /* Rubber banding a box      */
#define GEROUND                        102      /* Round off all coords->pixl*/
#define GEREADM                        103      /* Read  input  Meta from mem*/
#define GEDISPCROP                     104      /* Display crop box          */
#define GEPGRELINK                     105      /* Repoint poly/grp Private  */
#define GESCRUB                        106      /* Go through grf and tidy up*/
                                                /* Kill empty elements       */
#define GECRGRP                        107      /* Begin making a new group  */
#define GECREATEIMPORT		       108	/* Create a new TXT seg using*/
                                                /* contents of geUtilBuf     */
#define GEGRAVSEARCH                   109      /* Initiate grav-grid search */
#define GEGRAVLOCK                     110      /* Register a pt. as grav lok*/
#define GEZRELINK                      111      /* Relnk grp,grf,pol acording*/
                                                /* to Z order                */
#define GEZRENUM                       112      /* Renumber Zs accordn to lnk*/
#define GESCRDUMP                      113      /* Screen dump               */
#define GETESTPT                       114      /* Flag pts which maybe moved*/
#define GEIMGMAG                       115      /* Reg. mag. BUT only forIMGS*/
#define GEEQUATE_NOIMG                 116      /* Leave images out of it    */
#define GECREATESTAT		       117	/* Create obj non-interactive*/
#define GEMAGRESD		       118	/* Mag for DISPLAY res diffs */
#define GEKILPRIV                      119      /* Release SEG private struct*/
#define GEZREF                         120      /* Setting Z - Relayer - Ref */
#define GEINQFONT                      121      /* Get Obj's Font            */
#define GEGETSEG_COL                   122      /* Find Seg with given color */
#define GEGETLINE                      123      /* Get some outline attrbutes*/
#define GEPUTLINE                      124      /* Set some outline attrbutes*/
#define GEGETFILL                      125      /* Get some fill attributes  */
#define GEPUTFILL                      126      /* Set some fill attributes  */
#define GEGETFONT                      127      /* Get font attribute        */
#define GEPUTFONT                      128      /* Set font attribute        */
#define GEGETTXTBG                     129      /* Get text bkgrnd attribute */
#define GEPUTTXTBG                     130      /* Set text bkgrnd attribute */
#define GEGETTXTFG                     131      /* Get text forgrnd attribute*/
#define GEPUTTXTFG                     132      /* Set text forgrnd attribute*/
#define GEPSADJUST                     133      /* Adjust Y coord for PostScr*/
#define GEINQPTS                       134      /* Request obj's priv coords */
#define GEPURGE                        135      /* Purge delete buffer	     */
#define GEGRPINIT                      136      /* Set-up for a new grp	     */
#define GETESTPTCLR                    137      /* Clear edit point flags    */
#define GESELPTIN                      138      /* Pt Select-INSIDE AN OBJ   */
#define GESELPTINFILL                  139      /* Pt Select-INSIDE FILLD OBJ*/
#define GEDELNOUNDO                    140      /* Delete - no undo for this */
#define GEINQMEMIMG                    141      /* How much mem used by image*/
#define GEINQPOS                       142      /* Current location in geClip*/
#define GEPAN	                       143      /* Dynamic panning of drawing*/
#define GEEXTENT0                      144      /* Bounds - ignore linewidth */
#define GECREATE_NOSNAP1               145      /* No grid-grav snap pt1 obj */
#define GECREATESTAT_NOSNAP1           146      /* These are used in poly    */
#define GESETATTRERASE                 147      /* Set attributes for erasing*/
#define GESETATTRERASE_WHITE           148      /* Set attributes to WHITE   */
#define GEINQXVISIBLE		       149	/* How many Hidden segs	     */
#define GECUT                          150      /* DELETE, copy to cut buf   */
#define GEPASTE                        151      /* Yank from cut buf         */
#define GEAUTODIG                      152      /* Obj changes self into  IMG*/
#define GEXAUTODIG                     153      /* Obj RESTORES self from IMG*/
#define GEINQPSREF		       154	/* Num Refs on a PsBuf       */
#define GEANIMPREVIEW		       155	/* Running animation preview */
#define GEANIMRESET		       156	/* Reset obj. for nxt animatn*/
#define GECZUPDISP		       157	/* Display objs above, cliped*/
#define GEOBJEDITCANCEL                158      /* Cancel Edit of vector objs*/
#define GEPURGEHIDDEN                  159      /* Purge all hidden objects  */
#define GEINTERRUPT                    160      /* Received an interrupt     */
#define GECZDOWNDISP		       161	/* Display objs below, cliped*/
