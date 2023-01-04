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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: agent_authentication.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:30:46 $";
#endif
/*
 **  Title: agent_authentication.c	Perform authentication services
 **
 **  Copyright (c) Digital Equipment Corporation, 1990, 1991, 1992
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
 **
 */

/*
 **++
 **  FACILITY:  Management - POLYCENTER (tm) Common Agent
 **
 **  MODULE DESCRIPTION:
 **
 **	Perform authentication checks to prevent 'spoofing'
 **
 **  AUTHORS:
 **
 **      Pete Burgess
 **
 **  CREATION DATE:  1-Oct-1992
 **
 **  DESIGN ISSUES:
 **	
 **  MODIFICATION HISTORY:
 **
 ** 01	01-Oct-1992	Pete Burgess 	Created.
 **--
 */

/*
 *  Function Prototypes to satisfy c89 -std warnings.
 *
 *   - cma_lib_malloc() has no prototype, it is defined in
 *     /usr/include/dce/cmalib_crtlx.h to replace malloc()
 */

#include "man_data.h"

#ifndef NOIPC
extern void *cma_lib_malloc() ;
#else
#include <stdlib.h>
#endif /* NOIPC */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#ifdef RPCV2
#include <pthread.h>
#endif
#include "moss.h"
#include "extern_nil.h"
#include "man.h"
#include "ca_config.h"
#include "agent_access_config.h"

/*
 ** External functions
 */

extern int strcasecmp(), inet_addr();

#define LINEBUFSIZE 132
#define NULLPTR (void *)0
  
typedef enum e_access_mode { 
  ACCESS_MODE_NONE = 0, ACCESS_MODE_READONLY = 1,
  ACCESS_MODE_WRITEONLY = 2, ACCESS_MODE_READWRITE = 3} T_ACCESS_MODES;

typedef struct s_community {
  char            *comm_name;     /* Community Name                 */
  unsigned int    comm_addr;      /* Community Address (in network byte order) */
  T_ACCESS_MODES  access_mode;	  /* Access mode type */
  /*----char            *view_name;---*//* View Name                      */
  struct s_community *next;         /* Pointer to next in the list    */
} T_COMMUNITY;

/*
 ** Static storage
 */
static T_COMMUNITY *community_list;	/* List of community definitions */
static char *config_filename = DEFAULT_SNMPPE_CONFIG_PATH;
static char *white_space = " \t\n";
static char *read_mode = "r";

#ifdef RPCV2
static pthread_once_t  	once_block = pthread_once_init;
#endif
static int	   	init_status = -1;

/* init_config - Open and Parse the Configuration File */
static void  init_config()
{
  FILE    *config_stream;         /* Configuration File Descriptor pointer     */
  char    linebuf[LINEBUFSIZE];   /* Input line buffer                         */
  char    msgbuf[LINEBUFSIZE];    /* Crash Error messages built here           */
  char    *next_char;             /* -> Next character in input line           */
  int     lineno=0;               /* Line number in configuration file we're on*/
  char    *temp_str;              /* Temporary string pointer                  */
  T_COMMUNITY	*community;
  
  /* Initialize list of communities */
  community_list = NULLPTR;
  
  /* if (attempt to open configuration file failed), then quit */
  assert ((config_stream = fopen(config_filename, read_mode)) != NULLPTR);
  
  /*
   ** Process body of lines in configuration file,
   ** 	updating community definition list
   */
  
  while ( (next_char = fgets( linebuf, LINEBUFSIZE, config_stream)) != NULLPTR) {
    lineno++;
    
    /* span pointer into line over any whitespace at start of line */
    next_char += strspn( next_char, " \t");
    
    /* if (current character is '0' or '#' or '\n') */
    if (*next_char == '\0' || *next_char == '#' || *next_char == '\n')
      continue;
    
    /* if (parse of initial token fails) */
    if ( (next_char = strtok( next_char, " \t\n")) == NULLPTR)
      continue;
    
    /* Process (verb = "community") */
    if ( strcasecmp("community", next_char ) == 0 ) {
      
      /*
	|  We recognize a "Community" line accordingly:
	|
	|                  (Reqd)             (Reqd)             (Reqd)
	|    Verb           Arg1               Arg2               Arg3
	|    ----           ----               ----               ----
	|  community  <community name> <community inet-addr>  <accessmode>
	|
	|eg:
	|  community     PUBLIC                0.0.0.0         readonly
	|  community     OBSERVE               1.2.3.4         readonly
	|  community     MANAGE                1.2.3.5         readwrite
	|
	|  The 'spec' for this is pages 212 and 213 of Rose's
	|  'The Simple Book' (SNMP), except we make all args mandatory.
	*/
      
      /* 
       ** Create a community list block
       */
      assert((community = (T_COMMUNITY *) malloc(sizeof(T_COMMUNITY))) != NULLPTR);
      
      /* Insert the block into the Community list */
      community->next = community_list;
      community_list = community;

      /* assume "read-only" and "0.0.0.0" */
      community->access_mode = ACCESS_MODE_READONLY;
      community->comm_addr = 0;

      /* if (parse of community name failed), then quit */
      assert ((temp_str = strtok(NULLPTR, white_space)) != NULLPTR);
      
      /* Store COMMUNITY NAME field in the community block */
      assert ((community->comm_name = (char *) malloc( strlen(temp_str)+1 ) ) != NULLPTR);
      strcpy(community->comm_name, temp_str);

      /* if (parse of the IP ADDRESS failed), then quit */
      assert ((temp_str = strtok(NULLPTR, white_space)) != NULLPTR);
      
      /* convert IP address to binary and store in community block */
      assert ((community->comm_addr = inet_addr(temp_str)) != -1);
      
      /* if (parse of the ACCESS MODE failed), then quit */
      assert ((temp_str = strtok(NULLPTR, white_space)) != NULLPTR);
      
      /* set access mode in community block according to parsed value, crash exit if no match */
      if (strcasecmp( temp_str, "none" ) == 0) {
	community->access_mode = ACCESS_MODE_NONE;
	continue;
      }
      else if( strcasecmp( temp_str, "readonly" ) == 0) {
	community->access_mode = ACCESS_MODE_READONLY;
	continue;
      }
      else if ( strcasecmp( temp_str, "writeonly" ) == 0) {
	community->access_mode = ACCESS_MODE_WRITEONLY;
	continue;
      }
      else if( strcasecmp( temp_str, "readwrite" ) == 0) {
	community->access_mode = ACCESS_MODE_READWRITE;
	continue;
      }
      else {
	assert (0);	/* Else Invalid Access mode */
      }
    }
  }
  /* close the configuration file */
  fclose(config_stream);
}



static int
parse_access_control(access_control,
                     kind,
                     community_name,
                     ip_addr)

avl *access_control;
agent_access_type *kind;
char **community_name;
unsigned int *ip_addr;

{
  man_status status ;
  int access;
  octet_string *octet;

  if (access_control == NULLPTR)
    return(FALSE);

  status = moss_avl_reset(access_control);

  if (status == MAN_C_SUCCESS)
  {
    status = moss_avl_point(access_control, NULL, NULL, NULL,
			    &octet, NULL );
  }

  if (status == MAN_C_SUCCESS)
  {
    if (octet->length != sizeof( int ))
      return(FALSE);

    memcpy(&access, octet->string, sizeof( int ) );

    if (access >= MAX_AGENT_ACCESS_TYPE)
      return(FALSE);

    *kind = access;
  }

  if (status == MAN_C_SUCCESS)
  {
    status = moss_avl_point(access_control, NULL, NULL, NULL,
			    &octet, NULL );
  }

  if (status == MAN_C_SUCCESS)
  {
    if (octet->length == 0)
      return(FALSE);

    *community_name = ( char * ) octet->string;
  }

  if (status == MAN_C_SUCCESS)
  {
    status = moss_avl_point(access_control, NULL, NULL, NULL,
			    &octet, NULL );
  }

  if (status == MAN_C_SUCCESS)
  {
    if (octet->length != sizeof( unsigned int ))
      return(FALSE);

    memcpy(ip_addr, octet->string, sizeof( unsigned int ) );
  }

  if (status == MAN_C_SUCCESS)
  {
    return(TRUE);
  }
  else
  {
    return(FALSE);
  }

}
                     

/* authenticate_client - Perform Authentication checks on */

int authenticate_client(operation, access_control)
     int operation;
     avl *access_control;
     
{
  int      	status;         
  T_COMMUNITY	*community;       
  char *	ac_community_name;	/* Access Control Community name */
  unsigned int  ac_addr;		/* Access Control IP address */
  agent_access_type ac_kind;		/* Kind of access control */
  


  if (init_status == -1) 
#ifdef RPCV2
    init_status = pthread_once (&once_block, (pthread_initroutine_t)init_config);
#else
  init_config();
  init_status = 0;
#endif

  if ((status = parse_access_control(access_control, &ac_kind,
				     &ac_community_name, &ac_addr)) == FALSE)
      return(status);

  /* Assume it is FALSE until proved TRUE. */
  status = FALSE;

  if (ac_kind == SNMP_ACCESS) {
    /*
     ** Search community list for element with matching community name
     */
    for (community = community_list; community != NULLPTR; community = community->next) {
      
      /* if (community block name matches ACCESS CONTROL community) */
      if ((strlen(community->comm_name) == strlen(ac_community_name)) &&
	  (strcasecmp(community->comm_name, ac_community_name) == 0)) {
	
	/* if (community block address is zero or matches ACCESS CONTROL received-from) */
	if ( (community->comm_addr == 0) || (memcmp(&community->comm_addr, &ac_addr, sizeof(ac_addr)) == 0)) {
	  /* 
	   ** if (community mode is "none" OR                    
	   **     ACCESS CONTROL is "set" and community is "readonly")

	   */
	  if ((community->access_mode == ACCESS_MODE_NONE) ||
              (operation == MAN_C_SET && community->access_mode == ACCESS_MODE_READONLY)) {

	    return(FALSE);
	  }
	  else {
	    status = TRUE;   
	    break;
	  }
	}
      }
    }
  }
  /*
   ** Else UNSUPPORTED Access protcol
   */
  else status = FALSE;
  
  return(status);
}
