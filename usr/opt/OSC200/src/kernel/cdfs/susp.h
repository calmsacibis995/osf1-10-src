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
 * @(#)$RCSfile: susp.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/07/16 13:06:09 $
 */
#ifndef	_CDFS_SUSP_INCLUDE
#define	_CDFS_SUSP_INCLUDE	1
/*
 * System Use Sharing Protocol, version 1 (rev 1.09) (August 1991)
 */

struct cd_suf_header {
    unsigned char suf_sig_word[2];
    unsigned char suf_length;
    unsigned char suf_version;
    /* usually there is data below here */
    /* total size of a SUF is suf_length */
};

/* The SUFs defined by SUSP: */

#define SUF_CONTINUATION_AREA	"CE"
#define SUF_PADDING_FIELD	"PD"
#define SUF_SUSP_INDICATOR	"SP"
#define SUF_SUSP_TERMINATOR	"ST"
#define SUF_EXTENSIONS_REF	"ER"

/* Continuation Area:  when things don't fit in the directory
   System Use Area, a CE entry points to a continuation area.
   
   Multiple continuation areas may share one CE block, hence the offset
   field.
 */
struct cd_suf_ce {
    /* hdr.suf_sig_word == "CE"
       hdr.suf_length = 28
       hdr.suf_version = 1 */
    struct cd_suf_header hdr;
    unsigned char cont_lbn_lsb[4];	/* LBN of continuation area */
    unsigned char cont_lbn_msb[4];
    unsigned char cont_offset_lsb[4];	/* offset within LBN of continuation */
    unsigned char cont_offset_msb[4];
    unsigned char cont_len_lsb[4];	/* length of continuation */
    unsigned char cont_len_msb[4];
};
#define	SUF_CE_LEN	28
#define	SUF_CE_VERS	1
/* The SUSP in use ("SP") field is encoded only once per disc, in the
   root directory on the entry for the root directory */
struct cd_suf_sp {
    /* hdr.suf_sig_word == "SP"
       hdr.suf_length = 7
       hdr.suf_version = 1 */
    struct cd_suf_header hdr;
    unsigned char sp_checkbytes[2];	/* == 0xBEEF */
#define	SUF_SP_CHECK1	0xBE
#define	SUF_SP_CHECK2	0xEF
    unsigned char sp_len_skp;		/* skip offset for each dir entry
					   SUA to find first SUSP field */
};
#define	SUF_SP_LEN	7
#define	SUF_SP_VERS	1
/* The SUSP terminator ("ST") field indicates the end of SUSP fields in
   a SUA */ 
struct cd_suf_st {
    /* hdr.suf_sig_word == "ST"
       hdr.suf_length = 4
       hdr.suf_version = 1 */
    struct cd_suf_header hdr;
    /* nothing extra */
};
#define	SUF_ST_LEN	4
#define	SUF_ST_VERS	1
/* The Extension Reference ("ER") field is used to store information uniquely
   identifying a specific extension in use on a volume */
struct cd_suf_er {
    /* hdr.suf_sig_word == "ER"
       hdr.suf_length = 8 + LEN_ID + LEN_DES + LEN_SRC
       hdr.suf_version = 1 */
    struct cd_suf_header hdr;
    unsigned char er_len_id;		/* length of extension identifier */
    unsigned char er_len_des;		/* length of extension descriptor */
    unsigned char er_len_src;		/* length of extension spec source */
    unsigned char er_ext_ver;		/* extension version # */
    unsigned char er_data[1];		/* probably longer than 1 byte.
					   contains:
					   [LEN_ID] bytes of ID
					   [LEN_DES] bytes of descriptor
					   [LEN_SRC] bytes of source */
    /* DES & SRC may have extra bytes beyond those specified by
       the source reference */
};
#define	SUF_ER_VERS	1

/* convert signature to integral value: */
#define SUSP_SHORTIFY(sig) (((sig)[0] << 8)|((sig)[1]))

/* SUSP 6.3:  we must provide an API to retrieve, for a particular
   directory record:
   . a complete SUF selectable by Signature, field count, or both 
   . the contents of the skip area (those bypassed due to sp_len_skp)
 */

#ifndef _KERNEL
extern int susp_get_suf(const char *pathname,
			char signature[2],
			int fieldno,
			unsigned int flags);
#endif

#endif _CDFS_SUSP_INCLUDE
