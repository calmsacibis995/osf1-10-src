/*
** imgprot_ids.h - IMG prototypes of internal functions used by IDS, with
** the actual arguments replaced by unsigned longs.
*/

#ifndef IMGPROT_IDS_H

#define IMGPROT_IDS_H

#ifdef IMGDEF_H
PROTO( unsigned long ImgConvertFrame, (
	unsigned long /*srcfid*/, 
	struct ITMLST */*itmlst*/, 
	long /*flags*/));

PROTO( unsigned long ImgCvtAlignment, (
	unsigned long /*srcfid*/, 
	struct ITMLST */*itemlist*/, 
	unsigned long /*flags*/));

PROTO( struct UDP *_ImgSetRoi, (
	struct UDP */*udp*/, 
	unsigned long /*roi*/));
#endif

PROTO( unsigned long ImgCvtCompSpaceOrg, (
	unsigned long /*src_fid*/, 
	unsigned long /*ret_org*/, 
	unsigned long /*ret_dp_signif*/, 
	unsigned long /*flags*/));

PROTO( char *_ImgAllocateDataPlane, (
	int /*size*/, 
	int /*fill_mask*/));

PROTO( char *_ImgCalloc, (
	int /*number*/, 
	int /*user_size*/));

PROTO( void _ImgCfree, (
	void */*memptr*/));

PROTO( long _ImgCheckNormal, (
	unsigned long /*fid*/));

PROTO( unsigned long _ImgCloneFrame, (
	unsigned long /*src_fid*/));

PROTO( unsigned long _ImgErase, (
	unsigned long /*fct*/, 
	long /*itemcode*/, 
	long /*index*/));

PROTO( void _ImgFree, (
	void */*memptr*/));

PROTO( unsigned long _ImgGet, (
	unsigned long /*fct*/, 
	long /*itemcode*/, 
	void */*bufptr*/, 
	long /*buflen*/, 
	long */*retlen*/, 
	long /*index*/));

PROTO( char *_ImgMalloc, (
	int /*user_size*/));

PROTO( unsigned long _ImgPut, (
	unsigned long /*fct*/, 
	long /*itemcode*/, 
	void */*bufptr*/, 
	long /*buflen*/, 
	long /*index*/));

PROTO( char *_ImgRealloc, (
	char */*oldbuf*/, 
	int /*user_size*/));

PROTO( void _ImgStoreDataPlane, (
	unsigned long /*fid*/, 
	char */*image_data*/));

#endif /* IMGPROT_IDS_H */
