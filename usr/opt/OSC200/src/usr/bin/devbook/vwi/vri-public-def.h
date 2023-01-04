From decwet::haslock Mon Aug 13 17:29:19 1990
Date: Mon, 13 Aug 90 17:29:18 PDT
From: decwet::haslock
To: slough::haslock

/****************************************************************************
 *                                                                          *
 *  (C) DIGITAL EQUIPMENT CORPORATION 1988, 1989                            *
 *                                                                          *
 *      This is an unpublished work which was created in the indicated      *
 *      year, which contains confidential and secret information,  and      *
 *      which is protected under the copyright laws.  The existence of      *
 *      the copyright notice is not to be construed as an admission or      *
 *      presumption that publication has occurred. Reverse engineering      *
 *      and unauthorized copying is strictly prohibited.   All  rights      *
 *      reserved.                                                           *
 *                                                                          *
 ****************************************************************************
 *
 *
 * Facility:
 *
 *      Bookreader
 *
 * Abstract:
 *
 *      Provide public definitions for users of VOILA Reader/Writer Interface
 *      routines.
 *
 * Functions:
 *
 *      first_function
 *
 * Author:
 *
 *      David L. Ballenger
 *
 * Date:
 *
 *      Tue May 30 15:15:32 1989
 *
 * Revision History:
 *
 *
 */


#define VSE_C_BOOK 1
#define VSE_C_SHELF 2

/* Public type definitions */

/* Define a generic pointer type.  This should really be a pointer to
 * void but not all of our compilers support this yet.
 */

typedef char *VriGenericPtr;

typedef char *vri_t_pool_id ;
typedef char *VriBookId ;
typedef char *VriDirectoryId;
typedef char *VriShelfId;
typedef unsigned long int VriPageId ;
typedef unsigned long int VriChunkId ;
#ifdef major
#undef major
#undef minor
#endif 
typedef struct {
    unsigned short int major;
    unsigned short int minor;
} VoilaVersion ;

#ifdef _STDC_
#define P1(type) type
#define P2(type) type
#define P3(type) type
#define P4(type) type
#define P5(type) type
#define P6(type) type
#define P7(type) type
#define P8(type) type
#else
#define P1(type)
#define P2(type)
#define P3(type)
#define P4(type)
#define P5(type)
#define P6(type)
#define P7(type)
#define P8(type)
#endif

/* Book access routines
 */
extern void           VRI_BOOK_CLOSE();
extern VriChunkId VRI_BOOK_COPYRIGHT_CHUNK();
extern VriPageId  VRI_BOOK_COPYRIGHT_PAGE();
extern unsigned       VRI_BOOK_DIRECTORY_COUNT();
extern VriDirectoryId       VRI_BOOK_DIRECTORY_CONTENTS();
extern VriDirectoryId       VRI_BOOK_DIRECTORY_DEFAULT();
extern VriDirectoryId       VRI_BOOK_DIRECTORY_INDEX();
extern unsigned       VRI_BOOK_FONT_COUNT();
extern unsigned       VRI_BOOK_FONT_MAX_ID();
extern char *         VRI_BOOK_FONT_NAME();
extern VriBookId  VRI_BOOK_OPEN();
extern char *         VRI_BOOK_TITLE();

/* Directory access routines
 */
extern int                VRI_DIRECTORY_ENTRY();
extern char *             VRI_DIRECTORY_NAME();
extern VriDirectoryId VRI_DIRECTORY_OPEN();
extern VriDirectoryId VRI_DIRECTORY_NEXT();

extern char *VRI_PAGE_CHUNK_TITLE();

/* Public bookshelf handling routines
 */
extern void           VRI_SHELF_CLOSE();
extern void           VRI_SHELF_ENTRY();
extern unsigned       VRI_SHELF_ENTRY_COUNT();
extern char *         VRI_SHELF_HOME_FILE();
extern VriShelfId VRI_SHELF_OPEN();
extern VriShelfId VRI_SHELF_OPENLIB();
extern char *         VRI_SHELF_TARGET_FILE();

extern char *VRI_LOGICAL();

#undef P1(type)
#undef P2(type)
#undef P3(type)
#undef P4(type)
#undef P5(type)
#undef P6(type)
#undef P7(type)
#undef P8(type)

/* end vri-public-def.h */




