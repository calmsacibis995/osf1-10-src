    	.TITLE  IMG_XSTATUS_CODES - Status and messages for Image Utilities
    	.IDENT	'Version X1.0'

    	.FACILITY ISL,492 /PREFIX = ImgX_

!+
! NOTES:
!
!	* The .TITLE keyword must be separated by two spaces instead
!	  of a tab so that the .TPU procedure that modifies this file
!	  to produce the IMG_XSTATUS_CODES.OBJ file will be able to
!	  find it.
!
!	* The prefix string "Img$_" must be in mixed case so that the
!	  CHF message compiler will produce "ImgX_" prefixes also with
!	  mixed case.
!-

!+
! General success codes
!-
    	.SEVERITY SUCCESS
    	.BASE 0
NORMAL			<Normal successful completion>
SUCCESS			<Successful Completion>

!+
! Informational codes.
!-
    	.BASE 100
    	.SEVERITY INFORMATIONAL
!AGGNOTSEG	<aggregate type was not DDIF$_SEG>
IMGNOTCMP	<image not compressible>
NODATAFND	<No image data was found within the Begin/EndSegment pair>
NOSAVDCTX	<no saved context to restore, context not changed>

!+
! Warning codes. (LIB$SIGNAL only)
!-
    	.SEVERITY WARNING
    	.BASE 200
ATRNOTNOR	<attribute does not have normalized value, item code = !ZL>-
		/FAO_COUNT=1
DXCPYTRUN	<destination string truncated by descriptor copy function>
INVFLGARG       <invalid option flags argument passed>
INVITMCOD	<invalid item code>
INVSCNLEN       <invalid scanline length - line longer than specified in frame>
MODPRMSNG	<DDIF xlate stream-mode parameter missing>
NOIDUFOU	<no image data unit aggregate handle found in frame>
NOPRAFOU	<no segment presentation attributes aggregate handle found in frame>
NOROOTFOU	<no root aggregate handle found in frame>
NOSEGFOU	<no segment aggregate handle found in frame>
NOSGAFOU	<no segment attributes aggregate handle found in frame>
NULPTRFND	<null pointer found when expecting ptr to block of type !ZB>-
		/FAO_COUNT=1
PRVCTXINV	<previous context was invalid and not restored, context not changed>
RVALTRUNC	<possible ret value truncation, ret buf shorter than item len>
UNSASPRAT	<unsupported aspect ratio>
UNSCMPTYP	<unsupported compression type>
UNSGRDTYP	<unsupported grid type>
UNSPXLSTR	<unsupported pixel stride>
UNSSPCTYP	<unsupported spectral type code, cannot be processed>
UNXPCTEOB	<unexpected End of Data buffer condition, no action rtn given>-
		/FAO_COUNT=2
!
! V3 warning codes
!
ATRNOTSTD	<attribute value !ZL for item code !ZL, index !ZL, is not a standardized value>-
		/FAO_COUNT=3
BPCNOTEQL	<bits per component are not equal for component !ZL, value !ZL, and component !ZL, value !ZL>-
		/FAO_COUNT=4
BRXNESTRX	<bounding rectangle X coordinate is not equal to the starting X coordinate of the ROI definition>
BRYNESTRY	<bounding rectangle Y coordinate is not equal to the starting Y coordinate of the ROI definition>
DPNOTUNC	<frame data plane of index !ZL was not uncompressed>/FAO_COUNT=1
DTYPMISMA	<data type mismatch between lookup table definition value !ZL and frame data plane value !ZL>/FAO_COUNT=2
FRMNOTFXD	<frame not given DDIF attribute of fixed>
FRMNOTUNC	<frame data is not uncompressed>
INCBPPTOT	<inconsistant bits per pixel total>
INCNSARG	<inconsistent argument>
INSDPSIZE	<insufficient data plane size of !ZL bytes for uncompressed data plane index !ZL>-
		/FAO_COUNT=2
INVATRIDX	<invalid attribute index for item code !ZL, index !ZL>-
		/FAO_COUNT=2
INVATRVAL	<invalid attribute value for item code !ZL, value !ZL, index !ZL>-
		/FAO_COUNT=3
INVBRXSZ	<invalid bounding rectangle X dimension (number of pixels)>
INVBRYSZ	<invalid bounding rectangle Y dimension (number of scanlines)>
INVDARG		<invalid argument>
INVDPSIZE	<invalid data plane size of !ZL bytes when expecting !ZL for data plane index !ZL>-
		/FAO_COUNT=3
INVDTYPE	<src_udp has invalid datatype>
INVIDUCNT	<invalid image data unit count of !ZL for image content element index !ZL>-
		/FAO_COUNT=2
INVLUTATR	<invalid lookup table attribute for itemcode !ZL, value !ZL, index !ZL>/FAO_COUNT=3
INVLUTSIZ	<invalid lookup table size of !ZL was less than minimum required size of !ZL>/FAO_COUNT=2
INVPLNCNT	<invalid data plane count, !ZL planes per pixel is greater thanthe total bits per pixel !ZL>-
		/FAO_COUNT=2
INVPREDFL	<invalid predefined lut type specified, not a user lut either>
INVSCNSTR	<invalid scanline stride, stride value !ZL for index !ZL is less than the product of pixel count and pixel stride>-
		/FAO_COUNT=2
INVZERVAL	<invalid value of zero for item code !ZL, index !ZL>/FAO_COUNT=2
NODATAPLA	<no data plane was found for idu aggr index !ZL, for image content aggr index !ZL>-
		/FAO_COUNT=2
NOIMGFOU	<no image content aggregate handle found in frame>
NOINPLACE	<this function does not support an in place operation>
NOMATCH		<src_udps have inconsistent spatial or data type parameters>
NONSTDBPP	<nonstandard bits per pixel total, value not an 8 bit multiple>
VALNOTCON	<value type for item code !ZL not marked as a DDIF constant value>-
		/FAO_COUNT=1
ZERLUTTAB	<invalid zero lut table address>
!
! Fatal codes (LIB$STOP only)
!
	.BASE 400
	.SEVERITY FATAL

ACTNOTDEF	<action routine not defined when required>
ACTNOTSUC	<action routine did not return a status code for success, status = !ZL>-
		/FAO_COUNT=1
AGGNOTSEG	<aggregate passed in was not a DDIF$_SEG aggregate>
AGNOTINFR	<image content element aggregate was not in the frame>
ATTLSTREQ       <attribute list required for this routine>
BLKNOTINT	<block with address !XL is has not been internally allocated>-
		/FAO_COUNT=1
BUFOVRFLW	<buffer overflow with no action routine specified>
BUFWNOLEN	<buffer address given with no length value>
DDIFIMPER	<DDIF data import error, could not get next aggregate>
DLENGTR32	<destination pixel length is greater than 32 bits>
DSCNOTDYN	<descriptor was not of class DYNAMIC>
DSTLENZER	<destination pixel length is zero>
EDGBOUNDE	<edge boundary calculation error>
FRAMIMPER	<Frame import error when importing data for frame-id !ZB>-
		/FAO_COUNT=1
ILLDSTBUF	<illegal destination buffer address, cannot be set by item list>
ILLIMGSEG	<illegal DDIF$_IMG found, not within the scope of a DDIF$_SEG>
INCACCMOD	<inconsistent access mode, not consistent with func. operation>
INCFRMATT       <frame attributes of new frame inconsistent with old frame>
INCLSTCNT	<inconsistent list count>
INSVIRMEM	<insufficient virtual memory, could not sbrk(2) more memory>
INVAGRSEQ	<invalid aggregate sequence, DDIF$_DSC or DDIF$_DHD out of order>
INVALDFID       <invalid frame identifier>
INVALDSID	<invalid stream id, not a valid stream identifier>
INVALIGNV	<invalid alignment value !XL>/FAO_COUNT=1
INVALLSIZ	<invalid memory allocation size, must not be zero>
INVARGCNT	<invalid argument count, correct number of arguments is !ZL>/FAO_COUNT=1
INVARGCON	<invalid argument contents, argument values conflict>
INVBITPXL	<invalid bits per pixel>
INVBITSPC	<invalid bits per component for component !ZL>/FAO_COUNT=1
INVBLKTYP	<invalid block type, unexpected type value = !ZB>-
		/FAO_COUNT=1
INVBPCLST	<invalid bits per component list>
INVBUFLEN	<invalid buffer length or buffer size less than minimum >
INVCMPCNT	<invalid component count>
INVCMPIDX	<invalid component index>
INVCMPMAP	<invalid value !XL for component mapping scheme>/FAO_COUNT=1
INVCMPSTL	<invalid component stride list>
INVCMPSTR	<invalid component stride, lss bits per comp for component !ZL>-
		/FAO_COUNT=1
INVCMPTYP	<invalid compression type>
INVCODTYP	<invalid code type encountered during decompression>
INVCTXUSE	<invalid use of ctx-id, file or stream usage mixed up>
INVDDIFMD	<invalid ddif stream mode, not READ or WRITE>
INVDSCCLA	<invalid descriptor class value>
INVDSCLEN	<invalid descriptor length value>
INVDSCTYP	<invalid descriptor type>
INVDTHBIT       <invalid attempt to dither bitonal image>
INVDTHLEV	<invalid dither output level>
INVDTHSPT	<invalid spectral type for dithering>
INVDTHTYP       <invalid dither type specified>
INVFLAORD	<invalid free list block-address order>
INVFLBLEN	<invalid FLB length, must be greater than zero>
INVFLSORD	<invalid free list block-size order>
INVITMBFL	<invalid item buffer length, could not store item code !ZL >-
		/FAO_COUNT=1
INVHSTGRM	<invalue histogram identifier>
INVLINPRG	<invalid line progression value>
INVMODVAL	<invalid mode parameter value !ZL>/FAO_COUNT = 1
INVPLAIND	<invalid plane index>
INVPXLCNT	<invalid pixel count, must be greater than 0>
INVPXLIDX	<invalid pixel index>
INVPXLPTH	<invalid pixel path value>
INVPXLSTR	<invalid pixel stride, stride value !ZL for index !ZL is less than the number of planebits per pixel>-
		/FAO_COUNT=2
INVROI		<invalid region of interest definition>
INVSCLFTR	<invalid scale factor - scaled to zero size>
INVSCNIDX	<invalid scanline index>
INVSRCDSC	<invalid dynamic source descriptor>
INVTHRSPC	<invalid dither threshold matrix specification>
INVTYPCOD	<invalid type code>
IOBREALER       <internal IO Buffer reallocation error>
ITMCODERR	<item code error, item = !XL>/FAO_COUNT=1
MEMREALER       <memory reallocation error>
NOIMGAGRP	<no image data aggregate pointer in the frame>
NOIMGDATA	<no image data was returned from frame in data plane descriptor>
NOIOTARGT	<no IO target, buffer and action routine are both missing>
NORECTDSC	<no ROI_RECT structure was supplied to define rectangular ROI>
NOSPCPRC	<no low-level special processing available for itemcode !ZL>-
		/FAO_COUNT=1
NOTIMGSEG	<segment aggregate passed in was not for an image frame>
NOTYPESUP	<no support for spectral type of source frame>
NOTUSRMEM	<memory address passed in was not a user memory block>
NOUNCDATA	<no uncompressed data was found in the frame>
NOWRTACC	<nowrite access violation on attempt to set item code !ZL>-
		/FAO_COUNT=1
PARAMCONF	<parameter conflict between context-id and other parameters>
PVTITMCOD	<invalid item code, reserved for private use by Image Services>
PXLTOOLRG	<pixels greater than 25 bits not supported in this implementation>
SLCGTRDLC	<source line count greater than destination line count>
SLENGTR32	<source pixel length is greater than 32 bits>
SPCGTRDPC	<source pixel count greater than destination pixel count>
SPLGTRDPL	<source pixel length greater than destination pixel length>
SPLGTRDPS	<source pixel length greater than destination pixel stride>
SRCLENZER	<source pixel length equal to zero>
UNSCSPORG	<unsupported component space organzation type>
UNSDATALN	<unsupported data alignment -- not byte, work, or longword>
UNSDTHMAP	<dithering unsupported spectral map of input frame>
UNSDTHCPO	<dithering unsupported component organization of input frame>
UNSOPTION       <option not yet supported>

!
! V3 fatal errors
!
DCTCMPIDX	<dct component index error>
DCTDECFAI	<dct decode failure>
DCTENCFAI	<dct encode failure>
DCTFACTER	<dct encode factor error>
DPREMAPER	<data plane remap error, internal error value !ZL>/FAO_COUNT=1
EOF		<end of file condition detected>
FAILURE		<general failure condition>
FILCLOSER	<file close error, error status = !ZL>/FAO_COUNT=1
FILECRERR	<file create error, error status = !ZL>/FAO_COUNT=1
FILOPENER	<file open error, error status = !ZL>/FAO_COUNT=1
FILREADER	<file read error, error status = !ZL>/FAO_COUNT=1
FILWRITER	<file write error, error status = !ZL>/FAO_COUNT=1
FRAMEDEAL	<the frame passed in was already deallocated>
FRMLEVMIS	<frame levels mismatch between frames for component index = !ZL>/FAO_COUNT=1
FRMSIZMIS	<size mismatch between frames>
FRMATTMIS	<attributes mismatch between frames>
IDXRNGEXC	<index range for item code !ZL, index !ZL, exceeded in item list>-
		/FAO_COUNT=2
INVACCMOD	<invalid access mode>
INVACCTYP	<invalid access type>
INVDATCLA	<invalid data class value of zero>
INVDCPP         <invalid control processing plane>
INVDMTHARG	<invalid math argument encountered>
INVFILACC	<invalid file access type>
INVFILBUF	<invalid file buffer length or address>
INVFRMDEF	<invalid frame definition identifier>
INVFRMID	<invalid frame identifier>
INVFTYPE	<invalid file type>
INVHROIDM	<invalid horizontal roi dimension>
INVLUTDEF	<invalid lut definition object>
INVROIITM	<invalid ROI itemlist code>
INVROIOBJ	<invalid ROI object definition identifier>
INVROITYP	<invalid ROI object type set in ROI object>
INVVROIDM	<invalid vertical ROI dimension>
INVZERFID	<invalid value of zero passed in as a frame identifier>
INVZERITM	<invalid zero itemlist addr or first itemcode value>
LUTDFNCNF	<lut definition type parameter conflicts with the presence of specified lut definition object to copy>
LUTTOOSMA	<lut to attach is too small>
MEMALLOC	<memory allocation error, could not allocate dynamic memory>
NOTUSRLTD	<invalid operation, not a user lut definition>
OPARGCONF	<optional arguments conflict>
ROIRECTSZ	<data length value not equal to the size of a ROI_RECT structure for a rectangular ROI description>
TABATRCNF	<presence of table attribute params conflicts with predefined lut definition type>
TABSZNTFO	<table size was not found in the table attributes list>
UNSBITSPC	<unsupported bits per component>
UNSDATACL	<unsupported frame data class>
UNSLTDTYP	<unsupported lookup table data type>
UNSLTABTY	<unsupported lookup table type>
UNSROITYP	<unsupported ROI type>
ZERDFNTYP	<invalid zero specified for lut definition type>
ZERLUTADR	<invalid zero lut address specified>
ZERLUTSIZ	<invalid zero lut size specified>
ZEROROIID	<zero passed in as ROI identifier>

!
! more V3 fatal errors 
!
BADPARAM	<bad parameter value>
INVFILTER	<invalid filter type>

    	.END
