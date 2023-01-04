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
static char *rcsid = "@(#)$RCSfile: tftpboot.c,v $ $Revision: 1.1.8.2 $ (DEC) $Date: 1993/08/02 22:38:16 $";
#endif

/*
 * TFTP bootstrap code.
 */

/*
 */

#include <aouthdr.h>

#ifdef mips
#ifdef notdef
#define CALLBACK_VECTOR
#include "callbacks.h"
#include "ipprotocols.h"
#include "lance.h" 
#endif
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>	/* ntohl(), etc.*/
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <bootpd/bootp.h>
#include <arpa/tftp.h>
#include <filehdr.h>
#include <scnhdr.h>
/* #include <stddef.h> */

#define printf  _prom_printf
#define open(f,m)\
        ((rex_base) ? rex_open((f), (m)) : _prom_open((f), (m)))
#define read    _prom_read
#define write   _prom_write
#define close   _prom_close
#define lseek   _prom_lseek
#define strcmp  prom_strcmp
#define strcat  prom_strcat
#define strlen  prom_strlen
#define strcpy  prom_strcpy
#define getenv  prom_getenv
#define setenv  prom_setenv
#define stop()  ((rex_base) ? rex_rex('h') : _prom_restart())

#define DEFAULT_TIMEOUT 5
#define MAX_ENET_PACKET 2048

#ifdef __mips__
#ifdef __MIPSEB__
/*
 * Macros for number representation conversion.
 */
#define ntohl(x)        (x)
#define ntohs(x)        (x)
#define htonl(x)        (x)
#define htons(x)        (x)
#endif /* __MIPSEB */
#ifdef __MIPSEL__
#define ntohl(x)        nuxi_l(x)
#define ntohs(x)        nuxi_s(x)
#define htonl(x)        nuxi_l(x)
#define htons(x)        nuxi_s(x)
#endif /* __MIPSEL */

#endif /* !mips */
#endif  /* mips */

int jsdflag=0;
int tot=0, sofar, spin, check=10000; 	/* For %loaded */

#ifdef mips 
extern char broadcast[];
extern char state_char[];
extern int state;
extern int debug;

/*
 * I/O and utility routines.
 */

/*******
 * index
 */
char *index(char* s1, char tc)
{
  char c;
  while (c= *s1++)
    if (c==tc)
      return --s1;
  return(NULL);
}                               /* end index */


/*
 * Simple random number generator.
 * Taken from CACM 31:10, 1988, pg 1195
 */

#define a 16807
#define m 2147483647
#define q 127773
#define r 2836

int seed;
long random(void)
{
  int lo, hi, test;

  if (!seed)
    seed = 1066;

  hi = seed / q;
  lo = seed % q;
  test = a * lo - r * hi;
  if (test > 0)
    seed = test;
  else
    seed = test + m;
  return( seed );
}                               /* end random */


/*
 * Misc.
 */
unsigned long transfer_address;	/* Transfer address of boot image */
/*extern eaddr_t enet_broadcast;	 Ethernet broadcast addr. */
char station_address[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };	/* Our station address */
int need_station_address=1; /* rex: don't know our own hardware address */

typedef struct	bootp	BOOTPMSG;
typedef struct	ip	IPHDR;
typedef struct	udphdr	UDPHDR;
typedef struct	ether_header	ETHERHDR;
typedef struct	tftphdr	TFTPHDR;	/* tftp header */
/*typedef struct	tftpmsg	TFTPMSG;	 entire tftp packet */

/*****************************************************************************
 *  TFTP routines                                                            *
 *****************************************************************************/


int bootp(int, char**, char*);
int bootp_transact(BOOTPMSG *, BOOTPMSG *);
int tftp_connect(char*);
int tftp_disconnect(void);
int tftp_load(void);
int tftp_read(char *, int);
int tftp_lseek(int);
int udp_read(char *, int);
int udp_write(char *, int);

unsigned int in_cksum(char*, int);
extern _cksum1(char *, int, int);

/*
 * MIPS specific
 */
#define N_TXTOFF(f, a) \
 ((a).magic == ZMAGIC || (a).magic == LIBMAGIC ? 0 : \
  ((a).vstamp < 23 ? \
   ((FILHSZ + AOUTHSZ + (f).f_nscns * SCNHSZ + 7) & 0xfffffff8) : \
   ((FILHSZ + AOUTHSZ + (f).f_nscns * SCNHSZ + SCNROUND-1) & ~(SCNROUND-1)) ) )

/*
 * TFTP message packet definition.
 */
typedef struct{
  TFTPHDR thdr;
  char data[SEGSIZE];
} TFTPMSG;

/*
 * ip and udp addresses.
 */
int my_ip_addr=0;
int dst_ip_addr=0;
int gateway_ip_addr=0;
int dst_port_addr=0;
int my_port_addr=0;
int last_udp_source_port=0;

/*
 * bootp transaction id.
 */
int bootp_xid=0;

/*
 * tftp input stream data structures.
 */
TFTPMSG tftp_buf;
char   *tftp_buf_ptr;
int     tftp_offset, tftp_count, tftp_eof, tftp_blkno;
static u_short	blk;

/***********
 * tftp_boot
 */
int tftp_boot(int argc, char **argv, char *fn)
{
  char  filename[255];

  strcpy(filename,fn);
  transfer_address=0;

  /*
   * Call bootp to get source, destination, and gateway ip addresses.
   * Call tftp_connect to establish connection with server.
   * Call tftp_load to actually load the file.
   * Call tftp_disconnect to close connection with server.
   */
  if (bootp(argc,argv,filename))
    return -1;
  else if (debug) 
	printf("\n\tSent and received bootp message \n");
  if (tftp_connect(filename))
	return(-1);
  else if (debug)
	printf("\tSetup tftp connection\n");

  tftp_load();
  if (debug)
	printf("\tFinished tftp load of image\n");
  tftp_disconnect();
  if (debug)
	printf("\tDisconnected tftp channel\n");
  return(transfer_address);
}				/* end tftp_boot */

/*****************
 * bootp_transact
 */
int bootp_transact(BOOTPMSG* xbp, BOOTPMSG* rbp)
{
  int sts=0,retrys=5;
  time_t tod;

  /*
   * Repeat transaction attempt up to 5 times.
   *   Send a request with hdwr addr 0-0-0-0-0-0-0 first time.
   *   Grab our ethernet address from the ethernet header (driver added it)
   *   Then send the real bootp request
   *   Read response.
   *     If error, retry.
   *     If success, exit.
   *     If no response, repeat until timeout, then retry.
   */
  while (retrys--){
    tod = rex_time(NULL);
    if (need_station_address > 0) {
        if (need_station_address > 1) { /* got it */
		bcopy(&station_address, &xbp.bp_chaddr, 6);
       		need_station_address = 0;
	} else {
		tod -= 5; /* force retry first time around */
		retrys++;
	}
    }
    udp_write( (char*)xbp, sizeof *xbp);
    sts=0;
    while (rex_time(NULL) < (tod+5)) {
      sts = udp_read( (char*)rbp, sizeof *rbp);
      if (sts>0)
	return(0);
    }
  }
  return -1;
}				/* end bootp_transact */
/*******
 * bootp
 *
 * Broadcast a bootp packet to find out our ip address,
 * the server ip address, any gateway ip address (not used).
 */
int bootp(int argc, char **argv, char *filename)
{
  char buf[255], *file, *host;
  BOOTPMSG xbp, rbp;
  static unsigned char bp_vend[64];

  /*
   * Set our addresses for bootp transaction.
   */
  dst_ip_addr = INADDR_BROADCAST;
  dst_port_addr = htons( IPPORT_BOOTPS);
  my_port_addr  = htons( IPPORT_BOOTPC);
  gateway_ip_addr = 0;

  /*
   * Parse the filename into host and file.
   */
  if (strlen(filename) > sizeof buf -2)
    goto ftl_err1;
  strcpy(buf,filename);
  if ((file=index(buf,':'))==NULL){
    file=buf;
    host=NULL;
  }
  else{
    host=buf;
    *file++='\0';
  }
  if (( host!=NULL && strlen(host)>=sizeof(xbp.bp_sname) ) ||
      (strlen(file)>=sizeof(xbp.bp_file)))
    goto ftl_err2;
  /*
   * Generate the BOOTP request message.
   * Note that we generate a random udp address between 1024 and 65280.
   */
  bzero(&xbp, sizeof(xbp));
  xbp.bp_op = BOOTREQUEST;
  xbp.bp_htype = ARPHRD_ETHER;
  xbp.bp_hlen  = 6;
  xbp.bp_xid = bootp_xid = 1893;/* XXX: not random */
  xbp.bp_ciaddr.s_addr = my_ip_addr;
  bcopy(&station_address, &xbp.bp_chaddr, 6);/* nop, address not know (yet) */
  if (host)
    strcpy(xbp.bp_sname,host);
  if (file)
    strcpy(xbp.bp_file,file);

  /*
   * Send BOOTP request and get response.
   * Store ip addresses.
   * Store the address of the bootp vendor block in an ev.
   * Check that there is a filename specified.
   * Copy the filename to the callers filename string.
   */
  if (!bootp_transact(&xbp,&rbp)){
    if (my_ip_addr==0)
      my_ip_addr = rbp.bp_yiaddr.s_addr;
    dst_ip_addr = rbp.bp_siaddr.s_addr;
    gateway_ip_addr = rbp.bp_giaddr.s_addr;
    bcopy(&rbp.bp_vend, &bp_vend, sizeof bp_vend); /* XXX */

    /*** XXX: no vendor stuff for now:
    sprintf(buf,"0x%x",bp_vend);
     ***/

    if (strlen(rbp.bp_file)==0)
      goto fnf_err;
    strcpy(filename,rbp.bp_file);
    return(0);
  }

 /* fallthrough */ 
 bootp_err:
  printf("bootp: timeout error\n");
  return(-1);

 fnf_err:
  printf("bootp: file not found: %s\n",filename);
  return(-1);
  
 ftl_err1:
  printf("bootp: name1 too long: %s\n", filename);
  return(-1);

 ftl_err2:
  printf("bootp: name2 too long: %s\n", file);
  return(-1);
}				/* end bootp */

#define IPPORT_TFTP	69

/***************
 * tftp_transact
 *
 * Send message.  If message is not ERROR, await response.
 *
 * Return: 0 Success
 *        -1 Failure.
 */
int tftp_transact(int code, char *filename)
{
  TFTPMSG msg;
  int i,size,sts=0;

  /*
   * Build code message.
   */
  bzero(&msg, sizeof msg);
  switch(code){
  case RRQ:
    size = strlen(filename);

    blk = 0;	 /* Set our block (sequence) count to zero XXXX */
    msg.thdr.th_opcode = htons(RRQ);
    strcpy(msg.thdr.th_u.tu_stuff,filename);
    strcpy(msg.thdr.th_u.tu_stuff+size+1,"octet");
    size +=9;
    dst_port_addr = htons(IPPORT_TFTP);
    break;
  case ACK:
    /*if (tftp_blkno != blk+1) {
	    msg.thdr.th_u.tu_block = htons(blk); 
    } else {
	    blk++;
	    msg.thdr.th_u.tu_block = htons(tftp_blkno);
    } */
    msg.thdr.th_u.tu_block = htons(tftp_blkno);
    msg.thdr.th_opcode = htons(ACK);
    size = 4;
    break;
  case ERROR:
    msg.thdr.th_opcode = htons(ERROR);
    msg.thdr.th_u.tu_code = htons(ENOSPACE);
    size = 5;
    break;
  }
  /*
   * If its RRQ, choose a different port address for each send.
   * Send the message.
   * If tftp_eof is true, or if message is ERROR, quit.
   * Wait for a response for upto 10 seconds.
   * If RRQ, Use a different UDP port for each try.
   * Repeat 4 times.
   */
  for (i=0; sts<=0 && i<4; i++){
    int tod = rex_time(NULL)+5, delay=5;
    int tod2 = rex_time(NULL)+7, delay2=7;
    if (code==RRQ) {
	    my_port_addr = htons(++bootp_xid & 0xffff);
	    if (debug)
	       printf("\tSending tftp request for boot image\n");
    }

    udp_write((char*)&msg, size);
    if (tftp_eof || code==ERROR)
	    return(0);
for (i=0; i < 6000; i++);	/* back off */
    sts=0;
    while (sts==0) {
      sts = udp_read( (char*)&tftp_buf, sizeof tftp_buf);
      if (sts==0) {
	if (rex_time(NULL) > tod) {
	    tod = rex_time(NULL) + delay;
	    delay += 5;
	    sts=0; /* nop */
	    if (i == 3) {
		printf("\ntftp_transact: timed out\n");
		return(-1);
	    } else {
		sts++; /* interval timer expired, force top of loop */
	    }
	}
      }
      if (sts < 0) {
	sts = 0; /* retry on errors */
      } else {
	 /*
	  * Block checking...problem with checking after 1st block XXXX 
	  */
	      if (ntohs(tftp_buf.thdr.th_u.tu_block) != blk+1) {	
		sts = 0;
		if (rex_time(NULL) > tod2) {
			return(-2);
		}
	        if (debug > 1)
		  printf("Wrong block received - %d, expected - %d\n", ntohs(tftp_buf.thdr.th_u.tu_block), (blk+1));
	      } else {
		blk++;
		tod2 = rex_time(NULL) + delay2;
	      }
#ifdef notdef
#endif
	
      }
    }
  }
  /*
   * If we failed to read a response, quit.
   * Otherwise check the response.
   */
  if (sts<0)
    return(sts);

  switch(ntohs(tftp_buf.thdr.th_opcode)){
    /*
     * Data packet.  Update pointers/counters, etc.
     */
  case DATA:
    tftp_buf_ptr = tftp_buf.th_data;
    tftp_count = sts-4;
    tftp_eof = tftp_count < SEGSIZE;
    tftp_blkno = ntohs(tftp_buf.thdr.th_u.tu_block);
    dst_port_addr = last_udp_source_port;
    return(0);
    /*
     * Error.  Print error message and quit.
     */
  case ERROR:
    printf("tftp_transact: tftp sequence error(?): %d - \"%s\"\n",
	   ntohs(tftp_buf.thdr.th_u.tu_code), (char*)&tftp_buf + 4);
    return(-1);
    /*
     * Some other type of message.
     */
  default:
    printf("tftp_transact: message error (%d)\n",
	   tftp_buf.thdr.th_opcode);
    return(-1);
  }
}				/* end tftp_transact */
/**************
 * tftp_connect
 *
 * tftp_transact does all the work.  If the connect fails, quit.
 */
int tftp_connect(char *filename)
{
  /*
   * Init read stream offset.
   */
  tftp_offset = tftp_eof = tftp_count = 0;

  /*
   * tftp_transact forms the connection.
   */
  return(tftp_transact(RRQ,filename));
}				/* end tftp_connect */





/*****************
 * tftp_disconnect
 *
 * If the eof bit is set, send ack for last buffer.  Otherwise send
 * ERROR to stop the server.
 */
int tftp_disconnect(void)
{
  tftp_transact(tftp_eof ? ACK : ERROR, NULL);
}				/* end tftp_disconnect */
/***********
 * tftp_load
 *
 * Read in file in a.out format.
 */
int tftp_load(void)
{
  int errcode;

  /*
   * File header data.
   */
  struct execinfo{
    struct filehdr fh;
           AOUTHDR ah;
  } ei;
    
  /*
   * Read the header.
   * Check magic numbers.
   */
  if (debug) {
	printf("\tReading in file header\n");
	printf("\tSizeof ei - %d\n", sizeof(ei));	/* XXX */
  }
  if (tftp_read((char*)&ei, sizeof(ei)) != sizeof(ei)) {
    errcode = -1;
    goto bad;
  }
  if (ei.fh.f_magic!=MIPSELMAGIC) {
    errcode = -2;
    goto bad;
  }
  if (N_BADMAG(ei.ah)) {
    errcode = -3;
    goto bad;
  }

  tot = (ei.ah.tsize + ei.ah.dsize);
  sofar = 0;
  spin = 1;
  /*
   * number of bytes per 1% of image loaded
   * we transfer 512 bytes per packet (XXX: fix this to 1024?
   */
  check = (tot/(100*512));
  /*
   * Remove any previous printed char.
   */
  printf("\b\b\b   \nPercentage loaded:   0  ");

  /*
   * Skip to the text section.
   * Read it in.
   */
  tftp_lseek(N_TXTOFF(ei.fh, ei.ah));
  if (debug) {
	printf("\n\tReading in TEXT portion of code\n");
	printf("\b\b\b   \nPercentage loaded:   0  ");
  }
  if (tftp_read((char*)ei.ah.text_start, ei.ah.tsize)!= ei.ah.tsize) {
    errcode = -5;
    goto bad;
  }
  /*
   * We are already positioned at the data section.
   * Read it in.
   */
  if (debug) {
	printf("\n\tReading in DATA portion of code\n");
	/* printf("\b\b\b   \nPercentage loaded:   0  "); */
  }
  if (tftp_read((char*)ei.ah.data_start, ei.ah.dsize) != ei.ah.dsize) {
    errcode = -5;
    goto bad;
  }
  
  /*
   * Zero the bss.
   * Set execution address.
   * Exit.
   */

  printf("\b\b\b\b100  ");
  printf("\n");
  rex_memset((char *)ei.ah.bss_start, ei.ah.bsize);
  transfer_address = ei.ah.entry;
  return 0;

 bad:
  /*
   * Print error message.
   *
   * Errcode:  -1    Invalid size.
   *           -2    Invalid byte order.
   *           -3    Invalid magic.
   *           -4    
   *           -5    Read error.
   */
  printf("tftp_load: a.out error (%d)\n", errcode);
  return -1;
}				/* end tftp_load */
/***********
 * tftp_read
 */
int tftp_read(char *buf, int cnt)
{
  int i,sts,xcnt=cnt;

  /*
   * Copy data to caller buffer.
   */
  while (cnt){
    while(cnt>0 && tftp_count>0){
      if (spin++ >= check) {
	sofar += (check * 512);
	spin = 1;
	printf("\b\b\b\b%2d  ", (100*sofar/tot));
      }
      printf("\b%c", state_char[state++]);
      if (state >= 4)
	state = 0;

      i = (tftp_count < cnt) ? tftp_count : cnt;
      bcopy((char *)tftp_buf_ptr, (char *)buf, i); 
      buf += i; tftp_buf_ptr +=i; cnt -= i; tftp_count -=i;
    }
    if (cnt)
      /* if (tftp_transact(ACK,NULL))
	break; */
      for (i=0; tftp_transact(ACK,NULL) || i>4; i++)
	break; 
  }
  xcnt -= cnt;
  tftp_offset += xcnt;
  return(xcnt);
}				/* end tftp_read */

/************
 * tftp_lseek
 *
 * Skips to the desired position.  Only goes forward, won't go backwards.
 */
int tftp_lseek(int pos)
{
  char c;

  if (pos<tftp_offset){
    printf("tftp_lseek: error: negative lseek (%d, %d)\n", pos, tftp_offset);
    return(-1);
  }
  while ((pos!=tftp_offset) && (!tftp_eof)) {
    tftp_read(&c,1);
    spin--;
  }
  return(tftp_offset);
}				/* end tftp_lseek */
/*****************************************************************************
 *  UDP/IP I/O Routines                                                      *
 *****************************************************************************/

/**********
 * in_cksum
 */
unsigned int in_cksum(char* addr, int len)
{
  int ck;
  if ((int)addr & 1)
    ck = nuxi_s(_cksum1(addr,len,0));
  else
    ck = _cksum1(addr,len,0);
  return( ~ck & 0xffff);
}				/* end in_cksum */
/**********
 * udp_read
 */
int udp_read(char *buf, int cnt)
{
  char buffer[MAX_ENET_PACKET/2];
  struct ether_header *eh = (struct ether_header*)&buffer[2];
  IPHDR *iph = (IPHDR*)((char*)eh + sizeof(struct ether_header));
  UDPHDR *udph = (UDPHDR*)((char*)iph + sizeof(IPHDR));
  int sts;

  /*
   * Read in a buffer.
   * Exit if empty or if a read error.
   */
  sts = rex_bootread(0, (char*)eh, sizeof buffer);
  if (sts<=0)
    return(sts);

  /*
   * Verify that the packet is an IP packet.
   */
  if (eh->ether_type != ntohs(ETHERTYPE_IP)) {
  /*printf("udp_read: got packet type 0x%x (not IP)\n", eh->ether_type); */
    return 0;
  }

  /*
   * Validate IP data...
   *   Check the header checksum.
   *   Check that length is le than packet size.
   *   Check that it is a udp packet.
   */
  if (in_cksum((char*)iph,sizeof *iph)) {
	printf("udp_read: in_cksum failed\n");
    return(-1);
  }
  if (ntohs(iph->ip_len) > sts) {
	printf("udp_read: short packet\n");
    return(-2);
  }

  if (iph->ip_p != IPPROTO_UDP) {
	printf("udp_read: not UDP packet\n");
    return(0);
  }

  /*
   * Validate UDP data...
   *  Length checks.
   *  Check that it is to us.
   *  Record where it came from.
   */
  sts -= sizeof *iph;
  udph->uh_ulen = ntohs(udph->uh_ulen);
  if (udph->uh_ulen > sts)
    return(-3);

  sts = udph->uh_ulen - sizeof *udph;
  if (udph->uh_dport != my_port_addr)
    return(0);
/*printf("udp_read: got bootp reply!!\n"); */
  if (sts > cnt)
    return(-4);
  last_udp_source_port = udph->uh_sport;

  /*
   * Copy data to caller buffer.
   */
  bcopy((char*)udph + sizeof *udph, (char *)buf, sts);
  return(sts);
}				/* end udp_read */
/***********
 * udp_write
 *
 * Note that eh is setup to point to an odd halfword.  This then causes the
 * ip header and udp headers to be word aligned.
 */
int udp_write(char *buf, int cnt)
{
  char buffer[MAX_ENET_PACKET/2];
  struct ether_header *eh = (struct ether_header*)((char*)buffer + 2);
  IPHDR *iph = (IPHDR*)((char*)eh + sizeof(struct ether_header));
  UDPHDR *udph = (UDPHDR*)((char*)iph + sizeof(IPHDR));
  char *data = (char*)udph + sizeof *udph;
  static unsigned int ip_id;
  int ip_len;
  int ret=0;

  /*
   * Copy data to buffer.
   */
  bcopy((char *)buf, (char *)data, cnt);

  /*
   * Fill in the ethernet header.  Call arp to get the ethernet address of
   * the destination ip address.
   */
  if (arp_resolve(dst_ip_addr,eh->ether_dhost)) {
	if (debug)
	   printf("Unable to arp resolve IP address (%d)\n", dst_ip_addr); 
	return(-1); 
  }
  
  eh->ether_type = htons( ETHERTYPE_IP );
  /*
   * Construct ip header.
   */
  bzero((char *)iph, sizeof(struct ip));

  iph->ip_v =  IPVERSION;
  iph->ip_hl = sizeof(struct ip)>>2;
  iph->ip_len = htons( ip_len = (sizeof(struct udphdr) + sizeof(struct ip) + cnt));

  iph->ip_id = htons(ip_id++);
  iph->ip_off= htons(IP_DF);

  iph->ip_ttl = MAXTTL;
  iph->ip_p = IPPROTO_UDP;

  iph->ip_src.s_addr = my_ip_addr;
  iph->ip_dst.s_addr = dst_ip_addr;
  
  iph->ip_sum = in_cksum( (char*)iph,sizeof(IPHDR));
  
  /*
   * Construct udp header.  (No checksum.)
   */
  udph->uh_sport = my_port_addr;
  udph->uh_dport = dst_port_addr;
  udph->uh_ulen  = htons(cnt + sizeof *udph);
  udph->uh_sum = 0;

  /*
   * Transmit the packet.
   */
  ret=rex_bootwrite( 0, (char*)eh, ip_len + sizeof *eh);

  if (need_station_address) {
	bcopy((caddr_t) eh->ether_shost, &station_address, 6);
  	need_station_address++; /* ==2 */
  }
  return(ret);
}				/* end udp_write */

/*
 * Ethernet address comparison rtn.
 * Our ethernet address. 
 * Transmit routine.
 * Receive routine.
 * Callback vector
 * IP addresses.
 */
extern int eaddr_match(eaddr_t,eaddr_t);
void arp_input(char *,int);

/**********
 * listener
 */
int listener(char *buffer, int cnt)
{
  int sts=cnt;
  ETHERHDR *eh = ( ETHERHDR *)buffer;
    /*
     * Check for arp reply messages.  If so, call arp_input and set
     * 0 as status.
     */
    if (eh->ether_type==htons(ETHERTYPE_ARP)){
      arp_input(buffer,cnt);
      sts = 0;
    }
  return sts;
}				/* end listener */


/*
 * Forward routines.
 */
long _random(void);
int  arp_resolve(int, char ea[6]);
void arp_transact(int);

typedef unsigned short uhword;
typedef struct ether_arp ARP_PACKET; 
/***************************************************************************
* 
* ARP routines                                                             
* 
****************************************************************************/ 
/* 
 * ARP table.  Contains ethernet<->ip address pairs.  
 */ 
#define ARP_TABLE_SIZE 12 
struct arp_data { 
	int     ip_addr;		/* STORED IN NETWORK ORDER */ 
	char 	  enet_addr[6]; 
	enum    {isempty, isvalid} flags; 
	} arp_table[ARP_TABLE_SIZE]; 

/*********** 
 * print_arp 
 */ 
void print_arp(int idx)
{
  int i;
  printf("arp entry: %d, flags: %d, ip: %x, enet: ",
	 idx, arp_table[idx].flags, arp_table[idx].ip_addr);
  for (i=0;i<6;i++)
    printf("%02x ", arp_table[idx].enet_addr[i]);
  printf("\n");
}				/* end print_arp */

/************
 * arp_lookup
 */
int arp_lookup(int ip)
{
  int j;
  for (j=0; j<ARP_TABLE_SIZE; j++){
    if (arp_table[j].flags==isvalid && arp_table[j].ip_addr==ip)
      return j;
  }
  return -1;
}				/* end arp_lookup */

/*************
 * arp_resolve
 */
int arp_resolve(int ip, char *ea)
{
  int i,j;
  /*
   * If the ip address is broadcast, use broadcast ethernet address.
   * Look up the address in the table.  If found, return it.  If not
   * found, call arp_transact to get address from a server.
   * Repeat the loop 3 times.
   */
  if (ip==INADDR_BROADCAST){
    rex_memset((char*)ea, 0xff, 6);
    return(0);
  }
  for (i=0; i<3; i++){
    j = arp_lookup(ip);
    if (j>=0){
      bcopy((char*)arp_table[j].enet_addr, (char*)ea, 6);
      return(0);
    }
    arp_transact(ip);
  }
  return(-1);
}				/* end arp_resolve */

/**************
 * arp_transact
 *
 * Broadcast an arp query message.  Then read the input stream until
 * a arp message is received or 5 seconds elapses.
 */
void arp_transact(int ip)
{
  typedef struct{
    uhword dst[3];
    uhword src[3];
    uhword protocol;
    ARP_PACKET ap
    } APKT ;
  APKT pkt;
  static APKT init_pkt  = { -1, -1, -1, 0, 0, 0, 0, { 0, 0, 6, 4} };
  time_t tod;
  char buffer[MAX_ENET_PACKET/4];
  int sts;
		    
  /*
   * Init buffer.
   */
  bcopy( &init_pkt, &pkt, sizeof init_pkt);
  pkt.protocol = htons( ETHERTYPE_ARP);
  pkt.ap.arp_pro = htons( ETHERTYPE_IP);
  pkt.ap.arp_hrd = htons(ARPHRD_ETHER);
  pkt.ap.arp_op  = htons(ARPOP_REQUEST);

  /*
   * Enter our ethernet address and ip address.
   * Enter target ip address.
   */
  bcopy( &station_address, &pkt.ap.arp_sha, 6);
  bcopy( &my_ip_addr, &pkt.ap.arp_spa, 4);
  bcopy( &ip, &pkt.ap.arp_tpa, 4);

  /*
   * Transmit the packet.
   */
  rex_bootwrite(0, (char*)&pkt, sizeof pkt);

  /*
   * Now read packets for up to five seconds or when arp_lookup returns
   * a positive value.
   */
  tod = rex_time(NULL); /* XXX call rex time */
  while( (rex_time(NULL) < (tod+5)) && arp_lookup(ip)<0){
    sts = rex_bootread(0, buffer, sizeof buffer);
    sts = listener(buffer,sts);
    }
}				/* end arp_transact */


/***********
 * arp_input
 *
 * rpc 826
 */
void arp_input(char *buffer, int cnt)
{
  ARP_PACKET *apkt = (ARP_PACKET*)(buffer + sizeof(struct ether_header));
  int target_ip_addr, sender_ip_addr;
  int tidx;

#define MERGE_FLAG (tidx<ARP_TABLE_SIZE)

  /*
   * Quit if the hardware type isn't ethernet or if the protocol
   * type isn't IP.
   */
  if ((ntohs( apkt->arp_hrd)!=ARPHRD_ETHER) ||
      (ntohs( apkt->arp_pro)!=ETHERTYPE_IP))
    return;

  /*
   * Next extract the protocol addresses (internet addresses).  Retain in
   * network order.
   */
  bcopy( (caddr_t)apkt->arp_tpa, &target_ip_addr, 4);
  bcopy( (caddr_t)apkt->arp_spa, &sender_ip_addr, 4);

  /*
   * Now look the address up to see if we already have it.  If we do, update
   * the entry with the hardware address.
   */
  for (tidx=0; tidx<ARP_TABLE_SIZE; tidx++)
    if ((arp_table[tidx].flags==isvalid) &&
	(arp_table[tidx].ip_addr==sender_ip_addr)){
      bcopy( (caddr_t)apkt->arp_sha, &arp_table[tidx].enet_addr, 6);
      break;
    }
  
  /*
   * If we are the target, continue processing.
   */
  if (target_ip_addr==my_ip_addr){
    if (!MERGE_FLAG){
      /*
       * No entry from the sender is already in the table.
       * Insert the sender protocol and hardware address pair in the
       * table.
       * If we don't find an empty slot, pick a victum slot at random.
       * Enter the sender data in the table.
       */
      for (tidx=0;tidx<ARP_TABLE_SIZE;tidx++)
	if (arp_table[tidx].flags==isempty)
	  break;
      if (tidx>=ARP_TABLE_SIZE)
	tidx = random() % ARP_TABLE_SIZE;

      arp_table[tidx].flags=isvalid;
      arp_table[tidx].ip_addr=sender_ip_addr;
      bcopy( (caddr_t)apkt->arp_sha, &arp_table[tidx].enet_addr, 6);
      
      /*
       * If the packet is a request packet we must send a response
       * packet.
       */
      if (ntohs( apkt->arp_op)==ARPOP_REQUEST){
	/*
	 * Move sender data to target data.
	 * Put our address into the sender fields.
	 * Set the opcode to reply.
	 * Transmit the packet.
	 */
	bcopy( (caddr_t)apkt->arp_sha,   (caddr_t)apkt->arp_tha, sizeof(apkt->arp_sha));
	bcopy((caddr_t)apkt->arp_spa, (caddr_t)apkt->arp_tpa, sizeof(apkt->arp_spa));
	bcopy( &station_address, (caddr_t)apkt->arp_sha, sizeof(apkt->arp_sha));
	bcopy( &my_ip_addr, (caddr_t)apkt->arp_spa, sizeof(apkt->arp_spa));
	apkt->arp_op = htons(ARPOP_REPLY);
	
	rex_bootwrite(0,buffer,cnt);
      }
    }
  }
}				/* end arp_input */

				/* END MODULE:  listener.c */
#endif  /* mips */

#ifdef __alpha
/***********
 * alpha_tftp_read
 *
 * NOP for now, just so netload will build.
 */
int alpha_tftp_read(char *buf, int cnt)
{
	return(0);
}
#endif /* __alpha */
