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
static char *rcsid = "@(#)$RCSfile: fddi_mom_specific.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/11/19 17:43:56 $";
#endif
/*
 *++
 *  FACILITY: 
 *
 *
 * MODULE DESCRIPTION:
 *
 *      INET_MOM_SPECIFIC.C
 *
 *      This module is part of the Managed Object Module (MOM)
 *      for rfc1285 (FDDI MOM).
 *      It performs required initialization and other functions that
 *      are specific to the rfc1285.
 *
 *  AUTHORS:
 *
 *
 *  CREATION DATE:  
 *
 *  MODIFICATION HISTORY:
 *
 *
 *--
*/
#include "moss.h"
#include "common.h"
#include "extern_common.h"
#include "fddi_mom_specific.h"



static int total_smt_got = FALSE;


fddi_mom_specific_init()
{
     int status;

     if ((inetfd = open("/dev/streams/kinfo", O_RDWR, 0)) < 0){
        return(MAN_C_FAILURE);
       }

     /* Check if the interface exists.  If not exit */

     status = create_fddi_index_info();
     if (status == MAN_C_FAILURE){
        syslog(LOG_NOTICE,"No fddi interface - fddi_mom exiting\n");
        exit(1);
     }

     return(MAN_C_SUCCESS);

}

int
create_fddi_index_info()

/*    Function Description:
 *
 *      This function initializes the structure containing information about
 *      the respective indexes - PORT, ATTACHMENT, SMT and MAC.
 *      This initializes the fddi_index_info structure that is global to
 *      the FDDI MOM.
 *
 *    Arguments:
 *      None.
 *    Return Value:
 *
 *         MAN_C_SUCCESS
 *         MAN_C_FAILURE
 *
 *    Side Effects:
 *         None.
 *
 */
{
    if_blk buf;
    struct strioctl str;
    char *intf_prefix = "f";
    int index = 0, i;
    int found_it = FALSE;
    struct index_info *tmp_index_ptr, *tmp_ptr_next;
    int smt_count = 0;
    char tmp_buf[4];

    /*
     * First free the current list we have. If this is being
     * called at start up time we won't need it. If total_smt_got
     * is true, then this is not the start up time.
     */
    tmp_index_ptr = fddi_index_info.index;
    if (total_smt_got == TRUE) {
        while (tmp_index_ptr != NULL) {
            tmp_ptr_next = tmp_index_ptr->next;
            free(tmp_index_ptr);
            tmp_index_ptr = tmp_ptr_next;
        }
    }
    bzero(&buf, sizeof(buf));
    str.ic_cmd = KINFO_GET_INTERFACES;
    str.ic_timout = 15;
    str.ic_len = sizeof(buf);
    str.ic_dp = (char *)&buf;
    do {
        if (ioctl(inetfd, I_STR, &str) < 0) {
           perror(" ioctl");
           return(MAN_C_PROCESSING_FAILURE);
        }
        for (i=0; i < buf.total_cnt; i++) {
            index++;
            /*
             * NOTE: Here we assume that name of all FDDI interfaces start
             *       with the alphabet "f".
             */
            if (strncmp(intf_prefix, buf.info[i].if_name, 1) == 0) {
                smt_count++;
                if (smt_count == 1) {
		    fddi_index_info.index =
			 (struct index_info *)malloc(sizeof(struct index_info));
                      fddi_index_info.index->next = NULL;
                      tmp_index_ptr = fddi_index_info.index;
                } else {
		      tmp_index_ptr->next = 
			(struct index_info *)malloc(sizeof(struct index_info));
                      tmp_index_ptr = tmp_index_ptr->next;
                      tmp_index_ptr->next = NULL;
                }
                tmp_index_ptr->smt_index = smt_count;
                tmp_index_ptr->mac_index = index;
                tmp_index_ptr->port_index = smt_count;
                tmp_index_ptr->attach_index = smt_count;
                sprintf(tmp_index_ptr->intf_name, "%s%d",
                buf.info[i].if_name, buf.info[i].if_unit);
            }
        }
    } while (buf.more);

    /*
     * We need to get the total number of SMTs on this station
     * If there are none, this station doesn't have any FDDI interfaces.
     * NOTE: We have to do this at the end. We can't do this at the
     *       beginning because we don't have the names of any interfaces
     *       available. The names (fza0 etc.) become known to us
     *       only after we do the above.
     */
    if (total_smt_got == FALSE) {
        find_info_by_fddi_index(1, FDDIMIB_SMT);
        fddi_index_info.total_smt = ctr_info.ctr_ctrs.smt_fddi.smt_number;

        if (fddi_index_info.total_smt <= 0)
            /*
             * If this were a seperate FDDI MOM, this should exit() here.
             */
	{
            return(MAN_C_FAILURE); /* No FDDI interfaces */
	}
        total_smt_got = TRUE;
    }
    return(MAN_C_SUCCESS);
} /* end of create_fddi_index_info() */


find_info_by_fddi_index(index,
                   counter_type)
/*    Function Description:
 *
 *    This function does an ioctl to retrieve the SMT information.
 *    This checks the index. If the index doesn't match the index
 *    requested it returns an error.
 *    NOTE: Currently we have ONLY ONE index. So, if the index given
 *          is anything other than 1 we will return and error. This
 *          behaviour should be corrected when, and if, multiple SMTs
 *          are available.
 *
 *    Arguments:
 *
 *        index                 Input: The index received.
 *        counter_type          Input: The type of counter requested.
 *    Return Value:
 *
 *      MAN_C_SUCCESS
 *      MAN_C_PROCESSING_FAILURE
 *      MAN_C_FAILURE
 *
 *    Side Effects:
 *      NOTE: ctr_info structure is global to this file. Information
 *            about the counters is filled into that.
 */
int index;
int counter_type;
{
    int s;
    int tmp_index = index;
    char *buf;
    struct index_info *index_ptr = fddi_index_info.index;

    if ( tmp_index < 0 )
	{
        return (MAN_C_FAILURE);
	}
    /*
     * Get a socket.
     */

    s = socket( AF_INET, SOCK_DGRAM, 0 );

    if (s < 0) {
        fprintf(stderr,"smt_entry.c: Could not get socket!\n");
        return(MAN_C_PROCESSING_FAILURE);
    }

    /*
     * Get the name of the interface.
     */

    while (index_ptr != NULL) {
        if (index != fddi_index_info.index->smt_index)
            index_ptr = index_ptr->next;
        else {
            buf = index_ptr->intf_name;
            break;
        }
    }

    if (index_ptr == NULL){
        return(MAN_C_FAILURE);
	}

    strcpy(ctr_info.ctr_name, buf);

    ctr_info.ctr_type = counter_type;

    if (ioctl( s, SIOCRDCTRS, &ctr_info ) < 0) {
        fprintf(stderr,"FDDI_MOM: Error getting counter info!\n");
        close(s);
        return(MAN_C_PROCESSING_FAILURE);
    }

    close(s);


    if (ctr_info.ctr_ctrs.smt_fddi.smt_number < index){
        return(MAN_C_FAILURE);
	}

    return(MAN_C_SUCCESS);
}

int
find_mac_index(fddi_index,
               mac_index)


/*    Function Description:
 *
 *      Given the FDDI index the function finds out the index of the MAC
 *      for the entire entity, as a TCP/IP MOM would see it.
 *
 *    Arguments:
 *      fddi_index              Input: The index to be used for fetching
 *                             the interface information. This is the SMT
 *                              index of the interface.
 *      mac_index               Output: The mac index
 *    Return Value:
 *
 *         MAN_C_SUCCESS
 *         MAN_C_FAILURE
 *
 *    Side Effects:
 *         None.
 *
 */

int    fddi_index;
int    *mac_index;
{
    if_blk buf;
    struct strioctl str;
    char *tmp_buf, if_name_buf[8];
    int if_name_len;
    int index = 0, i;
    int found_it = FALSE;
    struct index_info *index_ptr = fddi_index_info.index;

    if (fddi_index < 0)
        return(MAN_C_FAILURE);

    /*
     * Get the name of the interface.
     */

    while (index_ptr != NULL) {
        if (fddi_index != fddi_index_info.index->smt_index)
            index_ptr = index_ptr->next;
        else {
            tmp_buf = index_ptr->intf_name;
            break;
        }
    }
    if (index_ptr == NULL)
        return(MAN_C_FAILURE);



    bzero(&buf, sizeof(buf));
    str.ic_cmd = KINFO_GET_INTERFACES;
    str.ic_timout = 15;
    str.ic_len = sizeof(buf);
    str.ic_dp = (char *)&buf;
    do {
        if (ioctl(inetfd, I_STR, &str) < 0) {
            perror(" ioctl");
            return(MAN_C_PROCESSING_FAILURE);
        }
        for (i=0; i < MAX_IF_INFOS; i++) {
            index++;
            sprintf(if_name_buf,"%s%d", buf.info[i].if_name,
                                        buf.info[i].if_unit);
            if_name_len = strlen(if_name_buf);
            if (strncmp(tmp_buf, if_name_buf, if_name_len) == 0) {
                found_it = TRUE;
                break;
            }
        }
    } while ((buf.more) && found_it != TRUE);


    /*
     * check if index was found
     */
    if (found_it != TRUE)
        return(MAN_C_FAILURE);

    *mac_index = index;

    return(MAN_C_SUCCESS);
} /* end of find_mac_index () */

